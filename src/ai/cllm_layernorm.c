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
 * Apply layer normalization
 * 
 * LayerNorm(x) = gamma * (x - mean) / sqrt(variance + epsilon) + beta
 * 
 * @param ln Layer normalization parameters
 * @param input Input vector [dim]
 * @param output Output normalized vector [dim]
 */

/**
 * Apply layer normalization in-place
 * 
 * @param ln Layer normalization parameters
 * @param data Input/output vector [dim]
 */
void cllm_layer_norm_inplace(CLLMLayerNorm* ln, float* data) {
    if (!ln || !data) return;
    
    cllm_layer_norm_bigfixed(ln, data, data);
}

/**
 * Apply layer normalization to batch of vectors
 * 
 * @param ln Layer normalization parameters
 * @param input Input matrix [batch_size x dim]
 * @param output Output matrix [batch_size x dim]
 * @param batch_size Number of vectors
 */
void cllm_layer_norm_batch(CLLMLayerNorm* ln, float* input, float* output, int batch_size) {
    if (!ln || !input || !output || batch_size <= 0) return;
    
    uint32_t dim = ln->dim;
    
    for (int b = 0; b < batch_size; b++) {
        cllm_layer_norm_bigfixed(ln, &input[b * dim], &output[b * dim]);
    }
}

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
    
    // Allocate gamma and beta
    ln->gamma = (float*)malloc(dim * sizeof(float));
    ln->beta = (float*)malloc(dim * sizeof(float));
    
    if (!ln->gamma || !ln->beta) {
        if (ln->gamma) free(ln->gamma);
        if (ln->beta) free(ln->beta);
        return;
    }
    
    // Initialize gamma to 1.0 and beta to 0.0
    for (uint32_t i = 0; i < dim; i++) {
// DISABLED - USE BigFixed version:         ln->gamma[i] = 1.0f;
// DISABLED - USE BigFixed version:         ln->beta[i] = 0.0f;
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

/**
 * Compute layer norm statistics (for debugging/analysis)
 * 
 * @param input Input vector [dim]
 * @param dim Dimension
 * @param mean Output: computed mean
 * @param variance Output: computed variance
 */
void cllm_layer_norm_stats(float* input, uint32_t dim, float* mean, float* variance) {
    if (!input || dim == 0 || !mean || !variance) return;
    
    // Compute mean
    *mean = 0.0f;
    for (uint32_t i = 0; i < dim; i++) {
        *mean += input[i];
    }
    *mean /= (float)dim;
    
    // Compute variance
    *variance = 0.0f;
    for (uint32_t i = 0; i < dim; i++) {
        float diff = input[i] - *mean;
        *variance += diff * diff;
    }
    *variance /= (float)dim;
}