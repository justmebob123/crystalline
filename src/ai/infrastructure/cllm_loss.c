#include "ai/cllm_loss.h"
#include "prime_float_math.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

// ============================================================================
// Helper Functions
// ============================================================================

static inline float safe_log(float x, float epsilon) {
    return prime_logf(prime_fmaxf(x, epsilon));
}

static inline float safe_exp(float x) {
    // Clamp to prevent overflow
    return prime_expf(prime_fminf(prime_fmaxf(x, -88.0f), 88.0f));
}

// ============================================================================
// Loss Computation Context
// ============================================================================

LossComputation* loss_computation_create(const LossConfig* config) {
    if (!config) return NULL;
    
    LossComputation* loss_comp = (LossComputation*)calloc(1, sizeof(LossComputation));
    if (!loss_comp) return NULL;
    
    // Copy configuration
    loss_comp->config = *config;
    
    // Initialize statistics
    loss_comp->total_loss = 0.0f;
    loss_comp->num_samples = 0;
    loss_comp->num_batches = 0;
    loss_comp->min_loss = FLT_MAX;
    loss_comp->max_loss = -FLT_MAX;
    loss_comp->avg_loss = 0.0f;
    
    loss_comp->gradient_norm = 0.0f;
    loss_comp->gradient_max = -FLT_MAX;
    loss_comp->gradient_min = FLT_MAX;
    
    // Numerical stability settings
    loss_comp->epsilon = 1e-7f;
    loss_comp->check_nan = true;
    loss_comp->check_gradients = true;
    
    // Gradient clipping (disabled by default)
    loss_comp->clip_gradients = false;
    loss_comp->clip_value = 1.0f;
    loss_comp->clip_norm = 1.0f;
    
    return loss_comp;
}

void loss_computation_free(LossComputation* loss_comp) {
    if (!loss_comp) return;
    free(loss_comp);
}

void loss_computation_reset_stats(LossComputation* loss_comp) {
    if (!loss_comp) return;
    
    loss_comp->total_loss = 0.0f;
    loss_comp->num_samples = 0;
    loss_comp->num_batches = 0;
    loss_comp->min_loss = FLT_MAX;
    loss_comp->max_loss = -FLT_MAX;
    loss_comp->avg_loss = 0.0f;
    
    loss_comp->gradient_norm = 0.0f;
    loss_comp->gradient_max = -FLT_MAX;
    loss_comp->gradient_min = FLT_MAX;
}

// ============================================================================
// Loss Result
// ============================================================================

void loss_result_free(LossResult* result) {
    if (!result) return;
    
    if (result->per_sample_loss) {
        tensor_free(result->per_sample_loss);
    }
    if (result->gradients) {
        tensor_free(result->gradients);
    }
    
    free(result);
}

// ============================================================================
// Forward Pass
// ============================================================================

LossResult* loss_compute_forward(
    LossComputation* loss_comp,
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask
) {
    if (!loss_comp || !predictions || !targets) return NULL;
    
    LossResult* result = (LossResult*)calloc(1, sizeof(LossResult));
    if (!result) return NULL;
    
    result->has_nan = false;
    result->has_inf = false;
    result->per_sample_loss = NULL;
    result->gradients = NULL;
    
    // Check numerical stability
    if (loss_comp->check_nan) {
        bool has_nan, has_inf;
        if (!loss_check_numerical_stability(predictions, &has_nan, &has_inf)) {
            result->has_nan = has_nan;
            result->has_inf = has_inf;
            result->loss_value = NAN;
            return result;
        }
    }
    
    // Compute loss based on type
    float loss_value = 0.0f;
    Tensor* per_sample_loss = NULL;
    
    switch (loss_comp->config.type) {
        case LOSS_CROSS_ENTROPY:
            loss_value = loss_cross_entropy_forward(
                predictions, targets, mask,
                loss_comp->config.reduction,
                loss_comp->config.label_smoothing,
                loss_comp->epsilon,
                &per_sample_loss
            );
            break;
            
        case LOSS_MSE:
            loss_value = loss_mse_forward(
                predictions, targets, mask,
                loss_comp->config.reduction,
                &per_sample_loss
            );
            break;
            
        case LOSS_MAE:
            loss_value = loss_mae_forward(
                predictions, targets, mask,
                loss_comp->config.reduction,
                &per_sample_loss
            );
            break;
            
        case LOSS_HUBER:
            loss_value = loss_huber_forward(
                predictions, targets, mask,
                loss_comp->config.huber_delta,
                loss_comp->config.reduction,
                &per_sample_loss
            );
            break;
            
        default:
            loss_result_free(result);
            return NULL;
    }
    
    result->loss_value = loss_value;
    result->per_sample_loss = per_sample_loss;
    
    // Update statistics
    loss_comp->total_loss += loss_value;
    loss_comp->num_samples += predictions->shape[0];
    loss_comp->num_batches++;
    
    if (loss_value < loss_comp->min_loss) {
        loss_comp->min_loss = loss_value;
    }
    if (loss_value > loss_comp->max_loss) {
        loss_comp->max_loss = loss_value;
    }
    
    loss_comp->avg_loss = loss_comp->total_loss / loss_comp->num_batches;
    
    return result;
}

// ============================================================================
// Backward Pass
// ============================================================================

Tensor* loss_compute_backward(
    LossComputation* loss_comp,
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask
) {
    if (!loss_comp || !predictions || !targets) return NULL;
    
    Tensor* gradients = NULL;
    
    // Compute gradients based on loss type
    switch (loss_comp->config.type) {
        case LOSS_CROSS_ENTROPY:
            gradients = loss_cross_entropy_backward(
                predictions, targets, mask,
                loss_comp->config.label_smoothing,
                loss_comp->epsilon
            );
            break;
            
        case LOSS_MSE:
            gradients = loss_mse_backward(predictions, targets, mask);
            break;
            
        case LOSS_MAE:
            gradients = loss_mae_backward(predictions, targets, mask);
            break;
            
        case LOSS_HUBER:
            gradients = loss_huber_backward(
                predictions, targets, mask,
                loss_comp->config.huber_delta
            );
            break;
            
        default:
            return NULL;
    }
    
    if (!gradients) return NULL;
    
    // Check gradient numerical stability
    if (loss_comp->check_gradients) {
        bool has_nan, has_inf;
        if (!loss_check_numerical_stability(gradients, &has_nan, &has_inf)) {
            fprintf(stderr, "Warning: Gradients contain NaN or Inf\n");
        }
    }
    
    // Clip gradients if enabled
    if (loss_comp->clip_gradients) {
        if (loss_comp->clip_value > 0.0f) {
            loss_clip_gradients_by_value(gradients, loss_comp->clip_value);
        }
        if (loss_comp->clip_norm > 0.0f) {
            loss_clip_gradients_by_norm(gradients, loss_comp->clip_norm);
        }
    }
    
    // Update gradient statistics
    loss_comp->gradient_norm = loss_compute_gradient_norm(gradients);
    
    // Find min/max gradients
    for (size_t i = 0; i < gradients->total_size; i++) {
        float g = gradients->data[i];
        if (g < loss_comp->gradient_min) loss_comp->gradient_min = g;
        if (g > loss_comp->gradient_max) loss_comp->gradient_max = g;
    }
    
    return gradients;
}

// ============================================================================
// Cross-Entropy Loss
// ============================================================================

float loss_cross_entropy_forward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    LossReduction reduction,
    float label_smoothing,
    float epsilon,
    Tensor** per_sample_loss
) {
    (void)epsilon;  // Reserved for numerical stability in future implementations
    if (!predictions || !targets) return NAN;
    
    size_t batch_size = predictions->shape[0];
    size_t num_classes = predictions->shape[1];
    
    // Apply log-softmax for numerical stability
    Tensor* log_probs = loss_log_softmax(predictions);
    if (!log_probs) return NAN;
    
    // Compute per-sample loss
    float* losses = (float*)calloc(batch_size, sizeof(float));
    if (!losses) {
        tensor_free(log_probs);
        return NAN;
    }
    
    float total_loss = 0.0f;
    size_t valid_samples = 0;
    
    for (size_t b = 0; b < batch_size; b++) {
        // Check mask
        if (mask && tensor_get(mask, (uint32_t[]){(uint32_t)b}) == 0.0f) {
            losses[b] = 0.0f;
            continue;
        }
        
        // Get target class
        int target_class = (int)tensor_get(targets, (uint32_t[]){(uint32_t)b});
        
        if (target_class < 0 || target_class >= (int)num_classes) {
            losses[b] = 0.0f;
            continue;
        }
        
        // Get log probability for target class
        float log_prob = tensor_get(log_probs, (uint32_t[]){(uint32_t)b, (uint32_t)target_class});
        
        // Apply label smoothing if enabled
        if (label_smoothing > 0.0f) {
            float smooth_loss = 0.0f;
            for (size_t c = 0; c < num_classes; c++) {
                float lp = tensor_get(log_probs, (uint32_t[]){(uint32_t)b, (uint32_t)c});
                if (c == (size_t)target_class) {
                    smooth_loss += (1.0f - label_smoothing) * (-lp);
                } else {
                    smooth_loss += (label_smoothing / (num_classes - 1)) * (-lp);
                }
            }
            losses[b] = smooth_loss;
        } else {
            losses[b] = -log_prob;
        }
        
        total_loss += losses[b];
        valid_samples++;
    }
    
    tensor_free(log_probs);
    
    // Apply reduction
    float result;
    if (reduction == LOSS_REDUCTION_MEAN && valid_samples > 0) {
        result = total_loss / valid_samples;
    } else if (reduction == LOSS_REDUCTION_SUM) {
        result = total_loss;
    } else {
        result = total_loss; // NONE - but we still return scalar
    }
    
    // Store per-sample losses if requested
    if (per_sample_loss) {
        *per_sample_loss = tensor_create((uint32_t[]){(uint32_t)batch_size}, 1);
        if (*per_sample_loss) {
            memcpy((*per_sample_loss)->data, losses, batch_size * sizeof(float));
        }
    }
    
    free(losses);
    return result;
}

Tensor* loss_cross_entropy_backward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    float label_smoothing,
    float epsilon
) {
    (void)epsilon;  // Reserved for numerical stability
    if (!predictions || !targets) return NULL;
    
    size_t batch_size = predictions->shape[0];
    size_t num_classes = predictions->shape[1];
    
    // Compute softmax probabilities
    Tensor* probs = loss_softmax(predictions);
    if (!probs) return NULL;
    
    // Create gradient tensor (same shape as predictions)
    Tensor* gradients = tensor_create(predictions->shape, predictions->ndim);
    if (!gradients) {
        tensor_free(probs);
        return NULL;
    }
    
    // Compute gradients: d_loss/d_logits = probs - targets
    for (size_t b = 0; b < batch_size; b++) {
        // Check mask
        float mask_value = 1.0f;
        if (mask) {
            mask_value = tensor_get(mask, (uint32_t[]){(uint32_t)b});
        }
        
        if (mask_value == 0.0f) {
            // Zero out gradients for masked samples
            for (size_t c = 0; c < num_classes; c++) {
                tensor_set(gradients, (uint32_t[]){(uint32_t)b, (uint32_t)c}, 0.0f);
            }
            continue;
        }
        
        // Get target class
        int target_class = (int)tensor_get(targets, (uint32_t[]){(uint32_t)b});
        
        if (target_class < 0 || target_class >= (int)num_classes) {
            // Invalid target - zero gradients
            for (size_t c = 0; c < num_classes; c++) {
                tensor_set(gradients, (uint32_t[]){(uint32_t)b, (uint32_t)c}, 0.0f);
            }
            continue;
        }
        
        // Compute gradient for each class
        for (size_t c = 0; c < num_classes; c++) {
            float prob = tensor_get(probs, (uint32_t[]){(uint32_t)b, (uint32_t)c});
            float target_prob;
            
            if (label_smoothing > 0.0f) {
                // With label smoothing
                if (c == (size_t)target_class) {
                    target_prob = 1.0f - label_smoothing;
                } else {
                    target_prob = label_smoothing / (num_classes - 1);
                }
            } else {
                // Without label smoothing (one-hot)
                target_prob = (c == (size_t)target_class) ? 1.0f : 0.0f;
            }
            
            float grad = (prob - target_prob) * mask_value;
            tensor_set(gradients, (uint32_t[]){(uint32_t)b, (uint32_t)c}, grad);
        }
    }
    
    tensor_free(probs);
    
    // Scale by batch size for mean reduction
    float scale = 1.0f / batch_size;
    for (size_t i = 0; i < gradients->total_size; i++) {
        gradients->data[i] *= scale;
    }
    
    return gradients;
}

// ============================================================================
// MSE Loss
// ============================================================================

float loss_mse_forward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    LossReduction reduction,
    Tensor** per_sample_loss
) {
    if (!predictions || !targets) return NAN;
    if (predictions->total_size != targets->total_size) return NAN;
    
    size_t batch_size = predictions->shape[0];
    size_t sample_size = predictions->total_size / batch_size;
    
    float* losses = (float*)calloc(batch_size, sizeof(float));
    if (!losses) return NAN;
    
    float total_loss = 0.0f;
    size_t valid_samples = 0;
    
    for (size_t b = 0; b < batch_size; b++) {
        // Check mask
        if (mask && tensor_get(mask, (uint32_t[]){(uint32_t)b}) == 0.0f) {
            losses[b] = 0.0f;
            continue;
        }
        
        // Compute squared error for this sample
        float sample_loss = 0.0f;
        for (size_t i = 0; i < sample_size; i++) {
            size_t idx = b * sample_size + i;
            float diff = predictions->data[idx] - targets->data[idx];
            sample_loss += diff * diff;
        }
        
        losses[b] = sample_loss / sample_size;
        total_loss += losses[b];
        valid_samples++;
    }
    
    // Apply reduction
    float result;
    if (reduction == LOSS_REDUCTION_MEAN && valid_samples > 0) {
        result = total_loss / valid_samples;
    } else if (reduction == LOSS_REDUCTION_SUM) {
        result = total_loss;
    } else {
        result = total_loss;
    }
    
    // Store per-sample losses if requested
    if (per_sample_loss) {
        *per_sample_loss = tensor_create((uint32_t[]){(uint32_t)batch_size}, 1);
        if (*per_sample_loss) {
            memcpy((*per_sample_loss)->data, losses, batch_size * sizeof(float));
        }
    }
    
    free(losses);
    return result;
}

Tensor* loss_mse_backward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask
) {
    if (!predictions || !targets) return NULL;
    if (predictions->total_size != targets->total_size) return NULL;
    
    size_t batch_size = predictions->shape[0];
    size_t sample_size = predictions->total_size / batch_size;
    
    // Create gradient tensor
    Tensor* gradients = tensor_create(predictions->shape, predictions->ndim);
    if (!gradients) return NULL;
    
    // Compute gradients: d_loss/d_pred = 2 * (pred - target) / N
    float scale = 2.0f / (batch_size * sample_size);
    
    for (size_t b = 0; b < batch_size; b++) {
        float mask_value = 1.0f;
        if (mask) {
            mask_value = tensor_get(mask, (uint32_t[]){(uint32_t)b});
        }
        
        for (size_t i = 0; i < sample_size; i++) {
            size_t idx = b * sample_size + i;
            float diff = predictions->data[idx] - targets->data[idx];
            gradients->data[idx] = scale * diff * mask_value;
        }
    }
    
    return gradients;
}

// ============================================================================
// MAE Loss
// ============================================================================

float loss_mae_forward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    LossReduction reduction,
    Tensor** per_sample_loss
) {
    if (!predictions || !targets) return NAN;
    if (predictions->total_size != targets->total_size) return NAN;
    
    size_t batch_size = predictions->shape[0];
    size_t sample_size = predictions->total_size / batch_size;
    
    float* losses = (float*)calloc(batch_size, sizeof(float));
    if (!losses) return NAN;
    
    float total_loss = 0.0f;
    size_t valid_samples = 0;
    
    for (size_t b = 0; b < batch_size; b++) {
        if (mask && tensor_get(mask, (uint32_t[]){(uint32_t)b}) == 0.0f) {
            losses[b] = 0.0f;
            continue;
        }
        
        float sample_loss = 0.0f;
        for (size_t i = 0; i < sample_size; i++) {
            size_t idx = b * sample_size + i;
            float diff = fabsf(predictions->data[idx] - targets->data[idx]);
            sample_loss += diff;
        }
        
        losses[b] = sample_loss / sample_size;
        total_loss += losses[b];
        valid_samples++;
    }
    
    float result;
    if (reduction == LOSS_REDUCTION_MEAN && valid_samples > 0) {
        result = total_loss / valid_samples;
    } else if (reduction == LOSS_REDUCTION_SUM) {
        result = total_loss;
    } else {
        result = total_loss;
    }
    
    if (per_sample_loss) {
        *per_sample_loss = tensor_create((uint32_t[]){(uint32_t)batch_size}, 1);
        if (*per_sample_loss) {
            memcpy((*per_sample_loss)->data, losses, batch_size * sizeof(float));
        }
    }
    
    free(losses);
    return result;
}

Tensor* loss_mae_backward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask
) {
    if (!predictions || !targets) return NULL;
    if (predictions->total_size != targets->total_size) return NULL;
    
    size_t batch_size = predictions->shape[0];
    size_t sample_size = predictions->total_size / batch_size;
    
    Tensor* gradients = tensor_create(predictions->shape, predictions->ndim);
    if (!gradients) return NULL;
    
    float scale = 1.0f / (batch_size * sample_size);
    
    for (size_t b = 0; b < batch_size; b++) {
        float mask_value = 1.0f;
        if (mask) {
            mask_value = tensor_get(mask, (uint32_t[]){(uint32_t)b});
        }
        
        for (size_t i = 0; i < sample_size; i++) {
            size_t idx = b * sample_size + i;
            float diff = predictions->data[idx] - targets->data[idx];
            // Sign of difference
            float sign = (diff > 0.0f) ? 1.0f : ((diff < 0.0f) ? -1.0f : 0.0f);
            gradients->data[idx] = scale * sign * mask_value;
        }
    }
    
    return gradients;
}

// ============================================================================
// Huber Loss
// ============================================================================

float loss_huber_forward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    float delta,
    LossReduction reduction,
    Tensor** per_sample_loss
) {
    if (!predictions || !targets) return NAN;
    if (predictions->total_size != targets->total_size) return NAN;
    
    size_t batch_size = predictions->shape[0];
    size_t sample_size = predictions->total_size / batch_size;
    
    float* losses = (float*)calloc(batch_size, sizeof(float));
    if (!losses) return NAN;
    
    float total_loss = 0.0f;
    size_t valid_samples = 0;
    
    for (size_t b = 0; b < batch_size; b++) {
        if (mask && tensor_get(mask, (uint32_t[]){(uint32_t)b}) == 0.0f) {
            losses[b] = 0.0f;
            continue;
        }
        
        float sample_loss = 0.0f;
        for (size_t i = 0; i < sample_size; i++) {
            size_t idx = b * sample_size + i;
            float diff = fabsf(predictions->data[idx] - targets->data[idx]);
            
            if (diff <= delta) {
                // Quadratic for small errors
                sample_loss += 0.5f * diff * diff;
            } else {
                // Linear for large errors
                sample_loss += delta * (diff - 0.5f * delta);
            }
        }
        
        losses[b] = sample_loss / sample_size;
        total_loss += losses[b];
        valid_samples++;
    }
    
    float result;
    if (reduction == LOSS_REDUCTION_MEAN && valid_samples > 0) {
        result = total_loss / valid_samples;
    } else if (reduction == LOSS_REDUCTION_SUM) {
        result = total_loss;
    } else {
        result = total_loss;
    }
    
    if (per_sample_loss) {
        *per_sample_loss = tensor_create((uint32_t[]){(uint32_t)batch_size}, 1);
        if (*per_sample_loss) {
            memcpy((*per_sample_loss)->data, losses, batch_size * sizeof(float));
        }
    }
    
    free(losses);
    return result;
}

Tensor* loss_huber_backward(
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask,
    float delta
) {
    if (!predictions || !targets) return NULL;
    if (predictions->total_size != targets->total_size) return NULL;
    
    size_t batch_size = predictions->shape[0];
    size_t sample_size = predictions->total_size / batch_size;
    
    Tensor* gradients = tensor_create(predictions->shape, predictions->ndim);
    if (!gradients) return NULL;
    
    float scale = 1.0f / (batch_size * sample_size);
    
    for (size_t b = 0; b < batch_size; b++) {
        float mask_value = 1.0f;
        if (mask) {
            mask_value = tensor_get(mask, (uint32_t[]){(uint32_t)b});
        }
        
        for (size_t i = 0; i < sample_size; i++) {
            size_t idx = b * sample_size + i;
            float diff = predictions->data[idx] - targets->data[idx];
            float abs_diff = fabsf(diff);
            
            float grad;
            if (abs_diff <= delta) {
                // Quadratic region: gradient = diff
                grad = diff;
            } else {
                // Linear region: gradient = delta * sign(diff)
                float sign = (diff > 0.0f) ? 1.0f : -1.0f;
                grad = delta * sign;
            }
            
            gradients->data[idx] = scale * grad * mask_value;
        }
    }
    
    return gradients;
}

// ============================================================================
// Utility Functions
// ============================================================================

float loss_log_sum_exp(const float* values, size_t size) {
    if (!values || size == 0) return -INFINITY;
    
    // Find max value for numerical stability
    float max_val = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] > max_val) {
            max_val = values[i];
        }
    }
    
    // Compute log(sum(exp(x - max)))
    float sum = 0.0f;
    for (size_t i = 0; i < size; i++) {
        sum += safe_exp(values[i] - max_val);
    }
    
    return max_val + safe_log(sum, 1e-7f);
}

Tensor* loss_log_softmax(const Tensor* logits) {
    if (!logits || logits->ndim != 2) return NULL;
    
    size_t batch_size = logits->shape[0];
    size_t num_classes = logits->shape[1];
    
    Tensor* log_probs = tensor_create(logits->shape, logits->ndim);
    if (!log_probs) return NULL;
    
    for (size_t b = 0; b < batch_size; b++) {
        // Get logits for this sample
        const float* sample_logits = &logits->data[b * num_classes];
        
        // Compute log-sum-exp
        float lse = loss_log_sum_exp(sample_logits, num_classes);
        
        // Compute log probabilities
        for (size_t c = 0; c < num_classes; c++) {
            size_t idx = b * num_classes + c;
            log_probs->data[idx] = logits->data[idx] - lse;
        }
    }
    
    return log_probs;
}

Tensor* loss_softmax(const Tensor* logits) {
    if (!logits || logits->ndim != 2) return NULL;
    
    size_t batch_size = logits->shape[0];
    size_t num_classes = logits->shape[1];
    
    Tensor* probs = tensor_create(logits->shape, logits->ndim);
    if (!probs) return NULL;
    
    for (size_t b = 0; b < batch_size; b++) {
        const float* sample_logits = &logits->data[b * num_classes];
        
        // Find max for numerical stability
        float max_logit = sample_logits[0];
        for (size_t c = 1; c < num_classes; c++) {
            if (sample_logits[c] > max_logit) {
                max_logit = sample_logits[c];
            }
        }
        
        // Compute exp(logits - max) and sum
        float sum = 0.0f;
        for (size_t c = 0; c < num_classes; c++) {
            size_t idx = b * num_classes + c;
            probs->data[idx] = safe_exp(logits->data[idx] - max_logit);
            sum += probs->data[idx];
        }
        
        // Normalize
        for (size_t c = 0; c < num_classes; c++) {
            size_t idx = b * num_classes + c;
            probs->data[idx] /= sum;
        }
    }
    
    return probs;
}

Tensor* loss_apply_label_smoothing(const Tensor* targets, float smoothing) {
    if (!targets || smoothing <= 0.0f || smoothing >= 1.0f) return NULL;
    
    Tensor* smoothed = tensor_copy(targets);
    if (!smoothed) return NULL;
    
    size_t num_classes = targets->shape[targets->ndim - 1];
    float smooth_value = smoothing / num_classes;
    float target_value = 1.0f - smoothing + smooth_value;
    
    for (size_t i = 0; i < smoothed->total_size; i++) {
        if (smoothed->data[i] == 1.0f) {
            smoothed->data[i] = target_value;
        } else {
            smoothed->data[i] = smooth_value;
        }
    }
    
    return smoothed;
}

bool loss_check_numerical_stability(const Tensor* tensor, bool* has_nan, bool* has_inf) {
    if (!tensor) return false;
    
    bool found_nan = false;
    bool found_inf = false;
    
    for (size_t i = 0; i < tensor->total_size; i++) {
        float val = tensor->data[i];
        if (isnan(val)) found_nan = true;
        if (isinf(val)) found_inf = true;
    }
    
    if (has_nan) *has_nan = found_nan;
    if (has_inf) *has_inf = found_inf;
    
    return !found_nan && !found_inf;
}

void loss_clip_gradients_by_value(Tensor* gradients, float clip_value) {
    if (!gradients || clip_value <= 0.0f) return;
    
    for (size_t i = 0; i < gradients->total_size; i++) {
        float val = gradients->data[i];
        if (val > clip_value) {
            gradients->data[i] = clip_value;
        } else if (val < -clip_value) {
            gradients->data[i] = -clip_value;
        }
    }
}

float loss_clip_gradients_by_norm(Tensor* gradients, float max_norm) {
    if (!gradients || max_norm <= 0.0f) return 0.0f;
    
    // Compute current norm
    float norm = loss_compute_gradient_norm(gradients);
    
    // Clip if necessary
    if (norm > max_norm) {
        float scale = max_norm / norm;
        for (size_t i = 0; i < gradients->total_size; i++) {
            gradients->data[i] *= scale;
        }
    }
    
    return norm;
}

float loss_compute_gradient_norm(const Tensor* gradients) {
    if (!gradients) return 0.0f;
    
    float sum_sq = 0.0f;
    for (size_t i = 0; i < gradients->total_size; i++) {
        float val = gradients->data[i];
        sum_sq += val * val;
    }
    
    return prime_sqrtf(sum_sq);
}

void loss_computation_print_stats(const LossComputation* loss_comp) {
    if (!loss_comp) return;
    
    printf("\n========================================\n");
    printf("  Loss Computation Statistics\n");
    printf("========================================\n");
    printf("Loss Type:       %d\n", loss_comp->config.type);
    printf("Reduction:       %d\n", loss_comp->config.reduction);
    printf("Label Smoothing: %.4f\n", loss_comp->config.label_smoothing);
    printf("\n");
    printf("Total Loss:      %.6f\n", loss_comp->total_loss);
    printf("Num Samples:     %zu\n", loss_comp->num_samples);
    printf("Num Batches:     %zu\n", loss_comp->num_batches);
    printf("Average Loss:    %.6f\n", loss_comp->avg_loss);
    printf("Min Loss:        %.6f\n", loss_comp->min_loss);
    printf("Max Loss:        %.6f\n", loss_comp->max_loss);
    printf("\n");
    printf("Gradient Norm:   %.6f\n", loss_comp->gradient_norm);
    printf("Gradient Min:    %.6f\n", loss_comp->gradient_min);
    printf("Gradient Max:    %.6f\n", loss_comp->gradient_max);
    printf("========================================\n\n");
}

void loss_computation_get_stats(
    const LossComputation* loss_comp,
    float* total_loss,
    size_t* num_samples,
    float* avg_loss,
    float* gradient_norm
) {
    if (!loss_comp) return;
    
    if (total_loss) *total_loss = loss_comp->total_loss;
    if (num_samples) *num_samples = loss_comp->num_samples;
    if (avg_loss) *avg_loss = loss_comp->avg_loss;
    if (gradient_norm) *gradient_norm = loss_comp->gradient_norm;
}