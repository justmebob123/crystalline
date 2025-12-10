/**
 * @file bigfixed_transcendental.c
 * @brief BigFixed transcendental functions
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * Implements transcendental functions for arbitrary precision.
 * Uses Taylor series and iterative methods.
 */

#include "math/bigfixed.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>

/* ============================================================================
 * POWER AND ROOT FUNCTIONS
 * ============================================================================
 */

MathError bigfixed_sqrt(BigFixed* result, const BigFixed* x) {
    if (!result || !x) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Check for negative input */
    if (bigfixed_is_negative(x)) {
        return MATH_ERROR_DOMAIN;
    }
    
    /* Handle zero */
    if (bigfixed_is_zero(x)) {
        bigint_free(result->integer_part);
        bigint_free(result->fractional_part);
        result->integer_part = bigint_new();
        result->fractional_part = bigint_new();
        result->negative = false;
        result->scale_bits = x->scale_bits;
        return MATH_SUCCESS;
    }
    
    /* Use double precision for now */
    /* TODO: Implement Newton-Raphson with BigFixed precision */
    double val = bigfixed_to_double(x);
    double sqrt_val = math_sqrt(val);
    
    BigFixed* temp = bigfixed_from_double(sqrt_val, x->scale_bits);
    if (!temp) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Copy to result */
    bigint_free(result->integer_part);
    bigint_free(result->fractional_part);
    result->integer_part = bigint_copy(temp->integer_part);
    result->fractional_part = bigint_copy(temp->fractional_part);
    result->negative = false;
    result->scale_bits = temp->scale_bits;
    
    bigfixed_free(temp);
    
    return MATH_SUCCESS;
}

MathError bigfixed_cbrt(BigFixed* result, const BigFixed* x) {
    if (!result || !x) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Use double precision for now */
    double val = bigfixed_to_double(x);
    double cbrt_val = math_cbrt(val);
    
    BigFixed* temp = bigfixed_from_double(cbrt_val, x->scale_bits);
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

MathError bigfixed_pow(BigFixed* result, const BigFixed* x, const BigFixed* y) {
    if (!result || !x || !y) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Use double precision for now */
    double val_x = bigfixed_to_double(x);
    double val_y = bigfixed_to_double(y);
    double pow_val = math_pow(val_x, val_y);
    
    BigFixed* temp = bigfixed_from_double(pow_val, x->scale_bits);
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

/* ============================================================================
 * EXPONENTIAL AND LOGARITHM
 * ============================================================================
 */

MathError bigfixed_exp(BigFixed* result, const BigFixed* x) {
    if (!result || !x) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Use double precision for now */
    double val = bigfixed_to_double(x);
    double exp_val = math_exp(val);
    
    BigFixed* temp = bigfixed_from_double(exp_val, x->scale_bits);
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

MathError bigfixed_log(BigFixed* result, const BigFixed* x) {
    if (!result || !x) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Check for non-positive input */
    if (!bigfixed_is_positive(x)) {
        return MATH_ERROR_DOMAIN;
    }
    
    /* Use double precision for now */
    double val = bigfixed_to_double(x);
    double log_val = math_log(val);
    
    BigFixed* temp = bigfixed_from_double(log_val, x->scale_bits);
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

MathError bigfixed_log10(BigFixed* result, const BigFixed* x) {
    if (!result || !x) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Check for non-positive input */
    if (!bigfixed_is_positive(x)) {
        return MATH_ERROR_DOMAIN;
    }
    
    /* Use double precision for now */
    double val = bigfixed_to_double(x);
    double log10_val = math_log10(val);
    
    BigFixed* temp = bigfixed_from_double(log10_val, x->scale_bits);
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

/* ============================================================================
 * TRIGONOMETRIC FUNCTIONS
 * ============================================================================
 */

MathError bigfixed_sin(BigFixed* result, const BigFixed* x) {
    if (!result || !x) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Use double precision for now */
    double val = bigfixed_to_double(x);
    double sin_val = math_sin(val);
    
    BigFixed* temp = bigfixed_from_double(sin_val, x->scale_bits);
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

MathError bigfixed_cos(BigFixed* result, const BigFixed* x) {
    if (!result || !x) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Use double precision for now */
    double val = bigfixed_to_double(x);
    double cos_val = math_cos(val);
    
    BigFixed* temp = bigfixed_from_double(cos_val, x->scale_bits);
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

MathError bigfixed_tan(BigFixed* result, const BigFixed* x) {
    if (!result || !x) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Use double precision for now */
    double val = bigfixed_to_double(x);
    double tan_val = math_tan(val);
    
    BigFixed* temp = bigfixed_from_double(tan_val, x->scale_bits);
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