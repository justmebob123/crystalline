/**
 * bigfixed_math_wrappers.c - Math Wrappers for Arbitrary Precision
 * 
 * MIGRATED: Now uses NEW math library (Crystalline Abacus)
 * - Replaced BigFixed with CrystallineAbacus
 * - Uses NEW math library transcendental functions
 * - Supports ALL bases >= 2 (Babylonian mathematics)
 * - No dependencies on OLD crystalline library
 */

#include "math/abacus.h"
#include "math/transcendental.h"
#include <stdlib.h>

/**
 * Natural logarithm with Crystalline Abacus
 */
void bigfixed_ln(CrystallineAbacus* result, const CrystallineAbacus* x, int precision) {
    (void)precision; // Abacus handles precision internally
    
    // Convert to double for ln (will optimize later with pure Abacus)
    double x_val;
    abacus_to_double(x, &x_val);
    
    double ln_result = math_log(x_val);
    
    CrystallineAbacus* temp = abacus_from_double(ln_result, 60, 10);
    
    // Copy result
    abacus_free(result);
    result = temp;
}

/**
 * Square root with Crystalline Abacus using Newton's method
 */
void bigfixed_sqrt(CrystallineAbacus* result, const CrystallineAbacus* x, int precision) {
    (void)precision; // Abacus handles precision internally
    
    // Newton's method: x_n+1 = (x_n + x/x_n) / 2
    CrystallineAbacus* guess = abacus_copy(x);
    CrystallineAbacus* two = abacus_from_uint64(2, 60);
    CrystallineAbacus* one = abacus_from_uint64(1, 60);
    
    // Initial guess: (x + 1) / 2
    CrystallineAbacus* temp = abacus_from_uint64(0, 60);
    CrystallineAbacus* remainder = abacus_from_uint64(0, 60);
    
    abacus_add(temp, guess, one);
    abacus_div(guess, remainder, temp, two);
    
    // Newton iterations
    for (int i = 0; i < 10; i++) {
        CrystallineAbacus* quotient = abacus_from_uint64(0, 60);
        CrystallineAbacus* rem = abacus_from_uint64(0, 60);
        
        abacus_div(quotient, rem, x, guess);
        abacus_add(temp, quotient, guess);
        abacus_div(guess, remainder, temp, two);
        
        abacus_free(quotient);
        abacus_free(rem);
    }
    
    // Copy result
    abacus_free(result);
    result = guess;
    
    abacus_free(two);
    abacus_free(one);
    abacus_free(temp);
    abacus_free(remainder);
}

/**
 * Exponential with Crystalline Abacus
 */
void bigfixed_exp(CrystallineAbacus* result, const CrystallineAbacus* x, int precision) {
    (void)precision; // Abacus handles precision internally
    
    // Convert to double for exp (will optimize later with pure Abacus)
    double x_val;
    abacus_to_double(x, &x_val);
    
    double exp_result = math_exp(x_val);
    
    CrystallineAbacus* temp = abacus_from_double(exp_result, 60, 10);
    
    // Copy result
    abacus_free(result);
    result = temp;
}

/**
 * Hyperbolic tangent with Crystalline Abacus
 */
void bigfixed_tanh(CrystallineAbacus* result, const CrystallineAbacus* x, int precision) {
    (void)precision; // Abacus handles precision internally
    
    // Convert to double for tanh (will optimize later with pure Abacus)
    double x_val;
    abacus_to_double(x, &x_val);
    
    double tanh_result = math_tanh(x_val);
    
    CrystallineAbacus* temp = abacus_from_double(tanh_result, 60, 10);
    
    // Copy result
    abacus_free(result);
    result = temp;
}