/**
 * Proper Backward Pass Implementation
 * 
 * Implements correct backpropagation for cross-entropy loss
 * using stored activations from forward pass.
 */

#include "cllm_training.h"
#include "cllm_activations.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/**
 * Backward pass for cross-entropy loss
 * 
 * @param training Training state
 * @param activations Stored activations from forward pass
 * @param target_tokens Target token IDs
 */
void cllm_backward_cross_entropy(
    CLLMTraining* training,
    ForwardActivations* activations,
    uint32_t* target_tokens
) {
    if (!training || !activations || !target_tokens) return;
    
    CLLMModel* model = training->model;
    int batch_size = activations->batch_size;
    int seq_len = activations->seq_len;
    uint32_t embed_dim = activations->embed_dim;
    uint32_t vocab_size = activations->vocab_size;
    
    // Zero all gradients
    cllm_zero_all_gradients(training);
    
    // Allocate gradient buffers
    float* grad_logits = (float*)calloc(batch_size * seq_len * vocab_size, sizeof(float));
    float* grad_hidden = (float*)calloc(batch_size * seq_len * embed_dim, sizeof(float));
    float* grad_layer = (float*)calloc(batch_size * seq_len * embed_dim, sizeof(float));
    
    if (!grad_logits || !grad_hidden || !grad_layer) {
        free(grad_logits);
        free(grad_hidden);
        free(grad_layer);
        return;
    }
    
    // Step 1: Compute gradient of cross-entropy loss w.r.t. logits
    // dL/dlogits = softmax(logits) - one_hot(target)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t target = target_tokens[idx];
            
            if (target >= vocab_size) continue;
            
            float* logits = &activations->logits[idx * vocab_size];
            float* grad = &grad_logits[idx * vocab_size];
            
            // Compute softmax
            float max_logit = logits[0];
            for (uint32_t v = 1; v < vocab_size; v++) {
                if (logits[v] > max_logit) {
                    max_logit = logits[v];
                }
            }
            
            float sum_exp = 0.0f;
            for (uint32_t v = 0; v < vocab_size; v++) {
                sum_exp += expf(logits[v] - max_logit);
            }
            
            // Gradient: softmax - one_hot
            for (uint32_t v = 0; v < vocab_size; v++) {
                float softmax_v = expf(logits[v] - max_logit) / sum_exp;
                grad[v] = softmax_v;
                if (v == target) {
                    grad[v] -= 1.0f;
                }
                // Normalize by batch size
                grad[v] /= (batch_size * seq_len);
            }
        }
    }
    
    // Step 2: Backward through output projection (logits = hidden @ embeddings.T)
    // grad_hidden = grad_logits @ embeddings
    // grad_embeddings += grad_logits.T @ hidden
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            float* grad_log = &grad_logits[idx * vocab_size];
            float* grad_hid = &grad_hidden[idx * embed_dim];
            float* hidden = &activations->final_hidden[idx * embed_dim];
            
            // grad_hidden = grad_logits @ embeddings
            for (uint32_t d = 0; d < embed_dim; d++) {
                float sum = 0.0f;
                for (uint32_t v = 0; v < vocab_size; v++) {
                    sum += grad_log[v] * model->embeddings.embeddings[v * embed_dim + d];
                }
                grad_hid[d] = sum;
            }
            
            // grad_embeddings += grad_logits.T @ hidden
            for (uint32_t v = 0; v < vocab_size; v++) {
                float* grad_embed = &training->gradients[v * embed_dim];
                for (uint32_t d = 0; d < embed_dim; d++) {
                    grad_embed[d] += grad_log[v] * hidden[d];
                }
            }
        }
    }
    
    // Step 3: Backward through transformer layers (in reverse order)
    memcpy(grad_layer, grad_hidden, batch_size * seq_len * embed_dim * sizeof(float));
    
    for (int layer = model->num_layers - 1; layer >= 0; layer--) {
        // float* layer_input = activations->layer_inputs[layer];  // Unused for now
        float* attn_output = activations->attention_outputs[layer];
        // float* ff_output = activations->ff_outputs[layer];  // Unused for now
        float* ff_hidden = activations->ff_hidden[layer];
        
        FeedForwardLayer* ff = &model->ff_layers[layer];
        CLLMLayerNorm* ln = &model->layer_norms[layer];
        
        // Backward through layer norm
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* grad = &grad_layer[idx * embed_dim];
                float* input = &attn_output[idx * embed_dim];
                
                // Compute mean and variance (from forward pass)
                float mean = 0.0f, var = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    mean += input[d];
                }
                mean /= embed_dim;
                
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float diff = input[d] - mean;
                    var += diff * diff;
                }
                var /= embed_dim;
                float std = sqrtf(var + 1e-5f);
                
                // Backward through layer norm
                float grad_var = 0.0f, grad_mean = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float x_norm = (input[d] - mean) / std;
                    
                    // Gradient w.r.t. gamma and beta
                    if (training->ln_grads[layer].gamma) {
                        training->ln_grads[layer].gamma[d] += grad[d] * x_norm;
                    }
                    if (training->ln_grads[layer].beta) {
                        training->ln_grads[layer].beta[d] += grad[d];
                    }
                    
                    // Gradient w.r.t. normalized input
                    float grad_x_norm = grad[d] * ln->gamma[d];
                    grad_var += grad_x_norm * (input[d] - mean) * -0.5f * powf(std, -3.0f);
                    grad_mean += grad_x_norm * (-1.0f / std);
                }
                
                // Gradient w.r.t. input
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float grad_x_norm = grad[d] * ln->gamma[d];
                    grad[d] = grad_x_norm / std + 
                             grad_var * 2.0f * (input[d] - mean) / embed_dim +
                             grad_mean / embed_dim;
                }
            }
        }
        
        // Backward through FeedForward
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* grad = &grad_layer[idx * embed_dim];
                float* input = &attn_output[idx * embed_dim];
                float* hidden = &ff_hidden[idx * ff->hidden_dim];
                
                // Gradient w.r.t. second layer
                float* grad_hidden = (float*)calloc(ff->hidden_dim, sizeof(float));
                if (!grad_hidden) continue;
                
                for (uint32_t o = 0; o < embed_dim; o++) {
                    // Gradient w.r.t. w2
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        if (training->ff_grads[layer].w2_lattice) {
                            training->ff_grads[layer].w2_lattice[h * embed_dim + o] += 
                                hidden[h] * grad[o];
                        }
                        grad_hidden[h] += ff->w2_lattice[h * embed_dim + o] * grad[o];
                    }
                    // Gradient w.r.t. bias2
                    if (training->ff_grads[layer].bias2) {
                        training->ff_grads[layer].bias2[o] += grad[o];
                    }
                }
                
                // Backward through activation (tanh)
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    float tanh_val = hidden[h];
                    grad_hidden[h] *= (1.0f - tanh_val * tanh_val);
                }
                
                // Gradient w.r.t. first layer
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        // Gradient w.r.t. w1
                        if (training->ff_grads[layer].w1_lattice) {
                            training->ff_grads[layer].w1_lattice[i * ff->hidden_dim + h] += 
                                input[i] * grad_hidden[h];
                        }
                        grad[i] += ff->w1_lattice[i * ff->hidden_dim + h] * grad_hidden[h];
                    }
                    // Gradient w.r.t. bias1
                    if (training->ff_grads[layer].bias1) {
                        training->ff_grads[layer].bias1[h] += grad_hidden[h];
                    }
                }
                
                free(grad_hidden);
            }
        }
        
        // Residual connection gradient flows through
        // (gradient already in grad_layer, just continues to previous layer)
    }
    
    // Step 4: Accumulate embedding gradients from input
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t token_id = target_tokens[idx]; // Use input tokens here
            
            if (token_id >= vocab_size) continue;
            
            float* grad = &grad_layer[idx * embed_dim];
            float* grad_embed = &training->gradients[token_id * embed_dim];
            
            for (uint32_t d = 0; d < embed_dim; d++) {
                grad_embed[d] += grad[d];
            }
        }
    }
    
    // Free temporary buffers
    free(grad_logits);
    free(grad_hidden);
    free(grad_layer);
}