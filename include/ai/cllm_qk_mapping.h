/**
 * @file cllm_qk_mapping.h
 * @brief Q→k mapping for AI inference (Question to Answer)
 * 
 * Maps Question vectors (Q) to Answer keys (k) using tensor distance
 * calculations and crystalline geometry. This is the core inference
 * mechanism for the AI model.
 * 
 * The mapping uses:
 * - Tensor distance: det(Q1 - Q2) with golden ratio geometry
 * - Entropy-guided search space reduction
 * - Harmonic alignment for validation
 */

#ifndef CLLM_QK_MAPPING_H
#define CLLM_QK_MAPPING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Q→k mapping context
 */
typedef struct {
    double* Q_vector;           /**< Question embedding vector */
    size_t Q_dim;               /**< Dimension of Q vector */
    uint64_t* k_candidates;     /**< Candidate answer keys */
    size_t num_candidates;      /**< Number of candidates */
    double* distances;          /**< Tensor distances for each candidate */
    uint64_t p_mod;             /**< Modulus for tensor calculations */
} QtoK_Mapping;

/**
 * @brief Create Q→k mapper
 * 
 * @param Q_dim Dimension of question vectors
 * @param num_candidates Number of candidate answer keys
 * @return Mapper context, or NULL on failure
 */
QtoK_Mapping* prime_qk_mapper_create(size_t Q_dim, size_t num_candidates);

/**
 * @brief Destroy Q→k mapper
 * 
 * @param mapper Mapper to destroy
 */
void prime_qk_mapper_destroy(QtoK_Mapping* mapper);

/**
 * @brief Compute tensor distance between two Q vectors
 * 
 * Calculates: det(Q1 - Q2) = (dx * gy - dy * gx) mod p_mod
 * where g = (gx, gy) is golden ratio geometry
 * 
 * @param Q1 First question vector
 * @param Q2 Second question vector
 * @param dim Vector dimension
 * @param p_mod Modulus for calculation
 * @return Tensor distance
 */
double prime_tensor_distance(const double* Q1, const double* Q2,
                              size_t dim, uint64_t p_mod);

/**
 * @brief Find best k for given Q query
 * 
 * Searches through candidates to find k with minimum tensor distance.
 * 
 * @param mapper Mapper context
 * @param Q_query Query question vector
 * @return Best matching answer key
 */
uint64_t prime_qk_find_best(const QtoK_Mapping* mapper,
                             const double* Q_query);

/**
 * @brief Find top N best k candidates
 * 
 * Returns N candidates with smallest tensor distances.
 * 
 * @param mapper Mapper context
 * @param Q_query Query question vector
 * @param top_k Output array for top k values (pre-allocated, size N)
 * @param N Number of top candidates to return
 * @return Number of candidates found
 */
size_t prime_qk_find_top_n(const QtoK_Mapping* mapper,
                            const double* Q_query,
                            uint64_t* top_k, size_t N);

/**
 * @brief Set candidate keys
 * 
 * @param mapper Mapper context
 * @param candidates Array of candidate keys
 * @param num_candidates Number of candidates
 * @return true on success
 */
bool prime_qk_set_candidates(QtoK_Mapping* mapper,
                              const uint64_t* candidates,
                              size_t num_candidates);

/**
 * @brief Generate anchors for search space
 * 
 * Creates anchor points using golden ratio geometry.
 * 
 * @param anchors Output array (pre-allocated)
 * @param num_anchors Number of anchors to generate
 * @param dim Dimension of anchor vectors
 * @param seed Random seed
 */
void prime_qk_generate_anchors(double* anchors, size_t num_anchors,
                                size_t dim, uint64_t seed);

/**
 * @brief Estimate k from anchors
 * 
 * Uses anchor-based triangulation to estimate k.
 * 
 * @param Q Query vector
 * @param anchors Anchor vectors
 * @param num_anchors Number of anchors
 * @param dim Vector dimension
 * @param p_mod Modulus
 * @return Estimated k value
 */
uint64_t prime_qk_estimate_from_anchors(const double* Q,
                                         const double* anchors,
                                         size_t num_anchors,
                                         size_t dim,
                                         uint64_t p_mod);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_QK_MAPPING_H */