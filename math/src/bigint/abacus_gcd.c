/**
 * @file abacus_gcd.c
 * @brief GCD, LCM, and coprimality operations for CrystallineAbacus
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * Implements arbitrary precision GCD, LCM, and coprimality testing
 * using pure CrystallineAbacus operations (no uint64 shortcuts).
 */

#include "math/abacus.h"
#include "math/types.h"
#include <stdbool.h>
#include <stdlib.h>

/* ============================================================================
 * GCD, LCM, AND COPRIMALITY OPERATIONS
 * ============================================================================
 */

/**
 * @brief Compute GCD of two arbitrary precision numbers using Euclidean algorithm
 * 
 * Uses the classical Euclidean algorithm:
 *   gcd(a, b) = gcd(b, a mod b)
 *   gcd(a, 0) = a
 * 
 * This is a pure geometric operation on the CrystallineAbacus.
 * 
 * @param result Output: GCD of a and b
 * @param a First number
 * @param b Second number
 * @return MATH_SUCCESS on success, error code on failure
 */
MathError abacus_gcd(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (a->base != b->base) {
        return MATH_ERROR_INVALID_BASE;
    }
    
    /* Create working copies */
    CrystallineAbacus *temp_a = abacus_copy(a);
    if (!temp_a) return MATH_ERROR_OUT_OF_MEMORY;
    
    CrystallineAbacus *temp_b = abacus_copy(b);
    if (!temp_b) {
        abacus_free(temp_a);
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* GCD is always positive */
    temp_a->negative = false;
    temp_b->negative = false;
    
    /* Euclidean algorithm: gcd(a, b) = gcd(b, a mod b) */
    while (!abacus_is_zero(temp_b)) {
        /* remainder = temp_a mod temp_b */
        CrystallineAbacus *quotient = abacus_new(a->base);
        CrystallineAbacus *remainder = abacus_new(a->base);
        
        if (!quotient || !remainder) {
            abacus_free(quotient);
            abacus_free(remainder);
            abacus_free(temp_a);
            abacus_free(temp_b);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        MathError err = abacus_div(quotient, remainder, temp_a, temp_b);
        abacus_free(quotient);
        
        if (err != MATH_SUCCESS) {
            abacus_free(remainder);
            abacus_free(temp_a);
            abacus_free(temp_b);
            return err;
        }
        
        /* temp_a = temp_b */
        abacus_free(temp_a);
        temp_a = temp_b;
        
        /* temp_b = remainder */
        temp_b = remainder;
    }
    
    /* Copy result from temp_a */
    CrystallineAbacus *result_copy = abacus_copy(temp_a);
    if (!result_copy) {
        abacus_free(temp_a);
        abacus_free(temp_b);
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Free old result beads and copy new ones */
    if (result->beads) {
        free(result->beads);
    }
    result->beads = result_copy->beads;
    result->num_beads = result_copy->num_beads;
    result->capacity = result_copy->capacity;
    result->base = result_copy->base;
    result->negative = false;
    result->min_exponent = result_copy->min_exponent;
    
    /* Free the wrapper but not the beads (we transferred ownership) */
    free(result_copy);
    
    abacus_free(temp_a);
    abacus_free(temp_b);
    
    return MATH_SUCCESS;
}

/**
 * @brief Compute LCM of two arbitrary precision numbers
 * 
 * Uses the formula: lcm(a, b) = (a * b) / gcd(a, b)
 * 
 * @param result Output: LCM of a and b
 * @param a First number
 * @param b Second number
 * @return MATH_SUCCESS on success, error code on failure
 */
MathError abacus_lcm(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (a->base != b->base) {
        return MATH_ERROR_INVALID_BASE;
    }
    
    /* Handle zero cases: lcm(0, x) = 0 */
    if (abacus_is_zero(a) || abacus_is_zero(b)) {
        result->num_beads = 0;
        result->negative = false;
        return MATH_SUCCESS;
    }
    
    CrystallineAbacus *gcd_val = abacus_new(a->base);
    CrystallineAbacus *product = abacus_new(a->base);
    CrystallineAbacus *remainder = abacus_new(a->base);
    
    if (!gcd_val || !product || !remainder) {
        abacus_free(gcd_val);
        abacus_free(product);
        abacus_free(remainder);
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Compute gcd(a, b) */
    MathError err = abacus_gcd(gcd_val, a, b);
    if (err != MATH_SUCCESS) {
        abacus_free(gcd_val);
        abacus_free(product);
        abacus_free(remainder);
        return err;
    }
    
    /* Compute a * b */
    err = abacus_mul(product, a, b);
    if (err != MATH_SUCCESS) {
        abacus_free(gcd_val);
        abacus_free(product);
        abacus_free(remainder);
        return err;
    }
    
    /* Compute (a * b) / gcd(a, b) */
    err = abacus_div(result, remainder, product, gcd_val);
    if (err != MATH_SUCCESS) {
        abacus_free(gcd_val);
        abacus_free(product);
        abacus_free(remainder);
        return err;
    }
    
    /* LCM is always positive */
    result->negative = false;
    
    abacus_free(gcd_val);
    abacus_free(product);
    abacus_free(remainder);
    
    return MATH_SUCCESS;
}

/**
 * @brief Test if two arbitrary precision numbers are coprime
 * 
 * Two numbers are coprime if gcd(a, b) = 1
 * 
 * @param a First number
 * @param b Second number
 * @return true if coprime, false otherwise
 */
bool abacus_coprime(const CrystallineAbacus* a, const CrystallineAbacus* b) {
    if (!a || !b) {
        return false;
    }
    
    if (a->base != b->base) {
        return false;
    }
    
    CrystallineAbacus *gcd_val = abacus_new(a->base);
    CrystallineAbacus *one = abacus_from_uint64(1, a->base);
    
    if (!gcd_val || !one) {
        abacus_free(gcd_val);
        abacus_free(one);
        return false;
    }
    
    /* Compute gcd(a, b) */
    MathError err = abacus_gcd(gcd_val, a, b);
    if (err != MATH_SUCCESS) {
        abacus_free(gcd_val);
        abacus_free(one);
        return false;
    }
    
    /* Check if gcd == 1 */
    int cmp = abacus_compare(gcd_val, one);
    
    abacus_free(gcd_val);
    abacus_free(one);
    
    return (cmp == 0);
}