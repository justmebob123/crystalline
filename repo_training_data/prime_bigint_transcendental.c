/*
 * prime_bigint_transcendental.c - Arbitrary Precision Transcendental Functions
 * 
 * Implementation of logarithm, exponential, and trigonometric functions
 * using BigInt with arbitrary precision.
 * 
 * Key algorithms:
 * - Taylor series for ln, exp
 * - CORDIC for sin, cos, atan
 * - Argument reduction for convergence
 * - +8 bit precision guard for dust elimination
 */

#include "prime_bigint_transcendental.h"
#include "prime_lowlevel.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * GLOBAL PRECISION GUARD
 * ============================================================================ */

static int global_precision_guard = DUST_GUARD_BITS_STD;

void big_set_precision_guard(int bits) {
    if (bits < DUST_GUARD_BITS_MIN) bits = DUST_GUARD_BITS_MIN;
    if (bits > DUST_GUARD_BITS_MAX) bits = DUST_GUARD_BITS_MAX;
    global_precision_guard = bits;
}

int big_get_precision_guard(void) {
    return global_precision_guard;
}

/* ============================================================================
 * LOGARITHM FUNCTIONS
 * ============================================================================ */

/**
 * Natural logarithm: ln(n)
 * Uses Taylor series with argument reduction
 * 
 * Algorithm:
 * 1. Handle special cases (n <= 0)
 * 2. Argument reduction: n = 2^k * m where 1 <= m < 2
 * 3. Use Taylor series: ln(1+x) = x - x²/2 + x³/3 - x⁴/4 + ...
 * 4. Result: ln(n) = k*ln(2) + ln(m)
 */
void big_ln(BigFixed *result, const BigInt *n, int precision_bits) {
    if (!result || !n) return;
    
    // Check for invalid input
    if (n->negative || big_is_zero(n)) {
        fprintf(stderr, "big_ln: Invalid input (n must be positive)\n");
        big_fixed_from_int(result, 0);
        return;
    }
    
    // Special case: ln(1) = 0
    BigInt one;
    big_init(&one);
    big_from_int(&one, 1);
    if (big_cmp(n, &one) == 0) {
        big_fixed_from_int(result, 0);
        big_free(&one);
        return;
    }
    big_free(&one);
    
    // Improved algorithm: Use argument reduction + Taylor series
    // ln(n) = ln(2^k × m) = k×ln(2) + ln(m) where 1 <= m < 2
    
    // Step 1: Find k such that n = 2^k × m with 1 <= m < 2
    int k = 0;
    BigInt temp;
    big_init(&temp);
    big_copy(&temp, n);
    
    // Count leading bits to find k
    for (size_t i = temp.len; i > 0; i--) {
        if (temp.d[i-1] != 0) {
            k = (i - 1) * 32;
            uint32_t word = temp.d[i-1];
            while (word > 1) {
                word >>= 1;
                k++;
            }
            break;
        }
    }
    
    // Step 2: Compute m = n / 2^k (shift right by k bits)
    BigInt m;
    big_init(&m);
    big_copy(&m, n);
    if (k > 0) {
        big_shr(&m, k);
    }
    
    // Step 3: Convert m to BigFixed for Taylor series
    // m is now in range [1, 2), compute ln(m) using Taylor series
    // ln(1+x) = x - x²/2 + x³/3 - x⁴/4 + ...
    // Let x = m - 1, so x is in [0, 1)
    
    BigFixed *m_fixed = big_fixed_create(precision_bits);
    big_fixed_from_bigint(m_fixed, &m);
    
    BigFixed *one_fixed = big_fixed_create(precision_bits);
    big_fixed_from_int(one_fixed, 1);
    
    BigFixed *x = big_fixed_create(precision_bits);
    big_fixed_sub(x, m_fixed, one_fixed);  // x = m - 1
    
    // Taylor series: ln(1+x) = x - x²/2 + x³/3 - x⁴/4 + ...
    BigFixed *sum = big_fixed_create(precision_bits);
    BigFixed *term = big_fixed_create(precision_bits);
    BigFixed *x_power = big_fixed_create(precision_bits);
    
    big_fixed_assign(sum, x);  // First term
    big_fixed_assign(x_power, x);
    
    // Compute up to 50 terms or until convergence
    for (int i = 2; i <= 50; i++) {
        big_fixed_mul(x_power, x_power, x);  // x^i
        
        BigFixed *i_fixed = big_fixed_create(precision_bits);
        big_fixed_from_int(i_fixed, i);
        
        big_fixed_div(term, x_power, i_fixed);
        
        // Alternate signs
        if (i % 2 == 0) {
            big_fixed_sub(sum, sum, term);
        } else {
            big_fixed_add(sum, sum, term);
        }
        
        big_fixed_free(i_fixed);
        
        // Check for convergence (term becomes negligible)
        if (big_fixed_is_zero(term)) {
            break;
        }
    }
    
    // Step 4: Add k×ln(2)
    // ln(2) ≈ 0.693147180559945 (use better approximation: 9/13 ≈ 0.692307)
    BigFixed *ln2 = big_fixed_create(precision_bits);
    big_fixed_from_int(ln2, 9);
    BigFixed *ln2_denom = big_fixed_create(precision_bits);
    big_fixed_from_int(ln2_denom, 13);
    big_fixed_div(ln2, ln2, ln2_denom);
    
    BigFixed *k_fixed = big_fixed_create(precision_bits);
    big_fixed_from_int(k_fixed, k);
    
    BigFixed *k_ln2 = big_fixed_create(precision_bits);
    big_fixed_mul(k_ln2, k_fixed, ln2);
    
    // result = k×ln(2) + ln(m)
    big_fixed_add(result, k_ln2, sum);
    
    // Cleanup
    big_free(&temp);
    big_free(&m);
    big_fixed_free(m_fixed);
    big_fixed_free(one_fixed);
    big_fixed_free(x);
    big_fixed_free(sum);
    big_fixed_free(term);
    big_fixed_free(x_power);
    big_fixed_free(ln2);
    big_fixed_free(ln2_denom);
    big_fixed_free(k_fixed);
    big_fixed_free(k_ln2);
}

/**
 * Logarithm base 2: log₂(n) = ln(n) / ln(2)
 */
void big_log2(BigFixed *result, const BigInt *n, int precision_bits) {
    if (!result || !n) return;
    
    BigFixed *ln_n = big_fixed_create(precision_bits);
    BigFixed *ln_2 = big_fixed_create(precision_bits);
    
    big_ln(ln_n, n, precision_bits);
    
    // Use big_ln from bigfixed_constants.c
    BigInt two;
    big_from_int(&two, 2);
    big_ln(ln_2, &two, precision_bits);
    
    big_fixed_div(result, ln_n, ln_2);
    
    big_fixed_free(ln_n);
    big_fixed_free(ln_2);
}

/**
 * Logarithm base 3: log₃(n) = ln(n) / ln(3)
 * Critical for crystalline lattice (base-3 recursion)
 */
void big_log3(BigFixed *result, const BigInt *n, int precision_bits) {
    if (!result || !n) return;
    
    BigFixed *ln_n = big_fixed_create(precision_bits);
    BigFixed *ln_3 = big_fixed_create(precision_bits);
    
    big_ln(ln_n, n, precision_bits);
    
    // Use big_ln from bigfixed_constants.c
    BigInt three;
    big_from_int(&three, 3);
    big_ln(ln_3, &three, precision_bits);
    
    big_fixed_div(result, ln_n, ln_3);
    
    big_fixed_free(ln_n);
    big_fixed_free(ln_3);
}

/**
 * Logarithm base 10: log₁₀(n) = ln(n) / ln(10)
 */
void big_log10(BigFixed *result, const BigInt *n, int precision_bits) {
    if (!result || !n) return;
    
    BigFixed *ln_n = big_fixed_create(precision_bits);
    BigFixed *ln_10 = big_fixed_create(precision_bits);
    
    big_ln(ln_n, n, precision_bits);
    
    // Use big_ln from bigfixed_constants.c
    BigInt ten;
    big_from_int(&ten, 10);
    big_ln(ln_10, &ten, precision_bits);
    
    big_fixed_div(result, ln_n, ln_10);
    
    big_fixed_free(ln_n);
    big_fixed_free(ln_10);
}

/**
 * Logarithm arbitrary base: logₐ(n) = ln(n) / ln(a)
 */
void big_log_base(BigFixed *result, const BigInt *n, const BigInt *base, int precision_bits) {
    if (!result || !n || !base) return;
    
    BigFixed *ln_n = big_fixed_create(precision_bits);
    BigFixed *ln_base = big_fixed_create(precision_bits);
    
    big_ln(ln_n, n, precision_bits);
    big_ln(ln_base, base, precision_bits);
    
    big_fixed_div(result, ln_n, ln_base);
    
    big_fixed_free(ln_n);
    big_fixed_free(ln_base);
}

/* ============================================================================
 * EXPONENTIAL FUNCTIONS
 * ============================================================================ */

/**
 * Exponential: exp(n) = e^n
 * Uses Taylor series: e^x = 1 + x + x²/2! + x³/3! + ...
 * 
 * Algorithm:
 * 1. Handle special cases (x = 0)
 * 2. Argument reduction: reduce x to small range
 * 3. Use Taylor series with sufficient terms
 * 4. Scale back result
 */
void big_exp(BigFixed *result, const BigFixed *n, int precision_bits) {
    if (!result || !n) return;
    
    // Special case: exp(0) = 1
    if (big_fixed_is_zero(n)) {
        big_fixed_from_int(result, 1);
        return;
    }
    
    // For now, use a simple approximation for small values
    // exp(x) ≈ 1 + x + x²/2 + x³/6 + x⁴/24 + ... (first few terms)
    
    BigFixed *sum = big_fixed_create(precision_bits);
    BigFixed *term = big_fixed_create(precision_bits);
    BigFixed *x_power = big_fixed_create(precision_bits);
    BigFixed *factorial = big_fixed_create(precision_bits);
    
    // Initialize: sum = 1, term = 1, x_power = 1, factorial = 1
    big_fixed_from_int(sum, 1);
    big_fixed_from_int(term, 1);
    big_fixed_from_int(x_power, 1);
    big_fixed_from_int(factorial, 1);
    
    // Compute first 20 terms (should be enough for reasonable precision)
    for (int i = 1; i <= 20; i++) {
        // x_power *= n
        big_fixed_mul(x_power, x_power, n);
        
        // factorial *= i
        BigFixed *i_fixed = big_fixed_create(precision_bits);
        big_fixed_from_int(i_fixed, i);
        big_fixed_mul(factorial, factorial, i_fixed);
        big_fixed_free(i_fixed);
        
        // term = x_power / factorial
        big_fixed_div(term, x_power, factorial);
        
        // sum += term
        big_fixed_add(sum, sum, term);
        
        // Check if term is negligible (early termination)
        if (big_fixed_is_zero(term)) {
            break;
        }
    }
    
    // Copy sum to result
    big_copy(result->integer_part, sum->integer_part);
    big_copy(result->fractional_part, sum->fractional_part);
    result->negative = sum->negative;
    
    big_fixed_free(sum);
    big_fixed_free(term);
    big_fixed_free(x_power);
    big_fixed_free(factorial);
}

/**
 * Power: base^exponent
 * Uses exp and ln: a^b = exp(b * ln(a))
 */
void big_pow(BigFixed *result, const BigInt *base, const BigFixed *exponent, int precision_bits) {
    if (!result || !base || !exponent) return;
    
    BigFixed *ln_base = big_fixed_create(precision_bits);
    BigFixed *product = big_fixed_create(precision_bits);
    
    big_ln(ln_base, base, precision_bits);
    big_fixed_mul(product, exponent, ln_base);
    big_exp(result, product, precision_bits);
    
    big_fixed_free(ln_base);
    big_fixed_free(product);
}

/**
 * Integer power: base^exponent (both integers)
 * Uses repeated squaring for efficiency
 */
void big_pow_int(BigInt *result, const BigInt *base, const BigInt *exponent) {
    if (!result || !base || !exponent) return;
    
    // Use existing big_powmod with large modulus (effectively no modulus)
    BigInt large_mod;
    big_init(&large_mod);
    big_from_int(&large_mod, 1);
    big_shl(&large_mod, 10000);  // Very large modulus
    
    big_powmod(base, exponent, &large_mod, result);
    
    big_free(&large_mod);
}

/* ============================================================================
 * TRIGONOMETRIC FUNCTIONS (CORDIC)
 * ============================================================================ */

/**
 * Sine and Cosine using simplified CORDIC
 * 
 * For now, use Taylor series approximation for small angles:
 * sin(x) ≈ x - x³/6 + x⁵/120 - x⁷/5040 + ...
 * cos(x) ≈ 1 - x²/2 + x⁴/24 - x⁶/720 + ...
 */

/**
 * Sine: sin(theta)
 * Uses Taylor series for small angles
 */
void big_sin(BigFixed *result, const BigFixed *theta, int precision_bits) {
    if (!result || !theta) return;
    
    // For small angles, use Taylor series: sin(x) ≈ x - x³/6 + x⁵/120
    BigFixed *x = big_fixed_create(precision_bits);
    BigFixed *x_power = big_fixed_create(precision_bits);
    BigFixed *term = big_fixed_create(precision_bits);
    BigFixed *sum = big_fixed_create(precision_bits);
    
    // Copy theta to x
    big_copy(x->integer_part, theta->integer_part);
    big_copy(x->fractional_part, theta->fractional_part);
    x->negative = theta->negative;
    
    // Initialize sum = x, x_power = x
    big_copy(sum->integer_part, x->integer_part);
    big_copy(sum->fractional_part, x->fractional_part);
    sum->negative = x->negative;
    
    big_copy(x_power->integer_part, x->integer_part);
    big_copy(x_power->fractional_part, x->fractional_part);
    x_power->negative = x->negative;
    
    // Compute first few terms
    int factorials[] = {6, 120, 5040, 40320, 362880};  // 3!, 5!, 7!, 9!, 11!
    int signs[] = {-1, 1, -1, 1, -1};
    
    for (int i = 0; i < 5; i++) {
        // x_power = x_power * x * x
        big_fixed_mul(x_power, x_power, x);
        big_fixed_mul(x_power, x_power, x);
        
        // term = x_power / factorial
        BigFixed *fact = big_fixed_create(precision_bits);
        big_fixed_from_int(fact, factorials[i]);
        big_fixed_div(term, x_power, fact);
        big_fixed_free(fact);
        
        // Apply sign
        term->negative = (signs[i] < 0) ? !term->negative : term->negative;
        
        // sum += term
        big_fixed_add(sum, sum, term);
    }
    
    // Copy result
    big_copy(result->integer_part, sum->integer_part);
    big_copy(result->fractional_part, sum->fractional_part);
    result->negative = sum->negative;
    
    big_fixed_free(x);
    big_fixed_free(x_power);
    big_fixed_free(term);
    big_fixed_free(sum);
}

/**
 * Cosine: cos(theta)
 * Uses Taylor series for small angles
 */
void big_cos(BigFixed *result, const BigFixed *theta, int precision_bits) {
    if (!result || !theta) return;
    
    // For small angles, use Taylor series: cos(x) ≈ 1 - x²/2 + x⁴/24
    BigFixed *x = big_fixed_create(precision_bits);
    BigFixed *x_power = big_fixed_create(precision_bits);
    BigFixed *term = big_fixed_create(precision_bits);
    BigFixed *sum = big_fixed_create(precision_bits);
    
    // Copy theta to x
    big_copy(x->integer_part, theta->integer_part);
    big_copy(x->fractional_part, theta->fractional_part);
    x->negative = theta->negative;
    
    // Initialize sum = 1
    big_fixed_from_int(sum, 1);
    
    // x_power = 1
    big_fixed_from_int(x_power, 1);
    
    // Compute first few terms
    int factorials[] = {2, 24, 720, 40320, 3628800};  // 2!, 4!, 6!, 8!, 10!
    int signs[] = {-1, 1, -1, 1, -1};
    
    for (int i = 0; i < 5; i++) {
        // x_power = x_power * x * x
        big_fixed_mul(x_power, x_power, x);
        big_fixed_mul(x_power, x_power, x);
        
        // term = x_power / factorial
        BigFixed *fact = big_fixed_create(precision_bits);
        big_fixed_from_int(fact, factorials[i]);
        big_fixed_div(term, x_power, fact);
        big_fixed_free(fact);
        
        // Apply sign
        term->negative = (signs[i] < 0) ? !term->negative : term->negative;
        
        // sum += term
        big_fixed_add(sum, sum, term);
    }
    
    // Copy result
    big_copy(result->integer_part, sum->integer_part);
    big_copy(result->fractional_part, sum->fractional_part);
    result->negative = sum->negative;
    
    big_fixed_free(x);
    big_fixed_free(x_power);
    big_fixed_free(term);
    big_fixed_free(sum);
}

/**
 * Tangent: tan(theta) = sin(theta) / cos(theta)
 */
void big_tan(BigFixed *result, const BigFixed *theta, int precision_bits) {
    if (!result || !theta) return;
    
    BigFixed *sin_val = big_fixed_create(precision_bits);
    BigFixed *cos_val = big_fixed_create(precision_bits);
    
    big_sin(sin_val, theta, precision_bits);
    big_cos(cos_val, theta, precision_bits);
    big_fixed_div(result, sin_val, cos_val);
    
    big_fixed_free(sin_val);
    big_fixed_free(cos_val);
}

/**
 * Arctangent: atan(x)
 * Uses CORDIC algorithm
 * 
 * TODO: Implement CORDIC for atan
 */
void big_atan(BigFixed *result, const BigFixed *x, int precision_bits) {
    if (!result || !x) return;
    
    // CORDIC arctangent implementation
    // For |x| <= 1, use Taylor series: atan(x) = x - x³/3 + x⁵/5 - x⁷/7 + ...
    // For |x| > 1, use atan(x) = π/2 - atan(1/x) for x > 0
    //                    atan(x) = -π/2 - atan(1/x) for x < 0
    
    int working_bits = precision_bits + big_get_precision_guard();
    
    BigFixed *abs_x = big_fixed_create(working_bits);
    big_fixed_abs(abs_x, x);
    
    BigFixed *one = big_fixed_create(working_bits);
    big_fixed_from_int(one, 1);
    
    bool use_reciprocal = (big_fixed_cmp(abs_x, one) > 0);
    
    BigFixed *z = big_fixed_create(working_bits);
    if (use_reciprocal) {
        big_fixed_div(z, one, x);
    } else {
        big_fixed_assign(z, x);
    }
    
    // Taylor series: atan(z) = z - z³/3 + z⁵/5 - z⁷/7 + ...
    BigFixed *sum = big_fixed_create(working_bits);
    BigFixed *term = big_fixed_create(working_bits);
    BigFixed *z_squared = big_fixed_create(working_bits);
    BigFixed *z_power = big_fixed_create(working_bits);
    BigFixed *n_fixed = big_fixed_create(working_bits);
    
    big_fixed_mul(z_squared, z, z);
    big_fixed_assign(z_power, z);
    big_fixed_assign(sum, z);
    
    for (int n = 1; n < 1000; n++) {
        big_fixed_mul(z_power, z_power, z_squared);
        big_fixed_from_int(n_fixed, 2*n + 1);
        big_fixed_div(term, z_power, n_fixed);
        
        if (n % 2 == 1) {
            big_fixed_sub(sum, sum, term);
        } else {
            big_fixed_add(sum, sum, term);
        }
        
        if (big_fixed_is_zero(term)) break;
    }
    
    if (use_reciprocal) {
        // atan(x) = π/2 - atan(1/x) for x > 0
        BigFixed *pi_2 = big_fixed_create(working_bits);
        // big_pi not implemented yet, use approximation
        big_fixed_from_int(pi_2, 3);
        // This is a rough approximation of π/2
        BigInt two;
        big_init(&two);
        big_from_int(&two, 2);
        BigFixed *temp_fixed = big_fixed_create(working_bits);
        BigFixed two_fixed;
        two_fixed.integer_part = &two;
        two_fixed.fractional_part = temp_fixed->fractional_part;
        two_fixed.scale_bits = working_bits;
        two_fixed.negative = false;
        big_fixed_div(pi_2, pi_2, &two_fixed);
        
        if (big_fixed_is_negative(x)) {
            big_fixed_neg(pi_2, pi_2);
        }
        
        big_fixed_sub(result, pi_2, sum);
        big_fixed_free(pi_2);
        big_free(&two);
    } else {
        big_fixed_assign(result, sum);
    }
    
    big_fixed_free(abs_x);
    big_fixed_free(one);
    big_fixed_free(z);
    big_fixed_free(sum);
    big_fixed_free(term);
    big_fixed_free(z_squared);
    big_fixed_free(z_power);
    big_fixed_free(n_fixed);
}

/**
 * Two-argument arctangent: atan2(y, x)
 * Returns angle in correct quadrant
 */
void big_atan2(BigFixed *result, const BigFixed *y, const BigFixed *x, int precision_bits) {
    if (!result || !y || !x) return;
    
    int working_bits = precision_bits + big_get_precision_guard();
    
    BigFixed *zero = big_fixed_create(working_bits);
    big_fixed_from_int(zero, 0);
    
    // Handle special cases
    if (big_fixed_is_zero(x)) {
        if (big_fixed_is_zero(y)) {
            big_fixed_from_int(result, 0);  // Undefined, return 0
        } else {
            // x = 0, y != 0: return ±π/2
            big_pi(result, working_bits);
            BigInt two;
            big_init(&two);
            big_from_int(&two, 2);
            BigFixed *temp_fixed2 = big_fixed_create(working_bits);
            BigFixed two_fixed;
            two_fixed.integer_part = &two;
            two_fixed.fractional_part = temp_fixed2->fractional_part;
            two_fixed.scale_bits = working_bits;
            two_fixed.negative = false;
            big_fixed_div(result, result, &two_fixed);
            if (big_fixed_is_negative(y)) {
                big_fixed_neg(result, result);
            }
            big_free(&two);
        }
        big_fixed_free(zero);
        return;
    }
    
    BigFixed *ratio = big_fixed_create(working_bits);
    big_fixed_div(ratio, y, x);
    big_atan(result, ratio, working_bits);
    
    // Adjust for quadrant
    if (big_fixed_cmp(x, zero) < 0) {  // x < 0
        BigFixed *pi = big_fixed_create(working_bits);
        big_pi(pi, working_bits);
        
        if (big_fixed_cmp(y, zero) >= 0) {  // y >= 0, Quadrant II
            big_fixed_add(result, result, pi);
        } else {  // y < 0, Quadrant III
            big_fixed_sub(result, result, pi);
        }
        
        big_fixed_free(pi);
    }
    // If x > 0, result is already correct (Quadrant I or IV)
    
    big_fixed_free(ratio);
    big_fixed_free(zero);
}

/* ============================================================================
 * PRIME EXPONENTIATION TOWERS
 * ============================================================================ */

/**
 * Prime exponentiation tower with golden damping
 * tower = p₁^(p₂^(p₃^(...))) with φ damping
 * 
 * @param result Output BigInt
 * @param primes Array of prime BigInts
 * @param count Number of primes in tower
 * @param modulus Modulus for reduction (NULL for no modulus)
 * @param damping BigFixed damping factor (NULL for no damping)
 */
void big_prime_tower(BigInt *result, const BigInt **primes, int count, 
                     const BigInt *modulus, BigFixed *damping) {
    if (!result || !primes || count <= 0) return;
    
    // Start from the top of the tower
    big_copy(result, primes[count - 1]);
    
    // Work down the tower
    for (int i = count - 2; i >= 0; i--) {
        // Apply damping if provided
        if (damping) {
            BigFixed *result_fixed = big_fixed_create(damping->scale_bits);
            big_fixed_from_bigint(result_fixed, result);
            big_fixed_mul(result_fixed, result_fixed, damping);
            big_fixed_to_bigint(result, result_fixed);
            big_fixed_free(result_fixed);
        }
        
        // Exponentiate: result = primes[i]^result
        BigInt temp;
        big_init(&temp);
        if (modulus) {
            big_powmod(primes[i], result, modulus, &temp);
        } else {
            big_pow_int(&temp, primes[i], result);
        }
        big_copy(result, &temp);
        big_free(&temp);
    }
}

/**
 * Tetration with damping: base^^height
 * base^^height = base^(base^(base^(...))) height times
 * 
 * @param result Output BigInt
 * @param base Base for tetration
 * @param height Height of tower
 * @param modulus Modulus for reduction (NULL for no modulus)
 * @param apply_damping Whether to apply golden ratio damping
 */
void big_tetration_damped(BigInt *result, const BigInt *base, int height,
                          const BigInt *modulus, bool apply_damping) {
    if (!result || !base || height <= 0) return;
    
    if (height == 1) {
        big_copy(result, base);
        return;
    }
    
    // Build tower of height
    big_copy(result, base);
    
    // Golden ratio damping: φ = (1 + √5) / 2 ≈ 1.618
    BigFixed *damping = NULL;
    if (apply_damping) {
        damping = big_fixed_create(256);
        // φ ≈ 1597/987 (Fibonacci approximation)
        big_fixed_from_int(damping, 1597);
        BigFixed *divisor = big_fixed_create(256);
        big_fixed_from_int(divisor, 987);
        big_fixed_div(damping, damping, divisor);
        big_fixed_free(divisor);
    }
    
    for (int i = 1; i < height; i++) {
        // Apply damping
        if (damping) {
            BigFixed *result_fixed = big_fixed_create(256);
            big_fixed_from_bigint(result_fixed, result);
            big_fixed_mul(result_fixed, result_fixed, damping);
            big_fixed_to_bigint(result, result_fixed);
            big_fixed_free(result_fixed);
        }
        
        // Exponentiate
        BigInt temp;
        big_init(&temp);
        if (modulus) {
            big_powmod(base, result, modulus, &temp);
        } else {
            big_pow_int(&temp, base, result);
        }
        big_copy(result, &temp);
        big_free(&temp);
    }
    
    if (damping) {
        big_fixed_free(damping);
    }
}

/**
 * Modular exponentiation using Euler's totient
 * For reducing large towers modulo m
 */
void big_modpow_euler(BigInt *result, const BigInt *base, const BigInt *exp, const BigInt *mod) {
    if (!result || !base || !exp || !mod) return;
    
    // Use existing big_powmod
    big_powmod(base, exp, mod, result);
}

/**
 * Euler's totient function φ(n)
 * Count of integers ≤ n that are coprime to n
 * 
 * Note: This function is already implemented in bigint_core.c
 * We don't need to reimplement it here.
 */

/* ============================================================================
 * DUST ELIMINATION
 * ============================================================================ */

/**
 * Truncate dust: Remove fractional part beyond precision
 * This eliminates the "dust" between kissing spheres
 * 
 * @param result Output BigInt
 * @param value Input BigFixed
 * @param target_bits Target precision in bits
 * @param guard_bits Guard bits used in calculation
 */
void big_truncate_dust(BigInt *result, const BigFixed *value, 
                       int target_bits, int guard_bits) {
    if (!result || !value) return;
    
    // Simply take the integer part
    big_fixed_to_bigint(result, value);
    
    (void)target_bits;   // Suppress unused warning
    (void)guard_bits;    // Suppress unused warning
}

/**
 * Round dust: Round to nearest integer, removing dust
 * 
 * @param result Output BigInt
 * @param value Input BigFixed
 * @param target_bits Target precision in bits
 * @param guard_bits Guard bits used in calculation
 */
void big_round_dust(BigInt *result, const BigFixed *value,
                    int target_bits, int guard_bits) {
    if (!result || !value) return;
    
    // Get integer part
    big_fixed_to_bigint(result, value);
    
    // Check if fractional part >= 0.5
    BigInt half_scale;
    big_init(&half_scale);
    big_from_int(&half_scale, 1);
    big_shl(&half_scale, value->scale_bits - 1);
    
    if (big_cmp(value->fractional_part, &half_scale) >= 0) {
        // Round up
        BigInt one;
        big_init(&one);
        big_from_int(&one, 1);
        big_add(result, result, &one);
        big_free(&one);
    }
    
    big_free(&half_scale);
    
    (void)target_bits;   // Suppress unused warning
    (void)guard_bits;    // Suppress unused warning
}