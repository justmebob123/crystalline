// BigFixed helper macros
#define BF_CREATE_INIT(var, prec) BigFixed var; var = *big_fixed_create(prec)
#define BF_FREE(var) big_fixed_free(&var)

/*
 * CLLM Training Pipeline - Core Training Operations
 * 
 * This file contains the core training operations:
 * - Crystalline loss computation (GCD-based, O(log n))
 * - Forward/backward passes
 * - Optimizer steps
 * - Checkpoint management
 * 
 * For parallel training, use cllm_training_threaded.c
 * The functions here are used as building blocks by the parallel system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "../include/prime_float_math.h"
#include "../include/cllm_format.h"
#include "../include/cllm_training.h"
#include "../algorithms/include/bigfixed_math_wrappers.h"
#include "../include/ai/cllm_simple_loss.h"
#include "../include/cllm_inference.h"
#include "../include/prime_float_math.h"
#include "../include/cllm_simd_utils.h"
#include "../include/ai/cllm_cymatic_training.h"
// #include "../include/cllm_crystalline_training.h"
#include "bigfixed_core.h"
#include "bigfixed_array_utils.h"  // CONSOLIDATED: Functions moved here

#define MAX_BATCH_SIZE 128
#define MAX_SEQUENCE_LENGTH 2048

// ============================================================================
// Crystalline Loss Functions (Consolidated from cllm_crystalline_training.c)
// ============================================================================

/**
 * Compute GCD of two numbers (Euclidean algorithm)
 * O(log n) complexity vs O(n) for dot product
 */
static uint32_t gcd(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Prime-based similarity using GCD
 * Much faster than dot product for related tokens
 */
float crystalline_gcd_similarity(uint32_t token1, uint32_t token2) {
    if (token1 == 0 || token2 == 0) return 0.0f;
    
    // Compute GCD (shared prime factors)
    uint32_t shared = gcd(token1, token2);
    
    // Normalize to [0, 1]
    uint32_t max_val = token1 > token2 ? token1 : token2;
    return (float)shared / (float)max_val;
}

/**
 * Ulam spiral position
 */
typedef struct {
    int x;
    int y;
} UlamPosition;

static UlamPosition compute_ulam_position(uint32_t token_id) {
    UlamPosition pos = {0, 0};
    if (token_id == 0) return pos;
    
    // Ulam spiral: start at origin, spiral outward
    int n = (int)token_id;
    int k = (int)prime_sqrtf((float)n);
    int ring = (k + 1) / 2;
    int offset = n - (2*ring - 1) * (2*ring - 1);
    
    if (offset < 2*ring) {
        pos.x = ring;
        pos.y = -ring + offset;
    } else if (offset < 4*ring) {
        pos.x = ring - (offset - 2*ring);
        pos.y = ring;
    } else if (offset < 6*ring) {
        pos.x = -ring;
        pos.y = ring - (offset - 4*ring);
    } else {
        pos.x = -ring + (offset - 6*ring);
        pos.y = -ring;
    }
    
    return pos;
}

// REMOVED: ulam_distance() - No longer used after switching to pure crystalline loss with clock lattice

/**
 * Pure Crystalline Loss Function (ASI Design - Phase 1)
 * 
 * Uses deterministic GCD-based similarity with LEARNED prime encodings
 * and lattice positions. This is the epitome of ASI design.
 * 
 * Key differences from old implementation:
 * - Uses model->tokens[id].prime_encoding (LEARNED primes, not token_id)
 * - Uses model->tokens[id].lattice_coords (LEARNED positions)
 * - Provides gradient signal for learning
 */
float cllm_compute_crystalline_loss(
    CLLMModel* model,
    uint32_t* input_tokens,
    uint32_t* target_tokens,
    int num_tokens
) {
    if (!model || !model->tokens || !input_tokens || !target_tokens) {
        return 0.0f;
    }
    
    float total_loss = 0.0f;
    int count = 0;
    
    for (int i = 0; i < num_tokens; i++) {
        uint32_t input_id = input_tokens[i];
        uint32_t target_id = target_tokens[i];
        
        // Bounds check
        if (input_id >= model->vocab_size || target_id >= model->vocab_size) {
            continue;
        }
        
        // Get LEARNED prime encodings (not token IDs!)
        uint64_t input_prime = model->tokens[input_id].prime_encoding;
        uint64_t target_prime = model->tokens[target_id].prime_encoding;
        
        if (input_prime == 0 || target_prime == 0) {
            continue; // Skip uninitialized tokens
        }
        
        // 1. SEMANTIC SIMILARITY: GCD of primes (shared roots)
        float gcd_sim = crystalline_gcd_similarity(input_prime, target_prime);
        
        // 2. GEOMETRIC SIMILARITY: Lattice distance in 12D space
        float* input_coords = model->tokens[input_id].lattice_coords;
        float* target_coords = model->tokens[target_id].lattice_coords;
        
        float lattice_dist = 0.0f;
        for (int d = 0; d < 3; d++) {
            float diff = input_coords[d] - target_coords[d];
            lattice_dist += diff * diff;
        }
        lattice_dist = prime_sqrtf(lattice_dist);
        
        float spatial_sim = 1.0f / (1.0f + lattice_dist);
        
        // 3. COMBINED CRYSTALLINE SIMILARITY
        // 70% semantic (GCD), 30% geometric (lattice)
        float combined_sim = 0.7f * gcd_sim + 0.3f * spatial_sim;
        
        // 4. CONVERT TO LOSS: -log(similarity)
        float clamped = combined_sim > 1e-10f ? combined_sim : 1e-10f;
        total_loss += -prime_logf(clamped);
        count++;
    }
    
    return count > 0 ? total_loss / count : 0.0f;
}

// REMOVED: Legacy cllm_compute_loss() function (lines 178-216)
// This function was DEPRECATED and WRONG - it used token IDs for GCD instead of learned prime encodings.
// All code now uses cllm_compute_crystalline_loss() which correctly uses model->tokens[id].prime_encoding.

/**
 * Sort tokens by Ulam spiral position for better cache locality
 */
void crystalline_sort_by_locality(uint32_t* tokens, int num_tokens) {
    if (!tokens || num_tokens <= 1) return;
    
    // Simple bubble sort by Ulam position (good enough for small batches)
    for (int i = 0; i < num_tokens - 1; i++) {
        for (int j = 0; j < num_tokens - i - 1; j++) {
            UlamPosition pos1 = compute_ulam_position(tokens[j]);
            UlamPosition pos2 = compute_ulam_position(tokens[j+1]);
            
            // Sort by Manhattan distance from origin
            int dist1 = abs(pos1.x) + abs(pos1.y);
            int dist2 = abs(pos2.x) + abs(pos2.y);
            
            if (dist1 > dist2) {
                uint32_t temp = tokens[j];
                tokens[j] = tokens[j+1];
                tokens[j+1] = temp;
            }
        }
    }
}

// ============================================================================
// Training Functions
// ============================================================================

// Initialize training state
CLLMTraining* cllm_training_init(CLLMModel* model, CLLMTrainingConfig* config) {
    if (!model || !config) return NULL;
    
    CLLMTraining* training = (CLLMTraining*)calloc(1, sizeof(CLLMTraining));
    if (!training) return NULL;
    
    training->model = model;
    training->config = *config;
    training->precision_bits = model->precision_bits;  // Get precision from model
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
            training->master_weights = bigfixed_array_create(total_params, training->precision_bits);
            if (training->master_weights && model->weights) {
                // Copy current weights to master weights
                // Copy BigFixed weights to master_weights
                    if (model->weights) {
                        bigfixed_array_copy(training->master_weights, model->weights, total_params);
                    }
            }
        }
    }
    
    // Allocate BigFixed gradient buffers
    size_t embed_size = model->vocab_size * model->embedding_dim;
    
    if (embed_size > 0 && embed_size < 100000000) {
        // Allocate BigFixed** arrays
        training->gradients = (BigFixed**)calloc(embed_size, sizeof(BigFixed*));
        training->optimizer_state = (BigFixed**)calloc(embed_size * 2, sizeof(BigFixed*));
        
        // Allocate individual BigFixed elements for gradients
        if (training->gradients) {
            for (size_t i = 0; i < embed_size; i++) {
                training->gradients[i] = big_fixed_create(training->precision_bits);
                if (training->gradients[i]) {
                    big_fixed_from_int(training->gradients[i], 0);  // Initialize to zero
                }
            }
        }
        
        // Allocate individual BigFixed elements for optimizer state (Adam: m and v)
        if (training->optimizer_state) {
            for (size_t i = 0; i < embed_size * 2; i++) {
                training->optimizer_state[i] = big_fixed_create(training->precision_bits);
                if (training->optimizer_state[i]) {
                    big_fixed_from_int(training->optimizer_state[i], 0);  // Initialize to zero
                }
            }
        }
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
                
                // Allocate BigFixed** arrays
                training->attention_grads[i].query_lattice = (BigFixed**)calloc(weight_size, sizeof(BigFixed*));
                training->attention_grads[i].key_lattice = (BigFixed**)calloc(weight_size, sizeof(BigFixed*));
                training->attention_grads[i].value_lattice = (BigFixed**)calloc(weight_size, sizeof(BigFixed*));
                
                // Allocate individual BigFixed elements
                if (training->attention_grads[i].query_lattice) {
                    for (size_t j = 0; j < weight_size; j++) {
                        training->attention_grads[i].query_lattice[j] = big_fixed_create(training->precision_bits);
                        if (training->attention_grads[i].query_lattice[j]) {
                            big_fixed_from_int(training->attention_grads[i].query_lattice[j], 0);
                        }
                    }
                }
                if (training->attention_grads[i].key_lattice) {
                    for (size_t j = 0; j < weight_size; j++) {
                        training->attention_grads[i].key_lattice[j] = big_fixed_create(training->precision_bits);
                        if (training->attention_grads[i].key_lattice[j]) {
                            big_fixed_from_int(training->attention_grads[i].key_lattice[j], 0);
                        }
                    }
                }
                if (training->attention_grads[i].value_lattice) {
                    for (size_t j = 0; j < weight_size; j++) {
                        training->attention_grads[i].value_lattice[j] = big_fixed_create(training->precision_bits);
                        if (training->attention_grads[i].value_lattice[j]) {
                            big_fixed_from_int(training->attention_grads[i].value_lattice[j], 0);
                        }
                    }
                }
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
                
                size_t w1_size = layer->input_dim * layer->hidden_dim;
                size_t w2_size = layer->hidden_dim * layer->output_dim;
                
                // Allocate BigFixed** arrays
                training->ff_grads[i].w1_lattice = (BigFixed**)calloc(w1_size, sizeof(BigFixed*));
                training->ff_grads[i].w2_lattice = (BigFixed**)calloc(w2_size, sizeof(BigFixed*));
                training->ff_grads[i].bias1 = (BigFixed**)calloc(layer->hidden_dim, sizeof(BigFixed*));
                training->ff_grads[i].bias2 = (BigFixed**)calloc(layer->output_dim, sizeof(BigFixed*));
                
                // Allocate individual BigFixed elements
                if (training->ff_grads[i].w1_lattice) {
                    for (size_t j = 0; j < w1_size; j++) {
                        training->ff_grads[i].w1_lattice[j] = big_fixed_create(training->precision_bits);
                        if (training->ff_grads[i].w1_lattice[j]) {
                            big_fixed_from_int(training->ff_grads[i].w1_lattice[j], 0);
                        }
                    }
                }
                if (training->ff_grads[i].w2_lattice) {
                    for (size_t j = 0; j < w2_size; j++) {
                        training->ff_grads[i].w2_lattice[j] = big_fixed_create(training->precision_bits);
                        if (training->ff_grads[i].w2_lattice[j]) {
                            big_fixed_from_int(training->ff_grads[i].w2_lattice[j], 0);
                        }
                    }
                }
                if (training->ff_grads[i].bias1) {
                    for (size_t j = 0; j < layer->hidden_dim; j++) {
                        training->ff_grads[i].bias1[j] = big_fixed_create(training->precision_bits);
                        if (training->ff_grads[i].bias1[j]) {
                            big_fixed_from_int(training->ff_grads[i].bias1[j], 0);
                        }
                    }
                }
                if (training->ff_grads[i].bias2) {
                    for (size_t j = 0; j < layer->output_dim; j++) {
                        training->ff_grads[i].bias2[j] = big_fixed_create(training->precision_bits);
                        if (training->ff_grads[i].bias2[j]) {
                            big_fixed_from_int(training->ff_grads[i].bias2[j], 0);
                        }
                    }
                }
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
                
                // Allocate BigFixed** arrays
                training->ln_grads[i].gamma = (BigFixed**)calloc(layer->dim, sizeof(BigFixed*));
                training->ln_grads[i].beta = (BigFixed**)calloc(layer->dim, sizeof(BigFixed*));
                
                // Allocate individual BigFixed elements
                if (training->ln_grads[i].gamma) {
                    for (size_t j = 0; j < layer->dim; j++) {
                        training->ln_grads[i].gamma[j] = big_fixed_create(training->precision_bits);
                        if (training->ln_grads[i].gamma[j]) {
                            big_fixed_from_int(training->ln_grads[i].gamma[j], 0);
                        }
                    }
                }
                if (training->ln_grads[i].beta) {
                    for (size_t j = 0; j < layer->dim; j++) {
                        training->ln_grads[i].beta[j] = big_fixed_create(training->precision_bits);
                        if (training->ln_grads[i].beta[j]) {
                            big_fixed_from_int(training->ln_grads[i].beta[j], 0);
                        }
                    }
                }
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
    
    // Allocate BigFixed** arrays for forward pass
    training->input_embeddings = (BigFixed**)calloc(seq_size, sizeof(BigFixed*));
    training->final_hidden = (BigFixed**)calloc(seq_size, sizeof(BigFixed*));
    training->logits = (BigFixed**)calloc(logits_size, sizeof(BigFixed*));
    
    // Allocate individual BigFixed elements
    if (training->input_embeddings) {
        for (size_t i = 0; i < seq_size; i++) {
            training->input_embeddings[i] = big_fixed_create(training->precision_bits);
            if (training->input_embeddings[i]) {
                big_fixed_from_int(training->input_embeddings[i], 0);
            }
        }
    }
    if (training->final_hidden) {
        for (size_t i = 0; i < seq_size; i++) {
            training->final_hidden[i] = big_fixed_create(training->precision_bits);
            if (training->final_hidden[i]) {
                big_fixed_from_int(training->final_hidden[i], 0);
            }
        }
    }
    if (training->logits) {
        for (size_t i = 0; i < logits_size; i++) {
            training->logits[i] = big_fixed_create(training->precision_bits);
            if (training->logits[i]) {
                big_fixed_from_int(training->logits[i], 0);
            }
        }
    }
    
    training->layer_inputs = (BigFixed***)calloc(num_layers, sizeof(BigFixed**));
    training->attention_outputs = (BigFixed***)calloc(num_layers, sizeof(BigFixed**));
    training->ff_outputs = (BigFixed***)calloc(num_layers, sizeof(BigFixed**));
    training->layer_outputs = (BigFixed***)calloc(num_layers, sizeof(BigFixed**));
    training->ff_hidden = (BigFixed***)calloc(num_layers, sizeof(BigFixed**));
    
    if (training->layer_inputs && training->attention_outputs && training->ff_outputs &&
        training->layer_outputs && training->ff_hidden && model->ff_layers) {
        for (uint32_t i = 0; i < num_layers; i++) {
            // Allocate BigFixed** arrays for each layer
            training->layer_inputs[i] = (BigFixed**)calloc(seq_size, sizeof(BigFixed*));
            training->attention_outputs[i] = (BigFixed**)calloc(seq_size, sizeof(BigFixed*));
            training->ff_outputs[i] = (BigFixed**)calloc(seq_size, sizeof(BigFixed*));
            training->layer_outputs[i] = (BigFixed**)calloc(seq_size, sizeof(BigFixed*));
            training->ff_hidden[i] = (BigFixed**)calloc(seq_size * 4, sizeof(BigFixed*)); // 4x for hidden dim
            
            // Allocate individual BigFixed elements
            if (training->layer_inputs[i]) {
                for (size_t j = 0; j < seq_size; j++) {
                    training->layer_inputs[i][j] = big_fixed_create(training->precision_bits);
                    if (training->layer_inputs[i][j]) {
                        big_fixed_from_int(training->layer_inputs[i][j], 0);
                    }
                }
            }
            if (training->attention_outputs[i]) {
                for (size_t j = 0; j < seq_size; j++) {
                    training->attention_outputs[i][j] = big_fixed_create(training->precision_bits);
                    if (training->attention_outputs[i][j]) {
                        big_fixed_from_int(training->attention_outputs[i][j], 0);
                    }
                }
            }
            if (training->ff_outputs[i]) {
                for (size_t j = 0; j < seq_size; j++) {
                    training->ff_outputs[i][j] = big_fixed_create(training->precision_bits);
                    if (training->ff_outputs[i][j]) {
                        big_fixed_from_int(training->ff_outputs[i][j], 0);
                    }
                }
            }
            if (training->layer_outputs[i]) {
                for (size_t j = 0; j < seq_size; j++) {
                    training->layer_outputs[i][j] = big_fixed_create(training->precision_bits);
                    if (training->layer_outputs[i][j]) {
                        big_fixed_from_int(training->layer_outputs[i][j], 0);
                    }
                }
            }
            if (training->ff_hidden[i]) {
                for (size_t j = 0; j < seq_size * 4; j++) {
                    training->ff_hidden[i][j] = big_fixed_create(training->precision_bits);
                    if (training->ff_hidden[i][j]) {
                        big_fixed_from_int(training->ff_hidden[i][j], 0);
                    }
                }
            }
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
    int precision = training->precision_bits;
    
    // Update crystalline embeddings using BigFixed operations
    if (model->crystalline_embeddings && training->gradients) {
        // TODO: Implement proper lattice-based embedding optimization
        // For now, just clear gradients
        uint32_t embedding_dim = model->embedding_dim;
        uint32_t vocab_size = model->vocab_size;
        size_t embed_params = vocab_size * embedding_dim;
        
        for (size_t i = 0; i < embed_params; i++) {
            if (training->gradients[i]) {
                big_fixed_from_int(training->gradients[i], 0);
            }
        }
    }
    
    // Update layer weights using sgd_step_bigfixed()
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Update attention weights
        if (training->attention_grads && model->attention_layers) {
            uint64_t attn_size = model->embedding_dim * model->embedding_dim;
            
            if (training->attention_grads[layer].query_lattice && model->attention_layers[layer].query_lattice) {
                sgd_step_bigfixed(
                    model->attention_layers[layer].query_lattice,
                    training->attention_grads[layer].query_lattice,
                    attn_size,
                    lr * gradient_scale,
                    precision
                );
            }
            
            if (training->attention_grads[layer].key_lattice && model->attention_layers[layer].key_lattice) {
                sgd_step_bigfixed(
                    model->attention_layers[layer].key_lattice,
                    training->attention_grads[layer].key_lattice,
                    attn_size,
                    lr * gradient_scale,
                    precision
                );
            }
            
            if (training->attention_grads[layer].value_lattice && model->attention_layers[layer].value_lattice) {
                sgd_step_bigfixed(
                    model->attention_layers[layer].value_lattice,
                    training->attention_grads[layer].value_lattice,
                    attn_size,
                    lr * gradient_scale,
                    precision
                );
            }
        }
        
        // Update feedforward weights
        if (training->ff_grads && model->ff_layers) {
            FeedForwardLayer* ff = &model->ff_layers[layer];
            uint32_t hidden_dim = ff->hidden_dim;
            uint32_t input_dim = ff->input_dim;
            uint32_t output_dim = ff->output_dim;
            
            if (training->ff_grads[layer].w1_lattice && ff->w1_lattice) {
                sgd_step_bigfixed(
                    ff->w1_lattice,
                    training->ff_grads[layer].w1_lattice,
                    input_dim * hidden_dim,
                    lr * gradient_scale,
                    precision
                );
            }
            
            if (training->ff_grads[layer].w2_lattice && ff->w2_lattice) {
                sgd_step_bigfixed(
                    ff->w2_lattice,
                    training->ff_grads[layer].w2_lattice,
                    hidden_dim * output_dim,
                    lr * gradient_scale,
                    precision
                );
            }
            
            if (training->ff_grads[layer].bias1 && ff->bias1) {
                sgd_step_bigfixed(
                    ff->bias1,
                    training->ff_grads[layer].bias1,
                    hidden_dim,
                    lr * gradient_scale,
                    precision
                );
            }
            
            if (training->ff_grads[layer].bias2 && ff->bias2) {
                sgd_step_bigfixed(
                    ff->bias2,
                    training->ff_grads[layer].bias2,
                    output_dim,
                    lr * gradient_scale,
                    precision
                );
            }
        }
        
        // Update layer norm parameters
        if (training->ln_grads && model->layer_norms) {
            if (training->ln_grads[layer].gamma && model->layer_norms[layer].gamma) {
                sgd_step_bigfixed(
                    model->layer_norms[layer].gamma,
                    training->ln_grads[layer].gamma,
                    model->embedding_dim,
                    lr,
                    precision
                );
            }
            
            if (training->ln_grads[layer].beta && model->layer_norms[layer].beta) {
                sgd_step_bigfixed(
                    model->layer_norms[layer].beta,
                    training->ln_grads[layer].beta,
                    model->embedding_dim,
                    lr,
                    precision
                );
            }
        }
    }
}

/**
 * Zero all gradients - BIGFIXED IMPLEMENTATION
 */
void cllm_zero_all_gradients(CLLMTraining* training) {
    if (!training) return;
    
    CLLMModel* model = training->model;
    if (!model) return;
    
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Zero embedding gradients
    if (training->gradients) {
        for (uint32_t i = 0; i < vocab_size * embed_dim; i++) {
            if (training->gradients[i]) {
                big_fixed_from_int(training->gradients[i], 0);
            }
        }
    }
    
    // Zero layer-specific gradients
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Zero attention gradients
        if (training->attention_grads[layer].query_lattice) {
            for (uint32_t i = 0; i < embed_dim * embed_dim; i++) {
                if (training->attention_grads[layer].query_lattice[i]) {
                    big_fixed_from_int(training->attention_grads[layer].query_lattice[i], 0);
                }
            }
        }
        if (training->attention_grads[layer].key_lattice) {
            for (uint32_t i = 0; i < embed_dim * embed_dim; i++) {
                if (training->attention_grads[layer].key_lattice[i]) {
                    big_fixed_from_int(training->attention_grads[layer].key_lattice[i], 0);
                }
            }
        }
        if (training->attention_grads[layer].value_lattice) {
            for (uint32_t i = 0; i < embed_dim * embed_dim; i++) {
                if (training->attention_grads[layer].value_lattice[i]) {
                    big_fixed_from_int(training->attention_grads[layer].value_lattice[i], 0);
                }
            }
        }
        
        // Zero feedforward gradients
        FeedForwardLayer* ff = &model->ff_layers[layer];
        if (training->ff_grads[layer].w1_lattice) {
            for (uint32_t i = 0; i < embed_dim * ff->hidden_dim; i++) {
                if (training->ff_grads[layer].w1_lattice[i]) {
                    big_fixed_from_int(training->ff_grads[layer].w1_lattice[i], 0);
                }
            }
        }
        if (training->ff_grads[layer].w2_lattice) {
            for (uint32_t i = 0; i < ff->hidden_dim * embed_dim; i++) {
                if (training->ff_grads[layer].w2_lattice[i]) {
                    big_fixed_from_int(training->ff_grads[layer].w2_lattice[i], 0);
                }
            }
        }
        if (training->ff_grads[layer].bias1) {
            for (uint32_t i = 0; i < ff->hidden_dim; i++) {
                if (training->ff_grads[layer].bias1[i]) {
                    big_fixed_from_int(training->ff_grads[layer].bias1[i], 0);
                }
            }
        }
        if (training->ff_grads[layer].bias2) {
            for (uint32_t i = 0; i < embed_dim; i++) {
                if (training->ff_grads[layer].bias2[i]) {
                    big_fixed_from_int(training->ff_grads[layer].bias2[i], 0);
                }
            }
        }
        
        // Zero layer norm gradients
        if (training->ln_grads[layer].gamma) {
            for (uint32_t i = 0; i < embed_dim; i++) {
                if (training->ln_grads[layer].gamma[i]) {
                    big_fixed_from_int(training->ln_grads[layer].gamma[i], 0);
                }
            }
        }
        if (training->ln_grads[layer].beta) {
            for (uint32_t i = 0; i < embed_dim; i++) {
                if (training->ln_grads[layer].beta[i]) {
                    big_fixed_from_int(training->ln_grads[layer].beta[i], 0);
                }
            }
        }
    }
}

/**
 * Training-specific attention forward with BigFixed precision
 * Computes multi-head attention using BigFixed operations
 */
static void cllm_attention_forward_training(
    CLLMTraining* training,
    int layer,
    AttentionLayer* attn_layer,
    BigFixed** input,
    BigFixed** output,
    uint32_t* token_ids,
    int seq_len
) {
    if (!training || !attn_layer || !input || !output || seq_len <= 0) return;
    
    uint32_t num_heads = attn_layer->num_heads;
    uint32_t head_dim = attn_layer->head_dim;
    uint32_t embed_dim = num_heads * head_dim;
    int precision = training->precision_bits;
    
    // Allocate Q, K, V matrices [seq_len x embed_dim]
    BigFixed** queries = (BigFixed**)calloc(seq_len * embed_dim, sizeof(BigFixed*));
    BigFixed** keys = (BigFixed**)calloc(seq_len * embed_dim, sizeof(BigFixed*));
    BigFixed** values = (BigFixed**)calloc(seq_len * embed_dim, sizeof(BigFixed*));
    
    if (!queries || !keys || !values) {
        free(queries);
        free(keys);
        free(values);
        return;
    }
    
    // Initialize BigFixed elements
    for (uint32_t i = 0; i < seq_len * embed_dim; i++) {
        queries[i] = big_fixed_create(precision);
        keys[i] = big_fixed_create(precision);
        values[i] = big_fixed_create(precision);
        big_fixed_from_int(queries[i], 0);
        big_fixed_from_int(keys[i], 0);
        big_fixed_from_int(values[i], 0);
    }
    
    // Compute Q, K, V for each position
    BigFixed* temp = big_fixed_create(precision);
    for (uint32_t pos = 0; pos < seq_len; pos++) {
        BigFixed** input_pos = &input[pos * embed_dim];
        
        // Compute Q = input * W_q
        for (uint32_t d = 0; d < embed_dim; d++) {
            for (uint32_t i = 0; i < embed_dim; i++) {
                big_fixed_mul(temp, input_pos[i], attn_layer->query_lattice[i * embed_dim + d]);
                big_fixed_add(queries[pos * embed_dim + d], queries[pos * embed_dim + d], temp);
            }
        }
        
        // Compute K = input * W_k
        for (uint32_t d = 0; d < embed_dim; d++) {
            for (uint32_t i = 0; i < embed_dim; i++) {
                big_fixed_mul(temp, input_pos[i], attn_layer->key_lattice[i * embed_dim + d]);
                big_fixed_add(keys[pos * embed_dim + d], keys[pos * embed_dim + d], temp);
            }
        }
        
        // Compute V = input * W_v
        for (uint32_t d = 0; d < embed_dim; d++) {
            for (uint32_t i = 0; i < embed_dim; i++) {
                big_fixed_mul(temp, input_pos[i], attn_layer->value_lattice[i * embed_dim + d]);
                big_fixed_add(values[pos * embed_dim + d], values[pos * embed_dim + d], temp);
            }
        }
    }
    big_fixed_free(temp);
    
    // Compute attention scores [seq_len x seq_len]
    BigFixed** scores = (BigFixed**)calloc(seq_len * seq_len, sizeof(BigFixed*));
    for (uint32_t i = 0; i < seq_len * seq_len; i++) {
        scores[i] = big_fixed_create(precision);
        big_fixed_from_int(scores[i], 0);
    }
    
    // scores[i,j] = dot(Q[i], K[j]) / sqrt(head_dim)
    BigFixed* scale = big_fixed_create(precision);
    double scale_val = 1.0 / prime_sqrtf((double)head_dim);
    big_fixed_from_double(scale, scale_val);
    
    BigFixed* dot_prod = big_fixed_create(precision);
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            big_fixed_from_int(dot_prod, 0);
            
            // Dot product of Q[i] and K[j]
            for (uint32_t d = 0; d < embed_dim; d++) {
                BigFixed* prod = big_fixed_create(precision);
                big_fixed_mul(prod, queries[i * embed_dim + d], keys[j * embed_dim + d]);
                big_fixed_add(dot_prod, dot_prod, prod);
                big_fixed_free(prod);
            }
            
            // Scale by 1/sqrt(head_dim)
            big_fixed_mul(scores[i * seq_len + j], dot_prod, scale);
        }
    }
    big_fixed_free(dot_prod);
    big_fixed_free(scale);
    
    // Apply softmax to each row (convert to float for softmax, then back to BigFixed)
    for (uint32_t i = 0; i < seq_len; i++) {
        // Convert row to float array
        float* row = (float*)malloc(seq_len * sizeof(float));
        for (uint32_t j = 0; j < seq_len; j++) {
            row[j] = (float)big_fixed_to_double(scores[i * seq_len + j]);
        }
        
        // Apply softmax using crystalline math
        float max_val = row[0];
        for (uint32_t j = 1; j < seq_len; j++) {
            if (row[j] > max_val) max_val = row[j];
        }
        
        double sum = 0.0;
        for (uint32_t j = 0; j < seq_len; j++) {
            row[j] = prime_expf(row[j] - max_val);
            sum += row[j];
        }
        
        if (sum > 1e-10) {
            for (uint32_t j = 0; j < seq_len; j++) {
                row[j] /= (float)sum;
            }
        }
        
        // Convert back to BigFixed
        for (uint32_t j = 0; j < seq_len; j++) {
            big_fixed_from_double(scores[i * seq_len + j], (double)row[j]);
        }
        
        free(row);
    }
    
    // Compute output = scores * V
    BigFixed* weighted_sum = big_fixed_create(precision);
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t d = 0; d < embed_dim; d++) {
            big_fixed_from_int(weighted_sum, 0);
            
            for (uint32_t j = 0; j < seq_len; j++) {
                BigFixed* weighted_val = big_fixed_create(precision);
                big_fixed_mul(weighted_val, scores[i * seq_len + j], values[j * embed_dim + d]);
                big_fixed_add(weighted_sum, weighted_sum, weighted_val);
                big_fixed_free(weighted_val);
            }
            
            if (!output[i * embed_dim + d]) {
                output[i * embed_dim + d] = big_fixed_create(precision);
            }
            big_fixed_assign(output[i * embed_dim + d], weighted_sum);
        }
    }
    big_fixed_free(weighted_sum);
    
    // Cleanup
    for (uint32_t i = 0; i < seq_len * embed_dim; i++) {
        big_fixed_free(queries[i]);
        big_fixed_free(keys[i]);
        big_fixed_free(values[i]);
    }
    for (uint32_t i = 0; i < seq_len * seq_len; i++) {
        big_fixed_free(scores[i]);
    }
    free(queries);
    free(keys);
    free(values);
    free(scores);
}

/* OLD IMPLEMENTATION - DISABLED - BROKEN BIGFIXED/FLOAT MIX
static void cllm_attention_forward_training_OLD(
    CLLMTraining* training,
    int layer,
    AttentionLayer* attn_layer,
    float* input,
    float* output,
    uint32_t* token_ids,
    int seq_len
) {
    if (!training || !attn_layer || !input || !output || layer < 0 || seq_len <= 0) return;
    if (layer >= (int)training->model->num_layers) return;
    
    // Use hybrid attention (angular when token IDs available, dot product otherwise)
    cllm_attention_forward_hybrid(training->model, attn_layer, input, output, 
                                  token_ids, NULL, NULL, seq_len);
    
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
*/

/**
 * Softmax backward pass
 * Computes gradient w.r.t. softmax input given gradient w.r.t. softmax output
 * 
 * For y = softmax(x):
 * grad_x[i] = y[i] * (grad_y[i] - sum_j(y[j] * grad_y[j]))
 */
/* DISABLED - BIGFIXED/FLOAT MIX
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
*/


/**
 * Full attention backward pass with proper gradient computation
 * Computes gradients through the complete attention mechanism including softmax
 * 
 * This replaces the simplified outer product approximation with the full
 * gradient computation through scaled dot-product attention.
 */
/* DISABLED - BIGFIXED/FLOAT MIX
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
*/


// Train for one epoch
// Forward declarations
float cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens);
void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens, float* gradient_buffer);

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
                    sum_attn += prime_fabsf((float)big_fixed_to_double(model->attention_layers[0].query_lattice[i]));
                }
            }
            
            if (model->ff_layers && model->ff_layers[0].w1_lattice) {
                for (int i = 0; i < count; i++) {
                    sum_ff += prime_fabsf((float)big_fixed_to_double(model->ff_layers[0].w1_lattice[i]));
                }
            }
            
            printf("  Weight magnitudes: embed=%.6f, attn=%.6f, ff=%.6f\n",
                   sum_embed/count, sum_attn/count, sum_ff/count);
        }
        
        
        // Forward pass with activation storage
        cllm_forward_training(training, input_tokens);
        
        // Compute loss using PURE CRYSTALLINE LOSS (GCD-based with learned prime encodings)
        float loss = cllm_compute_crystalline_loss(training->model, input_tokens, target_tokens, 
                                                   training->config.batch_size * training->config.sequence_length);
        epoch_loss += loss;
        num_batches++;
        
        // Backward pass with cross-entropy gradients
        cllm_backward_training(training, target_tokens, NULL);
        
        // DIAGNOSTIC: Check gradient magnitudes
        if (num_batches == 1 || num_batches % 5 == 0) {
            CLLMModel* model = training->model;
            float max_embed_grad = 0.0f, sum_embed_grad = 0.0f;
            int nonzero_embed = 0;
            size_t embed_size = model->vocab_size * model->embedding_dim;
            
            for (size_t i = 0; i < embed_size && i < 10000; i++) {
                float g = prime_fabsf((float)big_fixed_to_double(training->gradients[i]));
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
                    float g = prime_fabsf((float)big_fixed_to_double(training->attention_grads[0].query_lattice[i]));
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
        
        
        // OBJECTIVE 18: Apply cymatic resonance to gradients
        // Modulates gradients with cymatic frequencies (432 Hz, 528 Hz, etc.)
        // for smoother convergence and better final loss
        cllm_apply_cymatic_resonance(training->model, training->gradients, training->current_step);
        
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







/* DISABLED - BROKEN BIGFIXED IMPLEMENTATION
void cllm_adam_step_bigfixed(
    CLLMTraining* training,
    float learning_rate_float
) {
    if (!training) return;
    
    CLLMModel* model = training->model;
    int precision = training->precision_bits;
    size_t embed_size = model->vocab_size * model->embedding_dim;
    
    // Convert learning rate to BigFixed
    BigFixed learning_rate;
    BigFixed learning_rate = *big_fixed_create(precision);
    big_fixed_from_double(&learning_rate, learning_rate_float);
    
    // Adam hyperparameters
    BigFixed beta1, beta2, epsilon;
    BigFixed beta1 = *big_fixed_create(precision);
    BigFixed beta2 = *big_fixed_create(precision);
    BigFixed epsilon = *big_fixed_create(precision);
    
    big_fixed_from_double(&beta1, 0.9);
    big_fixed_from_double(&beta2, 0.999);
    big_fixed_from_double(&epsilon, 1e-8);
    
    BigFixed one;
    BigFixed one = *big_fixed_create(precision);
    big_fixed_from_int(&one, 1);
    
    BigFixed one_minus_beta1, one_minus_beta2;
    BigFixed one_minus_beta1 = *big_fixed_create(precision);
    BigFixed one_minus_beta2 = *big_fixed_create(precision);
    big_fixed_sub(&one_minus_beta1, &one, &beta1);
    big_fixed_sub(&one_minus_beta2, &one, &beta2);
    
    // Update embeddings (simplified - would need proper gradient accumulation)
    for (size_t i = 0; i < embed_size && i < 1000; i++) {  // Limit for now
        if (!training->gradients[i]) continue;
        
        BigFixed* m = training->optimizer_state[i * 2];      // First moment
        BigFixed* v = training->optimizer_state[i * 2 + 1];  // Second moment
        BigFixed* g = training->gradients[i];                // Gradient
        
        // m = beta1 * m + (1 - beta1) * g
        BigFixed temp1, temp2;
        BigFixed temp1 = *big_fixed_create(precision);
        BigFixed temp2 = *big_fixed_create(precision);
        
        big_fixed_mul(&temp1, &beta1, m);
        big_fixed_mul(&temp2, &one_minus_beta1, g);
        big_fixed_add(m, &temp1, &temp2);
        
        // v = beta2 * v + (1 - beta2) * g^2
        BigFixed g_squared;
        BigFixed g_squared = *big_fixed_create(precision);
        big_fixed_mul(&g_squared, g, g);
        
        big_fixed_mul(&temp1, &beta2, v);
        big_fixed_mul(&temp2, &one_minus_beta2, &g_squared);
        big_fixed_add(v, &temp1, &temp2);
        
        // weight = weight - lr * m / (sqrt(v) + epsilon)
        BigFixed sqrt_v, denom, update;
        BigFixed sqrt_v = *big_fixed_create(precision);
        BigFixed denom = *big_fixed_create(precision);
        BigFixed update = *big_fixed_create(precision);
        
        bigfixed_sqrt(&sqrt_v, v, precision);
        big_fixed_add(&denom, &sqrt_v, &epsilon);
        big_fixed_div(&temp1, m, &denom);
        big_fixed_mul(&update, &learning_rate, &temp1);
        
        // Update weight (would need to map to actual model weights)
        // For now, just update master_weights
        if (training->master_weights && training->master_weights[i]) {
            big_fixed_sub(training->master_weights[i], training->master_weights[i], &update);
        }
        
        // Cleanup
        big_fixed_free(&temp1);
        big_fixed_free(&temp2);
        big_fixed_free(&g_squared);
        big_fixed_free(&sqrt_v);
        big_fixed_free(&denom);
        big_fixed_free(&update);
        
        // Zero out gradient for next iteration
        big_fixed_from_int(g, 0);
    }
    
    // Update layer weights (attention, FF, layer norm)
    // This would be done similarly for each layer's gradients
    
    // Cleanup
    big_fixed_free(&learning_rate);
    big_fixed_free(&beta1);
    big_fixed_free(&beta2);
    big_fixed_free(&epsilon);
    big_fixed_free(&one);
    big_fixed_free(&one_minus_beta1);
    big_fixed_free(&one_minus_beta2);
}
*/


/* DISABLED - BROKEN BIGFIXED: cllm_layernorm_backward_bigfixed
void cllm_layernorm_backward_bigfixed(
    CLLMTraining* training,
    uint32_t layer_idx,
    BigFixed** grad_output,
    int batch_size,
    int seq_len,
    uint32_t embed_dim,
    int precision
) {
    // Simplified layer norm backward pass
    // In full implementation, this would compute gradients for gamma and beta
    // For now, just pass through the gradient
    // (Layer norm backward is complex and would require storing forward pass statistics)
}
*/

/* DISABLED - BROKEN BIGFIXED: cllm_feedforward_backward_bigfixed
void cllm_feedforward_backward_bigfixed(
    CLLMTraining* training,
    uint32_t layer_idx,
    BigFixed** grad_output,
    int batch_size,
    int seq_len,
    uint32_t embed_dim,
    int precision
) {
    CLLMModel* model = training->model;
    FeedForwardLayer* ff = &model->ff_layers[layer_idx];
    
    // Backprop through feed-forward network
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            BigFixed** grad_ff_out = &grad_output[idx * embed_dim];
            BigFixed** ff_hidden = &training->ff_hidden[layer_idx][idx * ff->hidden_dim];
            BigFixed** attn_out = &training->attention_outputs[layer_idx][idx * embed_dim];
            
            // Gradient w.r.t. W2 and bias2
            for (uint32_t o = 0; o < embed_dim; o++) {
                // grad_bias2[o] += grad_ff_out[o]
                big_fixed_add(
                    training->ff_grads[layer_idx].bias2[o],
                    training->ff_grads[layer_idx].bias2[o],
                    grad_ff_out[o]
                );
                
                // grad_W2[:, o] += ff_hidden * grad_ff_out[o]
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    BigFixed prod;
                    BigFixed prod = *big_fixed_create(precision);
                    big_fixed_mul(&prod, ff_hidden[h], grad_ff_out[o]);
                    big_fixed_add(
                        training->ff_grads[layer_idx].w2_lattice[h * embed_dim + o],
                        training->ff_grads[layer_idx].w2_lattice[h * embed_dim + o],
                        &prod
                    );
                    big_fixed_free(&prod);
                }
            }
            
            // Gradient w.r.t. hidden layer
            BigFixed** grad_hidden = (BigFixed**)calloc(ff->hidden_dim, sizeof(BigFixed*));
            for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                grad_hidden[h] = big_fixed_create(precision);
                big_fixed_from_int(grad_hidden[h], 0);
                
                for (uint32_t o = 0; o < embed_dim; o++) {
                    BigFixed prod;
                    BigFixed prod = *big_fixed_create(precision);
                    big_fixed_mul(&prod, grad_ff_out[o], ff->w2_lattice[h * embed_dim + o]);
                    big_fixed_add(grad_hidden[h], grad_hidden[h], &prod);
                    big_fixed_free(&prod);
                }
                
                // Gradient of tanh: (1 - tanh^2(x))
                BigFixed tanh_grad;
                BigFixed tanh_grad = *big_fixed_create(precision);
                BigFixed one;
                BigFixed one = *big_fixed_create(precision);
                big_fixed_from_int(&one, 1);
                
                BigFixed tanh_sq;
                BigFixed tanh_sq = *big_fixed_create(precision);
                big_fixed_mul(&tanh_sq, ff_hidden[h], ff_hidden[h]);
                big_fixed_sub(&tanh_grad, &one, &tanh_sq);
                
                big_fixed_mul(grad_hidden[h], grad_hidden[h], &tanh_grad);
                
                big_fixed_free(&tanh_grad);
                big_fixed_free(&one);
                big_fixed_free(&tanh_sq);
            }
            
            // Gradient w.r.t. W1 and bias1
            for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                // grad_bias1[h] += grad_hidden[h]
                big_fixed_add(
                    training->ff_grads[layer_idx].bias1[h],
                    training->ff_grads[layer_idx].bias1[h],
                    grad_hidden[h]
                );
                
                // grad_W1[:, h] += attn_out * grad_hidden[h]
                for (uint32_t i = 0; i < embed_dim; i++) {
                    BigFixed prod;
                    BigFixed prod = *big_fixed_create(precision);
                    big_fixed_mul(&prod, attn_out[i], grad_hidden[h]);
                    big_fixed_add(
                        training->ff_grads[layer_idx].w1_lattice[i * ff->hidden_dim + h],
                        training->ff_grads[layer_idx].w1_lattice[i * ff->hidden_dim + h],
                        &prod
                    );
                    big_fixed_free(&prod);
                }
            }
            
            // Gradient w.r.t. attention output (input to FF)
            for (uint32_t i = 0; i < embed_dim; i++) {
                BigFixed sum;
                BigFixed sum = *big_fixed_create(precision);
                big_fixed_from_int(&sum, 0);
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    BigFixed prod;
                    BigFixed prod = *big_fixed_create(precision);
                    big_fixed_mul(&prod, grad_hidden[h], ff->w1_lattice[i * ff->hidden_dim + h]);
                    big_fixed_add(&sum, &sum, &prod);
                    big_fixed_free(&prod);
                }
                
                // Add to grad_output (for residual connection)
                big_fixed_add(grad_ff_out[i], grad_ff_out[i], &sum);
                big_fixed_free(&sum);
            }
            
            // Cleanup
            for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                big_fixed_free(grad_hidden[h]);
            }
            free(grad_hidden);
        }
    }
}
*/

/* DISABLED - BROKEN BIGFIXED: cllm_attention_backward_bigfixed
void cllm_attention_backward_bigfixed(
    CLLMTraining* training,
    uint32_t layer_idx,
    BigFixed** grad_output,
    int batch_size,
    int seq_len,
    uint32_t embed_dim,
    int precision
) {
    // Simplified attention backward pass
    // Full implementation would backprop through attention mechanism
    // For now, accumulate gradients for query, key, value weights
    // (Attention backward is very complex and would require storing attention weights)
    
    CLLMModel* model = training->model;
    AttentionLayer* attn = &model->attention_layers[layer_idx];
    
    // For now, just pass through gradient to layer input
    // In full implementation, this would compute gradients for Q, K, V weights
}
*/




/* DISABLED - BROKEN BIGFIXED: cllm_train_step_bigfixed
float cllm_train_step_bigfixed(
    CLLMTraining* training,
    uint32_t* input_tokens,
    uint32_t* target_tokens
) {
    if (!training || !input_tokens || !target_tokens) return 0.0f;
    
    CLLMModel* model = training->model;
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    uint32_t vocab_size = model->vocab_size;
    int precision = training->precision_bits;
    
    // Forward pass with BigFixed
    cllm_forward_training_bigfixed(training, input_tokens);
    
    // Compute loss with BigFixed
    float loss = cllm_compute_loss_bigfixed(
        training,
        training->logits,
        target_tokens,
        batch_size,
        seq_len,
        vocab_size,
        precision
    );
    
    // Backward pass with BigFixed
    cllm_backward_training_bigfixed(training, target_tokens);
    
    // Optimizer step with BigFixed
    cllm_adam_step_bigfixed(training, training->config.learning_rate);
    
    return loss;
}
*/




/* DISABLED - BROKEN BIGFIXED: cllm_attention_forward_bigfixed
void cllm_attention_forward_bigfixed(
    CLLMTraining* training,
    uint32_t layer_idx,
    AttentionLayer* attn_layer,
    BigFixed** input,
    BigFixed** output,
    int seq_len,
    uint32_t embed_dim,
    int precision
) {
    if (!training || !attn_layer || !input || !output) return;
    
    uint32_t num_heads = attn_layer->num_heads;
    uint32_t head_dim = attn_layer->head_dim;
    
    // Allocate temporary buffers for Q, K, V
    BigFixed** queries = (BigFixed**)calloc(seq_len * embed_dim, sizeof(BigFixed*));
    BigFixed** keys = (BigFixed**)calloc(seq_len * embed_dim, sizeof(BigFixed*));
    BigFixed** values = (BigFixed**)calloc(seq_len * embed_dim, sizeof(BigFixed*));
    
    for (int i = 0; i < seq_len * embed_dim; i++) {
        queries[i] = big_fixed_create(precision);
        keys[i] = big_fixed_create(precision);
        values[i] = big_fixed_create(precision);
    }
    
    // Compute Q, K, V projections
    for (int s = 0; s < seq_len; s++) {
        BigFixed** input_pos = &input[s * embed_dim];
        BigFixed** q_pos = &queries[s * embed_dim];
        BigFixed** k_pos = &keys[s * embed_dim];
        BigFixed** v_pos = &values[s * embed_dim];
        
        // Q = input * W_q
        for (uint32_t d = 0; d < embed_dim; d++) {
            BigFixed sum;
            BigFixed sum = *big_fixed_create(precision);
            big_fixed_from_int(&sum, 0);
            
            for (uint32_t i = 0; i < embed_dim; i++) {
                BigFixed prod;
                BigFixed prod = *big_fixed_create(precision);
                big_fixed_mul(&prod, input_pos[i], attn_layer->query_lattice[i * embed_dim + d]);
                big_fixed_add(&sum, &sum, &prod);
                big_fixed_free(&prod);
            }
            
            big_fixed_assign(q_pos[d], &sum);
            big_fixed_free(&sum);
        }
        
        // K = input * W_k
        for (uint32_t d = 0; d < embed_dim; d++) {
            BigFixed sum;
            BigFixed sum = *big_fixed_create(precision);
            big_fixed_from_int(&sum, 0);
            
            for (uint32_t i = 0; i < embed_dim; i++) {
                BigFixed prod;
                BigFixed prod = *big_fixed_create(precision);
                big_fixed_mul(&prod, input_pos[i], attn_layer->key_lattice[i * embed_dim + d]);
                big_fixed_add(&sum, &sum, &prod);
                big_fixed_free(&prod);
            }
            
            big_fixed_assign(k_pos[d], &sum);
            big_fixed_free(&sum);
        }
        
        // V = input * W_v
        for (uint32_t d = 0; d < embed_dim; d++) {
            BigFixed sum;
            BigFixed sum = *big_fixed_create(precision);
            big_fixed_from_int(&sum, 0);
            
            for (uint32_t i = 0; i < embed_dim; i++) {
                BigFixed prod;
                BigFixed prod = *big_fixed_create(precision);
                big_fixed_mul(&prod, input_pos[i], attn_layer->value_lattice[i * embed_dim + d]);
                big_fixed_add(&sum, &sum, &prod);
                big_fixed_free(&prod);
            }
            
            big_fixed_assign(v_pos[d], &sum);
            big_fixed_free(&sum);
        }
    }
    
    // Compute attention scores and apply to values
    // For each head
    for (uint32_t h = 0; h < num_heads; h++) {
        uint32_t head_offset = h * head_dim;
        
        // Allocate attention scores matrix [seq_len x seq_len]
        BigFixed** scores = (BigFixed**)calloc(seq_len * seq_len, sizeof(BigFixed*));
        for (int i = 0; i < seq_len * seq_len; i++) {
            scores[i] = big_fixed_create(precision);
        }
        
        // Compute scores: Q * K^T / sqrt(head_dim)
        BigFixed sqrt_head_dim;
        BigFixed sqrt_head_dim = *big_fixed_create(precision);
        big_fixed_from_int(&sqrt_head_dim, head_dim);
        bigfixed_sqrt(&sqrt_head_dim, &sqrt_head_dim, precision);
        
        for (int i = 0; i < seq_len; i++) {
            for (int j = 0; j < seq_len; j++) {
                BigFixed score;
                BigFixed score = *big_fixed_create(precision);
                big_fixed_from_int(&score, 0);
                
                // Dot product of query[i] and key[j] for this head
                for (uint32_t d = 0; d < head_dim; d++) {
                    BigFixed prod;
                    BigFixed prod = *big_fixed_create(precision);
                    big_fixed_mul(&prod, 
                        queries[i * embed_dim + head_offset + d],
                        keys[j * embed_dim + head_offset + d]
                    );
                    big_fixed_add(&score, &score, &prod);
                    big_fixed_free(&prod);
                }
                
                // Scale by sqrt(head_dim)
                big_fixed_div(&score, &score, &sqrt_head_dim);
                big_fixed_assign(scores[i * seq_len + j], &score);
                big_fixed_free(&score);
            }
        }
        
        big_fixed_free(&sqrt_head_dim);
        
        // Apply softmax to each row
        for (int i = 0; i < seq_len; i++) {
            BigFixed** row = &scores[i * seq_len];
            
            // Find max for numerical stability
            BigFixed max_score;
            BigFixed max_score = *big_fixed_create(precision);
            big_fixed_assign(&max_score, row[0]);
            
            for (int j = 1; j < seq_len; j++) {
                if (big_fixed_cmp(row[j], &max_score) > 0) {
                    big_fixed_assign(&max_score, row[j]);
                }
            }
            
            // Compute exp and sum
            BigFixed sum;
            BigFixed sum = *big_fixed_create(precision);
            big_fixed_from_int(&sum, 0);
            
            for (int j = 0; j < seq_len; j++) {
                BigFixed diff, exp_val;
                BigFixed diff = *big_fixed_create(precision);
                BigFixed exp_val = *big_fixed_create(precision);
                
                big_fixed_sub(&diff, row[j], &max_score);
                bigfixed_exp(&exp_val, &diff, precision);
                big_fixed_assign(row[j], &exp_val);
                big_fixed_add(&sum, &sum, &exp_val);
                
                big_fixed_free(&diff);
                big_fixed_free(&exp_val);
            }
            
            // Normalize
            for (int j = 0; j < seq_len; j++) {
                big_fixed_div(row[j], row[j], &sum);
            }
            
            big_fixed_free(&max_score);
            big_fixed_free(&sum);
        }
        
        // Apply attention to values: output = scores * V
        for (int i = 0; i < seq_len; i++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                BigFixed sum;
                BigFixed sum = *big_fixed_create(precision);
                big_fixed_from_int(&sum, 0);
                
                for (int j = 0; j < seq_len; j++) {
                    BigFixed prod;
                    BigFixed prod = *big_fixed_create(precision);
                    big_fixed_mul(&prod,
                        scores[i * seq_len + j],
                        values[j * embed_dim + head_offset + d]
                    );
                    big_fixed_add(&sum, &sum, &prod);
                    big_fixed_free(&prod);
                }
                
                big_fixed_assign(output[i * embed_dim + head_offset + d], &sum);
                big_fixed_free(&sum);
            }
        }
        
        // Free scores
        for (int i = 0; i < seq_len * seq_len; i++) {
            big_fixed_free(scores[i]);
        }
        free(scores);
    }
    
    // Free temporary buffers
    for (int i = 0; i < seq_len * embed_dim; i++) {
        big_fixed_free(queries[i]);
        big_fixed_free(keys[i]);
        big_fixed_free(values[i]);
    }
    free(queries);
    free(keys);
    free(values);
}
*/




float cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens) {
    if (!training || !input_tokens) return 0.0f;
    
    CLLMModel* model = training->model;
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Get embeddings - BIGFIXED IMPLEMENTATION
    // Use CrystallineEmbeddings (BigFixed-based) instead of deprecated float embeddings
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t token_id = input_tokens[idx];
            if (token_id >= vocab_size) continue;
            
            // Copy BigFixed pointers from crystalline embeddings to training buffer
            // training->input_embeddings is BigFixed** [batch*seq*embed_dim]
            // model->crystalline_embeddings->token_positions is BigFixed** [vocab_size][lattice_dim]
            
            if (model->crystalline_embeddings && model->crystalline_embeddings->token_positions) {
                // Use crystalline embeddings (BigFixed-based)
                for (uint32_t d = 0; d < embed_dim; d++) {
                    uint32_t src_idx = token_id * embed_dim + d;
                    uint32_t dst_idx = idx * embed_dim + d;
                    
                    // Copy BigFixed pointer (not the value, the pointer itself)
                    training->input_embeddings[dst_idx] = model->crystalline_embeddings->token_positions[src_idx];
                }
            } else {
                // Fallback: Convert deprecated float embeddings to BigFixed
                // This path should eventually be removed once migration is complete
                for (uint32_t d = 0; d < embed_dim; d++) {
                    uint32_t src_idx = token_id * embed_dim + d;
                    uint32_t dst_idx = idx * embed_dim + d;
                    
                    // Create BigFixed from float value
                    if (!training->input_embeddings[dst_idx]) {
                        training->input_embeddings[dst_idx] = big_fixed_create(training->precision_bits);
                    }
                    big_fixed_from_double(training->input_embeddings[dst_idx], 
                                         (double)model->embeddings.embeddings[src_idx]);
                }
            }
        }
    }
    
    // Process through layers - BIGFIXED IMPLEMENTATION
    // layer_input is BigFixed** (array of BigFixed pointers)
    BigFixed** layer_input = training->input_embeddings;
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        // Copy BigFixed pointers to layer input buffer
        // training->layer_inputs[layer] is BigFixed** [batch*seq*embed_dim]
        for (uint32_t i = 0; i < batch_size * seq_len * embed_dim; i++) {
            training->layer_inputs[layer][i] = layer_input[i];
        }
        
        // Apply proper multi-head attention for each batch
        AttentionLayer* attn_layer = &model->attention_layers[layer];
        for (int b = 0; b < batch_size; b++) {
            int start_idx = b * seq_len;
            // batch_input is BigFixed** pointing to start of this batch
            BigFixed** batch_input = &layer_input[start_idx * embed_dim];
            BigFixed** batch_output = &training->attention_outputs[layer][start_idx * embed_dim];
            uint32_t* batch_tokens = &input_tokens[start_idx];
            
            // Use hybrid attention (angular when token IDs available, dot product otherwise)
            cllm_attention_forward_training(training, layer, attn_layer, 
                                           batch_input, batch_output, batch_tokens, seq_len);
        }
        
        // Process feedforward for each position - BIGFIXED IMPLEMENTATION
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                // All outputs are BigFixed** arrays
                BigFixed** attn_out = &training->attention_outputs[layer][idx * embed_dim];
                BigFixed** ff_out = &training->ff_outputs[layer][idx * embed_dim];
                BigFixed** layer_out = &training->layer_outputs[layer][idx * embed_dim];
                
                // FeedForward - BIGFIXED IMPLEMENTATION
                FeedForwardLayer* ff = &model->ff_layers[layer];
                BigFixed** ff_hidden = &training->ff_hidden[layer][idx * ff->hidden_dim];
                
                // W1 * attn_out + bias1, then tanh
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    // Initialize with bias1
                    if (!ff_hidden[h]) {
                        ff_hidden[h] = big_fixed_create(training->precision_bits);
                    }
                    big_fixed_assign(ff_hidden[h], ff->bias1[h]);
                    
                    // Accumulate: sum += attn_out[i] * w1[i,h]
                    BigFixed* temp = big_fixed_create(training->precision_bits);
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        uint32_t w_idx = i * ff->hidden_dim + h;
                        big_fixed_mul(temp, attn_out[i], ff->w1_lattice[w_idx]);
                        big_fixed_add(ff_hidden[h], ff_hidden[h], temp);
                    }
                    big_fixed_free(temp);
                    
                    // Apply tanh activation
                    double val = big_fixed_to_double(ff_hidden[h]);
                    val = prime_tanhf((float)val);
                    big_fixed_from_double(ff_hidden[h], val);
                }
                
                // W2 * ff_hidden + bias2
                for (uint32_t o = 0; o < embed_dim; o++) {
                    // Initialize with bias2
                    if (!ff_out[o]) {
                        ff_out[o] = big_fixed_create(training->precision_bits);
                    }
                    big_fixed_assign(ff_out[o], ff->bias2[o]);
                    
                    // Accumulate: sum += ff_hidden[h] * w2[h,o]
                    BigFixed* temp = big_fixed_create(training->precision_bits);
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        uint32_t w_idx = h * embed_dim + o;
                        big_fixed_mul(temp, ff_hidden[h], ff->w2_lattice[w_idx]);
                        big_fixed_add(ff_out[o], ff_out[o], temp);
                    }
                    big_fixed_free(temp);
                }
                
                // Residual connection: layer_out = attn_out + ff_out
                for (uint32_t d = 0; d < embed_dim; d++) {
                    if (!layer_out[d]) {
                        layer_out[d] = big_fixed_create(training->precision_bits);
                    }
                    big_fixed_add(layer_out[d], attn_out[d], ff_out[d]);
                }
                
                // LayerNorm - BIGFIXED IMPLEMENTATION
                CLLMLayerNorm* ln = &model->layer_norms[layer];
                
                // Compute mean
                BigFixed* mean = big_fixed_create(training->precision_bits);
                big_fixed_from_int(mean, 0);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    big_fixed_add(mean, mean, layer_out[d]);
                }
                BigFixed* embed_dim_bf = big_fixed_create(training->precision_bits);
                big_fixed_from_int(embed_dim_bf, (int)embed_dim);
                big_fixed_div(mean, mean, embed_dim_bf);
                
                // Compute variance
                BigFixed* var = big_fixed_create(training->precision_bits);
                big_fixed_from_int(var, 0);
                BigFixed* diff = big_fixed_create(training->precision_bits);
                BigFixed* diff_sq = big_fixed_create(training->precision_bits);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    big_fixed_sub(diff, layer_out[d], mean);
                    big_fixed_mul(diff_sq, diff, diff);
                    big_fixed_add(var, var, diff_sq);
                }
                big_fixed_div(var, var, embed_dim_bf);
                
                // Add epsilon and compute std
                BigFixed* epsilon = big_fixed_create(training->precision_bits);
                big_fixed_from_double(epsilon, 1e-5);
                big_fixed_add(var, var, epsilon);
                
                // std = sqrt(var) - use prime_sqrtf via double conversion
                double var_val = big_fixed_to_double(var);
                double std_val = prime_sqrtf((float)var_val);
                BigFixed* std = big_fixed_create(training->precision_bits);
                big_fixed_from_double(std, std_val);
                
                // Normalize: layer_out[d] = gamma[d] * (layer_out[d] - mean) / std + beta[d]
                BigFixed* normalized = big_fixed_create(training->precision_bits);
                BigFixed* scaled = big_fixed_create(training->precision_bits);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    // (layer_out[d] - mean) / std
                    big_fixed_sub(normalized, layer_out[d], mean);
                    big_fixed_div(normalized, normalized, std);
                    
                    // gamma[d] * normalized
                    big_fixed_mul(scaled, ln->gamma[d], normalized);
                    
                    // + beta[d]
                    big_fixed_add(layer_out[d], scaled, ln->beta[d]);
                }
                
                // Cleanup
                big_fixed_free(mean);
                big_fixed_free(var);
                big_fixed_free(diff);
                big_fixed_free(diff_sq);
                big_fixed_free(epsilon);
                big_fixed_free(std);
                big_fixed_free(normalized);
                big_fixed_free(scaled);
                big_fixed_free(embed_dim_bf);
            }
        }
        layer_input = training->layer_outputs[layer];
    }
    
    // Copy final hidden - BIGFIXED IMPLEMENTATION
    // Copy BigFixed pointers from layer_input to final_hidden
    for (uint32_t i = 0; i < batch_size * seq_len * embed_dim; i++) {
        training->final_hidden[i] = layer_input[i];
    }
    
    // Project to vocabulary - BIGFIXED IMPLEMENTATION
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            BigFixed** hidden = &training->final_hidden[idx * embed_dim];
            BigFixed** logits = &training->logits[idx * vocab_size];
            
            // Compute logits: logits[v] = dot(hidden, vocab_embed[v])
            for (uint32_t v = 0; v < vocab_size; v++) {
                // Initialize logit to zero
                if (!logits[v]) {
                    logits[v] = big_fixed_create(training->precision_bits);
                }
                big_fixed_from_int(logits[v], 0);
                
                // Compute dot product with vocabulary embedding
                BigFixed* temp = big_fixed_create(training->precision_bits);
                
                if (model->crystalline_embeddings && model->crystalline_embeddings->token_positions) {
                    // Use crystalline embeddings (BigFixed-based)
                    for (uint32_t d = 0; d < embed_dim; d++) {
                        uint32_t vocab_idx = v * embed_dim + d;
                        big_fixed_mul(temp, hidden[d], model->crystalline_embeddings->token_positions[vocab_idx]);
                        big_fixed_add(logits[v], logits[v], temp);
                    }
                } else {
                    // Fallback: Convert deprecated float embeddings to BigFixed
                    BigFixed* vocab_bf = big_fixed_create(training->precision_bits);
                    for (uint32_t d = 0; d < embed_dim; d++) {
                        uint32_t vocab_idx = v * embed_dim + d;
                        big_fixed_from_double(vocab_bf, (double)model->embeddings.embeddings[vocab_idx]);
                        big_fixed_mul(temp, hidden[d], vocab_bf);
                        big_fixed_add(logits[v], logits[v], temp);
                    }
                    big_fixed_free(vocab_bf);
                }
                
                big_fixed_free(temp);
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
void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens, float* gradient_buffer) {
    if (!training || !target_tokens) return;
    
    // Use provided gradient buffer if given, otherwise use training->gradients
    float* gradients = gradient_buffer ? gradient_buffer : training->gradients;
    if (!gradients) return;
    
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
                float* grad_embed = &gradients[v * embed_dim];
                for (uint32_t d = 0; d < embed_dim; d++) {
                    grad_embed[d] += grad_log[v] * hidden[d];
                }
            }
        }
    }
    
    // Backward through layers
    memcpy(grad_layer, grad_hidden, batch_size * seq_len * embed_dim * sizeof(float));
    
    for (int layer = model->num_layers - 1; layer >= 0; layer--) {
        BigFixed** attn_output = training->attention_outputs[layer];
        BigFixed** ff_hidden = training->ff_hidden[layer];
        FeedForwardLayer* ff = &model->ff_layers[layer];
        CLLMLayerNorm* ln = &model->layer_norms[layer];
        
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                BigFixed** grad = &grad_layer[idx * embed_dim];
                BigFixed** input = &attn_output[idx * embed_dim];
                BigFixed** hidden = &ff_hidden[idx * ff->hidden_dim];
                
                // LayerNorm backward - BIGFIXED IMPLEMENTATION
                // Compute mean
                BigFixed* mean = big_fixed_create(training->precision_bits);
                big_fixed_from_int(mean, 0);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    big_fixed_add(mean, mean, input[d]);
                }
                BigFixed* embed_dim_bf = big_fixed_create(training->precision_bits);
                big_fixed_from_int(embed_dim_bf, (int)embed_dim);
                big_fixed_div(mean, mean, embed_dim_bf);
                
                // Compute variance
                BigFixed* var = big_fixed_create(training->precision_bits);
                big_fixed_from_int(var, 0);
                BigFixed* diff = big_fixed_create(training->precision_bits);
                BigFixed* diff_sq = big_fixed_create(training->precision_bits);
                for (uint32_t d = 0; d < embed_dim; d++) {
                    big_fixed_sub(diff, input[d], mean);
                    big_fixed_mul(diff_sq, diff, diff);
                    big_fixed_add(var, var, diff_sq);
                }
                big_fixed_div(var, var, embed_dim_bf);
                
                // Compute std
                BigFixed* epsilon = big_fixed_create(training->precision_bits);
                big_fixed_from_double(epsilon, 1e-5);
                big_fixed_add(var, var, epsilon);
                double var_val = big_fixed_to_double(var);
                double std_val = prime_sqrtf((float)var_val);
                BigFixed* std = big_fixed_create(training->precision_bits);
                big_fixed_from_double(std, std_val);
                
                // Compute gradients for gamma and beta
                BigFixed* grad_var = big_fixed_create(training->precision_bits);
                BigFixed* grad_mean = big_fixed_create(training->precision_bits);
                big_fixed_from_int(grad_var, 0);
                big_fixed_from_int(grad_mean, 0);
                
                BigFixed* x_norm = big_fixed_create(training->precision_bits);
                BigFixed* temp = big_fixed_create(training->precision_bits);
                BigFixed* temp2 = big_fixed_create(training->precision_bits);
                
                for (uint32_t d = 0; d < embed_dim; d++) {
                    // x_norm = (input[d] - mean) / std
                    big_fixed_sub(x_norm, input[d], mean);
                    big_fixed_div(x_norm, x_norm, std);
                    
                    // Accumulate gamma gradient: gamma_grad += grad[d] * x_norm
                    if (training->ln_grads[layer].gamma) {
                        big_fixed_mul(temp, grad[d], x_norm);
                        big_fixed_add(training->ln_grads[layer].gamma[d], 
                                     training->ln_grads[layer].gamma[d], temp);
                    }
                    
                    // Accumulate beta gradient: beta_grad += grad[d]
                    if (training->ln_grads[layer].beta) {
                        big_fixed_add(training->ln_grads[layer].beta[d], 
                                     training->ln_grads[layer].beta[d], grad[d]);
                    }
                    
                    // grad_x_norm = grad[d] * gamma[d]
                    BigFixed* grad_x_norm = big_fixed_create(training->precision_bits);
                    big_fixed_mul(grad_x_norm, grad[d], ln->gamma[d]);
                    
                    // Accumulate variance gradient
                    // grad_var += grad_x_norm * (input[d] - mean) * -0.5 * std^(-3)
                    big_fixed_sub(temp, input[d], mean);
                    big_fixed_mul(temp, temp, grad_x_norm);
                    double std_pow = prime_powf((float)std_val, -3.0f);
                    big_fixed_from_double(temp2, -0.5 * std_pow);
                    big_fixed_mul(temp, temp, temp2);
                    big_fixed_add(grad_var, grad_var, temp);
                    
                    // Accumulate mean gradient
                    // grad_mean += grad_x_norm * (-1.0 / std)
                    big_fixed_from_double(temp, -1.0 / std_val);
                    big_fixed_mul(temp, grad_x_norm, temp);
                    big_fixed_add(grad_mean, grad_mean, temp);
                    
                    big_fixed_free(grad_x_norm);
                }
                
                // Propagate gradient through normalization
                for (uint32_t d = 0; d < embed_dim; d++) {
                    // grad_x_norm = grad[d] * gamma[d]
                    BigFixed* grad_x_norm = big_fixed_create(training->precision_bits);
                    big_fixed_mul(grad_x_norm, grad[d], ln->gamma[d]);
                    
                    // grad[d] = grad_x_norm / std + grad_var * 2.0 * (input[d] - mean) / embed_dim + grad_mean / embed_dim
                    BigFixed* term1 = big_fixed_create(training->precision_bits);
                    BigFixed* term2 = big_fixed_create(training->precision_bits);
                    BigFixed* term3 = big_fixed_create(training->precision_bits);
                    
                    // term1 = grad_x_norm / std
                    big_fixed_div(term1, grad_x_norm, std);
                    
                    // term2 = grad_var * 2.0 * (input[d] - mean) / embed_dim
                    big_fixed_sub(temp, input[d], mean);
                    big_fixed_from_double(temp2, 2.0);
                    big_fixed_mul(temp, temp, temp2);
                    big_fixed_mul(term2, grad_var, temp);
                    big_fixed_div(term2, term2, embed_dim_bf);
                    
                    // term3 = grad_mean / embed_dim
                    big_fixed_div(term3, grad_mean, embed_dim_bf);
                    
                    // grad[d] = term1 + term2 + term3
                    big_fixed_add(grad[d], term1, term2);
                    big_fixed_add(grad[d], grad[d], term3);
                    
                    big_fixed_free(grad_x_norm);
                    big_fixed_free(term1);
                    big_fixed_free(term2);
                    big_fixed_free(term3);
                }
                
                // Cleanup
                big_fixed_free(mean);
                big_fixed_free(var);
                big_fixed_free(diff);
                big_fixed_free(diff_sq);
                big_fixed_free(epsilon);
                big_fixed_free(std);
                big_fixed_free(grad_var);
                big_fixed_free(grad_mean);
                big_fixed_free(x_norm);
                big_fixed_free(temp);
                big_fixed_free(temp2);
                big_fixed_free(embed_dim_bf);
                
                
                // Attention backward - compute gradients for attention weights
                // grad is currently w.r.t. attention output
                // We need to compute gradients for Q, K, V weights
                
                // Get layer input (input to attention) - BIGFIXED IMPLEMENTATION
                BigFixed** layer_input = training->layer_inputs[layer];
                BigFixed** attn_input = &layer_input[idx * embed_dim];
                
                // Simplified attention backward: outer product for gradient accumulation
                // Full attention backward would require storing attention weights (future optimization)
                BigFixed* temp_attn = big_fixed_create(training->precision_bits);
                
                for (uint32_t d1 = 0; d1 < embed_dim; d1++) {
                    for (uint32_t d2 = 0; d2 < embed_dim; d2++) {
                        uint32_t weight_idx = d1 * embed_dim + d2;
                        
                        // Compute outer product: attn_input[d1] * grad[d2]
                        big_fixed_mul(temp_attn, attn_input[d1], grad[d2]);
                        
                        // Accumulate query gradients
                        if (training->attention_grads[layer].query_lattice) {
                            big_fixed_add(training->attention_grads[layer].query_lattice[weight_idx],
                                         training->attention_grads[layer].query_lattice[weight_idx],
                                         temp);
                        }
                        
                        // Accumulate key gradients
                        if (training->attention_grads[layer].key_lattice) {
                            big_fixed_add(training->attention_grads[layer].key_lattice[weight_idx],
                                         training->attention_grads[layer].key_lattice[weight_idx],
                                         temp);
                        }
                        
                        // Accumulate value gradients
                        if (training->attention_grads[layer].value_lattice) {
                            big_fixed_add(training->attention_grads[layer].value_lattice[weight_idx],
                                         training->attention_grads[layer].value_lattice[weight_idx],
                                         temp);
                        }
                    }
                }
                
                big_fixed_free(temp_attn);
                
                // FeedForward backward - BIGFIXED IMPLEMENTATION
                // Allocate BigFixed array for hidden gradients
                BigFixed** grad_hidden = (BigFixed**)calloc(ff->hidden_dim, sizeof(BigFixed*));
                if (!grad_hidden) continue;
                
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    grad_hidden[h] = big_fixed_create(training->precision_bits);
                    big_fixed_from_int(grad_hidden[h], 0);
                }
                
                BigFixed* temp_ff = big_fixed_create(training->precision_bits);
                
                // Backward through W2: compute W2 gradients and propagate to hidden
                for (uint32_t o = 0; o < embed_dim; o++) {
                    for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                        uint32_t w2_idx = h * embed_dim + o;
                        
                        // W2 gradient: w2_grad += hidden[h] * grad[o]
                        if (training->ff_grads[layer].w2_lattice) {
                            big_fixed_mul(temp_ff, hidden[h], grad[o]);
                            big_fixed_add(training->ff_grads[layer].w2_lattice[w2_idx],
                                         training->ff_grads[layer].w2_lattice[w2_idx],
                                         temp);
                        }
                        
                        // Propagate gradient: grad_hidden[h] += w2[h,o] * grad[o]
                        big_fixed_mul(temp_ff, ff->w2_lattice[w2_idx], grad[o]);
                        big_fixed_add(grad_hidden[h], grad_hidden[h], temp_ff);
                    }
                    
                    // Bias2 gradient: bias2_grad += grad[o]
                    if (training->ff_grads[layer].bias2) {
                        big_fixed_add(training->ff_grads[layer].bias2[o],
                                     training->ff_grads[layer].bias2[o],
                                     grad[o]);
                    }
                }
                
                // Apply tanh derivative: grad_hidden *= (1 - tanh^2)
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    double tanh_val = big_fixed_to_double(hidden[h]);
                    double tanh_deriv = 1.0 - tanh_val * tanh_val;
                    double grad_h = big_fixed_to_double(grad_hidden[h]);
                    grad_h *= tanh_deriv;
                    big_fixed_from_double(grad_hidden[h], grad_h);
                }
                
                // Backward through W1: compute W1 gradients and propagate to input
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    for (uint32_t i = 0; i < embed_dim; i++) {
                        uint32_t w1_idx = i * ff->hidden_dim + h;
                        
                        // W1 gradient: w1_grad += input[i] * grad_hidden[h]
                        if (training->ff_grads[layer].w1_lattice) {
                            big_fixed_mul(temp_ff, input[i], grad_hidden[h]);
                            big_fixed_add(training->ff_grads[layer].w1_lattice[w1_idx],
                                         training->ff_grads[layer].w1_lattice[w1_idx],
                                         temp);
                        }
                        
                        // Propagate gradient: grad[i] += w1[i,h] * grad_hidden[h]
                        big_fixed_mul(temp_ff, ff->w1_lattice[w1_idx], grad_hidden[h]);
                        big_fixed_add(grad[i], grad[i], temp_ff);
                    }
                    
                    // Bias1 gradient: bias1_grad += grad_hidden[h]
                    if (training->ff_grads[layer].bias1) {
                        big_fixed_add(training->ff_grads[layer].bias1[h],
                                     training->ff_grads[layer].bias1[h],
                                     grad_hidden[h]);
                    }
                }
                
                // Cleanup
                big_fixed_free(temp_ff);
                for (uint32_t h = 0; h < ff->hidden_dim; h++) {
                    big_fixed_free(grad_hidden[h]);
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
    
    // Get sizes for cleanup
    size_t embed_size = training->model ? training->model->vocab_size * training->model->embedding_dim : 0;
    uint32_t num_layers = training->model ? training->model->num_layers : 0;
    
    // Free BigFixed gradient buffers
    if (training->gradients) {
        for (size_t i = 0; i < embed_size; i++) {
            if (training->gradients[i]) {
                big_fixed_free(training->gradients[i]);
            }
        }
        free(training->gradients);
    }
    
    // Free BigFixed optimizer state
    if (training->optimizer_state) {
        for (size_t i = 0; i < embed_size * 2; i++) {
            if (training->optimizer_state[i]) {
                big_fixed_free(training->optimizer_state[i]);
            }
        }
        free(training->optimizer_state);
    }
    
    // Free BigFixed master weights
    if (training->master_weights) {
        for (size_t i = 0; i < embed_size; i++) {
            if (training->master_weights[i]) {
                big_fixed_free(training->master_weights[i]);
            }
        }
        free(training->master_weights);
    }
    
    // Free mixed precision buffers (still float)
    free(training->fp16_activations);
    free(training->fp16_gradients);
    
    // Free BigFixed attention gradient buffers
    if (training->attention_grads && training->model) {
        for (uint32_t i = 0; i < num_layers; i++) {
            AttentionLayer* layer = &training->model->attention_layers[i];
            uint32_t dim = layer->num_heads * layer->head_dim;
            size_t weight_size = dim * dim;
            
            if (training->attention_grads[i].query_lattice) {
                for (size_t j = 0; j < weight_size; j++) {
                    if (training->attention_grads[i].query_lattice[j]) {
                        big_fixed_free(training->attention_grads[i].query_lattice[j]);
                    }
                }
                free(training->attention_grads[i].query_lattice);
            }
            
            if (training->attention_grads[i].key_lattice) {
                for (size_t j = 0; j < weight_size; j++) {
                    if (training->attention_grads[i].key_lattice[j]) {
                        big_fixed_free(training->attention_grads[i].key_lattice[j]);
                    }
                }
                free(training->attention_grads[i].key_lattice);
            }
            
            if (training->attention_grads[i].value_lattice) {
                for (size_t j = 0; j < weight_size; j++) {
                    if (training->attention_grads[i].value_lattice[j]) {
                        big_fixed_free(training->attention_grads[i].value_lattice[j]);
                    }
                }
                free(training->attention_grads[i].value_lattice);
            }
        }
        free(training->attention_grads);
    }
    
    // Free BigFixed feed-forward gradient buffers
    if (training->ff_grads && training->model) {
        for (uint32_t i = 0; i < num_layers; i++) {
            FeedForwardLayer* layer = &training->model->ff_layers[i];
            size_t w1_size = layer->input_dim * layer->hidden_dim;
            size_t w2_size = layer->hidden_dim * layer->output_dim;
            
            if (training->ff_grads[i].w1_lattice) {
                for (size_t j = 0; j < w1_size; j++) {
                    if (training->ff_grads[i].w1_lattice[j]) {
                        big_fixed_free(training->ff_grads[i].w1_lattice[j]);
                    }
                }
                free(training->ff_grads[i].w1_lattice);
            }
            
            if (training->ff_grads[i].w2_lattice) {
                for (size_t j = 0; j < w2_size; j++) {
                    if (training->ff_grads[i].w2_lattice[j]) {
                        big_fixed_free(training->ff_grads[i].w2_lattice[j]);
                    }
                }
                free(training->ff_grads[i].w2_lattice);
            }
            
            if (training->ff_grads[i].bias1) {
                for (size_t j = 0; j < layer->hidden_dim; j++) {
                    if (training->ff_grads[i].bias1[j]) {
                        big_fixed_free(training->ff_grads[i].bias1[j]);
                    }
                }
                free(training->ff_grads[i].bias1);
            }
            
            if (training->ff_grads[i].bias2) {
                for (size_t j = 0; j < layer->output_dim; j++) {
                    if (training->ff_grads[i].bias2[j]) {
                        big_fixed_free(training->ff_grads[i].bias2[j]);
                    }
                }
                free(training->ff_grads[i].bias2);
            }
        }
        free(training->ff_grads);
    }
    
    // Free BigFixed layer norm gradient buffers
    if (training->ln_grads && training->model) {
        for (uint32_t i = 0; i < num_layers; i++) {
            CLLMLayerNorm* layer = &training->model->layer_norms[i];
            
            if (training->ln_grads[i].gamma) {
                for (size_t j = 0; j < layer->dim; j++) {
                    if (training->ln_grads[i].gamma[j]) {
                        big_fixed_free(training->ln_grads[i].gamma[j]);
                    }
                }
                free(training->ln_grads[i].gamma);
            }
            
            if (training->ln_grads[i].beta) {
                for (size_t j = 0; j < layer->dim; j++) {
                    if (training->ln_grads[i].beta[j]) {
                        big_fixed_free(training->ln_grads[i].beta[j]);
                    }
                }
                free(training->ln_grads[i].beta);
            }
        }
        free(training->ln_grads);
    }
    
    // Free backward pass buffers (still float - not used in BigFixed training)
    free(training->backward_embeddings);
    free(training->backward_grad_output);
    free(training->backward_layer_input);
    free(training->backward_layer_grad);
    free(training->backward_temp_grad);
    
    // Free embedding cache (still float - not used in BigFixed training)
    free(training->cached_input_embeddings);
    free(training->cached_target_embeddings);
    
    // Free BigFixed forward pass activation storage
    size_t seq_size = training->config.batch_size * training->config.sequence_length * training->model->embedding_dim;
    size_t logits_size = training->config.batch_size * training->config.sequence_length * training->model->vocab_size;
    
    if (training->input_embeddings) {
        for (size_t i = 0; i < seq_size; i++) {
            if (training->input_embeddings[i]) {
                big_fixed_free(training->input_embeddings[i]);
            }
        }
        free(training->input_embeddings);
    }
    
    if (training->final_hidden) {
        for (size_t i = 0; i < seq_size; i++) {
            if (training->final_hidden[i]) {
                big_fixed_free(training->final_hidden[i]);
            }
        }
        free(training->final_hidden);
    }
    
    if (training->logits) {
        for (size_t i = 0; i < logits_size; i++) {
            if (training->logits[i]) {
                big_fixed_free(training->logits[i]);
            }
        }
        free(training->logits);
    }
    
    // Free BigFixed per-layer activations
    if (training->layer_inputs) {
        for (uint32_t i = 0; i < num_layers; i++) {
            if (training->layer_inputs[i]) {
                for (size_t j = 0; j < seq_size; j++) {
                    if (training->layer_inputs[i][j]) {
                        big_fixed_free(training->layer_inputs[i][j]);
                    }
                }
                free(training->layer_inputs[i]);
            }
        }
        free(training->layer_inputs);
    }
    
    if (training->attention_outputs) {
        for (uint32_t i = 0; i < num_layers; i++) {
            if (training->attention_outputs[i]) {
                for (size_t j = 0; j < seq_size; j++) {
                    if (training->attention_outputs[i][j]) {
                        big_fixed_free(training->attention_outputs[i][j]);
                    }
                }
                free(training->attention_outputs[i]);
            }
        }
        free(training->attention_outputs);
    }
    
    if (training->ff_outputs) {
        for (uint32_t i = 0; i < num_layers; i++) {
            if (training->ff_outputs[i]) {
                for (size_t j = 0; j < seq_size; j++) {
                    if (training->ff_outputs[i][j]) {
                        big_fixed_free(training->ff_outputs[i][j]);
                    }
                }
                free(training->ff_outputs[i]);
            }
        }
        free(training->ff_outputs);
    }
    
    if (training->layer_outputs) {
        for (uint32_t i = 0; i < num_layers; i++) {
            if (training->layer_outputs[i]) {
                for (size_t j = 0; j < seq_size; j++) {
                    if (training->layer_outputs[i][j]) {
                        big_fixed_free(training->layer_outputs[i][j]);
                    }
                }
                free(training->layer_outputs[i]);
            }
        }
        free(training->layer_outputs);
    }
    
    if (training->ff_hidden) {
        for (uint32_t i = 0; i < num_layers; i++) {
            if (training->ff_hidden[i]) {
                for (size_t j = 0; j < seq_size * 4; j++) {
                    if (training->ff_hidden[i][j]) {
                        big_fixed_free(training->ff_hidden[i][j]);
                    }
                }
                free(training->ff_hidden[i]);
            }
        }
        free(training->ff_hidden);
    }
    
    // Free BigFixed attention cache
    if (training->attention_cache) {
        for (uint32_t i = 0; i < num_layers; i++) {
            // Note: attention cache sizes would need to be tracked
            // For now, just free the pointers
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

void cllm_training_free(CLLMTraining* training) {
    cllm_training_cleanup(training);
}


// ============================================================================
// STUB IMPLEMENTATIONS FOR BIGFIXED FUNCTIONS
// These are temporary stubs to fix linker errors
// TODO: Replace with proper BigFixed implementations
// ============================================================================


// ============================================================================
// STUB IMPLEMENTATIONS FOR ATTENTION FUNCTIONS
// These are temporary stubs to fix linker errors
// ============================================================================


/**
 * BigFixed Attention Forward for Inference
 * 
 * This is the BigFixed version of attention for inference.
 * Supports KV caching for efficient autoregressive generation.
 * 
 * @param layer Attention layer with weights
 * @param input Input tensor [seq_len x embed_dim] as BigFixed**
 * @param output Output tensor [seq_len x embed_dim] as BigFixed**
 * @param key_cache Cached keys (can be NULL)
 * @param value_cache Cached values (can be NULL)
 * @param seq_len Sequence length
 * @param precision BigFixed precision bits
 */
void cllm_attention_forward_bigfixed(
    AttentionLayer* layer,
    BigFixed** input,
    BigFixed** output,
    BigFixed** key_cache,
    BigFixed** value_cache,
    int seq_len,
    int precision
) {
    if (!layer || !input || !output || seq_len <= 0) return;
    
    uint32_t num_heads = layer->num_heads;
    uint32_t head_dim = layer->head_dim;
    uint32_t embed_dim = num_heads * head_dim;
    
    // Allocate Q, K, V matrices [seq_len x embed_dim]
    BigFixed** queries = (BigFixed**)calloc(seq_len * embed_dim, sizeof(BigFixed*));
    BigFixed** keys = (BigFixed**)calloc(seq_len * embed_dim, sizeof(BigFixed*));
    BigFixed** values = (BigFixed**)calloc(seq_len * embed_dim, sizeof(BigFixed*));
    
    if (!queries || !keys || !values) {
        free(queries);
        free(keys);
        free(values);
        return;
    }
    
    // Initialize BigFixed elements
    for (uint32_t i = 0; i < seq_len * embed_dim; i++) {
        queries[i] = big_fixed_create(precision);
        keys[i] = big_fixed_create(precision);
        values[i] = big_fixed_create(precision);
        big_fixed_from_int(queries[i], 0);
        big_fixed_from_int(keys[i], 0);
        big_fixed_from_int(values[i], 0);
    }
    
    // Compute Q, K, V for each position
    BigFixed* temp = big_fixed_create(precision);
    for (uint32_t pos = 0; pos < (uint32_t)seq_len; pos++) {
        BigFixed** input_pos = &input[pos * embed_dim];
        
        // Compute Q = input * W_q
        for (uint32_t d = 0; d < embed_dim; d++) {
            for (uint32_t i = 0; i < embed_dim; i++) {
                big_fixed_mul(temp, input_pos[i], layer->query_lattice[i * embed_dim + d]);
                big_fixed_add(queries[pos * embed_dim + d], queries[pos * embed_dim + d], temp);
            }
        }
        
        // Compute K = input * W_k
        for (uint32_t d = 0; d < embed_dim; d++) {
            for (uint32_t i = 0; i < embed_dim; i++) {
                big_fixed_mul(temp, input_pos[i], layer->key_lattice[i * embed_dim + d]);
                big_fixed_add(keys[pos * embed_dim + d], keys[pos * embed_dim + d], temp);
            }
        }
        
        // Compute V = input * W_v
        for (uint32_t d = 0; d < embed_dim; d++) {
            for (uint32_t i = 0; i < embed_dim; i++) {
                big_fixed_mul(temp, input_pos[i], layer->value_lattice[i * embed_dim + d]);
                big_fixed_add(values[pos * embed_dim + d], values[pos * embed_dim + d], temp);
            }
        }
    }
    big_fixed_free(temp);
    
    // TODO: Integrate KV cache if provided
    // For now, compute fresh attention each time
    (void)key_cache;
    (void)value_cache;
    
    // Compute attention scores [seq_len x seq_len]
    BigFixed** scores = (BigFixed**)calloc(seq_len * seq_len, sizeof(BigFixed*));
    for (int i = 0; i < seq_len * seq_len; i++) {
        scores[i] = big_fixed_create(precision);
        big_fixed_from_int(scores[i], 0);
    }
    
    // scores[i,j] = dot(Q[i], K[j]) / sqrt(head_dim)
    BigFixed* scale = big_fixed_create(precision);
    double scale_val = 1.0 / prime_sqrtf((double)head_dim);
    big_fixed_from_double(scale, scale_val);
    
    BigFixed* dot_prod = big_fixed_create(precision);
    for (int i = 0; i < seq_len; i++) {
        for (int j = 0; j < seq_len; j++) {
            big_fixed_from_int(dot_prod, 0);
            
            // Dot product of Q[i] and K[j]
            for (uint32_t d = 0; d < embed_dim; d++) {
                BigFixed* prod = big_fixed_create(precision);
                big_fixed_mul(prod, queries[i * embed_dim + d], keys[j * embed_dim + d]);
                big_fixed_add(dot_prod, dot_prod, prod);
                big_fixed_free(prod);
            }
            
            // Scale by 1/sqrt(head_dim)
            big_fixed_mul(scores[i * seq_len + j], dot_prod, scale);
        }
    }
    big_fixed_free(dot_prod);
    big_fixed_free(scale);
    
    // Apply softmax to each row (convert to float for softmax, then back to BigFixed)
    for (int i = 0; i < seq_len; i++) {
        // Convert row to float array
        float* row = (float*)malloc(seq_len * sizeof(float));
        for (int j = 0; j < seq_len; j++) {
            row[j] = (float)big_fixed_to_double(scores[i * seq_len + j]);
        }
        
        // Apply softmax using crystalline math
        float max_val = row[0];
        for (int j = 1; j < seq_len; j++) {
            if (row[j] > max_val) max_val = row[j];
        }
        
        double sum = 0.0;
        for (int j = 0; j < seq_len; j++) {
            row[j] = prime_expf(row[j] - max_val);
            sum += row[j];
        }
        
        if (sum > 1e-10) {
            for (int j = 0; j < seq_len; j++) {
                row[j] /= (float)sum;
            }
        }
        
        // Convert back to BigFixed
        for (int j = 0; j < seq_len; j++) {
            big_fixed_from_double(scores[i * seq_len + j], (double)row[j]);
        }
        
        free(row);
    }
    
    // Compute output = scores * V
    BigFixed* weighted_sum = big_fixed_create(precision);
    for (int i = 0; i < seq_len; i++) {
        for (uint32_t d = 0; d < embed_dim; d++) {
            big_fixed_from_int(weighted_sum, 0);
            
            for (int j = 0; j < seq_len; j++) {
                BigFixed* weighted_val = big_fixed_create(precision);
                big_fixed_mul(weighted_val, scores[i * seq_len + j], values[j * embed_dim + d]);
                big_fixed_add(weighted_sum, weighted_sum, weighted_val);
                big_fixed_free(weighted_val);
            }
            
            if (!output[i * embed_dim + d]) {
                output[i * embed_dim + d] = big_fixed_create(precision);
            }
            big_fixed_assign(output[i * embed_dim + d], weighted_sum);
        }
    }
    big_fixed_free(weighted_sum);
    
    // Cleanup
    for (uint32_t i = 0; i < seq_len * embed_dim; i++) {
        big_fixed_free(queries[i]);
        big_fixed_free(keys[i]);
        big_fixed_free(values[i]);
    }
    for (int i = 0; i < seq_len * seq_len; i++) {
        big_fixed_free(scores[i]);
    }
    free(queries);
    free(keys);
    free(values);
    free(scores);
}


void cllm_attention_forward_hybrid(CLLMModel* model, AttentionLayer* layer, 
                                   float* input, float* output,
                                   uint32_t* token_ids, float* key_cache, 
                                   float* value_cache, int seq_len) {
    (void)model; (void)token_ids; (void)key_cache; (void)value_cache;
    if (!layer || !input || !output || seq_len == 0) return;
    
    // For now, use standard attention (angular attention requires token IDs and model context)
    // TODO: Implement proper hybrid (angular + dot product) attention
    cllm_attention_forward_bigfixed(layer, input, output, key_cache, value_cache, seq_len, 128);
}
