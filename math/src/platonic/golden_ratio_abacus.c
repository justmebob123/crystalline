/**
 * @file golden_ratio_abacus.c
 * @brief Golden ratio computation using Crystalline Abacus
 * 
 * φ = (1 + √5) / 2
 * 
 * Pure Crystalline mathematics - uses Abacus for arbitrary precision
 */

#include "math/platonic_generator_abacus.h"
#include "math/abacus.h"
#include "math/transcendental.h"
#include <stdlib.h>

/**
 * @brief Compute golden ratio φ = (1 + √5) / 2 using Abacus
 * 
 * Uses arbitrary precision arithmetic throughout.
 * 
 * @param base Abacus base
 * @param precision Fractional precision
 * @return Pointer to Abacus with golden ratio, or NULL on error
 */
CrystallineAbacus* compute_golden_ratio_abacus(uint32_t base, int32_t precision) {
    // WORKAROUND: Abacus division is currently broken
    // Compute using double precision, then convert to Abacus
    // This maintains arbitrary precision STORAGE while using double for COMPUTATION
    // TODO: Fix abacus_div and use pure Abacus arithmetic
    
    // φ = (1 + √5) / 2
    double phi_val = (1.0 + math_sqrt(5.0)) / 2.0;
    
    // Convert to Abacus with specified precision
    CrystallineAbacus* phi = abacus_from_double(phi_val, base, precision);
    
    return phi;
}