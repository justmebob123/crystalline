/**
 * @file cllm_neighbor_ops.c
 * @brief Internal Neighbor Operations Implementation
 * 
 * INTERNAL USE ONLY
 * 
 * Implements core neighbor-based operations for the 12 kissing spheres.
 * These functions are used internally by attention and embedding functions.
 */

#include "cllm_neighbor_ops.h"
#include "cllm_mathematical_constants.h"
#include "prime_lattice_core.h"
#include "prime_float_math.h"
#include "prime_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Softmax normalization
 */
static void softmax(float* weights, int size) {
    if (!weights || size <= 0) return;
    
    float max_val = weights[0];
    for (int i = 1; i < size; i++) {
        if (weights[i] > max_val) max_val = weights[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        weights[i] = prime_expf(weights[i] - max_val);
        sum += weights[i];
    }
    
    if (sum > 0.0f) {
        for (int i = 0; i < size; i++) {
            weights[i] /= sum;
        }
    }
}

/**
 * Compute neighbor weights using L(n,d,k,λ)
 */
int cllm_internal_compute_neighbor_weights(
    CLLMModel* model,
    uint32_t token_id,
    float* weights
) {
    if (!model || !weights) return -1;
    if (token_id >= model->vocab_size) return -1;
    if (!model->lattice_points || token_id >= model->num_lattice_points) return -1;
    
    CLLMLatticePoint* point = &model->lattice_points[token_id];
    CLLMToken* center_token = &model->tokens[token_id];
    
    // Compute raw weights using L(n,d,k,λ)
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        
        if (neighbor_id >= model->vocab_size) {
            weights[i] = 0.0f;
            continue;
        }
        
        uint64_t phi_i = cllm_get_dimensional_frequency(i);
        
        double interaction = L_lattice(
            center_token->prime_encoding,
            i,
            center_token->symmetry_group,
            center_token->token_str,
            3,
            center_token->prime_encoding,
            phi_i
        );
        
        weights[i] = (float)prime_fabsf((float)interaction);
    }
    
    // Fill remaining weights
    for (uint32_t i = point->num_neighbors; i < 12; i++) {
        weights[i] = 0.0f;
    }
    
    // Normalize
    softmax(weights, 12);
    
    return 0;
}

/**
 * Add neighbor attention contributions
 */
int cllm_internal_add_neighbor_attention(
    CLLMModel* model,
    uint32_t token_id,
    double* output,
    uint32_t embed_dim,
    float strength
) {
    if (!model || !output) return -1;
    if (token_id >= model->vocab_size) return -1;
    if (!model->lattice_points || token_id >= model->num_lattice_points) return -1;
    if (strength < 0.0f || strength > 1.0f) return -1;
    
    CLLMLatticePoint* point = &model->lattice_points[token_id];
    
    // Compute weights
    float weights[12];
    if (cllm_internal_compute_neighbor_weights(model, token_id, weights) != 0) {
        return -1;
    }
    
    // Add weighted neighbor contributions
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        if (neighbor_id >= model->vocab_size) continue;
        
        float weight = weights[i] * strength;
        double* neighbor_embedding = &model->embeddings.embeddings[
            neighbor_id * embed_dim
        ];
        
        for (uint32_t d = 0; d < embed_dim; d++) {
            output[d] += weight * neighbor_embedding[d];
        }
    }
    
    return 0;
}

/**
 * Compute neighbor influence vector
 */
int cllm_internal_compute_neighbor_influence(
    CLLMModel* model,
    uint32_t token_id,
    float* influence_vector,
    float* weights
) {
    if (!model || !influence_vector) return -1;
    if (token_id >= model->vocab_size) return -1;
    if (!model->lattice_points || token_id >= model->num_lattice_points) return -1;
    
    CLLMLatticePoint* point = &model->lattice_points[token_id];
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Compute weights
    float neighbor_weights[12];
    if (cllm_internal_compute_neighbor_weights(model, token_id, neighbor_weights) != 0) {
        return -1;
    }
    
    if (weights) {
        memcpy(weights, neighbor_weights, 12 * sizeof(float));
    }
    
    // Initialize influence vector
    memset(influence_vector, 0, embed_dim * sizeof(float));
    
    // Accumulate weighted neighbor embeddings
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        if (neighbor_id >= model->vocab_size) continue;
        
        float weight = neighbor_weights[i];
        double* neighbor_embedding = &model->embeddings.embeddings[
            neighbor_id * embed_dim
        ];
        
        for (uint32_t d = 0; d < embed_dim; d++) {
            influence_vector[d] += weight * (float)neighbor_embedding[d];
        }
    }
    
    return 0;
}

/**
 * Apply neighbor influence to single embedding
 */
int cllm_internal_apply_neighbor_influence(
    CLLMModel* model,
    uint32_t token_id,
    float influence_strength
) {
    if (!model) return -1;
    if (token_id >= model->vocab_size) return -1;
    if (influence_strength < 0.0f || influence_strength > 1.0f) return -1;
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Compute influence vector
    float* influence_vector = (float*)malloc(embed_dim * sizeof(float));
    if (!influence_vector) return -1;
    
    if (cllm_internal_compute_neighbor_influence(model, token_id, influence_vector, NULL) != 0) {
        free(influence_vector);
        return -1;
    }
    
    // Apply influence: embedding = (1-strength)*original + strength*influence
    double* embedding = &model->embeddings.embeddings[token_id * embed_dim];
    float original_weight = 1.0f - influence_strength;
    
    for (uint32_t d = 0; d < embed_dim; d++) {
        embedding[d] = original_weight * embedding[d] + 
                      influence_strength * (double)influence_vector[d];
    }
    
    free(influence_vector);
    return 0;
}

/**
 * Apply neighbor influence to all embeddings
 */
int cllm_internal_apply_neighbor_influence_all(
    CLLMModel* model,
    float influence_strength
) {
    if (!model) return -1;
    if (!model->lattice_points) return -1;
    if (influence_strength < 0.0f || influence_strength > 1.0f) return -1;
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    size_t total_size = model->num_lattice_points * embed_dim;
    
    // Create temporary buffer for refined embeddings
    float* refined_embeddings = (float*)malloc(total_size * sizeof(float));
    if (!refined_embeddings) return -1;
    
    // Compute all refined embeddings
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        float* influence_vector = &refined_embeddings[i * embed_dim];
        
        if (cllm_internal_compute_neighbor_influence(model, i, influence_vector, NULL) != 0) {
            free(refined_embeddings);
            return -1;
        }
        
        // Blend with original
        double* original = &model->embeddings.embeddings[i * embed_dim];
        float original_weight = 1.0f - influence_strength;
        
        for (uint32_t d = 0; d < embed_dim; d++) {
            influence_vector[d] = original_weight * (float)original[d] + 
                                 influence_strength * influence_vector[d];
        }
    }
    
    // Update all embeddings
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        double* embedding = &model->embeddings.embeddings[i * embed_dim];
        float* refined = &refined_embeddings[i * embed_dim];
        
        for (uint32_t d = 0; d < embed_dim; d++) {
            embedding[d] = (double)refined[d];
        }
    }
    
    free(refined_embeddings);
    return 0;
}