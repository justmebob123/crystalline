/**
 * @file clock.h
 * @brief Clock lattice for deterministic prime generation
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * This module implements the revolutionary clock lattice approach for
 * deterministic prime generation based on Babylonian mathematics.
 * 
 * Clock Structure: 12, 60, 60, 100 (hours, minutes, seconds, milliseconds)
 * 
 * Key Principle: Structure IS validation
 * - Each prime maps to a unique position on the clock
 * - Given position, can compute prime deterministically
 * - Given prime, can compute position deterministically
 * - NO trial division, NO sieving, NO probabilistic tests
 * 
 * This replaces ALL legacy prime generation methods.
 */

#ifndef MATH_CLOCK_H
#define MATH_CLOCK_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CLOCK CONTEXT
 * ============================================================================
 */

/**
 * @brief Initialize clock context
 * @param ctx Clock context to initialize
 * @return MATH_SUCCESS or error code
 */
MathError clock_init(ClockContext* ctx);

/**
 * @brief Free clock context
 * @param ctx Clock context to free
 */
void clock_free(ClockContext* ctx);

/* ============================================================================
 * POSITION MAPPING
 * ============================================================================
 */

/**
 * @brief Map prime number to clock position
 * @param prime Prime number
 * @param pos Output clock position (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 * 
 * This is the fundamental mapping function.
 * Given a prime, computes its unique position on the clock lattice.
 */
MathError clock_map_prime_to_position(uint64_t prime, ClockPosition* pos);

/**
 * @brief Map clock position to prime number
 * @param pos Clock position
 * @return Prime number at that position
 * 
 * This is the inverse of clock_map_prime_to_position.
 * Given a valid clock position, computes the prime at that position.
 */
/**
 * @brief Get a representative prime at a clock position
 * @param pos Clock position
 * @return A prime number at this position, or 0 if invalid
 * 
 * NOTE: Multiple primes can map to the same position (they share
 * the same modular class). This function returns the SMALLEST prime
 * at the given position.
 * 
 * Example: Position (0, 0) contains primes 2, 13, 37, 61, ...
 * This function returns 2 (the smallest).
 */
uint64_t clock_position_to_prime(const ClockPosition* pos);

/**
 * @brief Map prime index to clock position
 * @param prime_index 1-based prime index (1 = first prime = 2)
 * @param pos Output clock position (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 * 
 * Maps the nth prime to its clock position.
 * This is used for sequential prime generation.
 */
MathError clock_map_index_to_position(uint64_t prime_index, ClockPosition* pos);

/**
 * @brief Map clock position to prime index
 * @param pos Clock position
 * @return Prime index (1-based)
 */
uint64_t clock_position_to_index(const ClockPosition* pos);

/* ============================================================================
 * POSITION VALIDATION
 * ============================================================================
 */

/**
 * @brief Check if clock position is valid
 * @param pos Clock position to validate
 * @return true if position is valid (corresponds to a prime)
 * 
 * Uses modular arithmetic to validate position.
 * This is the deterministic validation - no trial division needed.
 */
bool clock_is_valid_position(const ClockPosition* pos);

/**
 * @brief Get next valid clock position
 * @param current Current position
 * @param next Output next position (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 */
MathError clock_next_position(const ClockPosition* current, ClockPosition* next);

/**
 * @brief Get previous valid clock position
 * @param current Current position
 * @param prev Output previous position (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 */
MathError clock_prev_position(const ClockPosition* current, ClockPosition* prev);

/* ============================================================================
 * MODULAR RELATIONSHIPS
 * ============================================================================
 */

/**
 * @brief Get modular arithmetic relationships for a prime
 * @param prime Prime number
 * @param pm Output modular relationships (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 */
MathError clock_get_modular(uint64_t prime, PrimeModular* pm);

/**
 * @brief Check if number satisfies clock modular constraints
 * @param n Number to check
 * @return true if n satisfies modular constraints (necessary but not sufficient for primality)
 */
bool clock_satisfies_modular(uint64_t n);

/* ============================================================================
 * GEOMETRIC OPERATIONS
 * ============================================================================
 */

/**
 * @brief Fold clock position to 3D sphere
 * @param pos Clock position
 * @param sphere Output sphere coordinates (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 * 
 * Uses stereographic projection to map clock to sphere.
 * This is used for geometric analysis and visualization.
 */
MathError clock_fold_to_sphere(const ClockPosition* pos, SphereCoord* sphere);

/**
 * @brief Unfold sphere coordinates to clock position
 * @param sphere Sphere coordinates
 * @param pos Output clock position (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 * 
 * Inverse of clock_fold_to_sphere.
 */
MathError clock_unfold_from_sphere(const SphereCoord* sphere, ClockPosition* pos);

/**
 * @brief Compute angular distance between two clock positions
 * @param pos1 First position
 * @param pos2 Second position
 * @return Angular distance in radians
 */
double clock_angular_distance(const ClockPosition* pos1, const ClockPosition* pos2);

/**
 * @brief Compute geometric distance on sphere
 * @param pos1 First position
 * @param pos2 Second position
 * @return Great circle distance on unit sphere
 */
double clock_sphere_distance(const ClockPosition* pos1, const ClockPosition* pos2);

/* ============================================================================
 * CACHE MANAGEMENT
 * ============================================================================
 */

/**
 * @brief Populate prime cache up to index
 * @param ctx Clock context
 * @param max_index Maximum prime index to cache
 * @return MATH_SUCCESS or error code
 * 
 * Pre-computes primes for faster lookup.
 * Optional - clock functions work without cache.
 */
MathError clock_populate_cache(ClockContext* ctx, uint64_t max_index);

/**
 * @brief Get cached prime by index
 * @param ctx Clock context
 * @param index Prime index (1-based)
 * @return Cached prime, or 0 if not in cache
 */
uint64_t clock_get_cached_prime(const ClockContext* ctx, uint64_t index);

/**
 * @brief Clear prime cache
 * @param ctx Clock context
 */
void clock_clear_cache(ClockContext* ctx);

#ifdef __cplusplus
}
#endif

#endif /* MATH_CLOCK_H */