/**
 * @file bigfixed.h
 * @brief Arbitrary precision fixed-point arithmetic
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * This module provides arbitrary precision fixed-point arithmetic.
 * BigFixed maintains separate integer and fractional parts, both
 * with unlimited precision.
 * 
 * Use BigFixed when you need:
 * - More precision than double (53 bits)
 * - Exact decimal arithmetic
 * - Transcendental functions with high precision
 * 
 * Representation: value = (integer_part + fractional_part / 2^scale_bits) * sign
 */

#ifndef MATH_BIGFIXED_H
#define MATH_BIGFIXED_H

#include "types.h"
#include "bigint.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CREATION AND DESTRUCTION
 * ============================================================================
 */

/**
 * @brief Create new BigFixed with specified precision
 * @param scale_bits Number of fractional bits (precision)
 * @return Pointer to new BigFixed, or NULL on error
 * 
 * Typical values:
 * - 64 bits: ~19 decimal places
 * - 128 bits: ~38 decimal places
 * - 256 bits: ~77 decimal places (default)
 * - 512 bits: ~154 decimal places
 */
BigFixed* bigfixed_new(size_t scale_bits);

/**
 * @brief Free BigFixed
 * @param bf BigFixed to free
 */
void bigfixed_free(BigFixed* bf);

/**
 * @brief Create copy of BigFixed
 * @param bf BigFixed to copy
 * @return Pointer to new BigFixed copy
 */
BigFixed* bigfixed_copy(const BigFixed* bf);

/* ============================================================================
 * CONVERSION FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Create BigFixed from double
 * @param value Double value
 * @param scale_bits Precision in bits
 * @return Pointer to new BigFixed
 */
BigFixed* bigfixed_from_double(double value, size_t scale_bits);

/**
 * @brief Convert BigFixed to double
 * @param bf BigFixed value
 * @return Approximate double value
 * @note May lose precision if BigFixed has more than 53 bits
 */
double bigfixed_to_double(const BigFixed* bf);

/**
 * @brief Create BigFixed from integer
 * @param value Integer value
 * @param scale_bits Precision in bits
 * @return Pointer to new BigFixed
 */
BigFixed* bigfixed_from_int64(int64_t value, size_t scale_bits);

/**
 * @brief Convert BigFixed to integer (truncated)
 * @param bf BigFixed value
 * @return Integer part
 */
int64_t bigfixed_to_int64(const BigFixed* bf);

/**
 * @brief Create BigFixed from string
 * @param str String representation (e.g., "3.14159")
 * @param scale_bits Precision in bits
 * @return Pointer to new BigFixed, or NULL on parse error
 */
BigFixed* bigfixed_from_string(const char* str, size_t scale_bits);

/**
 * @brief Convert BigFixed to string
 * @param bf BigFixed value
 * @param decimal_places Number of decimal places to display
 * @return Dynamically allocated string (caller must free)
 */
char* bigfixed_to_string(const BigFixed* bf, int decimal_places);

/* ============================================================================
 * COMPARISON FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Compare two BigFixed values
 * @param a First value
 * @param b Second value
 * @return -1 if a < b, 0 if a == b, 1 if a > b
 */
int bigfixed_cmp(const BigFixed* a, const BigFixed* b);

/**
 * @brief Check if BigFixed is zero
 * @param bf BigFixed value
 * @return true if bf == 0
 */
bool bigfixed_is_zero(const BigFixed* bf);

/**
 * @brief Check if BigFixed is negative
 * @param bf BigFixed value
 * @return true if bf < 0
 */
bool bigfixed_is_negative(const BigFixed* bf);

/**
 * @brief Check if BigFixed is positive
 * @param bf BigFixed value
 * @return true if bf > 0
 */
bool bigfixed_is_positive(const BigFixed* bf);

/* ============================================================================
 * ARITHMETIC OPERATIONS
 * ============================================================================
 */

/**
 * @brief Add two BigFixed values
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_add(BigFixed* result, const BigFixed* a, const BigFixed* b);

/**
 * @brief Subtract two BigFixed values
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_sub(BigFixed* result, const BigFixed* a, const BigFixed* b);

/**
 * @brief Multiply two BigFixed values
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_mul(BigFixed* result, const BigFixed* a, const BigFixed* b);

/**
 * @brief Divide two BigFixed values
 * @param result Output (must be pre-allocated)
 * @param a Numerator
 * @param b Denominator
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_div(BigFixed* result, const BigFixed* a, const BigFixed* b);

/**
 * @brief Absolute value
 * @param result Output (must be pre-allocated)
 * @param bf Input value
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_abs(BigFixed* result, const BigFixed* bf);

/**
 * @brief Negate value
 * @param result Output (must be pre-allocated)
 * @param bf Input value
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_neg(BigFixed* result, const BigFixed* bf);

/* ============================================================================
 * TRANSCENDENTAL FUNCTIONS (ARBITRARY PRECISION)
 * ============================================================================
 */

/**
 * @brief Square root (arbitrary precision)
 * @param result Output (must be pre-allocated)
 * @param x Input value (must be >= 0)
 * @return MATH_SUCCESS or error code
 * 
 * Implementation: Newton-Raphson with BigFixed arithmetic
 */
MathError bigfixed_sqrt(BigFixed* result, const BigFixed* x);

/**
 * @brief Exponential function (arbitrary precision)
 * @param result Output (must be pre-allocated)
 * @param x Input value
 * @return MATH_SUCCESS or error code
 * 
 * Implementation: Taylor series with BigFixed arithmetic
 */
MathError bigfixed_exp(BigFixed* result, const BigFixed* x);

/**
 * @brief Natural logarithm (arbitrary precision)
 * @param result Output (must be pre-allocated)
 * @param x Input value (must be > 0)
 * @return MATH_SUCCESS or error code
 * 
 * Implementation: Taylor series with BigFixed arithmetic
 */
MathError bigfixed_log(BigFixed* result, const BigFixed* x);

/**
 * @brief Sine function (arbitrary precision)
 * @param result Output (must be pre-allocated)
 * @param x Angle in radians
 * @return MATH_SUCCESS or error code
 * 
 * Implementation: Taylor series with BigFixed arithmetic
 */
MathError bigfixed_sin(BigFixed* result, const BigFixed* x);

/**
 * @brief Cosine function (arbitrary precision)
 * @param result Output (must be pre-allocated)
 * @param x Angle in radians
 * @return MATH_SUCCESS or error code
 * 
 * Implementation: Taylor series with BigFixed arithmetic
 */
MathError bigfixed_cos(BigFixed* result, const BigFixed* x);

/**
 * @brief Power function (arbitrary precision)
 * @param result Output (must be pre-allocated)
 * @param x Base
 * @param y Exponent
 * @return MATH_SUCCESS or error code
 * 
 * Implementation: exp(y * log(x)) with BigFixed arithmetic
 */
MathError bigfixed_pow(BigFixed* result, const BigFixed* x, const BigFixed* y);

/* ============================================================================
 * ROUNDING FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Floor function (arbitrary precision)
 * @param result Output BigInt (must be pre-allocated)
 * @param bf Input BigFixed
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_floor(BigInt* result, const BigFixed* bf);

/**
 * @brief Ceiling function (arbitrary precision)
 * @param result Output BigInt (must be pre-allocated)
 * @param bf Input BigFixed
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_ceil(BigInt* result, const BigFixed* bf);

/**
 * @brief Round to nearest integer (arbitrary precision)
 * @param result Output BigInt (must be pre-allocated)
 * @param bf Input BigFixed
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_round(BigInt* result, const BigFixed* bf);

/**
 * @brief Truncate to integer (arbitrary precision)
 * @param result Output BigInt (must be pre-allocated)
 * @param bf Input BigFixed
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_trunc(BigInt* result, const BigFixed* bf);

/**
 * @brief Get fractional part
 * @param result Output BigFixed (must be pre-allocated)
 * @param bf Input BigFixed
 * @return MATH_SUCCESS or error code
 */
MathError bigfixed_frac(BigFixed* result, const BigFixed* bf);

#ifdef __cplusplus
}
#endif

#endif /* MATH_BIGFIXED_H */