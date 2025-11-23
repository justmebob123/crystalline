/**
 * Backward pass implementation for CLLM training
 * Computes gradients for all model parameters
 */

#include "cllm_training.h"
#include "cllm_utils.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

/**
 * Zero all gradient buffers before backward pass
 */
void cllm_zero_all_gradients(CLLMTraining* training) {
    if (!training || !training->model) return;
    
    CLLMModel* model = training->model;
    
    // Zero main gradient buffer (only embeddings are stored here)
    if (training->gradients) {
        size_t embed_size = model->vocab_size * model->embedding_dim;
        memset(training->gradients, 0, embed_size * sizeof(float));
    }
    
    // Zero attention gradients
    if (training->attention_grads) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            AttentionLayer* layer = &model->attention_layers[i];
            uint32_t dim = layer->num_heads * layer->head_dim;
            size_t lattice_size = dim * dim;
            
            if (training->attention_grads[i].query_lattice) {
                memset(training->attention_grads[i].query_lattice, 0, lattice_size * sizeof(float));
            }
            if (training->attention_grads[i].key_lattice) {
                memset(training->attention_grads[i].key_lattice, 0, lattice_size * sizeof(float));
            }
            if (training->attention_grads[i].value_lattice) {
                memset(training->attention_grads[i].value_lattice, 0, lattice_size * sizeof(float));
            }
        }
    }
    
    // Zero feed-forward gradients
    if (training->ff_grads) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            FeedForwardLayer* layer = &model->ff_layers[i];
            
            if (training->ff_grads[i].w1_lattice) {
                memset(training->ff_grads[i].w1_lattice, 0, 
                       layer->input_dim * layer->hidden_dim * sizeof(float));
            }
            if (training->ff_grads[i].w2_lattice) {
                memset(training->ff_grads[i].w2_lattice, 0, 
                       layer->hidden_dim * layer->output_dim * sizeof(float));
            }
            if (training->ff_grads[i].bias1) {
                memset(training->ff_grads[i].bias1, 0, layer->hidden_dim * sizeof(float));
            }
            if (training->ff_grads[i].bias2) {
                memset(training->ff_grads[i].bias2, 0, layer->output_dim * sizeof(float));
            }
        }
    }
    
    // Zero layer norm gradients
    if (training->ln_grads) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            CLLMLayerNorm* layer = &model->layer_norms[i];
            
            if (training->ln_grads[i].gamma) {
                memset(training->ln_grads[i].gamma, 0, layer->dim * sizeof(float));
            }
            if (training->ln_grads[i].beta) {
                memset(training->ln_grads[i].beta, 0, layer->dim * sizeof(float));
            }
        }
    }
}

/**
 * Backward pass through layer normalization
 */
static void backward_layer_norm(float* grad_out, float* grad_in, float* x, 
                                CLLMLayerNorm* ln, float* grad_gamma, float* grad_beta, uint64_t dim) {
    // CRITICAL: NULL pointer checks
    if (!grad_out || !grad_in || !x || !ln) return;
    if (!ln->gamma || !ln->beta) {
        fprintf(stderr, "ERROR: LayerNorm has NULL pointers!\n");
        fprintf(stderr, "  gamma=%p, beta=%p\n",
                (void*)ln->gamma, (void*)ln->beta);
        return;
    }
    
    // Compute mean and variance
    float mean = 0.0f, var = 0.0f;
    for (uint64_t i = 0; i < dim; i++) {
        mean += x[i];
    }
    mean /= dim;
    
    for (uint64_t i = 0; i < dim; i++) {
        float diff = x[i] - mean;
        var += diff * diff;
    }
    var /= dim;
    
    float std = sqrtf(var + 1e-5f);
    float inv_std = 1.0f / std;
    
    // Compute gradients
    float grad_var = 0.0f, grad_mean = 0.0f;
    
    for (uint64_t i = 0; i < dim; i++) {
        float x_norm = (x[i] - mean) * inv_std;
        
        // Gradient w.r.t. gamma and beta
        if (grad_gamma) grad_gamma[i] += grad_out[i] * x_norm;
        if (grad_beta) grad_beta[i] += grad_out[i];
        
        // Gradient w.r.t. normalized x
        float grad_x_norm = grad_out[i] * ln->gamma[i];
        
        // Accumulate for variance and mean gradients
        grad_var += grad_x_norm * (x[i] - mean) * -0.5f * inv_std * inv_std * inv_std;
        grad_mean += grad_x_norm * -inv_std;
    }
    
    // Gradient w.r.t. input
    for (uint64_t i = 0; i < dim; i++) {
        float grad_x_norm = grad_out[i] * ln->gamma[i];
        grad_in[i] = grad_x_norm * inv_std + 
                     grad_var * 2.0f * (x[i] - mean) / dim + 
                     grad_mean / dim;
    }
}

/**
 * Backward pass through feed-forward layer
 */
static void backward_feed_forward(float* grad_out, float* grad_in, float* x,
                                 FeedForwardLayer* ff, 
                                 float* grad_w1, float* grad_w2,
                                 float* grad_b1, float* grad_b2) {
    // CRITICAL: NULL pointer checks
    if (!grad_out || !grad_in || !x || !ff) return;
    if (!ff->w1_lattice || !ff->w2_lattice || !ff->bias1 || !ff->bias2) {
        fprintf(stderr, "ERROR: FeedForwardLayer has NULL pointers!\n");
        fprintf(stderr, "  w1_lattice=%p, w2_lattice=%p, bias1=%p, bias2=%p\n",
                (void*)ff->w1_lattice, (void*)ff->w2_lattice, 
                (void*)ff->bias1, (void*)ff->bias2);
        return;
    }
    
    int input_dim = ff->input_dim;
    int hidden_dim = ff->hidden_dim;
    int output_dim = ff->output_dim;
    
    // Allocate temporary buffers
    float* hidden = (float*)calloc(hidden_dim, sizeof(float));
    float* grad_hidden = (float*)calloc(hidden_dim, sizeof(float));
    
    if (!hidden || !grad_hidden) {
        free(hidden);
        free(grad_hidden);
        return;
    }
    
    // Forward pass to get hidden activations
    for (int h = 0; h < hidden_dim; h++) {
        float sum = ff->bias1[h];
        for (int i = 0; i < input_dim; i++) {
            sum += x[i] * ff->w1_lattice[i * hidden_dim + h];
        }
        hidden[h] = tanhf(sum);  // ReLU or tanh activation
    }
    
    // Backward through second layer
    for (int o = 0; o < output_dim; o++) {
        for (int h = 0; h < hidden_dim; h++) {
            // Gradient w.r.t. w2
            if (grad_w2) grad_w2[h * output_dim + o] += hidden[h] * grad_out[o];
            // Gradient w.r.t. hidden
            grad_hidden[h] += ff->w2_lattice[h * output_dim + o] * grad_out[o];
        }
        // Gradient w.r.t. bias2
        if (grad_b2) grad_b2[o] += grad_out[o];
    }
    
    // Backward through activation
    for (int h = 0; h < hidden_dim; h++) {
        float tanh_val = hidden[h];
        grad_hidden[h] *= (1.0f - tanh_val * tanh_val);  // tanh derivative
    }
    
    // Backward through first layer
    memset(grad_in, 0, input_dim * sizeof(float));
    for (int h = 0; h < hidden_dim; h++) {
        for (int i = 0; i < input_dim; i++) {
            // Gradient w.r.t. w1
            if (grad_w1) grad_w1[i * hidden_dim + h] += x[i] * grad_hidden[h];
            // Gradient w.r.t. input
            grad_in[i] += ff->w1_lattice[i * hidden_dim + h] * grad_hidden[h];
        }
        // Gradient w.r.t. bias1
        if (grad_b1) grad_b1[h] += grad_hidden[h];
    }
    
    free(hidden);
    free(grad_hidden);
}

/**
 * Backward pass through attention layer (simplified)
 */
static void backward_attention(float* grad_out, float* grad_in, float* x,
                              AttentionLayer* attn,
                              float* grad_query, float* grad_key, float* grad_value) {
    // CRITICAL: NULL pointer checks
    if (!grad_out || !grad_in || !x || !attn) return;
    if (!attn->query_lattice || !attn->key_lattice || !attn->value_lattice) {
        fprintf(stderr, "ERROR: AttentionLayer has NULL pointers!\n");
        fprintf(stderr, "  query_lattice=%p, key_lattice=%p, value_lattice=%p\n",
                (void*)attn->query_lattice, (void*)attn->key_lattice, 
                (void*)attn->value_lattice);
        return;
    }
    
    uint32_t dim = attn->num_heads * attn->head_dim;
    
    // Simplified: just compute gradients for Q, K, V projections
    // Full attention backward would require storing attention weights from forward pass
    
    // For now, approximate with identity + small update
    memcpy(grad_in, grad_out, dim * sizeof(float));
    
    // Accumulate gradients for projection matrices
    for (uint32_t i = 0; i < dim; i++) {
        for (uint32_t j = 0; j < dim; j++) {
            if (grad_query) grad_query[i * dim + j] += x[i] * grad_out[j] * 0.1f;
            if (grad_key) grad_key[i * dim + j] += x[i] * grad_out[j] * 0.1f;
            if (grad_value) grad_value[i * dim + j] += x[i] * grad_out[j] * 0.1f;
        }
    }
}

/**
 * Internal backward pass implementation
 */
static void cllm_backward_impl(CLLMTraining* training, uint32_t* input_tokens,
                               uint32_t* target_tokens, int batch_size, int seq_len) {
    if (!training || !training->model || !training->gradients) return;
    if (!input_tokens || !target_tokens || batch_size <= 0 || seq_len <= 0) return;
    
    CLLMModel* model = training->model;
    uint64_t embed_dim = model->embedding_dim;
    uint32_t num_layers = model->num_layers;
    
    // Zero all gradients
    cllm_zero_all_gradients(training);
    
    // Use pre-allocated buffers (OPTIMIZATION - no malloc/free overhead)
    float* embeddings = training->backward_embeddings;
    float* grad_output = training->backward_grad_output;
    float* layer_input = training->backward_layer_input;
    float* layer_grad = training->backward_layer_grad;
    float* temp_grad = training->backward_temp_grad;
    
    // Zero buffers before use
    size_t activation_size = batch_size * seq_len * embed_dim;
    
    // CRITICAL: Check buffer size to prevent overflow
    if (activation_size > training->backward_buffer_size) {
        fprintf(stderr, "ERROR: Activation size (%zu) exceeds buffer size (%zu)\n",
                activation_size, training->backward_buffer_size);
        fprintf(stderr, "  batch_size=%d, seq_len=%d, embed_dim=%lu\n",
                batch_size, seq_len, embed_dim);
        return;
    }
    
    memset(embeddings, 0, activation_size * sizeof(float));
    memset(grad_output, 0, activation_size * sizeof(float));
    memset(layer_input, 0, embed_dim * sizeof(float));
    memset(layer_grad, 0, embed_dim * sizeof(float));
    memset(temp_grad, 0, embed_dim * sizeof(float));
    
    // Simplified backward pass - just compute gradients for embeddings and layers
    // For a proper implementation, we would need to store activations from forward pass
    
    // CRITICAL: Check if model layers are initialized
    if (!model->ff_layers || !model->attention_layers || !model->layer_norms) {
        fprintf(stderr, "ERROR: Model layers are NULL!\n");
        fprintf(stderr, "  ff_layers=%p, attention_layers=%p, layer_norms=%p\n",
                (void*)model->ff_layers, (void*)model->attention_layers,
                (void*)model->layer_norms);
        return;
    }
    
    if (!model->embeddings.embeddings) {
        fprintf(stderr, "ERROR: Model embeddings are NULL!\n");
        return;
    }
    
    // Process each sequence in batch
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t token_id = input_tokens[idx];
            uint32_t target_id = target_tokens[idx];
            
            if (token_id >= model->vocab_size || target_id >= model->vocab_size) continue;
            
            // Get embedding
            float* embed_src = &model->embeddings.embeddings[token_id * embed_dim];
            memcpy(layer_input, embed_src, embed_dim * sizeof(float));
            
            // Compute simple loss gradient (MSE with target embedding)
            float* target_embed = &model->embeddings.embeddings[target_id * embed_dim];
            for (uint64_t d = 0; d < embed_dim; d++) {
                layer_grad[d] = 2.0f * (layer_input[d] - target_embed[d]) / (batch_size * seq_len);
            }
            
            // Backward through layers (simplified - using input embedding as proxy for layer activations)
            for (int layer = num_layers - 1; layer >= 0; layer--) {
                // Backward through feed-forward
                backward_feed_forward(layer_grad, temp_grad, layer_input,
                                    &model->ff_layers[layer],
                                    training->ff_grads[layer].w1_lattice,
                                    training->ff_grads[layer].w2_lattice,
                                    training->ff_grads[layer].bias1,
                                    training->ff_grads[layer].bias2);
                memcpy(layer_grad, temp_grad, embed_dim * sizeof(float));
                
                // Backward through attention
                backward_attention(layer_grad, temp_grad, layer_input,
                                 &model->attention_layers[layer],
                                 training->attention_grads[layer].query_lattice,
                                 training->attention_grads[layer].key_lattice,
                                 training->attention_grads[layer].value_lattice);
                memcpy(layer_grad, temp_grad, embed_dim * sizeof(float));
                
                // Backward through layer norm
                backward_layer_norm(layer_grad, temp_grad, layer_input,
                                  &model->layer_norms[layer],
                                  training->ln_grads[layer].gamma,
                                  training->ln_grads[layer].beta,
                                  embed_dim);
                memcpy(layer_grad, temp_grad, embed_dim * sizeof(float));
            }
            
            // Accumulate embedding gradients
            float* grad_embed = &training->gradients[token_id * embed_dim];
            for (uint64_t d = 0; d < embed_dim; d++) {
                grad_embed[d] += layer_grad[d];
            }
        }
    }
    
    // No need to free - using pre-allocated buffers (OPTIMIZATION)
}

/**
 * Public backward pass interface
 * Wrapper that extracts batch_size and seq_len from config
 */
void cllm_backward(CLLMTraining* training, uint32_t* input_tokens, 
                   uint32_t* target_tokens, int num_tokens) {
    if (!training || !input_tokens || !target_tokens) return;
    
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    
    if (num_tokens < batch_size * seq_len) {
        cllm_zero_all_gradients(training);
        return;
    }
    
    cllm_backward_impl(training, input_tokens, target_tokens, batch_size, seq_len);
}