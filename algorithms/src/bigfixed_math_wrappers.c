#include "bigfixed_core.h"
#include "bigint_core.h"
#include "prime_bigint_transcendental.h"
#include <stdlib.h>

/*
 * BigFixed Math Wrappers
 * Wrapper functions to use transcendental functions with BigFixed
 */

void bigfixed_ln(BigFixed* result, const BigFixed* x, int precision) {
    // Convert BigFixed to BigInt for ln computation
    BigInt* x_int = big_create();
    big_fixed_to_bigint(x_int, x);
    
    // Compute ln
    big_ln(result, x_int, precision);
    
    big_free(x_int);
}

void bigfixed_sqrt(BigFixed* result, const BigFixed* x, int precision) {
    // Convert BigFixed to BigInt for sqrt computation
    BigInt* x_int = big_create();
    big_fixed_to_bigint(x_int, x);
    
    // Compute sqrt
    BigFixed* sqrt_result = big_fixed_create(precision);
    big_sqrt(sqrt_result, x_int, precision);
    
    big_fixed_assign(result, sqrt_result);
    
    big_fixed_free(sqrt_result);
    big_free(x_int);
}

void bigfixed_exp(BigFixed* result, const BigFixed* x, int precision) {
    // big_exp already takes BigFixed*, so just call it
    big_exp(result, x, precision);
}

void bigfixed_tanh(BigFixed* result, const BigFixed* x, int precision) {
    // big_tanh already takes BigFixed*, so just call it
    big_tanh(result, x, precision);
}
