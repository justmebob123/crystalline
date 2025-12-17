#ifndef CLLM_TRAINING_88D_H
#define CLLM_TRAINING_88D_H

/**
 * @file cllm_training_88d.h
 * @brief CLLM Training System using 88D Hierarchical Threading
 * 
 * This is the NEW training system that fully integrates with the 88D
 * hierarchical threading infrastructure from the algorithms library.
 * 
 * Key Features:
 * - Uses HierarchicalThreadPool for all threading
 * - Uses SharedMemoryEnhanced for gradient accumulation
 * - Uses MessageSystem for coordination
 * - Uses WorkDistribution for load balancing
 * - Respects 12-fold symmetry throughout
 * - Lock-free gradient accumulation
 * - Automatic work stealing
 * 
 * This replaces the legacy cllm_training_threaded.c system.
 */

#include "cllm.h"
#include "cllm_training.h"
#include "../../algorithms/include/hierarchical_threading.h"
#include "../../algorithms/include/shared_memory_enhanced.h"
#include "../../algorithms/include/message_passing.h"
#include "../../algorithms/include/work_distribution.h"
#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct ThreadLocalTrainingContext ThreadLocalTrainingContext;

// Include cllm_batch.h to get CLLMBatch and CLLMBatchIterator definitions
#include "cllm_batch.h"

/**
 * Thread-Local Training Context
 * 
 * Each worker thread gets its own activation buffers to avoid race conditions.
 * Model weights are shared (read-only), but activations are thread-local.
 */
struct ThreadLocalTrainingContext {
    // Forward pass activation storage (thread-local)
    double* input_embeddings;        // [batch * seq * embed]
    double** layer_inputs;           // [num_layers][batch * seq * embed]
    double** attention_outputs;      // [num_layers][batch * seq * embed]
    double** ff_outputs;             // [num_layers][batch * seq * embed]
    double** layer_outputs;          // [num_layers][batch * seq * embed]
    double** ff_hidden;              // [num_layers][batch * seq * ff_hidden]
    double* final_hidden;            // [batch * seq * embed]
    double* logits;                  // [batch * seq * vocab]
    
    // Attention cache (thread-local)
    struct {
        double* attention_weights;   // [num_heads * seq * seq]
        double* queries;             // [seq * embed]
        double* keys;                // [seq * embed]
        double* values;              // [seq * embed]
        double* scores;              // [num_heads * seq * seq]
    }* attention_cache;              // [num_layers]
    
    // Backward pass temporary buffers (thread-local)
    double* grad_logits;             // [batch * seq * vocab]
    double* grad_hidden;             // [batch * seq * embed]
    double* grad_layer;              // [batch * seq * embed]
    
    // Configuration (copied from main training)
    int batch_size;
    int seq_len;
    int num_layers;
    int embed_dim;
    int vocab_size;
    int ff_hidden_dim;
    int num_heads;
};

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * Batch processing context (passed to worker threads)
 */
typedef struct {
    CLLMBatch* batch;
    uint64_t batch_id;
    double loss;
    int valid_sequences;
    struct CLLMTraining88D* training_ctx;  // Reference to training context
    int thread_id;  // ID of thread that will process this work
} BatchWorkItem;

/**
 * Training context for 88D system
 * 
 * This structure manages the entire training process using the 88D
 * hierarchical threading infrastructure.
 */
typedef struct CLLMTraining88D {
    // Core components
    CLLMModel* model;
    CLLMTraining* training;
    CLLMBatchIterator* batch_iterator;
    
    // 88D Infrastructure
    HierarchicalThreadPool* thread_pool;
    SharedMemoryEnhanced* gradient_memory;
    MessageSystem* message_system;
    WorkPool* work_pool;
    
    // Thread-local contexts (one per thread)
    ThreadLocalTrainingContext** thread_contexts;
    uint32_t num_thread_contexts;
    
    // Gradient accumulation
    double* accumulated_gradients;
    size_t gradient_size;
    
    // Statistics
    double epoch_loss;
    uint64_t batches_processed;
    uint64_t total_batches;
    uint64_t total_sequences_processed;
    
    // Timing
    double epoch_start_time;
    double epoch_end_time;
    double total_training_time;
    
    // Configuration
    uint32_t num_threads;
    uint32_t batch_size;
    uint32_t num_levels;
    bool use_ntt_attention;
    bool use_work_stealing;
    
    // State
    volatile bool training_active;
    volatile bool epoch_complete;
    
} CLLMTraining88D;

// ============================================================================
// API FUNCTIONS
// ============================================================================

/**
 * Create 88D training context
 * 
 * @param model CLLM model to train
 * @param training Training configuration
 * @param batch_iterator Batch iterator for data
 * @param num_threads Number of worker threads (will be adjusted for 12-fold symmetry)
 * @return Training context, or NULL on error
 */
CLLMTraining88D* cllm_training_88d_create(
    CLLMModel* model,
    CLLMTraining* training,
    CLLMBatchIterator* batch_iterator,
    uint32_t num_threads
);

/**
 * Free 88D training context
 * 
 * @param ctx Training context to free
 */
void cllm_training_88d_free(CLLMTraining88D* ctx);

/**
 * Create thread-local training context
 * 
 * @param batch_size Batch size
 * @param seq_len Sequence length
 * @param num_layers Number of layers
 * @param embed_dim Embedding dimension
 * @param vocab_size Vocabulary size
 * @param ff_hidden_dim Feedforward hidden dimension
 * @param num_heads Number of attention heads
 * @return Thread-local context, or NULL on error
 */
ThreadLocalTrainingContext* thread_local_training_create_88d(
    int batch_size,
    int seq_len,
    int num_layers,
    int embed_dim,
    int vocab_size,
    int ff_hidden_dim,
    int num_heads
);

/**
 * Free thread-local training context
 * 
 * @param ctx Thread-local context to free
 */
void thread_local_training_free_88d(ThreadLocalTrainingContext* ctx);

/**
 * Train one epoch using 88D system
 * 
 * This is the main training function. It:
 * 1. Resets epoch state
 * 2. Creates work items for all batches
 * 3. Submits work to thread pool
 * 4. Waits for completion
 * 5. Synchronizes gradients
 * 6. Applies optimizer step
 * 7. Returns epoch loss
 * 
 * @param ctx Training context
 * @param epoch_num Current epoch number (for logging)
 * @return Average loss for the epoch
 */
double cllm_train_epoch_88d(
    CLLMTraining88D* ctx,
    int epoch_num
);

/**
 * Train multiple epochs
 * 
 * @param ctx Training context
 * @param num_epochs Number of epochs to train
 * @return Final epoch loss
 */
double cllm_train_88d(
    CLLMTraining88D* ctx,
    int num_epochs
);

// ============================================================================
// BATCH PROCESSING (Internal)
// ============================================================================

/**
 * Process a single batch (called by worker threads)
 * 
 * This function is called by the 88D thread pool for each batch.
 * It performs:
 * 1. Forward pass
 * 2. Loss computation
 * 3. Backward pass
 * 4. Gradient writing to shared memory
 * 5. Completion notification
 * 
 * @param item Work item containing batch
 * @param user_data Training context (CLLMTraining88D*)
 */
void cllm_process_batch_88d(
    WorkItem* item,
    void* user_data
);

/**
 * Process a single sequence within a batch
 * 
 * @param ctx Training context
 * @param batch Batch containing the sequence
 * @param seq_idx Sequence index within batch
 * @param thread_id Thread ID for gradient buffer
 * @return Loss for this sequence
 */
double cllm_process_sequence_88d(
    CLLMTraining88D* ctx,
    CLLMBatch* batch,
    uint32_t seq_idx,
    int thread_id
);

/**
 * Work wrapper for batch processing (for hierarchical work submission)
 */
void cllm_process_batch_work_wrapper(void* data);

// ============================================================================
// GRADIENT SYNCHRONIZATION
// ============================================================================

/**
 * Synchronize gradients from all threads
 * 
 * This function:
 * 1. Reads gradients from all SharedMemoryEnhanced boundaries
 * 2. Accumulates them into accumulated_gradients
 * 3. Prepares for optimizer step
 * 
 * @param ctx Training context
 */
void cllm_sync_gradients_88d(CLLMTraining88D* ctx);

/**
 * Zero all gradient buffers
 * 
 * @param ctx Training context
 */
void cllm_zero_gradients_88d(CLLMTraining88D* ctx);

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

/**
 * Print training statistics
 * 
 * @param ctx Training context
 */
void cllm_print_training_stats_88d(const CLLMTraining88D* ctx);

/**
 * Get thread pool statistics
 * 
 * @param ctx Training context
 * @param stats Output statistics structure
 */
void cllm_get_thread_pool_stats_88d(
    const CLLMTraining88D* ctx,
    HierarchicalThreadPoolStats* stats
);

/**
 * Print detailed thread statistics
 * 
 * @param ctx Training context
 */
void cllm_print_thread_stats_88d(const CLLMTraining88D* ctx);

// ============================================================================
// CONFIGURATION
// ============================================================================

/**
 * Enable/disable work stealing
 * 
 * @param ctx Training context
 * @param enable true to enable, false to disable
 */
void cllm_set_work_stealing_88d(CLLMTraining88D* ctx, bool enable);

/**
 * Enable/disable NTT attention
 * 
 * @param ctx Training context
 * @param enable true to enable, false to disable
 */
void cllm_set_ntt_attention_88d(CLLMTraining88D* ctx, bool enable);

/**
 * Set batch size
 * 
 * @param ctx Training context
 * @param batch_size New batch size
 */
void cllm_set_batch_size_88d(CLLMTraining88D* ctx, uint32_t batch_size);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Calculate number of hierarchy levels for given thread count
 * 
 * @param num_threads Number of threads
 * @return Number of levels (1-8)
 */
int cllm_calculate_num_levels_88d(uint32_t num_threads);

/**
 * Calculate gradient size for model
 * 
 * @param model CLLM model
 * @return Total gradient size in doubles
 */
size_t cllm_calculate_gradient_size_88d(const CLLMModel* model);

/**
 * Adjust thread count for 12-fold symmetry
 * 
 * @param requested Number of threads requested
 * @return Adjusted number (multiple of 12 or 12n+1)
 */
uint32_t cllm_adjust_thread_count_88d(uint32_t requested);

#endif // CLLM_TRAINING_88D_H