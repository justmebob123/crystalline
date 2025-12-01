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
    BigInt x_int;
    big_init(&x_int);
    big_fixed_to_bigint(&x_int, x);
    
    // Compute ln
    big_ln(result, &x_int, precision);
    
    big_free(&x_int);
}

void bigfixed_sqrt(BigFixed* result, const BigFixed* x, int precision) {
    // For sqrt, we can use Newton's method with BigFixed directly
    // sqrt(x) ≈ (x + 1) / 2 as initial guess, then iterate
    BigFixed* guess = big_fixed_create(precision);
    BigFixed* temp = big_fixed_create(precision);
    BigFixed* two = big_fixed_create(precision);
    
    big_fixed_from_int(two, 2);
    big_fixed_assign(guess, x);
    big_fixed_add(guess, guess, two);
    big_fixed_div(guess, guess, two);
    
    // Newton iteration: x_n+1 = (x_n + x/x_n) / 2
    for (int i = 0; i < 10; i++) {
        big_fixed_div(temp, x, guess);
        big_fixed_add(temp, temp, guess);
        big_fixed_div(guess, temp, two);
    }
    
    big_fixed_assign(result, guess);
    
    big_fixed_free(guess);
    big_fixed_free(temp);
    big_fixed_free(two);
}

void bigfixed_exp(BigFixed* result, const BigFixed* x, int precision) {
    // big_exp already takes BigFixed*, so just call it
    big_exp(result, x, precision);
}

void bigfixed_tanh(BigFixed* result, const BigFixed* x, int precision) {
    // big_tanh already takes BigFixed*, so just call it
    big_tanh(result, x, precision);
}
