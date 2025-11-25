/**
 * @file loss_functions.c
 * @brief Implementation of mathematical loss functions
 */

#include "loss_functions.h"
#include "numerical.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/* ============================================================================
 * Configuration
 * ============================================================================ */

LossConfig loss_config_create(LossFunctionType type) {
    LossConfig config = {
        .type = type,
        .reduction = LOSS_REDUCTION_MEAN,
        .label_smoothing = 0.0,
        .huber_delta = 1.0,
        .clip_gradients = false,
        .gradient_clip_value = 1.0,
        .gradient_clip_norm = 1.0
    };
    return config;
}

/* ============================================================================
 * Cross-Entropy Loss
 * ============================================================================ */

LossResult loss_cross_entropy(
    const double* predictions,
    const double* targets,
    size_t batch_size,
    size_t num_classes,
    const LossConfig* config
) {
    LossResult result = {0};
    
    if (!predictions || !targets || batch_size == 0 || num_classes == 0) {
        return result;
    }
    
    result.num_samples = batch_size;
    
    // Allocate per-sample losses if needed
    if (config && config->reduction == LOSS_REDUCTION_NONE) {
        result.per_sample_losses = (double*)calloc(batch_size, sizeof(double));
    }
    
    double total_loss = 0.0;
    const double epsilon = 1e-10;
    
    for (size_t i = 0; i < batch_size; i++) {
        double sample_loss = 0.0;
        
        for (size_t j = 0; j < num_classes; j++) {
            size_t idx = i * num_classes + j;
            double pred = predictions[idx];
            double target = targets[idx];
            
            // Apply label smoothing if configured
            if (config && config->label_smoothing > 0.0) {
                target = target * (1.0 - config->label_smoothing) + 
                        config->label_smoothing / (double)num_classes;
            }
            
            // Cross-entropy: -sum(target * log(pred))
            sample_loss -= target * numerical_safe_log(pred, epsilon);
        }
        
        if (result.per_sample_losses) {
            result.per_sample_losses[i] = sample_loss;
        }
        
        total_loss += sample_loss;
    }
    
    // Apply reduction
    if (!config || config->reduction == LOSS_REDUCTION_MEAN) {
        result.loss_value = total_loss / (double)batch_size;
    } else if (config->reduction == LOSS_REDUCTION_SUM) {
        result.loss_value = total_loss;
    } else {
        result.loss_value = total_loss; // NONE - total is stored, per-sample in array
    }
    
    // Check for numerical issues
    loss_check_numerical_issues(&result.loss_value, 1, 
                               &result.has_nan, &result.has_inf);
    
    return result;
}

/* ============================================================================
 * Mean Squared Error Loss
 * ============================================================================ */

LossResult loss_mse(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
) {
    LossResult result = {0};
    
    if (!predictions || !targets || size == 0) {
        return result;
    }
    
    result.num_samples = size;
    
    double total_loss = 0.0;
    
    for (size_t i = 0; i < size; i++) {
        double diff = predictions[i] - targets[i];
        total_loss += diff * diff;
    }
    
    // Apply reduction
    if (!config || config->reduction == LOSS_REDUCTION_MEAN) {
        result.loss_value = total_loss / (double)size;
    } else {
        result.loss_value = total_loss;
    }
    
    loss_check_numerical_issues(&result.loss_value, 1,
                               &result.has_nan, &result.has_inf);
    
    return result;
}

/* ============================================================================
 * Mean Absolute Error Loss
 * ============================================================================ */

LossResult loss_mae(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
) {
    LossResult result = {0};
    
    if (!predictions || !targets || size == 0) {
        return result;
    }
    
    result.num_samples = size;
    
    double total_loss = 0.0;
    
    for (size_t i = 0; i < size; i++) {
        double diff = predictions[i] - targets[i];
        total_loss += (diff < 0) ? -diff : diff;
    }
    
    // Apply reduction
    if (!config || config->reduction == LOSS_REDUCTION_MEAN) {
        result.loss_value = total_loss / (double)size;
    } else {
        result.loss_value = total_loss;
    }
    
    loss_check_numerical_issues(&result.loss_value, 1,
                               &result.has_nan, &result.has_inf);
    
    return result;
}

/* ============================================================================
 * Huber Loss
 * ============================================================================ */

LossResult loss_huber(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
) {
    LossResult result = {0};
    
    if (!predictions || !targets || size == 0) {
        return result;
    }
    
    result.num_samples = size;
    
    double delta = (config && config->huber_delta > 0) ? config->huber_delta : 1.0;
    double total_loss = 0.0;
    
    for (size_t i = 0; i < size; i++) {
        double diff = predictions[i] - targets[i];
        double abs_diff = (diff < 0) ? -diff : diff;
        
        if (abs_diff <= delta) {
            // Quadratic for small errors
            total_loss += 0.5 * diff * diff;
        } else {
            // Linear for large errors
            total_loss += delta * (abs_diff - 0.5 * delta);
        }
    }
    
    // Apply reduction
    if (!config || config->reduction == LOSS_REDUCTION_MEAN) {
        result.loss_value = total_loss / (double)size;
    } else {
        result.loss_value = total_loss;
    }
    
    loss_check_numerical_issues(&result.loss_value, 1,
                               &result.has_nan, &result.has_inf);
    
    return result;
}

/* ============================================================================
 * KL Divergence
 * ============================================================================ */

LossResult loss_kl_divergence(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
) {
    LossResult result = {0};
    
    if (!predictions || !targets || size == 0) {
        return result;
    }
    
    result.num_samples = size;
    
    const double epsilon = 1e-10;
    double total_loss = 0.0;
    
    for (size_t i = 0; i < size; i++) {
        double p = targets[i];
        double q = predictions[i];
        
        if (p > epsilon) {
            // KL(P||Q) = sum(P * log(P/Q))
            total_loss += p * (log(p + epsilon) - log(q + epsilon));
        }
    }
    
    // Apply reduction
    if (!config || config->reduction == LOSS_REDUCTION_MEAN) {
        result.loss_value = total_loss / (double)size;
    } else {
        result.loss_value = total_loss;
    }
    
    loss_check_numerical_issues(&result.loss_value, 1,
                               &result.has_nan, &result.has_inf);
    
    return result;
}

/* ============================================================================
 * Binary Cross-Entropy Loss
 * ============================================================================ */

LossResult loss_binary_cross_entropy(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
) {
    LossResult result = {0};
    
    if (!predictions || !targets || size == 0) {
        return result;
    }
    
    result.num_samples = size;
    
    const double epsilon = 1e-10;
    double total_loss = 0.0;
    
    for (size_t i = 0; i < size; i++) {
        double pred = predictions[i];
        double target = targets[i];
        
        // BCE: -[y*log(p) + (1-y)*log(1-p)]
        total_loss -= target * numerical_safe_log(pred, epsilon) +
                     (1.0 - target) * numerical_safe_log(1.0 - pred, epsilon);
    }
    
    // Apply reduction
    if (!config || config->reduction == LOSS_REDUCTION_MEAN) {
        result.loss_value = total_loss / (double)size;
    } else {
        result.loss_value = total_loss;
    }
    
    loss_check_numerical_issues(&result.loss_value, 1,
                               &result.has_nan, &result.has_inf);
    
    return result;
}

/* ============================================================================
 * Gradient Computation
 * ============================================================================ */

GradientInfo loss_cross_entropy_gradient(
    const double* predictions,
    const double* targets,
    size_t batch_size,
    size_t num_classes,
    const LossConfig* config
) {
    GradientInfo info = {0};
    
    if (!predictions || !targets || batch_size == 0 || num_classes == 0) {
        return info;
    }
    
    size_t total_size = batch_size * num_classes;
    info.size = total_size;
    info.gradients = (double*)calloc(total_size, sizeof(double));
    
    if (!info.gradients) return info;
    
    const double epsilon = 1e-10;
    
    // Gradient of cross-entropy: -target / prediction
    for (size_t i = 0; i < total_size; i++) {
        double pred = predictions[i];
        double target = targets[i];
        
        info.gradients[i] = -target / (pred + epsilon);
    }
    
    // Apply gradient clipping if configured
    if (config && config->clip_gradients) {
        if (config->gradient_clip_value > 0) {
            info.clipped = loss_clip_gradients_by_value(
                info.gradients, total_size, config->gradient_clip_value
            );
        }
        if (config->gradient_clip_norm > 0) {
            info.clipped |= loss_clip_gradients_by_norm(
                info.gradients, total_size, config->gradient_clip_norm
            );
        }
    }
    
    info.gradient_norm = loss_gradient_norm(info.gradients, total_size);
    
    return info;
}

GradientInfo loss_mse_gradient(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
) {
    GradientInfo info = {0};
    
    if (!predictions || !targets || size == 0) {
        return info;
    }
    
    info.size = size;
    info.gradients = (double*)calloc(size, sizeof(double));
    
    if (!info.gradients) return info;
    
    // Gradient of MSE: 2 * (prediction - target)
    for (size_t i = 0; i < size; i++) {
        info.gradients[i] = 2.0 * (predictions[i] - targets[i]);
    }
    
    // Apply gradient clipping if configured
    if (config && config->clip_gradients) {
        if (config->gradient_clip_value > 0) {
            info.clipped = loss_clip_gradients_by_value(
                info.gradients, size, config->gradient_clip_value
            );
        }
        if (config->gradient_clip_norm > 0) {
            info.clipped |= loss_clip_gradients_by_norm(
                info.gradients, size, config->gradient_clip_norm
            );
        }
    }
    
    info.gradient_norm = loss_gradient_norm(info.gradients, size);
    
    return info;
}

GradientInfo loss_mae_gradient(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
) {
    GradientInfo info = {0};
    
    if (!predictions || !targets || size == 0) {
        return info;
    }
    
    info.size = size;
    info.gradients = (double*)calloc(size, sizeof(double));
    
    if (!info.gradients) return info;
    
    // Gradient of MAE: sign(prediction - target)
    for (size_t i = 0; i < size; i++) {
        double diff = predictions[i] - targets[i];
        info.gradients[i] = (diff > 0) ? 1.0 : ((diff < 0) ? -1.0 : 0.0);
    }
    
    // Apply gradient clipping if configured
    if (config && config->clip_gradients) {
        if (config->gradient_clip_value > 0) {
            info.clipped = loss_clip_gradients_by_value(
                info.gradients, size, config->gradient_clip_value
            );
        }
        if (config->gradient_clip_norm > 0) {
            info.clipped |= loss_clip_gradients_by_norm(
                info.gradients, size, config->gradient_clip_norm
            );
        }
    }
    
    info.gradient_norm = loss_gradient_norm(info.gradients, size);
    
    return info;
}

GradientInfo loss_huber_gradient(
    const double* predictions,
    const double* targets,
    size_t size,
    const LossConfig* config
) {
    GradientInfo info = {0};
    
    if (!predictions || !targets || size == 0) {
        return info;
    }
    
    info.size = size;
    info.gradients = (double*)calloc(size, sizeof(double));
    
    if (!info.gradients) return info;
    
    double delta = (config && config->huber_delta > 0) ? config->huber_delta : 1.0;
    
    // Gradient of Huber loss
    for (size_t i = 0; i < size; i++) {
        double diff = predictions[i] - targets[i];
        double abs_diff = (diff < 0) ? -diff : diff;
        
        if (abs_diff <= delta) {
            // Quadratic region: gradient = diff
            info.gradients[i] = diff;
        } else {
            // Linear region: gradient = delta * sign(diff)
            info.gradients[i] = delta * ((diff > 0) ? 1.0 : -1.0);
        }
    }
    
    // Apply gradient clipping if configured
    if (config && config->clip_gradients) {
        if (config->gradient_clip_value > 0) {
            info.clipped = loss_clip_gradients_by_value(
                info.gradients, size, config->gradient_clip_value
            );
        }
        if (config->gradient_clip_norm > 0) {
            info.clipped |= loss_clip_gradients_by_norm(
                info.gradients, size, config->gradient_clip_norm
            );
        }
    }
    
    info.gradient_norm = loss_gradient_norm(info.gradients, size);
    
    return info;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

double* loss_apply_label_smoothing(
    const double* targets,
    size_t num_samples,
    size_t num_classes,
    double smoothing
) {
    if (!targets || num_samples == 0 || num_classes == 0 || smoothing <= 0.0) {
        return NULL;
    }
    
    size_t total_size = num_samples * num_classes;
    double* smoothed = (double*)malloc(total_size * sizeof(double));
    if (!smoothed) return NULL;
    
    double smooth_value = smoothing / (double)num_classes;
    
    for (size_t i = 0; i < total_size; i++) {
        smoothed[i] = targets[i] * (1.0 - smoothing) + smooth_value;
    }
    
    return smoothed;
}

bool loss_clip_gradients_by_value(double* gradients, size_t size, double clip_value) {
    if (!gradients || size == 0 || clip_value <= 0.0) return false;
    
    bool clipped = false;
    
    for (size_t i = 0; i < size; i++) {
        if (gradients[i] > clip_value) {
            gradients[i] = clip_value;
            clipped = true;
        } else if (gradients[i] < -clip_value) {
            gradients[i] = -clip_value;
            clipped = true;
        }
    }
    
    return clipped;
}

bool loss_clip_gradients_by_norm(double* gradients, size_t size, double max_norm) {
    if (!gradients || size == 0 || max_norm <= 0.0) return false;
    
    double norm = loss_gradient_norm(gradients, size);
    
    if (norm > max_norm) {
        double scale = max_norm / norm;
        for (size_t i = 0; i < size; i++) {
            gradients[i] *= scale;
        }
        return true;
    }
    
    return false;
}

double loss_gradient_norm(const double* gradients, size_t size) {
    if (!gradients || size == 0) return 0.0;
    
    double norm = 0.0;
    for (size_t i = 0; i < size; i++) {
        norm += gradients[i] * gradients[i];
    }
    return sqrt(norm);
}

void loss_check_numerical_issues(
    const double* values,
    size_t size,
    bool* has_nan,
    bool* has_inf
) {
    numerical_check_array(values, size, has_nan, has_inf);
}

void loss_result_free(LossResult* result) {
    if (!result) return;
    
    if (result->per_sample_losses) {
        free(result->per_sample_losses);
        result->per_sample_losses = NULL;
    }
}

void gradient_info_free(GradientInfo* info) {
    if (!info) return;
    
    if (info->gradients) {
        free(info->gradients);
        info->gradients = NULL;
    }
}