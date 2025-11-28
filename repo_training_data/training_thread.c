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
 * Update sphere statistics in AppState for UI display
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
    
    // Create batch iterator from training data
    printf("\nCreating batch iterator...\n");
    g_batch_iterator = cllm_batch_iterator_create(
        state->cllm_training->tokens,
        state->cllm_training->num_tokens,
        state->cllm_training->config.batch_size,
        state->cllm_training->config.sequence_length,
        0,  // no shuffle for now
        1   // drop last incomplete batch
    );
    
    if (!g_batch_iterator) {
        printf("ERROR: Failed to create batch iterator\n");
        
        pthread_mutex_lock(&training_mutex);
        state->training_in_progress = false;
        training_thread_active = false;
        pthread_mutex_unlock(&training_mutex);
        
        return NULL;
    }
    
    size_t num_batches = cllm_batch_iterator_num_batches(g_batch_iterator);
    printf("✓ Batch iterator created: %zu batches\n", num_batches);
    
    // Create threaded training system with 12 kissing spheres
    printf("\nInitializing 12 kissing spheres...\n");
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
    printf("✓ Gradient accumulation buffers allocated\n\n");
    
    // Training loop with kissing spheres
    while (state->training_in_progress && 
           state->training_current_epoch < state->training_epochs) {
        
        printf("Epoch %d/%d - Training with 12 kissing spheres...\n",
               state->training_current_epoch + 1, state->training_epochs);
        
        // Train one epoch using 12-way parallelization
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
        
        // Yield to UI thread for responsive updates
        SDL_Delay(10);
        
        // Check if complete
        if (state->training_current_epoch >= state->training_epochs) {
            pthread_mutex_lock(&training_mutex);
            state->training_in_progress = false;
            pthread_mutex_unlock(&training_mutex);
            
            printf("=== TRAINING COMPLETE ===\n");
            printf("Total epochs: %d\n", state->training_current_epoch);
            printf("Final loss: %.4f\n", loss);
            
            // Print detailed sphere statistics
            printf("\n");
            threaded_training_print_stats(g_threaded_system);
            
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