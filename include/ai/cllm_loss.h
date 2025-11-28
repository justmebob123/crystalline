#ifndef CLLM_LOSS_H
#define CLLM_LOSS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "ai/cllm_batch.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Loss Function Types
 */
typedef enum {
    LOSS_CROSS_ENTROPY,      // Cross-entropy loss for classification
    LOSS_MSE,                // Mean Squared Error
    LOSS_MAE,                // Mean Absolute Error
    LOSS_HUBER,              // Huber loss (robust to outliers)
    LOSS_KL_DIVERGENCE,      // KL divergence
    LOSS_BINARY_CROSS_ENTROPY, // Binary cross-entropy
    LOSS_CUSTOM              // Custom loss function
} LossType;

/**
 * Loss Reduction Strategy
 */
typedef enum {
    LOSS_REDUCTION_MEAN,     // Average loss over batch
    LOSS_REDUCTION_SUM,      // Sum of losses
    LOSS_REDUCTION_NONE      // No reduction (return per-sample losses)
} LossReduction;

/**
 * Loss Configuration
 */
typedef struct {
    LossType type;
    LossReduction reduction;
    float label_smoothing;   // Label smoothing factor (0.0 = no smoothing)
    float huber_delta;       // Delta parameter for Huber loss
    bool ignore_index;       // Whether to ignore a specific index
    int ignore_index_value;  // Index to ignore (e.g., padding token)
    bool use_log_softmax;    // Use log-softmax for numerical stability
} LossConfig;

/**
 * Loss Computation Context
 */
typedef struct {
    LossConfig config;
    
    // Statistics
    float total_loss;
    size_t num_samples;
    size_t num_batches;
    float min_loss;
    float max_loss;
    float avg_loss;
    
    // Gradient statistics
    float gradient_norm;
    float gradient_max;
    float gradient_min;
    
    // Numerical stability
    float epsilon;           // Small constant for numerical stability
    bool check_nan;          // Check for NaN/Inf in loss
    bool check_gradients;    // Check for NaN/Inf in gradients
    
    // Gradient clipping
    bool clip_gradients;
    float clip_value;
    float clip_norm;
} LossComputation;

/**
 * Loss Result
 */
typedef struct {
    float loss_value;        // Scalar loss value
    Tensor* per_sample_loss; // Per-sample losses (if reduction=NONE)
    Tensor* gradients;       // Gradients w.r.t. predictions
    bool has_nan;            // Whether NaN was detected
    bool has_inf;            // Whether Inf was detected
} LossResult;

// ============================================================================
// Loss Computation Functions
// ============================================================================

/**
 * Create loss computation context
 */
LossComputation* loss_computation_create(const LossConfig* config);

/**
 * Free loss computation context
 */
void loss_computation_free(LossComputation* loss_comp);

/**
 * Reset loss statistics
 */
void loss_computation_reset_stats(LossComputation* loss_comp);

/**
 * Compute loss (forward pass)
 * 
 * @param loss_comp Loss computation context
 * @param predictions Predicted values [batch_size, num_classes] or [batch_size, seq_len, num_classes]
 * @param targets Target values (same shape as predictions or [batch_size] for class indices)
 * @param mask Optional mask [batch_size] or [batch_size, seq_len] (NULL if no masking)
 * @return Loss result (caller must free)
 */
LossResult* loss_compute_forward(
    LossComputation* loss_comp,
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask
);

/**
 * Compute gradients (backward pass)
 * 
 * @param loss_comp Loss computation context
 * @param predictions Predicted values
 * @param targets Target values
 * @param mask Optional mask
 * @return Gradients w.r.t. predictions (caller must free)
 */
Tensor* loss_compute_backward(
    LossComputation* loss_comp,
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask
);

/**
 * Free loss result
 */
void loss_result_free(LossResult* result);

// ============================================================================
// Specific Loss Functions
// ============================================================================

/**
 * Cross-Entropy Loss
 * 
 * For classification tasks. Expects:
 * - predictions: [batch_size, num_classes] (logits, not probabilities)
 * - targets: [batch_size] (class indices) or [batch_size, num_classes] (one-hot)
 */
float loss_cross_entropy_forward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    LossReduction reduction,
    float label_smoothing,
    float epsilon,
    Tensor** per_sample_loss
);

Tensor* loss_cross_entropy_backward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    float label_smoothing,
    float epsilon
);

/**
 * Mean Squared Error Loss
 * 
 * For regression tasks. Expects:
 * - predictions: [batch_size, ...] (any shape)
 * - targets: same shape as predictions
 */
float loss_mse_forward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    LossReduction reduction,
    Tensor** per_sample_loss
);

Tensor* loss_mse_backward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask
);

/**
 * Mean Absolute Error Loss
 * 
 * For regression tasks. Expects:
 * - predictions: [batch_size, ...] (any shape)
 * - targets: same shape as predictions
 */
float loss_mae_forward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    LossReduction reduction,
    Tensor** per_sample_loss
);

Tensor* loss_mae_backward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask
);

/**
 * Huber Loss
 * 
 * Robust loss for regression. Expects:
 * - predictions: [batch_size, ...] (any shape)
 * - targets: same shape as predictions
 * - delta: threshold for switching between L1 and L2
 */
float loss_huber_forward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    float delta,
    LossReduction reduction,
    Tensor** per_sample_loss
);

Tensor* loss_huber_backward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    float delta
);

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Apply log-softmax for numerical stability
 * 
 * @param logits Input logits [batch_size, num_classes]
 * @return Log probabilities (caller must free)
 */
Tensor* loss_log_softmax(const Tensor* logits);

/**
 * Apply softmax
 * 
 * @param logits Input logits [batch_size, num_classes]
 * @return Probabilities (caller must free)
 */
Tensor* loss_softmax(const Tensor* logits);

/**
 * Compute log-sum-exp trick for numerical stability
 * 
 * @param values Input values
 * @param size Number of values
 * @return log(sum(exp(values)))
 */
float loss_log_sum_exp(const float* values, size_t size);

/**
 * Apply label smoothing
 * 
 * @param targets One-hot targets [batch_size, num_classes]
 * @param smoothing Smoothing factor (0.0 = no smoothing, 0.1 = 10% smoothing)
 * @return Smoothed targets (caller must free)
 */
Tensor* loss_apply_label_smoothing(const Tensor* targets, float smoothing);

/**
 * Check for NaN or Inf in tensor
 * 
 * @param tensor Tensor to check
 * @param has_nan Output: whether NaN was found
 * @param has_inf Output: whether Inf was found
 * @return true if tensor is valid (no NaN/Inf)
 */
bool loss_check_numerical_stability(const Tensor* tensor, bool* has_nan, bool* has_inf);

/**
 * Clip gradients by value
 * 
 * @param gradients Gradients to clip (modified in-place)
 * @param clip_value Maximum absolute value
 */
void loss_clip_gradients_by_value(Tensor* gradients, float clip_value);

/**
 * Clip gradients by norm
 * 
 * @param gradients Gradients to clip (modified in-place)
 * @param max_norm Maximum norm
 * @return Actual gradient norm before clipping
 */
float loss_clip_gradients_by_norm(Tensor* gradients, float max_norm);

/**
 * Compute gradient norm
 * 
 * @param gradients Gradients tensor
 * @return L2 norm of gradients
 */
float loss_compute_gradient_norm(const Tensor* gradients);

/**
 * Print loss statistics
 */
void loss_computation_print_stats(const LossComputation* loss_comp);

/**
 * Get loss statistics
 */
void loss_computation_get_stats(
    const LossComputation* loss_comp,
    float* total_loss,
    size_t* num_samples,
    float* avg_loss,
    float* gradient_norm
);

// ============================================================================
// Simple Cross-Entropy Loss (for direct use without Tensor API)
// ============================================================================

/**
 * Compute cross-entropy loss for a single prediction
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target Target token ID
 * @param vocab_size Vocabulary size
 * @return Cross-entropy loss value
 */
float cllm_compute_cross_entropy_loss(float* logits, uint32_t target, int vocab_size);

/**
 * Compute cross-entropy loss gradient
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target Target token ID
 * @param vocab_size Vocabulary size
 * @param grad_output Output gradients [vocab_size] (caller must allocate)
 */
void cllm_compute_loss_gradient(float* logits, uint32_t target, 
                                int vocab_size, float* grad_output);

#ifdef __cplusplus
}
#endif

#endif // CLLM_LOSS_H