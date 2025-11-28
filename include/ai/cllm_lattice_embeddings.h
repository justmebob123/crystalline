#ifndef CLLM_LATTICE_EMBEDDINGS_H
#define CLLM_LATTICE_EMBEDDINGS_H

#include "../cllm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize embeddings using L(n,d,k,λ) lattice formula
 * 
 * Replaces random initialization with crystalline lattice formula.
 * Each embedding value is computed using:
 * L(n,d,k,λ) = 3^O(n,k,λ) · ∏cos(θ·φᵢ) · Γ(k) · ν(λ) · Γ(n,d)
 * 
 * @param model CLLM model with tokens and embeddings
 */
void cllm_embeddings_init_lattice(CLLMModel* model);

/**
 * Verify embedding quality by checking symmetry group similarity
 * 
 * Tokens in the same symmetry group should have more similar embeddings.
 * 
 * @param model CLLM model with initialized embeddings
 */
void cllm_verify_embedding_symmetry(CLLMModel* model);

/**
 * Compare lattice embeddings to random baseline
 * 
 * @param model CLLM model
 * @param num_samples Number of tokens to sample for comparison
 */
void cllm_compare_embedding_methods(CLLMModel* model, uint32_t num_samples);

/**
 * Initialize embeddings with lattice formula and verify quality
 * 
 * Main entry point for lattice-based embedding initialization.
 * 
 * @param model CLLM model
 */
void cllm_init_embeddings_with_lattice(CLLMModel* model);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_LATTICE_EMBEDDINGS_H */