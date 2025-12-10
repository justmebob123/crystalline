/**
 * @file bigfixed_arithmetic.c
 * @brief BigFixed arithmetic operations
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 */

#include "math/bigfixed.h"
#include "math/bigint.h"
#include "math/arithmetic.h"
#include <stdlib.h>

/* ============================================================================
 * BASIC ARITHMETIC
 * ============================================================================
 */

MathError bigfixed_add(BigFixed* result, const BigFixed* a, const BigFixed* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Ensure same scale */
    if (a->scale_bits != b->scale_bits) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Use double precision for now (simple implementation) */
    /* TODO: Implement proper fixed-point addition with carry handling */
    
    double val_a = bigfixed_to_double(a);
    double val_b = bigfixed_to_double(b);
    double sum = val_a + val_b;
    
    BigFixed* temp = bigfixed_from_double(sum, a->scale_bits);
    if (!temp) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Copy to result */
    bigint_free(result->integer_part);
    bigint_free(result->fractional_part);
    
    result->integer_part = bigint_copy(temp->integer_part);
    result->fractional_part = bigint_copy(temp->fractional_part);
    result->negative = temp->negative;
    result->scale_bits = temp->scale_bits;
    
    bigfixed_free(temp);
    
    return MATH_SUCCESS;
}

MathError bigfixed_sub(BigFixed* result, const BigFixed* a, const BigFixed* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* a - b = a + (-b) */
    BigFixed* neg_b = bigfixed_copy(b);
    if (!neg_b) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    neg_b->negative = !neg_b->negative;
    
    MathError err = bigfixed_add(result, a, neg_b);
    
    bigfixed_free(neg_b);
    
    return err;
}

MathError bigfixed_mul(BigFixed* result, const BigFixed* a, const BigFixed* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Convert to doubles for now (simple implementation) */
    /* TODO: Implement proper fixed-point multiplication */
    
    double val_a = bigfixed_to_double(a);
    double val_b = bigfixed_to_double(b);
    double product = val_a * val_b;
    
    BigFixed* temp = bigfixed_from_double(product, a->scale_bits);
    if (!temp) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Copy to result */
    bigint_free(result->integer_part);
    bigint_free(result->fractional_part);
    
    result->integer_part = bigint_copy(temp->integer_part);
    result->fractional_part = bigint_copy(temp->fractional_part);
    result->negative = temp->negative;
    result->scale_bits = temp->scale_bits;
    
    bigfixed_free(temp);
    
    return MATH_SUCCESS;
}

MathError bigfixed_div(BigFixed* result, const BigFixed* a, const BigFixed* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Check for division by zero */
    if (bigfixed_is_zero(b)) {
        return MATH_ERROR_DIVISION_BY_ZERO;
    }
    
    /* Convert to doubles for now (simple implementation) */
    /* TODO: Implement proper fixed-point division */
    
    double val_a = bigfixed_to_double(a);
    double val_b = bigfixed_to_double(b);
    double quotient = val_a / val_b;
    
    BigFixed* temp = bigfixed_from_double(quotient, a->scale_bits);
    if (!temp) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Copy to result */
    bigint_free(result->integer_part);
    bigint_free(result->fractional_part);
    
    result->integer_part = bigint_copy(temp->integer_part);
    result->fractional_part = bigint_copy(temp->fractional_part);
    result->negative = temp->negative;
    result->scale_bits = temp->scale_bits;
    
    bigfixed_free(temp);
    
    return MATH_SUCCESS;
}

MathError bigfixed_neg(BigFixed* result, const BigFixed* a) {
    if (!result || !a) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    if (result != a) {
        bigint_free(result->integer_part);
        bigint_free(result->fractional_part);
        
        result->integer_part = bigint_copy(a->integer_part);
        result->fractional_part = bigint_copy(a->fractional_part);
        result->scale_bits = a->scale_bits;
    }
    
    result->negative = !a->negative;
    
    /* Zero is always positive */
    if (bigfixed_is_zero(result)) {
        result->negative = false;
    }
    
    return MATH_SUCCESS;
}

MathError bigfixed_abs(BigFixed* result, const BigFixed* a) {
    if (!result || !a) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    if (result != a) {
        bigint_free(result->integer_part);
        bigint_free(result->fractional_part);
        
        result->integer_part = bigint_copy(a->integer_part);
        result->fractional_part = bigint_copy(a->fractional_part);
        result->scale_bits = a->scale_bits;
    }
    
    result->negative = false;
    
    return MATH_SUCCESS;
}