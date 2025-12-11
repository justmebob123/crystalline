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
 * - Provides alternative representation for BigInt/BigFixed
 * 
 * Structure:
 * - Each "bead" is a position on the clock lattice
 * - Multiple beads represent multi-digit numbers
 * - Base can be 12, 60, or 100 (matching clock rings)
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
#include "math/bigint.h"
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
 */
typedef struct {
    ClockPosition position;  /**< Position on clock lattice */
    uint32_t value;          /**< Digit value (0 to base-1) */
    uint32_t weight;         /**< Position weight (base^index) */
} AbacusBead;

/**
 * @brief Crystalline Abacus structure
 * 
 * Represents a number as a collection of beads on the clock lattice.
 */
typedef struct {
    AbacusBead* beads;       /**< Array of beads */
    size_t num_beads;        /**< Number of beads */
    size_t capacity;         /**< Allocated capacity */
    uint32_t base;           /**< Number base (12, 60, or 100) */
    bool negative;           /**< Sign of the number */
} CrystallineAbacus;

/* ============================================================================
 * INITIALIZATION AND CLEANUP
 * ============================================================================
 */

/**
 * @brief Create a new crystalline abacus
 * @param base Number base (12, 60, or 100)
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
 * @brief Create abacus from BigInt
 * @param bi BigInt to convert
 * @param base Number base (12, 60, or 100)
 * @return Pointer to new abacus, or NULL on error
 */
CrystallineAbacus* abacus_from_bigint(const BigInt* bi, uint32_t base);

/**
 * @brief Convert abacus to BigInt
 * @param abacus Abacus to convert
 * @return Pointer to new BigInt, or NULL on error
 */
BigInt* abacus_to_bigint(const CrystallineAbacus* abacus);

/**
 * @brief Create abacus from uint64
 * @param value Value to convert
 * @param base Number base (12, 60, or 100)
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

#ifdef __cplusplus
}
#endif

#endif /* MATH_ABACUS_H */