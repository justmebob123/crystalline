/**
 * @file abacus_sparse.c
 * @brief Sparse representation implementation for CrystallineAbacus
 * 
 * Implements memory-optimized sparse representation for numbers with many zeros.
 * 
 * Memory Savings:
 * - Dense: 32 bytes per bead (includes ClockPosition)
 * - Sparse: 8 bytes per non-zero bead
 * - Reduction: 75-98% for sparse numbers
 * 
 * Automatic Switching:
 * - Switches to sparse when >50% zeros
 * - Switches to dense when >80% non-zero
 * - Transparent to users (internal optimization)
 */

#include "math/abacus.h"
#include "math/types.h"
#include <stdlib.h>
#include <string.h>

/* Sparsity threshold for automatic switching */
#define SPARSE_THRESHOLD 0.5   /* 50% or more zeros → sparse */
#define DENSE_THRESHOLD 0.8    /* 80% or more non-zero → dense */
#define LARGE_NUMBER_THRESHOLD 100  /* Numbers with >100 beads benefit from sparse */

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

/**
 * @brief Count non-zero beads in dense representation
 */
static size_t count_nonzero_beads(const CrystallineAbacus* abacus) {
    if (!abacus || !abacus->beads) {
        return 0;
    }
    
    size_t count = 0;
    for (size_t i = 0; i < abacus->num_beads; i++) {
        if (abacus->beads[i].value != 0) {
            count++;
        }
    }
    
    return count;
}

/**
 * @brief Calculate sparsity ratio (0.0 = all non-zero, 1.0 = all zeros)
 */
static double calculate_sparsity(const CrystallineAbacus* abacus) {
    if (!abacus || abacus->num_beads == 0) {
        return 0.0;
    }
    
    size_t nonzero = abacus->is_sparse ? abacus->num_nonzero : count_nonzero_beads(abacus);
    size_t total = abacus->num_beads;
    
    if (total == 0) {
        return 0.0;
    }
    
    return 1.0 - ((double)nonzero / (double)total);
}

/* ============================================================================
 * CONVERSION FUNCTIONS
 * ============================================================================ */

MathError abacus_sparsify(CrystallineAbacus* abacus) {
    if (!abacus) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    /* Already sparse */
    if (abacus->is_sparse) {
        return MATH_SUCCESS;
    }
    
    /* Count non-zero beads */
    size_t nonzero_count = count_nonzero_beads(abacus);
    
    /* Allocate sparse array */
    SparseBead* sparse = (SparseBead*)malloc(nonzero_count * sizeof(SparseBead));
    if (!sparse && nonzero_count > 0) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Copy non-zero beads to sparse array */
    size_t sparse_idx = 0;
    int32_t min_exp = 0;
    int32_t max_exp = 0;
    bool first = true;
    
    for (size_t i = 0; i < abacus->num_beads; i++) {
        if (abacus->beads[i].value != 0) {
            sparse[sparse_idx].value = abacus->beads[i].value;
            sparse[sparse_idx].weight_exponent = abacus->beads[i].weight_exponent;
            
            /* Track min/max exponents */
            if (first) {
                min_exp = sparse[sparse_idx].weight_exponent;
                max_exp = sparse[sparse_idx].weight_exponent;
                first = false;
            } else {
                if (sparse[sparse_idx].weight_exponent < min_exp) {
                    min_exp = sparse[sparse_idx].weight_exponent;
                }
                if (sparse[sparse_idx].weight_exponent > max_exp) {
                    max_exp = sparse[sparse_idx].weight_exponent;
                }
            }
            
            sparse_idx++;
        }
    }
    
    /* Free dense array and switch to sparse */
    free(abacus->beads);
    abacus->beads = NULL;
    abacus->num_beads = 0;
    abacus->capacity = 0;
    
    abacus->sparse_beads = sparse;
    abacus->num_nonzero = nonzero_count;
    abacus->sparse_capacity = nonzero_count;
    abacus->is_sparse = true;
    
    /* Update exponent range */
    if (nonzero_count > 0) {
        abacus->min_exponent = min_exp;
        abacus->max_exponent = max_exp;
    }
    
    return MATH_SUCCESS;
}

MathError abacus_densify(CrystallineAbacus* abacus) {
    if (!abacus) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    /* Already dense */
    if (!abacus->is_sparse) {
        return MATH_SUCCESS;
    }
    
    /* Calculate number of beads needed (from min to max exponent) */
    if (abacus->num_nonzero == 0) {
        /* Empty number - create single zero bead */
        abacus->beads = (AbacusBead*)calloc(1, sizeof(AbacusBead));
        if (!abacus->beads) {
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        abacus->beads[0].value = 0;
        abacus->beads[0].weight_exponent = 0;
        abacus->num_beads = 1;
        abacus->capacity = 1;
        
        free(abacus->sparse_beads);
        abacus->sparse_beads = NULL;
        abacus->num_nonzero = 0;
        abacus->sparse_capacity = 0;
        abacus->is_sparse = false;
        
        return MATH_SUCCESS;
    }
    
    /* Calculate range */
    int32_t range = abacus->max_exponent - abacus->min_exponent + 1;
    if (range <= 0) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Allocate dense array */
    AbacusBead* dense = (AbacusBead*)calloc(range, sizeof(AbacusBead));
    if (!dense) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Initialize all beads to zero */
    for (int32_t i = 0; i < range; i++) {
        dense[i].value = 0;
        dense[i].weight_exponent = abacus->min_exponent + i;
    }
    
    /* Copy non-zero values from sparse array */
    for (size_t i = 0; i < abacus->num_nonzero; i++) {
        int32_t idx = abacus->sparse_beads[i].weight_exponent - abacus->min_exponent;
        if (idx >= 0 && idx < range) {
            dense[idx].value = abacus->sparse_beads[i].value;
        }
    }
    
    /* Free sparse array and switch to dense */
    free(abacus->sparse_beads);
    abacus->sparse_beads = NULL;
    abacus->num_nonzero = 0;
    abacus->sparse_capacity = 0;
    
    abacus->beads = dense;
    abacus->num_beads = range;
    abacus->capacity = range;
    abacus->is_sparse = false;
    
    return MATH_SUCCESS;
}

MathError abacus_optimize_representation(CrystallineAbacus* abacus) {
    if (!abacus) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    /* Calculate sparsity */
    double sparsity = calculate_sparsity(abacus);
    
    /* Determine optimal representation */
    bool should_be_sparse = false;
    
    if (abacus->is_sparse) {
        /* Currently sparse - switch to dense if too many non-zeros */
        if (sparsity < (1.0 - DENSE_THRESHOLD)) {
            should_be_sparse = false;
        } else {
            should_be_sparse = true;
        }
    } else {
        /* Currently dense - switch to sparse if many zeros */
        size_t total_beads = abacus->num_beads;
        if (sparsity > SPARSE_THRESHOLD || total_beads > LARGE_NUMBER_THRESHOLD) {
            should_be_sparse = true;
        } else {
            should_be_sparse = false;
        }
    }
    
    /* Convert if needed */
    if (should_be_sparse && !abacus->is_sparse) {
        return abacus_sparsify(abacus);
    } else if (!should_be_sparse && abacus->is_sparse) {
        return abacus_densify(abacus);
    }
    
    return MATH_SUCCESS;
}

/* ============================================================================
 * QUERY FUNCTIONS
 * ============================================================================ */

double abacus_get_sparsity(const CrystallineAbacus* abacus) {
    if (!abacus) {
        return -1.0;
    }
    
    return calculate_sparsity(abacus);
}

bool abacus_is_sparse(const CrystallineAbacus* abacus) {
    if (!abacus) {
        return false;
    }
    
    return abacus->is_sparse;
}

size_t abacus_memory_usage(const CrystallineAbacus* abacus) {
    if (!abacus) {
        return 0;
    }
    
    size_t total = sizeof(CrystallineAbacus);
    
    if (abacus->is_sparse) {
        /* Sparse mode */
        total += abacus->sparse_capacity * sizeof(SparseBead);
    } else {
        /* Dense mode */
        total += abacus->capacity * sizeof(AbacusBead);
    }
    
    return total;
}