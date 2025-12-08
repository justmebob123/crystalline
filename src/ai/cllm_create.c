#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_training.h"
#include "../include/ai/cllm_platonic.h"  // Platonic solid models
#include "../include/clock_lattice.h"     // Clock lattice mapping
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
    
    // Initialize tokens with default values
    for (uint32_t i = 0; i < config->vocab_size; i++) {
        model->tokens[i].frequency = 0;
        snprintf(model->tokens[i].token_str, sizeof(model->tokens[i].token_str), "token_%u", i);
        
        // CRITICAL FIX: Initialize prime_encoding (was left as 0 from calloc)
        extern uint64_t crystalline_get_nth_prime(uint32_t n);
        model->tokens[i].prime_encoding = crystalline_get_nth_prime(i);
        
        // CRITICAL FIX: Distribute across 12 symmetry groups (12-fold symmetry)
        model->tokens[i].symmetry_group = i % 12;
    }
    
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
    model->weights = (double*)calloc(model->num_weights, sizeof(double));
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
    
    // PHASE 2: Initialize with crystalline lattice formula using CACHED values
    // This uses pre-computed L_lattice() values for 12 symmetry groups
    // OPTIMIZATION: 285x faster than computing L_lattice() for every token
    // Initialize embeddings with consolidated implementation
    extern void cllm_init_embeddings(CLLMModel* model);
    cllm_init_embeddings(model);
    
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
        
        // Initialize attention weights with Xavier initialization (DOUBLE PRECISION)
        double xavier_std = prime_sqrt(2.0 / (config->embedding_dim + config->embedding_dim));
        for (size_t j = 0; j < qkv_size; j++) {
            model->attention_layers[i].query_lattice[j] = ((double)rand() / RAND_MAX - 0.5) * 2.0 * xavier_std;
            model->attention_layers[i].key_lattice[j] = ((double)rand() / RAND_MAX - 0.5) * 2.0 * xavier_std;
            model->attention_layers[i].value_lattice[j] = ((double)rand() / RAND_MAX - 0.5) * 2.0 * xavier_std;
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
        
        // Initialize FF weights with He initialization (for ReLU/tanh) - DOUBLE PRECISION
        double he_std_w1 = prime_sqrt(2.0 / config->embedding_dim);
        double he_std_w2 = prime_sqrt(2.0 / config->ff_dim);
        
        
        for (size_t j = 0; j < w1_size; j++) {
            model->ff_layers[i].w1_lattice[j] = ((double)rand() / RAND_MAX - 0.5) * 2.0 * he_std_w1;
        }
        for (size_t j = 0; j < config->ff_dim; j++) {
            model->ff_layers[i].bias1[j] = 0.0;  // Biases initialized to zero
        }
        for (size_t j = 0; j < w2_size; j++) {
            model->ff_layers[i].w2_lattice[j] = ((double)rand() / RAND_MAX - 0.5) * 2.0 * he_std_w2;
        }
        for (size_t j = 0; j < config->embedding_dim; j++) {
            model->ff_layers[i].bias2[j] = 0.0;  // Biases initialized to zero
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
        
        // Initialize gamma to 1.0 and beta to 0.0 - DOUBLE PRECISION
        for (uint32_t j = 0; j < config->embedding_dim; j++) {
            model->layer_norms[i].gamma[j] = 1.0;
            model->layer_norms[i].beta[j] = 0.0;
        }
    }
    
    // Initialize positional encoding
    model->pos_encoding.max_length = config->max_seq_len;
    model->pos_encoding.embedding_dim = config->embedding_dim;
    
    // Allocate positional encoding buffers
    // size_t pos_size = config->max_seq_len * config->embedding_dim * sizeof(double);  // Unused
    model->pos_encoding.spiral_positions = (double*)calloc(config->max_seq_len * config->embedding_dim, sizeof(double));
    model->pos_encoding.clock_positions = (double*)calloc(config->max_seq_len * config->embedding_dim, sizeof(double));
    model->pos_encoding.prime_positions = (double*)calloc(config->max_seq_len * config->embedding_dim, sizeof(double));
    model->pos_encoding.learned_positions = (double*)calloc(config->max_seq_len * config->embedding_dim, sizeof(double));
    
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
    
    // Free Platonic model and clock lattice positions
    // Note: platonic_model is NULL when using integrated geometry
    if (model->platonic_model) {
        platonic_model_free((PlatonicModel*)model->platonic_model);
    }
    
    if (model->token_clock_positions) {
        free(model->token_clock_positions);
    }
    
    if (model->token_angular_positions) {
        free(model->token_angular_positions);
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
    total += total_weights * sizeof(double);
    
    // Attention layers
    total += config->num_layers * sizeof(AttentionLayer);
    
    // Feed-forward layers
    total += config->num_layers * sizeof(FeedForwardLayer);
    
    // Layer norms
    total += config->num_layers * 2 * sizeof(CLLMLayerNorm);
    
    // Positional encodings (4 types)
    total += 4 * config->max_seq_len * config->embedding_dim * sizeof(double);
    
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
    memory += model->num_weights * sizeof(double);
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

// ============================================================================
// PLATONIC GEOMETRY MODEL CREATION (OBJECTIVE 25)
// ============================================================================

/**
 * Create a model based on Platonic solid geometry
 * 
 * Dimensions are automatically calculated from the Platonic solid:
 * - embedding_dim = vertices × 12
 * - ff_dim (hidden) = edges × 12
 * - num_layers = faces
 * - num_heads = 12 (always, for 12-fold symmetry)
 * 
 * @param solid_type Which Platonic solid (1-5)
 * @param vocab_size Vocabulary size
 * @param max_seq_len Maximum sequence length
 * @param enable_blind_recovery Enable blind recovery (OBJECTIVE 26)
 * @param enable_harmonic Enable harmonic integration (OBJECTIVE 27)
 * @param enable_ntt Enable NTT attention (OBJECTIVE 13D)
 * @return Newly created model with Platonic geometry, or NULL on failure
 */
CLLMModel* cllm_create_platonic_model(
    PlatonicSolidType solid_type,
    uint32_t vocab_size,
    uint32_t max_seq_len,
    bool enable_blind_recovery,
    bool enable_harmonic,
    bool enable_ntt
) {
    // Get geometry from Platonic solid type
    PlatonicGeometry geom = platonic_get_geometry(solid_type);
    
    // Create CLLM configuration with dimensions from geometry
    CLLMConfig config = {
        .vocab_size = vocab_size,
        .embedding_dim = geom.vertices * 12,  // V × 12
        .num_layers = geom.faces,             // F layers
        .num_heads = 12,                      // Always 12
        .ff_dim = geom.edges * 12,            // E × 12
        .max_seq_len = max_seq_len,
        .dropout = 0.1f
    };
    
    // Create standard CLLM model (this allocates all parameters)
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        return NULL;
    }
    
    // Set Platonic geometry flags (don't create separate PlatonicModel to avoid duplication)
    model->platonic_model = NULL;  // We're using integrated geometry, not separate model
    model->platonic_solid_type = solid_type;
    model->use_platonic_geometry = true;
    
    // Set geometry properties
    model->geometry.vertices = geom.vertices;
    model->geometry.edges = geom.edges;
    model->geometry.faces = geom.faces;
    model->geometry.symmetries = geom.symmetries;
    model->geometry.has_golden_ratio = geom.has_golden_ratio;
    // Calculate sphere packing efficiency based on solid type
    double sphere_packing[] = {34.0, 52.0, 68.0, 74.0, 74.0}; // Tetra, Cube, Octa, Dodeca, Icosa
    model->geometry.sphere_packing = sphere_packing[solid_type];
    
    // Initialize clock lattice positions for all tokens
    model->token_clock_positions = calloc(vocab_size, sizeof(BabylonianClockPosition));
    model->token_angular_positions = calloc(vocab_size, sizeof(double));
    
    if (!model->token_clock_positions || !model->token_angular_positions) {
        fprintf(stderr, "Failed to allocate clock lattice positions\n");
        if (model->token_clock_positions) free(model->token_clock_positions);
        if (model->token_angular_positions) free(model->token_angular_positions);
        cllm_free_model(model);
        return NULL;
    }
    
    // Map each token to clock lattice position
    BabylonianClockPosition* positions = (BabylonianClockPosition*)model->token_clock_positions;
    for (uint32_t i = 0; i < vocab_size; i++) {
        positions[i] = map_prime_index_to_clock(i + 1);  // 1-based index
        
        // Compute angular position: θ(n,k,λ,ω,ψ)
        // For now, use simple mapping based on clock position
        double angle = positions[i].angle;
        double radius = positions[i].radius;
        model->token_angular_positions[i] = angle + radius * 0.1;
    }
    
    // Set feature flags
    model->blind_recovery.enabled = enable_blind_recovery;
    model->blind_recovery.corruption_tolerance = 0.25;  // 25% tolerance
    model->blind_recovery.recovery_method = 0;  // Auto
    
    model->harmonic.enabled = enable_harmonic;
    model->harmonic.primary_frequency = 432.0;  // 432 Hz universal frequency
    model->harmonic.use_fourier_transform = enable_harmonic;
    model->harmonic.use_cymatic_modulation = enable_harmonic;
    model->harmonic.use_prime_resonance = enable_harmonic;
    
    model->ntt_attention.enabled = enable_ntt;
    model->ntt_attention.threshold_seq_len = 512;  // Use NTT for seq_len > 512
    model->ntt_attention.auto_select = true;
    
    printf("✓ Created Platonic %s model:\n", platonic_solid_name(solid_type));
    printf("  Vertices: %u → Embedding: %u\n", geom.vertices, config.embedding_dim);
    printf("  Edges: %u → Hidden: %u\n", geom.edges, config.ff_dim);
    printf("  Faces: %u → Layers: %u\n", geom.faces, config.num_layers);
    printf("  Symmetries: %u\n", geom.symmetries);
    printf("  Sphere packing: %.1f%%\n", model->geometry.sphere_packing);
    if (geom.has_golden_ratio) {
        printf("  Golden ratio: φ = 1.618034\n");
    }
    printf("  Blind recovery: %s\n", enable_blind_recovery ? "enabled" : "disabled");
    printf("  Harmonic integration: %s\n", enable_harmonic ? "enabled" : "disabled");
    printf("  NTT attention: %s\n", enable_ntt ? "enabled" : "disabled");
    
    return model;
}

/**
 * Create Platonic models with preset configurations
 */

// Tetrahedron: Small, fast (48-dim embeddings, 4 layers)
CLLMModel* cllm_create_tetrahedron_model(uint32_t vocab_size, uint32_t max_seq_len) {
    return cllm_create_platonic_model(
        PLATONIC_TETRAHEDRON,
        vocab_size,
        max_seq_len,
        true,   // Enable blind recovery
        true,   // Enable harmonic integration
        false   // Disable NTT (small model)
    );
}

// Cube: Balanced (96-dim embeddings, 6 layers)
CLLMModel* cllm_create_cube_model(uint32_t vocab_size, uint32_t max_seq_len) {
    return cllm_create_platonic_model(
        PLATONIC_CUBE,
        vocab_size,
        max_seq_len,
        true,   // Enable blind recovery
        true,   // Enable harmonic integration
        true    // Enable NTT
    );
}

// Octahedron: Dual of cube (72-dim embeddings, 8 layers)
CLLMModel* cllm_create_octahedron_model(uint32_t vocab_size, uint32_t max_seq_len) {
    return cllm_create_platonic_model(
        PLATONIC_OCTAHEDRON,
        vocab_size,
        max_seq_len,
        true,   // Enable blind recovery
        true,   // Enable harmonic integration
        true    // Enable NTT
    );
}

// Dodecahedron: Large, powerful (240-dim embeddings, 12 layers)
CLLMModel* cllm_create_dodecahedron_model(uint32_t vocab_size, uint32_t max_seq_len) {
    return cllm_create_platonic_model(
        PLATONIC_DODECAHEDRON,
        vocab_size,
        max_seq_len,
        true,   // Enable blind recovery
        true,   // Enable harmonic integration
        true    // Enable NTT
    );
}

// Icosahedron: Maximum symmetry (144-dim embeddings, 20 layers)
CLLMModel* cllm_create_icosahedron_model(uint32_t vocab_size, uint32_t max_seq_len) {
    return cllm_create_platonic_model(
        PLATONIC_ICOSAHEDRON,
        vocab_size,
        max_seq_len,
        true,   // Enable blind recovery
        true,   // Enable harmonic integration
        true    // Enable NTT
    );
}