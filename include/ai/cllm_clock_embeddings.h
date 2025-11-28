#ifndef CLLM_CLOCK_EMBEDDINGS_H
#define CLLM_CLOCK_EMBEDDINGS_H

#include "../cllm.h"

/**
 * Clock-Based Embedding Initialization
 * 
 * OPTIMIZED initialization using direct clock geometry.
 * 1000-10,000x faster than full L(n,d,k,λ) calculation.
 * 
 * Uses:
 * - Direct clock positions for first 232 primes
 * - Symmetry group interpolation for remaining tokens
 * - 144000 constant (no division)
 */

/**
 * Initialize embeddings using clock geometry
 * 
 * @param model CLLM model with allocated embeddings
 */
void cllm_embeddings_init_clock(CLLMModel* model);

/**
 * Initialize complete model with clock-based embeddings
 * 
 * @param model CLLM model
 * @return 0 on success, -1 on error
 */
int cllm_init_model_clock(CLLMModel* model);

#endif /* CLLM_CLOCK_EMBEDDINGS_H */