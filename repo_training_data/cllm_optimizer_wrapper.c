/**
 * Optimizer Wrapper with Gradient Accumulation Support
 * Combines gradient accumulation with Adam optimizer
 */

#include "../include/cllm_training.h"
#include "../include/prime_float_math.h"
#include <string.h>

// Forward declaration of Adam optimizer
extern void cllm_adam_step(CLLMTraining* training, float learning_rate);

/**
 * Optimizer step with gradient accumulation and Adam
 * This replaces the simple SGD in cllm_training.c
 */
void cllm_optimizer_step_adam(CLLMTraining* training) {
    if (!training) return;
    
    // Gradient accumulation logic
    int accum_steps = training->config.gradient_accumulation_steps;
    if (accum_steps <= 0) accum_steps = 1;
    
    training->accumulation_step++;
    
    // Only apply gradients when we've accumulated enough steps
    if (training->accumulation_step < accum_steps) {
        return;  // Continue accumulating
    }
    
    // Reset accumulation counter
    training->accumulation_step = 0;
    
    // Scale gradients by 1/accum_steps
    float gradient_scale = 1.0f / (float)accum_steps;
    
    CLLMModel* model = training->model;
    
    // Scale embedding gradients
    size_t embed_size = model->vocab_size * model->embedding_dim;
    if (training->gradients) {
        for (size_t i = 0; i < embed_size; i++) {
            training->gradients[i] *= gradient_scale;
        }
    }
    
    // Scale attention gradients
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        if (training->attention_grads) {
            uint64_t attn_size = model->embedding_dim * model->embedding_dim;
            
            if (training->attention_grads[layer].query_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    training->attention_grads[layer].query_lattice[i] *= gradient_scale;
                }
            }
            if (training->attention_grads[layer].key_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    training->attention_grads[layer].key_lattice[i] *= gradient_scale;
                }
            }
            if (training->attention_grads[layer].value_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    training->attention_grads[layer].value_lattice[i] *= gradient_scale;
                }
            }
        }
        
        // Scale feed-forward gradients
        if (training->ff_grads && model->ff_layers) {
            FeedForwardLayer* ff = &model->ff_layers[layer];
            
            if (training->ff_grads[layer].w1_lattice) {
                size_t w1_size = ff->input_dim * ff->hidden_dim;
                for (size_t i = 0; i < w1_size; i++) {
                    training->ff_grads[layer].w1_lattice[i] *= gradient_scale;
                }
            }
            
            if (training->ff_grads[layer].w2_lattice) {
                size_t w2_size = ff->hidden_dim * ff->output_dim;
                for (size_t i = 0; i < w2_size; i++) {
                    training->ff_grads[layer].w2_lattice[i] *= gradient_scale;
                }
            }
            
            if (training->ff_grads[layer].bias1) {
                for (uint32_t i = 0; i < ff->hidden_dim; i++) {
                    training->ff_grads[layer].bias1[i] *= gradient_scale;
                }
            }
            
            if (training->ff_grads[layer].bias2) {
                for (uint32_t i = 0; i < ff->output_dim; i++) {
                    training->ff_grads[layer].bias2[i] *= gradient_scale;
                }
            }
        }
    }
    
    // Use the proper Adam optimizer from cllm_optimizer.c
    // This provides momentum, adaptive learning rates, and bias correction
    cllm_adam_step(training, training->config.learning_rate);
}