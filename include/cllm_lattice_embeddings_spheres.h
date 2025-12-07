/**
 * @file cllm_lattice_embeddings_spheres.h
 * @brief Neighbor-Influenced Embeddings using Kissing Spheres
 * 
 * Enhances token embeddings by incorporating influence from their 12 kissing
 * sphere neighbors. Uses weighted contributions based on the L(n,d,k,λ) formula
 * to create richer, more contextual representations.
 * 
 * Key Features:
 * - Each token's embedding influenced by its 12 neighbors
 * - Weighted contributions using L(n,d,k,λ) formula
 * - Preserves original embedding while adding neighbor context
 * - Configurable influence strength (0.0-1.0)
 * 
 * Architecture:
 * - Works with existing embedding system
 * - Can be applied during initialization or training
 * - Supports both in-place and copy modes
 */

#ifndef CLLM_KISSING_SPHERES_EMBEDDINGS_H
#define CLLM_KISSING_SPHERES_EMBEDDINGS_H

#include "cllm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Refine embedding using kissing sphere neighbors
 * 
 * Adds weighted contributions from the 12 kissing sphere neighbors to
 * enhance the token's embedding with contextual information from its
 * geometric neighborhood.
 * 
 * @param model CLLM model with lattice points and embeddings
 * @param token_id Token to refine embedding for
 * @param influence_strength How much neighbor influence to add (0.0-1.0)
 * 
 * @return 0 on success, -1 on error
 */
int cllm_embedding_refine_with_neighbors(
    CLLMModel* model,
    uint32_t token_id,
    float influence_strength
);

/**
 * Refine all embeddings using kissing sphere neighbors
 * 
 * Applies neighbor influence to all token embeddings in the model.
 * This creates a more contextual embedding space where tokens are
 * influenced by their geometric neighbors.
 * 
 * @param model CLLM model
 * @param influence_strength How much neighbor influence to add (0.0-1.0)
 * 
 * @return 0 on success, -1 on error
 */
int cllm_embeddings_refine_all_with_neighbors(
    CLLMModel* model,
    float influence_strength
);

/**
 * Compute neighbor-influenced embedding (non-destructive)
 * 
 * Computes a new embedding that includes neighbor influence without
 * modifying the original embedding in the model.
 * 
 * @param model CLLM model
 * @param token_id Token to compute embedding for
 * @param influence_strength How much neighbor influence to add (0.0-1.0)
 * @param output Output embedding [embed_dim]
 * 
 * @return 0 on success, -1 on error
 */
int cllm_embedding_compute_with_neighbors(
    CLLMModel* model,
    uint32_t token_id,
    float influence_strength,
    float* output
);

/**
 * Initialize embeddings with neighbor influence
 * 
 * Initializes token embeddings using both the standard initialization
 * and neighbor influence. This creates embeddings that are inherently
 * aware of their geometric neighborhood from the start.
 * 
 * @param model CLLM model
 * @param influence_strength How much neighbor influence to add (0.0-1.0)
 * 
 * @return 0 on success, -1 on error
 */
int cllm_embeddings_init_with_neighbors(
    CLLMModel* model,
    float influence_strength
);

/**
 * Compute neighbor influence vector
 * 
 * Computes the weighted sum of neighbor embeddings without adding it
 * to the original embedding. Useful for analysis and debugging.
 * 
 * @param model CLLM model
 * @param token_id Token to compute influence for
 * @param influence_vector Output influence vector [embed_dim]
 * @param weights Optional output for neighbor weights [12]
 * 
 * @return 0 on success, -1 on error
 */
int cllm_embedding_compute_neighbor_influence(
    CLLMModel* model,
    uint32_t token_id,
    float* influence_vector,
    float* weights
);

/**
 * Get statistics about neighbor influence
 * 
 * Computes statistics about how much neighbors influence embeddings.
 * 
 * @param model CLLM model
 * @param avg_influence Average influence magnitude
 * @param max_influence Maximum influence magnitude
 * @param min_influence Minimum influence magnitude
 * 
 * @return 0 on success, -1 on error
 */
int cllm_embedding_neighbor_influence_stats(
    CLLMModel* model,
    float* avg_influence,
    float* max_influence,
    float* min_influence
);

/**
 * Apply iterative neighbor refinement
 * 
 * Applies neighbor influence multiple times, allowing influence to
 * propagate through the embedding space. Each iteration refines
 * embeddings based on the current state of neighbor embeddings.
 * 
 * @param model CLLM model
 * @param num_iterations Number of refinement iterations
 * @param influence_strength Influence strength per iteration (0.0-1.0)
 * 
 * @return 0 on success, -1 on error
 */
int cllm_embeddings_iterative_refinement(
    CLLMModel* model,
    int num_iterations,
    float influence_strength
);

#ifdef __cplusplus
}
#endif

#endif // CLLM_KISSING_SPHERES_EMBEDDINGS_H