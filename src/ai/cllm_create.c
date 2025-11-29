#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_training.h"
#include "../include/cllm_pure_crystalline.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/prime_float_math.h"

// Create a model from configuration
CLLMModel* cllm_create_model(const CLLMConfig* config) {
    if (!config) return NULL;
    
    // Validate configuration
    if (config->vocab_size == 0 || config->embedding_dim == 0 || 
        config->num_layers == 0 || config->num_heads == 0) {
        fprintf(stderr, "Invalid model configuration\n");
        return NULL;
    }
    
    // Check that embedding_dim is divisible by num_heads
    if (config->embedding_dim % config->num_heads != 0) {
        fprintf(stderr, "embedding_dim must be divisible by num_heads\n");
        return NULL;
    }
    
    // Allocate model
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) {
        fprintf(stderr, "Failed to allocate model\n");
        return NULL;
    }
    
    // Set basic parameters
    model->vocab_size = config->vocab_size;
    model->embedding_dim = config->embedding_dim;
    model->num_layers = config->num_layers;
    
    // Initialize header
    memcpy(model->header.magic, "CLLM", 4);
    model->header.version = 1;
    model->header.vocab_size = config->vocab_size;
    model->header.embedding_dim = config->embedding_dim;
    model->header.num_layers = config->num_layers;
    model->header.num_heads = config->num_heads;
    model->header.context_length = config->max_seq_len;
    
    // Allocate tokens array
    model->tokens = (CLLMToken*)calloc(config->vocab_size, sizeof(CLLMToken));
    if (!model->tokens) {
        fprintf(stderr, "Failed to allocate tokens\n");
        free(model);
        return NULL;
    }
    
    // Initialize tokens with crystalline lattice structure
    // Use hierarchical abacus design with 12-fold symmetry
    printf("Initializing %u tokens with crystalline lattice structure...\n", config->vocab_size);
    
    for (uint32_t i = 0; i < config->vocab_size; i++) {
        model->tokens[i].token_id = i;
        model->tokens[i].frequency = 0;
        snprintf(model->tokens[i].token_str, sizeof(model->tokens[i].token_str), "token_%u", i);
        
        // EFFICIENT PRIME ENCODING using cached primes
        // For tokens within cache range, use direct lookup
        // For tokens beyond cache, use modular mapping to lattice
        if (i < 100000) {
            // Direct lookup from pre-computed cache (fast)
            model->tokens[i].prime_encoding = crystalline_get_nth_prime(i + 1);
        } else {
            // Use lattice mapping for large vocab (clock sudoku structure)
            // Map to one of the first 100,000 primes using 12-fold symmetry
            uint32_t lattice_layer = i / 12;
            uint32_t symmetry_group = i % 12;
            uint32_t mapped_index = (lattice_layer % 8333) * 12 + symmetry_group;  // 8333*12 ≈ 100k
            model->tokens[i].prime_encoding = crystalline_get_nth_prime(mapped_index + 1);
        }
        
        // Distribute tokens across 12 symmetry groups (kissing spheres)
        model->tokens[i].symmetry_group = i % 12;
        
        // Initialize lattice coordinates (will be computed during training)
        model->tokens[i].lattice_coords[0] = 0.0f;
        model->tokens[i].lattice_coords[1] = 0.0f;
        model->tokens[i].lattice_coords[2] = 0.0f;
        model->tokens[i].angle = 0.0f;
        model->tokens[i].radius = 0.0f;
        model->tokens[i].spiral_angle = 0.0f;
        model->tokens[i].radial_distance = 0.0f;
    }
    
    printf("✓ Token initialization complete\n");
    
    // Calculate total weights needed
    // Embedding weights: vocab_size * embedding_dim
    uint64_t embedding_weights = config->vocab_size * config->embedding_dim;
    
    // Per-layer weights:
    // - Attention: 3 * embedding_dim * embedding_dim (Q, K, V projections)
    // - Feed-forward: 2 * embedding_dim * ff_dim + embedding_dim + ff_dim (weights + biases)
    // - Layer norm: 4 * embedding_dim (2 layer norms per layer, each with gamma and beta)
    uint64_t per_layer_weights = 
        3 * config->embedding_dim * config->embedding_dim +
        2 * config->embedding_dim * config->ff_dim +
        config->embedding_dim + config->ff_dim +
        4 * config->embedding_dim;
    
    model->num_weights = embedding_weights + config->num_layers * per_layer_weights;
    model->header.total_params = model->num_weights;
    
    // Allocate weights
    model->weights = (float*)calloc(model->num_weights, sizeof(float));
    if (!model->weights) {
        fprintf(stderr, "Failed to allocate weights\n");
        free(model->tokens);
        free(model);
        return NULL;
    }
    
    // Initialize embeddings
    model->embeddings.vocab_size = config->vocab_size;
    model->embeddings.embedding_dim = config->embedding_dim;
    model->embeddings.embeddings = model->weights;
    
    // Initialize with small random values
    for (uint64_t i = 0; i < embedding_weights; i++) {
        model->embeddings.embeddings[i] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
    }
    
    // Allocate attention layers
    model->attention_layers = (AttentionLayer*)calloc(config->num_layers, sizeof(AttentionLayer));
    if (!model->attention_layers) {
        fprintf(stderr, "Failed to allocate attention layers\n");
        free(model->weights);
        free(model->tokens);
        free(model);
        return NULL;
    }
    
    // Initialize attention layers
    size_t weight_offset = embedding_weights;
    uint32_t head_dim = config->embedding_dim / config->num_heads;
    
    for (uint32_t i = 0; i < config->num_layers; i++) {
        model->attention_layers[i].layer_id = i;
        model->attention_layers[i].num_heads = config->num_heads;
        model->attention_layers[i].head_dim = head_dim;
        
        // Assign weight pointers with bounds checking
        size_t qkv_size = config->embedding_dim * config->embedding_dim;
        
        // Verify we don't exceed allocated weight buffer
        if (weight_offset + 3 * qkv_size > model->num_weights) {
            fprintf(stderr, "Error: Weight offset exceeds allocated buffer at layer %u\n", i);
            free(model->attention_layers);
            free(model->weights);
            free(model->tokens);
            free(model);
            return NULL;
        }
        
        model->attention_layers[i].query_lattice = model->weights + weight_offset;
        weight_offset += qkv_size;
        model->attention_layers[i].key_lattice = model->weights + weight_offset;
        weight_offset += qkv_size;
        model->attention_layers[i].value_lattice = model->weights + weight_offset;
        weight_offset += qkv_size;
        
        // Initialize attention weights with Xavier initialization
        float xavier_std = prime_sqrtf(2.0f / (config->embedding_dim + config->embedding_dim));
        for (size_t j = 0; j < qkv_size; j++) {
            model->attention_layers[i].query_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * xavier_std;
            model->attention_layers[i].key_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * xavier_std;
            model->attention_layers[i].value_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * xavier_std;
        }
    }
    
    // Allocate feed-forward layers
    model->ff_layers = (FeedForwardLayer*)calloc(config->num_layers, sizeof(FeedForwardLayer));
    if (!model->ff_layers) {
        fprintf(stderr, "Failed to allocate feed-forward layers\n");
        free(model->attention_layers);
        free(model->weights);
        free(model->tokens);
        free(model);
        return NULL;
    }
    
    // Initialize feed-forward layers
    for (uint32_t i = 0; i < config->num_layers; i++) {
        model->ff_layers[i].layer_id = i;
        model->ff_layers[i].input_dim = config->embedding_dim;
        model->ff_layers[i].hidden_dim = config->ff_dim;
        model->ff_layers[i].output_dim = config->embedding_dim;
        
        size_t w1_size = config->embedding_dim * config->ff_dim;
        size_t w2_size = config->ff_dim * config->embedding_dim;
        size_t total_ff_size = w1_size + config->ff_dim + w2_size + config->embedding_dim;
        
        // Verify we don't exceed allocated weight buffer
        if (weight_offset + total_ff_size > model->num_weights) {
            fprintf(stderr, "Error: Weight offset exceeds allocated buffer in FF layer %u\n", i);
            free(model->ff_layers);
            free(model->attention_layers);
            free(model->weights);
            free(model->tokens);
            free(model);
            return NULL;
        }
        
        model->ff_layers[i].w1_lattice = model->weights + weight_offset;
        weight_offset += w1_size;
        model->ff_layers[i].bias1 = model->weights + weight_offset;
        weight_offset += config->ff_dim;
        model->ff_layers[i].w2_lattice = model->weights + weight_offset;
        weight_offset += w2_size;
        model->ff_layers[i].bias2 = model->weights + weight_offset;
        weight_offset += config->embedding_dim;
        
        // Initialize FF weights with He initialization (for ReLU/tanh)
        float he_std_w1 = prime_sqrtf(2.0f / config->embedding_dim);
        float he_std_w2 = prime_sqrtf(2.0f / config->ff_dim);
        
        
        for (size_t j = 0; j < w1_size; j++) {
            model->ff_layers[i].w1_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * he_std_w1;
        }
        for (size_t j = 0; j < config->ff_dim; j++) {
            model->ff_layers[i].bias1[j] = 0.0f;  // Biases initialized to zero
        }
        for (size_t j = 0; j < w2_size; j++) {
            model->ff_layers[i].w2_lattice[j] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * he_std_w2;
        }
        for (size_t j = 0; j < config->embedding_dim; j++) {
            model->ff_layers[i].bias2[j] = 0.0f;  // Biases initialized to zero
        }
    }
    
    // Allocate layer norms
    model->layer_norms = (CLLMLayerNorm*)calloc(config->num_layers * 2, sizeof(CLLMLayerNorm));
    if (!model->layer_norms) {
        fprintf(stderr, "Failed to allocate layer norms\n");
        free(model->ff_layers);
        free(model->attention_layers);
        free(model->weights);
        free(model->tokens);
        free(model);
        return NULL;
    }
    
    // Initialize layer norms (2 per layer: pre-attention and pre-feedforward)
    for (uint32_t i = 0; i < config->num_layers * 2; i++) {
        model->layer_norms[i].layer_id = i;
        model->layer_norms[i].dim = config->embedding_dim;
        model->layer_norms[i].epsilon = 1e-5f;
        
        model->layer_norms[i].gamma = model->weights + weight_offset;
        weight_offset += config->embedding_dim;
        model->layer_norms[i].beta = model->weights + weight_offset;
        weight_offset += config->embedding_dim;
        
        // Initialize gamma to 1.0 and beta to 0.0
        for (uint32_t j = 0; j < config->embedding_dim; j++) {
            model->layer_norms[i].gamma[j] = 1.0f;
            model->layer_norms[i].beta[j] = 0.0f;
        }
    }
    
    // Initialize positional encoding
    model->pos_encoding.max_length = config->max_seq_len;
    model->pos_encoding.embedding_dim = config->embedding_dim;
    
    // Allocate positional encoding buffers
    // size_t pos_size = config->max_seq_len * config->embedding_dim * sizeof(float);  // Unused
    model->pos_encoding.spiral_positions = (float*)calloc(config->max_seq_len * config->embedding_dim, sizeof(float));
    model->pos_encoding.clock_positions = (float*)calloc(config->max_seq_len * config->embedding_dim, sizeof(float));
    model->pos_encoding.prime_positions = (float*)calloc(config->max_seq_len * config->embedding_dim, sizeof(float));
    model->pos_encoding.learned_positions = (float*)calloc(config->max_seq_len * config->embedding_dim, sizeof(float));
    
    if (!model->pos_encoding.spiral_positions || !model->pos_encoding.clock_positions ||
        !model->pos_encoding.prime_positions || !model->pos_encoding.learned_positions) {
        fprintf(stderr, "Failed to allocate positional encodings\n");
        if (model->pos_encoding.spiral_positions) free(model->pos_encoding.spiral_positions);
        if (model->pos_encoding.clock_positions) free(model->pos_encoding.clock_positions);
        if (model->pos_encoding.prime_positions) free(model->pos_encoding.prime_positions);
        if (model->pos_encoding.learned_positions) free(model->pos_encoding.learned_positions);
        free(model->layer_norms);
        free(model->ff_layers);
        free(model->attention_layers);
        free(model->weights);
        free(model->tokens);
        free(model);
        return NULL;
    }
    
    // PHASE 1: Initialize Crystalline Prime Encodings (ASI Design)
    if (model->tokens) {
        printf("\n=== Initializing Crystalline Structure ===\n");
        printf("Generating prime encodings for %u tokens...\n", config->vocab_size);
        cllm_generate_lattice_embeddings(model);
        printf("✓ Crystalline prime encodings initialized\n");
        printf("✓ 12D lattice coordinates computed\n");
        printf("==========================================\n\n");
    }
    
    return model;
}

// Free model and all associated memory
void cllm_free_model(CLLMModel* model) {
    if (!model) return;
    
    if (model->pos_encoding.spiral_positions) {
        free(model->pos_encoding.spiral_positions);
    }
    if (model->pos_encoding.clock_positions) {
        free(model->pos_encoding.clock_positions);
    }
    if (model->pos_encoding.prime_positions) {
        free(model->pos_encoding.prime_positions);
    }
    if (model->pos_encoding.learned_positions) {
        free(model->pos_encoding.learned_positions);
    }
    
    if (model->layer_norms) {
        free(model->layer_norms);
    }
    
    if (model->ff_layers) {
        free(model->ff_layers);
    }
    
    if (model->attention_layers) {
        free(model->attention_layers);
    }
    
    if (model->weights) {
        free(model->weights);
    }
    
    if (model->tokens) {
        free(model->tokens);
    }
    
    if (model->lattice_points) {
        free(model->lattice_points);
    }
    
    free(model);
}

// Estimate memory usage for a model configuration
size_t cllm_estimate_memory(const CLLMConfig* config) {
    if (!config) return 0;
    
    size_t total = 0;
    
    // Model structure
    total += sizeof(CLLMModel);
    
    // Tokens
    total += config->vocab_size * sizeof(CLLMToken);
    
    // Weights
    uint64_t embedding_weights = config->vocab_size * config->embedding_dim;
    uint64_t per_layer_weights = 
        3 * config->embedding_dim * config->embedding_dim +
        2 * config->embedding_dim * config->ff_dim +
        config->embedding_dim + config->ff_dim +
        4 * config->embedding_dim;
    uint64_t total_weights = embedding_weights + config->num_layers * per_layer_weights;
    total += total_weights * sizeof(float);
    
    // Attention layers
    total += config->num_layers * sizeof(AttentionLayer);
    
    // Feed-forward layers
    total += config->num_layers * sizeof(FeedForwardLayer);
    
    // Layer norms
    total += config->num_layers * 2 * sizeof(CLLMLayerNorm);
    
    // Positional encodings (4 types)
    total += 4 * config->max_seq_len * config->embedding_dim * sizeof(float);
    
    return total;
}

// Note: cllm_validate_model is already defined in cllm_utils.c

// Print model information
void cllm_print_model_info(const CLLMModel* model) {
    if (!model) {
        printf("Model is NULL\n");
        return;
    }
    
    printf("=== CLLM Model Information ===\n");
    printf("Version: %u\n", model->header.version);
    printf("Vocabulary Size: %lu\n", (unsigned long)model->vocab_size);
    printf("Embedding Dimension: %lu\n", (unsigned long)model->embedding_dim);
    printf("Number of Layers: %u\n", model->num_layers);
    printf("Total Weights: %lu\n", (unsigned long)model->num_weights);
    
    if (model->num_layers > 0 && model->attention_layers) {
        printf("\nAttention Configuration:\n");
        printf("  Number of Heads: %u\n", model->attention_layers[0].num_heads);
        printf("  Head Dimension: %u\n", model->attention_layers[0].head_dim);
    }
    
    if (model->num_layers > 0 && model->ff_layers) {
        printf("\nFeed-Forward Configuration:\n");
        printf("  Input Dimension: %u\n", model->ff_layers[0].input_dim);
        printf("  Hidden Dimension: %u\n", model->ff_layers[0].hidden_dim);
    }
    
    // Calculate memory usage
    size_t memory = sizeof(CLLMModel);
    memory += model->vocab_size * sizeof(CLLMToken);
    memory += model->num_weights * sizeof(float);
    memory += model->num_layers * sizeof(AttentionLayer);
    memory += model->num_layers * sizeof(FeedForwardLayer);
    memory += model->num_layers * 2 * sizeof(CLLMLayerNorm);
    
    printf("\nMemory Usage: %.2f MB\n", memory / (1024.0 * 1024.0));
    printf("==============================\n");
}

// Create a default small model for testing
CLLMModel* cllm_create_small_model(void) {
    CLLMConfig config = {
        .vocab_size = 1000,
        .embedding_dim = 128,
        .num_layers = 4,
        .num_heads = 4,
        .ff_dim = 512,
        .max_seq_len = 512,
        .dropout = 0.1f
    };
    
    return cllm_create_model(&config);
}

// Create a default medium model
CLLMModel* cllm_create_medium_model(void) {
    CLLMConfig config = {
        .vocab_size = 50000,      // Increased for better coverage
        .embedding_dim = 1024,    // Increased for richer representations
        .num_layers = 8,
        .num_heads = 8,
        .ff_dim = 4096,           // Increased for more capacity
        .max_seq_len = 1024,
        .dropout = 0.1f
    };
    
    return cllm_create_model(&config);
}

// Create a default large model
CLLMModel* cllm_create_large_model(void) {
    CLLMConfig config = {
        .vocab_size = 50000,
        .embedding_dim = 1024,
        .num_layers = 12,
        .num_heads = 16,
        .ff_dim = 4096,
        .max_seq_len = 2048,
        .dropout = 0.1f
    };
    
    return cllm_create_model(&config);
}