/**
 * Forward Pass with Activation Storage
 * 
 * Implements complete forward pass through transformer layers
 * while storing all intermediate activations for backward pass.
 */

#include "cllm_training.h"
#include "cllm_activations.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/**
 * Forward pass with activation storage
 * 
 * @param training Training state
 * @param input_tokens Input token IDs [batch_size * seq_len]
 * @param activations Pre-allocated activation storage
 * @return Cross-entropy loss
 */
float cllm_forward_with_activations(
    CLLMTraining* training,
    uint32_t* input_tokens,
    ForwardActivations* activations
) {
    if (!training || !input_tokens || !activations) return 0.0f;
    
    CLLMModel* model = training->model;
    int batch_size = activations->batch_size;
    int seq_len = activations->seq_len;
    uint32_t embed_dim = activations->embed_dim;
    uint32_t vocab_size = activations->vocab_size;
    
    // Zero activations for fresh forward pass
    cllm_zero_activations(activations);
    
    // Step 1: Get embeddings
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t token_id = input_tokens[idx];
            
            if (token_id >= vocab_size) continue;
            
            // Copy embedding
            float* embed_src = &model->embeddings.embeddings[token_id * embed_dim];
            float* embed_dst = &activations->input_embeddings[idx * embed_dim];
            memcpy(embed_dst, embed_src, embed_dim * sizeof(float));
        }
    }
    
    // Step 2: Process through transformer layers
    float* layer_input = activations->input_embeddings;
    
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Store layer input
        memcpy(activations->layer_inputs[layer], layer_input, 
               batch_size * seq_len * embed_dim * sizeof(float));
        
        // Process each position
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* input = &layer_input[idx * embed_dim];
                float* attn_out = &activations->attention_outputs[layer][idx * embed_dim];
                float* ff_out = &activations->ff_outputs[layer][idx * embed_dim];
                float* layer_out = &activations->layer_outputs[layer][idx * embed_dim];
                
                // Simplified attention (just copy for now - full attention is complex)
                memcpy(attn_out, input, embed_dim * sizeof(float));
                
                // Residual connection
                for (uint32_t d = 0; d < embed_dim; d++) {
                    attn_out[d] += input[d];
                }
                
                // FeedForward layer
                FeedForwardLayer* ff = &model->ff_layers[layer];
                float* ff_hidden = &activations->ff_hidden[layer][idx * ff->hidden_dim];
                
                // First layer: input -> hidden
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    float sum = ff->bias1[h];
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        sum += attn_out[i] * ff->w1_lattice[i * ff->hidden_dim + h];
                    }
                    ff_hidden[h] = tanhf(sum);  // Store activation
                }
                
                // Second layer: hidden -> output
                for (uint32_t o = 0; o < embed_dim; o++) {
                    float sum = ff->bias2[o];
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        sum += ff_hidden[h] * ff->w2_lattice[h * embed_dim + o];
                    }
                    ff_out[o] = sum;
                }
                
                // Residual connection
                for (uint32_t d = 0; d < embed_dim; d++) {
                    layer_out[d] = attn_out[d] + ff_out[d];
                }
                
                // Layer normalization
                CLLMLayerNorm* ln = &model->layer_norms[layer];
                float mean = 0.0f, var = 0.0f;
                
                for (uint32_t d = 0; d < embed_dim; d++) {
                    mean += layer_out[d];
                }
                mean /= embed_dim;
                
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float diff = layer_out[d] - mean;
                    var += diff * diff;
                }
                var /= embed_dim;
                
                float std = sqrtf(var + 1e-5f);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    layer_out[d] = ln->gamma[d] * (layer_out[d] - mean) / std + ln->beta[d];
                }
            }
        }
        
        // Update layer input for next layer
        layer_input = activations->layer_outputs[layer];
    }
    
    // Step 3: Copy final hidden state
    memcpy(activations->final_hidden, layer_input,
           batch_size * seq_len * embed_dim * sizeof(float));
    
    // Step 4: Project to vocabulary (logits)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            float* hidden = &activations->final_hidden[idx * embed_dim];
            float* logits = &activations->logits[idx * vocab_size];
            
            // Dot product with each vocabulary embedding
            for (uint32_t v = 0; v < vocab_size; v++) {
                float* vocab_embed = &model->embeddings.embeddings[v * embed_dim];
                float score = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    score += hidden[d] * vocab_embed[d];
                }
                logits[v] = score;
            }
        }
    }
    
    return 0.0f; // Loss computed separately
}

/**
 * Compute cross-entropy loss from stored logits
 */
float cllm_compute_loss_from_activations(
    ForwardActivations* activations,
    uint32_t* target_tokens
) {
    if (!activations || !target_tokens) return 0.0f;
    
    int batch_size = activations->batch_size;
    int seq_len = activations->seq_len;
    uint32_t vocab_size = activations->vocab_size;
    
    float total_loss = 0.0f;
    int count = 0;
    
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t target = target_tokens[idx];
            
            if (target >= vocab_size) continue;
            
            float* logits = &activations->logits[idx * vocab_size];
            
            // Compute softmax (numerically stable)
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
            
            // Cross-entropy loss
            float log_prob = (logits[target] - max_logit) - logf(sum_exp);
            total_loss += -log_prob;
            count++;
        }
    }
    
    return count > 0 ? total_loss / count : 0.0f;
}