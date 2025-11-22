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
 * Adam update for a single parameter array
 * 
 * @param weights Weight array to update
 * @param gradients Gradient array
 * @param m First moment array
 * @param v Second moment array
 * @param size Number of parameters
 * @param learning_rate Learning rate
 * @param beta1 First moment decay
 * @param beta2 Second moment decay
 * @param epsilon Small constant for numerical stability
 * @param bias_correction1 Bias correction for first moment
 * @param bias_correction2 Bias correction for second moment
 */
static void adam_update_params(float* weights, float* gradients, float* m, float* v,
                               size_t size, float learning_rate, float beta1, float beta2,
                               float epsilon, float bias_correction1, float bias_correction2) {
    if (!weights || !gradients || !m || !v) return;
    
    for (size_t i = 0; i < size; i++) {
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
 * Adam optimizer step - updates all model parameters
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
    if (!training || !training->model) return;
    
    CLLMModel* model = training->model;
    
    // Adam hyperparameters
    float beta1 = 0.9f;
    float beta2 = 0.999f;
    float epsilon = 1e-8f;
    
    // Update step count
    int t = training->current_step + 1;
    
    // Bias correction terms
    float bias_correction1 = 1.0f - prime_pow(beta1, (float)t);
    float bias_correction2 = 1.0f - prime_pow(beta2, (float)t);
    
    // Update embeddings (if gradients available)
    if (training->gradients && training->optimizer_state) {
        size_t embed_size = model->vocab_size * model->embedding_dim;
        float* m = training->optimizer_state;
        float* v = &training->optimizer_state[embed_size];
        
        adam_update_params(model->embeddings.embeddings, training->gradients,
                          m, v, embed_size, learning_rate, beta1, beta2,
                          epsilon, bias_correction1, bias_correction2);
    }
    
    // Update attention layers
    if (training->attention_grads) {
        for (uint32_t layer = 0; layer < model->num_layers; layer++) {
            AttentionLayer* attn = &model->attention_layers[layer];
            size_t weight_size = attn->num_heads * attn->head_dim * attn->head_dim;
            
            // For now, use simple gradient descent without Adam state for layer weights
            // TODO: Allocate separate Adam state for each layer type
            float* grad_q = training->attention_grads[layer].query_lattice;
            float* grad_k = training->attention_grads[layer].key_lattice;
            float* grad_v = training->attention_grads[layer].value_lattice;
            
            if (grad_q && attn->query_lattice) {
                for (size_t i = 0; i < weight_size; i++) {
                    attn->query_lattice[i] -= learning_rate * grad_q[i];
                }
            }
            
            if (grad_k && attn->key_lattice) {
                for (size_t i = 0; i < weight_size; i++) {
                    attn->key_lattice[i] -= learning_rate * grad_k[i];
                }
            }
            
            if (grad_v && attn->value_lattice) {
                for (size_t i = 0; i < weight_size; i++) {
                    attn->value_lattice[i] -= learning_rate * grad_v[i];
                }
            }
        }
    }
    
    // Update feed-forward layers
    if (training->ff_grads) {
        for (uint32_t layer = 0; layer < model->num_layers; layer++) {
            FeedForwardLayer* ff = &model->ff_layers[layer];
            
            // Update W1
            if (training->ff_grads[layer].w1_lattice && ff->w1_lattice) {
                size_t w1_size = ff->input_dim * ff->hidden_dim;
                for (size_t i = 0; i < w1_size; i++) {
                    ff->w1_lattice[i] -= learning_rate * training->ff_grads[layer].w1_lattice[i];
                }
            }
            
            // Update W2
            if (training->ff_grads[layer].w2_lattice && ff->w2_lattice) {
                size_t w2_size = ff->hidden_dim * ff->output_dim;
                for (size_t i = 0; i < w2_size; i++) {
                    ff->w2_lattice[i] -= learning_rate * training->ff_grads[layer].w2_lattice[i];
                }
            }
            
            // Update bias1
            if (training->ff_grads[layer].bias1 && ff->bias1) {
                for (uint32_t i = 0; i < ff->hidden_dim; i++) {
                    ff->bias1[i] -= learning_rate * training->ff_grads[layer].bias1[i];
                }
            }
            
            // Update bias2
            if (training->ff_grads[layer].bias2 && ff->bias2) {
                for (uint32_t i = 0; i < ff->output_dim; i++) {
                    ff->bias2[i] -= learning_rate * training->ff_grads[layer].bias2[i];
                }
            }
        }
    }
    
    // Update layer normalization
    if (training->ln_grads) {
        for (uint32_t layer = 0; layer < model->num_layers; layer++) {
            CLLMLayerNorm* ln = &model->layer_norms[layer];
            
            // Update gamma
            if (training->ln_grads[layer].gamma && ln->gamma) {
                for (uint32_t i = 0; i < ln->dim; i++) {
                    ln->gamma[i] -= learning_rate * training->ln_grads[layer].gamma[i];
                }
            }
            
            // Update beta
            if (training->ln_grads[layer].beta && ln->beta) {
                for (uint32_t i = 0; i < ln->dim; i++) {
                    ln->beta[i] -= learning_rate * training->ln_grads[layer].beta[i];
                }
            }
        }
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