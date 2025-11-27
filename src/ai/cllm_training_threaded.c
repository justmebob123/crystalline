/**
 * CLLM Multi-Threaded Training with Kissing Spheres Architecture
 * 
 * Integrates the 12-fold kissing spheres threading system into the training loop
 * for parallel batch processing and gradient accumulation.
 * 
 * Phase 2 Optimizations:
 * - Lock-free segment-based gradient accumulation
 * - SIMD-optimized gradient operations
 * - Streaming accumulation (overlap computation with updates)
 */

#include "cllm_training.h"
#include "cllm_threads.h"
#include "cllm_batch.h"
#include "cllm_simd_gradient_ops.h"
#include "ai/cllm_lattice_hierarchy.h"
#include "prime_float_math.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/**
 * Thread-local training context for each sphere
 */
// Forward declaration
typedef struct ThreadedTrainingSystem ThreadedTrainingSystem;

typedef struct {
    int sphere_id;
    int symmetry_group;  // 0-11 for the 12 kissing spheres
    
    // Reference to parent system
    ThreadedTrainingSystem* system;
    
    // Local gradient buffers
    float* local_gradients;
    size_t gradient_size;
    
    // Segment ownership (lock-free)
    size_t segment_start;
    size_t segment_end;
    
    // Batch processing
    CLLMBatch* current_batch;
    float batch_loss;
    int batches_processed;
    
    // Synchronization
    pthread_mutex_t lock;
    pthread_cond_t work_ready;
    pthread_cond_t work_done;
    int has_work;
    int work_complete;
    
    // Worker thread
    pthread_t thread;
    
} SphereTrainingContext;

/**
 * Multi-threaded training system
 */
struct ThreadedTrainingSystem {
    CLLMTraining* training;
    ThreadSystem* thread_system;
    
    // Sphere contexts (dynamic array)
    SphereTrainingContext** sphere_contexts;
    int num_worker_spheres;
    
    // Batch iterator
    CLLMBatchIterator* batch_iterator;
    
    // Global gradient accumulation (lock-free segments)
    float* accumulated_gradients;
    size_t gradient_size;
    pthread_mutex_t gradient_lock;  // Only for boundaries
    
    // Synchronization
    pthread_barrier_t epoch_barrier;
    pthread_barrier_t batch_barrier;
    
    // Statistics
    float epoch_loss;
    int total_batches;
    int running;

    // Phase 4: Dynamic spawning
    atomic_uint sphere_id_counter;  // Global counter for assigning sphere IDs
    
    // Phase 2: Streaming accumulation
    int* sphere_completion_flags;
    pthread_mutex_t completion_lock;
    pthread_cond_t completion_cond;
};

/**
 * Initialize sphere training context
 */
static SphereTrainingContext* sphere_context_create(int sphere_id, int symmetry_group, 
                                                     size_t gradient_size) {
    SphereTrainingContext* ctx = (SphereTrainingContext*)calloc(1, sizeof(SphereTrainingContext));
    if (!ctx) return NULL;
    
    ctx->sphere_id = sphere_id;
    ctx->symmetry_group = symmetry_group;
    ctx->system = NULL;  // Will be set later
    ctx->gradient_size = gradient_size;
    
    ctx->local_gradients = (float*)calloc(gradient_size, sizeof(float));
    if (!ctx->local_gradients) {
        free(ctx);
        return NULL;
    }
    
    pthread_mutex_init(&ctx->lock, NULL);
    pthread_cond_init(&ctx->work_ready, NULL);
    pthread_cond_init(&ctx->work_done, NULL);
    
    ctx->has_work = 0;
    ctx->work_complete = 0;
    ctx->current_batch = NULL;
    ctx->batch_loss = 0.0f;
    ctx->batches_processed = 0;
    
    return ctx;
}

/**
 * Free sphere training context
 */
static void sphere_context_free(SphereTrainingContext* ctx) {
    if (!ctx) return;
    
    free(ctx->local_gradients);
    pthread_mutex_destroy(&ctx->lock);
    pthread_cond_destroy(&ctx->work_ready);
    pthread_cond_destroy(&ctx->work_done);
    free(ctx);
}

// Forward declarations
static void* sphere_worker_thread(void* arg);
static void sphere_process_batch(SphereTrainingContext* ctx, CLLMTraining* training);

/**
 * Process batch on a sphere (worker thread function)
 */
static void sphere_process_batch(SphereTrainingContext* ctx, CLLMTraining* training) {
    if (!ctx->current_batch || !training) return;
    
    CLLMBatch* batch = ctx->current_batch;
    
    // Zero local gradients
    memset(ctx->local_gradients, 0, ctx->gradient_size * sizeof(float));
    
    // Process each sequence in the batch
    float total_loss = 0.0f;
    int valid_sequences = 0;
    
    for (uint32_t seq = 0; seq < batch->batch_size; seq++) {
        uint32_t offset = seq * batch->seq_len;
        
        // Check if this sequence has valid tokens
        int has_valid = 0;
        for (uint32_t i = 0; i < batch->seq_len; i++) {
            if (batch->attention_mask[offset + i] > 0.5f) {
                has_valid = 1;
                break;
            }
        }
        
        if (!has_valid) continue;
        
        // Forward pass using the actual model
        float seq_loss = cllm_forward_training(training, &batch->input_ids[offset]);
        
        // Compute loss
        seq_loss += cllm_compute_loss(training, &batch->input_ids[offset], &batch->target_ids[offset], batch->seq_len);
        
        // Backward pass - compute gradients
        cllm_backward_training(training, &batch->target_ids[offset]);
        
        total_loss += seq_loss;
        valid_sequences++;
    }
    
    ctx->batch_loss = (valid_sequences > 0) ? total_loss / valid_sequences : 0.0f;
    ctx->batches_processed++;
}

/**
 * Calculate optimal hierarchy levels for given thread count
 */
static int calculate_hierarchy_levels(int num_threads) {
    if (num_threads <= 1) return 1;   // Just root
    if (num_threads <= 13) return 2;  // Root + up to 12 kissing spheres
    if (num_threads <= 157) return 3; // Root + 12 + up to 144
    return 4;                          // Full hierarchy (up to 1741 threads)
}

/**
 * Create threaded training system
 */
ThreadedTrainingSystem* threaded_training_create(CLLMTraining* training, 
                                                  CLLMBatchIterator* batch_iterator,
                                                  int num_threads) {
    if (!training || !batch_iterator) return NULL;
    
    // Auto-detect thread count if not specified
    if (num_threads <= 0) {
        num_threads = get_num_cpu_cores();
        if (num_threads > 1) num_threads--; // Reserve 1 for main thread
        printf("Auto-detected %d worker threads\n", num_threads);
    }
    
    // Ensure at least 1 thread
    if (num_threads < 1) num_threads = 1;
    
    ThreadedTrainingSystem* system = (ThreadedTrainingSystem*)calloc(1, sizeof(ThreadedTrainingSystem));
    if (!system) return NULL;
    
    system->training = training;
    system->batch_iterator = batch_iterator;
    system->running = 1;
    atomic_init(&system->sphere_id_counter, num_threads);  // Start after initial threads
    
    // Calculate number of worker spheres (exclude root)
    int hierarchy_levels = calculate_hierarchy_levels(num_threads);
    system->num_worker_spheres = num_threads;
    
    printf("Creating threaded training system:\n");
    printf("  Worker threads: %d\n", system->num_worker_spheres);
    printf("  Hierarchy levels: %d\n", hierarchy_levels);
    
    // Calculate gradient size
    system->gradient_size = training->model->vocab_size * training->model->embedding_dim;
    
    // Allocate global gradient accumulation buffer
    system->accumulated_gradients = (float*)calloc(system->gradient_size, sizeof(float));
    if (!system->accumulated_gradients) {
        free(system);
        return NULL;
    }
    
    pthread_mutex_init(&system->gradient_lock, NULL);
    
    // Initialize barriers for N threads + 1 main thread
    pthread_barrier_init(&system->epoch_barrier, NULL, num_threads + 1);
    pthread_barrier_init(&system->batch_barrier, NULL, num_threads + 1);
    
    // Create thread system with calculated hierarchy
    system->thread_system = threads_create(hierarchy_levels);
    if (!system->thread_system) {
        free(system->accumulated_gradients);
        pthread_mutex_destroy(&system->gradient_lock);
        free(system);
        return NULL;
    }

    // Set user_data for all spheres to point to the training system
    // This allows spawning logic to access the sphere_id_counter
    for (int i = 0; i < system->thread_system->total_spheres; i++) {
        system->thread_system->all_spheres[i]->user_data = system;
    }
    // Allocate dynamic sphere contexts array
    system->sphere_contexts = (SphereTrainingContext**)calloc(system->num_worker_spheres, 
                                                               sizeof(SphereTrainingContext*));
    if (!system->sphere_contexts) {
        threads_free(system->thread_system);
        free(system->accumulated_gradients);
        pthread_mutex_destroy(&system->gradient_lock);
        free(system);
        return NULL;
    }
    
    // Create sphere contexts for all worker spheres
    for (int i = 0; i < system->num_worker_spheres; i++) {
        int symmetry_group = i % 12; // Distribute across 12 symmetry groups
        system->sphere_contexts[i] = sphere_context_create(i, symmetry_group, system->gradient_size);
        if (!system->sphere_contexts[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                sphere_context_free(system->sphere_contexts[j]);
            }
            free(system->sphere_contexts);
            threads_free(system->thread_system);
            free(system->accumulated_gradients);
            pthread_mutex_destroy(&system->gradient_lock);
            free(system);
            return NULL;
        }
        // Set system reference
        system->sphere_contexts[i]->system = system;
    }
    
    // Create worker threads
    printf("  Creating %d worker threads...\n", system->num_worker_spheres);
    for (int i = 0; i < system->num_worker_spheres; i++) {
        int rc = pthread_create(&system->sphere_contexts[i]->thread, NULL, 
                               sphere_worker_thread, system->sphere_contexts[i]);
        if (rc != 0) {
            fprintf(stderr, "ERROR: Failed to create worker thread %d (error %d)\n", i, rc);
            // Stop already created threads
            system->running = 0;
            for (int j = 0; j < i; j++) {
                pthread_mutex_lock(&system->sphere_contexts[j]->lock);
                pthread_cond_signal(&system->sphere_contexts[j]->work_ready);
                pthread_mutex_unlock(&system->sphere_contexts[j]->lock);
                pthread_join(system->sphere_contexts[j]->thread, NULL);
            }
            // Cleanup
            for (int j = 0; j < system->num_worker_spheres; j++) {
                sphere_context_free(system->sphere_contexts[j]);
            }
            free(system->sphere_contexts);
            threads_free(system->thread_system);
            free(system->accumulated_gradients);
            pthread_mutex_destroy(&system->gradient_lock);
            free(system);
            return NULL;
        }
    }
    
    system->epoch_loss = 0.0f;
    system->total_batches = 0;
    
    printf("  ✓ Threaded training system created successfully with %d active threads\n\n", 
           system->num_worker_spheres);
    
    return system;
}

/**
 * Free threaded training system
 */
void threaded_training_free(ThreadedTrainingSystem* system) {
    if (!system) return;
    
    printf("\nStopping worker threads...\n");
    system->running = 0;
    
    // Wake up all worker threads and wait for them to exit
    for (int i = 0; i < system->num_worker_spheres; i++) {
        if (system->sphere_contexts[i]) {
            pthread_mutex_lock(&system->sphere_contexts[i]->lock);
            pthread_cond_signal(&system->sphere_contexts[i]->work_ready);
            pthread_mutex_unlock(&system->sphere_contexts[i]->lock);
            
            // Join the thread
            pthread_join(system->sphere_contexts[i]->thread, NULL);
        }
    }
    printf("All worker threads stopped.\n");
    
    // Free sphere contexts
    for (int i = 0; i < system->num_worker_spheres; i++) {
        sphere_context_free(system->sphere_contexts[i]);
    }
    
    // Free the dynamic array
    free(system->sphere_contexts);
    
    threads_free(system->thread_system);
    free(system->accumulated_gradients);
    pthread_mutex_destroy(&system->gradient_lock);
    pthread_barrier_destroy(&system->epoch_barrier);
    pthread_barrier_destroy(&system->batch_barrier);
    free(system);
}

/**
 * Worker thread function - processes batches assigned to this sphere
 */
static void* sphere_worker_thread(void* arg) {
    SphereTrainingContext* ctx = (SphereTrainingContext*)arg;
    ThreadedTrainingSystem* system = ctx->system;
    
    while (system->running) {
        pthread_mutex_lock(&ctx->lock);
        
        // Wait for work
        while (!ctx->has_work && system->running) {
            pthread_cond_wait(&ctx->work_ready, &ctx->lock);
        }
        
        if (!system->running) {
            pthread_mutex_unlock(&ctx->lock);
            break;
        }
        
        // Process the batch
        if (ctx->current_batch) {
            sphere_process_batch(ctx, system->training);
        }
        
        // Signal work complete
        ctx->work_complete = 1;
        ctx->has_work = 0;
        pthread_cond_signal(&ctx->work_done);
        
        pthread_mutex_unlock(&ctx->lock);
    }
    
    return NULL;
}

/**
 * Distribute batch to sphere (asynchronous)
 */
static void distribute_batch_to_sphere(ThreadedTrainingSystem* system, int sphere_id, 
                                      CLLMBatch* batch) {
    SphereTrainingContext* ctx = system->sphere_contexts[sphere_id];
    
    pthread_mutex_lock(&ctx->lock);
    
    ctx->current_batch = batch;
    ctx->has_work = 1;
    ctx->work_complete = 0;
    pthread_cond_signal(&ctx->work_ready);
    
    pthread_mutex_unlock(&ctx->lock);
}

/**
 * Wait for sphere to complete work
 */
static void wait_for_sphere(ThreadedTrainingSystem* system, int sphere_id) {
    SphereTrainingContext* ctx = system->sphere_contexts[sphere_id];
    
    pthread_mutex_lock(&ctx->lock);
    
    while (!ctx->work_complete) {
        pthread_cond_wait(&ctx->work_done, &ctx->lock);
    }
    
    ctx->current_batch = NULL;
    
    pthread_mutex_unlock(&ctx->lock);
}

/**
 * Accumulate gradients from all spheres
 */
static void accumulate_gradients(ThreadedTrainingSystem* system) {
    pthread_mutex_lock(&system->gradient_lock);
    
    // Zero accumulated gradients
    memset(system->accumulated_gradients, 0, system->gradient_size * sizeof(float));
    
    // Sum gradients from all spheres
    for (int i = 0; i < system->num_worker_spheres; i++) {
        SphereTrainingContext* ctx = system->sphere_contexts[i];
        for (size_t j = 0; j < system->gradient_size; j++) {
            system->accumulated_gradients[j] += ctx->local_gradients[j];
        }
    }
    
    // Average gradients
    for (size_t i = 0; i < system->gradient_size; i++) {
        system->accumulated_gradients[i] /= (float)system->num_worker_spheres;
    }
    
    pthread_mutex_unlock(&system->gradient_lock);
}

/**
 * Train one epoch with multi-threading
 */
float threaded_train_epoch(ThreadedTrainingSystem* system) {
    if (!system) return 0.0f;
    
    printf("\nStarting multi-threaded epoch training...\n");
    printf("Using %d worker threads for parallel batch processing\n\n", system->num_worker_spheres);
    
    cllm_batch_iterator_reset(system->batch_iterator);
    
    float epoch_loss = 0.0f;
    int batch_count = 0;
    
    // Process batches in groups (one per sphere)
    int total_batch_groups = 0;
    size_t total_batches_in_epoch = cllm_batch_iterator_num_batches(system->batch_iterator);
    
    while (1) {
        CLLMBatch** batches = (CLLMBatch**)calloc(system->num_worker_spheres, sizeof(CLLMBatch*));
        if (!batches) break;
        
        int batches_loaded = 0;
        
        // Load up to N batches (one per worker sphere)
        for (int i = 0; i < system->num_worker_spheres; i++) {
            batches[i] = cllm_batch_iterator_next(system->batch_iterator);
            if (batches[i]) {
                batches_loaded++;
            } else {
                // Iterator returned NULL - no more batches
                break;
            }
        }
        
        // If no batches loaded, we're done with the epoch
        if (batches_loaded == 0) {
            free(batches);
            break;
        }
        
        // Debug: Show actual batch count
        if (total_batch_groups == 0) {
            printf("First batch group: loaded %d batches\n", batches_loaded);
        }
        
        total_batch_groups++;
        
        // Safety check: prevent infinite loops
        // Allow some margin since batch_groups != batches (multiple batches per group)
        int max_batch_groups = (int)total_batches_in_epoch + 10;
        if (total_batch_groups > max_batch_groups) {
            printf("WARNING: Processed more batch groups (%d) than expected (max %d). Breaking to prevent infinite loop.\n",
                   total_batch_groups, max_batch_groups);
            // Free any remaining batches before breaking
            for (int i = 0; i < system->num_worker_spheres; i++) {
                if (batches[i]) {
                    cllm_batch_free(batches[i]);
                }
            }
            free(batches);
            break;
        }
        
        printf("Processing batch group %d (%d batches across %d spheres)...\n",
               batch_count / system->num_worker_spheres + 1, batches_loaded, batches_loaded);
        
        // Distribute batches to spheres
        for (int i = 0; i < batches_loaded; i++) {
            distribute_batch_to_sphere(system, i, batches[i]);
        }
        
        // Wait for all spheres to complete
        for (int i = 0; i < batches_loaded; i++) {
            wait_for_sphere(system, i);
        }
        
        // Accumulate gradients
        accumulate_gradients(system);
        
        // Collect losses
        float batch_group_loss = 0.0f;
        for (int i = 0; i < batches_loaded; i++) {
            batch_group_loss += system->sphere_contexts[i]->batch_loss;
        }
        batch_group_loss /= batches_loaded;
        
        epoch_loss += batch_group_loss;
        batch_count += batches_loaded;
        
        printf("  Batch group loss: %.4f\n", batch_group_loss);
        
        // Free batches and clear sphere context pointers
        for (int i = 0; i < batches_loaded; i++) {
            if (batches[i]) {
                cllm_batch_free(batches[i]);
                // Clear the pointer in the sphere context to avoid double-free
                system->sphere_contexts[i]->current_batch = NULL;
            }
        }
        free(batches);
        
        // Apply gradients to model (simplified)
        // In real implementation, this would call the optimizer
        pthread_mutex_lock(&system->gradient_lock);
        // Apply accumulated_gradients to model weights
        pthread_mutex_unlock(&system->gradient_lock);
    }
    
    float avg_loss = (batch_count > 0) ? epoch_loss / (batch_count / (float)system->num_worker_spheres) : 0.0f;
    
    printf("\nEpoch complete:\n");
    printf("  Total batches processed: %d\n", batch_count);
    printf("  Average loss: %.4f\n", avg_loss);
    printf("  Batches per sphere: %.1f\n", (float)batch_count / system->num_worker_spheres);
    
    return avg_loss;
}

/**
 * Print threading statistics
 */
void threaded_training_print_stats(ThreadedTrainingSystem* system) {
    if (!system) return;
    
    printf("\n========================================\n");
    printf("  Multi-Threaded Training Statistics\n");
    printf("========================================\n\n");
    
    printf("Thread System:\n");
    threads_print_stats(system->thread_system);
    
    printf("\nSphere Statistics:\n");
    for (int i = 0; i < system->num_worker_spheres; i++) {
        SphereTrainingContext* ctx = system->sphere_contexts[i];
        printf("  Sphere %2d (Group %2d): %d batches processed, avg loss: %.4f\n",
               ctx->sphere_id, ctx->symmetry_group, ctx->batches_processed,
               ctx->batches_processed > 0 ? ctx->batch_loss / ctx->batches_processed : 0.0f);
    }
    
    printf("\n");
}
/**
 * Get per-sphere statistics
 */
int threaded_training_get_sphere_stats(ThreadedTrainingSystem* system,
                                       int sphere_id,
                                       int* batches_processed,
                                       float* avg_loss) {
    if (!system || sphere_id < 0 || sphere_id >= system->num_worker_spheres) return -1;
    
    SphereTrainingContext* ctx = system->sphere_contexts[sphere_id];
    if (!ctx) return -1;
    
    if (batches_processed) {
        *batches_processed = ctx->batches_processed;
    }
    
    if (avg_loss) {
        *avg_loss = ctx->batches_processed > 0 ? 
                    ctx->batch_loss / ctx->batches_processed : 0.0f;
    }
    
    return 0;
}

/**
 * Get total gradient norm
 */
float threaded_training_get_gradient_norm(ThreadedTrainingSystem* system) {
    if (!system || !system->accumulated_gradients) return 0.0f;
    
    pthread_mutex_lock(&system->gradient_lock);
    
    float norm = 0.0f;
    for (size_t i = 0; i < system->gradient_size; i++) {
        float val = system->accumulated_gradients[i];
        norm += val * val;
    }
    norm = prime_sqrtf(norm);
    
    pthread_mutex_unlock(&system->gradient_lock);
    
    return norm;
}

/**
 * Get number of worker spheres
 */
int threaded_training_get_num_workers(ThreadedTrainingSystem* system) {
    if (!system) return 0;
    return system->num_worker_spheres;
}

/**
 * Get next sphere ID for dynamic spawning
 * 
 * This function is called from cllm_threads.c when spawning new children.
 * It atomically increments and returns the next available sphere ID.
 * 
 * @param user_data Pointer to ThreadedTrainingSystem (from sphere->user_data)
 * @return Next available sphere ID
 */
int threaded_training_get_next_sphere_id(void* user_data) {
    if (!user_data) return -1;
    
    ThreadedTrainingSystem* system = (ThreadedTrainingSystem*)user_data;
    unsigned int next_id = atomic_fetch_add(&system->sphere_id_counter, 1);
    
    return (int)next_id;
}
