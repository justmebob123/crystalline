/*
 * CLLM Layer Normalization
 * Implements layer normalization for transformer layers
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/prime_float_math.h"
#include "bigfixed_core.h"
#include "bigfixed_array_utils.h"
#include "../include/cllm_layernorm_bigfixed.h"

/**
 * Initialize layer normalization parameters
 * 
 * @param ln Layer normalization structure to initialize
 * @param dim Dimension to normalize
 * @param epsilon Small constant for numerical stability
 */
void cllm_layer_norm_init(CLLMLayerNorm* ln, uint32_t dim, float epsilon) {
    if (!ln || dim == 0) return;
    
    ln->dim = dim;
    ln->epsilon = epsilon;
    
    // Allocate gamma and beta using BigFixed
    ln->gamma = bigfixed_array_create(dim, 128);
    ln->beta = bigfixed_array_create(dim, 128);
    
    if (!ln->gamma || !ln->beta) {
        if (ln->gamma) bigfixed_array_free(ln->gamma, dim);
        if (ln->beta) bigfixed_array_free(ln->beta, dim);
        return;
    }
    
    // Initialize gamma to 1.0 and beta to 0.0 using BigFixed
    BigFixed* one = big_fixed_create(128);
    BigFixed* zero = big_fixed_create(128);
    big_fixed_from_double(one, 1.0);
    big_fixed_from_double(zero, 0.0);
    
    for (uint32_t i = 0; i < dim; i++) {
        big_fixed_from_double(ln->gamma[i], 1.0);
        big_fixed_from_double(ln->beta[i], 0.0);
    }
    
    big_fixed_free(one);
    big_fixed_free(zero);
}

/**
 * Free layer normalization parameters
 * 
 * @param ln Layer normalization structure to free
 */
void cllm_layer_norm_free(CLLMLayerNorm* ln) {
    if (!ln) return;
    
    if (ln->gamma) {
        bigfixed_array_free(ln->gamma, ln->dim);
        ln->gamma = NULL;
    }
    
    if (ln->beta) {
        bigfixed_array_free(ln->beta, ln->dim);
        ln->beta = NULL;
    }
}