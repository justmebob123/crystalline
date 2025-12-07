/**
 * @file cllm_kissing_spheres_attention.h
 * @brief Kissing Spheres Attention Mechanism
 * 
 * Implements attention where each token attends to its 12 kissing sphere neighbors.
 * Uses the L(n,d,k,λ) lattice formula to compute attention weights based on
 * geometric relationships in the Babylonian clock structure.
 * 
 * Key Features:
 * - Each token has exactly 12 neighbors (kissing spheres configuration)
 * - Attention weights computed using L(n,d,k,λ) formula
 * - Integrates with existing attention mechanisms
 * - O(12) complexity per token (constant time)
 * 
 * Architecture:
 * - Uses CLLMLatticePoint for neighbor information
 * - Leverages clock lattice geometry for deterministic neighbors
 * - Compatible with standard attention layers
 */

#ifndef CLLM_KISSING_SPHERES_ATTENTION_H
#define CLLM_KISSING_SPHERES_ATTENTION_H

#include "cllm.h"
#include "cllm_attention.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute attention using kissing spheres neighbors
 * 
 * Each token attends to its 12 kissing sphere neighbors. Attention weights
 * are computed using the L(n,d,k,λ) lattice formula, which captures the
 * geometric relationship between tokens in the clock lattice structure.
 * 
 * @param model CLLM model with lattice points and tokens
 * @param token_id Token to compute attention for
 * @param query Query vector [embed_dim]
 * @param output Output vector [embed_dim] (accumulated attention)
 * @param attention_weights Optional output for attention weights [12]
 * 
 * @return 0 on success, -1 on error
 */
int cllm_attention_kissing_spheres(
    CLLMModel* model,
    uint32_t token_id,
    const float* query,
    float* output,
    float* attention_weights  // Optional, can be NULL
);

/**
 * Compute attention for a batch of tokens using kissing spheres
 * 
 * Processes multiple tokens in parallel, computing attention for each
 * token based on its 12 kissing sphere neighbors.
 * 
 * @param model CLLM model
 * @param token_ids Array of token IDs [batch_size]
 * @param batch_size Number of tokens to process
 * @param queries Query vectors [batch_size x embed_dim]
 * @param outputs Output vectors [batch_size x embed_dim]
 * 
 * @return 0 on success, -1 on error
 */
int cllm_attention_kissing_spheres_batch(
    CLLMModel* model,
    const uint32_t* token_ids,
    uint32_t batch_size,
    const float* queries,
    float* outputs
);

/**
 * Compute attention weights for kissing sphere neighbors
 * 
 * Computes the 12 attention weights using L(n,d,k,λ) formula.
 * Weights are normalized using softmax.
 * 
 * @param model CLLM model
 * @param token_id Token to compute weights for
 * @param weights Output weights [12]
 * 
 * @return 0 on success, -1 on error
 */
int cllm_compute_kissing_spheres_weights(
    CLLMModel* model,
    uint32_t token_id,
    float* weights
);

/**
 * Integrate kissing spheres attention with standard attention layer
 * 
 * Adds kissing spheres attention as an additional component to the
 * standard attention mechanism. The output is a weighted combination
 * of standard attention and kissing spheres attention.
 * 
 * @param layer Standard attention layer
 * @param model CLLM model
 * @param token_id Token to compute attention for
 * @param input Input vector [embed_dim]
 * @param output Output vector [embed_dim]
 * @param ks_weight Weight for kissing spheres component (0.0-1.0)
 * 
 * @return 0 on success, -1 on error
 */
int cllm_attention_hybrid_kissing_spheres(
    AttentionLayer* layer,
    CLLMModel* model,
    uint32_t token_id,
    const double* input,
    double* output,
    float ks_weight
);

/**
 * Get statistics about kissing spheres attention
 * 
 * @param model CLLM model
 * @param avg_weight Average attention weight across all neighbors
 * @param max_weight Maximum attention weight
 * @param min_weight Minimum attention weight
 * 
 * @return 0 on success, -1 on error
 */
int cllm_kissing_spheres_attention_stats(
    CLLMModel* model,
    float* avg_weight,
    float* max_weight,
    float* min_weight
);

#ifdef __cplusplus
}
#endif

#endif // CLLM_KISSING_SPHERES_ATTENTION_H