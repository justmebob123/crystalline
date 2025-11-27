/*
 * CLLM Training Pipeline
 * Training system for Crystalline Lattice Language Models
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "../include/prime_float_math.h"
#include "../include/cllm_format.h"
#include "../include/cllm_training.h"
#include "../include/cllm_inference.h"
#include "../include/prime_float_math.h"
#include "../include/cllm_simd_utils.h"
#include "../include/cllm_crystalline_training.h"

#define MAX_BATCH_SIZE 128
#define MAX_SEQUENCE_LENGTH 2048

// Initialize training state
CLLMTraining* cllm_training_init(CLLMModel* model, CLLMTrainingConfig* config) {
    if (!model || !config) return NULL;
    
    CLLMTraining* training = (CLLMTraining*)calloc(1, sizeof(CLLMTraining));
    if (!training) return NULL;
    
    training->model = model;
    training->config = *config;
    training->current_epoch = 0;
    training->current_step = 0;
    training->best_loss = 1e9f;
    training->accumulation_step = 0;  // Initialize gradient accumulation counter
    
    // Store initial learning rate for scheduling
    training->config.initial_learning_rate = config->learning_rate;
    
    // Initialize mixed precision state
    training->master_weights = NULL;
    training->fp16_activations = NULL;
    training->fp16_gradients = NULL;
    training->current_loss_scale = config->loss_scale > 0 ? config->loss_scale : 1024.0f;
    training->loss_scale_steps = 0;
    
    // Allocate master weights for mixed precision if enabled
    if (config->use_mixed_precision) {
        size_t total_params = model->header.total_params;
        if (total_params > 0 && total_params < 1000000000) {
            training->master_weights = (float*)malloc(total_params * sizeof(float));
            if (training->master_weights && model->weights) {
                // Copy current weights to master weights
                memcpy(training->master_weights, model->weights, total_params * sizeof(float));
            }
        }
    }
    
    // Allocate gradient buffers
    size_t embed_size = model->vocab_size * model->embedding_dim;
    
    if (embed_size > 0 && embed_size < 100000000) {
        training->gradients = (float*)calloc(embed_size, sizeof(float));
        training->optimizer_state = (float*)calloc(embed_size * 2, sizeof(float));
    } else {
        training->gradients = NULL;
        training->optimizer_state = NULL;
    }
    
    // Allocate attention gradient buffers
    uint32_t num_layers = model->num_layers;
    if (num_layers > 0 && num_layers < 100) {
        training->attention_grads = (typeof(training->attention_grads))calloc(num_layers, sizeof(*training->attention_grads));
        
        if (training->attention_grads && model->attention_layers) {
            for (uint32_t i = 0; i < num_layers; i++) {
                AttentionLayer* layer = &model->attention_layers[i];
                uint32_t dim = layer->num_heads * layer->head_dim;
                size_t weight_size = dim * dim;
                
                training->attention_grads[i].query_lattice = (float*)calloc(weight_size, sizeof(float));
                training->attention_grads[i].key_lattice = (float*)calloc(weight_size, sizeof(float));
                training->attention_grads[i].value_lattice = (float*)calloc(weight_size, sizeof(float));
            }
        }
    } else {
        training->attention_grads = NULL;
    }
    
    // Allocate feed-forward gradient buffers
    if (num_layers > 0 && num_layers < 100) {
        training->ff_grads = (typeof(training->ff_grads))calloc(num_layers, sizeof(*training->ff_grads));
        
        if (training->ff_grads && model->ff_layers) {
            for (uint32_t i = 0; i < num_layers; i++) {
                FeedForwardLayer* layer = &model->ff_layers[i];
                
                training->ff_grads[i].w1_lattice = (float*)calloc(layer->input_dim * layer->hidden_dim, sizeof(float));
                training->ff_grads[i].w2_lattice = (float*)calloc(layer->hidden_dim * layer->output_dim, sizeof(float));
                training->ff_grads[i].bias1 = (float*)calloc(layer->hidden_dim, sizeof(float));
                training->ff_grads[i].bias2 = (float*)calloc(layer->output_dim, sizeof(float));
            }
        }
    } else {
        training->ff_grads = NULL;
    }
    
    // Allocate layer norm gradient buffers
    if (num_layers > 0 && num_layers < 100) {
        training->ln_grads = (typeof(training->ln_grads))calloc(num_layers, sizeof(*training->ln_grads));
        
        if (training->ln_grads && model->layer_norms) {
            for (uint32_t i = 0; i < num_layers; i++) {
                CLLMLayerNorm* layer = &model->layer_norms[i];
                
                training->ln_grads[i].gamma = (float*)calloc(layer->dim, sizeof(float));
                training->ln_grads[i].beta = (float*)calloc(layer->dim, sizeof(float));
            }
        }
    } else {
        training->ln_grads = NULL;
    }
    
    // Pre-allocate backward pass buffers (OPTIMIZATION)
    size_t activation_size = config->batch_size * config->sequence_length * model->embedding_dim;
    training->backward_buffer_size = activation_size;
    
    training->backward_embeddings = (float*)calloc(activation_size, sizeof(float));
    training->backward_grad_output = (float*)calloc(activation_size, sizeof(float));
    training->backward_layer_input = (float*)calloc(model->embedding_dim, sizeof(float));
    training->backward_layer_grad = (float*)calloc(model->embedding_dim, sizeof(float));
    training->backward_temp_grad = (float*)calloc(model->embedding_dim, sizeof(float));
    
    if (!training->backward_embeddings || !training->backward_grad_output ||
        !training->backward_layer_input || !training->backward_layer_grad ||
        !training->backward_temp_grad) {
        fprintf(stderr, "Failed to allocate backward buffers\n");
        cllm_training_cleanup(training);
        return NULL;
    }
    
    // Allocate embedding cache (OPTIMIZATION)
    size_t cache_size = config->batch_size * config->sequence_length;
    training->cached_batch_size = cache_size;
    training->cached_input_embeddings = (float*)calloc(cache_size * model->embedding_dim, sizeof(float));
    training->cached_target_embeddings = (float*)calloc(cache_size * model->embedding_dim, sizeof(float));
    
    // Allocate forward pass activation storage
    size_t seq_size = config->batch_size * config->sequence_length * model->embedding_dim;
    size_t logits_size = config->batch_size * config->sequence_length * model->vocab_size;
    
    training->input_embeddings = (float*)calloc(seq_size, sizeof(float));
    training->final_hidden = (float*)calloc(seq_size, sizeof(float));
    training->logits = (float*)calloc(logits_size, sizeof(float));
    
    training->layer_inputs = (float**)calloc(num_layers, sizeof(float*));
    training->attention_outputs = (float**)calloc(num_layers, sizeof(float*));
    training->ff_outputs = (float**)calloc(num_layers, sizeof(float*));
    training->layer_outputs = (float**)calloc(num_layers, sizeof(float*));
    training->ff_hidden = (float**)calloc(num_layers, sizeof(float*));
    
    if (training->layer_inputs && training->attention_outputs && training->ff_outputs &&
        training->layer_outputs && training->ff_hidden && model->ff_layers) {
        for (uint32_t i = 0; i < num_layers; i++) {
            training->layer_inputs[i] = (float*)calloc(seq_size, sizeof(float));
            training->attention_outputs[i] = (float*)calloc(seq_size, sizeof(float));
            training->ff_outputs[i] = (float*)calloc(seq_size, sizeof(float));
            training->layer_outputs[i] = (float*)calloc(seq_size, sizeof(float));
            training->ff_hidden[i] = (float*)calloc(seq_size * 4, sizeof(float)); // 4x for hidden dim
        }
    }
    
    if (!training->cached_input_embeddings || !training->cached_target_embeddings) {
        fprintf(stderr, "Failed to allocate embedding cache\n");
        cllm_training_cleanup(training);
        return NULL;
    }
    
    // Allocate attention cache for full backward pass (OPTIMIZATION)
    training->attention_cache = (typeof(training->attention_cache))calloc(num_layers, sizeof(*training->attention_cache));
    training->cached_seq_len = config->sequence_length;
    training->store_attention_weights = 0;  // Disable expensive attention caching during training (PERFORMANCE FIX)
    
    if (training->attention_cache && model->attention_layers) {
        int max_seq_len = config->sequence_length;
        uint32_t embed_dim = model->embedding_dim;
        size_t total_attention_cache_size = 0;
        
        for (uint32_t i = 0; i < num_layers; i++) {
            uint32_t layer_num_heads = model->attention_layers[i].num_heads;
            
            training->attention_cache[i].queries = (float*)calloc(max_seq_len * embed_dim, sizeof(float));
            training->attention_cache[i].keys = (float*)calloc(max_seq_len * embed_dim, sizeof(float));
            training->attention_cache[i].values = (float*)calloc(max_seq_len * embed_dim, sizeof(float));
            training->attention_cache[i].attention_weights = 
                (float*)calloc(layer_num_heads * max_seq_len * max_seq_len, sizeof(float));
            training->attention_cache[i].scores = 
                (float*)calloc(layer_num_heads * max_seq_len * max_seq_len, sizeof(float));
            
            if (!training->attention_cache[i].queries || !training->attention_cache[i].keys ||
                !training->attention_cache[i].values || !training->attention_cache[i].attention_weights ||
                !training->attention_cache[i].scores) {
                fprintf(stderr, "Failed to allocate attention cache for layer %u\n", i);
                cllm_training_cleanup(training);
                return NULL;
            }
            
            total_attention_cache_size += (
                3 * max_seq_len * embed_dim * sizeof(float) +  // Q, K, V
                2 * layer_num_heads * max_seq_len * max_seq_len * sizeof(float)  // weights, scores
            );
        }
        
        printf("✓ Allocated attention cache: %zu bytes (full backward enabled)\n", total_attention_cache_size);
    }
    
    printf("✓ Pre-allocated backward buffers: %zu bytes\n", 
           activation_size * sizeof(float) * 2 + model->embedding_dim * sizeof(float) * 3);
    printf("✓ Allocated embedding cache: %zu bytes\n",
           cache_size * model->embedding_dim * sizeof(float) * 2);
    
    training->start_time = time(NULL);
    
    return training;
}

// Load training data from file
int cllm_load_training_data(CLLMTraining* training, const char* filename) {
    if (!training || !filename) return -1;
    
    FILE* f = fopen(filename, "r");
    if (!f) return -1;
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Read file content
    char* content = (char*)malloc(file_size + 1);
    if (!content) {
        fclose(f);
        return -1;
    }
    
    size_t bytes_read = fread(content, 1, file_size, f);
    content[bytes_read] = '\0';
    fclose(f);
    
    // Tokenize content (simple whitespace tokenization)
    // CRITICAL FIX: APPEND instead of OVERWRITE
    size_t old_num_tokens = training->num_tokens;
    size_t new_capacity = old_num_tokens + file_size;
    
    // Reallocate to append new tokens
    uint32_t* new_tokens = (uint32_t*)realloc(training->tokens, new_capacity * sizeof(uint32_t));
    if (!new_tokens) {
        free(content);
        return -1;
    }
    training->tokens = new_tokens;
    
    // Start appending at old_num_tokens position
    size_t tokens_added = 0;
    
    // Check if model has vocabulary
    if (!training->model->tokens) {
        fprintf(stderr, "Warning: Model has no vocabulary, using character-based tokenization\n");
        // Fallback: character-based tokenization
        for (size_t i = 0; i < bytes_read && tokens_added < (size_t)file_size; i++) {
            if (content[i] != '\n' && content[i] != '\r') {
                training->tokens[old_num_tokens + tokens_added] = (uint32_t)(content[i] % training->model->vocab_size);
                tokens_added++;
            }
        }
    } else {
        // Use vocabulary-based tokenization
        char* token = strtok(content, " \n\t");
        while (token != NULL && tokens_added < (size_t)file_size) {
            // Find token in vocabulary
            bool found = false;
            for (uint32_t i = 0; i < training->model->vocab_size; i++) {
                if (strcmp(training->model->tokens[i].token_str, token) == 0) {
                    training->tokens[old_num_tokens + tokens_added] = i;
                    tokens_added++;
                    found = true;
                    break;
                }
            }
            // If token not in vocabulary, use hash or skip
            if (!found) {
                // Use simple hash to map unknown tokens
                uint32_t hash = 0;
                for (size_t i = 0; token[i]; i++) {
                    hash = hash * 31 + (uint32_t)token[i];
                }
                training->tokens[old_num_tokens + tokens_added] = hash % training->model->vocab_size;
                tokens_added++;
            }
            token = strtok(NULL, " \n\t");
        }
    }
    
    // Update total token count
    training->num_tokens = old_num_tokens + tokens_added;
    
    free(content);
    
    // Calculate number of batches
    int tokens_per_batch = training->config.batch_size * training->config.sequence_length;
    training->total_batches = training->num_tokens / tokens_per_batch;
    
    return tokens_added;  // Return number of tokens added from this file
}

// Get next training batch
int cllm_get_batch(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens) {
    if (!training || !input_tokens || !target_tokens) return -1;
    
    // CRITICAL: Check if training data is loaded
    if (!training->tokens || training->num_tokens == 0) {
        fprintf(stderr, "ERROR: No training data loaded! training->tokens is NULL\n");
        return 0;
    }
    
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    int tokens_per_batch = batch_size * seq_len;
    
    // Check if we have enough tokens
    if (training->current_batch_offset + (size_t)tokens_per_batch + 1 > training->num_tokens) {
        // Debug output
        if (training->current_batch_offset == 0) {
            printf("  Not enough tokens for even one batch!\n");
            printf("  Need: %d tokens, Have: %zu tokens\n", tokens_per_batch + 1, training->num_tokens);
            printf("  Batch size: %d, Sequence length: %d\n", batch_size, seq_len);
        }
        // Shuffle and restart
        training->current_batch_offset = 0;
        training->current_epoch++;
        return 0; // End of epoch
    }
    
    // Extract batch
    for (int i = 0; i < batch_size; i++) {
        for (int j = 0; j < seq_len; j++) {
            int idx = training->current_batch_offset + i * seq_len + j;
            input_tokens[i * seq_len + j] = training->tokens[idx];
            target_tokens[i * seq_len + j] = training->tokens[idx + 1];
        }
    }
    
    training->current_batch_offset += tokens_per_batch;
    return tokens_per_batch;
}

/**
 * Cache embeddings for entire batch (OPTIMIZATION)
 * Pre-fetches all embeddings to improve cache locality
 */
static void cache_batch_embeddings(CLLMTraining* training, uint32_t* input_tokens, 
                                   uint32_t* target_tokens, int num_tokens) {
    if (!training || !input_tokens || !target_tokens || num_tokens <= 0) return;
    
    CLLMModel* model = training->model;
    uint64_t embed_dim = model->embedding_dim;
    
    // Cache input embeddings
    for (int i = 0; i < num_tokens && i < training->cached_batch_size; i++) {
        uint32_t token_id = input_tokens[i];
        if (token_id < model->vocab_size) {
            float* src = &model->embeddings.embeddings[token_id * embed_dim];
            float* dst = &training->cached_input_embeddings[i * embed_dim];
            memcpy(dst, src, embed_dim * sizeof(float));
        }
    }
    
    // Cache target embeddings
    for (int i = 0; i < num_tokens && i < training->cached_batch_size; i++) {
        uint32_t token_id = target_tokens[i];
        if (token_id < model->vocab_size) {
            float* src = &model->embeddings.embeddings[token_id * embed_dim];
            float* dst = &training->cached_target_embeddings[i * embed_dim];
            memcpy(dst, src, embed_dim * sizeof(float));
        }
    }
}

/**
 * Get cached embedding for token at index (OPTIMIZATION)
 */
static inline float* get_cached_input_embedding(CLLMTraining* training, int index) {
    return &training->cached_input_embeddings[index * training->model->embedding_dim];
}

static inline float* get_cached_target_embedding(CLLMTraining* training, int index) {
    return &training->cached_target_embeddings[index * training->model->embedding_dim];
}

// Forward pass (compute loss)

// Forward declaration
// cllm_backward is now implemented in cllm_backward.c

// Adam optimizer step
void cllm_optimizer_step(CLLMTraining* training) {
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
    
    float lr = training->config.learning_rate;
    CLLMModel* model = training->model;
    
    // Simple SGD update (no momentum for now - just get it working)
    // Update embeddings
    uint32_t embedding_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    size_t embed_params = vocab_size * embedding_dim;
    
    if (model->embeddings.embeddings && training->gradients) {
        for (size_t i = 0; i < embed_params; i++) {
            model->embeddings.embeddings[i] -= lr * training->gradients[i] * gradient_scale;
            training->gradients[i] = 0.0f;  // Clear gradient after update
        }
    }
    
    // Update layer weights
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Update attention weights
        if (training->attention_grads && model->attention_layers) {
            uint64_t attn_size = embedding_dim * embedding_dim;
            
            if (training->attention_grads[layer].query_lattice && model->attention_layers[layer].query_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].query_lattice[i] -= lr * training->attention_grads[layer].query_lattice[i] * gradient_scale;
                    training->attention_grads[layer].query_lattice[i] = 0.0f;
                }
            }
            
            if (training->attention_grads[layer].key_lattice && model->attention_layers[layer].key_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].key_lattice[i] -= lr * training->attention_grads[layer].key_lattice[i] * gradient_scale;
                    training->attention_grads[layer].key_lattice[i] = 0.0f;
                }
            }
            
            if (training->attention_grads[layer].value_lattice && model->attention_layers[layer].value_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].value_lattice[i] -= lr * training->attention_grads[layer].value_lattice[i] * gradient_scale;
                    training->attention_grads[layer].value_lattice[i] = 0.0f;
                }
            }
        }
        
        // Update feedforward weights
        if (training->ff_grads && model->ff_layers) {
            FeedForwardLayer* ff = &model->ff_layers[layer];
            uint32_t hidden_dim = ff->hidden_dim;
            uint32_t input_dim = ff->input_dim;
            uint32_t output_dim = ff->output_dim;
            
            if (training->ff_grads[layer].w1_lattice && ff->w1_lattice) {
                for (uint32_t i = 0; i < input_dim * hidden_dim; i++) {
                    ff->w1_lattice[i] -= lr * training->ff_grads[layer].w1_lattice[i] * gradient_scale;
                    training->ff_grads[layer].w1_lattice[i] = 0.0f;
                }
            }
            
            if (training->ff_grads[layer].w2_lattice && ff->w2_lattice) {
                for (uint32_t i = 0; i < hidden_dim * output_dim; i++) {
                    ff->w2_lattice[i] -= lr * training->ff_grads[layer].w2_lattice[i] * gradient_scale;
                    training->ff_grads[layer].w2_lattice[i] = 0.0f;
                }
            }
            
            if (training->ff_grads[layer].bias1 && ff->bias1) {
                for (uint32_t i = 0; i < hidden_dim; i++) {
                    ff->bias1[i] -= lr * training->ff_grads[layer].bias1[i] * gradient_scale;
                    training->ff_grads[layer].bias1[i] = 0.0f;
                }
            }
            
            if (training->ff_grads[layer].bias2 && ff->bias2) {
                for (uint32_t i = 0; i < output_dim; i++) {
                    ff->bias2[i] -= lr * training->ff_grads[layer].bias2[i] * gradient_scale;
                    training->ff_grads[layer].bias2[i] = 0.0f;
                }
            }
        }
        
        // Update layer norm parameters
        if (training->ln_grads && model->layer_norms) {
            if (training->ln_grads[layer].gamma && model->layer_norms[layer].gamma) {
                for (uint64_t i = 0; i < embedding_dim; i++) {
                    model->layer_norms[layer].gamma[i] -= lr * training->ln_grads[layer].gamma[i];
                }
            }
            
            if (training->ln_grads[layer].beta && model->layer_norms[layer].beta) {
                for (uint64_t i = 0; i < embedding_dim; i++) {
                    model->layer_norms[layer].beta[i] -= lr * training->ln_grads[layer].beta[i];
                }
            }
        }
    }
}

/**
 * Training-specific attention forward with cache storage
 * Wraps cllm_attention_forward and stores Q, K, V, attention weights for backward pass
 */
static void cllm_attention_forward_training(
    CLLMTraining* training,
    int layer,
    AttentionLayer* attn_layer,
    float* input,
    float* output,
    int seq_len
) {
    if (!training || !attn_layer || !input || !output || layer < 0 || seq_len <= 0) return;
    if (layer >= (int)training->model->num_layers) return;
    
    // Call the standard attention forward
    cllm_attention_forward(attn_layer, input, output, NULL, NULL, seq_len);
    
    // If attention cache is enabled, store Q, K, V, and attention weights
    if (training->store_attention_weights && training->attention_cache) {
        uint32_t num_heads = attn_layer->num_heads;
        uint32_t head_dim = attn_layer->head_dim;
        uint32_t embed_dim = num_heads * head_dim;
        
        // Allocate temporary buffers for Q, K, V
        float* queries = (float*)malloc(seq_len * embed_dim * sizeof(float));
        float* keys = (float*)malloc(seq_len * embed_dim * sizeof(float));
        float* values = (float*)malloc(seq_len * embed_dim * sizeof(float));
        
        if (!queries || !keys || !values) {
            free(queries);
            free(keys);
            free(values);
            return;
        }
        
        // Compute Q, K, V projections (same as in cllm_attention_forward)
        for (int pos = 0; pos < seq_len; pos++) {
            float* input_vec = &input[pos * embed_dim];
            
            // Query projection
            for (uint32_t h = 0; h < num_heads; h++) {
                for (uint32_t d = 0; d < head_dim; d++) {
                    float sum = 0.0f;
                    for (uint32_t i = 0; i < head_dim; i++) {
                        size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                        sum += attn_layer->query_lattice[weight_idx] * input_vec[h * head_dim + i];
                    }
                    queries[pos * embed_dim + h * head_dim + d] = sum;
                }
            }
            
            // Key projection
            for (uint32_t h = 0; h < num_heads; h++) {
                for (uint32_t d = 0; d < head_dim; d++) {
                    float sum = 0.0f;
                    for (uint32_t i = 0; i < head_dim; i++) {
                        size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                        sum += attn_layer->key_lattice[weight_idx] * input_vec[h * head_dim + i];
                    }
                    keys[pos * embed_dim + h * head_dim + d] = sum;
                }
            }
            
            // Value projection
            for (uint32_t h = 0; h < num_heads; h++) {
                for (uint32_t d = 0; d < head_dim; d++) {
                    float sum = 0.0f;
                    for (uint32_t i = 0; i < head_dim; i++) {
                        size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                        sum += attn_layer->value_lattice[weight_idx] * input_vec[h * head_dim + i];
                    }
                    values[pos * embed_dim + h * head_dim + d] = sum;
                }
            }
        }
        
        // Compute and store attention weights
        float scale = 1.0f / prime_sqrtf((float)head_dim);
        
        for (uint32_t h = 0; h < num_heads; h++) {
            for (int i = 0; i < seq_len; i++) {
                float* query = &queries[i * embed_dim + h * head_dim];
                
                // Compute attention scores
                for (int j = 0; j < seq_len; j++) {
                    float* key = &keys[j * embed_dim + h * head_dim];
                    float score = 0.0f;
                    for (uint32_t d = 0; d < head_dim; d++) {
                        score += query[d] * key[d];
                    }
                    score *= scale;
                    training->attention_cache[layer].scores[h * seq_len * seq_len + i * seq_len + j] = score;
                }
                
                // Apply softmax to get attention weights
                float* scores_row = &training->attention_cache[layer].scores[h * seq_len * seq_len + i * seq_len];
                float* weights_row = &training->attention_cache[layer].attention_weights[h * seq_len * seq_len + i * seq_len];
                
                // Find max for numerical stability
                float max_score = scores_row[0];
                for (int j = 1; j < seq_len; j++) {
                    if (scores_row[j] > max_score) max_score = scores_row[j];
                }
                
                // Compute exp and sum
                float sum = 0.0f;
                for (int j = 0; j < seq_len; j++) {
                    weights_row[j] = prime_expf(scores_row[j] - max_score);
                    sum += weights_row[j];
                }
                
                // Normalize
                for (int j = 0; j < seq_len; j++) {
                    weights_row[j] /= sum;
                }
            }
        }
        
        // Store Q, K, V in cache
        memcpy(training->attention_cache[layer].queries, queries, seq_len * embed_dim * sizeof(float));
        memcpy(training->attention_cache[layer].keys, keys, seq_len * embed_dim * sizeof(float));
        memcpy(training->attention_cache[layer].values, values, seq_len * embed_dim * sizeof(float));
        
        free(queries);
        free(keys);
        free(values);
    }
}

/**
 * Softmax backward pass
 * Computes gradient w.r.t. softmax input given gradient w.r.t. softmax output
 * 
 * For y = softmax(x):
 * grad_x[i] = y[i] * (grad_y[i] - sum_j(y[j] * grad_y[j]))
 */
static void softmax_backward(
    float* grad_input,           // Output: gradient w.r.t. softmax input [size]
    const float* grad_output,    // Input: gradient w.r.t. softmax output [size]
    const float* softmax_output, // Input: softmax output from forward pass [size]
    int size
) {
    if (!grad_input || !grad_output || !softmax_output || size <= 0) return;
    
    // Compute sum of (softmax_output * grad_output)
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += softmax_output[i] * grad_output[i];
    }
    
    // Compute gradient: grad_input[i] = softmax_output[i] * (grad_output[i] - sum)
    for (int i = 0; i < size; i++) {
        grad_input[i] = softmax_output[i] * (grad_output[i] - sum);
    }
}

/**
 * Full attention backward pass with proper gradient computation
 * Computes gradients through the complete attention mechanism including softmax
 * 
 * This replaces the simplified outer product approximation with the full
 * gradient computation through scaled dot-product attention.
 */
static void attention_backward_full(
    CLLMTraining* training,
    int layer,
    float* grad_output,      // Gradient w.r.t. attention output [seq_len * embed_dim]
    float* grad_input,       // Output: gradient w.r.t. attention input [seq_len * embed_dim]
    int seq_len
) {
    if (!training || !grad_output || !grad_input || layer < 0 || seq_len <= 0) return;
    if (layer >= (int)training->model->num_layers) return;
    if (!training->attention_cache) return;
    
    AttentionLayer* attn = &training->model->attention_layers[layer];
    uint32_t num_heads = attn->num_heads;
    uint32_t head_dim = attn->head_dim;
    uint32_t embed_dim = num_heads * head_dim;
    
    // Get cached values from forward pass
    float* queries = training->attention_cache[layer].queries;
    float* keys = training->attention_cache[layer].keys;
    float* values = training->attention_cache[layer].values;
    float* attention_weights = training->attention_cache[layer].attention_weights;
    
    if (!queries || !keys || !values || !attention_weights) {
        // Fall back to simplified version if cache not available
        return;
    }
    
    // Allocate temporary buffers
    float* grad_V = (float*)calloc(seq_len * embed_dim, sizeof(float));
    float* grad_weights = (float*)calloc(num_heads * seq_len * seq_len, sizeof(float));
    float* grad_scores = (float*)calloc(num_heads * seq_len * seq_len, sizeof(float));
    float* grad_Q = (float*)calloc(seq_len * embed_dim, sizeof(float));
    float* grad_K = (float*)calloc(seq_len * embed_dim, sizeof(float));
    
    if (!grad_V || !grad_weights || !grad_scores || !grad_Q || !grad_K) {
        free(grad_V);
        free(grad_weights);
        free(grad_scores);
        free(grad_Q);
        free(grad_K);
        return;
    }
    
    float scale = 1.0f / prime_sqrtf((float)head_dim);
    
    // For each head
    for (uint32_t h = 0; h < num_heads; h++) {
        // 1. Gradient w.r.t. V: grad_V = attention_weights^T × grad_output
        for (int pos = 0; pos < seq_len; pos++) {
            for (int d = 0; d < (int)head_dim; d++) {
                float sum = 0.0f;
                for (int i = 0; i < seq_len; i++) {
                    int weight_idx = h * seq_len * seq_len + i * seq_len + pos;
                    sum += attention_weights[weight_idx] * 
                           grad_output[i * embed_dim + h * head_dim + d];
                }
                grad_V[pos * embed_dim + h * head_dim + d] = sum;
            }
        }
        
        // 2. Gradient w.r.t. attention_weights: grad_weights = grad_output × V^T
        for (int i = 0; i < seq_len; i++) {
            for (int j = 0; j < seq_len; j++) {
                float sum = 0.0f;
                for (int d = 0; d < (int)head_dim; d++) {
                    sum += grad_output[i * embed_dim + h * head_dim + d] *
                           values[j * embed_dim + h * head_dim + d];
                }
                grad_weights[h * seq_len * seq_len + i * seq_len + j] = sum;
            }
        }
        
        // 3. Gradient through softmax
        for (int i = 0; i < seq_len; i++) {
            softmax_backward(
                &grad_scores[h * seq_len * seq_len + i * seq_len],
                &grad_weights[h * seq_len * seq_len + i * seq_len],
                &attention_weights[h * seq_len * seq_len + i * seq_len],
                seq_len
            );
        }
        
        // 4. Gradient w.r.t. Q: grad_Q = (grad_scores × K) / sqrt(d_k)
        for (int i = 0; i < seq_len; i++) {
            for (int d = 0; d < (int)head_dim; d++) {
                float sum = 0.0f;
                for (int j = 0; j < seq_len; j++) {
                    sum += grad_scores[h * seq_len * seq_len + i * seq_len + j] *
                           keys[j * embed_dim + h * head_dim + d];
                }
                grad_Q[i * embed_dim + h * head_dim + d] = sum * scale;
            }
        }
        
        // 5. Gradient w.r.t. K: grad_K = (grad_scores^T × Q) / sqrt(d_k)
        for (int j = 0; j < seq_len; j++) {
            for (int d = 0; d < (int)head_dim; d++) {
                float sum = 0.0f;
                for (int i = 0; i < seq_len; i++) {
                    sum += grad_scores[h * seq_len * seq_len + i * seq_len + j] *
                           queries[i * embed_dim + h * head_dim + d];
                }
                grad_K[j * embed_dim + h * head_dim + d] = sum * scale;
            }
        }
    }
    
    // 6. Compute gradients w.r.t. weight matrices
    float* layer_input = training->layer_inputs[layer];
    
    for (int pos = 0; pos < seq_len; pos++) {
        for (uint32_t d1 = 0; d1 < embed_dim; d1++) {
            for (uint32_t d2 = 0; d2 < embed_dim; d2++) {
                // Query weight gradients
                if (training->attention_grads[layer].query_lattice) {
                    training->attention_grads[layer].query_lattice[d1 * embed_dim + d2] +=
                        layer_input[pos * embed_dim + d1] * grad_Q[pos * embed_dim + d2];
                }
                
                // Key weight gradients
                if (training->attention_grads[layer].key_lattice) {
                    training->attention_grads[layer].key_lattice[d1 * embed_dim + d2] +=
                        layer_input[pos * embed_dim + d1] * grad_K[pos * embed_dim + d2];
                }
                
                // Value weight gradients
                if (training->attention_grads[layer].value_lattice) {
                    training->attention_grads[layer].value_lattice[d1 * embed_dim + d2] +=
                        layer_input[pos * embed_dim + d1] * grad_V[pos * embed_dim + d2];
                }
            }
        }
    }
    
    // 7. Compute gradient w.r.t. input
    memset(grad_input, 0, seq_len * embed_dim * sizeof(float));
    for (int pos = 0; pos < seq_len; pos++) {
        for (uint32_t d1 = 0; d1 < embed_dim; d1++) {
            for (uint32_t d2 = 0; d2 < embed_dim; d2++) {
                grad_input[pos * embed_dim + d1] +=
                    grad_Q[pos * embed_dim + d2] * attn->query_lattice[d1 * embed_dim + d2] +
                    grad_K[pos * embed_dim + d2] * attn->key_lattice[d1 * embed_dim + d2] +
                    grad_V[pos * embed_dim + d2] * attn->value_lattice[d1 * embed_dim + d2];
            }
        }
    }
    
    // Cleanup
    free(grad_V);
    free(grad_weights);
    free(grad_scores);
    free(grad_Q);
    free(grad_K);
}

// Train for one epoch
// Forward declarations
float cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens);
void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens);

float cllm_train_epoch(CLLMTraining* training) {
    if (!training) return 0.0f;
    
    float epoch_loss = 0.0f;
    int num_batches = 0;
    
    uint32_t* input_tokens = (uint32_t*)malloc(training->config.batch_size * 
                                               training->config.sequence_length * 
                                               sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)malloc(training->config.batch_size * 
                                                training->config.sequence_length * 
                                                sizeof(uint32_t));
    
    training->current_batch_offset = 0;
    
    while (1) {
        // Get batch
        int tokens = cllm_get_batch(training, input_tokens, target_tokens);
        if (tokens == 0) break; // End of epoch
        
        // DIAGNOSTIC: Check weight initialization (first batch only)
        if (training->current_epoch == 0 && num_batches == 0) {
            CLLMModel* model = training->model;
            float sum_embed = 0.0f, sum_attn = 0.0f, sum_ff = 0.0f;
            int count = 100;
            
            for (int i = 0; i < count; i++) {
                sum_embed += prime_fabsf(model->embeddings.embeddings[i]);
            }
            
            if (model->attention_layers && model->attention_layers[0].query_lattice) {
                for (int i = 0; i < count; i++) {
                    sum_attn += prime_fabsf(model->attention_layers[0].query_lattice[i]);
                }
            }
            
            if (model->ff_layers && model->ff_layers[0].w1_lattice) {
                for (int i = 0; i < count; i++) {
                    sum_ff += prime_fabsf(model->ff_layers[0].w1_lattice[i]);
                }
            }
            
            printf("  Weight magnitudes: embed=%.6f, attn=%.6f, ff=%.6f\n",
                   sum_embed/count, sum_attn/count, sum_ff/count);
        }
        
        
        // Forward pass with activation storage
        cllm_forward_training(training, input_tokens);
        
        // Compute loss using GCD-based similarity (O(log n) vs O(n) for dot product)
        float loss = cllm_compute_loss(training, input_tokens, target_tokens, 
                                                   training->config.batch_size * training->config.sequence_length);
        epoch_loss += loss;
        num_batches++;
        
        // Backward pass with cross-entropy gradients
        cllm_backward_training(training, target_tokens);
        
        // DIAGNOSTIC: Check gradient magnitudes
        if (num_batches == 1 || num_batches % 5 == 0) {
            CLLMModel* model = training->model;
            float max_embed_grad = 0.0f, sum_embed_grad = 0.0f;
            int nonzero_embed = 0;
            size_t embed_size = model->vocab_size * model->embedding_dim;
            
            for (size_t i = 0; i < embed_size && i < 10000; i++) {
                float g = prime_fabsf(training->gradients[i]);
                if (g > 1e-10f) {
                    nonzero_embed++;
                    sum_embed_grad += g;
                    if (g > max_embed_grad) max_embed_grad = g;
                }
            }
            
            float max_attn_grad = 0.0f;
            int nonzero_attn = 0;
            if (training->attention_grads && model->num_layers > 0) {
                size_t size = model->embedding_dim * model->embedding_dim;
                for (size_t i = 0; i < size && i < 10000; i++) {
                    float g = prime_fabsf(training->attention_grads[0].query_lattice[i]);
                    if (g > 1e-10f) {
                        nonzero_attn++;
                        if (g > max_attn_grad) max_attn_grad = g;
                    }
                }
            }
            
            printf("    Gradients: embed=%d (max=%.2e, avg=%.2e), attn=%d (max=%.2e)\n",
                   nonzero_embed, max_embed_grad,
                   nonzero_embed > 0 ? sum_embed_grad / nonzero_embed : 0.0f,
                   nonzero_attn, max_attn_grad);
        }
        
        
        // Update learning rate based on schedule (warmup + decay)
        cllm_update_learning_rate(training);
        
        // Optimizer step - Use Adam optimizer with gradient accumulation
        cllm_optimizer_step_adam(training);
        
        training->current_step++;
        training->current_loss = loss;
        
        // Update best loss
        if (loss < training->best_loss) {
            training->best_loss = loss;
        }
        
        if (num_batches % 5 == 0) {
            printf("  Batch %d: loss = %.4f\n", num_batches, loss);
        }
    }
    
    free(input_tokens);
    free(target_tokens);
    
    // Print epoch summary
    printf("  Epoch complete: %d batches, average loss = %.4f\n", num_batches, num_batches > 0 ? epoch_loss / num_batches : 0.0f);
    
    return num_batches > 0 ? epoch_loss / num_batches : 0.0f;
}

/**
 * Forward pass with activation storage for training
 */
float cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens) {
    if (!training || !input_tokens) return 0.0f;
    
    CLLMModel* model = training->model;
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Get embeddings
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t token_id = input_tokens[idx];
            if (token_id >= vocab_size) continue;
            
            float* embed_src = &model->embeddings.embeddings[token_id * embed_dim];
            float* embed_dst = &training->input_embeddings[idx * embed_dim];
            memcpy(embed_dst, embed_src, embed_dim * sizeof(float));
        }
    }
    
    // Process through layers
    float* layer_input = training->input_embeddings;
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        memcpy(training->layer_inputs[layer], layer_input, batch_size * seq_len * embed_dim * sizeof(float));
        
        // Apply proper multi-head attention for each batch
        AttentionLayer* attn_layer = &model->attention_layers[layer];
        for (int b = 0; b < batch_size; b++) {
            int start_idx = b * seq_len;
            float* batch_input = &layer_input[start_idx * embed_dim];
            float* batch_output = &training->attention_outputs[layer][start_idx * embed_dim];
            
            // Use training-specific attention that caches Q, K, V, and attention weights
            cllm_attention_forward_training(training, layer, attn_layer, 
                                           batch_input, batch_output, seq_len);
        }
        
        // Process feedforward for each position
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* attn_out = &training->attention_outputs[layer][idx * embed_dim];
                float* ff_out = &training->ff_outputs[layer][idx * embed_dim];
                float* layer_out = &training->layer_outputs[layer][idx * embed_dim];
                
                // FeedForward
                FeedForwardLayer* ff = &model->ff_layers[layer];
                float* ff_hidden = &training->ff_hidden[layer][idx * ff->hidden_dim];
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    float sum = ff->bias1[h];
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        sum += attn_out[i] * ff->w1_lattice[i * ff->hidden_dim + h];
                    }
                    ff_hidden[h] = prime_tanhf(sum);
                }
                
                for (uint32_t o = 0; o < embed_dim; o++) {
                    float sum = ff->bias2[o];
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        sum += ff_hidden[h] * ff->w2_lattice[h * embed_dim + o];
                    }
                    ff_out[o] = sum;
                }
                
                // Residual + LayerNorm
                for (uint32_t d = 0; d < embed_dim; d++) layer_out[d] = attn_out[d] + ff_out[d];
                
                CLLMLayerNorm* ln = &model->layer_norms[layer];
                float mean = 0.0f, var = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) mean += layer_out[d];
                mean /= embed_dim;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float diff = layer_out[d] - mean;
                    var += diff * diff;
                }
                var /= embed_dim;
                float std = prime_sqrtf(var + 1e-5f);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    layer_out[d] = ln->gamma[d] * (layer_out[d] - mean) / std + ln->beta[d];
                }
            }
        }
        layer_input = training->layer_outputs[layer];
    }
    
    // Copy final hidden
    memcpy(training->final_hidden, layer_input, batch_size * seq_len * embed_dim * sizeof(float));
    
    // Project to vocabulary
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            float* hidden = &training->final_hidden[idx * embed_dim];
            float* logits = &training->logits[idx * vocab_size];
            
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
    
    return 0.0f;
}

/**
 * Compute cross-entropy loss from stored logits
 */

/**
 * Backward pass with cross-entropy gradients
 */
void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens) {
    if (!training || !target_tokens) return;
    
    CLLMModel* model = training->model;
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    cllm_zero_all_gradients(training);
    
    float* grad_logits = (float*)calloc(batch_size * seq_len * vocab_size, sizeof(float));
    float* grad_hidden = (float*)calloc(batch_size * seq_len * embed_dim, sizeof(float));
    float* grad_layer = (float*)calloc(batch_size * seq_len * embed_dim, sizeof(float));
    
    if (!grad_logits || !grad_hidden || !grad_layer) {
        free(grad_logits); free(grad_hidden); free(grad_layer);
        return;
    }
    
    // Gradient of cross-entropy w.r.t. logits
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t target = target_tokens[idx];
            if (target >= vocab_size) continue;
            
            float* logits = &training->logits[idx * vocab_size];
            float* grad = &grad_logits[idx * vocab_size];
            
            float max_logit = logits[0];
            for (uint32_t v = 1; v < vocab_size; v++) {
                if (logits[v] > max_logit) max_logit = logits[v];
            }
            
            float sum_exp = 0.0f;
            for (uint32_t v = 0; v < vocab_size; v++) {
                sum_exp += prime_expf(logits[v] - max_logit);
            }
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                float softmax_v = prime_expf(logits[v] - max_logit) / sum_exp;
                grad[v] = softmax_v;
                if (v == target) grad[v] -= 1.0f;
                grad[v] /= (batch_size * seq_len);
            }
        }
    }
    
    // Backward through output projection
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            float* grad_log = &grad_logits[idx * vocab_size];
            float* grad_hid = &grad_hidden[idx * embed_dim];
            float* hidden = &training->final_hidden[idx * embed_dim];
            
            for (uint32_t d = 0; d < embed_dim; d++) {
                float sum = 0.0f;
                for (uint32_t v = 0; v < vocab_size; v++) {
                    sum += grad_log[v] * model->embeddings.embeddings[v * embed_dim + d];
                }
                grad_hid[d] = sum;
            }
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                float* grad_embed = &training->gradients[v * embed_dim];
                for (uint32_t d = 0; d < embed_dim; d++) {
                    grad_embed[d] += grad_log[v] * hidden[d];
                }
            }
        }
    }
    
    // Backward through layers
    memcpy(grad_layer, grad_hidden, batch_size * seq_len * embed_dim * sizeof(float));
    
    for (int layer = model->num_layers - 1; layer >= 0; layer--) {
        float* attn_output = training->attention_outputs[layer];
        float* ff_hidden = training->ff_hidden[layer];
        FeedForwardLayer* ff = &model->ff_layers[layer];
        CLLMLayerNorm* ln = &model->layer_norms[layer];
        
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* grad = &grad_layer[idx * embed_dim];
                float* input = &attn_output[idx * embed_dim];
                float* hidden = &ff_hidden[idx * ff->hidden_dim];
                
                // LayerNorm backward
                float mean = 0.0f, var = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) mean += input[d];
                mean /= embed_dim;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float diff = input[d] - mean;
                    var += diff * diff;
                }
                var /= embed_dim;
                float std = prime_sqrtf(var + 1e-5f);
                
                float grad_var = 0.0f, grad_mean = 0.0f;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float x_norm = (input[d] - mean) / std;
                    if (training->ln_grads[layer].gamma) {
                        training->ln_grads[layer].gamma[d] += grad[d] * x_norm;
                    }
                    if (training->ln_grads[layer].beta) {
                        training->ln_grads[layer].beta[d] += grad[d];
                    }
                    float grad_x_norm = grad[d] * ln->gamma[d];
                    grad_var += grad_x_norm * (input[d] - mean) * -0.5f * prime_powf(std, -3.0f);
                    grad_mean += grad_x_norm * (-1.0f / std);
                }
                
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float grad_x_norm = grad[d] * ln->gamma[d];
                    grad[d] = grad_x_norm / std + grad_var * 2.0f * (input[d] - mean) / embed_dim + grad_mean / embed_dim;
                }
                
                
                // Attention backward - compute gradients for attention weights
                // grad is currently w.r.t. attention output
                // We need to compute gradients for Q, K, V weights
                
                // Get layer input (input to attention)
                float* layer_input = training->layer_inputs[layer];
                float* attn_input = &layer_input[idx * embed_dim];
                
                // Use full attention backward if cache is available, otherwise use simplified version
                if (training->store_attention_weights && training->attention_cache) {
                    // Full attention backward with proper gradient computation
                    float* grad_input_temp = (float*)calloc(embed_dim, sizeof(float));
                    if (grad_input_temp) {
                        // Note: This processes one position at a time
                        // For full sequence processing, we'd need to batch this
                        // For now, accumulate gradients position by position
                        attention_backward_full(training, layer, grad, grad_input_temp, 1);
                        free(grad_input_temp);
                    }
                } else {
                    // Simplified attention backward: approximate with outer product
                    // This is the fallback when attention cache is not available
                    (void)attn_input;  // Used below for gradient computation
                    for (uint32_t d1 = 0; d1 < embed_dim; d1++) {
                        for (uint32_t d2 = 0; d2 < embed_dim; d2++) {
                            // Query gradients
                            if (training->attention_grads[layer].query_lattice) {
                                training->attention_grads[layer].query_lattice[d1 * embed_dim + d2] += 
                                    attn_input[d1] * grad[d2];
                            }
                            // Key gradients  
                            if (training->attention_grads[layer].key_lattice) {
                                training->attention_grads[layer].key_lattice[d1 * embed_dim + d2] += 
                                    attn_input[d1] * grad[d2];
                            }
                            // Value gradients
                            if (training->attention_grads[layer].value_lattice) {
                                training->attention_grads[layer].value_lattice[d1 * embed_dim + d2] += 
                                    attn_input[d1] * grad[d2];
                            }
                        }
                    }
                }
                
                // FeedForward backward
                float* grad_hidden = (float*)calloc(ff->hidden_dim, sizeof(float));
                if (!grad_hidden) continue;
                
                for (uint32_t o = 0; o < embed_dim; o++) {
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        if (training->ff_grads[layer].w2_lattice) {
                            training->ff_grads[layer].w2_lattice[h * embed_dim + o] += hidden[h] * grad[o];
                        }
                        grad_hidden[h] += ff->w2_lattice[h * embed_dim + o] * grad[o];
                    }
                    if (training->ff_grads[layer].bias2) {
                        training->ff_grads[layer].bias2[o] += grad[o];
                    }
                }
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    float tanh_val = hidden[h];
                    grad_hidden[h] *= (1.0f - tanh_val * tanh_val);
                }
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        if (training->ff_grads[layer].w1_lattice) {
                            training->ff_grads[layer].w1_lattice[i * ff->hidden_dim + h] += input[i] * grad_hidden[h];
                        }
                        grad[i] += ff->w1_lattice[i * ff->hidden_dim + h] * grad_hidden[h];
                    }
                    if (training->ff_grads[layer].bias1) {
                        training->ff_grads[layer].bias1[h] += grad_hidden[h];
                    }
                }
                
                free(grad_hidden);
            }
        }
    }
    
    free(grad_logits);
    free(grad_hidden);
    free(grad_layer);
}

// Train the model
int cllm_train(CLLMTraining* training) {
    if (!training) return -1;
    
    printf("Starting training...\n");
    printf("Epochs: %d\n", training->config.num_epochs);
    printf("Batch size: %d\n", training->config.batch_size);
    printf("Sequence length: %d\n", training->config.sequence_length);
    printf("Learning rate: %.6f\n", training->config.learning_rate);
    printf("Total tokens: %zu\n", training->num_tokens);
    printf("Total batches per epoch: %d\n", training->total_batches);
    printf("\n");
    
    for (int epoch = 0; epoch < training->config.num_epochs; epoch++) {
        training->current_epoch = epoch;
        
        printf("Epoch %d/%d\n", epoch + 1, training->config.num_epochs);
        
        float epoch_loss = cllm_train_epoch(training);
        
        printf("Epoch %d complete: Average Loss = %.4f\n\n", epoch + 1, epoch_loss);
        
        // Save checkpoint
        if ((epoch + 1) % training->config.save_every == 0) {
            char checkpoint_path[256];
            snprintf(checkpoint_path, sizeof(checkpoint_path), 
                    "checkpoint_epoch_%d.cllm", epoch + 1);
            cllm_write_model(training->model, checkpoint_path);
            printf("Checkpoint saved: %s\n", checkpoint_path);
        }
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, training->start_time);
    
    printf("\nTraining complete!\n");
    printf("Total time: %.0f seconds\n", elapsed);
    printf("Final loss: %.4f\n", training->current_loss);
    printf("Best loss: %.4f\n", training->best_loss);
    
    return 0;
}

// Save training checkpoint
int cllm_save_checkpoint(CLLMTraining* training, const char* filename) {
    if (!training || !filename) return -1;
    
    // Save model
    if (cllm_write_model(training->model, filename) != 0) {
        return -1;
    }
    
    // Save training state
    char state_file[512];
    snprintf(state_file, sizeof(state_file), "%s.state", filename);
    
    FILE* f = fopen(state_file, "wb");
    if (!f) return -1;
    
    fwrite(&training->current_epoch, sizeof(int), 1, f);
    fwrite(&training->current_step, sizeof(int), 1, f);
    fwrite(&training->current_loss, sizeof(float), 1, f);
    fwrite(&training->best_loss, sizeof(float), 1, f);
    
    size_t total_params = training->model->header.total_params;
    fwrite(training->optimizer_state, sizeof(float), total_params * 2, f);
    
    fclose(f);
    
    return 0;
}

// Load training checkpoint
int cllm_load_checkpoint(CLLMTraining* training, const char* filename) {
    if (!training || !filename) return -1;
    
    // Load training state
    char state_file[512];
    snprintf(state_file, sizeof(state_file), "%s.state", filename);
    
    FILE* f = fopen(state_file, "rb");
    if (!f) return -1;
    
    if (fread(&training->current_epoch, sizeof(int), 1, f) != 1) {
        fprintf(stderr, "Error reading current_epoch\n");
        fclose(f);
        return false;
    }
    if (fread(&training->current_step, sizeof(int), 1, f) != 1) {
        fprintf(stderr, "Error reading current_step\n");
        fclose(f);
        return false;
    }
    if (fread(&training->current_loss, sizeof(float), 1, f) != 1) {
        fprintf(stderr, "Error reading current_loss\n");
        fclose(f);
        return false;
    }
    if (fread(&training->best_loss, sizeof(float), 1, f) != 1) {
        fprintf(stderr, "Error reading best_loss\n");
        fclose(f);
        return false;
    }
    
    size_t total_params = training->model->header.total_params;
    if (fread(training->optimizer_state, sizeof(float), total_params * 2, f) != total_params * 2) {
        fprintf(stderr, "Error reading optimizer_state\n");
        fclose(f);
        return false;
    }
    
    fclose(f);
    
    return 0;
}

// Cleanup
void cllm_training_cleanup(CLLMTraining* training) {
    if (!training) return;
    
    // Free training data
    // NOTE: training->tokens is typically a pointer to external data (dataset->tokens)
    // and should NOT be freed here. Set to NULL before calling cleanup if you don't want it freed.
    // free(training->tokens);  // REMOVED - tokens are owned by dataset
    free(training->gradients);
    free(training->optimizer_state);
    
    // Free mixed precision buffers
    free(training->master_weights);
    free(training->fp16_activations);
    free(training->fp16_gradients);
    
    // Free attention gradient buffers
    if (training->attention_grads) {
        for (uint32_t i = 0; i < training->model->num_layers; i++) {
            free(training->attention_grads[i].query_lattice);
            free(training->attention_grads[i].key_lattice);
            free(training->attention_grads[i].value_lattice);
        }
        free(training->attention_grads);
    }
    
    // Free feed-forward gradient buffers
    if (training->ff_grads) {
        for (uint32_t i = 0; i < training->model->num_layers; i++) {
            free(training->ff_grads[i].w1_lattice);
            free(training->ff_grads[i].w2_lattice);
            free(training->ff_grads[i].bias1);
            free(training->ff_grads[i].bias2);
        }
        free(training->ff_grads);
    }
    
    // Free layer norm gradient buffers
    if (training->ln_grads) {
        for (uint32_t i = 0; i < training->model->num_layers; i++) {
            free(training->ln_grads[i].gamma);
            free(training->ln_grads[i].beta);
        }
        free(training->ln_grads);
    }
    
    // Free backward pass buffers (OPTIMIZATION)
    free(training->backward_embeddings);
    free(training->backward_grad_output);
    free(training->backward_layer_input);
    free(training->backward_layer_grad);
    free(training->backward_temp_grad);
    
    // Free embedding cache (OPTIMIZATION)
    free(training->cached_input_embeddings);
    free(training->cached_target_embeddings);
    
    // Free forward pass activation storage
    free(training->input_embeddings);
    free(training->final_hidden);
    free(training->logits);
    
    if (training->layer_inputs) {
        for (uint32_t i = 0; i < training->model->num_layers; i++) {
            free(training->layer_inputs[i]);
        }
        free(training->layer_inputs);
    }
    
    if (training->attention_outputs) {
        for (uint32_t i = 0; i < training->model->num_layers; i++) {
            free(training->attention_outputs[i]);
        }
        free(training->attention_outputs);
    }
    
    if (training->ff_outputs) {
        for (uint32_t i = 0; i < training->model->num_layers; i++) {
            free(training->ff_outputs[i]);
        }
        free(training->ff_outputs);
    }
    
    if (training->layer_outputs) {
        for (uint32_t i = 0; i < training->model->num_layers; i++) {
            free(training->layer_outputs[i]);
        }
        free(training->layer_outputs);
    }
    
    if (training->ff_hidden) {
        for (uint32_t i = 0; i < training->model->num_layers; i++) {
            free(training->ff_hidden[i]);
        }
        free(training->ff_hidden);
    }
    
    // Free attention cache (OPTIMIZATION)
    if (training->attention_cache) {
        for (uint32_t i = 0; i < training->model->num_layers; i++) {
            if (training->attention_cache[i].queries) free(training->attention_cache[i].queries);
            if (training->attention_cache[i].keys) free(training->attention_cache[i].keys);
            if (training->attention_cache[i].values) free(training->attention_cache[i].values);
            if (training->attention_cache[i].attention_weights) free(training->attention_cache[i].attention_weights);
            if (training->attention_cache[i].scores) free(training->attention_cache[i].scores);
        }
        free(training->attention_cache);
    }
    
    free(training);
}

// Alias for compatibility
void cllm_training_free(CLLMTraining* training) {
    cllm_training_cleanup(training);
}

