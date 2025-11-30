#!/usr/bin/env python3
"""
Phase 1: Migrate model creation to use BigFixed and CrystallineEmbeddings
This is a comprehensive migration that will:
1. Update weight allocation to use BigFixed**
2. Initialize CrystallineEmbeddings instead of float embeddings
3. Add precision configuration
"""

import os
import sys

def create_bigfixed_model_creation():
    """Create a new BigFixed-based model creation function"""
    
    code = '''
/*
 * Create BigFixed-based model with CrystallineEmbeddings
 * This is the NEW implementation using arbitrary precision
 */
CLLMModel* cllm_create_model_bigfixed(const CLLMConfig* config) {
    if (!config) return NULL;
    
    // Validate configuration
    if (config->vocab_size == 0 || config->embedding_dim == 0 || 
        config->num_layers == 0 || config->num_heads == 0) {
        fprintf(stderr, "Invalid model configuration\\n");
        return NULL;
    }
    
    // Check that embedding_dim is divisible by num_heads
    if (config->embedding_dim % config->num_heads != 0) {
        fprintf(stderr, "embedding_dim must be divisible by num_heads\\n");
        return NULL;
    }
    
    // Allocate model
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) {
        fprintf(stderr, "Failed to allocate model\\n");
        return NULL;
    }
    
    // Set basic parameters
    model->vocab_size = config->vocab_size;
    model->embedding_dim = config->embedding_dim;
    model->num_layers = config->num_layers;
    
    // NEW: Set precision configuration
    model->precision_bits = 256;  // Default: 256-bit precision
    model->use_bigfixed = true;   // Always true
    
    printf("Creating model with BigFixed arbitrary precision (%d bits)\\n", model->precision_bits);
    
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
        fprintf(stderr, "Failed to allocate tokens\\n");
        free(model);
        return NULL;
    }
    
    // Initialize tokens with crystalline lattice structure
    printf("Initializing %u tokens with crystalline lattice structure...\\n", config->vocab_size);
    
    for (uint32_t i = 0; i < config->vocab_size; i++) {
        model->tokens[i].token_id = i;
        model->tokens[i].frequency = 0;
        snprintf(model->tokens[i].token_str, sizeof(model->tokens[i].token_str), "token_%u", i);
        
        // EFFICIENT PRIME ENCODING using cached primes
        if (i < 100000) {
            model->tokens[i].prime_encoding = crystalline_get_nth_prime(i + 1);
        } else {
            uint32_t lattice_layer = i / 12;
            uint32_t symmetry_group = i % 12;
            uint32_t mapped_index = (lattice_layer % 8333) * 12 + symmetry_group;
            model->tokens[i].prime_encoding = crystalline_get_nth_prime(mapped_index + 1);
        }
        
        // Distribute tokens across 12 symmetry groups
        model->tokens[i].symmetry_group = i % 12;
        
        // Initialize lattice coordinates (will be computed by CrystallineEmbeddings)
        model->tokens[i].lattice_coords[0] = 0.0f;
        model->tokens[i].lattice_coords[1] = 0.0f;
        model->tokens[i].lattice_coords[2] = 0.0f;
        model->tokens[i].angle = 0.0f;
        model->tokens[i].radius = 0.0f;
        model->tokens[i].spiral_angle = 0.0f;
        model->tokens[i].radial_distance = 0.0f;
    }
    
    printf("✓ Token initialization complete\\n");
    
    // NEW: Create CrystallineEmbeddings (BigFixed-based)
    printf("Creating CrystallineEmbeddings with BigFixed (lattice_dim=3)...\\n");
    model->crystalline_embeddings = crystalline_embeddings_create(config->vocab_size, 3);
    if (!model->crystalline_embeddings) {
        fprintf(stderr, "Failed to create CrystallineEmbeddings\\n");
        free(model->tokens);
        free(model);
        return NULL;
    }
    
    // Initialize crystalline embeddings with tokens
    printf("Adding tokens to CrystallineEmbeddings...\\n");
    for (uint32_t i = 0; i < config->vocab_size; i++) {
        CrystallineToken* cryst_token = crystalline_token_create(
            model->tokens[i].token_id,
            model->tokens[i].token_str,
            model->tokens[i].prime_encoding
        );
        
        if (cryst_token) {
            crystalline_embeddings_add_token(model->crystalline_embeddings, cryst_token);
        }
    }
    
    // Initialize lattice basis
    printf("Initializing lattice basis...\\n");
    if (!crystalline_initialize_basis(model->crystalline_embeddings)) {
        fprintf(stderr, "Failed to initialize lattice basis\\n");
        crystalline_embeddings_free(model->crystalline_embeddings);
        free(model->tokens);
        free(model);
        return NULL;
    }
    
    printf("✓ CrystallineEmbeddings initialized\\n");
    
    // NEW: Allocate BigFixed weights
    printf("Allocating BigFixed weights...\\n");
    
    // Calculate total weights needed
    uint64_t embedding_weights = config->vocab_size * config->embedding_dim;
    uint64_t per_layer_weights = 
        3 * config->embedding_dim * config->embedding_dim +
        2 * config->embedding_dim * config->ff_dim +
        config->embedding_dim + config->ff_dim +
        4 * config->embedding_dim;
    
    model->num_weights = embedding_weights + config->num_layers * per_layer_weights;
    model->header.total_params = model->num_weights;
    
    // Allocate BigFixed weight matrix
    model->weights = (BigFixed**)malloc(model->num_weights * sizeof(BigFixed*));
    if (!model->weights) {
        fprintf(stderr, "Failed to allocate BigFixed weight array\\n");
        crystalline_embeddings_free(model->crystalline_embeddings);
        free(model->tokens);
        free(model);
        return NULL;
    }
    
    // Initialize each BigFixed weight
    for (uint64_t i = 0; i < model->num_weights; i++) {
        model->weights[i] = (BigFixed*)malloc(sizeof(BigFixed));
        if (!model->weights[i]) {
            fprintf(stderr, "Failed to allocate BigFixed weight %lu\\n", i);
            // Free previously allocated weights
            for (uint64_t j = 0; j < i; j++) {
                big_fixed_free(model->weights[j]);
                free(model->weights[j]);
            }
            free(model->weights);
            crystalline_embeddings_free(model->crystalline_embeddings);
            free(model->tokens);
            free(model);
            return NULL;
        }
        
        // Initialize with small random values using BigFixed
        // Range: [-0.05, 0.05]
        double rand_val = ((double)rand() / RAND_MAX - 0.5) * 0.1;
        big_fixed_from_double(model->weights[i], rand_val);
    }
    
    printf("✓ BigFixed weights allocated and initialized\\n");
    
    // DEPRECATED: Initialize legacy float embeddings for backward compatibility
    // These will be removed in Phase 6
    model->embeddings.vocab_size = config->vocab_size;
    model->embeddings.embedding_dim = config->embedding_dim;
    model->embeddings.embeddings = NULL;  // Not allocated - use crystalline_embeddings instead
    
    // TODO: Allocate and initialize attention layers with BigFixed
    // TODO: Allocate and initialize feed-forward layers with BigFixed
    // TODO: Allocate and initialize layer norms with BigFixed
    
    printf("✓ Model creation complete (BigFixed mode)\\n");
    printf("  - Vocab size: %u\\n", config->vocab_size);
    printf("  - Embedding dim: %u\\n", config->embedding_dim);
    printf("  - Num layers: %u\\n", config->num_layers);
    printf("  - Precision: %d bits\\n", model->precision_bits);
    printf("  - Total weights: %lu (BigFixed)\\n", model->num_weights);
    
    return model;
}
'''
    
    return code

def main():
    # Read the current cllm_create.c
    with open('src/ai/cllm_create.c', 'r') as f:
        content = f.read()
    
    # Add the new function before the existing cllm_create_model
    new_function = create_bigfixed_model_creation()
    
    # Find where to insert (before the existing cllm_create_model function)
    insert_pos = content.find('// Create a model from configuration')
    if insert_pos == -1:
        insert_pos = content.find('CLLMModel* cllm_create_model(const CLLMConfig* config)')
    
    if insert_pos == -1:
        print("✗ Could not find insertion point")
        return False
    
    # Insert the new function
    content = content[:insert_pos] + new_function + '\n\n' + content[insert_pos:]
    
    # Write back
    with open('src/ai/cllm_create.c', 'w') as f:
        f.write(content)
    
    print("✓ Added cllm_create_model_bigfixed() to src/ai/cllm_create.c")
    print("✓ Old cllm_create_model() kept for backward compatibility")
    return True

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)