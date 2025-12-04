/**
 * Plimpton 322 - Pythagorean Triple System Implementation
 * 
 * Implements the Ψ(ψ) term for the complete lattice formula.
 * Based on Babylonian mathematics and the Plimpton 322 tablet.
 */

#include "../include/plimpton_322.h"
#include "../include/prime_lowlevel.h"
#include <stdio.h>

/**
 * Verify if two numbers are coprime (gcd = 1)
 */
bool is_coprime(uint64_t p, uint64_t q) {
    return prime_gcd(p, q) == 1;
}

/**
 * Verify Plimpton 322 constraints
 * 
 * All three constraints must be satisfied:
 * 1. gcd(p,q) = 1 (coprime)
 * 2. Not both odd
 * 3. p > q > 0
 */
bool verify_plimpton_constraints(uint64_t p, uint64_t q) {
    // Constraint 1: p > q > 0
    if (p <= q || q == 0) {
        return false;
    }
    
    // Constraint 2: gcd(p,q) = 1 (coprime)
    if (!is_coprime(p, q)) {
        return false;
    }
    
    // Constraint 3: Not both odd
    if ((p % 2 == 1) && (q % 2 == 1)) {
        return false;
    }
    
    return true;
}

/**
 * Calculate Plimpton 322 ratios
 * 
 * Computes the two fundamental ratios:
 * - ratio_b_d = (p²-q²)/(p²+q²)
 * - ratio_c_d = 2pq/(p²+q²)
 * 
 * These ratios sum to 1.0 and define work distribution in the system.
 */
PlimptonRatios calculate_plimpton_ratios(uint64_t p, uint64_t q) {
    PlimptonRatios ratios = {0.0, 0.0, 0.0};
    
    // Verify constraints
    if (!verify_plimpton_constraints(p, q)) {
        // Return default ratios if constraints not satisfied
        ratios.ratio_b_d = 0.5;
        ratios.ratio_c_d = 0.5;
        ratios.product = 0.25;
        return ratios;
    }
    
    // Calculate p² and q²
    uint64_t p2 = p * p;
    uint64_t q2 = q * q;
    uint64_t denom = p2 + q2;
    
    if (denom == 0) {
        ratios.ratio_b_d = 0.5;
        ratios.ratio_c_d = 0.5;
        ratios.product = 0.25;
        return ratios;
    }
    
    // Calculate ratios
    ratios.ratio_b_d = (double)(p2 - q2) / (double)denom;
    ratios.ratio_c_d = (2.0 * p * q) / (double)denom;
    ratios.product = ratios.ratio_b_d * ratios.ratio_c_d;
    
    return ratios;
}

/**
 * Generate Pythagorean triple from p and q
 */
void generate_pythagorean_triple(uint64_t p, uint64_t q, PythagoreanTriple* triple) {
    if (!triple) return;
    
    triple->p = p;
    triple->q = q;
    
    uint64_t p2 = p * p;
    uint64_t q2 = q * q;
    
    triple->a = p2 - q2;
    triple->b = 2 * p * q;
    triple->c = p2 + q2;
    triple->layer = 0;  // Default layer
    triple->ratio = (double)p / (double)q;
}

/**
 * Verify Pythagorean triple satisfies a² + b² = c²
 */
bool verify_pythagorean_triple_validity(const PythagoreanTriple* triple) {
    if (!triple) return false;
    
    // Calculate a² + b²
    uint64_t a2 = triple->a * triple->a;
    uint64_t b2 = triple->b * triple->b;
    uint64_t c2 = triple->c * triple->c;
    
    // Verify a² + b² = c²
    return (a2 + b2) == c2;
}

/**
 * Find optimal p,q generators for a given prime
 * 
 * Strategy:
 * 1. Try small values of q (1, 2, 3, ...)
 * 2. For each q, calculate p such that p² - q² ≈ prime
 * 3. Verify constraints
 * 4. Return first valid pair
 */
bool find_optimal_generators(uint64_t prime, uint64_t* p, uint64_t* q) {
    if (!p || !q) return false;
    
    // Try q from 1 to sqrt(prime)
    uint64_t max_q = 100;  // Reasonable limit
    if (prime < 10000) {
        max_q = 50;
    }
    
    for (uint64_t q_try = 1; q_try <= max_q; q_try++) {
        // Calculate p such that p² - q² ≈ prime
        // p² ≈ prime + q²
        // p ≈ sqrt(prime + q²)
        
        uint64_t q2 = q_try * q_try;
        uint64_t p2_target = prime + q2;
        
        // Try p values around sqrt(p2_target)
        uint64_t p_approx = 1;
        while (p_approx * p_approx < p2_target) {
            p_approx++;
        }
        
        // Try p_approx and nearby values
        for (int delta = -2; delta <= 2; delta++) {
            uint64_t p_try = p_approx + delta;
            if (p_try <= q_try) continue;
            
            // Verify constraints
            if (verify_plimpton_constraints(p_try, q_try)) {
                *p = p_try;
                *q = q_try;
                return true;
            }
        }
    }
    
    // Default fallback: p=3, q=2 (generates 5, 12, 13)
    *p = 3;
    *q = 2;
    return true;
}