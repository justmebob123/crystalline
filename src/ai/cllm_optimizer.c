/*
 * CLLM Optimizer
 * Implements Adam optimizer and gradient operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/prime_float_math.h"

/**
 * Apply gradient clipping by global norm
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 * @param max_norm Maximum gradient norm
 */
void cllm_apply_gradient_clipping(float* gradients, size_t size, float max_norm) {
    if (!gradients || size == 0 || max_norm <= 0.0f) return;
    
    // Compute global norm
    float norm = 0.0f;
    for (size_t i = 0; i < size; i++) {
        norm += gradients[i] * gradients[i];
    }
    norm = prime_sqrt(norm);
    
    // Clip if necessary
    if (norm > max_norm) {
        float scale = max_norm / norm;
        for (size_t i = 0; i < size; i++) {
            gradients[i] *= scale;
        }
    }
}

/**
 * Apply gradient clipping by value
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 * @param clip_value Maximum absolute value
 */
void cllm_clip_gradients_by_value(float* gradients, size_t size, float clip_value) {
    if (!gradients || size == 0 || clip_value <= 0.0f) return;
    
    for (size_t i = 0; i < size; i++) {
        if (gradients[i] > clip_value) {
            gradients[i] = clip_value;
        } else if (gradients[i] < -clip_value) {
            gradients[i] = -clip_value;
        }
    }
}

/**
 * Adam optimizer step
 * 
 * Adam: Adaptive Moment Estimation
 * m_t = β₁ * m_{t-1} + (1 - β₁) * g_t
 * v_t = β₂ * v_{t-1} + (1 - β₂) * g_t²
 * m̂_t = m_t / (1 - β₁^t)
 * v̂_t = v_t / (1 - β₂^t)
 * θ_t = θ_{t-1} - α * m̂_t / (√v̂_t + ε)
 * 
 * @param training Training state
 * @param learning_rate Learning rate (α)
 */
void cllm_adam_step(CLLMTraining* training, float learning_rate) {
    if (!training || !training->gradients || !training->optimizer_state) return;
    
    size_t total_params = training->model->header.total_params;
    
    // Adam hyperparameters
    float beta1 = 0.9f;
    float beta2 = 0.999f;
    float epsilon = 1e-8f;
    
    // Get optimizer state (m and v)
    float* m = training->optimizer_state;
    float* v = &training->optimizer_state[total_params];
    
    // Update step count
    int t = training->current_step + 1;
    
    // Bias correction terms
    float bias_correction1 = 1.0f - prime_pow(beta1, (float)t);
    float bias_correction2 = 1.0f - prime_pow(beta2, (float)t);
    
    // Get model weights (simplified - assumes contiguous weight array)
    float* weights = training->model->weights;
    float* gradients = training->gradients;
    
    // Update parameters
    for (size_t i = 0; i < total_params; i++) {
        // Update biased first moment estimate
        m[i] = beta1 * m[i] + (1.0f - beta1) * gradients[i];
        
        // Update biased second raw moment estimate
        v[i] = beta2 * v[i] + (1.0f - beta2) * gradients[i] * gradients[i];
        
        // Compute bias-corrected first moment estimate
        float m_hat = m[i] / bias_correction1;
        
        // Compute bias-corrected second raw moment estimate
        float v_hat = v[i] / bias_correction2;
        
        // Update parameters
        weights[i] -= learning_rate * m_hat / (prime_sqrt(v_hat) + epsilon);
    }
}

/**
 * SGD with momentum optimizer step
 * 
 * @param training Training state
 * @param learning_rate Learning rate
 * @param momentum Momentum coefficient (typically 0.9)
 */
void cllm_sgd_momentum_step(CLLMTraining* training, float learning_rate, float momentum) {
    if (!training || !training->gradients || !training->optimizer_state) return;
    
    size_t total_params = training->model->header.total_params;
    
    float* weights = training->model->weights;
    float* gradients = training->gradients;
    float* velocity = training->optimizer_state;
    
    for (size_t i = 0; i < total_params; i++) {
        // Update velocity: v = momentum * v - lr * grad
        velocity[i] = momentum * velocity[i] - learning_rate * gradients[i];
        
        // Update weights: w = w + v
        weights[i] += velocity[i];
    }
}

/**
 * Update learning rate with warmup and decay
 * 
 * @param training Training state
 */
void cllm_update_learning_rate(CLLMTraining* training) {
    if (!training) return;
    
    int step = training->current_step;
    int warmup_steps = training->config.warmup_steps;
    float base_lr = training->config.learning_rate;
    
    float lr;
    
    if (step < warmup_steps) {
        // Linear warmup
        lr = base_lr * (float)step / (float)warmup_steps;
    } else {
        // Cosine decay after warmup
        int decay_steps = training->config.max_steps - warmup_steps;
        int steps_since_warmup = step - warmup_steps;
        
        float progress = (float)steps_since_warmup / (float)decay_steps;
        lr = base_lr * 0.5f * (1.0f + prime_cos(3.14159265f * progress));
    }
    
    // Ensure minimum learning rate
    if (lr < 1e-6f) {
        lr = 1e-6f;
    }
    
    training->config.learning_rate = lr;
}

/**
 * Apply weight decay (L2 regularization)
 * 
 * @param weights Weight array [size]
 * @param size Number of parameters
 * @param weight_decay Weight decay coefficient
 * @param learning_rate Learning rate
 */
void cllm_apply_weight_decay(float* weights, size_t size, 
                            float weight_decay, float learning_rate) {
    if (!weights || size == 0 || weight_decay <= 0.0f) return;
    
    for (size_t i = 0; i < size; i++) {
        weights[i] *= (1.0f - learning_rate * weight_decay);
    }
}

/**
 * Zero gradients
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 */
void cllm_zero_gradients(float* gradients, size_t size) {
    if (!gradients || size == 0) return;
    memset(gradients, 0, size * sizeof(float));
}

/**
 * Compute gradient norm
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 * @return L2 norm of gradients
 */
float cllm_compute_gradient_norm(float* gradients, size_t size) {
    if (!gradients || size == 0) return 0.0f;
    
    float norm = 0.0f;
    for (size_t i = 0; i < size; i++) {
        norm += gradients[i] * gradients[i];
    }
    
    return prime_sqrt(norm);
}

/**
 * Apply gradient accumulation
 * Adds current gradients to accumulated gradients
 * 
 * @param accumulated_grads Accumulated gradients [size]
 * @param current_grads Current gradients [size]
 * @param size Number of parameters
 */
void cllm_accumulate_gradients(float* accumulated_grads, float* current_grads, size_t size) {
    if (!accumulated_grads || !current_grads || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        accumulated_grads[i] += current_grads[i];
    }
}

/**
 * Scale gradients (for gradient accumulation)
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 * @param scale Scale factor
 */
void cllm_scale_gradients(float* gradients, size_t size, float scale) {
    if (!gradients || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        gradients[i] *= scale;
    }
}

/**
 * Check for NaN or Inf in gradients
 * 
 * @param gradients Gradient array [size]
 * @param size Number of parameters
 * @return 1 if NaN/Inf found, 0 otherwise
 */
int cllm_check_gradients_valid(float* gradients, size_t size) {
    if (!gradients || size == 0) return 0;
    
    for (size_t i = 0; i < size; i++) {
        // Check for NaN
        if (gradients[i] != gradients[i]) {
            return 1;
        }
        
        // Check for Inf
        if (gradients[i] > 1e38f || gradients[i] < -1e38f) {
            return 1;
        }
    }
    
    return 0;
}

/**
 * Exponential moving average for model parameters
 * Used for model averaging (e.g., EMA of weights)
 * 
 * @param ema_weights EMA weights [size]
 * @param current_weights Current weights [size]
 * @param size Number of parameters
 * @param decay Decay rate (typically 0.999)
 */
void cllm_update_ema_weights(float* ema_weights, float* current_weights,
                            size_t size, float decay) {
    if (!ema_weights || !current_weights || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        ema_weights[i] = decay * ema_weights[i] + (1.0f - decay) * current_weights[i];
    }
}