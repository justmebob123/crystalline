#include "cllm_inference.h"
#include "cllm.h"
#include "prime_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/prime_float_math.h"

// Constants
#define MAX_SEQUENCE_LENGTH 512
#define TEMPERATURE_MIN 0.1f
#define TEMPERATURE_MAX 2.0f

// Initialize inference context
CLLMInference* cllm_inference_init(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "Error: Cannot initialize inference with NULL model\n");
        return NULL;
    }
    
    CLLMInference* inference = (CLLMInference*)calloc(1, sizeof(CLLMInference));
    if (!inference) {
        fprintf(stderr, "Error: Failed to allocate inference context\n");
        return NULL;
    }
    
    inference->model = model;
    inference->temperature = 1.0f;
    inference->top_p = 0.9f;
    inference->top_k = 50;
    inference->max_tokens = 50;
    
    // Allocate working memory
    uint32_t embed_dim = model->embeddings.embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    inference->hidden_states = (float*)calloc(embed_dim, sizeof(float));
    inference->logits = (float*)calloc(vocab_size, sizeof(float));
    
    if (!inference->hidden_states || !inference->logits) {
        fprintf(stderr, "Error: Failed to allocate inference buffers\n");
        cllm_inference_cleanup(inference);
        return NULL;
    }
    
    printf("Inference context initialized successfully\n");
    return inference;
}

// Cleanup inference context
void cllm_inference_cleanup(CLLMInference* inference) {
    if (!inference) return;
    
    if (inference->hidden_states) free(inference->hidden_states);
    if (inference->logits) free(inference->logits);
    
    free(inference);
}

// Get embedding for a token
void cllm_get_embedding(CLLMInference* inference, uint32_t token_id, float* output) {
    if (!inference || !output) return;
    
    CLLMModel* model = inference->model;
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    if (token_id >= model->vocab_size) {
        memset(output, 0, embed_dim * sizeof(float));
        return;
    }
    
    float* embedding = &model->embeddings.embeddings[token_id * embed_dim];
    
    // Lazy initialization: compute embedding on first access
    if (prime_isnanf(embedding[0])) {
        extern void cllm_compute_embedding_lazy(CLLMModel* model, uint32_t token_id);
        cllm_compute_embedding_lazy(model, token_id);
    }
    
    memcpy(output, embedding, embed_dim * sizeof(float));
}

// Tokenize text - FIXED VERSION
int cllm_tokenize(CLLMInference* inference, const char* text, uint32_t* tokens, int max_tokens) {
    if (!inference || !text || !tokens) return 0;
    
    // Check if tokens array exists
    if (!inference->model->tokens) {
        fprintf(stderr, "Warning: model->tokens is NULL - using character-based fallback tokenization\n");
        // Fallback: create simple character-based tokens
        int len = strlen(text);
        int count = len < max_tokens ? len : max_tokens;
        for (int i = 0; i < count; i++) {
            tokens[i] = (uint32_t)(text[i] % inference->model->vocab_size);
        }
        return count;
    }
    
    int token_count = 0;
    char buffer[256];
    int buf_pos = 0;
    
    for (int i = 0; text[i] && token_count < max_tokens; i++) {
        if (text[i] == ' ' || text[i] == '\n' || text[i] == '\t') {
            if (buf_pos > 0) {
                buffer[buf_pos] = '\0';
                
                // Find token in vocabulary
                bool found = false;
                for (uint32_t j = 0; j < inference->model->vocab_size; j++) {
                    if (strcmp(inference->model->tokens[j].token_str, buffer) == 0) {
                        tokens[token_count++] = j;
                        found = true;
                        break;
                    }
                }
                
                // Handle unknown words with hash (like training does)
                if (!found) {
                    uint32_t hash = 0;
                    for (int k = 0; buffer[k]; k++) {
                        hash = hash * 31 + (uint32_t)buffer[k];
                    }
                    tokens[token_count++] = hash % inference->model->vocab_size;
                }
                
                buf_pos = 0;
            }
        } else {
            if (buf_pos < 255) {
                buffer[buf_pos++] = text[i];
            }
        }
    }
    
    // Handle last token
    if (buf_pos > 0 && token_count < max_tokens) {
        buffer[buf_pos] = '\0';
        bool found = false;
        for (uint32_t j = 0; j < inference->model->vocab_size; j++) {
            if (strcmp(inference->model->tokens[j].token_str, buffer) == 0) {
                tokens[token_count++] = j;
                found = true;
                break;
            }
        }
        
        // Handle unknown words with hash
        if (!found) {
            uint32_t hash = 0;
            for (int k = 0; buffer[k]; k++) {
                hash = hash * 31 + (uint32_t)buffer[k];
            }
            tokens[token_count++] = hash % inference->model->vocab_size;
        }
    }
    
    return token_count;
}

// Detokenize tokens to text - FIXED VERSION
void cllm_detokenize(CLLMInference* inference, uint32_t* tokens, int num_tokens, char* output, int max_length) {
    if (!inference || !tokens || !output) return;
    
    // Check if tokens array exists
    if (!inference->model->tokens) {
        fprintf(stderr, "Warning: model->tokens is NULL - using character-based fallback detokenization\n");
        // Fallback: convert token IDs to characters
        int pos = 0;
        for (int i = 0; i < num_tokens && pos < max_length - 1; i++) {
            output[pos++] = (char)(tokens[i] % 128); // ASCII range
        }
        output[pos] = '\0';
        return;
    }
    
    int pos = 0;
    for (int i = 0; i < num_tokens && pos < max_length - 1; i++) {
        if (tokens[i] < inference->model->vocab_size) {
            const char* token_str = inference->model->tokens[tokens[i]].token_str;
            
            // Skip special tokens (PAD, UNK, BOS, EOS, etc.)
            if (token_str[0] == '<' && token_str[strlen(token_str)-1] == '>') {
                continue;  // Skip special tokens like <PAD>, <UNK>, etc.
            }
            
            int len = strlen(token_str);
            
            if (pos + len < max_length - 1) {
                strcpy(&output[pos], token_str);
                pos += len;
                
                // Add space between tokens
                if (i < num_tokens - 1 && pos < max_length - 1) {
                    output[pos++] = ' ';
                }
            }
        }
    }
    
    output[pos] = '\0';
}

// Apply positional encoding
void cllm_apply_positional_encoding(CLLMInference* inference, float* hidden_states, int position) {
    if (!inference || !hidden_states) return;
    
    CLLMModel* model = inference->model;
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    if (position >= (int)model->pos_encoding.max_length) {
        position = model->pos_encoding.max_length - 1;
    }
    
    // Add positional encoding if available
    if (model->pos_encoding.spiral_positions) {
        float* pos_enc = &model->pos_encoding.spiral_positions[position * embed_dim];
        for (uint32_t i = 0; i < embed_dim; i++) {
            hidden_states[i] += pos_enc[i];
        }
    }
}

// Layer normalization (old version for compatibility)
void cllm_layer_norm_old(float* x, CLLMLayerNorm* ln, uint32_t dim) {
    if (!x || !ln) return;
    
    // Compute mean
    float mean = 0.0f;
    for (uint32_t i = 0; i < dim; i++) {
        mean += x[i];
    }
    mean /= dim;
    
    // Compute variance
    float var = 0.0f;
    for (uint32_t i = 0; i < dim; i++) {
        float diff = x[i] - mean;
        var += diff * diff;
    }
    var /= dim;
    
    // Normalize
    float std = prime_sqrtf(var + ln->epsilon);
    for (uint32_t i = 0; i < dim; i++) {
        x[i] = (x[i] - mean) / std;
        
        // Apply learned parameters if available
        if (ln->gamma && ln->beta) {
            x[i] = x[i] * ln->gamma[i] + ln->beta[i];
        }
    }
}

// Feed-forward network
void cllm_feed_forward(float* x, FeedForwardLayer* ff) {
    if (!x || !ff) return;
    
    uint32_t input_dim = ff->input_dim;
    uint32_t hidden_dim = ff->hidden_dim;
    
    // Allocate temporary buffer
    float* hidden = (float*)calloc(hidden_dim, sizeof(float));
    if (!hidden) return;
    
    // First layer: input -> hidden
    if (ff->w1_lattice && ff->bias1) {
        for (uint32_t i = 0; i < hidden_dim; i++) {
            hidden[i] = ff->bias1[i];
            for (uint32_t j = 0; j < input_dim; j++) {
                hidden[i] += x[j] * ff->w1_lattice[j * hidden_dim + i];
            }
            // ReLU activation
            if (hidden[i] < 0) hidden[i] = 0;
        }
    }
    
    // Second layer: hidden -> output
    if (ff->w2_lattice && ff->bias2) {
        for (uint32_t i = 0; i < input_dim; i++) {
            x[i] = ff->bias2[i];
            for (uint32_t j = 0; j < hidden_dim; j++) {
                x[i] += hidden[j] * ff->w2_lattice[j * input_dim + i];
            }
        }
    }
    
    free(hidden);
}

// Forward pass
void cllm_forward(CLLMInference* inference, uint32_t* tokens, int num_tokens) {
    if (!inference || !tokens || num_tokens <= 0) return;
    
    CLLMModel* model = inference->model;
    if (!model) {
        fprintf(stderr, "Error: Model is NULL in cllm_forward\n");
        return;
    }
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Check critical pointers
    if (!inference->hidden_states) {
        fprintf(stderr, "Error: hidden_states is NULL\n");
        return;
    }
    if (!inference->logits) {
        fprintf(stderr, "Error: logits is NULL\n");
        return;
    }
    if (!model->embeddings.embeddings) {
        fprintf(stderr, "Error: embeddings is NULL\n");
        return;
    }
    
    // Get embedding for last token
    uint32_t last_token = tokens[num_tokens - 1];
    if (last_token >= model->vocab_size) {
        fprintf(stderr, "Error: token %u out of range (vocab_size=%lu)\n", last_token, (unsigned long)model->vocab_size);
        return;
    }
    
    cllm_get_embedding(inference, last_token, inference->hidden_states);
    
    // Apply positional encoding
    cllm_apply_positional_encoding(inference, inference->hidden_states, num_tokens - 1);
    
    // Pass through transformer layers
    if (model->attention_layers && model->ff_layers && model->layer_norms) {
        float* attn_output = (float*)malloc(embed_dim * sizeof(float));
        if (!attn_output) {
            fprintf(stderr, "Error: Failed to allocate attention output buffer\n");
            return;
        }
        
        for (uint32_t layer = 0; layer < model->num_layers; layer++) {
            // Layer norm
            cllm_layer_norm_old(inference->hidden_states, &model->layer_norms[layer], embed_dim);
            
            // Attention - use proper multi-head attention
            AttentionLayer* attn_layer = &model->attention_layers[layer];
            cllm_attention_forward(attn_layer, inference->hidden_states, attn_output, NULL, NULL, 1);
            
            // Copy attention output back to hidden states
            memcpy(inference->hidden_states, attn_output, embed_dim * sizeof(float));
            
            // Feed-forward
            cllm_feed_forward(inference->hidden_states, &model->ff_layers[layer]);
        }
        
        free(attn_output);
        
        // Final layer norm
        cllm_layer_norm_old(inference->hidden_states, &model->layer_norms[model->num_layers - 1], embed_dim);
    }

    
    // Project to vocabulary
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        inference->logits[i] = 0.0f;
        float* token_embed = &model->embeddings.embeddings[i * embed_dim];
        for (uint32_t j = 0; j < embed_dim; j++) {
            inference->logits[i] += inference->hidden_states[j] * token_embed[j];
        }
    }
}

// Apply temperature scaling
void cllm_apply_temperature(float* logits, int vocab_size, float temperature) {
    if (temperature < TEMPERATURE_MIN) temperature = TEMPERATURE_MIN;
    if (temperature > TEMPERATURE_MAX) temperature = TEMPERATURE_MAX;
    for (int i = 0; i < vocab_size; i++) {
        logits[i] /= temperature;
    }
}

// Softmax
void cllm_softmax(float* logits, int vocab_size) {
    // Find max for numerical stability
    float max_logit = logits[0];
    for (int i = 1; i < vocab_size; i++) {
        if (logits[i] > max_logit) max_logit = logits[i];
    }
    
    // Compute exp and sum
    float sum = 0.0f;
    for (int i = 0; i < vocab_size; i++) {
        logits[i] = prime_expf(logits[i] - max_logit);
        sum += logits[i];
    }
    
    // Normalize
    for (int i = 0; i < vocab_size; i++) {
        logits[i] /= sum;
    }
}

// Sample top-k
uint32_t cllm_sample_top_k(float* probs, int vocab_size, int k) {
    if (k <= 0 || k > vocab_size) k = vocab_size;
    
    // Simple sampling from top-k
    float r = (float)rand() / RAND_MAX;
    float cumsum = 0.0f;
    
    for (int i = 0; i < k && i < vocab_size; i++) {
        cumsum += probs[i];
        if (r < cumsum) return i;
    }
    
    return 0;
}

// Sample top-p (nucleus sampling)
uint32_t cllm_sample_top_p(float* probs, int vocab_size, float p) {
    float r = (float)rand() / RAND_MAX;
    float cumsum = 0.0f;
    
    for (int i = 0; i < vocab_size; i++) {
        cumsum += probs[i];
        if (cumsum >= p || r < cumsum) return i;
    }
    
    return 0;
}

// Generate text - MAIN FUNCTION
int cllm_generate(CLLMInference* inference, const char* prompt, char* output, int max_output_length) {
    if (!inference || !prompt || !output) return -1;
    
    // Silent generation - no terminal spam
    
    // Tokenize prompt
    uint32_t tokens[MAX_SEQUENCE_LENGTH];
    int num_tokens = cllm_tokenize(inference, prompt, tokens, MAX_SEQUENCE_LENGTH);
    
    if (num_tokens <= 0) {
        strcpy(output, "Error: Could not tokenize prompt");
        return -1;
    }
    
    // Generate tokens
    int tokens_generated = 0;
    while (tokens_generated < inference->max_tokens && num_tokens < MAX_SEQUENCE_LENGTH) {
        // Forward pass
        cllm_forward(inference, tokens, num_tokens);
        
        // Apply temperature
        cllm_apply_temperature(inference->logits, inference->model->vocab_size, inference->temperature);
        
        // Softmax
        cllm_softmax(inference->logits, inference->model->vocab_size);
        
        // Sample next token
        uint32_t next_token;
        if (inference->top_k > 0) {
            next_token = cllm_sample_top_k(inference->logits, inference->model->vocab_size, inference->top_k);
        } else {
            next_token = cllm_sample_top_p(inference->logits, inference->model->vocab_size, inference->top_p);
        }
        
        // Add to sequence
        tokens[num_tokens++] = next_token;
        tokens_generated++;
        
        // Silent generation - no terminal spam
    }
    
    // Detokenize
    cllm_detokenize(inference, tokens, num_tokens, output, max_output_length);
    
    // Silent generation - no terminal spam
    return tokens_generated;
}

// Set generation parameters
void cllm_set_temperature(CLLMInference* inference, float temperature) {
    if (inference) {
        if (temperature < TEMPERATURE_MIN) temperature = TEMPERATURE_MIN;
        if (temperature > TEMPERATURE_MAX) temperature = TEMPERATURE_MAX;
        inference->temperature = temperature;
    }
}

void cllm_set_top_p(CLLMInference* inference, float top_p) {
    if (inference) {
        if (top_p < 0.0f) top_p = 0.0f;
        if (top_p > 1.0f) top_p = 1.0f;
        inference->top_p = top_p;
    }
}

void cllm_set_top_k(CLLMInference* inference, int top_k) {
    if (inference) {
        inference->top_k = top_k > 0 ? top_k : 0;
    }
}

void cllm_set_max_tokens(CLLMInference* inference, int max_tokens) {
    if (inference) {
        inference->max_tokens = max_tokens > 0 ? max_tokens : 512;
    }
}

// Sample token from logits distribution
int cllm_sample_token(CLLMInference* inf, float* logits) {
    if (!inf || !logits) return 0;
    
    uint32_t vocab_size = inf->model->vocab_size;
    
    // Apply temperature
    if (inf->temperature != 1.0f) {
        for (uint32_t i = 0; i < vocab_size; i++) {
            logits[i] /= inf->temperature;
        }
    }
    
    // Convert logits to probabilities using softmax
    float max_logit = logits[0];
    for (uint32_t i = 1; i < vocab_size; i++) {
        if (logits[i] > max_logit) max_logit = logits[i];
    }
    
    float sum = 0.0f;
    for (uint32_t i = 0; i < vocab_size; i++) {
        logits[i] = prime_expf(logits[i] - max_logit);
        sum += logits[i];
    }
    
    for (uint32_t i = 0; i < vocab_size; i++) {
        logits[i] /= sum;
    }
    
    // Sample from distribution
    float r = (float)rand() / RAND_MAX;
    float cumsum = 0.0f;
    
    for (uint32_t i = 0; i < vocab_size; i++) {
        cumsum += logits[i];
        if (r < cumsum) {
            return i;
        }
    }
    
    return 0;
}