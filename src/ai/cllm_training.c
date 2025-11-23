/*
 * CLLM Training Pipeline
 * Training system for Crystalline Lattice Language Models
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "../include/cllm_format.h"
#include "../include/cllm_training.h"
#include "../include/prime_float_math.h"
#include "../include/cllm_simd_utils.h"

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
    training->num_tokens = 0;
    training->tokens = (uint32_t*)malloc(file_size * sizeof(uint32_t)); // Overallocate
    
    // Check if model has vocabulary
    if (!training->model->tokens) {
        fprintf(stderr, "Warning: Model has no vocabulary, using character-based tokenization\n");
        // Fallback: character-based tokenization
        for (size_t i = 0; i < bytes_read && training->num_tokens < (size_t)file_size; i++) {
            if (content[i] != '\n' && content[i] != '\r') {
                training->tokens[training->num_tokens++] = (uint32_t)(content[i] % training->model->vocab_size);
            }
        }
    } else {
        // Use vocabulary-based tokenization
        char* token = strtok(content, " \n\t");
        while (token != NULL && training->num_tokens < (size_t)file_size) {
            // Find token in vocabulary
            bool found = false;
            for (uint32_t i = 0; i < training->model->vocab_size; i++) {
                if (strcmp(training->model->tokens[i].token_str, token) == 0) {
                    training->tokens[training->num_tokens++] = i;
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
                training->tokens[training->num_tokens++] = hash % training->model->vocab_size;
            }
            token = strtok(NULL, " \n\t");
        }
    }
    
    free(content);
    
    // Calculate number of batches
    int tokens_per_batch = training->config.batch_size * training->config.sequence_length;
    training->total_batches = training->num_tokens / tokens_per_batch;
    
    return training->num_tokens;
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
float cllm_compute_loss(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens, int num_tokens __attribute__((unused))) {
    if (!training || !input_tokens || !target_tokens) return 0.0f;
    
    // Cache all embeddings for batch (OPTIMIZATION)
    cache_batch_embeddings(training, input_tokens, target_tokens, num_tokens);
    
    float total_loss = 0.0f;
    int count = 0;
    uint64_t embed_dim = training->model->embedding_dim;
    
    // Simplified loss computation using cached embeddings
    for (int i = 0; i < num_tokens; i++) {
        uint32_t input = input_tokens[i];
        uint32_t target = target_tokens[i];
        
        if (input < training->model->vocab_size && target < training->model->vocab_size) {
            float* input_embed;
            float* target_embed;
            
            // Use cached embeddings if within cache bounds (OPTIMIZATION)
            if (i < training->cached_batch_size) {
                input_embed = get_cached_input_embedding(training, i);
                target_embed = get_cached_target_embedding(training, i);
            } else {
                // Fallback to direct lookup for tokens beyond cache
                input_embed = &training->model->embeddings.embeddings[input * embed_dim];
                target_embed = &training->model->embeddings.embeddings[target * embed_dim];
            }
            
            // Use vectorized dot product (OPTIMIZATION - 4-8x faster)
            float similarity = dot_product(input_embed, target_embed, embed_dim);
            
            // Convert to loss (negative log likelihood approximation)
            float clamped = similarity > 1e-10f ? similarity : 1e-10f;
            total_loss += -logf(clamped);
            count++;
        }
    }
    
    return count > 0 ? total_loss / count : 0.0f;
}

// Forward declaration
// cllm_backward is now implemented in cllm_backward.c

// Adam optimizer step
void cllm_optimizer_step(CLLMTraining* training) {
    if (!training) return;
    
    float lr = training->config.learning_rate;
    CLLMModel* model = training->model;
    
    // Simple SGD update (no momentum for now - just get it working)
    // Update embeddings
    uint32_t embedding_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    size_t embed_params = vocab_size * embedding_dim;
    
    if (model->embeddings.embeddings && training->gradients) {
        for (size_t i = 0; i < embed_params; i++) {
            model->embeddings.embeddings[i] -= lr * training->gradients[i];
        }
    }
    
    // Update layer weights
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Update attention weights
        if (training->attention_grads && model->attention_layers) {
            uint64_t attn_size = embedding_dim * embedding_dim;
            
            if (training->attention_grads[layer].query_lattice && model->attention_layers[layer].query_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].query_lattice[i] -= lr * training->attention_grads[layer].query_lattice[i];
                }
            }
            
            if (training->attention_grads[layer].key_lattice && model->attention_layers[layer].key_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].key_lattice[i] -= lr * training->attention_grads[layer].key_lattice[i];
                }
            }
            
            if (training->attention_grads[layer].value_lattice && model->attention_layers[layer].value_lattice) {
                for (uint64_t i = 0; i < attn_size; i++) {
                    model->attention_layers[layer].value_lattice[i] -= lr * training->attention_grads[layer].value_lattice[i];
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
                    ff->w1_lattice[i] -= lr * training->ff_grads[layer].w1_lattice[i];
                }
            }
            
            if (training->ff_grads[layer].w2_lattice && ff->w2_lattice) {
                for (uint32_t i = 0; i < hidden_dim * output_dim; i++) {
                    ff->w2_lattice[i] -= lr * training->ff_grads[layer].w2_lattice[i];
                }
            }
            
            if (training->ff_grads[layer].bias1 && ff->bias1) {
                for (uint32_t i = 0; i < hidden_dim; i++) {
                    ff->bias1[i] -= lr * training->ff_grads[layer].bias1[i];
                }
            }
            
            if (training->ff_grads[layer].bias2 && ff->bias2) {
                for (uint32_t i = 0; i < output_dim; i++) {
                    ff->bias2[i] -= lr * training->ff_grads[layer].bias2[i];
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

// Train for one epoch
// Forward declarations
static float cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens);
static float cllm_compute_loss_training(CLLMTraining* training, uint32_t* target_tokens);
static void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens);

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
        
        // Forward pass with activation storage
        cllm_forward_training(training, input_tokens);
        
        // Compute loss from stored logits
        float loss = cllm_compute_loss_training(training, target_tokens);
        epoch_loss += loss;
        num_batches++;
        
        // Backward pass with cross-entropy gradients
        cllm_backward_training(training, target_tokens);
        
        // Optimizer step
        cllm_optimizer_step(training);
        
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
    
    return num_batches > 0 ? epoch_loss / num_batches : 0.0f;
}

/**
 * Forward pass with activation storage for training
 */
static float cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens) {
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
        
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                float* input = &layer_input[idx * embed_dim];
                float* attn_out = &training->attention_outputs[layer][idx * embed_dim];
                float* ff_out = &training->ff_outputs[layer][idx * embed_dim];
                float* layer_out = &training->layer_outputs[layer][idx * embed_dim];
                
                // Attention (simplified)
                memcpy(attn_out, input, embed_dim * sizeof(float));
                for (uint32_t d = 0; d < embed_dim; d++) attn_out[d] += input[d];
                
                // FeedForward
                FeedForwardLayer* ff = &model->ff_layers[layer];
                float* ff_hidden = &training->ff_hidden[layer][idx * ff->hidden_dim];
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    float sum = ff->bias1[h];
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        sum += attn_out[i] * ff->w1_lattice[i * ff->hidden_dim + h];
                    }
                    ff_hidden[h] = tanhf(sum);
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
                float std = sqrtf(var + 1e-5f);
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
static float cllm_compute_loss_training(CLLMTraining* training, uint32_t* target_tokens) {
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    uint32_t vocab_size = training->model->vocab_size;
    
    float total_loss = 0.0f;
    int count = 0;
    
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t target = target_tokens[idx];
            if (target >= vocab_size) continue;
            
            float* logits = &training->logits[idx * vocab_size];
            
            float max_logit = logits[0];
            for (uint32_t v = 1; v < vocab_size; v++) {
                if (logits[v] > max_logit) max_logit = logits[v];
            }
            
            float sum_exp = 0.0f;
            for (uint32_t v = 0; v < vocab_size; v++) {
                sum_exp += expf(logits[v] - max_logit);
            }
            
            float log_prob = (logits[target] - max_logit) - logf(sum_exp);
            total_loss += -log_prob;
            count++;
        }
    }
    
    return count > 0 ? total_loss / count : 0.0f;
}

/**
 * Backward pass with cross-entropy gradients
 */
static void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens) {
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
                sum_exp += expf(logits[v] - max_logit);
            }
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                float softmax_v = expf(logits[v] - max_logit) / sum_exp;
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
                float std = sqrtf(var + 1e-5f);
                
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
                    grad_var += grad_x_norm * (input[d] - mean) * -0.5f * powf(std, -3.0f);
                    grad_mean += grad_x_norm * (-1.0f / std);
                }
                
                for (uint32_t d = 0; d < embed_dim; d++) {
                    float grad_x_norm = grad[d] * ln->gamma[d];
                    grad[d] = grad_x_norm / std + grad_var * 2.0f * (input[d] - mean) / embed_dim + grad_mean / embed_dim;
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
            cllm_write(checkpoint_path, training->model);
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
    if (cllm_write(filename, training->model) != 0) {
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
    
    free(training);
}

// Alias for compatibility
void cllm_training_free(CLLMTraining* training) {
    cllm_training_cleanup(training);
}

