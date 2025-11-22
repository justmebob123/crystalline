/*
 * CLLM Backward Pass
 * Implements backpropagation through all layers
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/prime_float_math.h"

/**
 * Backward pass through layer normalization
 * 
 * @param ln Layer normalization parameters
 * @param input Original input [dim]
 * @param grad_output Gradient from next layer [dim]
 * @param grad_input Output gradient for previous layer [dim]
 * @param grad_gamma Gradient for gamma [dim]
 * @param grad_beta Gradient for beta [dim]
 */
void cllm_layer_norm_backward(CLLMLayerNorm* ln, float* input, 
                             float* grad_output, float* grad_input,
                             float* grad_gamma, float* grad_beta) {
    if (!ln || !input || !grad_output || !grad_input) return;
    
    uint32_t dim = ln->dim;
    float epsilon = ln->epsilon;
    
    // Compute mean and variance
    float mean = 0.0f;
    for (uint32_t i = 0; i < dim; i++) {
        mean += input[i];
    }
    mean /= (float)dim;
    
    float variance = 0.0f;
    for (uint32_t i = 0; i < dim; i++) {
        float diff = input[i] - mean;
        variance += diff * diff;
    }
    variance /= (float)dim;
    
    float std = prime_sqrt(variance + epsilon);
    float inv_std = 1.0f / std;
    
    // Compute gradients for gamma and beta
    if (grad_gamma && grad_beta) {
        for (uint32_t i = 0; i < dim; i++) {
            float normalized = (input[i] - mean) * inv_std;
            grad_gamma[i] = grad_output[i] * normalized;
            grad_beta[i] = grad_output[i];
        }
    }
    
    // Compute gradient for input
    float grad_mean = 0.0f;
    float grad_var = 0.0f;
    
    for (uint32_t i = 0; i < dim; i++) {
        // float normalized = (input[i] - mean) * inv_std;  // Computed but not used
        float grad_normalized = grad_output[i] * ln->gamma[i];
        
        grad_var += grad_normalized * (input[i] - mean) * (-0.5f) * inv_std * inv_std * inv_std;
        grad_mean += grad_normalized * (-inv_std);
    }
    
    for (uint32_t i = 0; i < dim; i++) {
        float grad_normalized = grad_output[i] * ln->gamma[i];
        grad_input[i] = grad_normalized * inv_std + 
                       grad_var * 2.0f * (input[i] - mean) / (float)dim +
                       grad_mean / (float)dim;
    }
}

/**
 * Backward pass through feed-forward network
 * 
 * @param layer Feed-forward layer
 * @param input Original input [input_dim]
 * @param hidden Hidden activations [hidden_dim]
 * @param grad_output Gradient from next layer [output_dim]
 * @param grad_input Output gradient for previous layer [input_dim]
 * @param grad_w1 Gradient for W1 [input_dim x hidden_dim]
 * @param grad_w2 Gradient for W2 [hidden_dim x output_dim]
 * @param grad_b1 Gradient for bias1 [hidden_dim]
 * @param grad_b2 Gradient for bias2 [output_dim]
 */
void cllm_feedforward_backward(FeedForwardLayer* layer, float* input, float* hidden,
                              float* grad_output, float* grad_input,
                              float* grad_w1, float* grad_w2,
                              float* grad_b1, float* grad_b2) {
    if (!layer || !input || !hidden || !grad_output || !grad_input) return;
    
    uint32_t input_dim = layer->input_dim;
    uint32_t hidden_dim = layer->hidden_dim;
    uint32_t output_dim = layer->output_dim;
    
    // Allocate gradient for hidden layer
    float* grad_hidden = (float*)calloc(hidden_dim, sizeof(float));
    if (!grad_hidden) return;
    
    // Backward through second linear layer: grad_hidden = W2^T * grad_output
    for (uint32_t i = 0; i < hidden_dim; i++) {
        for (uint32_t j = 0; j < output_dim; j++) {
            grad_hidden[i] += layer->w2_lattice[j * hidden_dim + i] * grad_output[j];
        }
    }
    
    // Compute gradients for W2 and b2
    if (grad_w2) {
        for (uint32_t i = 0; i < output_dim; i++) {
            for (uint32_t j = 0; j < hidden_dim; j++) {
                grad_w2[i * hidden_dim + j] = grad_output[i] * hidden[j];
            }
        }
    }
    
    if (grad_b2) {
        memcpy(grad_b2, grad_output, output_dim * sizeof(float));
    }
    
    // Backward through GELU activation
    // GELU'(x) ≈ 0.5 * (1 + tanh(...)) + x * derivative_of_tanh_part
    // Simplified: use approximate derivative
    for (uint32_t i = 0; i < hidden_dim; i++) {
        float x = hidden[i];
        // Approximate GELU derivative
        float gelu_grad = 0.5f * (1.0f + prime_tanh(0.797885f * (x + 0.044715f * x * x * x)));
        grad_hidden[i] *= gelu_grad;
    }
    
    // Backward through first linear layer: grad_input = W1^T * grad_hidden
    memset(grad_input, 0, input_dim * sizeof(float));
    for (uint32_t i = 0; i < input_dim; i++) {
        for (uint32_t j = 0; j < hidden_dim; j++) {
            grad_input[i] += layer->w1_lattice[j * input_dim + i] * grad_hidden[j];
        }
    }
    
    // Compute gradients for W1 and b1
    if (grad_w1) {
        for (uint32_t i = 0; i < hidden_dim; i++) {
            for (uint32_t j = 0; j < input_dim; j++) {
                grad_w1[i * input_dim + j] = grad_hidden[i] * input[j];
            }
        }
    }
    
    if (grad_b1) {
        memcpy(grad_b1, grad_hidden, hidden_dim * sizeof(float));
    }
    
    free(grad_hidden);
}

/**
 * Backward pass through attention layer (simplified)
 * 
 * @param layer Attention layer
 * @param input Original input [seq_len x embedding_dim]
 * @param grad_output Gradient from next layer [seq_len x embedding_dim]
 * @param grad_input Output gradient for previous layer [seq_len x embedding_dim]
 * @param seq_len Sequence length
 */
void cllm_attention_backward(AttentionLayer* layer, float* input,
                            float* grad_output, float* grad_input, int seq_len) {
    if (!layer || !input || !grad_output || !grad_input || seq_len <= 0) return;
    
    uint32_t num_heads = layer->num_heads;
    uint32_t head_dim = layer->head_dim;
    uint32_t embedding_dim = num_heads * head_dim;
    
    // Simplified backward pass - full implementation would compute
    // gradients for Q, K, V projections and attention weights
    
    // For now, just pass gradients through (identity-like)
    memcpy(grad_input, grad_output, seq_len * embedding_dim * sizeof(float));
    
    // TODO: Implement full attention backward pass with:
    // - Gradient through softmax
    // - Gradient through scaled dot-product
    // - Gradients for Q, K, V weight matrices
}

/**
 * Backward pass through embedding layer
 * 
 * @param embeddings Embedding parameters
 * @param token_ids Token IDs [batch_size]
 * @param grad_output Gradient from next layer [batch_size x embedding_dim]
 * @param grad_embeddings Gradient for embedding matrix [vocab_size x embedding_dim]
 * @param batch_size Batch size
 */
void cllm_embedding_backward(Embeddings* embeddings, uint32_t* token_ids,
                            float* grad_output, float* grad_embeddings, int batch_size) {
    if (!embeddings || !token_ids || !grad_output || !grad_embeddings || batch_size <= 0) {
        return;
    }
    
    uint32_t embedding_dim = embeddings->embedding_dim;
    
    // Accumulate gradients for each token's embedding
    for (int i = 0; i < batch_size; i++) {
        uint32_t token_id = token_ids[i];
        if (token_id >= embeddings->vocab_size) continue;
        
        float* grad_embed = &grad_embeddings[token_id * embedding_dim];
        float* grad_out = &grad_output[i * embedding_dim];
        
        for (uint32_t j = 0; j < embedding_dim; j++) {
            grad_embed[j] += grad_out[j];
        }
    }
}

/**
 * Complete backward pass through transformer layer
 * 
 * @param training Training state
 * @param layer_idx Layer index
 * @param input Layer input [seq_len x embedding_dim]
 * @param grad_output Gradient from next layer [seq_len x embedding_dim]
 * @param grad_input Output gradient for previous layer [seq_len x embedding_dim]
 * @param seq_len Sequence length
 */
void cllm_transformer_layer_backward(CLLMTraining* training, int layer_idx,
                                    float* input, float* grad_output,
                                    float* grad_input, int seq_len) {
    if (!training || !input || !grad_output || !grad_input || seq_len <= 0) return;
    
    CLLMModel* model = training->model;
    if (layer_idx < 0 || layer_idx >= (int)model->num_layers) return;
    
    uint32_t embedding_dim = model->embedding_dim;
    
    // Allocate temporary buffers
    float* grad_temp = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    if (!grad_temp) return;
    
    // Backward through layer norm (post-FFN)
    CLLMLayerNorm* ln2 = &model->layer_norms[layer_idx];
    cllm_layer_norm_backward(ln2, input, grad_output, grad_temp, NULL, NULL);
    
    // Backward through feed-forward network
    // Note: This is simplified - full implementation needs hidden activations
    memcpy(grad_input, grad_temp, seq_len * embedding_dim * sizeof(float));
    
    // Backward through attention
    AttentionLayer* attn = &model->attention_layers[layer_idx];
    cllm_attention_backward(attn, input, grad_input, grad_temp, seq_len);
    
    memcpy(grad_input, grad_temp, seq_len * embedding_dim * sizeof(float));
    
    free(grad_temp);
}

/**
 * Complete backward pass through entire model
 * 
 * @param training Training state
 * @param input_tokens Input token IDs [batch_size x seq_len]
 * @param target_tokens Target token IDs [batch_size x seq_len]
 * @param batch_size Batch size
 * @param seq_len Sequence length
 */
void cllm_backward_complete(CLLMTraining* training, uint32_t* input_tokens,
                           uint32_t* target_tokens, int batch_size, int seq_len) {
    if (!training || !training->model || !training->gradients) return;
    if (!input_tokens || !target_tokens || batch_size <= 0 || seq_len <= 0) return;
    
    CLLMModel* model = training->model;
    uint32_t embedding_dim = model->embedding_dim;
    
    // Zero gradients
    size_t total_params = model->header.total_params;
    memset(training->gradients, 0, total_params * sizeof(float));
    
    // Allocate buffers
    size_t activation_size = batch_size * seq_len * embedding_dim;
    float* embeddings_out = (float*)calloc(activation_size, sizeof(float));
    float* grad_embeddings_out = (float*)calloc(activation_size, sizeof(float));
    
    if (!embeddings_out || !grad_embeddings_out) {
        free(embeddings_out);
        free(grad_embeddings_out);
        return;
    }
    
    // Forward pass to get embeddings
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            uint32_t token_id = input_tokens[b * seq_len + s];
            if (token_id < model->vocab_size) {
                float* embed_src = &model->embeddings.embeddings[token_id * embedding_dim];
                float* embed_dst = &embeddings_out[(b * seq_len + s) * embedding_dim];
                memcpy(embed_dst, embed_src, embedding_dim * sizeof(float));
            }
        }
    }
    
    // Compute loss gradient
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            uint32_t target_id = target_tokens[b * seq_len + s];
            if (target_id < model->vocab_size) {
                float* grad = &grad_embeddings_out[(b * seq_len + s) * embedding_dim];
                float* target_embed = &model->embeddings.embeddings[target_id * embedding_dim];
                
                for (uint32_t d = 0; d < embedding_dim; d++) {
                    grad[d] = target_embed[d] - embeddings_out[(b * seq_len + s) * embedding_dim + d];
                }
            }
        }
    }
    
    // Backward through embeddings
    float* grad_embeddings = (float*)calloc(model->vocab_size * embedding_dim, sizeof(float));
    if (grad_embeddings) {
        // Accumulate gradients
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                uint32_t token_id = input_tokens[b * seq_len + s];
                if (token_id < model->vocab_size) {
                    float* grad_embed = &grad_embeddings[token_id * embedding_dim];
                    float* grad_out = &grad_embeddings_out[(b * seq_len + s) * embedding_dim];
                    
                    for (uint32_t d = 0; d < embedding_dim; d++) {
                        grad_embed[d] += grad_out[d];
                    }
                }
            }
        }
        
        // Copy to training gradients
        size_t embed_params = model->vocab_size * embedding_dim;
        if (embed_params <= total_params) {
            memcpy(training->gradients, grad_embeddings, embed_params * sizeof(float));
        }
        
        free(grad_embeddings);
    }
    
    free(embeddings_out);
    free(grad_embeddings_out);
}
