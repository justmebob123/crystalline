/**
 * @file bigint.h
 * @brief Arbitrary precision integer arithmetic
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * This module provides unlimited precision integer arithmetic.
 * BigInt uses a dynamic array of 32-bit digits for unlimited range.
 * 
 * Use BigInt when you need:
 * - Integer operations beyond int64_t range
 * - Cryptographic operations
 * - Number theory algorithms
 * - Exact integer arithmetic
 */

#ifndef MATH_BIGINT_H
#define MATH_BIGINT_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CREATION AND DESTRUCTION
 * ============================================================================
 */

/**
 * @brief Create new BigInt (initialized to zero)
 * @return Pointer to new BigInt, or NULL on error
 */
BigInt* bigint_new(void);

/**
 * @brief Free BigInt
 * @param bi BigInt to free
 */
void bigint_free(BigInt* bi);

/**
 * @brief Create copy of BigInt
 * @param bi BigInt to copy
 * @return Pointer to new BigInt copy
 */
BigInt* bigint_copy(const BigInt* bi);

/**
 * @brief Initialize BigInt (for stack-allocated BigInt)
 * @param bi BigInt to initialize
 */
void bigint_init(BigInt* bi);

/**
 * @brief Clear BigInt contents (for stack-allocated BigInt)
 * @param bi BigInt to clear
 */
void bigint_clear(BigInt* bi);

/* ============================================================================
 * CONVERSION FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Create BigInt from int64
 * @param value Integer value
 * @return Pointer to new BigInt
 */
BigInt* bigint_from_int64(int64_t value);

/**
 * @brief Convert BigInt to int64
 * @param bi BigInt value
 * @return int64 value (truncated if too large)
 */
int64_t bigint_to_int64(const BigInt* bi);

/**
 * @brief Create BigInt from uint64
 * @param value Unsigned integer value
 * @return Pointer to new BigInt
 */
BigInt* bigint_from_uint64(uint64_t value);

/**
 * @brief Convert BigInt to uint64
 * @param bi BigInt value
 * @return uint64 value (truncated if too large)
 */
uint64_t bigint_to_uint64(const BigInt* bi);

/**
 * @brief Create BigInt from hexadecimal string
 * @param hex Hexadecimal string (e.g., "1a2b3c")
 * @return Pointer to new BigInt, or NULL on parse error
 */
BigInt* bigint_from_hex(const char* hex);

/**
 * @brief Convert BigInt to hexadecimal string
 * @param bi BigInt value
 * @return Dynamically allocated string (caller must free)
 */
char* bigint_to_hex(const BigInt* bi);

/**
 * @brief Create BigInt from decimal string
 * @param dec Decimal string (e.g., "12345")
 * @return Pointer to new BigInt, or NULL on parse error
 */
BigInt* bigint_from_dec(const char* dec);

/**
 * @brief Convert BigInt to decimal string
 * @param bi BigInt value
 * @return Dynamically allocated string (caller must free)
 */
char* bigint_to_dec(const BigInt* bi);

/**
 * @brief Create BigInt from binary data
 * @param data Binary data (big-endian)
 * @param len Length of data in bytes
 * @return Pointer to new BigInt
 */
BigInt* bigint_from_bin(const uint8_t* data, size_t len);

/**
 * @brief Convert BigInt to binary data
 * @param bi BigInt value
 * @param out Output buffer
 * @param len Length of output buffer
 * @return Number of bytes written, or 0 on error
 */
size_t bigint_to_bin(const BigInt* bi, uint8_t* out, size_t len);

/* ============================================================================
 * COMPARISON FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Compare two BigInt values
 * @param a First value
 * @param b Second value
 * @return -1 if a < b, 0 if a == b, 1 if a > b
 */
int bigint_cmp(const BigInt* a, const BigInt* b);

/**
 * @brief Check if BigInt is zero
 * @param bi BigInt value
 * @return true if bi == 0
 */
bool bigint_is_zero(const BigInt* bi);

/**
 * @brief Check if BigInt is one
 * @param bi BigInt value
 * @return true if bi == 1
 */
bool bigint_is_one(const BigInt* bi);

/**
 * @brief Check if BigInt is negative
 * @param bi BigInt value
 * @return true if bi < 0
 */
bool bigint_is_negative(const BigInt* bi);

/**
 * @brief Check if BigInt is even
 * @param bi BigInt value
 * @return true if bi is even
 */
bool bigint_is_even(const BigInt* bi);

/**
 * @brief Check if BigInt is odd
 * @param bi BigInt value
 * @return true if bi is odd
 */
bool bigint_is_odd(const BigInt* bi);

/* ============================================================================
 * ARITHMETIC OPERATIONS
 * ============================================================================
 */

/**
 * @brief Add two BigInt values
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @return MATH_SUCCESS or error code
 */
MathError bigint_add(BigInt* result, const BigInt* a, const BigInt* b);

/**
 * @brief Subtract two BigInt values
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @return MATH_SUCCESS or error code
 */
MathError bigint_sub(BigInt* result, const BigInt* a, const BigInt* b);

/**
 * @brief Multiply two BigInt values
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @return MATH_SUCCESS or error code
 */
MathError bigint_mul(BigInt* result, const BigInt* a, const BigInt* b);

/**
 * @brief Divide two BigInt values
 * @param quotient Output quotient (must be pre-allocated)
 * @param remainder Output remainder (must be pre-allocated, can be NULL)
 * @param a Dividend
 * @param b Divisor
 * @return MATH_SUCCESS or error code
 */
MathError bigint_div(BigInt* quotient, BigInt* remainder, const BigInt* a, const BigInt* b);

/**
 * @brief Modulo operation
 * @param result Output (must be pre-allocated)
 * @param a Dividend
 * @param m Modulus
 * @return MATH_SUCCESS or error code
 */
MathError bigint_mod(BigInt* result, const BigInt* a, const BigInt* m);

/**
 * @brief Absolute value
 * @param result Output (must be pre-allocated)
 * @param bi Input value
 * @return MATH_SUCCESS or error code
 */
MathError bigint_abs(BigInt* result, const BigInt* bi);

/**
 * @brief Negate value
 * @param result Output (must be pre-allocated)
 * @param bi Input value
 * @return MATH_SUCCESS or error code
 */
MathError bigint_neg(BigInt* result, const BigInt* bi);

/* ============================================================================
 * BIT OPERATIONS
 * ============================================================================
 */

/**
 * @brief Left shift
 * @param result Output (must be pre-allocated)
 * @param bi Input value
 * @param bits Number of bits to shift
 * @return MATH_SUCCESS or error code
 */
MathError bigint_lshift(BigInt* result, const BigInt* bi, size_t bits);

/**
 * @brief Right shift
 * @param result Output (must be pre-allocated)
 * @param bi Input value
 * @param bits Number of bits to shift
 * @return MATH_SUCCESS or error code
 */
MathError bigint_rshift(BigInt* result, const BigInt* bi, size_t bits);

/**
 * @brief Get bit at position
 * @param bi BigInt value
 * @param bit_pos Bit position (0 = LSB)
 * @return 0 or 1
 */
int bigint_get_bit(const BigInt* bi, size_t bit_pos);

/**
 * @brief Set bit at position
 * @param bi BigInt value
 * @param bit_pos Bit position (0 = LSB)
 * @param value 0 or 1
 * @return MATH_SUCCESS or error code
 */
MathError bigint_set_bit(BigInt* bi, size_t bit_pos, int value);

/**
 * @brief Count number of bits
 * @param bi BigInt value
 * @return Number of bits in binary representation
 */
size_t bigint_num_bits(const BigInt* bi);

/* ============================================================================
 * MODULAR ARITHMETIC
 * ============================================================================
 */

/**
 * @brief Modular addition
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @param m Modulus
 * @return MATH_SUCCESS or error code
 */
MathError bigint_mod_add(BigInt* result, const BigInt* a, const BigInt* b, const BigInt* m);

/**
 * @brief Modular subtraction
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @param m Modulus
 * @return MATH_SUCCESS or error code
 */
MathError bigint_mod_sub(BigInt* result, const BigInt* a, const BigInt* b, const BigInt* m);

/**
 * @brief Modular multiplication
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @param m Modulus
 * @return MATH_SUCCESS or error code
 */
MathError bigint_mod_mul(BigInt* result, const BigInt* a, const BigInt* b, const BigInt* m);

/**
 * @brief Modular exponentiation
 * @param result Output (must be pre-allocated)
 * @param base Base
 * @param exponent Exponent
 * @param modulus Modulus
 * @return MATH_SUCCESS or error code
 * 
 * Computes base^exponent mod modulus efficiently
 * Implementation: Binary exponentiation
 */
MathError bigint_mod_exp(BigInt* result, const BigInt* base, const BigInt* exponent, const BigInt* modulus);

/**
 * @brief Modular inverse
 * @param result Output (must be pre-allocated)
 * @param a Value to invert
 * @param m Modulus
 * @return MATH_SUCCESS or error code
 * 
 * Computes a^(-1) mod m (multiplicative inverse)
 * Implementation: Extended Euclidean algorithm
 */
MathError bigint_mod_inverse(BigInt* result, const BigInt* a, const BigInt* m);

/* ============================================================================
 * NUMBER THEORY
 * ============================================================================
 */

/**
 * @brief Greatest common divisor
 * @param result Output (must be pre-allocated)
 * @param a First value
 * @param b Second value
 * @return MATH_SUCCESS or error code
 * 
 * Implementation: Binary GCD algorithm
 */
MathError bigint_gcd(BigInt* result, const BigInt* a, const BigInt* b);

/**
 * @brief Least common multiple
 * @param result Output (must be pre-allocated)
 * @param a First value
 * @param b Second value
 * @return MATH_SUCCESS or error code
 */
MathError bigint_lcm(BigInt* result, const BigInt* a, const BigInt* b);

/**
 * @brief Extended Euclidean algorithm
 * @param gcd Output GCD (must be pre-allocated)
 * @param x Output coefficient x (can be NULL)
 * @param y Output coefficient y (can be NULL)
 * @param a First value
 * @param b Second value
 * @return MATH_SUCCESS or error code
 * 
 * Computes gcd(a, b) and coefficients x, y such that ax + by = gcd(a, b)
 */
MathError bigint_extended_gcd(BigInt* gcd, BigInt* x, BigInt* y, const BigInt* a, const BigInt* b);

#ifdef __cplusplus
}
#endif

#endif /* MATH_BIGINT_H */