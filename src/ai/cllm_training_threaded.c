/**
 * CLLM Parallel Training System - Main Training API
 * 
 * This is the PRIMARY training system for CLLM.
 * Uses 12-fold kissing spheres architecture for parallel batch processing.
 * 
 * Features:
 * - Parallel batch processing across N worker threads
 * - Thread-local activation buffers (no locking during forward/backward)
 * - Lock-free gradient accumulation via barriers
 * - Crystalline loss computation (GCD-based, O(log n))
 * - 12-fold symmetry structure (infinite recursive self-similar)
 * 
 * Usage:
 *   ThreadedTrainingSystem* system = threaded_training_create(training, batch_iterator, num_threads);
 *   float loss = threaded_train_epoch(system);
 *   threaded_training_free(system);
 * 
 * This is NOT an optional "threaded" version - it's the main implementation.
 * The "threaded" name is legacy and will be renamed in future refactoring.
 */

#include "cllm_training.h"
#include "cllm_training_threaded.h"
#include "cllm_threads.h"
#include "cllm_batch.h"
#include "cllm_simd_gradient_ops.h"
#include "ai/cllm_lattice_hierarchy.h"
#include "ai/cllm_shared_memory.h"
#include "ai/cllm_control_process.h"
#include "ai/cllm_sphere_stats.h"        // PHASE 7: Sphere statistics
#include "ai/cllm_sphere_message.h"      // PHASE 7: Sphere messaging
#include "cllm_metrics.h"                // UI Integration: Real-time metrics
#include "prime_float_math.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <immintrin.h>  // AVX/AVX2 SIMD instructions
// #include <math.h>  // OBJECTIVE 3A: Removed - using crystalline math only
#include <unistd.h>

/**
 * Thread-local training context for each sphere
 */
// Forward declarations
typedef struct ThreadedTrainingSystem ThreadedTrainingSystem;
typedef struct SphereTrainingContext SphereTrainingContext;

struct SphereTrainingContext {
    int sphere_id;
    int symmetry_group;  // 0-11 for the 12 kissing spheres
    
    // Reference to parent system
    ThreadedTrainingSystem* system;
    
    // Shared memory access (kissing spheres architecture)
    SharedMemoryRegion* shared_gradients;  // Reference to shared gradient buffer
    size_t gradient_segment_start;         // This sphere's lock-free segment start
    size_t gradient_segment_end;           // This sphere's lock-free segment end
    
    // Thread-local training context (PHASE 8: Remove model_lock)
    ThreadLocalTrainingContext* thread_local_training;
    
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
    
    // PHASE 6: Recursive Hierarchy
    int is_control_thread;                     // 1 if this is a control thread (has children)
    int hierarchy_level;                       // Level in hierarchy (0 = root)
    SphereTrainingContext** children;          // Array of child contexts (up to 12)
    int num_children;                          // Number of active children
    SphereTrainingContext* parent;             // Parent context (NULL for root)
    CLLMLatticeHierarchy* hierarchy_node;      // Corresponding hierarchy node
    
    // PHASE 7: Sphere Integration
    SphereStatistics sphere_stats;             // Sphere statistics tracking
    void* sphere_geometry;                     // Sphere geometry data (future)
};

/**
 * Multi-threaded training system
 */
// PHASE 2A: Batch Queue for Pre-fetching
#define BATCH_QUEUE_CAPACITY 128  // Pre-fetch up to 128 batches

typedef struct {
    CLLMBatch** batches;           // Array of batch pointers
    size_t capacity;               // Maximum queue size
    _Atomic size_t head;           // Read position (consumer)
    _Atomic size_t tail;           // Write position (producer)
    _Atomic int producer_done;     // 1 when no more batches to produce
    pthread_t prefetch_thread;     // Pre-fetch thread
    _Atomic int prefetch_running;  // 1 while pre-fetch thread is active
} BatchQueue;

// PHASE 2B: Lock-Free Work Queue for Worker Threads
#define MAX_WORK_ITEMS 256  // Maximum work items in queue

typedef struct {
    _Atomic(CLLMBatch*) batches[MAX_WORK_ITEMS];  // Work items (batches)
    _Atomic size_t head;                          // Consumer position (workers)
    _Atomic size_t tail;                          // Producer position (main thread)
    _Atomic int epoch_done;                       // 1 when epoch complete
    _Atomic size_t total_pushed;                  // Total batches pushed
    _Atomic size_t total_popped;                  // Total batches popped
} WorkQueue;

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
    
    // PHASE 2A: Batch pre-fetching queue
    BatchQueue* batch_queue;       // Lock-free batch queue for pre-fetching
    
    // PHASE 2B: Lock-free work queue for workers
    WorkQueue* work_queue;         // Lock-free work queue (replaces barriers)
    
    // Shared memory regions (kissing spheres architecture)
    SharedMemoryRegion* shared_gradients;      // Single shared gradient buffer
    SharedMemoryRegion* shared_model_weights;  // Shared model weights (read-only)
    size_t gradient_size;
    
    // Gradient accumulation (temporary until shared memory fully integrated)
    float* accumulated_gradients;              // Accumulated gradients from all spheres
    // pthread_mutex_t gradient_lock;             // PHASE 4: Removed - using lock-free accumulation
    // pthread_mutex_t model_lock;                // PHASE 8: Removed - using thread-local contexts
    
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
    
    // PHASE 5: Infrastructure Integration
    ControlProcess* control_process;           // Control process infrastructure
    CLLMLatticeHierarchy* root_hierarchy;      // Root of lattice hierarchy
    
    // UI Integration: Real-time metrics collection
    CLLMMetrics* metrics;                      // Metrics system for UI updates
};

/**
 * Initialize sphere training context
 */
/**
 * Thread-Local Training Context Functions
 * 
 * Each worker thread gets its own activation buffers to avoid race conditions.
 */

ThreadLocalTrainingContext* thread_local_training_create(
    int batch_size,
    int seq_len,
    int num_layers,
    int embed_dim,
    int vocab_size,
    int ff_hidden_dim,
    int num_heads
) {
    ThreadLocalTrainingContext* ctx = (ThreadLocalTrainingContext*)calloc(1, sizeof(ThreadLocalTrainingContext));
    if (!ctx) return NULL;
    
    // Store configuration
    ctx->batch_size = batch_size;
    ctx->seq_len = seq_len;
    ctx->num_layers = num_layers;
    ctx->embed_dim = embed_dim;
    ctx->vocab_size = vocab_size;
    ctx->ff_hidden_dim = ff_hidden_dim;
    ctx->num_heads = num_heads;
    
    size_t seq_size = batch_size * seq_len * embed_dim;
    size_t logits_size = batch_size * seq_len * vocab_size;
    size_t ff_size = batch_size * seq_len * ff_hidden_dim;
    
    // Allocate forward pass buffers
    ctx->input_embeddings = (float*)calloc(seq_size, sizeof(float));
    ctx->final_hidden = (float*)calloc(seq_size, sizeof(float));
    ctx->logits = (float*)calloc(logits_size, sizeof(float));
    
    // Allocate per-layer buffers
    ctx->layer_inputs = (float**)calloc(num_layers, sizeof(float*));
    ctx->attention_outputs = (float**)calloc(num_layers, sizeof(float*));
    ctx->ff_outputs = (float**)calloc(num_layers, sizeof(float*));
    ctx->layer_outputs = (float**)calloc(num_layers, sizeof(float*));
    ctx->ff_hidden = (float**)calloc(num_layers, sizeof(float*));
    
    if (ctx->layer_inputs && ctx->attention_outputs && ctx->ff_outputs &&
        ctx->layer_outputs && ctx->ff_hidden) {
        for (int i = 0; i < num_layers; i++) {
            ctx->layer_inputs[i] = (float*)calloc(seq_size, sizeof(float));
            ctx->attention_outputs[i] = (float*)calloc(seq_size, sizeof(float));
            ctx->ff_outputs[i] = (float*)calloc(seq_size, sizeof(float));
            ctx->layer_outputs[i] = (float*)calloc(seq_size, sizeof(float));
            ctx->ff_hidden[i] = (float*)calloc(ff_size, sizeof(float));
        }
    }
    
    // Allocate attention cache
    ctx->attention_cache = (typeof(ctx->attention_cache))calloc(num_layers, sizeof(*ctx->attention_cache));
    if (ctx->attention_cache) {
        for (int i = 0; i < num_layers; i++) {
            ctx->attention_cache[i].queries = (float*)calloc(seq_len * embed_dim, sizeof(float));
            ctx->attention_cache[i].keys = (float*)calloc(seq_len * embed_dim, sizeof(float));
            ctx->attention_cache[i].values = (float*)calloc(seq_len * embed_dim, sizeof(float));
            ctx->attention_cache[i].attention_weights = (float*)calloc(num_heads * seq_len * seq_len, sizeof(float));
            ctx->attention_cache[i].scores = (float*)calloc(num_heads * seq_len * seq_len, sizeof(float));
        }
    }
    
    // Allocate backward pass temporary buffers
    ctx->grad_logits = (float*)calloc(logits_size, sizeof(float));
    ctx->grad_hidden = (float*)calloc(seq_size, sizeof(float));
    ctx->grad_layer = (float*)calloc(seq_size, sizeof(float));
    
    return ctx;
}

void thread_local_training_free(ThreadLocalTrainingContext* ctx) {
    if (!ctx) return;
    
    // Free forward pass buffers
    free(ctx->input_embeddings);
    free(ctx->final_hidden);
    free(ctx->logits);
    
    // Free per-layer buffers
    if (ctx->layer_inputs) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->layer_inputs[i]);
        }
        free(ctx->layer_inputs);
    }
    
    if (ctx->attention_outputs) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->attention_outputs[i]);
        }
        free(ctx->attention_outputs);
    }
    
    if (ctx->ff_outputs) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->ff_outputs[i]);
        }
        free(ctx->ff_outputs);
    }
    
    if (ctx->layer_outputs) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->layer_outputs[i]);
        }
        free(ctx->layer_outputs);
    }
    
    if (ctx->ff_hidden) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->ff_hidden[i]);
        }
        free(ctx->ff_hidden);
    }
    
    // Free attention cache
    if (ctx->attention_cache) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->attention_cache[i].queries);
            free(ctx->attention_cache[i].keys);
            free(ctx->attention_cache[i].values);
            free(ctx->attention_cache[i].attention_weights);
            free(ctx->attention_cache[i].scores);
        }
        free(ctx->attention_cache);
    }
    
    // Free backward pass buffers
    free(ctx->grad_logits);
    free(ctx->grad_hidden);
    free(ctx->grad_layer);
    
    free(ctx);
}

/**
 * Threaded Forward Pass
 * 
 * Same as cllm_forward_training() but uses thread-local activation buffers.
 * This allows multiple threads to execute in parallel without locking.
 */
float cllm_forward_training_threaded(
    CLLMTraining* training,
    ThreadLocalTrainingContext* local_ctx,
    uint32_t* input_tokens
) {
    if (!training || !local_ctx || !input_tokens) return 0.0f;
    printf("    [DEBUG] Entered cllm_forward_training_threaded\n");
    fflush(stdout);
    
    CLLMModel* model = training->model;
    int batch_size = local_ctx->batch_size;
    int seq_len = local_ctx->seq_len;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Get embeddings (write to thread-local buffer)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t token_id = input_tokens[idx];
            if (token_id >= vocab_size) continue;
            
            float* embed_src = &model->embeddings.embeddings[token_id * embed_dim];
            float* embed_dst = &local_ctx->input_embeddings[idx * embed_dim];
            memcpy(embed_dst, embed_src, embed_dim * sizeof(float));
        }
    }
    
    // Process through layers (all writes go to thread-local buffers)
    printf("    [DEBUG] Embeddings copied, starting layer processing (num_layers=%d)\n", model->num_layers);
    fflush(stdout);
    float* layer_input = local_ctx->input_embeddings;
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        printf("    [DEBUG] Processing layer %d\n", layer);
        fflush(stdout);
        memcpy(local_ctx->layer_inputs[layer], layer_input, batch_size * seq_len * embed_dim * sizeof(float));
        
        // Apply multi-head attention (simplified for thread-local context)
        // TODO: Implement full attention with thread-local caching
        // For now, just copy input to output (identity mapping)
        memcpy(local_ctx->attention_outputs[layer], layer_input, 
               batch_size * seq_len * embed_dim * sizeof(float));
        
        // Process feedforward
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* attn_out = &local_ctx->attention_outputs[layer][idx * embed_dim];
                float* ff_out = &local_ctx->ff_outputs[layer][idx * embed_dim];
                float* layer_out = &local_ctx->layer_outputs[layer][idx * embed_dim];
                
                // FeedForward
                FeedForwardLayer* ff = &model->ff_layers[layer];
                float* ff_hidden = &local_ctx->ff_hidden[layer][idx * ff->hidden_dim];
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    float sum = ff->bias1[h];
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        sum += attn_out[i] * ff->w1_lattice[i * ff->hidden_dim + h];
                    }
                    ff_hidden[h] = prime_tanhf(sum);
                }
                
                for (uint32_t o = 0; o < embed_dim; o++) {
                    float sum = ff->bias2[o];
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        sum += ff_hidden[h] * ff->w2_lattice[h * embed_dim + o];
                    }
                    ff_out[o] = sum;
                }
                
                // Residual + LayerNorm
                for (uint32_t d = 0; d < embed_dim; d++) layer_out[d] = attn_out[d] + ff_out[d];
                
                CLLMLayerNorm* ln = &model->layer_norms[layer];
                float mean = 0.0f, var = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) mean += layer_out[d];
                mean /= embed_dim;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float diff = layer_out[d] - mean;
                    var += diff * diff;
                }
                var /= embed_dim;
                float std = prime_sqrtf(var + 1e-5f);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    layer_out[d] = ln->gamma[d] * (layer_out[d] - mean) / std + ln->beta[d];
                }
            }
        }
        layer_input = local_ctx->layer_outputs[layer];
    }
    
    printf("    [DEBUG] All layers processed, computing logits\n");
    fflush(stdout);
    // Copy final hidden (to thread-local buffer)
    memcpy(local_ctx->final_hidden, layer_input, batch_size * seq_len * embed_dim * sizeof(float));
    
    // Project to vocabulary (write to thread-local logits)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            float* hidden = &local_ctx->final_hidden[idx * embed_dim];
            float* logits = &local_ctx->logits[idx * vocab_size];
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                float* vocab_embed = &model->embeddings.embeddings[v * embed_dim];
                float score = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    score += hidden[d] * vocab_embed[d];
                }
                logits[v] = score;
            }
        }
    }
    
    return 0.0f;
}

/**
 * Threaded Backward Pass
 * 
 * Same as cllm_backward_training() but uses thread-local activation buffers.
 * Gradients are written to the provided gradient_buffer (lock-free segment).
 */
void cllm_backward_training_threaded(
    CLLMTraining* training,
    ThreadLocalTrainingContext* local_ctx,
    uint32_t* target_tokens,
    float* gradient_buffer
) {
    if (!training || !local_ctx || !target_tokens) return;
    if (!gradient_buffer) return;
    
    CLLMModel* model = training->model;
    int batch_size = local_ctx->batch_size;
    int seq_len = local_ctx->seq_len;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Use thread-local temporary buffers
    float* grad_logits = local_ctx->grad_logits;
    float* grad_hidden = local_ctx->grad_hidden;
    float* grad_layer = local_ctx->grad_layer;
    
    // Zero the buffers
    memset(grad_logits, 0, batch_size * seq_len * vocab_size * sizeof(float));
    memset(grad_hidden, 0, batch_size * seq_len * embed_dim * sizeof(float));
    memset(grad_layer, 0, batch_size * seq_len * embed_dim * sizeof(float));
    
    // Gradient of cross-entropy w.r.t. logits (using thread-local logits)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t target = target_tokens[idx];
            if (target >= vocab_size) continue;
            
            float* logits = &local_ctx->logits[idx * vocab_size];
            float* grad = &grad_logits[idx * vocab_size];
            
            float max_logit = logits[0];
            for (uint32_t v = 1; v < vocab_size; v++) {
                if (logits[v] > max_logit) max_logit = logits[v];
            }
            
            float sum_exp = 0.0f;
            for (uint32_t v = 0; v < vocab_size; v++) {
                sum_exp += prime_expf(logits[v] - max_logit);
            }
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                float prob = prime_expf(logits[v] - max_logit) / sum_exp;
                grad[v] = prob - (v == target ? 1.0f : 0.0f);
            }
        }
    }
    
    // Backprop through vocabulary projection (write to gradient_buffer)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            float* grad_logit = &grad_logits[idx * vocab_size];
            float* hidden = &local_ctx->final_hidden[idx * embed_dim];
            float* grad_h = &grad_hidden[idx * embed_dim];
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                float* vocab_embed = &model->embeddings.embeddings[v * embed_dim];
                float grad_v = grad_logit[v];
                
                // Accumulate to gradient_buffer (lock-free segment)
                for (uint32_t d = 0; d < embed_dim; d++) {
                    gradient_buffer[v * embed_dim + d] += grad_v * hidden[d];
                    grad_h[d] += grad_v * vocab_embed[d];
                }
            }
        }
    }
    
    // Note: Full backward pass through layers would go here
    // For now, we're just doing the vocabulary projection gradients
    // The full implementation would backprop through all layers
}

/**
 * Sphere Context Functions
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
    
    
    // PHASE 6: Initialize recursive hierarchy fields
    ctx->is_control_thread = 0;  // Start as worker
    ctx->hierarchy_level = 0;    // Will be set by parent
    ctx->children = NULL;        // No children initially
    ctx->num_children = 0;
    ctx->parent = NULL;          // Will be set if spawned
    ctx->hierarchy_node = NULL;  // Will be linked to hierarchy
    
    // PHASE 7: Initialize sphere statistics
    cllm_sphere_stats_init(&ctx->sphere_stats, symmetry_group, 0);
    ctx->sphere_geometry = NULL;  // Future: sphere geometry data
    
    // PHASE 8: Thread-local training context (will be allocated later with model info)
    ctx->thread_local_training = NULL;
    
    return ctx;
}

/**
 * Free sphere training context
 */
static void sphere_context_free(SphereTrainingContext* ctx) {
    if (!ctx) return;
    
    
    // PHASE 6: Free children recursively
    if (ctx->children) {
        for (int i = 0; i < ctx->num_children; i++) {
            if (ctx->children[i]) {
                sphere_context_free(ctx->children[i]);
            }
        }
        free(ctx->children);
    }
    // PHASE 8: Free thread-local training context
    if (ctx->thread_local_training) {
    
    // Ensure current_batch is not freed here (should be NULL)
    if (ctx->current_batch) {
        fprintf(stderr, "WARNING: sphere_context_free called with non-NULL current_batch!\n");
        ctx->current_batch = NULL;  // Don't free it, just clear the pointer
    }
        thread_local_training_free(ctx->thread_local_training);
    }
    
    free(ctx->local_gradients);
    pthread_mutex_destroy(&ctx->lock);
    pthread_cond_destroy(&ctx->work_ready);
    pthread_cond_destroy(&ctx->work_done);
    free(ctx);
}

// Forward declarations
static void* sphere_worker_thread(void* arg);
static void* sphere_worker_thread_lockfree(void* arg);
static void* control_thread_func(void* arg);  // Node Zero - NEVER processes batches
static void sphere_process_batch(SphereTrainingContext* ctx, CLLMTraining* training);
static void accumulate_gradients(ThreadedTrainingSystem* system);
static void accumulate_gradients_lockfree(ThreadedTrainingSystem* system);  // PHASE 4
static int sphere_spawn_children(SphereTrainingContext* parent, int num_children);  // PHASE 6
static int validate_gradients(float* gradients, size_t size, const char* source);
static void clip_gradients(float* gradients, size_t size, float max_norm);

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
        
        // PHASE 8: Use thread-local context (NO LOCKING NEEDED!)
        // Each thread has its own activation buffers, so no race conditions
        
        // Forward pass using thread-local buffers
        float seq_loss = cllm_forward_training_threaded(
            training, 
            ctx->thread_local_training,
            &batch->input_ids[offset]
        );
        
        // Compute loss
        seq_loss += cllm_compute_loss(training, &batch->input_ids[offset], &batch->target_ids[offset], batch->seq_len);
        
        // Backward pass - compute gradients to local buffer (thread-local)
        cllm_backward_training_threaded(
            training,
            ctx->thread_local_training,
            &batch->target_ids[offset],
            ctx->local_gradients
        );
        
        total_loss += seq_loss;
        valid_sequences++;
    }
    
    ctx->batch_loss = (valid_sequences > 0) ? total_loss / valid_sequences : 0.0f;
    ctx->batches_processed++;
    
    // PHASE 7: Record sphere statistics
    cllm_sphere_stats_record_batch(&ctx->sphere_stats, ctx->batch_loss, valid_sequences);
    
    // PHASE 4: Lock-free gradient accumulation
    // Each worker writes ONLY to its own segment (no locking needed)
    // This is safe because each worker has exclusive access to its segment
    ThreadedTrainingSystem* system = ctx->system;
    for (size_t i = ctx->gradient_segment_start; i < ctx->gradient_segment_end && i < ctx->gradient_size; i++) {
        system->accumulated_gradients[i] = ctx->local_gradients[i];
    }
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
 * PHASE 2B: Lock-Free Work Queue Functions
 */

/**
 * Create work queue
 */
static WorkQueue* work_queue_create(void) {
    WorkQueue* queue = (WorkQueue*)calloc(1, sizeof(WorkQueue));
    if (!queue) return NULL;
    
    // Initialize all batch pointers to NULL
    for (int i = 0; i < MAX_WORK_ITEMS; i++) {
        atomic_init(&queue->batches[i], NULL);
    }
    
    atomic_init(&queue->head, 0);
    atomic_init(&queue->tail, 0);
    atomic_init(&queue->epoch_done, 0);
    atomic_init(&queue->total_pushed, 0);
    atomic_init(&queue->total_popped, 0);
    
    return queue;
}

/**
 * Free work queue
 */
static void work_queue_free(WorkQueue* queue) {
    if (!queue) return;
    
    // Free any remaining batches
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    for (size_t i = head; i < tail; i++) {
        size_t index = i % MAX_WORK_ITEMS;
        CLLMBatch* batch = atomic_load(&queue->batches[index]);
        if (batch) {
            cllm_batch_free(batch);
        }
    }
    
    free(queue);
}

/**
 * Push work item to queue (main thread)
 * Returns 1 on success, 0 if queue is full
 */
static int work_queue_push(WorkQueue* queue, CLLMBatch* batch) {
    if (!queue || !batch) return 0;
    
    size_t tail = atomic_load(&queue->tail);
    size_t head = atomic_load(&queue->head);
    
    // Check if queue is full
    if (tail - head >= MAX_WORK_ITEMS) {
        return 0;  // Queue full
    }
    
    size_t index = tail % MAX_WORK_ITEMS;
    atomic_store(&queue->batches[index], batch);
    atomic_store(&queue->tail, tail + 1);
    atomic_fetch_add(&queue->total_pushed, 1);
    
    return 1;
}

/**
 * Pop work item from queue (worker threads)
 * Returns batch pointer on success, NULL if no work available
 */
static CLLMBatch* work_queue_pop(WorkQueue* queue) {
    if (!queue) return NULL;
    
    // Try a few times before giving up
    for (int attempts = 0; attempts < 10; attempts++) {
        size_t head = atomic_load(&queue->head);
        size_t tail = atomic_load(&queue->tail);
        
        // Check if queue is empty
        if (head >= tail) {
            // Queue empty - check if epoch done
            if (atomic_load(&queue->epoch_done)) {
                return NULL;  // No more work
            }
            
            // Spin briefly
            for (int i = 0; i < 100; i++) {
                __asm__ __volatile__("pause" ::: "memory");  // CPU hint for spin-wait
            }
            continue;
        }
        
        // Try to claim this work item using CAS
        size_t new_head = head + 1;
        if (atomic_compare_exchange_weak(&queue->head, &head, new_head)) {
            // Successfully claimed - get the batch
            size_t index = head % MAX_WORK_ITEMS;
            CLLMBatch* batch = atomic_exchange(&queue->batches[index], NULL);
            
            if (batch) {
                atomic_fetch_add(&queue->total_popped, 1);
                return batch;
            }
        }
        
        // CAS failed or batch was NULL - try again
    }
    
    // Couldn't get work after several attempts
    return NULL;
}

/**
 * Reset work queue for new epoch
 */
static void work_queue_reset(WorkQueue* queue) {
    if (!queue) return;
    
    atomic_store(&queue->head, 0);
    atomic_store(&queue->tail, 0);
    atomic_store(&queue->epoch_done, 0);
    atomic_store(&queue->total_pushed, 0);
    atomic_store(&queue->total_popped, 0);
    
    // Clear all batch pointers
    for (int i = 0; i < MAX_WORK_ITEMS; i++) {
        atomic_store(&queue->batches[i], NULL);
    }
}

/**
 * Check if all work is complete
 */
static int work_queue_is_complete(WorkQueue* queue) {
    if (!queue) return 1;
    
    size_t pushed = atomic_load(&queue->total_pushed);
    size_t popped = atomic_load(&queue->total_popped);
    int done = atomic_load(&queue->epoch_done);
    
    return (done && pushed == popped);
}

/**
 * Get work queue statistics
 */
static void work_queue_stats(WorkQueue* queue, size_t* pending, size_t* pushed, size_t* popped) {
    if (!queue) return;
    
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    *pending = (tail > head) ? (tail - head) : 0;
    *pushed = atomic_load(&queue->total_pushed);
    *popped = atomic_load(&queue->total_popped);
}

/**
 * PHASE 2A: Batch Queue Functions
 */

/**
 * Create batch queue for pre-fetching
 */
static BatchQueue* batch_queue_create(size_t capacity) {
    BatchQueue* queue = (BatchQueue*)calloc(1, sizeof(BatchQueue));
    if (!queue) return NULL;
    
    queue->batches = (CLLMBatch**)calloc(capacity, sizeof(CLLMBatch*));
    if (!queue->batches) {
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    atomic_store(&queue->head, 0);
    atomic_store(&queue->tail, 0);
    atomic_store(&queue->producer_done, 0);
    atomic_store(&queue->prefetch_running, 0);
    
    return queue;
}

/**
 * Free batch queue
 */
static void batch_queue_free(BatchQueue* queue) {
    if (!queue) return;
    
    // Free any remaining batches
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    for (size_t i = head; i < tail; i++) {
        size_t index = i % queue->capacity;
        if (queue->batches[index]) {
            cllm_batch_free(queue->batches[index]);
            queue->batches[index] = NULL;
        }
    }
    
    free(queue->batches);
    free(queue);
}

/**
 * Try to push a batch to the queue (non-blocking)
 * Returns 1 on success, 0 if queue is full
 */
static int batch_queue_try_push(BatchQueue* queue, CLLMBatch* batch) {
    if (!queue || !batch) return 0;
    
    size_t tail = atomic_load(&queue->tail);
    size_t head = atomic_load(&queue->head);
    
    // Check if queue is full
    if (tail - head >= queue->capacity) {
        return 0;  // Queue full
    }
    
    size_t index = tail % queue->capacity;
    queue->batches[index] = batch;
    
    // Advance tail
    atomic_store(&queue->tail, tail + 1);
    
    return 1;
}

/**
 * Try to pop a batch from the queue (non-blocking)
 * Returns batch pointer on success, NULL if queue is empty
 */
static CLLMBatch* batch_queue_try_pop(BatchQueue* queue) {
    if (!queue) return NULL;
    
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    // Check if queue is empty
    if (head >= tail) {
        return NULL;  // Queue empty
    }
    
    size_t index = head % queue->capacity;
    CLLMBatch* batch = queue->batches[index];
    queue->batches[index] = NULL;
    
    // Advance head
    atomic_store(&queue->head, head + 1);
    
    return batch;
}

/**
 * Check if queue is empty
 */
__attribute__((unused))
static int batch_queue_is_empty(BatchQueue* queue) {
    if (!queue) return 1;
    
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    return head >= tail;
}

/**
 * Get number of batches in queue
 */
static size_t batch_queue_size(BatchQueue* queue) {
    if (!queue) return 0;
    
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    return tail - head;
}

/**
 * Batch pre-fetch thread function
 * Runs in background, loading batches ahead of time
 */
static void* batch_prefetch_thread_func(void* arg) {
    ThreadedTrainingSystem* system = (ThreadedTrainingSystem*)arg;
    if (!system || !system->batch_queue || !system->batch_iterator) {
        return NULL;
    }
    
    BatchQueue* queue = system->batch_queue;
    CLLMBatchIterator* iterator = system->batch_iterator;
    
    printf("[Pre-fetch] Thread started\n");
    
    int batches_loaded = 0;
    
    while (atomic_load(&system->running)) {
        // Try to load next batch
        CLLMBatch* batch = cllm_batch_iterator_next(iterator);
        
        if (!batch) {
            // No more batches - mark producer as done
            atomic_store(&queue->producer_done, 1);
            printf("[Pre-fetch] Loaded %d batches total, iterator exhausted\n", batches_loaded);
            break;
        }
        
        // Try to push to queue (non-blocking)
        while (!batch_queue_try_push(queue, batch)) {
            // Queue is full - wait a bit for consumers to catch up
            if (!atomic_load(&system->running)) {
                // System shutting down - free batch and exit
                cllm_batch_free(batch);
                goto cleanup;
            }
            usleep(1000);  // Sleep 1ms
        }
        
        batches_loaded++;
        
        // Log progress every 100 batches
        if (batches_loaded % 100 == 0) {
            printf("[Pre-fetch] Loaded %d batches (queue size: %zu)\n", 
                   batches_loaded, batch_queue_size(queue));
        }
    }
    
cleanup:
    atomic_store(&queue->prefetch_running, 0);
    printf("[Pre-fetch] Thread exiting\n");
    return NULL;
}

/**
 * Start batch pre-fetching
 */
static int batch_queue_start_prefetch(ThreadedTrainingSystem* system) {
    if (!system || !system->batch_queue) return 0;
    
    BatchQueue* queue = system->batch_queue;
    
    // Reset queue state
    atomic_store(&queue->head, 0);
    atomic_store(&queue->tail, 0);
    atomic_store(&queue->producer_done, 0);
    atomic_store(&queue->prefetch_running, 1);
    
    // Create pre-fetch thread
    if (pthread_create(&queue->prefetch_thread, NULL, 
                      batch_prefetch_thread_func, system) != 0) {
        fprintf(stderr, "Failed to create pre-fetch thread\n");
        atomic_store(&queue->prefetch_running, 0);
        return 0;
    }
    
    printf("[Pre-fetch] Started background batch loading\n");
    return 1;
}

/**
 * Stop batch pre-fetching and wait for thread to finish
 */
static void batch_queue_stop_prefetch(ThreadedTrainingSystem* system) {
    if (!system || !system->batch_queue) return;
    
    BatchQueue* queue = system->batch_queue;
    
    if (atomic_load(&queue->prefetch_running)) {
        // Wait for pre-fetch thread to finish
        pthread_join(queue->prefetch_thread, NULL);
        atomic_store(&queue->prefetch_running, 0);
    }
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
    // pthread_mutex_init(&system->gradient_lock, NULL);  // PHASE 4: Removed
    // pthread_mutex_init(&system->model_lock, NULL);     // PHASE 8: Removed
    
    // Initialize barriers for N worker threads + 1 control thread + 1 main thread
    // Total participants: num_threads (workers) + 1 (control) + 1 (main)
    pthread_barrier_init(&system->epoch_barrier, NULL, num_threads + 2);
    pthread_barrier_init(&system->batch_barrier, NULL, num_threads + 2);
    
    // OPTIMIZATION PHASE 2: Skip full kissing spheres hierarchy creation
    // Only create sphere contexts for active workers (saves 376MB)
    // The full 157-sphere hierarchy is not needed until dynamic spawning
    system->thread_system = NULL;  // Skip for now
    
    // Note: threads_create() would create 157 spheres (1 + 12 + 144)
    // We only need num_threads spheres, so we skip it and create contexts directly
    
    // PHASE 5: Initialize control process infrastructure
    SystemConfiguration control_config = {
        .max_hierarchy_depth = hierarchy_levels,
        .max_spheres_per_level = 12,
        .initial_sphere_count = num_threads,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = num_threads,
        .max_memory_bytes = 1024 * 1024 * 1024,  // 1GB
        .sync_interval_batches = 1,
        .checkpoint_interval_epochs = 10,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 60.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    system->control_process = control_process_create(&control_config);
    if (!system->control_process) {
        fprintf(stderr, "WARNING: Failed to create control process (continuing without it)\n");
        system->control_process = NULL;  // Continue without control process
    } else {
        printf("  ✓ Control process infrastructure initialized\n");
    }
    
    // PHASE 5: Initialize root lattice hierarchy
    int root_symmetry_groups[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    system->root_hierarchy = lattice_hierarchy_create(
        0,                      // sphere_id (root)
        0,                      // hierarchy_level (root level)
        root_symmetry_groups,   // symmetry_groups
        12,                     // num_symmetry_groups
        -1,                     // physical_thread_id (not assigned yet)
        NULL                    // parent (root has no parent)
    );
    if (!system->root_hierarchy) {
        fprintf(stderr, "WARNING: Failed to create root hierarchy (continuing without it)\n");
        system->root_hierarchy = NULL;
    } else {
        printf("  ✓ Root lattice hierarchy created (12-fold structure)\n");
    }
    // Allocate dynamic sphere contexts array
    system->sphere_contexts = (SphereTrainingContext**)calloc(system->num_worker_spheres, 
                                                               sizeof(SphereTrainingContext*));
    if (!system->sphere_contexts) {
        if (system->thread_system) threads_free(system->thread_system);
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
            if (system->thread_system) threads_free(system->thread_system);
            shared_memory_free(system->shared_gradients);
            free(system);
            return NULL;
        }
        // Set system reference
        system->sphere_contexts[i]->system = system;
        
        // PHASE 8: Allocate thread-local training context for each worker
        CLLMModel* model = training->model;
        int ff_hidden_dim = model->ff_layers ? model->ff_layers[0].hidden_dim : 1024;
        int num_heads = 8;  // TODO: Get from model config
        
        system->sphere_contexts[i]->thread_local_training = thread_local_training_create(
            training->config.batch_size,
            training->config.sequence_length,
            model->num_layers,
            model->embedding_dim,
            model->vocab_size,
            ff_hidden_dim,
            num_heads
        );
        
        if (!system->sphere_contexts[i]->thread_local_training) {
            fprintf(stderr, "ERROR: Failed to allocate thread-local training context for worker %d\n", i);
            // Cleanup on failure
            for (int j = 0; j <= i; j++) {
                sphere_context_free(system->sphere_contexts[j]);
            }
            free(system->sphere_contexts);
            if (system->thread_system) threads_free(system->thread_system);
            shared_memory_free(system->shared_gradients);
            free(system);
            return NULL;
        }
    }
    
    // PHASE 2A: Create batch queue for pre-fetching
    system->batch_queue = batch_queue_create(BATCH_QUEUE_CAPACITY);
    if (!system->batch_queue) {
        fprintf(stderr, "ERROR: Failed to create batch queue\n");
        // Cleanup on failure
        for (int j = 0; j < system->num_worker_spheres; j++) {
            sphere_context_free(system->sphere_contexts[j]);
        }
        free(system->sphere_contexts);
        if (system->thread_system) threads_free(system->thread_system);
        shared_memory_free(system->shared_gradients);
        free(system->accumulated_gradients);
        free(system);
        return NULL;
    }
    printf("  ✓ Batch queue created (capacity: %d batches)\n", BATCH_QUEUE_CAPACITY);
    
    // PHASE 2B: Create lock-free work queue
    system->work_queue = work_queue_create();
    if (!system->work_queue) {
        fprintf(stderr, "ERROR: Failed to create work queue\n");
        // Cleanup on failure
        batch_queue_free(system->batch_queue);
        for (int j = 0; j < system->num_worker_spheres; j++) {
            sphere_context_free(system->sphere_contexts[j]);
        }
        free(system->sphere_contexts);
        if (system->thread_system) threads_free(system->thread_system);
        shared_memory_free(system->shared_gradients);
        free(system->accumulated_gradients);
        free(system);
        return NULL;
    }
    printf("  ✓ Work queue created (capacity: %d work items)\n", MAX_WORK_ITEMS);
    
    // MASTER PLAN: Create control thread (Node Zero) first
    // OPTIMIZATION: Reduce thread stack size from 8MB to 1MB (saves 455MB with 65 threads)
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setstacksize(&thread_attr, 1024 * 1024);  // 1MB stack
    system->has_control_thread = 1;
    system->control_running = 1;
    
    int rc = pthread_create(&system->control_thread, &thread_attr, 
                           control_thread_func, system);
    pthread_attr_destroy(&thread_attr);
    if (rc != 0) {
        fprintf(stderr, "ERROR: Failed to create control thread (error %d)\n", rc);
        // Cleanup
        for (int j = 0; j < system->num_worker_spheres; j++) {
            sphere_context_free(system->sphere_contexts[j]);
        }
        free(system->sphere_contexts);
        if (system->thread_system) threads_free(system->thread_system);
        shared_memory_free(system->shared_gradients);
        free(system);
        return NULL;
    }
    printf("  ✓ Node Zero created (control thread NEVER processes batches)\n");
    
    // Create worker threads
    printf("  Creating %d worker threads (PHASE 2B: LOCK-FREE MODE)...\n", system->num_worker_spheres);
    for (int i = 0; i < system->num_worker_spheres; i++) {
        // OPTIMIZATION: Use 1MB stack for workers
        pthread_attr_t worker_attr;
        pthread_attr_init(&worker_attr);
        pthread_attr_setstacksize(&worker_attr, 1024 * 1024);
        // PHASE 2B: Use lock-free worker thread (no barriers!)
        rc = pthread_create(&system->sphere_contexts[i]->thread, &worker_attr, 
                           sphere_worker_thread_lockfree, system->sphere_contexts[i]);
        pthread_attr_destroy(&worker_attr);
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
            if (system->thread_system) threads_free(system->thread_system);
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
    
    // UI Integration: Initialize metrics system
    system->metrics = cllm_metrics_create(system->num_worker_spheres);
    if (!system->metrics) {
        fprintf(stderr, "WARNING: Failed to create metrics system (UI updates disabled)\n");
    } else {
        printf("  ✓ Metrics system initialized for UI integration\n");
    }
    
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
    
    if (system->thread_system) threads_free(system->thread_system);
    
    // Free shared memory regions
    shared_memory_free(system->shared_gradients);
    if (system->shared_model_weights) {
        shared_memory_free(system->shared_model_weights);
    }
    
    // Free accumulated gradients buffer
    free(system->accumulated_gradients);
    
    // PHASE 2A: Stop pre-fetch thread and free batch queue
    if (system->batch_queue) {
        batch_queue_stop_prefetch(system);
        batch_queue_free(system->batch_queue);
        printf("  ✓ Batch queue freed\n");
    }
    
    // PHASE 2B: Free work queue
    if (system->work_queue) {
        work_queue_free(system->work_queue);
        printf("  ✓ Work queue freed\n");
    }
    
    // Destroy gradient lock
    // pthread_mutex_destroy(&system->gradient_lock);  // PHASE 4: Removed
    // pthread_mutex_destroy(&system->model_lock);     // PHASE 8: Removed
    
    pthread_barrier_destroy(&system->epoch_barrier);
    pthread_barrier_destroy(&system->batch_barrier);
    
    // PHASE 5: Cleanup infrastructure
    if (system->control_process) {
        control_process_free(system->control_process);
        printf("  ✓ Control process freed\n");
    }
    
    if (system->root_hierarchy) {
        lattice_hierarchy_free(system->root_hierarchy);
        printf("  ✓ Root hierarchy freed\n");
    }
    
    // UI Integration: Cleanup metrics system
    if (system->metrics) {
        cllm_metrics_free(system->metrics);
        printf("  ✓ Metrics system freed\n");
    }
    
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
    printf("[Node Zero] Using barrier synchronization + lock-free gradient accumulation\n");
    
    while (1) {
        // POINT A: Wait for batch distribution
        pthread_barrier_wait(&system->batch_barrier);
        
        // Check if we should stop
       // Check if we should stop (AFTER barrier to avoid deadlock)
       if (!atomic_load(&system->running)) {
           // Must still participate in Point B barrier before exiting
           pthread_barrier_wait(&system->batch_barrier);
           break;
       }
        
        // Workers are now processing batches...
        // Control thread waits at next barrier
        
        // POINT B: Wait for batch completion
        pthread_barrier_wait(&system->batch_barrier);
        
        // PHASE 4: After Point B, workers are waiting at barrier
        // Safe to read their gradient segments and accumulate
        // This is lock-free because workers are blocked at barrier
        accumulate_gradients_lockfree(system);
    }
    
    printf("[Node Zero] Control thread stopping\n");
    return NULL;
}

/**
 * PHASE 2B: Lock-Free Worker Thread Function
 * 
 * Workers pull batches from work queue (no barriers!)
 * Continuous work processing without synchronization overhead
 * 
 * PHASE 2B IMPLEMENTATION:
 * - Pop batch from work queue (non-blocking)
 * - Process batch immediately
 * - Repeat until epoch done
 */
static void* sphere_worker_thread_lockfree(void* arg) {
    SphereTrainingContext* ctx = (SphereTrainingContext*)arg;
    ThreadedTrainingSystem* system = ctx->system;
    
    printf("[Worker %d] Thread started (symmetry group %d) - LOCK-FREE MODE\n", 
           ctx->sphere_id, ctx->symmetry_group);
    
    // UI Integration: Update thread state to WORKING
    if (system->metrics) {
        cllm_metrics_update_thread_state(system->metrics, ctx->sphere_id, THREAD_STATE_WORKING);
    }
    
    int batches_processed = 0;
    
    while (atomic_load(&system->running)) {
        // Pop work from queue (non-blocking)
        CLLMBatch* batch = work_queue_pop(system->work_queue);
        
        if (!batch) {
            // No work available - check if epoch done
            if (atomic_load(&system->work_queue->epoch_done)) {
                break;  // Epoch complete
            }
            
            // UI Integration: Update thread state to IDLE
            if (system->metrics) {
                cllm_metrics_update_thread_state(system->metrics, ctx->sphere_id, THREAD_STATE_IDLE);
            }
            
            // Yield CPU to other threads
            sched_yield();
            continue;
        }
        
        // UI Integration: Update thread state to WORKING
        if (system->metrics) {
            cllm_metrics_update_thread_state(system->metrics, ctx->sphere_id, THREAD_STATE_WORKING);
        }
        
        // Process batch immediately (no waiting!)
        ctx->current_batch = batch;
        
        // PHASE 6: Control threads NEVER process batches
        if (!ctx->is_control_thread) {
            sphere_process_batch(ctx, system->training);
            batches_processed++;
            
            // UI Integration: Update thread workload
            if (system->metrics) {
                cllm_metrics_update_thread_workload(system->metrics, ctx->sphere_id, batches_processed);
            }
        }
        
        // Free batch
        cllm_batch_free(batch);
        ctx->current_batch = NULL;
    }
    
    // UI Integration: Update thread state to TERMINATED
    if (system->metrics) {
        cllm_metrics_update_thread_state(system->metrics, ctx->sphere_id, THREAD_STATE_TERMINATED);
    }
    
    printf("[Worker %d] Thread stopping (processed %d batches) - LOCK-FREE\n", 
           ctx->sphere_id, batches_processed);
    
    return NULL;
}

/**
 * Worker thread function - PHASE 3: Barrier Synchronization (LEGACY)
 * 
 * NOTE: This function is kept for comparison but not used in Phase 2B
 * Phase 2B uses sphere_worker_thread_lockfree() instead
 */
static void* sphere_worker_thread(void* arg) {
    SphereTrainingContext* ctx = (SphereTrainingContext*)arg;
    ThreadedTrainingSystem* system = ctx->system;
    
    printf("[Worker %d] Thread started (symmetry group %d)\n", 
           ctx->sphere_id, ctx->symmetry_group);
    
    int batches_processed = 0;
    
    while (1) {
        // POINT A: Wait for batch assignment from main thread
        pthread_barrier_wait(&system->batch_barrier);
        
        // Check if we should stop
       // Check if we should stop (AFTER barrier to avoid deadlock)
       if (!atomic_load(&system->running)) {
           // Must still participate in Point B barrier before exiting
           pthread_barrier_wait(&system->batch_barrier);
           break;
       }
        
        // PHASE 6: Control threads NEVER process batches
        // Only leaf workers (no children) process batches
        if (ctx->current_batch && !ctx->is_control_thread) {
            sphere_process_batch(ctx, system->training);
            batches_processed++;
        } else if (ctx->is_control_thread) {
            // Control thread: coordinate children instead
            // Children will process batches at their level
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
 * PHASE 4: Lock-free gradient accumulation
 * Control thread reads all worker segments at barrier (safe - workers are waiting)
 */
static void accumulate_gradients_lockfree(ThreadedTrainingSystem* system) {
    if (!system || !system->accumulated_gradients) return;
    
    // Workers are waiting at barrier - safe to read their segments
    // Zero the accumulated gradients first
    memset(system->accumulated_gradients, 0, system->gradient_size * sizeof(float));
    
    int valid_workers = 0;
    
    // Sum gradients from all worker segments
    for (int i = 0; i < system->num_worker_spheres; i++) {
        SphereTrainingContext* ctx = system->sphere_contexts[i];
        if (!ctx || !ctx->local_gradients) continue;
        
        // Validate gradients before accumulation
        char source[64];
        snprintf(source, sizeof(source), "Worker %d", i);
        if (!validate_gradients(ctx->local_gradients, ctx->gradient_size, source)) {
            fprintf(stderr, "WARNING: Skipping invalid gradients from worker %d\n", i);
            continue;
        }
        
        // Clip gradients to prevent overflow
        clip_gradients(ctx->local_gradients, ctx->gradient_size, 10.0f);
        
        // OBJECTIVE 6: Use SIMD-optimized gradient accumulation
        // Accumulate from this worker's segment
        cllm_simd_accumulate_gradients(
            system->accumulated_gradients,
            ctx->local_gradients,
            system->gradient_size
        );
        
        valid_workers++;
    }
    
    // OBJECTIVE 6: Use SIMD-optimized gradient scaling
    // Average the gradients
    if (valid_workers > 0) {
        float scale = 1.0f / valid_workers;
        cllm_simd_scale_gradients(
            system->accumulated_gradients,
            scale,
            system->gradient_size
        );
    }
}

/**
 * Validate gradients for NaN/Inf values
 */
static int validate_gradients(float* gradients, size_t size, const char* source) {
    int nan_count = 0;
    int inf_count = 0;
    
    for (size_t i = 0; i < size; i++) {
        if (prime_isnanf(gradients[i])) {
            nan_count++;
            if (nan_count <= 5) {  // Only log first 5
                fprintf(stderr, "ERROR: NaN gradient in %s at index %zu\n", source, i);
            }
        } else if (prime_isinff(gradients[i])) {
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
    norm = prime_sqrtf(norm);
    
    if (norm > max_norm) {
        float scale = max_norm / norm;
        for (size_t i = 0; i < size; i++) {
            gradients[i] *= scale;
        }
        printf("  Clipped gradients: norm %.4f -> %.4f\n", norm, max_norm);
    }
}

/**
 * PHASE 6: Spawn child threads for recursive hierarchy
 * 
 * When a worker thread spawns children, it transitions to a control thread.
 * Control threads NEVER process batches - only coordinate children.
 */
   __attribute__((unused))
static int sphere_spawn_children(SphereTrainingContext* parent, int num_children) {
    if (!parent || num_children <= 0 || num_children > 12) return -1;
    
    // Transition parent to control thread
    parent->is_control_thread = 1;
    
    // Allocate children array
    parent->children = (SphereTrainingContext**)calloc(num_children, sizeof(SphereTrainingContext*));
    if (!parent->children) {
        parent->is_control_thread = 0;  // Revert
        return -1;
    }
    
    parent->num_children = num_children;
    
    // Create child contexts
    for (int i = 0; i < num_children; i++) {
        int child_symmetry_group = i;  // 0-11
        int child_id = atomic_fetch_add(&parent->system->sphere_id_counter, 1);
        
        parent->children[i] = sphere_context_create(
            child_id,
            child_symmetry_group,
            parent->gradient_size,
            parent->shared_gradients,
            parent->system->num_worker_spheres
        );
        
        if (!parent->children[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                sphere_context_free(parent->children[j]);
            }
            free(parent->children);
            parent->children = NULL;
            parent->num_children = 0;
            parent->is_control_thread = 0;
            return -1;
        }
        
        // Set child relationships
        parent->children[i]->parent = parent;
        parent->children[i]->hierarchy_level = parent->hierarchy_level + 1;
        parent->children[i]->system = parent->system;
        
        // Create hierarchy node for child
        int child_symmetry_groups[1] = {child_symmetry_group};
        parent->children[i]->hierarchy_node = lattice_hierarchy_create(
            child_id,
            parent->hierarchy_level + 1,
            child_symmetry_groups,
            1,
            -1,  // physical_thread_id (not assigned yet)
            parent->hierarchy_node
        );
        
        // Start child thread
        // OPTIMIZATION: Use 1MB stack for child threads
        pthread_attr_t child_attr;
        pthread_attr_init(&child_attr);
        pthread_attr_setstacksize(&child_attr, 1024 * 1024);
        pthread_create(&parent->children[i]->thread, &child_attr, 
                      sphere_worker_thread, parent->children[i]);
        pthread_attr_destroy(&child_attr);
    }
    
    printf("[Sphere %d] Spawned %d children, transitioned to control thread\n", 
           parent->sphere_id, num_children);
    
    return 0;
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
/**
 * PHASE 2B: Lock-Free Training Epoch
 * 
 * Workers pull batches from work queue (no barriers!)
 * Main thread pushes batches and waits for completion
 */
float threaded_train_epoch_lockfree(ThreadedTrainingSystem* system) {
    if (!system) return 0.0f;
    
    printf("\n=== PHASE 2B: LOCK-FREE TRAINING EPOCH ===\n");
    printf("Using %d worker threads (lock-free work queue)\n", system->num_worker_spheres);
    
    // UI Integration: Update framework status
    if (system->metrics) {
        cllm_metrics_update_framework_status(system->metrics, 1, 1, 1, 1);  // All active
    }
    
    // Reset work queue for new epoch
    work_queue_reset(system->work_queue);
    
    // PHASE 2A: Reset batch iterator and start pre-fetching
    cllm_batch_iterator_reset(system->batch_iterator);
    
    if (!batch_queue_start_prefetch(system)) {
        fprintf(stderr, "ERROR: Failed to start batch pre-fetching\n");
        return 0.0f;
    }
    
    printf("Batch pre-fetching enabled + Lock-free work queue active\n\n");
    
    int batches_pushed = 0;
    size_t total_batches_in_epoch = cllm_batch_iterator_num_batches(system->batch_iterator);
    
    // UI Integration: Initialize epoch metrics
    if (system->metrics) {
        cllm_metrics_update_training_progress(system->metrics, 0, 0, total_batches_in_epoch);
    }
    
    // Push all batches to work queue
    printf("Pushing batches to work queue...\n");
    while (1) {
        // Get batch from pre-fetch queue
        CLLMBatch* batch = batch_queue_try_pop(system->batch_queue);
        
        if (!batch) {
            // Check if producer is done
            if (atomic_load(&system->batch_queue->producer_done)) {
                break;  // No more batches
            }
            
            // Queue temporarily empty - wait for pre-fetch
            usleep(100);
            continue;
        }
        
        // Push to work queue (workers will pull from here)
        while (!work_queue_push(system->work_queue, batch)) {
            // Work queue full - wait for workers to catch up
            usleep(10);
        }
        
        batches_pushed++;
        
        // UI Integration: Update step progress
        if (system->metrics) {
            cllm_metrics_update_training_progress(system->metrics, 0, batches_pushed, total_batches_in_epoch);
        }
        
        // Log progress
        if (batches_pushed % 500 == 0) {
            size_t pending, pushed, popped;
            work_queue_stats(system->work_queue, &pending, &pushed, &popped);
            printf("  Pushed %d batches (pending: %zu, processed: %zu)\n", 
                   batches_pushed, pending, popped);
        }
    }
    
    printf("All %d batches pushed to work queue\n", batches_pushed);
    
    // Signal epoch done
    atomic_store(&system->work_queue->epoch_done, 1);
    
    // Wait for all work to complete
    printf("Waiting for workers to complete...\n");
    int wait_iterations = 0;
    while (!work_queue_is_complete(system->work_queue)) {
        usleep(1000);  // 1ms
        
        wait_iterations++;
        if (wait_iterations % 1000 == 0) {
            size_t pending, pushed, popped;
            work_queue_stats(system->work_queue, &pending, &pushed, &popped);
            printf("  Still working... (pushed: %zu, processed: %zu, pending: %zu)\n",
                   pushed, popped, pending);
        }
    }
    
    printf("All batches processed!\n");
    
    // Stop pre-fetch thread
    batch_queue_stop_prefetch(system);
    
    // Accumulate gradients from all workers
    printf("Accumulating gradients...\n");
    accumulate_gradients_lockfree(system);
    
    // Copy accumulated gradients to training object
    memcpy(system->training->gradients, system->accumulated_gradients, 
           system->gradient_size * sizeof(float));
    
    // Apply gradients using Adam optimizer
    printf("Applying optimizer step...\n");
    cllm_optimizer_step_adam(system->training);
    
    // Calculate average loss
    float epoch_loss = 0.0f;
    int valid_workers = 0;
    for (int i = 0; i < system->num_worker_spheres; i++) {
        if (system->sphere_contexts[i]->batches_processed > 0) {
            epoch_loss += system->sphere_contexts[i]->batch_loss;
            valid_workers++;
        }
    }
    
    float avg_loss = (valid_workers > 0) ? epoch_loss / valid_workers : 0.0f;
    
    // UI Integration: Update final loss and invoke callbacks
    if (system->metrics) {
        cllm_metrics_update_loss(system->metrics, avg_loss);
        cllm_metrics_invoke_callbacks(system->metrics);
    }
    
    printf("\nEpoch complete (LOCK-FREE):\n");
    printf("  Total batches: %d\n", batches_pushed);
    printf("  Average loss: %.4f\n", avg_loss);
    printf("  Workers active: %d\n", valid_workers);
    
    return avg_loss;
}

/**
 * LEGACY: Barrier-based training epoch (Phase 2A only)
 * NOTE: This is kept for comparison but Phase 2B uses threaded_train_epoch_lockfree()
 */
float threaded_train_epoch(ThreadedTrainingSystem* system) {
    if (!system) return 0.0f;
    
    printf("\nStarting multi-threaded epoch training...\n");
    printf("Using %d worker threads for parallel batch processing\n", system->num_worker_spheres);
    
    // PHASE 2A: Reset batch iterator and start pre-fetching
    cllm_batch_iterator_reset(system->batch_iterator);
    
    if (!batch_queue_start_prefetch(system)) {
        fprintf(stderr, "ERROR: Failed to start batch pre-fetching\n");
        return 0.0f;
    }
    
    printf("PHASE 2A: Batch pre-fetching enabled (overlapping I/O with computation)\n\n");
    
    float epoch_loss = 0.0f;
    int batch_count = 0;
    
    // Process batches in groups (one per sphere)
    int total_batch_groups = 0;
    size_t total_batches_in_epoch = cllm_batch_iterator_num_batches(system->batch_iterator);
    
    while (1) {
        CLLMBatch** batches = (CLLMBatch**)calloc(system->num_worker_spheres, sizeof(CLLMBatch*));
        if (!batches) {
            break;
        }
        
        int batches_loaded = 0;
        
        // PHASE 2A: Load batches from pre-fetch queue (non-blocking)
        for (int i = 0; i < system->num_worker_spheres; i++) {
            // Try to get batch from queue
            batches[i] = batch_queue_try_pop(system->batch_queue);
            
            if (batches[i]) {
                batches_loaded++;
            } else {
                // Queue empty - check if producer is done
                if (atomic_load(&system->batch_queue->producer_done)) {
                    // No more batches will be produced
                    break;
                }
                
                // Queue temporarily empty - wait a bit for pre-fetch to catch up
                usleep(100);  // 100 microseconds
                
                // Try again
                batches[i] = batch_queue_try_pop(system->batch_queue);
                if (batches[i]) {
                    batches_loaded++;
                } else {
                    // Still empty and producer done - no more batches
                    if (atomic_load(&system->batch_queue->producer_done)) {
                        break;
                    }
                }
            }
        }
        
        // If no batches loaded, we're done with the epoch
        if (batches_loaded == 0) {
            free(batches);
            break;
        }
        
        // Debug: Show actual batch count
        if (total_batch_groups == 0) {
            printf("First batch group: loaded %d batches from pre-fetch queue\n", batches_loaded);
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
        
        // PHASE 4: Lock-free gradient accumulation
        // Control thread has already accumulated gradients at barrier
        // Safe to read and apply without locking
        
        // Copy accumulated gradients to training object
        memcpy(system->training->gradients, system->accumulated_gradients, 
               system->gradient_size * sizeof(float));
        
        // Apply gradients using Adam optimizer
        cllm_optimizer_step_adam(system->training);
    }
    // PHASE 2A: Stop pre-fetch thread
    batch_queue_stop_prefetch(system);
    printf("\nPre-fetch thread stopped\n");
    
    // CRITICAL FIX: Signal threads to stop by setting running=0 and releasing barriers
    printf("Epoch complete - signaling threads to stop...\n");
    atomic_store(&system->running, 0);
    
    // Release threads from barriers by participating one more time
    // This allows them to check the running flag and exit cleanly
    pthread_barrier_wait(&system->batch_barrier);  // Release Point A
    pthread_barrier_wait(&system->batch_barrier);  // Release Point B

    
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
    
       // PHASE 4: Lock-free - safe to read after barrier synchronization
    float norm = 0.0f;
    for (size_t i = 0; i < system->gradient_size; i++) {
        float val = system->accumulated_gradients[i];
        norm += val * val;
    }
    norm = prime_sqrtf(norm);
    
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

/**
 * Get metrics from training system
 * 
 * UI Integration: Returns the metrics object for real-time monitoring
 */
struct CLLMMetrics* threaded_training_get_metrics(ThreadedTrainingSystem* system) {
    if (!system) return NULL;
    return system->metrics;
}
