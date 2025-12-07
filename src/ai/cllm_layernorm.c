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
    
    // Allocate gamma and beta using double arrays
    ln->gamma = (double*)malloc(dim * sizeof(double));
    ln->beta = (double*)malloc(dim * sizeof(double));
    
    if (!ln->gamma || !ln->beta) {
        if (ln->gamma) free(ln->gamma);
        if (ln->beta) free(ln->beta);
        return;
    }
    
    // Initialize gamma to 1.0 and beta to 0.0
    for (uint32_t i = 0; i < dim; i++) {
        ln->gamma[i] = 1.0;
        ln->beta[i] = 0.0;
    }
}

/**
 * Free layer normalization parameters
 * 
 * @param ln Layer normalization structure to free
 */
void cllm_layer_norm_free(CLLMLayerNorm* ln) {
    if (!ln) return;
    
    if (ln->gamma) {
        free(ln->gamma);
        ln->gamma = NULL;
    }
    
    if (ln->beta) {
        free(ln->beta);
        ln->beta = NULL;
    }
}