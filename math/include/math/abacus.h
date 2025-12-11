/**
 * @file abacus.h
 * @brief Crystalline Abacus for geometric arithmetic operations
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * The Crystalline Abacus represents numbers geometrically on the clock lattice
 * and performs arithmetic operations using geometric transformations.
 * 
 * Revolutionary Approach:
 * - Numbers are represented as positions on the Babylonian clock
 * - Arithmetic operations become geometric transformations
 * - Leverages the crystalline lattice structure
 * - Self-contained geometric arithmetic system (no BigInt dependency)
 * 
 * Babylonian Mathematics - Universal Base Support:
 * - Supports ALL bases >= 2 (binary, octal, decimal, hexadecimal, etc.)
 * - Traditional Babylonian bases: 12, 60, 100 (clock rings)
 * - Base conversion without fractions (pure geometric transformation)
 * - Fractional support through negative weight exponents
 * 
 * Structure:
 * - Each "bead" is a position on the clock lattice
 * - Multiple beads represent multi-digit numbers
 * - Any base >= 2 supported (not limited to 12, 60, 100)
 * 
 * Example:
 *   Number 157 in base 12:
 *   157 = 13*12 + 1 = [13, 1] in base 12
 *   Represented as 2 beads at positions corresponding to 13 and 1
 */

#ifndef MATH_ABACUS_H
#define MATH_ABACUS_H

#include "math/types.h"
#include "math/clock.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ABACUS STRUCTURE
 * ============================================================================
 */

/**
 * @brief A bead on the crystalline abacus
 * 
 * Each bead represents a digit in the number, positioned on the clock lattice.
 * Supports both integer and fractional positions through weight_exponent.
 * 
 * Examples:
 *   weight_exponent = 2  → weight = base^2  (hundreds place)
 *   weight_exponent = 1  → weight = base^1  (tens place)
 *   weight_exponent = 0  → weight = base^0  (ones place)
 *   weight_exponent = -1 → weight = base^-1 (tenths place)
 *   weight_exponent = -2 → weight = base^-2 (hundredths place)
 */
typedef struct {
    ClockPosition position;  /**< Position on clock lattice */
    uint32_t value;          /**< Digit value (0 to base-1) */
    int32_t weight_exponent; /**< Weight exponent (can be negative for fractions) */
} AbacusBead;

/**
 * @brief Crystalline Abacus structure
 * 
 * Represents a number as a collection of beads on the clock lattice.
 * Supports both integer and fractional numbers through weight exponents.
 * 
 * Beads are stored in order from least significant to most significant.
 * The weight_exponent of each bead determines its position relative to
 * the decimal point.
 * 
 * Example: 157.25 in base 12
 *   beads[0]: value=3, weight_exponent=-1  (3 * 12^-1 = 0.25)
 *   beads[1]: value=1, weight_exponent=0   (1 * 12^0 = 1)
 *   beads[2]: value=13, weight_exponent=1  (13 * 12^1 = 156)
 *   Total: 156 + 1 + 0.25 = 157.25
 */
typedef struct {
    AbacusBead* beads;       /**< Array of beads (ordered by weight_exponent) */
    size_t num_beads;        /**< Number of beads */
    size_t capacity;         /**< Allocated capacity */
    uint32_t base;           /**< Number base (12, 60, or 100) */
    bool negative;           /**< Sign of the number */
    int32_t min_exponent;    /**< Minimum weight exponent (for fractional precision) */
} CrystallineAbacus;

/* ============================================================================
 * INITIALIZATION AND CLEANUP
 * ============================================================================
 */

/**
 * @brief Create a new crystalline abacus
 * @param base Number base (any base >= 2)
 *             Traditional Babylonian: 12, 60, 100
 *             Common: 2 (binary), 8 (octal), 10 (decimal), 16 (hex)
 *             Any positive integer >= 2 is supported
 * @return Pointer to new abacus, or NULL on error
 */
CrystallineAbacus* abacus_new(uint32_t base);

/**
 * @brief Free an abacus
 * @param abacus Abacus to free
 */
void abacus_free(CrystallineAbacus* abacus);

/**
 * @brief Initialize abacus to zero
 * @param abacus Abacus to initialize
 * @return MATH_SUCCESS or error code
 */
MathError abacus_init_zero(CrystallineAbacus* abacus);

/* ============================================================================
 * CONVERSION
 * ============================================================================
 */

/**
 * @brief Create abacus from uint64
 * @param value Value to convert
 * @param base Number base (any base >= 2)
 * @return Pointer to new abacus, or NULL on error
 */
CrystallineAbacus* abacus_from_uint64(uint64_t value, uint32_t base);

/**
 * @brief Convert abacus to uint64
 * @param abacus Abacus to convert
 * @param value Output value
 * @return MATH_SUCCESS or error code
 */
MathError abacus_to_uint64(const CrystallineAbacus* abacus, uint64_t* value);

/**
 * @brief Create abacus from double (with fractional part)
 * @param value Value to convert
 * @param base Number base (any base >= 2)
 * @param precision Number of fractional digits (negative exponents)
 * @return Pointer to new abacus, or NULL on error
 * 
 * Example: abacus_from_double(157.25, 12, 2)
 *   Creates abacus with beads for 157.25 in base 12
 *   precision=2 means 2 fractional digits (12^-1, 12^-2)
 */
CrystallineAbacus* abacus_from_double(double value, uint32_t base, int32_t precision);

/**
 * @brief Convert abacus to double
 * @param abacus Abacus to convert
 * @param value Output value
 * @return MATH_SUCCESS or error code
 */
MathError abacus_to_double(const CrystallineAbacus* abacus, double* value);

/**
 * @brief Set precision (number of fractional digits)
 * @param abacus Abacus to modify
 * @param precision Number of fractional digits (negative exponents)
 * @return MATH_SUCCESS or error code
 * 
 * Adjusts min_exponent to support the specified precision.
 * Pads with zeros if needed, truncates if reducing precision.
 */
MathError abacus_set_precision(CrystallineAbacus* abacus, int32_t precision);

/**
 * @brief Get precision (number of fractional digits)
 * @param abacus Abacus to query
 * @return Number of fractional digits (0 if integer only)
 */
int32_t abacus_get_precision(const CrystallineAbacus* abacus);

/**
 * @brief Round abacus to specified precision
 * @param result Output abacus
 * @param a Input abacus
 * @param precision Number of fractional digits to keep
 * @return MATH_SUCCESS or error code
 */
MathError abacus_round(CrystallineAbacus* result, const CrystallineAbacus* a, int32_t precision);

/**
 * @brief Truncate abacus to specified precision
 * @param result Output abacus
 * @param a Input abacus
 * @param precision Number of fractional digits to keep
 * @return MATH_SUCCESS or error code
 */
MathError abacus_truncate(CrystallineAbacus* result, const CrystallineAbacus* a, int32_t precision);

/* ============================================================================
 * GEOMETRIC ARITHMETIC OPERATIONS
 * ============================================================================
 */

/**
 * @brief Add two abacuses (geometric addition)
 * @param result Output abacus (must be pre-allocated)
 * @param a First abacus
 * @param b Second abacus
 * @return MATH_SUCCESS or error code
 * 
 * Performs addition using geometric transformations on the clock lattice.
 */
MathError abacus_add(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b);

/**
 * @brief Subtract two abacuses (geometric subtraction)
 * @param result Output abacus (must be pre-allocated)
 * @param a First abacus
 * @param b Second abacus
 * @return MATH_SUCCESS or error code
 */
MathError abacus_sub(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b);

/**
 * @brief Multiply two abacuses (geometric multiplication)
 * @param result Output abacus (must be pre-allocated)
 * @param a First abacus
 * @param b Second abacus
 * @return MATH_SUCCESS or error code
 */
MathError abacus_mul(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b);

/**
 * @brief Divide two abacuses (geometric division)
 * @param quotient Output quotient (must be pre-allocated)
 * @param remainder Output remainder (must be pre-allocated, can be NULL)
 * @param a Dividend
 * @param b Divisor
 * @return MATH_SUCCESS or error code
 */
MathError abacus_div(CrystallineAbacus* quotient, CrystallineAbacus* remainder,
                     const CrystallineAbacus* a, const CrystallineAbacus* b);

/**
 * @brief Shift abacus left (multiply by base^n)
 * @param result Output abacus (must be pre-allocated)
 * @param a Input abacus
 * @param n Number of positions to shift
 * @return MATH_SUCCESS or error code
 * 
 * Geometric operation: rotates positions on clock lattice.
 */
MathError abacus_shift_left(CrystallineAbacus* result, const CrystallineAbacus* a, size_t n);

/**
 * @brief Shift abacus right (divide by base^n)
 * @param result Output abacus (must be pre-allocated)
 * @param a Input abacus
 * @param n Number of positions to shift
 * @return MATH_SUCCESS or error code
 */
MathError abacus_shift_right(CrystallineAbacus* result, const CrystallineAbacus* a, size_t n);

/* ============================================================================
 * COMPARISON
 * ============================================================================
 */

/**
 * @brief Compare two abacuses
 * @param a First abacus
 * @param b Second abacus
 * @return -1 if a < b, 0 if a == b, 1 if a > b
 */
int abacus_compare(const CrystallineAbacus* a, const CrystallineAbacus* b);

/**
 * @brief Check if abacus is zero
 * @param abacus Abacus to check
 * @return true if zero
 */
bool abacus_is_zero(const CrystallineAbacus* abacus);

/**
 * @brief Check if abacus is negative
 * @param abacus Abacus to check
 * @return true if negative
 */
bool abacus_is_negative(const CrystallineAbacus* abacus);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Copy an abacus
 * @param src Source abacus
 * @return Pointer to new abacus, or NULL on error
 */
CrystallineAbacus* abacus_copy(const CrystallineAbacus* src);

/**
 * @brief Normalize abacus (remove leading zeros, handle carries)
 * @param abacus Abacus to normalize
 * @return MATH_SUCCESS or error code
 */
MathError abacus_normalize(CrystallineAbacus* abacus);

/**
 * @brief Get string representation of abacus
 * @param abacus Abacus to convert
 * @return String representation (caller must free), or NULL on error
 */
char* abacus_to_string(const CrystallineAbacus* abacus);

/**
 * @brief Print abacus for debugging
 * @param abacus Abacus to print
 */
void abacus_print(const CrystallineAbacus* abacus);

/* ============================================================================
 * MODULAR ARITHMETIC OPERATIONS
 * ============================================================================ */

/**
 * @brief Modular reduction (a mod m)
 * @param result Output abacus (must be pre-allocated)
 * @param a Input abacus
 * @param modulus Modulus
 * @return MATH_SUCCESS on success, error code on failure
 */
MathError abacus_mod(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* modulus);

/**
 * @brief Modular addition ((a + b) mod m)
 * @param result Output abacus (must be pre-allocated)
 * @param a First abacus
 * @param b Second abacus
 * @param modulus Modulus
 * @return MATH_SUCCESS on success, error code on failure
 */
MathError abacus_mod_add(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b, const CrystallineAbacus* modulus);

/**
 * @brief Modular subtraction ((a - b) mod m)
 * @param result Output abacus (must be pre-allocated)
 * @param a First abacus
 * @param b Second abacus
 * @param modulus Modulus
 * @return MATH_SUCCESS on success, error code on failure
 */
MathError abacus_mod_sub(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b, const CrystallineAbacus* modulus);

/**
 * @brief Modular multiplication ((a * b) mod m)
 * @param result Output abacus (must be pre-allocated)
 * @param a First abacus
 * @param b Second abacus
 * @param modulus Modulus
 * @return MATH_SUCCESS on success, error code on failure
 */
MathError abacus_mod_mul(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b, const CrystallineAbacus* modulus);

/**
 * @brief Modular exponentiation ((base ^ exponent) mod m)
 * Uses binary exponentiation for efficiency
 * @param result Output abacus (must be pre-allocated)
 * @param base Base
 * @param exponent Exponent
 * @param modulus Modulus
 * @return MATH_SUCCESS on success, error code on failure
 */
MathError abacus_mod_exp(CrystallineAbacus* result, const CrystallineAbacus* base, const CrystallineAbacus* exponent, const CrystallineAbacus* modulus);

/**
 * @brief Modular multiplicative inverse (a^-1 mod m)
 * Uses extended Euclidean algorithm
 * @param result Output abacus (must be pre-allocated)
 * @param a Input abacus
 * @param modulus Modulus
 * @return MATH_SUCCESS on success, error code on failure
 */
MathError abacus_mod_inverse(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* modulus);

/* ============================================================================
 * BASE CONVERSION
 * ============================================================================ */

/**
 * @brief Convert abacus to a different base
 * 
 * Babylonian mathematics supports conversion between ANY bases without
 * requiring fractions. This is a pure geometric transformation on the
 * clock lattice.
 * 
 * @param result Output abacus in new base (will be created)
 * @param source Input abacus in original base
 * @param new_base Target base (any base >= 2)
 * @return MATH_SUCCESS on success, error code on failure
 * 
 * Example:
 *   Convert 157 from base 12 to base 10:
 *   Input:  [13, 1] in base 12 = 13*12 + 1 = 157
 *   Output: [1, 5, 7] in base 10 = 1*100 + 5*10 + 7 = 157
 */
MathError abacus_convert_base(CrystallineAbacus** result, const CrystallineAbacus* source, uint32_t new_base);

/**
 * @brief Get the base of an abacus
 * @param abacus Input abacus
 * @return Base of the abacus, or 0 on error
 */
uint32_t abacus_get_base(const CrystallineAbacus* abacus);

#ifdef __cplusplus
}
#endif

#endif /* MATH_ABACUS_H */
