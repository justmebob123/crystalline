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
    
    // Accumulate gradients for gamma and beta
    if (grad_gamma && grad_beta) {
        for (uint32_t i = 0; i < dim; i++) {
            float normalized = (input[i] - mean) * inv_std;
            grad_gamma[i] += grad_output[i] * normalized;
            grad_beta[i] += grad_output[i];
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
    
    // Accumulate gradients for W2 and b2
    if (grad_w2) {
        for (uint32_t i = 0; i < output_dim; i++) {
            for (uint32_t j = 0; j < hidden_dim; j++) {
                grad_w2[i * hidden_dim + j] += grad_output[i] * hidden[j];
            }
        }
    }
    
    if (grad_b2) {
        for (uint32_t i = 0; i < output_dim; i++) {
            grad_b2[i] += grad_output[i];
        }
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
    
    // Accumulate gradients for W1 and b1
    if (grad_w1) {
        for (uint32_t i = 0; i < hidden_dim; i++) {
            for (uint32_t j = 0; j < input_dim; j++) {
                grad_w1[i * input_dim + j] += grad_hidden[i] * input[j];
            }
        }
    }
    
    if (grad_b1) {
        for (uint32_t i = 0; i < hidden_dim; i++) {
            grad_b1[i] += grad_hidden[i];
        }
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
/**
 * Softmax backward pass
 * 
 * Given softmax output y and gradient dy, compute dx where y = softmax(x)
 * 
 * @param softmax_output Softmax output [size]
 * @param grad_output Gradient w.r.t. output [size]
 * @param grad_input Output gradient w.r.t. input [size]
 * @param size Array size
 */
static void softmax_backward(float* softmax_output, float* grad_output,
                            float* grad_input, int size) {
    if (!softmax_output || !grad_output || !grad_input || size <= 0) return;
    
    // Compute sum of (grad_output * softmax_output)
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += grad_output[i] * softmax_output[i];
    }
    
    // Compute gradient: grad_input[i] = softmax_output[i] * (grad_output[i] - sum)
    for (int i = 0; i < size; i++) {
        grad_input[i] = softmax_output[i] * (grad_output[i] - sum);
    }
}

/**
 * Scaled dot-product attention backward pass
 * 
 * Computes gradients for query, keys, and values given gradient w.r.t. output
 * 
 * @param query Query vector [head_dim]
 * @param keys Key matrix [seq_len x head_dim]
 * @param values Value matrix [seq_len x head_dim]
 * @param attention_weights Attention weights from forward pass [seq_len]
 * @param grad_output Gradient w.r.t. output [head_dim]
 * @param grad_query Output gradient for query [head_dim]
 * @param grad_keys Output gradient for keys [seq_len x head_dim]
 * @param grad_values Output gradient for values [seq_len x head_dim]
 * @param head_dim Dimension per head
 * @param seq_len Sequence length
 */
static void scaled_dot_product_attention_backward(
    float* query, float* keys, float* values, float* attention_weights,
    float* grad_output, float* grad_query, float* grad_keys, float* grad_values,
    int head_dim, int seq_len) {
    
    if (!query || !keys || !values || !attention_weights || !grad_output ||
        !grad_query || !grad_keys || !grad_values || head_dim <= 0 || seq_len <= 0) {
        return;
    }
    
    float scale = 1.0f / prime_sqrt((float)head_dim);
    
    // Allocate temporary buffers
    float* grad_attention = (float*)calloc(seq_len, sizeof(float));
    float* grad_scores = (float*)calloc(seq_len, sizeof(float));
    
    if (!grad_attention || !grad_scores) {
        free(grad_attention);
        free(grad_scores);
        return;
    }
    
    // Gradient w.r.t. values: grad_values[i] = attention_weights[i] * grad_output
    for (int i = 0; i < seq_len; i++) {
        for (int j = 0; j < head_dim; j++) {
            grad_values[i * head_dim + j] += attention_weights[i] * grad_output[j];
        }
    }
    
    // Gradient w.r.t. attention weights: grad_attention[i] = sum(values[i] * grad_output)
    for (int i = 0; i < seq_len; i++) {
        float sum = 0.0f;
        for (int j = 0; j < head_dim; j++) {
            sum += values[i * head_dim + j] * grad_output[j];
        }
        grad_attention[i] = sum;
    }
    
    // Gradient through softmax
    softmax_backward(attention_weights, grad_attention, grad_scores, seq_len);
    
    // Gradient w.r.t. scaled scores (before softmax)
    for (int i = 0; i < seq_len; i++) {
        grad_scores[i] *= scale;
    }
    
    // Gradient w.r.t. query: grad_query = sum(grad_scores[i] * keys[i])
    memset(grad_query, 0, head_dim * sizeof(float));
    for (int i = 0; i < seq_len; i++) {
        for (int j = 0; j < head_dim; j++) {
            grad_query[j] += grad_scores[i] * keys[i * head_dim + j];
        }
    }
    
    // Gradient w.r.t. keys: grad_keys[i] = grad_scores[i] * query
    for (int i = 0; i < seq_len; i++) {
        for (int j = 0; j < head_dim; j++) {
            grad_keys[i * head_dim + j] += grad_scores[i] * query[j];
        }
    }
    
    free(grad_attention);
    free(grad_scores);
}

/**
 * Complete attention backward pass with weight gradients
 * 
 * @param layer Attention layer
 * @param input Original input [seq_len x embedding_dim]
 * @param grad_output Gradient from next layer [seq_len x embedding_dim]
 * @param grad_input Output gradient for previous layer [seq_len x embedding_dim]
 * @param grad_query_weights Gradient accumulator for query weights (can be NULL)
 * @param grad_key_weights Gradient accumulator for key weights (can be NULL)
 * @param grad_value_weights Gradient accumulator for value weights (can be NULL)
 * @param seq_len Sequence length
 */
void cllm_attention_backward(AttentionLayer* layer, float* input,
                            float* grad_output, float* grad_input,
                            float* grad_query_weights, float* grad_key_weights, float* grad_value_weights,
                            int seq_len) {
    if (!layer || !input || !grad_output || !grad_input || seq_len <= 0) return;
    
    uint32_t num_heads = layer->num_heads;
    uint32_t head_dim = layer->head_dim;
    uint32_t embedding_dim = num_heads * head_dim;
    
    // Allocate buffers for forward pass intermediate values
    float* queries = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* keys = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* values = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* attention_weights = (float*)calloc(seq_len * seq_len * num_heads, sizeof(float));
    
    // Allocate buffers for gradients
    float* grad_queries = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* grad_keys = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* grad_values = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    if (!queries || !keys || !values || !attention_weights ||
        !grad_queries || !grad_keys || !grad_values) {
        free(queries);
        free(keys);
        free(values);
        free(attention_weights);
        free(grad_queries);
        free(grad_keys);
        free(grad_values);
        return;
    }
    
    // Re-compute forward pass to get intermediate values
    // Project input to Q, K, V
    for (int pos = 0; pos < seq_len; pos++) {
        float* input_vec = &input[pos * embedding_dim];
        
        // Query projection
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                float sum = 0.0f;
                for (uint32_t i = 0; i < head_dim; i++) {
                    size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                    sum += layer->query_lattice[weight_idx] * input_vec[h * head_dim + i];
                }
                queries[pos * embedding_dim + h * head_dim + d] = sum;
            }
        }
        
        // Key projection
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                float sum = 0.0f;
                for (uint32_t i = 0; i < head_dim; i++) {
                    size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                    sum += layer->key_lattice[weight_idx] * input_vec[h * head_dim + i];
                }
                keys[pos * embedding_dim + h * head_dim + d] = sum;
            }
        }
        
        // Value projection
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                float sum = 0.0f;
                for (uint32_t i = 0; i < head_dim; i++) {
                    size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                    sum += layer->value_lattice[weight_idx] * input_vec[h * head_dim + i];
                }
                values[pos * embedding_dim + h * head_dim + d] = sum;
            }
        }
    }
    
    // Compute attention weights for backward pass
    float scale = 1.0f / prime_sqrt((float)head_dim);
    for (int pos = 0; pos < seq_len; pos++) {
        for (uint32_t h = 0; h < num_heads; h++) {
            float* query = &queries[pos * embedding_dim + h * head_dim];
            float* scores = &attention_weights[(pos * num_heads + h) * seq_len];
            
            // Compute attention scores
            for (int i = 0; i < seq_len; i++) {
                float* key = &keys[i * embedding_dim + h * head_dim];
                float dot = 0.0f;
                for (uint32_t d = 0; d < head_dim; d++) {
                    dot += query[d] * key[d];
                }
                scores[i] = dot * scale;
            }
            
            // Apply softmax
            float max_score = scores[0];
            for (int i = 1; i < seq_len; i++) {
                if (scores[i] > max_score) max_score = scores[i];
            }
            
            float sum = 0.0f;
            for (int i = 0; i < seq_len; i++) {
                scores[i] = prime_exp(scores[i] - max_score);
                sum += scores[i];
            }
            
            if (sum > 1e-8f) {
                for (int i = 0; i < seq_len; i++) {
                    scores[i] /= sum;
                }
            }
        }
    }
    
    // Backward pass through attention for each position and head
    for (int pos = 0; pos < seq_len; pos++) {
        for (uint32_t h = 0; h < num_heads; h++) {
            float* query = &queries[pos * embedding_dim + h * head_dim];
            float* head_keys = &keys[h * head_dim];
            float* head_values = &values[h * head_dim];
            float* attn_weights = &attention_weights[(pos * num_heads + h) * seq_len];
            float* grad_out = &grad_output[pos * embedding_dim + h * head_dim];
            float* grad_q = &grad_queries[pos * embedding_dim + h * head_dim];
            float* grad_k = &grad_keys[h * head_dim];
            float* grad_v = &grad_values[h * head_dim];
            
            scaled_dot_product_attention_backward(
                query, head_keys, head_values, attn_weights,
                grad_out, grad_q, grad_k, grad_v,
                head_dim, seq_len
            );
        }
    }
    
    // Backward through Q, K, V projections to get gradient w.r.t. input
    memset(grad_input, 0, seq_len * embedding_dim * sizeof(float));
    
    for (int pos = 0; pos < seq_len; pos++) {
        float* grad_in = &grad_input[pos * embedding_dim];
        
        // Gradient from query projection
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t i = 0; i < head_dim; i++) {
                float grad_sum = 0.0f;
                for (uint32_t d = 0; d < head_dim; d++) {
                    size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                    grad_sum += layer->query_lattice[weight_idx] * 
                               grad_queries[pos * embedding_dim + h * head_dim + d];
                }
                grad_in[h * head_dim + i] += grad_sum;
            }
        }
        
        // Gradient from key projection
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t i = 0; i < head_dim; i++) {
                float grad_sum = 0.0f;
                for (uint32_t d = 0; d < head_dim; d++) {
                    size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                    grad_sum += layer->key_lattice[weight_idx] * 
                               grad_keys[pos * embedding_dim + h * head_dim + d];
                }
                grad_in[h * head_dim + i] += grad_sum;
            }
        }
        
        // Gradient from value projection
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t i = 0; i < head_dim; i++) {
                float grad_sum = 0.0f;
                for (uint32_t d = 0; d < head_dim; d++) {
                    size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                    grad_sum += layer->value_lattice[weight_idx] * 
                               grad_values[pos * embedding_dim + h * head_dim + d];
                }
                grad_in[h * head_dim + i] += grad_sum;
            }
        }
    }
    
    // Accumulate gradients for weight matrices
    if (grad_query_weights || grad_key_weights || grad_value_weights) {
        for (int pos = 0; pos < seq_len; pos++) {
            float* input_vec = &input[pos * embedding_dim];
            
            // Accumulate query weight gradients: dL/dW_Q = input^T × dL/dQ
            if (grad_query_weights) {
                for (uint32_t h = 0; h < num_heads; h++) {
                    for (uint32_t d = 0; d < head_dim; d++) {
                        float* grad_q = &grad_queries[pos * embedding_dim + h * head_dim + d];
                        for (uint32_t i = 0; i < head_dim; i++) {
                            size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                            grad_query_weights[weight_idx] += input_vec[h * head_dim + i] * (*grad_q);
                        }
                    }
                }
            }
            
            // Accumulate key weight gradients: dL/dW_K = input^T × dL/dK
            if (grad_key_weights) {
                for (uint32_t h = 0; h < num_heads; h++) {
                    for (uint32_t d = 0; d < head_dim; d++) {
                        float* grad_k = &grad_keys[pos * embedding_dim + h * head_dim + d];
                        for (uint32_t i = 0; i < head_dim; i++) {
                            size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                            grad_key_weights[weight_idx] += input_vec[h * head_dim + i] * (*grad_k);
                        }
                    }
                }
            }
            
            // Accumulate value weight gradients: dL/dW_V = input^T × dL/dV
            if (grad_value_weights) {
                for (uint32_t h = 0; h < num_heads; h++) {
                    for (uint32_t d = 0; d < head_dim; d++) {
                        float* grad_v = &grad_values[pos * embedding_dim + h * head_dim + d];
                        for (uint32_t i = 0; i < head_dim; i++) {
                            size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                            grad_value_weights[weight_idx] += input_vec[h * head_dim + i] * (*grad_v);
                        }
                    }
                }
            }
        }
    }
    
    free(queries);
    free(keys);
    free(values);
    free(attention_weights);
    free(grad_queries);
    free(grad_keys);
    free(grad_values);
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
    
    // Get gradient buffers for this layer
    float* grad_gamma = training->ln_grads ? training->ln_grads[layer_idx].gamma : NULL;
    float* grad_beta = training->ln_grads ? training->ln_grads[layer_idx].beta : NULL;
    
    float* grad_query = training->attention_grads ? training->attention_grads[layer_idx].query_lattice : NULL;
    float* grad_key = training->attention_grads ? training->attention_grads[layer_idx].key_lattice : NULL;
    float* grad_value = training->attention_grads ? training->attention_grads[layer_idx].value_lattice : NULL;
    
    // Backward through layer norm (post-FFN)
    CLLMLayerNorm* ln2 = &model->layer_norms[layer_idx];
    cllm_layer_norm_backward(ln2, input, grad_output, grad_temp, grad_gamma, grad_beta);
    
    // Backward through feed-forward network
    // Note: This is simplified - full implementation needs hidden activations
    memcpy(grad_input, grad_temp, seq_len * embedding_dim * sizeof(float));
    
    // Backward through attention
    AttentionLayer* attn = &model->attention_layers[layer_idx];
    cllm_attention_backward(attn, input, grad_input, grad_temp, 
                           grad_query, grad_key, grad_value, seq_len);
    
    memcpy(grad_input, grad_temp, seq_len * embedding_dim * sizeof(float));
    
    free(grad_temp);
}

/**
 * Zero all gradient buffers
 * 
 * @param training Training state
 */
void cllm_zero_all_gradients(CLLMTraining* training) {
    if (!training) return;
    
    CLLMModel* model = training->model;
    
    // Zero embedding gradients
    if (training->gradients) {
        size_t embed_size = model->vocab_size * model->embedding_dim;
        memset(training->gradients, 0, embed_size * sizeof(float));
    }
    
    // Zero attention gradients
    if (training->attention_grads) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            AttentionLayer* layer = &model->attention_layers[i];
            size_t weight_size = layer->num_heads * layer->head_dim * layer->head_dim;
            
            if (training->attention_grads[i].query_lattice) {
                memset(training->attention_grads[i].query_lattice, 0, weight_size * sizeof(float));
            }
            if (training->attention_grads[i].key_lattice) {
                memset(training->attention_grads[i].key_lattice, 0, weight_size * sizeof(float));
            }
            if (training->attention_grads[i].value_lattice) {
                memset(training->attention_grads[i].value_lattice, 0, weight_size * sizeof(float));
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
    
    // Zero all gradients before backward pass
    cllm_zero_all_gradients(training);
    
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
        size_t total_params = model->header.total_params;
        if (embed_params <= total_params) {
            memcpy(training->gradients, grad_embeddings, embed_params * sizeof(float));
        }
        
        free(grad_embeddings);
    }
    
    free(embeddings_out);
    free(grad_embeddings_out);
}
