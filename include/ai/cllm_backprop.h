#ifndef CLLM_BACKPROP_H
#define CLLM_BACKPROP_H

#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include "cllm_batch.h"
#include "cllm_loss.h"
#include "cllm_lattice_hierarchy.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Gradient Accumulation Strategy
 */
typedef enum {
    GRADIENT_ACCUMULATION_IMMEDIATE,  // Accumulate immediately after each batch
    GRADIENT_ACCUMULATION_DEFERRED,   // Accumulate at end of epoch
    GRADIENT_ACCUMULATION_HIERARCHICAL // Hierarchical tree reduction
} GradientAccumulationStrategy;

/**
 * Gradient Buffer
 * 
 * Stores gradients for a single sphere
 */
typedef struct {
    float* data;              // Gradient data
    size_t size;              // Number of elements
    size_t capacity;          // Allocated capacity
    
    // Metadata
    int sphere_id;
    int symmetry_group;
    size_t batch_count;       // Number of batches accumulated
    
    // Statistics
    float norm;               // L2 norm of gradients
    float max_value;
    float min_value;
    float mean_value;
    
    // Synchronization
    pthread_mutex_t mutex;
    bool is_ready;            // Ready for accumulation
} GradientBuffer;

/**
 * Hierarchical Gradient Context
 * 
 * Manages gradient computation and accumulation across the hierarchy
 */
typedef struct {
    // Configuration
    GradientAccumulationStrategy strategy;
    bool use_gradient_clipping;
    float clip_value;
    float clip_norm;
    
    // Gradient buffers
    GradientBuffer* local_gradients;      // This sphere's gradients
    GradientBuffer** child_gradients;     // Children's gradients (12 max)
    size_t num_children;
    
    // Accumulation state
    size_t batches_processed;
    size_t batches_per_accumulation;
    bool accumulation_complete;
    
    // Synchronization
    pthread_barrier_t* accumulation_barrier;
    pthread_mutex_t state_mutex;
    
    // Statistics
    float total_gradient_norm;
    size_t gradient_updates;
    double accumulation_time;
    
    // Numerical stability
    bool check_gradients;
    size_t nan_count;
    size_t inf_count;
} BackpropContext;

/**
 * Backpropagation Result
 */
typedef struct {
    GradientBuffer* gradients;
    float loss_value;
    bool has_nan;
    bool has_inf;
    double compute_time;
} BackpropResult;

// ============================================================================
// Gradient Buffer Functions
// ============================================================================

/**
 * Create gradient buffer
 */
GradientBuffer* gradient_buffer_create(size_t size, int sphere_id, int symmetry_group);

/**
 * Free gradient buffer
 */
void gradient_buffer_free(GradientBuffer* buffer);

/**
 * Zero out gradient buffer
 */
void gradient_buffer_zero(GradientBuffer* buffer);

/**
 * Copy gradient buffer
 */
GradientBuffer* gradient_buffer_copy(const GradientBuffer* buffer);

/**
 * Add gradients to buffer (accumulation)
 */
void gradient_buffer_add(GradientBuffer* dest, const GradientBuffer* src);

/**
 * Scale gradient buffer by constant
 */
void gradient_buffer_scale(GradientBuffer* buffer, float scale);

/**
 * Compute gradient statistics
 */
void gradient_buffer_compute_stats(GradientBuffer* buffer);

/**
 * Check for NaN/Inf in gradients
 */
bool gradient_buffer_check_stability(const GradientBuffer* buffer, bool* has_nan, bool* has_inf);

/**
 * Clip gradients by value
 */
void gradient_buffer_clip_by_value(GradientBuffer* buffer, float clip_value);

/**
 * Clip gradients by norm
 */
float gradient_buffer_clip_by_norm(GradientBuffer* buffer, float max_norm);

/**
 * Print gradient buffer info
 */
void gradient_buffer_print(const GradientBuffer* buffer);

// ============================================================================
// Backpropagation Context Functions
// ============================================================================

/**
 * Create backpropagation context
 */
BackpropContext* backprop_create(
    size_t gradient_size,
    int sphere_id,
    int symmetry_group,
    GradientAccumulationStrategy strategy
);

/**
 * Free backpropagation context
 */
void backprop_free(BackpropContext* ctx);

/**
 * Reset backpropagation context for new epoch
 */
void backprop_reset(BackpropContext* ctx);

/**
 * Register child gradient buffer
 */
bool backprop_register_child(BackpropContext* ctx, GradientBuffer* child_buffer);

/**
 * Unregister child gradient buffer
 */
void backprop_unregister_child(BackpropContext* ctx, int child_id);

// ============================================================================
// Gradient Computation Functions
// ============================================================================

/**
 * Compute gradients for a single batch
 * 
 * @param ctx Backpropagation context
 * @param loss_comp Loss computation context
 * @param predictions Model predictions
 * @param targets Target values
 * @param mask Optional mask
 * @return Backpropagation result (caller must free)
 */
BackpropResult* backprop_compute_batch(
    BackpropContext* ctx,
    LossComputation* loss_comp,
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask
);

/**
 * Accumulate gradients from a batch into local buffer
 * 
 * @param ctx Backpropagation context
 * @param batch_gradients Gradients from current batch
 */
void backprop_accumulate_batch(BackpropContext* ctx, const GradientBuffer* batch_gradients);

/**
 * Accumulate gradients from children (hierarchical reduction)
 * 
 * @param ctx Backpropagation context
 * @return true if accumulation successful
 */
bool backprop_accumulate_from_children(BackpropContext* ctx);

/**
 * Average accumulated gradients
 * 
 * @param ctx Backpropagation context
 * @param num_batches Number of batches accumulated
 */
void backprop_average_gradients(BackpropContext* ctx, size_t num_batches);

/**
 * Finalize gradients (clip, check stability, compute stats)
 * 
 * @param ctx Backpropagation context
 * @return true if gradients are valid
 */
bool backprop_finalize_gradients(BackpropContext* ctx);

// ============================================================================
// Hierarchical Gradient Flow Functions
// ============================================================================

/**
 * Propagate gradients up the hierarchy
 * 
 * @param sphere Current sphere
 * @param ctx Backpropagation context
 * @return true if propagation successful
 */
bool backprop_propagate_to_parent(
    CLLMLatticeHierarchy* sphere,
    BackpropContext* ctx
);

/**
 * Broadcast gradients down the hierarchy
 * 
 * @param sphere Current sphere
 * @param ctx Backpropagation context
 * @return true if broadcast successful
 */
bool backprop_broadcast_to_children(
    CLLMLatticeHierarchy* sphere,
    BackpropContext* ctx
);

/**
 * Synchronize gradients across siblings
 * 
 * @param sphere Current sphere
 * @param ctx Backpropagation context
 * @return true if synchronization successful
 */
bool backprop_synchronize_siblings(
    CLLMLatticeHierarchy* sphere,
    BackpropContext* ctx
);

/**
 * Perform tree reduction of gradients
 * 
 * @param root Root sphere
 * @param ctx Backpropagation context
 * @return true if reduction successful
 */
bool backprop_tree_reduction(
    CLLMLatticeHierarchy* root,
    BackpropContext* ctx
);

// ============================================================================
// Gradient Verification Functions
// ============================================================================

/**
 * Verify gradient correctness (numerical gradient check)
 * 
 * @param ctx Backpropagation context
 * @param loss_comp Loss computation context
 * @param predictions Model predictions
 * @param targets Target values
 * @param epsilon Perturbation size
 * @return Maximum relative error
 */
float backprop_verify_gradients(
    BackpropContext* ctx,
    LossComputation* loss_comp,
    const Tensor* predictions,
    const Tensor* targets,
    float epsilon
);

/**
 * Check gradient flow through hierarchy
 * 
 * @param root Root sphere
 * @return true if gradient flow is correct
 */
bool backprop_check_gradient_flow(CLLMLatticeHierarchy* root);

// ============================================================================
// Statistics and Monitoring Functions
// ============================================================================

/**
 * Get backpropagation statistics
 */
void backprop_get_stats(
    const BackpropContext* ctx,
    float* gradient_norm,
    size_t* gradient_updates,
    double* accumulation_time,
    size_t* nan_count,
    size_t* inf_count
);

/**
 * Print backpropagation statistics
 */
void backprop_print_stats(const BackpropContext* ctx);

/**
 * Free backpropagation result
 */
void backprop_result_free(BackpropResult* result);

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Convert tensor to gradient buffer
 */
GradientBuffer* backprop_tensor_to_buffer(const Tensor* tensor, int sphere_id, int symmetry_group);

/**
 * Convert gradient buffer to tensor
 */
Tensor* backprop_buffer_to_tensor(const GradientBuffer* buffer, const size_t* shape, size_t ndim);

/**
 * Merge multiple gradient buffers
 */
GradientBuffer* backprop_merge_buffers(GradientBuffer** buffers, size_t num_buffers);

/**
 * Split gradient buffer into multiple buffers
 */
bool backprop_split_buffer(
    const GradientBuffer* source,
    GradientBuffer** destinations,
    size_t num_destinations
);

#ifdef __cplusplus
}
#endif

#endif // CLLM_BACKPROP_H