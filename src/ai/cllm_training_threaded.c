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
#include "ai/cllm_shared_memory.h"
#include "prime_float_math.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>

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
    
    // Shared memory access (kissing spheres architecture)
    SharedMemoryRegion* shared_gradients;  // Reference to shared gradient buffer
    size_t gradient_segment_start;         // This sphere's lock-free segment start
    size_t gradient_segment_end;           // This sphere's lock-free segment end
    
    // Legacy fields (for compatibility during transition)
    float* local_gradients;  // TODO: Remove after shared memory integration
    size_t gradient_size;
    
    // Batch processing
    CLLMBatch* current_batch;
    float batch_loss;
    int batches_processed;
    
    // Synchronization
    pthread_mutex_t lock;
    pthread_cond_t work_ready;
    pthread_cond_t work_done;
    volatile int has_work;        // MUST be volatile - checked without lock in some paths!
    volatile int work_complete;   // MUST be volatile - checked without lock in some paths!
    
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
    
    // 12-fold symmetry structure
    int num_symmetry_positions;  // Always 12
    int num_active_workers;      // Can be < 12 (based on CPU count)
    
    // Control thread (Node Zero)
    pthread_t control_thread;
    volatile int control_running;
    int has_control_thread;      // 1 if using separate control thread
    
    // Batch iterator
    CLLMBatchIterator* batch_iterator;
    
    // Shared memory regions (kissing spheres architecture)
    SharedMemoryRegion* shared_gradients;      // Single shared gradient buffer
    SharedMemoryRegion* shared_model_weights;  // Shared model weights (read-only)
    size_t gradient_size;
    
    // Gradient accumulation (temporary until shared memory fully integrated)
    float* accumulated_gradients;              // Accumulated gradients from all spheres
    pthread_mutex_t gradient_lock;             // Lock for gradient accumulation
    pthread_mutex_t model_lock;                // Lock for model state during forward/backward (TEMPORARY)
    
    // Synchronization (MASTER PLAN - use barriers!)
    pthread_barrier_t epoch_barrier;
    pthread_barrier_t batch_barrier;
    
    // Statistics
    float epoch_loss;
    int total_batches;
    atomic_int running;  // MUST be atomic - accessed by multiple threads without lock!

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
                                                     size_t gradient_size,
                                                     SharedMemoryRegion* shared_gradients,
                                                     int num_spheres) {
    SphereTrainingContext* ctx = (SphereTrainingContext*)calloc(1, sizeof(SphereTrainingContext));
    if (!ctx) return NULL;
    
    ctx->sphere_id = sphere_id;
    ctx->symmetry_group = symmetry_group;
    ctx->system = NULL;  // Will be set later
    ctx->gradient_size = gradient_size;
    
    // Use shared memory instead of local allocation
    ctx->shared_gradients = shared_gradients;
    
    // Assign lock-free segment for this sphere
    size_t segment_size = gradient_size / num_spheres;
    ctx->gradient_segment_start = sphere_id * segment_size;
    ctx->gradient_segment_end = (sphere_id + 1) * segment_size;
    
    // Keep local_gradients for now (compatibility during transition)
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
static void* control_thread_func(void* arg);  // Node Zero - NEVER processes batches
static void sphere_process_batch(SphereTrainingContext* ctx, CLLMTraining* training);
static void accumulate_gradients(ThreadedTrainingSystem* system);

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
        
        // CRITICAL: Lock model state during forward/backward to prevent race conditions
        pthread_mutex_lock(&ctx->system->model_lock);
        
        // Forward pass using the actual model
        float seq_loss = cllm_forward_training(training, &batch->input_ids[offset]);
        
        // Compute loss
        seq_loss += cllm_compute_loss(training, &batch->input_ids[offset], &batch->target_ids[offset], batch->seq_len);
        
        // Backward pass - compute gradients to local buffer
        cllm_backward_training(training, &batch->target_ids[offset], ctx->local_gradients);
        
        pthread_mutex_unlock(&ctx->system->model_lock);
        
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
    printf("DEBUG: [ENTER] threaded_training_create, num_threads=%d\n", num_threads); fflush(stdout);
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
    atomic_init(&system->running, 1);  // MUST use atomic_init for atomic_int!
    atomic_init(&system->sphere_id_counter, num_threads);  // Start after initial threads
    
    // MASTER PLAN: 12-fold symmetry structure
    system->num_symmetry_positions = 12;  // Always 12 positions
    system->num_active_workers = num_threads;  // Can be < 12
    
    // Calculate number of worker spheres
    int hierarchy_levels = calculate_hierarchy_levels(num_threads);
    system->num_worker_spheres = num_threads;
    
    printf("Creating 12-fold symmetric threading system (MASTER PLAN):\n");
    printf("  Symmetry positions: %d (12-fold structure)\n", system->num_symmetry_positions);
    printf("  Active workers: %d (rotating through positions)\n", system->num_active_workers);
    printf("  Control thread: Node Zero (NEVER processes batches)\n");
    printf("  Hierarchy levels: %d\n", hierarchy_levels);
    
    // Calculate gradient size
    system->gradient_size = training->model->vocab_size * training->model->embedding_dim;
    
    // Create shared gradient buffer (kissing spheres architecture)
    system->shared_gradients = shared_memory_create(
        system->gradient_size * sizeof(float),
        SHARED_LOCKED_WRITE  // Multiple writers allowed
    );
    if (!system->shared_gradients) {
        fprintf(stderr, "Failed to create shared gradient buffer\n");
        free(system);
        return NULL;
    }
    
    printf("  ✓ Created shared gradient buffer: %.2f MB\n", 
           (system->gradient_size * sizeof(float)) / (1024.0f * 1024.0f));
    
    // Allocate accumulated gradients buffer (temporary until shared memory fully integrated)
    system->accumulated_gradients = (float*)calloc(system->gradient_size, sizeof(float));
    if (!system->accumulated_gradients) {
        fprintf(stderr, "Failed to allocate accumulated gradients buffer\n");
        shared_memory_free(system->shared_gradients);
        free(system);
        return NULL;
    }
    
    // Initialize gradient lock
    pthread_mutex_init(&system->gradient_lock, NULL);
    pthread_mutex_init(&system->model_lock, NULL);
    
    // Initialize barriers for N worker threads + 1 control thread + 1 main thread
    // Total participants: num_threads (workers) + 1 (control) + 1 (main)
    pthread_barrier_init(&system->epoch_barrier, NULL, num_threads + 2);
    pthread_barrier_init(&system->batch_barrier, NULL, num_threads + 2);
    
    // Create thread system with calculated hierarchy
    system->thread_system = threads_create(hierarchy_levels);
    printf("DEBUG: [STEP 2] threads_create returned: %p\n", (void*)system->thread_system); fflush(stdout);
    if (!system->thread_system) {
        shared_memory_free(system->shared_gradients);
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
        shared_memory_free(system->shared_gradients);
        free(system);
        return NULL;
    }
    
    // Create sphere contexts for all worker spheres
    for (int i = 0; i < system->num_worker_spheres; i++) {
        int symmetry_group = i % 12; // Distribute across 12 symmetry groups
        system->sphere_contexts[i] = sphere_context_create(i, symmetry_group, system->gradient_size,
                                                           system->shared_gradients, system->num_worker_spheres);
        if (!system->sphere_contexts[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                sphere_context_free(system->sphere_contexts[j]);
            }
            free(system->sphere_contexts);
            threads_free(system->thread_system);
            shared_memory_free(system->shared_gradients);
            free(system);
            return NULL;
        }
        // Set system reference
        system->sphere_contexts[i]->system = system;
    }
    
    // MASTER PLAN: Create control thread (Node Zero) first
    printf("DEBUG: [STEP 3] Creating Node Zero (control thread)\n"); fflush(stdout);
    system->has_control_thread = 1;
    system->control_running = 1;
    
    int rc = pthread_create(&system->control_thread, NULL, 
                           control_thread_func, system);
    if (rc != 0) {
        fprintf(stderr, "ERROR: Failed to create control thread (error %d)\n", rc);
        // Cleanup
        for (int j = 0; j < system->num_worker_spheres; j++) {
            sphere_context_free(system->sphere_contexts[j]);
        }
        free(system->sphere_contexts);
        threads_free(system->thread_system);
        shared_memory_free(system->shared_gradients);
        free(system);
        return NULL;
    }
    printf("  ✓ Node Zero created (control thread NEVER processes batches)\n");
    
    // Create worker threads
    printf("DEBUG: [STEP 4] Creating worker threads\n"); fflush(stdout);
    printf("  Creating %d worker threads...\n", system->num_worker_spheres);
    for (int i = 0; i < system->num_worker_spheres; i++) {
        printf("DEBUG: [STEP 5] Creating worker %d/%d\n", i+1, system->num_worker_spheres); fflush(stdout);
        rc = pthread_create(&system->sphere_contexts[i]->thread, NULL, 
                           sphere_worker_thread, system->sphere_contexts[i]);
        if (rc != 0) {
            fprintf(stderr, "ERROR: Failed to create worker thread %d (error %d)\n", i, rc);
            // Stop control thread
            atomic_store(&system->running, 0);
            system->control_running = 0;
            pthread_join(system->control_thread, NULL);
            // Stop already created workers
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
            shared_memory_free(system->shared_gradients);
            free(system);
            return NULL;
        }
    }
    
    system->epoch_loss = 0.0f;
    system->total_batches = 0;
    
    printf("  ✓ Threaded training system created successfully\n");
    printf("    - 1 control thread (Node Zero)\n");
    printf("    - %d worker threads\n", system->num_worker_spheres);
    printf("    - 12-fold symmetry structure\n\n");
    
    // Give threads time to initialize before returning
    usleep(10000);  // 10ms - allow worker threads to start and enter wait state
    
    return system;
}

/**
 * Free threaded training system
 */
void threaded_training_free(ThreadedTrainingSystem* system) {
    if (!system) return;
    
    printf("\nStopping threads...\n");
    atomic_store(&system->running, 0);
    
    // Stop control thread first (Node Zero)
    if (system->has_control_thread) {
        printf("  Stopping Node Zero (control thread)...\n");
        system->control_running = 0;
        pthread_join(system->control_thread, NULL);
        printf("  ✓ Node Zero stopped\n");
    }
    
    // Wake up all worker threads and wait for them to exit
    printf("  Stopping worker threads...\n");
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
    
    // Free shared memory regions
    shared_memory_free(system->shared_gradients);
    if (system->shared_model_weights) {
        shared_memory_free(system->shared_model_weights);
    }
    
    // Free accumulated gradients buffer
    free(system->accumulated_gradients);
    
    // Destroy gradient lock
    pthread_mutex_destroy(&system->gradient_lock);
    pthread_mutex_destroy(&system->model_lock);
    
    pthread_barrier_destroy(&system->epoch_barrier);
    pthread_barrier_destroy(&system->batch_barrier);
    free(system);
}

/**
 * Control thread function (Node Zero) - MASTER PLAN IMPLEMENTATION
 * 
 * CRITICAL: This thread NEVER processes batches!
 * 
 * Responsibilities:
 * 1. Distribute batches to workers
 * 2. Wait at barriers for workers to complete
 * 3. Accumulate gradients (safe - workers waiting at barrier)
 * 4. Apply optimizer step
 * 5. Coordinate next batch
 */
/**
 * Node Zero (Control Thread) - PHASE 3: Barrier Synchronization
 * 
 * CRITICAL: This thread NEVER processes batches (master plan requirement)
 * 
 * PHASE 3 IMPLEMENTATION:
 * - Participates in barrier synchronization
 * - Coordinates with main thread and workers
 * - Synchronizes at Point A (batch distribution)
 * - Synchronizes at Point B (batch completion)
 */
static void* control_thread_func(void* arg) {
    ThreadedTrainingSystem* system = (ThreadedTrainingSystem*)arg;
    
    printf("[Node Zero] Control thread started - NEVER processes batches\n");
    printf("[Node Zero] Using barrier synchronization\n");
    
    while (atomic_load(&system->running)) {
        // POINT A: Wait for batch distribution
        pthread_barrier_wait(&system->batch_barrier);
        
        // Check if we should stop
        if (!atomic_load(&system->running)) {
            break;
        }
        
        // POINT B: Wait for batch completion
        pthread_barrier_wait(&system->batch_barrier);
    }
    
    printf("[Node Zero] Control thread stopping\n");
    return NULL;
}

/**
 * Worker thread function - PHASE 3: Barrier Synchronization
 * 
 * Workers process batches assigned by main thread
 * Synchronization via barriers (no condition variables)
 * 
 * PHASE 3 IMPLEMENTATION:
 * - Wait at barrier for batch assignment (Point A)
 * - Process assigned batch
 * - Wait at barrier to signal completion (Point B)
 */
static void* sphere_worker_thread(void* arg) {
    SphereTrainingContext* ctx = (SphereTrainingContext*)arg;
    ThreadedTrainingSystem* system = ctx->system;
    
    printf("[Worker %d] Thread started (symmetry group %d)\n", 
           ctx->sphere_id, ctx->symmetry_group);
    fflush(stdout);
    
    int batches_processed = 0;
    
    while (atomic_load(&system->running)) {
        // POINT A: Wait for batch assignment from main thread
        pthread_barrier_wait(&system->batch_barrier);
        
        // Check if we should stop
        if (!atomic_load(&system->running)) {
            break;
        }
        
        // Process the batch if assigned
        if (ctx->current_batch) {
            sphere_process_batch(ctx, system->training);
            batches_processed++;
        }
        
        // POINT B: Signal completion to main thread
        pthread_barrier_wait(&system->batch_barrier);
    }
    
    printf("[Worker %d] Thread stopping (processed %d batches)\n", 
           ctx->sphere_id, batches_processed);
    
    return NULL;
}

/**
 * PHASE 3: Removed distribute_batch_to_sphere() and wait_for_sphere()
 * Now using barrier synchronization instead of condition variables
 */

/**
 * Validate gradients for NaN/Inf values
 */
static int validate_gradients(float* gradients, size_t size, const char* source) {
    int nan_count = 0;
    int inf_count = 0;
    
    for (size_t i = 0; i < size; i++) {
        if (isnan(gradients[i])) {
            nan_count++;
            if (nan_count <= 5) {  // Only log first 5
                fprintf(stderr, "ERROR: NaN gradient in %s at index %zu\n", source, i);
            }
        } else if (isinf(gradients[i])) {
            inf_count++;
            if (inf_count <= 5) {  // Only log first 5
                fprintf(stderr, "ERROR: Inf gradient in %s at index %zu: %f\n", source, i, gradients[i]);
            }
        }
    }
    
    if (nan_count > 0 || inf_count > 0) {
        fprintf(stderr, "ERROR: %s has %d NaN and %d Inf gradients (total size: %zu)\n", 
                source, nan_count, inf_count, size);
        return 0;
    }
    
    return 1;
}

/**
 * Clip gradients to prevent overflow
 */
static void clip_gradients(float* gradients, size_t size, float max_norm) {
    float norm = 0.0f;
    for (size_t i = 0; i < size; i++) {
        norm += gradients[i] * gradients[i];
    }
    norm = sqrtf(norm);
    
    if (norm > max_norm) {
        float scale = max_norm / norm;
        for (size_t i = 0; i < size; i++) {
            gradients[i] *= scale;
        }
        printf("  Clipped gradients: norm %.4f -> %.4f\n", norm, max_norm);
    }
}

/**
 * Accumulate gradients from all spheres (using shared memory)
 */
static void accumulate_gradients(ThreadedTrainingSystem* system) {
    if (!system || !system->accumulated_gradients) return;
    
    // Zero accumulated gradients
    memset(system->accumulated_gradients, 0, system->gradient_size * sizeof(float));
    
    int valid_spheres = 0;
    
    // Sum gradients from all spheres
    for (int i = 0; i < system->num_worker_spheres; i++) {
        SphereTrainingContext* ctx = system->sphere_contexts[i];
        if (!ctx || !ctx->local_gradients) continue;
        
        // Validate gradients before accumulation
        char source[64];
        snprintf(source, sizeof(source), "Sphere %d", i);
        if (!validate_gradients(ctx->local_gradients, ctx->gradient_size, source)) {
            fprintf(stderr, "WARNING: Skipping sphere %d due to invalid gradients\n", i);
            continue;
        }
        
        // Clip gradients to prevent overflow
        clip_gradients(ctx->local_gradients, ctx->gradient_size, 10.0f);
        
        for (size_t j = 0; j < system->gradient_size; j++) {
            system->accumulated_gradients[j] += ctx->local_gradients[j];
        }
        
        valid_spheres++;
    }
    
    // Average gradients across valid spheres only
    if (valid_spheres > 0) {
        for (size_t i = 0; i < system->gradient_size; i++) {
            system->accumulated_gradients[i] /= (float)valid_spheres;
        }
    }
    
    // Final validation of accumulated gradients
    if (!validate_gradients(system->accumulated_gradients, system->gradient_size, "Accumulated")) {
        fprintf(stderr, "CRITICAL: Accumulated gradients are invalid!\n");
    }
}

/**
 * Train one epoch with multi-threading
 */
float threaded_train_epoch(ThreadedTrainingSystem* system) {
    if (!system) return 0.0f;
    
    printf("\nStarting multi-threaded epoch training...\n");
    printf("Using %d worker threads for parallel batch processing\n\n", system->num_worker_spheres);
    
    printf("DEBUG: About to reset batch iterator\n"); fflush(stdout);
    cllm_batch_iterator_reset(system->batch_iterator);
    printf("DEBUG: Batch iterator reset complete\n"); fflush(stdout);
    
    float epoch_loss = 0.0f;
    int batch_count = 0;
    
    // Process batches in groups (one per sphere)
    int total_batch_groups = 0;
    size_t total_batches_in_epoch = cllm_batch_iterator_num_batches(system->batch_iterator);
    
    while (1) {
        printf("DEBUG: Allocating batch array for %d workers\n", system->num_worker_spheres); fflush(stdout);
        CLLMBatch** batches = (CLLMBatch**)calloc(system->num_worker_spheres, sizeof(CLLMBatch*));
        if (!batches) {
            printf("DEBUG: Failed to allocate batch array\n"); fflush(stdout);
            break;
        }
        
        int batches_loaded = 0;
        
        printf("DEBUG: Loading batches...\n"); fflush(stdout);
        // Load up to N batches (one per worker sphere)
        for (int i = 0; i < system->num_worker_spheres; i++) {
            printf("DEBUG: Loading batch %d/%d\n", i+1, system->num_worker_spheres); fflush(stdout);
            batches[i] = cllm_batch_iterator_next(system->batch_iterator);
            printf("DEBUG: Batch %d loaded: %p\n", i+1, (void*)batches[i]); fflush(stdout);
            if (batches[i]) {
                batches_loaded++;
            } else {
                // Iterator returned NULL - no more batches
                printf("DEBUG: No more batches at index %d\n", i); fflush(stdout);
                break;
            }
        }
        printf("DEBUG: Loaded %d batches total\n", batches_loaded); fflush(stdout);
        
        // If no batches loaded, we're done with the epoch
        if (batches_loaded == 0) {
            printf("DEBUG: No batches loaded, breaking\n"); fflush(stdout);
            free(batches);
            break;
        }
        
        printf("DEBUG: Checking if first batch group (total_batch_groups=%d)\n", total_batch_groups); fflush(stdout);
        // Debug: Show actual batch count
        if (total_batch_groups == 0) {
            printf("First batch group: loaded %d batches\n", batches_loaded);
        }
        
        printf("DEBUG: Incrementing total_batch_groups\n"); fflush(stdout);
        
        total_batch_groups++;
        
        printf("DEBUG: total_batch_groups=%d, checking safety\n", total_batch_groups); fflush(stdout);
        // Safety check: prevent infinite loops
        // Allow some margin since batch_groups != batches (multiple batches per group)
        int max_batch_groups = (int)total_batches_in_epoch + 10;
        printf("DEBUG: max_batch_groups=%d\n", max_batch_groups); fflush(stdout);
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
        
        printf("DEBUG: Safety check passed, about to print processing message\n"); fflush(stdout);
        printf("Processing batch group %d (%d batches across %d spheres)...\n",
               batch_count / system->num_worker_spheres + 1, batches_loaded, batches_loaded);
        printf("DEBUG: Processing message printed\n"); fflush(stdout);
        
        // PHASE 3: Assign batches to workers (no signaling yet)
        for (int i = 0; i < batches_loaded; i++) {
            system->sphere_contexts[i]->current_batch = batches[i];
        }
        
        // POINT A: Release workers to process batches
        pthread_barrier_wait(&system->batch_barrier);
        
        // POINT B: Wait for workers to complete
        pthread_barrier_wait(&system->batch_barrier);
        
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
        
        // Apply accumulated gradients to model
        pthread_mutex_lock(&system->gradient_lock);
        
        // Copy accumulated gradients to training object
        memcpy(system->training->gradients, system->accumulated_gradients, 
               system->gradient_size * sizeof(float));
        
        // Apply gradients using Adam optimizer
        cllm_optimizer_step_adam(system->training);
        
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
