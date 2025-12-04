#ifndef PLIMPTON_322_H
#define PLIMPTON_322_H

/**
 * Plimpton 322 - Pythagorean Triple System
 * 
 * Implements the Ψ(ψ) term in the complete lattice formula.
 * Based on Babylonian mathematics and the Plimpton 322 tablet.
 * 
 * Triple Generation: ψ(p,q) = (a, b, c) where:
 *   a = p² - q²
 *   b = 2pq
 *   c = p² + q²
 * 
 * Constraints:
 *   - gcd(p,q) = 1 (coprime)
 *   - Not both odd
 *   - p > q > 0
 * 
 * Ratios (Plimpton 322 pattern):
 *   b/d = (p²-q²)/(p²+q²)
 *   c/d = 2pq/(p²+q²)
 */

#include <stdint.h>
#include <stdbool.h>
#include "prime_types.h"  // Use existing PythagoreanTriple definition

/**
 * Plimpton 322 ratios structure
 */
typedef struct {
    double ratio_b_d;  // (p²-q²)/(p²+q²)
    double ratio_c_d;  // 2pq/(p²+q²)
    double product;    // ratio_b_d × ratio_c_d
} PlimptonRatios;

/**
 * Verify if two numbers are coprime (gcd = 1)
 * 
 * @param p First number
 * @param q Second number
 * @return true if gcd(p,q) = 1, false otherwise
 */
bool is_coprime(uint64_t p, uint64_t q);

/**
 * Verify Plimpton 322 constraints
 * 
 * Checks all three constraints:
 * 1. gcd(p,q) = 1 (coprime)
 * 2. Not both odd
 * 3. p > q > 0
 * 
 * @param p First generator
 * @param q Second generator
 * @return true if all constraints satisfied, false otherwise
 */
bool verify_plimpton_constraints(uint64_t p, uint64_t q);

/**
 * Calculate Plimpton 322 ratios
 * 
 * Computes the two fundamental ratios from the Plimpton 322 tablet:
 * - b/d = (p²-q²)/(p²+q²)
 * - c/d = 2pq/(p²+q²)
 * 
 * @param p First generator (must satisfy constraints)
 * @param q Second generator (must satisfy constraints)
 * @return PlimptonRatios structure with both ratios and their product
 */
PlimptonRatios calculate_plimpton_ratios(uint64_t p, uint64_t q);

/**
 * Generate Pythagorean triple from p and q
 * 
 * @param p First generator
 * @param q Second generator
 * @param triple Output: generated triple
 */
void generate_pythagorean_triple(uint64_t p, uint64_t q, PythagoreanTriple* triple);

/**
 * Verify Pythagorean triple satisfies a² + b² = c²
 * 
 * @param triple Triple to verify
 * @return true if valid, false otherwise
 */
bool verify_pythagorean_triple_validity(const PythagoreanTriple* triple);

/**
 * Find optimal p,q generators for a given prime
 * 
 * Finds the p,q pair that best represents the prime in the
 * Plimpton 322 system.
 * 
 * @param prime Prime number
 * @param p Output: optimal p value
 * @param q Output: optimal q value
 * @return true if found, false otherwise
 */
bool find_optimal_generators(uint64_t prime, uint64_t* p, uint64_t* q);

#endif // PLIMPTON_322_H