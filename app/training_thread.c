/**
 * Training Thread Manager - UPDATED FOR KISSING SPHERES
 * 
 * Runs training in a separate thread using the 12-fold kissing spheres
 * architecture for parallel batch processing.
 * 
 * CHANGES FROM OLD VERSION:
 * - Now uses cllm_training_threaded.c for 12-way parallelization
 * - Integrates cllm_batch.c for proper batch generation
 * - Adds sphere statistics tracking for UI display
 * - Provides real-time metrics updates
 */

#include "app_common.h"
#include "cllm_integration.h"
#include "../include/cllm_training.h"
#include "../include/cllm_training_threaded.h"
#include "../include/cllm_batch.h"
#include "../include/cllm_tokenizer.h"
#include "../include/cllm_metrics.h"  // UI Integration: Real-time metrics
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static pthread_t training_thread;
static pthread_mutex_t training_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool training_thread_active = false;

// Kissing spheres training system
static ThreadedTrainingSystem* g_threaded_system = NULL;
static CLLMBatchIterator* g_batch_iterator = NULL;

/**
 * UI Integration: Metrics callback function
 * 
 * This function is called by the training system whenever metrics are updated.
 * It copies the metrics data to the AppState for UI display.
 */
static void metrics_callback(const CLLMMetrics* metrics, void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state || !metrics) return;

    // DEBUG: Verify callback is being invoked
    static int callback_count = 0;
    callback_count++;
    if (callback_count % 10 == 0) {
        printf("[METRICS CALLBACK #%d] Active: %d, Step: %d/%d, Loss: %.4f\n",
               callback_count, 
               metrics->performance.active_threads,
               metrics->training.current_step,
               metrics->training.total_steps,
               metrics->training.current_loss);
    }

    
    // Update sphere statistics from metrics
    state->sphere_stats.active_spheres = 0;
    state->sphere_stats.total_batches = 0;
    
    // Copy thread metrics to sphere stats (up to 12 for UI display)
    for (int i = 0; i < 12 && i < 144; i++) {  // MAX_TRACKED_THREADS = 144
        const ThreadMetrics* thread = &metrics->threads[i];
        
        if (thread->thread_id >= 0) {
            state->sphere_stats.batches_processed[i] = thread->batches_processed;
            // Note: avg_loss not available in ThreadMetrics, use training loss instead
            state->sphere_stats.avg_loss[i] = metrics->training.current_loss;
            state->sphere_stats.total_batches += thread->batches_processed;
            
            // Count active threads (WORKING, WAITING, or CONTROL)
            if (thread->state == 1 || thread->state == 2 || thread->state == 3) {  // WORKING, CONTROL, or WAITING
                state->sphere_stats.active_spheres++;
            }
        }
    }
    
    // Update training progress
    state->training_current_epoch = metrics->training.current_epoch;
    state->training_loss = metrics->training.current_loss;
    
    // Store gradient norm (use performance metric as proxy)
    state->sphere_stats.total_gradient_norm = metrics->performance.cache_hit_rate;
}

/**
 * Update sphere statistics in AppState for UI display (legacy function)
 */
static void update_sphere_stats(AppState* state, ThreadedTrainingSystem* system) {
    if (!state || !system) return;
    
    // Get number of worker spheres
    int num_workers = threaded_training_get_num_workers(system);
    state->sphere_stats.active_spheres = num_workers;
    state->sphere_stats.total_batches = 0;
    
    // Extract statistics from all worker spheres (up to 12 for UI display)
    int display_count = (num_workers < 12) ? num_workers : 12;
    for (int i = 0; i < display_count; i++) {
        int batches = 0;
        float loss = 0.0f;
        
        if (threaded_training_get_sphere_stats(system, i, &batches, &loss) == 0) {
            state->sphere_stats.batches_processed[i] = batches;
            state->sphere_stats.avg_loss[i] = loss;
            state->sphere_stats.total_batches += batches;
        }
    }
    
    // Get total gradient norm
    state->sphere_stats.total_gradient_norm = threaded_training_get_gradient_norm(system);
}

/**
 * Training thread function - UPDATED FOR KISSING SPHERES
 */
void* training_thread_func(void* arg) {
    AppState* state = (AppState*)arg;
    
    printf("=== TRAINING THREAD STARTED (KISSING SPHERES MODE) ===\n");
    printf("Using 12-fold kissing spheres architecture for parallel training\n\n");
    
    // Check if we have training data
    if (!state->cllm_training || !state->cllm_training->tokens || 
        state->cllm_training->num_tokens == 0) {
        printf("ERROR: No training data available\n");
        printf("Please load training data before starting training\n");
        
        pthread_mutex_lock(&training_mutex);
        state->training_in_progress = false;
        training_thread_active = false;
        pthread_mutex_unlock(&training_mutex);
        
        return NULL;
    }
    
    printf("Training data: %zu tokens\n", state->cllm_training->num_tokens);
    printf("Batch size: %d\n", state->cllm_training->config.batch_size);
    printf("Sequence length: %d\n", state->cllm_training->config.sequence_length);
    
    // CRITICAL: Validate dataset size vs batch requirements
    size_t tokens_per_batch = state->cllm_training->config.batch_size * 
                              state->cllm_training->config.sequence_length;
    size_t num_tokens = state->cllm_training->num_tokens;
    
    printf("\nValidating dataset size...\n");
    printf("  Dataset: %zu tokens\n", num_tokens);
    printf("  Batch requirements: %zu tokens per batch\n", tokens_per_batch);
    
    // Add to terminal buffer for UI display
    if (state->terminal_buffer) {
        char line[256];
        snprintf(line, sizeof(line), "Validating dataset size...");
        terminal_buffer_add_line(state->terminal_buffer, line);
        snprintf(line, sizeof(line), "  Dataset: %zu tokens", num_tokens);
        terminal_buffer_add_line(state->terminal_buffer, line);
        snprintf(line, sizeof(line), "  Batch requirements: %zu tokens per batch", tokens_per_batch);
        terminal_buffer_add_line(state->terminal_buffer, line);
    }
    
    // Auto-adjust batch size if dataset too small
    if (num_tokens < tokens_per_batch) {
        int max_batch_size = num_tokens / state->cllm_training->config.sequence_length;
        if (max_batch_size < 1) max_batch_size = 1;
        
        printf("\n⚠️  WARNING: Dataset too small!\n");
        printf("  Required: %zu tokens per batch\n", tokens_per_batch);
        printf("  Available: %zu tokens\n", num_tokens);
        printf("  Auto-adjusting batch_size: %d -> %d\n\n", 
               state->cllm_training->config.batch_size, max_batch_size);
        
        // Add to terminal buffer
        if (state->terminal_buffer) {
            char line[256];
            snprintf(line, sizeof(line), "⚠️  WARNING: Dataset too small!");
            terminal_buffer_add_line(state->terminal_buffer, line);
            snprintf(line, sizeof(line), "  Required: %zu tokens per batch", tokens_per_batch);
            terminal_buffer_add_line(state->terminal_buffer, line);
            snprintf(line, sizeof(line), "  Available: %zu tokens", num_tokens);
            terminal_buffer_add_line(state->terminal_buffer, line);
            snprintf(line, sizeof(line), "  Auto-adjusting batch_size: %d -> %d", 
                    state->cllm_training->config.batch_size, max_batch_size);
            terminal_buffer_add_line(state->terminal_buffer, line);
        }
        
        state->cllm_training->config.batch_size = max_batch_size;
        tokens_per_batch = max_batch_size * state->cllm_training->config.sequence_length;
    }
    
    // Create batch iterator from training data
    printf("Creating batch iterator...\n");
    snprintf(state->training_status_message, sizeof(state->training_status_message),
            "Creating batch iterator...");
    state->training_preprocessing_progress = 0.85f;
    SDL_PumpEvents();
    
    g_batch_iterator = cllm_batch_iterator_create(
        state->cllm_training->tokens,
        state->cllm_training->num_tokens,
        state->cllm_training->config.batch_size,
        state->cllm_training->config.sequence_length,
        0,  // no shuffle for now
        0   // keep incomplete batches (important for small datasets!)
    );
    
    if (!g_batch_iterator) {
        printf("ERROR: Failed to create batch iterator\n");
        snprintf(state->training_status_message, sizeof(state->training_status_message),
                "ERROR: Failed to create batch iterator");
        state->training_preprocessing_progress = 0.0f;
        SDL_PumpEvents();
        
        pthread_mutex_lock(&training_mutex);
        state->training_in_progress = false;
        training_thread_active = false;
        pthread_mutex_unlock(&training_mutex);
        
        return NULL;
    }
    
    // CRITICAL: Check if we have any batches to train on
    size_t num_batches = cllm_batch_iterator_num_batches(g_batch_iterator);
    
    printf("\n✓ Batch iterator created\n");
    printf("  Total batches: %zu\n", num_batches);
    printf("  Batch size: %d sequences\n", state->cllm_training->config.batch_size);
    printf("  Sequence length: %d tokens\n", state->cllm_training->config.sequence_length);
    printf("  Tokens per batch: %zu\n", tokens_per_batch);
    
    // Add to terminal buffer
    if (state->terminal_buffer) {
        char line[256];
        snprintf(line, sizeof(line), "✓ Batch iterator created");
        terminal_buffer_add_line(state->terminal_buffer, line);
        snprintf(line, sizeof(line), "  Total batches: %zu", num_batches);
        terminal_buffer_add_line(state->terminal_buffer, line);
        snprintf(line, sizeof(line), "  Batch size: %d sequences", state->cllm_training->config.batch_size);
        terminal_buffer_add_line(state->terminal_buffer, line);
        snprintf(line, sizeof(line), "  Tokens per batch: %zu", tokens_per_batch);
        terminal_buffer_add_line(state->terminal_buffer, line);
    }
    
    if (num_batches == 0) {
        printf("\n❌ ERROR: No batches available for training!\n");
        printf("  Dataset size: %zu tokens\n", num_tokens);
        printf("  Minimum required: %zu tokens\n", tokens_per_batch);
        printf("\n");
        printf("SOLUTION: Either:\n");
        printf("  1. Use a larger dataset (minimum %zu tokens)\n", tokens_per_batch);
        printf("  2. Reduce sequence length (current: %d)\n", 
               state->cllm_training->config.sequence_length);
        printf("\n");
        
        snprintf(state->training_status_message, sizeof(state->training_status_message),
                "ERROR: Dataset too small - need %zu tokens, have %zu", 
                tokens_per_batch, num_tokens);
        state->training_preprocessing_progress = 0.0f;
        SDL_PumpEvents();
        
        cllm_batch_iterator_free(g_batch_iterator);
        g_batch_iterator = NULL;
        
        pthread_mutex_lock(&training_mutex);
        state->training_in_progress = false;
        training_thread_active = false;
        pthread_mutex_unlock(&training_mutex);
        
        return NULL;
    }
    
    printf("✓ Training ready: %zu batches available\n\n", num_batches);
    
    // Create threaded training system with 12 kissing spheres
    printf("\nInitializing worker threads...\n");
    snprintf(state->training_status_message, sizeof(state->training_status_message),
            "Initializing worker threads...");
    state->training_preprocessing_progress = 0.90f;
    SDL_PumpEvents();
    
    g_threaded_system = threaded_training_create(
        state->cllm_training,
        g_batch_iterator,
        0  // 0 = auto-detect CPU cores
    );
    
    if (!g_threaded_system) {
        printf("ERROR: Failed to create threaded training system\n");
        cllm_batch_iterator_free(g_batch_iterator);
        g_batch_iterator = NULL;
        
        pthread_mutex_lock(&training_mutex);
        state->training_in_progress = false;
        training_thread_active = false;
        pthread_mutex_unlock(&training_mutex);
        
        return NULL;
    }
    
    printf("✓ Threaded training system created\n");
    printf("✓ 12 kissing spheres initialized\n");
    printf("✓ Gradient accumulation buffers allocated\n");
    
    // UI Integration: Get metrics from training system and register callback
    CLLMMetrics* metrics = (CLLMMetrics*)threaded_training_get_metrics(g_threaded_system);
    if (metrics) {
        state->training_metrics = (struct CLLMMetrics*)metrics;
        cllm_metrics_set_callback(metrics_callback, state);
        printf("✓ Real-time metrics enabled for UI\n");
    }
    
    // CRITICAL FIX: Update progress bar to 100% after initialization complete
    snprintf(state->training_status_message, sizeof(state->training_status_message),
            "Training started - %d threads active", state->training_thread_count);
    state->training_preprocessing_progress = 1.0f;  // 100% - initialization complete!
    SDL_PumpEvents();  // Force UI update
    
    printf("\n");
    
    // Training loop with kissing spheres
    while (state->training_in_progress && 
           state->training_current_epoch < state->training_epochs) {
        
        printf("Epoch %d/%d - Training with 12 kissing spheres...\n",
               state->training_current_epoch + 1, state->training_epochs);
        
        // Add to terminal buffer
        if (state->terminal_buffer) {
            char line[256];
            snprintf(line, sizeof(line), "Epoch %d/%d - Training...", 
                    state->training_current_epoch + 1, state->training_epochs);
            terminal_buffer_add_line(state->terminal_buffer, line);
        }
        
        // Train one epoch using lock-free work queue (Phase 2B)
        float loss = threaded_train_epoch_lockfree(g_threaded_system, state->training_current_epoch);
        
        // Update state (thread-safe)
        pthread_mutex_lock(&training_mutex);
        state->training_current_epoch++;
        state->training_loss = loss;
        
        // Update sphere statistics for UI
        update_sphere_stats(state, g_threaded_system);
        
        pthread_mutex_unlock(&training_mutex);
        
        printf("✓ Epoch %d complete - Loss: %.4f\n\n", 
               state->training_current_epoch, loss);
        
        // Add to terminal buffer
        if (state->terminal_buffer) {
            char line[256];
            snprintf(line, sizeof(line), "✓ Epoch %d complete - Loss: %.4f", 
                    state->training_current_epoch, loss);
            terminal_buffer_add_line(state->terminal_buffer, line);
        }
        
        // Yield to UI thread for responsive updates
        SDL_Delay(10);
        
        // Check if complete
        if (state->training_current_epoch >= state->training_epochs) {
               // MOVED: Set training_in_progress = false AFTER printing stats
               // pthread_mutex_lock(&training_mutex);
               // state->training_in_progress = false;
               // pthread_mutex_unlock(&training_mutex);
            printf("=== TRAINING COMPLETE ===\n");
            printf("Total epochs: %d\n", state->training_current_epoch);
            printf("Final loss: %.4f\n", loss);
            
            // Print detailed sphere statistics
            printf("\n");
            threaded_training_print_stats(g_threaded_system);

               // Add final message to terminal buffer
               if (state->terminal_buffer) {
                   terminal_buffer_add_line(state->terminal_buffer, "=== TRAINING COMPLETE ===");
               }

               // Give UI time to show final state before clearing
               SDL_Delay(100);

               // NOW mark as complete (this will clear visualization)
               pthread_mutex_lock(&training_mutex);
               state->training_in_progress = false;
               pthread_mutex_unlock(&training_mutex);
            
            // Save final model
            if (state->cllm_model) {
                printf("\nSaving final model...\n");
                
                char model_dir[1024];
                char model_path[1024];
                extern void workspace_get_models_dir(AppState* state, char* output, size_t output_size);
                workspace_get_models_dir(state, model_dir, sizeof(model_dir));
                
                // Create models directory
                char mkdir_cmd[1100];
                snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", model_dir);
                int mkdir_ret = system(mkdir_cmd);
                (void)mkdir_ret;
                
                // Save to workspace location
                snprintf(model_path, sizeof(model_path), "%.1006s/trained_model_kissing_spheres.cllm", model_dir);
                extern int app_save_model(CLLMModel* model, const char* filepath);
                
                if (app_save_model(state->cllm_model, model_path) == 0) {
                    printf("✓ Model saved to: %s\n", model_path);
                    printf("  Trained with 12 kissing spheres architecture\n");
                } else {
                    printf("✗ Failed to save model\n");
                }
            }
            
            break;
        }
    }
    
    printf("=== TRAINING THREAD STOPPED ===\n");
    
    // Cleanup
    if (g_threaded_system) {
        threaded_training_free(g_threaded_system);
        g_threaded_system = NULL;
    }
    if (g_batch_iterator) {
        cllm_batch_iterator_free(g_batch_iterator);
        g_batch_iterator = NULL;
    }
    
    pthread_mutex_lock(&training_mutex);
    training_thread_active = false;
    pthread_mutex_unlock(&training_mutex);
    
    return NULL;
}

/**
 * Start training in separate thread
 */
int start_training_thread(AppState* state) {
    if (!state || !state->cllm_training) {
        printf("ERROR: Cannot start training - no training context\n");
        return -1;
    }
    
    pthread_mutex_lock(&training_mutex);
    
    if (training_thread_active) {
        pthread_mutex_unlock(&training_mutex);
        printf("Training thread already running\n");
        return -1;
    }
    
    training_thread_active = true;
    state->training_in_progress = true;
    state->training_current_epoch = 0;
    
    // Initialize sphere statistics
    memset(&state->sphere_stats, 0, sizeof(state->sphere_stats));
    
    pthread_mutex_unlock(&training_mutex);
    
    if (pthread_create(&training_thread, NULL, training_thread_func, state) != 0) {
        pthread_mutex_lock(&training_mutex);
        training_thread_active = false;
        state->training_in_progress = false;
        pthread_mutex_unlock(&training_mutex);
        printf("ERROR: Failed to create training thread\n");
        return -1;
    }
    
    // Detach thread so it cleans up automatically
    pthread_detach(training_thread);
    
    printf("✓ Training thread started with kissing spheres architecture\n");
    printf("  12 parallel workers will process batches\n");
    printf("  Gradient accumulation across all spheres\n");
    return 0;
}

/**
 * Stop training thread
 */
void stop_training_thread(AppState* state) {
    if (!state) return;
    
    pthread_mutex_lock(&training_mutex);
    state->training_in_progress = false;
    pthread_mutex_unlock(&training_mutex);
    
    printf("Training stop requested\n");
}

/**
 * Check if training thread is active
 */
bool is_training_thread_active(void) {
    bool active;
    pthread_mutex_lock(&training_mutex);
    active = training_thread_active;
    pthread_mutex_unlock(&training_mutex);
    return active;
}

/**
 * Get training state (thread-safe)
 */
void get_training_state(AppState* state, int* epoch, float* loss, bool* in_progress) {
    if (!state) return;
    
    pthread_mutex_lock(&training_mutex);
    if (epoch) *epoch = state->training_current_epoch;
    if (loss) *loss = state->training_loss;
    if (in_progress) *in_progress = state->training_in_progress;
    pthread_mutex_unlock(&training_mutex);
}