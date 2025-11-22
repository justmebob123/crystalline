/*
 * bigrational.c - Arbitrary Precision Rational Arithmetic
 * 
 * Exact rational number arithmetic using BigInt for numerator and denominator.
 * 
 * This is CRITICAL for:
 * - Exact computations (no rounding errors)
 * - Symbolic mathematics
 * - Continued fractions
 * - Diophantine equations
 * - CLLM exact gradient computations
 * 
 * All operations maintain exact precision through:
 * - Automatic simplification (GCD reduction)
 * - Proper sign handling
 * - Overflow prevention
 * 
 * All algorithms implemented WITHOUT external dependencies.
 */

#include "../include/bigint_core.h"
#include "../../include/prime_float_math.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * RATIONAL NUMBER STRUCTURE
 * ============================================================================ */

typedef struct {
    BigInt numerator;
    BigInt denominator;
} BigRational;

/* ============================================================================
 * BASIC OPERATIONS
 * ============================================================================ */

/**
 * Initialize rational number
 */
void big_rational_init(BigRational* r) {
    if (!r) return;
    big_init(&r->numerator);
    big_init(&r->denominator);
    big_from_int(&r->denominator, 1);
}

/**
 * Free rational number
 */
void big_rational_free(BigRational* r) {
    if (!r) return;
    big_free(&r->numerator);
    big_free(&r->denominator);
}

/**
 * Create rational from integers: r = num / den
 */
void big_rational_from_ints(BigRational* r, int64_t num, int64_t den) {
    if (!r) return;
    
    if (den == 0) {
        fprintf(stderr, "big_rational_from_ints: denominator cannot be zero\n");
        return;
    }
    
    big_from_int(&r->numerator, num < 0 ? -num : num);
    big_from_int(&r->denominator, den < 0 ? -den : den);
    
    r->numerator.negative = (num < 0) ^ (den < 0);
}

/**
 * Simplify rational: reduce to lowest terms
 * 
 * Algorithm:
 * 1. Compute g = gcd(numerator, denominator)
 * 2. numerator = numerator / g
 * 3. denominator = denominator / g
 * 4. Ensure denominator > 0
 */
void big_rational_simplify(BigRational* r) {
    if (!r) return;
    
    // Compute GCD
    BigInt gcd, q, rem;
    big_init(&gcd);
    big_init(&q);
    big_init(&rem);
    
    big_gcd(&gcd, &r->numerator, &r->denominator);
    
    // Divide both by GCD
    big_div(&r->numerator, &gcd, &q, &rem);
    big_copy(&r->numerator, &q);
    
    big_div(&r->denominator, &gcd, &q, &rem);
    big_copy(&r->denominator, &q);
    
    // Ensure denominator is positive
    if (r->denominator.negative) {
        r->numerator.negative = !r->numerator.negative;
        r->denominator.negative = 0;
    }
    
    big_free(&gcd);
    big_free(&q);
    big_free(&rem);
}

/* ============================================================================
 * ARITHMETIC OPERATIONS
 * ============================================================================ */

/**
 * Rational addition: result = a + b
 * 
 * Formula: a/b + c/d = (ad + bc) / (bd)
 * 
 * @param result Output: a + b
 * @param a First rational
 * @param b Second rational
 */
void big_rational_add(BigRational* result, const BigRational* a, const BigRational* b) {
    if (!result || !a || !b) return;
    
    // ad
    BigInt ad, bc, bd, temp;
    big_init(&ad);
    big_init(&bc);
    big_init(&bd);
    big_init(&temp);
    
    big_mul(&ad, &a->numerator, &b->denominator);
    
    // bc
    big_mul(&bc, &b->numerator, &a->denominator);
    
    // ad + bc
    big_add(&temp, &ad, &bc);
    big_copy(&result->numerator, &temp);
    
    // bd
    big_mul(&bd, &a->denominator, &b->denominator);
    big_copy(&result->denominator, &bd);
    
    // Simplify
    big_rational_simplify(result);
    
    big_free(&ad);
    big_free(&bc);
    big_free(&bd);
    big_free(&temp);
}

/**
 * Rational subtraction: result = a - b
 */
void big_rational_sub(BigRational* result, const BigRational* a, const BigRational* b) {
    if (!result || !a || !b) return;
    
    // Create -b
    BigRational neg_b;
    big_rational_init(&neg_b);
    big_copy(&neg_b.numerator, &b->numerator);
    big_copy(&neg_b.denominator, &b->denominator);
    neg_b.numerator.negative = !neg_b.numerator.negative;
    
    // result = a + (-b)
    big_rational_add(result, a, &neg_b);
    
    big_rational_free(&neg_b);
}

/**
 * Rational multiplication: result = a × b
 * 
 * Formula: (a/b) × (c/d) = (ac) / (bd)
 */
void big_rational_mul(BigRational* result, const BigRational* a, const BigRational* b) {
    if (!result || !a || !b) return;
    
    // ac
    big_mul(&result->numerator, &a->numerator, &b->numerator);
    
    // bd
    big_mul(&result->denominator, &a->denominator, &b->denominator);
    
    // Simplify
    big_rational_simplify(result);
}

/**
 * Rational division: result = a / b
 * 
 * Formula: (a/b) / (c/d) = (ad) / (bc)
 */
void big_rational_div(BigRational* result, const BigRational* a, const BigRational* b) {
    if (!result || !a || !b) return;
    
    // Check for division by zero
    if (big_is_zero(&b->numerator)) {
        fprintf(stderr, "big_rational_div: division by zero\n");
        return;
    }
    
    // ad
    big_mul(&result->numerator, &a->numerator, &b->denominator);
    
    // bc
    big_mul(&result->denominator, &a->denominator, &b->numerator);
    
    // Handle sign
    if (result->denominator.negative) {
        result->numerator.negative = !result->numerator.negative;
        result->denominator.negative = 0;
    }
    
    // Simplify
    big_rational_simplify(result);
}

/* ============================================================================
 * CONTINUED FRACTIONS
 * ============================================================================ */

/**
 * Compute continued fraction representation
 * 
 * Represents x as: x = a₀ + 1/(a₁ + 1/(a₂ + 1/(a₃ + ...)))
 * 
 * Algorithm:
 * 1. a₀ = ⌊x⌋
 * 2. If x = a₀, stop
 * 3. x = 1/(x - a₀)
 * 4. Repeat
 * 
 * Applications:
 * - Best rational approximations
 * - Diophantine equations
 * - Calendar calculations
 * - Musical intervals
 * 
 * Properties:
 * - Convergents give best rational approximations
 * - Truncating gives approximation with minimal denominator
 * 
 * @param cf_terms Output: continued fraction terms [a₀, a₁, a₂, ...]
 * @param num_terms Output: number of terms
 * @param r Input rational number
 * @param max_terms Maximum terms to compute
 */
void big_rational_to_continued_fraction(BigInt* cf_terms, int* num_terms,
                                       const BigRational* r, int max_terms) {
    if (!cf_terms || !num_terms || !r) return;
    
    BigRational current;
    big_rational_init(&current);
    big_copy(&current.numerator, &r->numerator);
    big_copy(&current.denominator, &r->denominator);
    
    *num_terms = 0;
    
    for (int i = 0; i < max_terms; i++) {
        // aᵢ = ⌊current⌋
        BigInt q, rem;
        big_init(&q);
        big_init(&rem);
        
        big_div(&current.numerator, &current.denominator, &q, &rem);
        big_copy(&cf_terms[i], &q);
        (*num_terms)++;
        
        // If remainder is zero, we're done
        if (big_is_zero(&rem)) {
            big_free(&q);
            big_free(&rem);
            break;
        }
        
        // current = 1 / (current - aᵢ)
        // = denominator / remainder
        big_copy(&current.numerator, &current.denominator);
        big_copy(&current.denominator, &rem);
        
        big_free(&q);
        big_free(&rem);
    }
    
    big_rational_free(&current);
}

/**
 * Compute convergent from continued fraction
 * 
 * Convergent pₙ/qₙ is the rational approximation using first n terms.
 * 
 * Algorithm:
 * p₋₁ = 1, p₀ = a₀
 * q₋₁ = 0, q₀ = 1
 * pₙ = aₙ×pₙ₋₁ + pₙ₋₂
 * qₙ = aₙ×qₙ₋₁ + qₙ₋₂
 * 
 * @param result Output: convergent pₙ/qₙ
 * @param cf_terms Continued fraction terms
 * @param num_terms Number of terms to use
 */
void big_rational_convergent(BigRational* result, const BigInt* cf_terms, int num_terms) {
    if (!result || !cf_terms || num_terms <= 0) return;
    
    BigInt p_prev, p_curr, q_prev, q_curr, temp;
    big_init(&p_prev);
    big_init(&p_curr);
    big_init(&q_prev);
    big_init(&q_curr);
    big_init(&temp);
    
    // Initialize
    big_from_int(&p_prev, 1);
    big_copy(&p_curr, &cf_terms[0]);
    big_from_int(&q_prev, 0);
    big_from_int(&q_curr, 1);
    
    // Compute convergents
    for (int i = 1; i < num_terms; i++) {
        BigInt p_next, q_next;
        big_init(&p_next);
        big_init(&q_next);
        
        // pₙ = aₙ×pₙ₋₁ + pₙ₋₂
        big_mul(&temp, &cf_terms[i], &p_curr);
        big_add(&p_next, &temp, &p_prev);
        
        // qₙ = aₙ×qₙ₋₁ + qₙ₋₂
        big_mul(&temp, &cf_terms[i], &q_curr);
        big_add(&q_next, &temp, &q_prev);
        
        // Shift
        big_copy(&p_prev, &p_curr);
        big_copy(&p_curr, &p_next);
        big_copy(&q_prev, &q_curr);
        big_copy(&q_curr, &q_next);
        
        big_free(&p_next);
        big_free(&q_next);
    }
    
    // Result
    big_copy(&result->numerator, &p_curr);
    big_copy(&result->denominator, &q_curr);
    
    big_free(&p_prev);
    big_free(&p_curr);
    big_free(&q_prev);
    big_free(&q_curr);
    big_free(&temp);
}

/**
 * Best rational approximation with bounded denominator
 * 
 * Finds p/q closest to x with q ≤ max_denominator.
 * 
 * Uses continued fractions to find optimal approximation.
 * 
 * Applications:
 * - Musical intervals (frequency ratios)
 * - Calendar calculations
 * - Gear ratios
 * - CLLM quantization
 * 
 * @param result Output: best approximation
 * @param x Target value
 * @param max_denominator Maximum allowed denominator
 */
void big_rational_best_approximation(BigRational* result, const BigRational* x,
                                     uint64_t max_denominator) {
    if (!result || !x) return;
    
    // Compute continued fraction
    BigInt cf_terms[100];
    int num_terms;
    
    for (int i = 0; i < 100; i++) {
        big_init(&cf_terms[i]);
    }
    
    big_rational_to_continued_fraction(cf_terms, &num_terms, x, 100);
    
    // Find convergent with denominator ≤ max_denominator
    for (int i = 0; i < num_terms; i++) {
        BigRational convergent;
        big_rational_init(&convergent);
        big_rational_convergent(&convergent, cf_terms, i + 1);
        
        // Check denominator
        uint64_t den_val = 0;
        if (convergent.denominator.len == 1) {
            den_val = convergent.denominator.d[0];
        } else if (convergent.denominator.len == 2) {
            den_val = ((uint64_t)convergent.denominator.d[1] << 32) | 
                      convergent.denominator.d[0];
        }
        
        if (den_val <= max_denominator) {
            big_copy(&result->numerator, &convergent.numerator);
            big_copy(&result->denominator, &convergent.denominator);
        } else {
            big_rational_free(&convergent);
            break;
        }
        
        big_rational_free(&convergent);
    }
    
    // Cleanup
    for (int i = 0; i < 100; i++) {
        big_free(&cf_terms[i]);
    }
}

/* ============================================================================
 * CONVERSION FUNCTIONS
 * ============================================================================ */

/**
 * Convert rational to double
 */
double big_rational_to_double(const BigRational* r) {
    if (!r) return 0.0;
    
    // Convert numerator and denominator to double
    // This is approximate for large numbers
    double num = 0.0;
    double den = 0.0;
    
    for (size_t i = 0; i < r->numerator.len && i < 2; i++) {
        num += r->numerator.d[i] * prime_pow(2.0, 32.0 * i);
    }
    
    for (size_t i = 0; i < r->denominator.len && i < 2; i++) {
        den += r->denominator.d[i] * prime_pow(2.0, 32.0 * i);
    }
    
    if (r->numerator.negative) num = -num;
    
    return num / den;
}

/**
 * Convert double to rational (exact within precision)
 * 
 * Uses continued fractions to find exact representation.
 */
void big_rational_from_double(BigRational* r, double x, uint64_t max_denominator) {
    if (!r) return;
    
    // Handle special cases
    if (x == 0.0) {
        big_from_int(&r->numerator, 0);
        big_from_int(&r->denominator, 1);
        return;
    }
    
    // Extract sign
    int negative = (x < 0);
    if (negative) x = -x;
    
    // Continued fraction algorithm
    int64_t a0 = (int64_t)x;
    big_from_int(&r->numerator, a0);
    big_from_int(&r->denominator, 1);
    
    double frac = x - a0;
    
    BigInt p_prev, p_curr, q_prev, q_curr;
    big_init(&p_prev);
    big_init(&p_curr);
    big_init(&q_prev);
    big_init(&q_curr);
    
    big_from_int(&p_prev, 1);
    big_from_int(&p_curr, a0);
    big_from_int(&q_prev, 0);
    big_from_int(&q_curr, 1);
    
    int max_iterations = 50;
    for (int i = 0; i < max_iterations && frac > 1e-10; i++) {
        frac = 1.0 / frac;
        int64_t a = (int64_t)frac;
        frac = frac - a;
        
        // pₙ = aₙ×pₙ₋₁ + pₙ₋₂
        BigInt p_next, q_next, temp, a_bigint;
        big_init(&p_next);
        big_init(&q_next);
        big_init(&temp);
        big_init(&a_bigint);
        big_from_int(&a_bigint, a);
        
        big_mul(&temp, &a_bigint, &p_curr);
        big_add(&p_next, &temp, &p_prev);
        
        // qₙ = aₙ×qₙ₋₁ + qₙ₋₂
        big_mul(&temp, &a_bigint, &q_curr);
        big_add(&q_next, &temp, &q_prev);
        
        // Check denominator limit
        uint64_t q_val = 0;
        if (q_next.len == 1) {
            q_val = q_next.d[0];
        } else if (q_next.len == 2) {
            q_val = ((uint64_t)q_next.d[1] << 32) | q_next.d[0];
        } else {
            q_val = max_denominator + 1;
        }
        
        if (q_val > max_denominator) {
            big_free(&p_next);
            big_free(&q_next);
            big_free(&temp);
            big_free(&a_bigint);
            break;
        }
        
        // Update
        big_copy(&p_prev, &p_curr);
        big_copy(&p_curr, &p_next);
        big_copy(&q_prev, &q_curr);
        big_copy(&q_curr, &q_next);
        
        big_free(&p_next);
        big_free(&q_next);
        big_free(&temp);
        big_free(&a_bigint);
    }
    
    big_copy(&r->numerator, &p_curr);
    big_copy(&r->denominator, &q_curr);
    
    if (negative) {
        r->numerator.negative = 1;
    }
    
    big_free(&p_prev);
    big_free(&p_curr);
    big_free(&q_prev);
    big_free(&q_curr);
}

/**
 * Print rational number
 */
void big_rational_print(const BigRational* r) {
    if (!r) return;
    
    char* num_str = big_to_string(&r->numerator);
    char* den_str = big_to_string(&r->denominator);
    
    if (num_str && den_str) {
        printf("%s / %s", num_str, den_str);
    }
    
    free(num_str);
    free(den_str);
}
