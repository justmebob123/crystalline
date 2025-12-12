/*
 * CLLM Lattice Conversion Utilities
 * 
 * MIGRATED FROM: BigFixed conversion utilities
 * NEW API: Uses CrystallineAbacus from NEW math library
 */

#include "../../include/cllm.h"
#include "math/abacus.h"
#include "math/types.h"
#include "math/clock.h"
#include "../../include/prime_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/prime_float_math.h"

void cllm_float_to_abacus(CrystallineAbacus** output, float* input, int n, 
                         uint32_t base, int32_t precision, ClockContext* ctx) {
    if (!output || !input || n <= 0 || !ctx) return;
    
    for (int i = 0; i < n; i++) {
        if (output[i]) {
            MathError err = abacus_from_double((double)input[i], base, precision, ctx, output[i]);
            if (err != MATH_SUCCESS) {
                fprintf(stderr, "WARNING: cllm_float_to_abacus failed at index %d with error %d\n", i, err);
            }
        }
    }
}

void cllm_abacus_to_float(float* output, CrystallineAbacus** input, int n) {
    if (!output || !input || n <= 0) return;
    
    for (int i = 0; i < n; i++) {
        if (input[i]) {
            output[i] = (float)abacus_to_double(input[i]);
        } else {
            output[i] = 0.0f;
        }
    }
}

void cllm_embeddings_to_basis(CrystallineAbacus*** basis, float* embeddings, 
                              int n, int dim, uint32_t base, int32_t precision, 
                              ClockContext* ctx) {
    if (!basis || !embeddings || n <= 0 || dim <= 0 || !ctx) return;
    
    for (int i = 0; i < n; i++) {
        if (basis[i]) {
            cllm_float_to_abacus(basis[i], &embeddings[i * dim], dim, base, precision, ctx);
        }
    }
}

void cllm_basis_to_embeddings(float* embeddings, CrystallineAbacus*** basis,
                              int n, int dim) {
    if (!embeddings || !basis || n <= 0 || dim <= 0) return;
    
    for (int i = 0; i < n; i++) {
        if (basis[i]) {
            cllm_abacus_to_float(&embeddings[i * dim], basis[i], dim);
        }
    }
}

CrystallineAbacus*** cllm_alloc_abacus_basis(int n, int dim, uint32_t base, ClockContext* ctx) {
    if (n <= 0 || dim <= 0 || !ctx) return NULL;
    
    CrystallineAbacus*** basis = (CrystallineAbacus***)malloc(n * sizeof(CrystallineAbacus**));
    if (!basis) return NULL;
    
    for (int i = 0; i < n; i++) {
        basis[i] = (CrystallineAbacus**)malloc(dim * sizeof(CrystallineAbacus*));
        if (!basis[i]) {
            for (int j = 0; j < i; j++) {
                for (int d = 0; d < dim; d++) {
                    abacus_free(basis[j][d]);
                }
                free(basis[j]);
            }
            free(basis);
            return NULL;
        }
        
        for (int d = 0; d < dim; d++) {
            basis[i][d] = abacus_create_from_uint64(0, base, ctx);
            if (!basis[i][d]) {
                // Cleanup on failure
                for (int dd = 0; dd < d; dd++) {
                    abacus_free(basis[i][dd]);
                }
                for (int j = 0; j < i; j++) {
                    for (int dd = 0; dd < dim; dd++) {
                        abacus_free(basis[j][dd]);
                    }
                    free(basis[j]);
                }
                free(basis[i]);
                free(basis);
                return NULL;
            }
        }
    }
    
    return basis;
}

void cllm_free_abacus_basis(CrystallineAbacus*** basis, int n, int dim) {
    if (!basis) return;
    
    for (int i = 0; i < n; i++) {
        if (basis[i]) {
            for (int d = 0; d < dim; d++) {
                if (basis[i][d]) {
                    abacus_free(basis[i][d]);
                }
            }
            free(basis[i]);
        }
    }
    free(basis);
}

void cllm_embedding_to_abacus(CrystallineAbacus** output, float* embedding,
                              int dim, uint32_t base, int32_t precision, 
                              ClockContext* ctx) {
    cllm_float_to_abacus(output, embedding, dim, base, precision, ctx);
}

void cllm_abacus_to_embedding(float* embedding, CrystallineAbacus** vector, int dim) {
    cllm_abacus_to_float(embedding, vector, dim);
}

float cllm_test_conversion_accuracy(float* input, int n, uint32_t base, 
                                   int32_t precision, ClockContext* ctx) {
    if (!input || n <= 0 || !ctx) return -1.0f;
    
    CrystallineAbacus** abacus = (CrystallineAbacus**)malloc(n * sizeof(CrystallineAbacus*));
    float* output = (float*)malloc(n * sizeof(float));
    
    if (!abacus || !output) {
        if (abacus) free(abacus);
        if (output) free(output);
        return -1.0f;
    }
    
    // Create abacus array
    for (int i = 0; i < n; i++) {
        abacus[i] = abacus_create_from_uint64(0, base, ctx);
        if (!abacus[i]) {
            for (int j = 0; j < i; j++) {
                abacus_free(abacus[j]);
            }
            free(abacus);
            free(output);
            return -1.0f;
        }
    }
    
    cllm_float_to_abacus(abacus, input, n, base, precision, ctx);
    cllm_abacus_to_float(output, abacus, n);
    
    float max_error = 0.0f;
    for (int i = 0; i < n; i++) {
        float error = prime_fabsf(output[i] - input[i]);
        if (error > max_error) {
            max_error = error;
        }
    }
    
    // Cleanup
    for (int i = 0; i < n; i++) {
        abacus_free(abacus[i]);
    }
    free(abacus);
    free(output);
    
    return max_error;
}

void cllm_print_conversion_stats(float* input, int n, uint32_t base, 
                                int32_t precision, ClockContext* ctx) {
    if (!input || n <= 0 || !ctx) return;
    
    float max_error = cllm_test_conversion_accuracy(input, n, base, precision, ctx);
    
    printf("Conversion Statistics:\n");
    printf("  Elements: %d\n", n);
    printf("  Base: %u\n", base);
    printf("  Precision: %d (10^%d)\n", precision, precision);
    printf("  Max error: %.10e\n", max_error);
    
    if (max_error < 1e-6f) {
        printf("  Status: EXCELLENT (error < 1e-6)\n");
    } else if (max_error < 1e-4f) {
        printf("  Status: GOOD (error < 1e-4)\n");
    } else if (max_error < 1e-2f) {
        printf("  Status: ACCEPTABLE (error < 1e-2)\n");
    } else {
        printf("  Status: WARNING (error >= 1e-2)\n");
    }
}

/* ============================================================================
 * LEGACY COMPATIBILITY WRAPPERS (for gradual migration)
 * These will be removed once all code is migrated
 * ============================================================================
 */

typedef CrystallineAbacus BigFixed;

static ClockContext global_ctx;
static bool global_ctx_initialized = false;

static void ensure_global_ctx() {
    if (!global_ctx_initialized) {
        clock_init(&global_ctx);
        global_ctx_initialized = true;
    }
}

void cllm_float_to_bigfixed(BigFixed* output, float* input, int n, int precision) {
    ensure_global_ctx();
    CrystallineAbacus** abacus_array = (CrystallineAbacus**)malloc(n * sizeof(CrystallineAbacus*));
    for (int i = 0; i < n; i++) {
        abacus_array[i] = &output[i];
    }
    cllm_float_to_abacus(abacus_array, input, n, 12, precision, &global_ctx);
    free(abacus_array);
}

void cllm_bigfixed_to_float(float* output, BigFixed* input, int n) {
    CrystallineAbacus** abacus_array = (CrystallineAbacus**)malloc(n * sizeof(CrystallineAbacus*));
    for (int i = 0; i < n; i++) {
        abacus_array[i] = &input[i];
    }
    cllm_abacus_to_float(output, abacus_array, n);
    free(abacus_array);
}

void cllm_embeddings_to_basis(BigFixed** basis, float* embeddings, 
                              int n, int dim, int precision) {
    ensure_global_ctx();
    CrystallineAbacus*** abacus_basis = (CrystallineAbacus***)malloc(n * sizeof(CrystallineAbacus**));
    for (int i = 0; i < n; i++) {
        abacus_basis[i] = (CrystallineAbacus**)malloc(dim * sizeof(CrystallineAbacus*));
        for (int d = 0; d < dim; d++) {
            abacus_basis[i][d] = (CrystallineAbacus*)&basis[i][d];
        }
    }
    cllm_embeddings_to_basis(abacus_basis, embeddings, n, dim, 12, precision, &global_ctx);
    for (int i = 0; i < n; i++) {
        free(abacus_basis[i]);
    }
    free(abacus_basis);
}

void cllm_basis_to_embeddings(float* embeddings, BigFixed** basis, int n, int dim) {
    CrystallineAbacus*** abacus_basis = (CrystallineAbacus***)malloc(n * sizeof(CrystallineAbacus**));
    for (int i = 0; i < n; i++) {
        abacus_basis[i] = (CrystallineAbacus**)malloc(dim * sizeof(CrystallineAbacus*));
        for (int d = 0; d < dim; d++) {
            abacus_basis[i][d] = (CrystallineAbacus*)&basis[i][d];
        }
    }
    cllm_basis_to_embeddings(embeddings, abacus_basis, n, dim);
    for (int i = 0; i < n; i++) {
        free(abacus_basis[i]);
    }
    free(abacus_basis);
}

BigFixed** cllm_alloc_bigfixed_basis(int n, int dim) {
    ensure_global_ctx();
    return (BigFixed**)cllm_alloc_abacus_basis(n, dim, 12, &global_ctx);
}

void cllm_free_bigfixed_basis(BigFixed** basis, int n) {
    cllm_free_abacus_basis((CrystallineAbacus***)basis, n, 0); // dim not needed for free
}

void cllm_embedding_to_bigfixed(BigFixed* output, float* embedding, int dim, int precision) {
    ensure_global_ctx();
    CrystallineAbacus** abacus_array = (CrystallineAbacus**)malloc(dim * sizeof(CrystallineAbacus*));
    for (int i = 0; i < dim; i++) {
        abacus_array[i] = &output[i];
    }
    cllm_embedding_to_abacus(abacus_array, embedding, dim, 12, precision, &global_ctx);
    free(abacus_array);
}

void cllm_bigfixed_to_embedding(float* embedding, BigFixed* vector, int dim) {
    CrystallineAbacus** abacus_array = (CrystallineAbacus**)malloc(dim * sizeof(CrystallineAbacus*));
    for (int i = 0; i < dim; i++) {
        abacus_array[i] = &vector[i];
    }
    cllm_abacus_to_embedding(embedding, abacus_array, dim);
    free(abacus_array);
}

float cllm_test_conversion_accuracy(float* input, int n, int precision) {
    ensure_global_ctx();
    return cllm_test_conversion_accuracy(input, n, 12, precision, &global_ctx);
}

void cllm_print_conversion_stats(float* input, int n, int precision) {
    ensure_global_ctx();
    cllm_print_conversion_stats(input, n, 12, precision, &global_ctx);
}