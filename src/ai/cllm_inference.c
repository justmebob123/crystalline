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
    
    // Allocate working memory using double*
    uint32_t embed_dim = model->embeddings.embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Use standard malloc for double* arrays
    inference->hidden_states = (double*)calloc(embed_dim, sizeof(double));
    inference->logits = (double*)calloc(vocab_size, sizeof(double));
    
    if (!inference->hidden_states || !inference->logits) {
        fprintf(stderr, "Error: Failed to allocate inference buffers\n");
        cllm_inference_cleanup(inference);
        return NULL;
    }
    
    printf("Inference context initialized successfully (double precision)\n");
    return inference;
}

// Cleanup inference context
void cllm_inference_cleanup(CLLMInference* inference) {
    if (!inference) return;
    
    // Use standard free for double* arrays
    if (inference->hidden_states) {
        free(inference->hidden_states);
    }
    if (inference->logits) {
        free(inference->logits);
    }
    if (inference->key_cache) {
        free(inference->key_cache);
    }
    if (inference->value_cache) {
        free(inference->value_cache);
    }
    
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

// Feed-forward network

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
    
    // Copy float embedding to double hidden_states
    float* float_embedding = &model->embeddings.embeddings[last_token * embed_dim];
    for (uint32_t i = 0; i < embed_dim; i++) {
        inference->hidden_states[i] = (double)float_embedding[i];
    }
    
    // Apply positional encoding
    cllm_apply_positional_encoding(inference, (float*)inference->hidden_states, num_tokens - 1);
    
    // Pass through transformer layers using standard operations
    if (model->attention_layers && model->ff_layers && model->layer_norms) {
        // Allocate attention output buffer
        double* attn_output = (double*)calloc(embed_dim, sizeof(double));
        if (!attn_output) {
            fprintf(stderr, "Error: Failed to allocate attention output buffer\n");
            return;
        }
        
        // Convert hidden_states to float for layer operations
        float* hidden_float = (float*)malloc(embed_dim * sizeof(float));
        if (!hidden_float) {
            fprintf(stderr, "Error: Failed to allocate float buffer\n");
            free(attn_output);
            return;
        }
        
        for (uint32_t layer = 0; layer < model->num_layers; layer++) {
            // Convert double to float for operations
            for (uint32_t i = 0; i < embed_dim; i++) {
                hidden_float[i] = (float)inference->hidden_states[i];
            }
            
            // Layer norm (in-place)
            cllm_layer_norm(&model->layer_norms[layer], hidden_float, hidden_float);
            
            // Attention
            AttentionLayer* attn_layer = &model->attention_layers[layer];
            cllm_attention_forward(attn_layer, hidden_float, hidden_float, NULL, NULL, 1);
            
            // Feed-forward (in-place)
            cllm_feedforward(&model->ff_layers[layer], hidden_float, hidden_float);
            
            // Convert back to double
            for (uint32_t i = 0; i < embed_dim; i++) {
                inference->hidden_states[i] = (double)hidden_float[i];
            }
        }
        
        // Final layer norm
        for (uint32_t i = 0; i < embed_dim; i++) {
            hidden_float[i] = (float)inference->hidden_states[i];
        }
        cllm_layer_norm(&model->layer_norms[model->num_layers - 1], hidden_float, hidden_float);
        for (uint32_t i = 0; i < embed_dim; i++) {
            inference->hidden_states[i] = (double)hidden_float[i];
        }
        
        free(hidden_float);
        free(attn_output);
    }
    
    // Project to vocabulary - compute logits
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        double logit_value = 0.0;
        float* token_embed = &model->embeddings.embeddings[i * embed_dim];
        for (uint32_t j = 0; j < embed_dim; j++) {
            logit_value += inference->hidden_states[j] * (double)token_embed[j];
        }
        inference->logits[i] = logit_value;
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
        cllm_apply_temperature((float*)inference->logits, inference->model->vocab_size, inference->temperature);
        
        // Softmax
        cllm_softmax((float*)inference->logits, inference->model->vocab_size);
        
        // Sample next token
        uint32_t next_token;
        if (inference->top_k > 0) {
            next_token = cllm_sample_top_k((float*)inference->logits, inference->model->vocab_size, inference->top_k);
        } else {
            next_token = cllm_sample_top_p((float*)inference->logits, inference->model->vocab_size, inference->top_p);
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