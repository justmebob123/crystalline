/**
 * CLLM Multi-Threaded Training Header
 * 
 * Multi-threaded training with kissing spheres architecture
 */

#ifndef CLLM_TRAINING_THREADED_H
#define CLLM_TRAINING_THREADED_H

#include "cllm_training.h"
#include "cllm_batch.h"
#include "cllm_metrics.h"

/**
 * Thread-Local Training Context
 * 
 * Each worker thread gets its own activation buffers to avoid race conditions.
 * Model weights are shared (read-only), but activations are thread-local.
 * 
 * Memory per thread: ~6.1 MB
 * For 63 threads: ~386 MB (acceptable overhead)
 */
typedef struct {
    // Forward pass activation storage (thread-local)
    float* input_embeddings;         // [batch * seq * embed]
    float** layer_inputs;            // [num_layers][batch * seq * embed]
    float** attention_outputs;       // [num_layers][batch * seq * embed]
    float** ff_outputs;              // [num_layers][batch * seq * embed]
    float** layer_outputs;           // [num_layers][batch * seq * embed]
    float** ff_hidden;               // [num_layers][batch * seq * ff_hidden]
    float* final_hidden;             // [batch * seq * embed]
    float* logits;                   // [batch * seq * vocab]
    
    // Attention cache (thread-local)
    struct {
        float* attention_weights;    // [num_heads * seq * seq]
        float* queries;              // [seq * embed]
        float* keys;                 // [seq * embed]
        float* values;               // [seq * embed]
        float* scores;               // [num_heads * seq * seq]
    }* attention_cache;              // [num_layers]
    
    // Backward pass temporary buffers (thread-local)
    float* grad_logits;              // [batch * seq * vocab]
    float* grad_hidden;              // [batch * seq * embed]
    float* grad_layer;               // [batch * seq * embed]
    
    // Configuration (copied from main training)
    int batch_size;
    int seq_len;
    int num_layers;
    int embed_dim;
    int vocab_size;
    int ff_hidden_dim;
    int num_heads;
} ThreadLocalTrainingContext;

/**
 * Get next sphere ID for dynamic spawning
 *
 * Helper function for cllm_threads.c to get the next available sphere ID
 * when dynamically spawning children.
 *
 * @param user_data Pointer to ThreadedTrainingSystem (from sphere->user_data)
 * @return Next available sphere ID, or -1 on error
 */
int threaded_training_get_next_sphere_id(void* user_data);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Threaded Training System (opaque)
 */
typedef struct ThreadedTrainingSystem ThreadedTrainingSystem;

/**
 * Create threaded training system
 * 
 * @param training Training context
 * @param batch_iterator Batch iterator for data
 * @param num_threads Number of worker threads (0 = auto-detect from CPU cores)
 * @return Threaded training system or NULL on failure
 */
/**
 * Thread-Local Training Context Functions
 */
ThreadLocalTrainingContext* thread_local_training_create(
    int batch_size,
    int seq_len,
    int num_layers,
    int embed_dim,
    int vocab_size,
    int ff_hidden_dim,
    int num_heads
);

void thread_local_training_free(ThreadLocalTrainingContext* ctx);

/**
 * Threaded Forward/Backward Functions
 * 
 * These functions use thread-local activation buffers instead of shared CLLMTraining buffers.
 * This allows multiple threads to execute forward/backward passes in parallel without locking.
 */
float cllm_forward_training_threaded(
    CLLMTraining* training,
    ThreadLocalTrainingContext* local_ctx,
    uint32_t* input_tokens
);

void cllm_backward_training_threaded(
    CLLMTraining* training,
    ThreadLocalTrainingContext* local_ctx,
    uint32_t* target_tokens,
    float* gradient_buffer
);

/**
 * Threaded Training System Functions
 */
ThreadedTrainingSystem* threaded_training_create(CLLMTraining* training, 
                                                  CLLMBatchIterator* batch_iterator,
                                                  int num_threads);

/**
 * Free threaded training system
 */
void threaded_training_free(ThreadedTrainingSystem* system);

/**
 * Train one epoch with multi-threading
 * 
 * Uses 12 kissing spheres to process batches in parallel
 * 
 * @param system Threaded training system
 * @return Average epoch loss
 */
float threaded_train_epoch(ThreadedTrainingSystem* system);
   
   /**
    * PHASE 2B: Train one epoch with lock-free work queue
    * 
    * @param system Threaded training system
    * @return Average epoch loss
    */
   float threaded_train_epoch_lockfree(ThreadedTrainingSystem* system);

/**
 * Print threading statistics
 */
void threaded_training_print_stats(ThreadedTrainingSystem* system);

/**
 * Get per-sphere statistics
 * 
 * @param system Threaded training system
 * @param sphere_id Sphere ID (0-11)
 * @param batches_processed Output: number of batches processed by this sphere
 * @param avg_loss Output: average loss for this sphere
 * @return 0 on success, -1 on failure
 */
int threaded_training_get_sphere_stats(ThreadedTrainingSystem* system,
                                       int sphere_id,
                                       int* batches_processed,
                                       float* avg_loss);

/**
 * Get total gradient norm
 * 
 * @param system Threaded training system
 * @return Total gradient norm
 */
float threaded_training_get_gradient_norm(ThreadedTrainingSystem* system);

/**
 * Get number of worker spheres
 * 
 * @param system Threaded training system
 * @return Number of worker spheres
 */
int threaded_training_get_num_workers(ThreadedTrainingSystem* system);

/**
 * Get next sphere ID for dynamic spawning
 *
 * Helper function for cllm_threads.c to get the next available sphere ID
 * when dynamically spawning children.
 *
 * @param user_data Pointer to ThreadedTrainingSystem (from sphere->user_data)
 * @return Next available sphere ID, or -1 on error
 */
int threaded_training_get_next_sphere_id(void* user_data);

/**
 * Get metrics from training system
 * 
 * UI Integration: Returns the metrics object for real-time monitoring
 * 
 * @param system Threaded training system
 * @return Pointer to metrics object, or NULL if not available
 */
CLLMMetrics* threaded_training_get_metrics(ThreadedTrainingSystem* system);

#ifdef __cplusplus
}
#endif

#endif // CLLM_TRAINING_THREADED_H