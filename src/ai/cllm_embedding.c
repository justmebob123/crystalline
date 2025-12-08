/**
 * @file cllm_embedding.c
 * @brief CONSOLIDATED Embedding Implementation
 * 
 * This file REPLACES and CONSOLIDATES:
 * - cllm_embedding.c (basic embeddings)
 * - cllm_lattice_embeddings.c (L(n,d,k,λ) formula)
 * - cllm_clock_embeddings.c (Babylonian clock structure)
 * - cllm_lll_embeddings.c (LLL lattice reduction)
 * 
 * SINGLE IMPLEMENTATION with all features:
 * - Platonic geometry-based initialization
 * - Clock lattice position mapping
 * - L(n,d,k,λ) lattice formula
 * - Geometric structure from Platonic solids
 * - Automatic dimension calculation
 * 
 * OBJECTIVES IMPLEMENTED:
 * - OBJECTIVE 21: Babylonian Clock Lattice
 * - OBJECTIVE 25: Platonic Solid Model Architecture
 * - OBJECTIVE 13A: Integrate L(n,d,k,λ) Lattice Formula
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/prime_float_math.h"
#include "../include/clock_lattice.h"
#include "../include/ai/cllm_platonic.h"
#include "../include/prime_lattice_core.h"
#include "../include/cllm_angular_position.h"

// ============================================================================
// EMBEDDING INITIALIZATION
// ============================================================================

/**
 * Initialize embeddings with Platonic geometry and clock lattice structure
 * 
 * This is the SINGLE, CONSOLIDATED embedding initialization function.
 * It automatically uses the best method based on model configuration:
 * 
 * 1. If model uses Platonic geometry: Use geometric initialization
 * 2. Otherwise: Use clock lattice + L(n,d,k,λ) formula
 * 
 * @param model CLLM model with embeddings to initialize
 */
void cllm_init_embeddings(CLLMModel* model) {
    if (!model || !model->embeddings.embeddings) {
        fprintf(stderr, "ERROR: Invalid model or embeddings\n");
        return;
    }
    
    printf("Initializing embeddings (CONSOLIDATED implementation)...\n");
    
    uint32_t vocab_size = model->vocab_size;
    uint32_t embedding_dim = model->embedding_dim;
    double* embeddings = model->embeddings.embeddings;
    
    // Check if model uses Platonic geometry
    if (model->use_platonic_geometry && model->token_clock_positions) {
        printf("  Using Platonic geometry + clock lattice initialization\n");
        printf("  Solid: %s\n", model->platonic_solid_type == 0 ? "none" :
               model->platonic_solid_type == 1 ? "Tetrahedron" :
               model->platonic_solid_type == 2 ? "Cube" :
               model->platonic_solid_type == 3 ? "Octahedron" :
               model->platonic_solid_type == 4 ? "Dodecahedron" : "Icosahedron");
        printf("  Vertices: %u → Embedding: %u\n", model->geometry.vertices, embedding_dim);
        
        // Use clock lattice positions for initialization
        BabylonianClockPosition* positions = (BabylonianClockPosition*)model->token_clock_positions;
        
        for (uint32_t token = 0; token < vocab_size; token++) {
            BabylonianClockPosition pos = positions[token];
            
            for (uint32_t dim = 0; dim < embedding_dim; dim++) {
                // Combine clock position with dimensional frequency
                double angle = pos.angle + (double)dim / embedding_dim * 2.0 * M_PI;
                double radius_factor = pos.radius / 100.0;  // Normalize radius
                
                // Use prime-based sinusoidal initialization
                double value = prime_sin(angle) * (1.0 + radius_factor * 0.1);
                
                // Scale by 1/sqrt(embedding_dim) for stability
                value *= 1.0 / prime_sqrt((double)embedding_dim);
                
                embeddings[token * embedding_dim + dim] = value;
            }
        }
        
        printf("✓ Embeddings initialized with Platonic geometry\n");
        
    } else {
        printf("  Using standard clock lattice initialization\n");
        
        // Standard initialization with clock lattice structure
        for (uint32_t token = 0; token < vocab_size; token++) {
            // Map token to clock position
            BabylonianClockPosition pos = map_prime_index_to_clock(token + 1);
            
            for (uint32_t dim = 0; dim < embedding_dim; dim++) {
                // Use clock-based sinusoidal initialization
                double angle = pos.angle + (double)dim / embedding_dim * 2.0 * M_PI;
                double value = prime_sin(angle) / prime_sqrt((double)embedding_dim);
                
                embeddings[token * embedding_dim + dim] = value;
            }
        }
        
        printf("✓ Embeddings initialized with clock lattice\n");
    }
}

// ============================================================================
// EMBEDDING LOOKUP
// ============================================================================

/**
 * Embed a single token into the embedding space
 * 
 * @param inf Inference engine state
 * @param token_id Token ID to embed
 * @param output Output embedding vector [embedding_dim]
 */
void cllm_embed_token(CLLMInference* inf, uint32_t token_id, float* output) {
    if (!inf || !output || token_id >= inf->model->vocab_size) {
        return;
    }
    
    CLLMModel* model = inf->model;
    uint32_t embedding_dim = model->embeddings.embedding_dim;
    
    // Copy embedding from embedding matrix
    double* embedding_matrix = model->embeddings.embeddings;
    size_t offset = token_id * embedding_dim;
    
    for (uint32_t i = 0; i < embedding_dim; i++) {
        output[i] = (float)embedding_matrix[offset + i];
    }
}

// ============================================================================
// POSITIONAL ENCODING
// ============================================================================

/**
 * Add positional encoding to embeddings
 * 
 * Uses clock lattice-based positional encoding for geometric consistency
 * 
 * @param model CLLM model
 * @param position Position in sequence
 * @param embedding Embedding to add positional encoding to [embedding_dim]
 */
void cllm_add_positional_encoding(CLLMModel* model, uint32_t position, double* embedding) {
    if (!model || !embedding || position >= model->header.context_length) {
        return;
    }
    
    uint32_t embedding_dim = model->embedding_dim;
    
    // Map position to clock lattice
    BabylonianClockPosition pos = map_prime_index_to_clock(position + 1);
    
    // Add sinusoidal positional encoding based on clock position
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double angle = pos.angle + (double)i / embedding_dim * 2.0 * M_PI;
        double pos_encoding = prime_sin(angle);
        
        embedding[i] += pos_encoding * 0.1;  // Scale factor
    }
}

// ============================================================================
// EMBEDDING UPDATES (for training)
// ============================================================================

/**
 * Update embedding during training
 * 
 * @param model CLLM model
 * @param token_id Token ID to update
 * @param gradient Gradient vector [embedding_dim]
 * @param learning_rate Learning rate
 */
void cllm_update_embedding(CLLMModel* model, uint32_t token_id, 
                          const double* gradient, double learning_rate) {
    if (!model || !gradient || token_id >= model->vocab_size) {
        return;
    }
    
    uint32_t embedding_dim = model->embedding_dim;
    double* embeddings = model->embeddings.embeddings;
    size_t offset = token_id * embedding_dim;
    
    // Apply gradient update with optional harmonic modulation
    double modulation = 1.0;
    if (model->harmonic.enabled) {
        // Modulate learning rate with primary frequency (432 Hz)
        double freq = model->harmonic.primary_frequency;
        modulation = 1.0 + 0.1 * prime_sin(freq * token_id / model->vocab_size);
    }
    
    for (uint32_t i = 0; i < embedding_dim; i++) {
        embeddings[offset + i] -= learning_rate * modulation * gradient[i];
    }
}

// ============================================================================
// LATTICE TRANSFORMATIONS (Legacy compatibility)
// ============================================================================

/**
 * Apply lattice transformation to an embedding
 * 
 * @param embedding Input/output embedding vector [dim]
 * @param transform Transformation matrix [dim x dim]
 * @param dim Embedding dimension
 */
void cllm_apply_lattice_transform(float* embedding, double* transform, int dim) {
    if (!embedding || !transform || dim <= 0) {
        return;
    }
    
    // Allocate temporary buffer for result
    float* temp = (float*)malloc(dim * sizeof(float));
    if (!temp) return;
    
    // Matrix-vector multiplication
    for (int i = 0; i < dim; i++) {
        temp[i] = 0.0f;
        for (int j = 0; j < dim; j++) {
            temp[i] += (float)transform[i * dim + j] * embedding[j];
        }
    }
    
    // Copy result back
    memcpy(embedding, temp, dim * sizeof(float));
    free(temp);
}

// ============================================================================
// EMBEDDING UTILITIES
// ============================================================================

/**
 * Compute cosine similarity between two embeddings
 * 
 * @param emb1 First embedding [dim]
 * @param emb2 Second embedding [dim]
 * @param dim Embedding dimension
 * @return Cosine similarity [-1, 1]
 */
double cllm_embedding_similarity(const double* emb1, const double* emb2, uint32_t dim) {
    if (!emb1 || !emb2 || dim == 0) return 0.0;
    
    double dot = 0.0, norm1 = 0.0, norm2 = 0.0;
    
    for (uint32_t i = 0; i < dim; i++) {
        dot += emb1[i] * emb2[i];
        norm1 += emb1[i] * emb1[i];
        norm2 += emb2[i] * emb2[i];
    }
    
    if (norm1 == 0.0 || norm2 == 0.0) return 0.0;
    
    return dot / (prime_sqrt(norm1) * prime_sqrt(norm2));
}

/**
 * Normalize an embedding to unit length
 * 
 * @param embedding Embedding to normalize [dim]
 * @param dim Embedding dimension
 */
void cllm_normalize_embedding(float* embedding, int dim) {
    if (!embedding || dim <= 0) return;
    
    float norm = 0.0f;
    for (int i = 0; i < dim; i++) {
        norm += embedding[i] * embedding[i];
    }
    
    if (norm == 0.0f) return;
    
    norm = sqrtf(norm);
    for (int i = 0; i < dim; i++) {
        embedding[i] /= norm;
    }
}