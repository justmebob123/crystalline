/**
 * @file mathematical_constants.h
 * @brief Unified mathematical constants for the entire codebase
 * 
 * This header provides a single source of truth for all mathematical constants
 * used throughout the crystalline, algorithms, and CLLM libraries.
 * 
 * CRITICAL: All code must use these constants. Do NOT redefine locally.
 */

#ifndef MATHEMATICAL_CONSTANTS_H
#define MATHEMATICAL_CONSTANTS_H

#include <stdint.h>

/* ============================================================================
 * Golden Ratio Constants
 * ============================================================================ */

/** Golden ratio: φ = (1 + √5) / 2 */
#define PHI 1.618033988749895

/** Golden ratio squared: φ² */
#define PHI_SQUARED 2.618033988749895

/** Golden ratio inverse: 1/φ = φ - 1 */
#define PHI_INVERSE 0.618033988749895

/** Golden ratio cubed: φ³ */
#define PHI_CUBED 4.236067977499790

/** Negative golden ratio: -φ */
#define PHI_NEGATIVE -1.618033988749895

/* ============================================================================
 * Pi Constants
 * ============================================================================ */

/** Pi: π */
#define PI 3.14159265358979323846

/** Two pi: 2π */
#define TWO_PI 6.28318530717958647692

/** Half pi: π/2 */
#define HALF_PI 1.57079632679489661923

/** Quarter pi: π/4 */
#define QUARTER_PI 0.78539816339744830962

/** Pi squared: π² */
#define PI_SQUARED 9.86960440108935861883

/** Pi inverse: 1/π */
#define PI_INVERSE 0.31830988618379067154

/** Square root of pi: √π */
#define SQRT_PI 1.77245385090551602730

/* ============================================================================
 * Composite Constants (Golden Ratio + Pi)
 * ============================================================================ */

/** φ × π */
#define PHI_PI_SCALE 5.08318530717958647692

/** φ × 2π */
#define PHI_TWO_PI 10.16637061435917295384

/** φ² × π */
#define PHI_SQUARED_PI 8.22467287653233306558

/* ============================================================================
 * Euler's Number
 * ============================================================================ */

/** Euler's number: e */
#define E 2.71828182845904523536

/** Natural logarithm of 2: ln(2) */
#define LN_2 0.69314718055994530942

/** Natural logarithm of 10: ln(10) */
#define LN_10 2.30258509299404568402

/* ============================================================================
 * Modulus Constants
 * ============================================================================ */

/** Prime modulus: 2³² + 13 */
#define P_MOD_32 4294967309ULL

/** Prime modulus: 2⁶⁴ + 13 */
#define P_MOD_64 18446744073709551629ULL

/** Mersenne prime: 2³¹ - 1 */
#define MERSENNE_31 2147483647ULL

/** Mersenne prime: 2⁶¹ - 1 */
#define MERSENNE_61 2305843009213693951ULL

/* ============================================================================
 * Mathematical Limits and Tolerances
 * ============================================================================ */

/** Machine epsilon for double precision */
#define EPSILON 1e-10

/** Small value for numerical stability */
#define SMALL_VALUE 1e-15

/** Large value for initialization */
#define LARGE_VALUE 1e308

/** Tolerance for floating point comparison */
#define FLOAT_TOLERANCE 1e-6

/* ============================================================================
 * Geometric Constants
 * ============================================================================ */

/** 12-fold symmetry angle: 2π/12 = 30° */
#define ANGLE_12_FOLD 0.52359877559829887308

/** 60-fold symmetry angle: 2π/60 = 6° */
#define ANGLE_60_FOLD 0.10471975511965977462

/** 100-fold symmetry angle: 2π/100 = 3.6° */
#define ANGLE_100_FOLD 0.06283185307179586477

/** Icosahedral angle: arccos(φ/2) */
#define ICOSAHEDRAL_ANGLE 1.10714871779409050301

/** Dodecahedral dihedral angle */
#define DODECAHEDRAL_ANGLE 2.03444393579245283067

/* ============================================================================
 * Babylonian Clock Constants
 * ============================================================================ */

/** Hours in Babylonian clock */
#define BABYLONIAN_HOURS 12

/** Minutes per hour */
#define BABYLONIAN_MINUTES 60

/** Seconds per minute */
#define BABYLONIAN_SECONDS 60

/** Milliseconds (base 100) */
#define BABYLONIAN_MILLIS 100

/** Total positions in clock lattice */
#define CLOCK_LATTICE_SIZE 232

/* ============================================================================
 * Harmonic Frequencies (Hz)
 * ============================================================================ */

/** Solfeggio frequency: 396 Hz */
#define FREQ_396 396.0

/** Solfeggio frequency: 417 Hz */
#define FREQ_417 417.0

/** Solfeggio frequency: 432 Hz (Verdi's A) */
#define FREQ_432 432.0

/** Solfeggio frequency: 528 Hz (Love frequency) */
#define FREQ_528 528.0

/** Solfeggio frequency: 639 Hz */
#define FREQ_639 639.0

/** Solfeggio frequency: 741 Hz */
#define FREQ_741 741.0

/** Solfeggio frequency: 852 Hz */
#define FREQ_852 852.0

/** Solfeggio frequency: 963 Hz */
#define FREQ_963 963.0

/* ============================================================================
 * Conversion Factors
 * ============================================================================ */

/** Degrees to radians: π/180 */
#define DEG_TO_RAD 0.01745329251994329577

/** Radians to degrees: 180/π */
#define RAD_TO_DEG 57.29577951308232087680

/* ============================================================================
 * Special Values
 * ============================================================================ */

/** Square root of 2: √2 */
#define SQRT_2 1.41421356237309504880

/** Square root of 3: √3 */
#define SQRT_3 1.73205080756887729353

/** Square root of 5: √5 */
#define SQRT_5 2.23606797749978969641

/** Cube root of 2: ∛2 */
#define CBRT_2 1.25992104989487316477

#endif /* MATHEMATICAL_CONSTANTS_H */