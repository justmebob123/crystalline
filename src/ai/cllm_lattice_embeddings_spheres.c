/**
 * @file cllm_kissing_spheres_embeddings.c
 * @brief Neighbor-Influenced Embeddings Implementation
 * 
 * Implements embedding refinement using kissing sphere neighbors.
 * Each token's embedding is enhanced with weighted contributions from
 * its 12 geometric neighbors.
 */

#include "cllm_lattice_embeddings_spheres.h"
#include "cllm_mathematical_constants.h"
#include "prime_lattice_core.h"
#include "prime_float_math.h"
#include "prime_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Compute neighbor influence weights using L(n,d,k,λ)
 */
static int compute_neighbor_weights(
    CLLMModel* model,
    uint32_t token_id,
    double* weights
) {
    if (!model || !weights) return -1;
    
    if (token_id >= model->vocab_size) return -1;
    if (!model->lattice_points || token_id >= model->num_lattice_points) return -1;
    
    CLLMLatticePoint* point = &model->lattice_points[token_id];
    CLLMToken* center_token = &model->tokens[token_id];
    
    double sum = 0.0;
    
    // Compute raw weights using L(n,d,k,λ)
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        
        if (neighbor_id >= model->vocab_size) {
            weights[i] = 0.0;
            continue;
        }
        
        // Get dimensional frequency
        uint64_t phi_i = cllm_get_dimensional_frequency(i);
        
        // Compute interaction strength
        double interaction = L_lattice(
            center_token->prime_encoding,
            i,
            center_token->symmetry_group,
            center_token->token_str,
            3,
            center_token->prime_encoding,
            phi_i
        );
        
        // Use absolute value and normalize
        weights[i] = (double)prime_fabsf((double)interaction);
        sum += weights[i];
    }
    
    // Fill remaining weights
    for (uint32_t i = point->num_neighbors; i < 12; i++) {
        weights[i] = 0.0;
    }
    
    // Normalize to sum to 1.0
    if (sum > 0.0) {
        for (int i = 0; i < 12; i++) {
            weights[i] /= (double)sum;
        }
    }
    
    return 0;
}

/**
 * Compute neighbor influence vector
 */
int cllm_embedding_compute_neighbor_influence(
    CLLMModel* model,
    uint32_t token_id,
    double* influence_vector,
    double* weights
) {
    if (!model || !influence_vector) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_embedding_compute_neighbor_influence\n");
        return -1;
    }
    
    if (token_id >= model->vocab_size) {
        fprintf(stderr, "ERROR: Token ID %u out of range\n", token_id);
        return -1;
    }
    
    if (!model->lattice_points || token_id >= model->num_lattice_points) {
        fprintf(stderr, "ERROR: No lattice point for token %u\n", token_id);
        return -1;
    }
    
    CLLMLatticePoint* point = &model->lattice_points[token_id];
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Compute weights
    double neighbor_weights[12];
    if (compute_neighbor_weights(model, token_id, neighbor_weights) != 0) {
        return -1;
    }
    
    // Copy weights if requested
    if (weights) {
        memcpy(weights, neighbor_weights, 12 * sizeof(double));
    }
    
    // Initialize influence vector to zero
    memset(influence_vector, 0, embed_dim * sizeof(double));
    
    // Accumulate weighted neighbor embeddings
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        
        if (neighbor_id >= model->vocab_size) continue;
        
        double weight = neighbor_weights[i];
        double* neighbor_embedding = &model->embeddings.embeddings[
            neighbor_id * embed_dim
        ];
        
        // Add weighted contribution
        for (uint32_t d = 0; d < embed_dim; d++) {
            influence_vector[d] += weight * (double)neighbor_embedding[d];
        }
    }
    
    return 0;
}

/**
 * Compute neighbor-influenced embedding (non-destructive)
 */
int cllm_embedding_compute_with_neighbors(
    CLLMModel* model,
    uint32_t token_id,
    double influence_strength,
    double* output
) {
    if (!model || !output) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_embedding_compute_with_neighbors\n");
        return -1;
    }
    
    if (influence_strength < 0.0 || influence_strength > 1.0) {
        fprintf(stderr, "ERROR: influence_strength must be in range [0.0, 1.0]\n");
        return -1;
    }
    
    if (token_id >= model->vocab_size) {
        fprintf(stderr, "ERROR: Token ID %u out of range\n", token_id);
        return -1;
    }
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Get original embedding
    double* original_embedding = &model->embeddings.embeddings[
        token_id * embed_dim
    ];
    
    // Compute neighbor influence
    double* influence_vector = (double*)malloc(embed_dim * sizeof(double));
    if (!influence_vector) {
        fprintf(stderr, "ERROR: Failed to allocate influence vector\n");
        return -1;
    }
    
    if (cllm_embedding_compute_neighbor_influence(model, token_id, influence_vector, NULL) != 0) {
        free(influence_vector);
        return -1;
    }
    
    // Combine: output = (1 - strength) * original + strength * influence
    double original_weight = 1.0 - influence_strength;
    for (uint32_t d = 0; d < embed_dim; d++) {
        output[d] = original_weight * (double)original_embedding[d] + 
                    influence_strength * influence_vector[d];
    }
    
    free(influence_vector);
    return 0;
}

/**
 * Refine embedding using kissing sphere neighbors
 */
int cllm_embedding_refine_with_neighbors(
    CLLMModel* model,
    uint32_t token_id,
    double influence_strength
) {
    if (!model) {
        fprintf(stderr, "ERROR: Invalid model\n");
        return -1;
    }
    
    if (token_id >= model->vocab_size) {
        fprintf(stderr, "ERROR: Token ID %u out of range\n", token_id);
        return -1;
    }
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Compute refined embedding
    double* refined_embedding = (double*)malloc(embed_dim * sizeof(double));
    if (!refined_embedding) {
        fprintf(stderr, "ERROR: Failed to allocate refined embedding\n");
        return -1;
    }
    
    if (cllm_embedding_compute_with_neighbors(model, token_id, influence_strength, 
                                              refined_embedding) != 0) {
        free(refined_embedding);
        return -1;
    }
    
    // Update embedding in model
    double* embedding = &model->embeddings.embeddings[token_id * embed_dim];
    for (uint32_t d = 0; d < embed_dim; d++) {
        embedding[d] = (double)refined_embedding[d];
    }
    
    free(refined_embedding);
    return 0;
}

/**
 * Refine all embeddings using kissing sphere neighbors
 */
int cllm_embeddings_refine_all_with_neighbors(
    CLLMModel* model,
    double influence_strength
) {
    if (!model) {
        fprintf(stderr, "ERROR: Invalid model\n");
        return -1;
    }
    
    if (!model->lattice_points) {
        fprintf(stderr, "ERROR: No lattice points in model\n");
        return -1;
    }
    
    printf("Refining all embeddings with neighbor influence (strength=%.2f)...\n", 
           influence_strength);
    
    // Create temporary buffer for all refined embeddings
    uint32_t embed_dim = model->embeddings.embedding_dim;
    size_t total_size = model->num_lattice_points * embed_dim;
    double* refined_embeddings = (double*)malloc(total_size * sizeof(double));
    if (!refined_embeddings) {
        fprintf(stderr, "ERROR: Failed to allocate refined embeddings buffer\n");
        return -1;
    }
    
    // Compute all refined embeddings
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        double* output = &refined_embeddings[i * embed_dim];
        if (cllm_embedding_compute_with_neighbors(model, i, influence_strength, output) != 0) {
            fprintf(stderr, "ERROR: Failed to refine embedding for token %u\n", i);
            free(refined_embeddings);
            return -1;
        }
    }
    
    // Update all embeddings at once
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        double* embedding = &model->embeddings.embeddings[i * embed_dim];
        double* refined = &refined_embeddings[i * embed_dim];
        for (uint32_t d = 0; d < embed_dim; d++) {
            embedding[d] = (double)refined[d];
        }
    }
    
    free(refined_embeddings);
    printf("✓ All embeddings refined successfully\n");
    return 0;
}

/**
 * Initialize embeddings with neighbor influence
 */
int cllm_embeddings_init_with_neighbors(
    CLLMModel* model,
    double influence_strength
) {
    if (!model) {
        fprintf(stderr, "ERROR: Invalid model\n");
        return -1;
    }
    
    printf("Initializing embeddings with neighbor influence...\n");
    
    // First, ensure embeddings are initialized
    if (!model->embeddings.embeddings) {
        fprintf(stderr, "ERROR: Embeddings not initialized\n");
        return -1;
    }
    
    // Apply neighbor influence
    return cllm_embeddings_refine_all_with_neighbors(model, influence_strength);
}

/**
 * Get statistics about neighbor influence
 */
int cllm_embedding_neighbor_influence_stats(
    CLLMModel* model,
    double* avg_influence,
    double* max_influence,
    double* min_influence
) {
    if (!model || !avg_influence || !max_influence || !min_influence) {
        fprintf(stderr, "ERROR: Invalid parameters\n");
        return -1;
    }
    
    if (!model->lattice_points) {
        fprintf(stderr, "ERROR: No lattice points\n");
        return -1;
    }
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    double* influence_vector = (double*)malloc(embed_dim * sizeof(double));
    if (!influence_vector) {
        fprintf(stderr, "ERROR: Failed to allocate influence vector\n");
        return -1;
    }
    
    double sum = 0.0;
    double max_val = 0.0;
    double min_val = 1e10;
    uint32_t count = 0;
    
    // Sample first 100 tokens
    uint32_t sample_size = (model->num_lattice_points < 100) ? 
                           model->num_lattice_points : 100;
    
    for (uint32_t i = 0; i < sample_size; i++) {
        if (cllm_embedding_compute_neighbor_influence(model, i, influence_vector, NULL) != 0) {
            continue;
        }
        
        // Compute magnitude of influence
        double magnitude = 0.0;
        for (uint32_t d = 0; d < embed_dim; d++) {
            magnitude += influence_vector[d] * influence_vector[d];
        }
        magnitude = prime_sqrtf(magnitude);
        
        sum += magnitude;
        if (magnitude > max_val) max_val = magnitude;
        if (magnitude < min_val) min_val = magnitude;
        count++;
    }
    
    *avg_influence = (count > 0) ? (double)(sum / count) : 0.0;
    *max_influence = max_val;
    *min_influence = min_val;
    
    free(influence_vector);
    return 0;
}

/**
 * Apply iterative neighbor refinement
 */
int cllm_embeddings_iterative_refinement(
    CLLMModel* model,
    int num_iterations,
    double influence_strength
) {
    if (!model) {
        fprintf(stderr, "ERROR: Invalid model\n");
        return -1;
    }
    
    if (num_iterations <= 0) {
        fprintf(stderr, "ERROR: num_iterations must be positive\n");
        return -1;
    }
    
    printf("Applying iterative neighbor refinement (%d iterations, strength=%.2f)...\n",
           num_iterations, influence_strength);
    
    for (int iter = 0; iter < num_iterations; iter++) {
        printf("  Iteration %d/%d...\n", iter + 1, num_iterations);
        
        if (cllm_embeddings_refine_all_with_neighbors(model, influence_strength) != 0) {
            fprintf(stderr, "ERROR: Failed at iteration %d\n", iter + 1);
            return -1;
        }
    }
    
    printf("✓ Iterative refinement complete\n");
    return 0;
}