#ifndef CLLM_MATHEMATICAL_CONSTANTS_H
#define CLLM_MATHEMATICAL_CONSTANTS_H

#include <stdint.h>
#include "bigfixed_core.h"

/**
 * Mathematical Constants for Crystalline Lattice Language Model
 * 
 * These constants are IMMUTABLE and represent fundamental mathematical
 * relationships in the prime lattice structure.
 */

// ============================================================================
// CRITICAL CONSTANTS - DO NOT MODIFY
// ============================================================================

/**
 * Vector Culmination Point
 * 144000 = 3 × 12³ × (250/9)
 * This is the point where dimensional vectors culminate
 * Note: Also defined in prime_types.h - use include guards to avoid redefinition
 */
#ifndef VECTOR_CULMINATION
#define VECTOR_CULMINATION 144000ULL
#endif

/**
 * Twin Prime Boundaries
 * These primes bracket the vector culmination point
 * Note: Also defined in prime_types.h - use include guards to avoid redefinition
 */
#ifndef TWIN_PRIME_LOWER
#define TWIN_PRIME_LOWER 143999ULL  // Prime below 144000
#endif
#ifndef TWIN_PRIME_UPPER
#define TWIN_PRIME_UPPER 144001ULL  // Prime above 144000
#endif

/**
 * Einstein's Lambda (Λ) Correction
 * Λ = 3/144000 = 0.0000208333...
 * 
 * This correction factor appears in angular position calculations
 * and represents the cosmological constant in our lattice geometry.
 */
#define EINSTEIN_LAMBDA_NUMERATOR 3
#define EINSTEIN_LAMBDA_DENOMINATOR 144000

// High-precision representation (10 decimal places)
#define EINSTEIN_LAMBDA_DOUBLE 0.0000208333333333

/**
 * Symmetry Group Count
 * The lattice is partitioned into exactly 12 symmetry groups
 * based on p mod 12 for primes p
 */
#define SYMMETRY_GROUP_COUNT 12

/**
 * Base for Recursive Growth
 * Each level of hierarchy grows by factor of 3
 * Levels: 1, 3, 9, 27, 81, ...
 */
#define RECURSIVE_BASE 3

/**
 * Kissing Spheres Pattern
 * Central sphere surrounded by 12 equal spheres
 */
#define KISSING_SPHERES_COUNT 12

// ============================================================================
// DIMENSIONAL FREQUENCIES (φᵢ)
// ============================================================================

/**
 * Dimensional Frequencies
 * These are the fundamental frequencies that define each dimension
 * φᵢ = [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, ...]
 */
extern const uint64_t DIMENSIONAL_FREQUENCIES[];
extern const size_t DIMENSIONAL_FREQUENCIES_COUNT;

// First 12 dimensional frequencies (most commonly used)
#define PHI_0  3
#define PHI_1  7
#define PHI_2  31
#define PHI_3  12
#define PHI_4  19
#define PHI_5  5
#define PHI_6  11
#define PHI_7  13
#define PHI_8  17
#define PHI_9  23
#define PHI_10 29
#define PHI_11 37

// ============================================================================
// CYMATIC FREQUENCIES
// ============================================================================

/**
 * Cymatic Frequencies (Hz)
 * These frequencies correspond to harmonic resonances in the lattice
 */
extern const double CYMATIC_FREQUENCIES[];
extern const size_t CYMATIC_FREQUENCIES_COUNT;

// Common cymatic frequencies
#define CYMATIC_432_HZ  432.0  // Universal frequency
#define CYMATIC_528_HZ  528.0  // DNA repair frequency
#define CYMATIC_639_HZ  639.0  // Connection frequency
#define CYMATIC_741_HZ  741.0  // Awakening frequency
#define CYMATIC_852_HZ  852.0  // Intuition frequency
#define CYMATIC_963_HZ  963.0  // Divine frequency

// ============================================================================
// ANGULAR POSITION CONSTANTS
// ============================================================================

/**
 * Golden Ratio (φ)
 * φ = (1 + √5) / 2 ≈ 1.618033988749895
 * Note: Also defined in prime_types.h - use include guards to avoid redefinition
 */
#ifndef GOLDEN_RATIO
#define GOLDEN_RATIO 1.618033988749894848204586834365638117720309179805762862135
#endif

#ifndef PHI
#define PHI GOLDEN_RATIO
#endif

/** Golden ratio squared: φ² */
#ifndef PHI_SQUARED
#define PHI_SQUARED 2.618033988749895
#endif

/** Golden ratio inverse: 1/φ = φ - 1 */
#ifndef PHI_INVERSE
#define PHI_INVERSE 0.618033988749895
#endif

/** Golden ratio cubed: φ³ */
#ifndef PHI_CUBED
#define PHI_CUBED 4.236067977499790
#endif

/**
 * Natural Logarithm of 3
 * ln(3) ≈ 1.0986122886681098
 */
#define LN_3 1.0986122886681096613952891730059917642046077108666

/** Natural logarithm of 2: ln(2) */
#ifndef LN_2
#define LN_2 0.69314718055994530942
#endif

/** Natural logarithm of 10: ln(10) */
#ifndef LN_10
#define LN_10 2.30258509299404568402
#endif

/** Euler's number: e */
#ifndef E
#define E 2.71828182845904523536
#endif

/**
 * Pi (π)
 * π ≈ 3.14159265358979323846
 */
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419716939937510
#endif

#ifndef PI
#define PI M_PI
#endif

/** Two pi: 2π */
#ifndef TWO_PI
#define TWO_PI 6.28318530717958647692
#endif

/** Half pi: π/2 */
#ifndef HALF_PI
#define HALF_PI 1.57079632679489661923
#endif

/** Pi squared: π² */
#ifndef PI_SQUARED
#define PI_SQUARED 9.86960440108935861883
#endif

// ============================================================================
// PLIMPTON 322 CONSTANTS
// ============================================================================

/**
 * Plimpton 322 Triple System
 * Ancient Babylonian mathematical tablet encoding Pythagorean triples
 * Used for ψ correction in angular position
 */
typedef struct {
    uint64_t a;  // First element of triple
    uint64_t b;  // Second element of triple
    uint64_t c;  // Hypotenuse
} PlimptonTriple;

extern const PlimptonTriple PLIMPTON_322_TRIPLES[];
extern const size_t PLIMPTON_322_TRIPLES_COUNT;

// ============================================================================
// BIGFIXED REPRESENTATIONS
// ============================================================================

/**
 * High-precision BigFixed representations of key constants
 * These are initialized at runtime for maximum precision
 */
typedef struct {
    BigFixed* einstein_lambda;      // Λ = 3/144000
    BigFixed* golden_ratio;         // φ = (1+√5)/2
    BigFixed* ln_3;                 // ln(3)
    BigFixed* pi;                   // π
    BigFixed* two_pi;               // 2π
    BigFixed* pi_over_12;           // π/12
    BigFixed* two_pi_over_12_ln3;   // 2π/(12·ln3)
} MathematicalConstantsBigFixed;

/**
 * Initialize BigFixed representations of mathematical constants
 * Must be called once at program startup
 */
void cllm_init_mathematical_constants(MathematicalConstantsBigFixed* constants);

/**
 * Free BigFixed representations
 * Must be called at program shutdown
 */
void cllm_free_mathematical_constants(MathematicalConstantsBigFixed* constants);

// ============================================================================
// MODULUS CONSTANTS
// ============================================================================

/** Prime modulus: 2³² + 13 */
#ifndef P_MOD_32
#define P_MOD_32 4294967309ULL
#endif

/** Prime modulus: 2⁶⁴ + 13 */
#ifndef P_MOD_64
#define P_MOD_64 18446744073709551629ULL
#endif

/** Machine epsilon for double precision */
#ifndef EPSILON
#define EPSILON 1e-10
#endif

/** Small value for numerical stability */
#ifndef SMALL_VALUE
#define SMALL_VALUE 1e-15
#endif

// ============================================================================
// COMPOSITE CONSTANTS (Golden Ratio + Pi)
// ============================================================================

/** φ × π */
#ifndef PHI_PI_SCALE
#define PHI_PI_SCALE 5.08318530717958647692
#endif

/** φ × 2π */
#ifndef PHI_TWO_PI
#define PHI_TWO_PI 10.16637061435917295384
#endif

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Check if a prime is near the 144000 boundary
 * Returns: 1 if within boundary region, 0 otherwise
 */
int cllm_is_near_144000_boundary(uint64_t prime);

/**
 * Get the dimensional frequency for a given dimension index
 * Returns: φᵢ for dimension i
 */
uint64_t cllm_get_dimensional_frequency(size_t dimension);

/**
 * Calculate the product of dimensional frequencies up to dimension d
 * Returns: φ₀ × φ₁ × ... × φ_d
 */
uint64_t cllm_product_dimensional_frequencies(size_t max_dimension);

/**
 * Get Einstein's Lambda as a double
 * Returns: Λ = 3/144000
 */
static inline double cllm_get_einstein_lambda(void) {
    return EINSTEIN_LAMBDA_DOUBLE;
}

/**
 * Get Einstein's Lambda as a fraction
 * Returns: numerator and denominator via pointers
 */
static inline void cllm_get_einstein_lambda_fraction(int* numerator, int* denominator) {
    *numerator = EINSTEIN_LAMBDA_NUMERATOR;
    *denominator = EINSTEIN_LAMBDA_DENOMINATOR;
}

/**
 * Validate mathematical integrity
 * Checks that all constants are correctly defined
 * Returns: 1 if valid, 0 if corrupted
 */
int cllm_validate_mathematical_constants(void);

#endif // CLLM_MATHEMATICAL_CONSTANTS_H