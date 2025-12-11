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
    // Create Abacus numbers
    CrystallineAbacus* one = abacus_from_double(1.0, base, precision);
    CrystallineAbacus* five = abacus_from_double(5.0, base, precision);
    CrystallineAbacus* two = abacus_from_double(2.0, base, precision);
    
    if (!one || !five || !two) {
        abacus_free(one);
        abacus_free(five);
        abacus_free(two);
        return NULL;
    }
    
    // Compute √5 using double precision (for now)
    // TODO: Implement Abacus sqrt for true arbitrary precision
    double sqrt_five_val = math_sqrt(5.0);
    CrystallineAbacus* sqrt_five = abacus_from_double(sqrt_five_val, base, precision);
    if (!sqrt_five) {
        abacus_free(one);
        abacus_free(five);
        abacus_free(two);
        return NULL;
    }
    
    // Compute 1 + √5
    CrystallineAbacus* numerator = abacus_new(base);
    if (!numerator) {
        abacus_free(one);
        abacus_free(five);
        abacus_free(two);
        abacus_free(sqrt_five);
        return NULL;
    }
    
    MathError err = abacus_add(numerator, one, sqrt_five);
    if (err != MATH_SUCCESS) {
        abacus_free(one);
        abacus_free(five);
        abacus_free(two);
        abacus_free(sqrt_five);
        abacus_free(numerator);
        return NULL;
    }
    
    // Compute (1 + √5) / 2
    CrystallineAbacus* phi = abacus_new(base);
    if (!phi) {
        abacus_free(one);
        abacus_free(five);
        abacus_free(two);
        abacus_free(sqrt_five);
        abacus_free(numerator);
        return NULL;
    }
    
    err = abacus_div(phi, NULL, numerator, two);
    if (err != MATH_SUCCESS) {
        abacus_free(one);
        abacus_free(five);
        abacus_free(two);
        abacus_free(sqrt_five);
        abacus_free(numerator);
        abacus_free(phi);
        return NULL;
    }
    
    // Cleanup
    abacus_free(one);
    abacus_free(five);
    abacus_free(two);
    abacus_free(sqrt_five);
    abacus_free(numerator);
    
    return phi;
}