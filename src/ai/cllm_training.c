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
    float beta1 = 0.9f;
    float beta2 = 0.999f;
    
    size_t total_params = training->model->header.total_params;
    float* m = training->optimizer_state;  // First moment
    float* v = &training->optimizer_state[total_params];  // Second moment
    
    int t = training->current_step + 1;
    float lr_t = lr * sqrtf(1.0f - powf(beta2, (float)t)) / 
                      (1.0f - powf(beta1, (float)t));
    float epsilon = 1e-8f;
    
    // Get model parameters
    CLLMModel* model = training->model;
    uint32_t embedding_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    size_t embed_params = vocab_size * embedding_dim;
    float* embeddings = model->embeddings.embeddings;
    
    // Update embedding parameters
    for (size_t i = 0; i < embed_params && i < total_params; i++) {
        float g = training->gradients[i];
        
        // Update biased first moment estimate
        m[i] = beta1 * m[i] + (1.0f - beta1) * g;
        
        // Update biased second raw moment estimate
        v[i] = beta2 * v[i] + (1.0f - beta2) * g * g;
        
        // Compute and apply update
        float update = lr_t * m[i] / (prime_sqrtf(v[i]) + epsilon);
        
        if (embeddings) {
            embeddings[i] -= update;
        }
    }
}

// Train for one epoch
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
        
        // Forward pass
        float loss = cllm_compute_loss(training, input_tokens, target_tokens, tokens);
        epoch_loss += loss;
        num_batches++;
        
        // Backward pass
        cllm_backward(training, input_tokens, target_tokens, tokens);
        
        // Optimizer step
        cllm_optimizer_step(training);
        
        training->current_step++;
        training->current_loss = loss;
        
        // Update best loss
        if (loss < training->best_loss) {
            training->best_loss = loss;
        }
        
        // Silent training - progress shown in UI
        // (No terminal spam)
    }
    
    free(input_tokens);
    free(target_tokens);
    
    return num_batches > 0 ? epoch_loss / num_batches : 0.0f;
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
    
    free(training);
}

// Alias for compatibility
void cllm_training_free(CLLMTraining* training) {
    cllm_training_cleanup(training);
}


