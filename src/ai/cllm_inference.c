/*
 * CLLM Inference Engine
 * Performs inference using the Crystalline Lattice Language Model
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/cllm_format.h"
#include "../include/cllm_inference.h"
#include "../include/prime_float_math.h"

// Standard math functions for min/max
static inline float prime_prime_fmaxf(float a, float b) { return (a > b) ? a : b; }
static inline float prime_prime_fminf(float a, float b) { return (a < b) ? a : b; }
static inline float prime_expf_custom(float x) { return (float)prime_exp((double)x); }

#define MAX_SEQUENCE_LENGTH 2048
#define TEMPERATURE_MIN 0.1f
#define TEMPERATURE_MAX 2.0f

// Initialize inference engine
CLLMInference* cllm_inference_init(CLLMModel* model) {
    if (!model) return NULL;
    
    CLLMInference* inference = (CLLMInference*)calloc(1, sizeof(CLLMInference));
    if (!inference) return NULL;
    
    inference->model = model;
    inference->temperature = 0.7f;
    inference->top_p = 0.9f;
    inference->top_k = 50;
    inference->max_tokens = 512;
    inference->repetition_penalty = 1.1f;
    
    // Allocate KV cache
    inference->kv_cache_size = model->header.context_length;
    inference->kv_cache_used = 0;
    
    size_t cache_size = model->header.num_layers * 
                       model->header.num_heads * 
                       model->header.context_length * 
                       (model->header.embedding_dim / model->header.num_heads);
    
    inference->key_cache = (float*)calloc(cache_size, sizeof(float));
    inference->value_cache = (float*)calloc(cache_size, sizeof(float));
    
    if (!inference->key_cache || !inference->value_cache) {
        cllm_inference_cleanup(inference);
        return NULL;
    }
    
    // Allocate working buffers
    inference->hidden_states = (float*)calloc(model->header.embedding_dim, sizeof(float));
    inference->logits = (float*)calloc(model->header.vocab_size, sizeof(float));
    
    // Initialize random seed
    srand(time(NULL));
    
    return inference;
}

// Tokenize input text (simple whitespace tokenization for now)
int cllm_tokenize(CLLMInference* inference, const char* text, uint32_t* tokens, int max_tokens) {
    if (!inference || !text || !tokens) return 0;
    
    int token_count = 0;
    char buffer[256];
    int buf_pos = 0;
    
    for (int i = 0; text[i] && token_count < max_tokens; i++) {
        if (text[i] == ' ' || text[i] == '\n' || text[i] == '\t') {
            if (buf_pos > 0) {
                buffer[buf_pos] = '\0';
                
                // Find token in vocabulary
                for (uint32_t j = 0; j < inference->model->vocab_size; j++) {
                    if (strcmp(inference->model->tokens[j].token_str, buffer) == 0) {
                        tokens[token_count++] = j;
                        break;
                    }
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
        for (uint32_t j = 0; j < inference->model->vocab_size; j++) {
            if (strcmp(inference->model->tokens[j].token_str, buffer) == 0) {
                tokens[token_count++] = j;
                break;
            }
        }
    }
    
    return token_count;
}

// Detokenize tokens to text
void cllm_detokenize(CLLMInference* inference, uint32_t* tokens, int num_tokens, char* output, int max_length) {
    if (!inference || !tokens || !output) return;
    
    int pos = 0;
    for (int i = 0; i < num_tokens && pos < max_length - 1; i++) {
        if (tokens[i] < inference->model->vocab_size) {
            const char* token_str = inference->model->tokens[tokens[i]].token_str;
            int len = strlen(token_str);
            
            if (pos + len + 1 < max_length) {
                strcpy(output + pos, token_str);
                pos += len;
                if (i < num_tokens - 1) {
                    output[pos++] = ' ';
                }
            }
        }
    }
    output[pos] = '\0';
}

// Get embedding for a token
void cllm_get_embedding(CLLMInference* inference, uint32_t token_id, float* embedding) {
    if (!inference || !embedding || token_id >= inference->model->vocab_size) return;
    
    uint32_t embed_dim = inference->model->embeddings.embedding_dim;
    float* token_embed = &inference->model->embeddings.embeddings[token_id * embed_dim];
    
    memcpy(embedding, token_embed, embed_dim * sizeof(float));
}

// Apply positional encoding
void cllm_apply_positional_encoding(CLLMInference* inference, float* embedding, int position) {
    if (!inference || !embedding) return;
    
    uint32_t embed_dim = inference->model->embeddings.embedding_dim;
    PositionalEncoding* pos_enc = &inference->model->pos_encoding;
    
    if (position < (int)pos_enc->max_length) {
        // Combine spiral, clock, and prime-based positional encodings
        float* spiral_pos = &pos_enc->spiral_positions[position * embed_dim];
        float* clock_pos = &pos_enc->clock_positions[position * embed_dim];
        float* prime_pos = &pos_enc->prime_positions[position * embed_dim];
        
        for (uint32_t i = 0; i < embed_dim; i++) {
            embedding[i] += 0.4f * spiral_pos[i] + 0.3f * clock_pos[i] + 0.3f * prime_pos[i];
        }
    }
}

// Layer normalization
void cllm_layer_norm_old(float* x, CLLMLayerNorm* ln, int dim) {
    if (!x || !ln) return;
    
    // Calculate mean
    float mean = 0.0f;
    for (int i = 0; i < dim; i++) {
        mean += x[i];
    }
    mean /= dim;
    
    // Calculate variance
    float variance = 0.0f;
    for (int i = 0; i < dim; i++) {
        float diff = x[i] - mean;
        variance += diff * diff;
    }
    variance /= dim;
    
    // Normalize
    float std = prime_sqrtf(variance + ln->epsilon);
    for (int i = 0; i < dim; i++) {
        x[i] = (x[i] - mean) / std;
        x[i] = x[i] * ln->gamma[i] + ln->beta[i];
    }
}

// Crystalline attention (simplified)
void cllm_crystalline_attention(CLLMInference* inference, float* hidden, AttentionLayer* attn, int layer_id __attribute__((unused))) {
    if (!inference || !hidden || !attn) return;
    
    // This is a simplified version - full implementation would use lattice geometry
    uint32_t embed_dim = inference->model->embeddings.embedding_dim;
    uint32_t head_dim = attn->head_dim;
    (void)attn; // num_heads used implicitly
    
    // For now, just apply a simple transformation
    // TODO: Implement full crystalline attention with geometric properties
    
    // Apply query transformation
    float query[head_dim];
    for (uint32_t i = 0; i < head_dim; i++) {
        query[i] = 0.0f;
        for (uint32_t j = 0; j < embed_dim; j++) {
            query[i] += hidden[j] * attn->query_lattice[i * embed_dim + j];
        }
    }
    
    // Simplified attention output (identity for now)
    // Full implementation would compute attention over lattice structure
}

// Feed-forward network
void cllm_feed_forward(float* hidden, FeedForwardLayer* ff) {
    if (!hidden || !ff) return;
    
    // First layer
    float* intermediate = (float*)malloc(ff->hidden_dim * sizeof(float));
    for (uint32_t i = 0; i < ff->hidden_dim; i++) {
        intermediate[i] = ff->bias1[i];
        for (uint32_t j = 0; j < ff->input_dim; j++) {
            intermediate[i] += hidden[j] * ff->w1_lattice[i * ff->input_dim + j];
        }
        // ReLU activation
        if (intermediate[i] < 0) intermediate[i] = 0;
    }
    
    // Second layer
    for (uint32_t i = 0; i < ff->output_dim; i++) {
        hidden[i] = ff->bias2[i];
        for (uint32_t j = 0; j < ff->hidden_dim; j++) {
            hidden[i] += intermediate[j] * ff->w2_lattice[i * ff->hidden_dim + j];
        }
    }
    
    free(intermediate);
}

// Forward pass through the model
void cllm_forward(CLLMInference* inference, uint32_t* tokens, int num_tokens) {
    if (!inference || !tokens || num_tokens <= 0) return;
    
    CLLMModel* model = inference->model;
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Get embedding for last token
    uint32_t last_token = tokens[num_tokens - 1];
    cllm_get_embedding(inference, last_token, inference->hidden_states);
    
    // Apply positional encoding
    cllm_apply_positional_encoding(inference, inference->hidden_states, num_tokens - 1);
    
    // Pass through transformer layers
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Layer norm
        cllm_layer_norm_old(inference->hidden_states, &model->layer_norms[layer], embed_dim);
        
        // Attention
        cllm_crystalline_attention(inference, inference->hidden_states, 
                                   &model->attention_layers[layer], layer);
        
        // Feed-forward
        cllm_feed_forward(inference->hidden_states, &model->ff_layers[layer]);
    }
    
    // Final layer norm
    cllm_layer_norm_old(inference->hidden_states, &model->layer_norms[model->num_layers - 1], embed_dim);
    
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
    temperature = prime_fmaxf(TEMPERATURE_MIN, prime_fminf(TEMPERATURE_MAX, temperature));
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
        logits[i] = prime_expf_custom(logits[i] - max_logit);
        sum += logits[i];
    }
    
    // Normalize
    for (int i = 0; i < vocab_size; i++) {
        logits[i] /= sum;
    }
}

// Top-k sampling
uint32_t cllm_sample_top_k(float* probs, int vocab_size, int k) {
    // Create array of (index, prob) pairs
    typedef struct { int idx; float prob; } IndexProb;
    IndexProb* pairs = (IndexProb*)malloc(vocab_size * sizeof(IndexProb));
    
    for (int i = 0; i < vocab_size; i++) {
        pairs[i].idx = i;
        pairs[i].prob = probs[i];
    }
    
    // Partial sort to get top-k
    for (int i = 0; i < k && i < vocab_size; i++) {
        for (int j = i + 1; j < vocab_size; j++) {
            if (pairs[j].prob > pairs[i].prob) {
                IndexProb temp = pairs[i];
                pairs[i] = pairs[j];
                pairs[j] = temp;
            }
        }
    }
    
    // Renormalize top-k
    float sum = 0.0f;
    for (int i = 0; i < k && i < vocab_size; i++) {
        sum += pairs[i].prob;
    }
    
    // Sample from top-k
    float r = ((float)rand() / RAND_MAX) * sum;
    float cumsum = 0.0f;
    int selected = 0;
    
    for (int i = 0; i < k && i < vocab_size; i++) {
        cumsum += pairs[i].prob;
        if (r <= cumsum) {
            selected = pairs[i].idx;
            break;
        }
    }
    
    free(pairs);
    return selected;
}

// Top-p (nucleus) sampling
uint32_t cllm_sample_top_p(float* probs, int vocab_size, float p) {
    // Create sorted array
    typedef struct { int idx; float prob; } IndexProb;
    IndexProb* pairs = (IndexProb*)malloc(vocab_size * sizeof(IndexProb));
    
    for (int i = 0; i < vocab_size; i++) {
        pairs[i].idx = i;
        pairs[i].prob = probs[i];
    }
    
    // Sort by probability (descending)
    for (int i = 0; i < vocab_size - 1; i++) {
        for (int j = i + 1; j < vocab_size; j++) {
            if (pairs[j].prob > pairs[i].prob) {
                IndexProb temp = pairs[i];
                pairs[i] = pairs[j];
                pairs[j] = temp;
            }
        }
    }
    
    // Find nucleus
    float cumsum = 0.0f;
    int nucleus_size = 0;
    for (int i = 0; i < vocab_size; i++) {
        cumsum += pairs[i].prob;
        nucleus_size++;
        if (cumsum >= p) break;
    }
    
    // Renormalize nucleus
    float sum = 0.0f;
    for (int i = 0; i < nucleus_size; i++) {
        sum += pairs[i].prob;
    }
    
    // Sample from nucleus
    float r = ((float)rand() / RAND_MAX) * sum;
    cumsum = 0.0f;
    int selected = 0;
    
    for (int i = 0; i < nucleus_size; i++) {
        cumsum += pairs[i].prob;
        if (r <= cumsum) {
            selected = pairs[i].idx;
            break;
        }
    }
    
    free(pairs);
    return selected;
}

// Generate tokens
int cllm_generate(CLLMInference* inference, const char* prompt, char* output, int max_output_length) {
    if (!inference || !prompt || !output) return -1;
    
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
        
        // Check for end of sequence token (if defined)
        // TODO: Add EOS token handling
    }
    
    // Detokenize
    cllm_detokenize(inference, tokens, num_tokens, output, max_output_length);
    
    return tokens_generated;
}

// Set generation parameters
void cllm_set_temperature(CLLMInference* inference, float temperature) {
    if (inference) {
        inference->temperature = prime_fmaxf(TEMPERATURE_MIN, prime_fminf(TEMPERATURE_MAX, temperature));
    }
}

void cllm_set_top_p(CLLMInference* inference, float top_p) {
    if (inference) {
        inference->top_p = prime_fmaxf(0.0f, prime_fminf(1.0f, top_p));
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
        logits[i] = prime_expf_custom(logits[i] - max_logit);
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
            return (int)i;
        }
    }
    
    return vocab_size - 1;
}

// Cleanup
void cllm_inference_cleanup(CLLMInference* inference) {
    if (!inference) return;
    
    free(inference->key_cache);
    free(inference->value_cache);
    free(inference->hidden_states);
    free(inference->logits);
    free(inference);
}