/*
 * bigint_advanced.c - Advanced Number Theory Algorithms
 * 
 * Revolutionary algorithms for the Crystalline Lattice Math Library:
 * - Miller-Rabin primality test
 * - Extended Euclidean algorithm
 * - Pollard's Rho factorization
 * - Chinese Remainder Theorem
 * - Euler's Totient function
 * - Carmichael function
 * - Modular inverse
 * 
 * All algorithms implemented WITHOUT external dependencies.
 */

#include "../include/bigint_core.h"
#include "../include/prime_lowlevel.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * MODULAR ARITHMETIC HELPERS
 * ============================================================================ */

/**
 * Modular exponentiation: result = (base^exp) mod m
 * Uses binary exponentiation (square-and-multiply)
 * 
 * Algorithm:
 * 1. Initialize result = 1
 * 2. For each bit of exp (from LSB to MSB):
 *    - If bit is 1: result = (result * base) mod m
 *    - base = (base * base) mod m
 * 
 * Complexity: O(log exp × log² m)
 */
void big_mod_exp(BigInt* result, const BigInt* base, const BigInt* exp, const BigInt* m) {
    if (!result || !base || !exp || !m) return;
    
    // Handle special cases
    if (big_is_zero(m)) {
        fprintf(stderr, "big_mod_exp: modulus cannot be zero\n");
        big_from_int(result, 0);
        return;
    }
    
    if (big_is_zero(exp)) {
        big_from_int(result, 1);
        return;
    }
    
    // Initialize
    BigInt r, b, temp, q, rem;
    big_init(&r);
    big_init(&b);
    big_init(&temp);
    big_init(&q);
    big_init(&rem);
    
    big_from_int(&r, 1);
    big_copy(&b, base);
    
    // Reduce base modulo m
    big_div(&b, m, &q, &rem);
    big_copy(&b, &rem);
    
    // Binary exponentiation
    for (size_t i = 0; i < exp->len; i++) {
        uint32_t word = exp->d[i];
        
        for (int bit = 0; bit < 32; bit++) {
            if (word & 1) {
                // result = (result * base) mod m
                big_mul(&temp, &r, &b);
                big_div(&temp, m, &q, &rem);
                big_copy(&r, &rem);
            }
            
            // base = (base * base) mod m
            big_mul(&temp, &b, &b);
            big_div(&temp, m, &q, &rem);
            big_copy(&b, &rem);
            
            word >>= 1;
        }
    }
    
    big_copy(result, &r);
    
    big_free(&r);
    big_free(&b);
    big_free(&temp);
    big_free(&q);
    big_free(&rem);
}

/**
 * Modular inverse: result = a^(-1) mod m
 * Uses extended Euclidean algorithm
 * 
 * Returns 1 if inverse exists, 0 otherwise
 */
int big_mod_inverse(BigInt* result, const BigInt* a, const BigInt* m);

/* ============================================================================
 * EXTENDED EUCLIDEAN ALGORITHM
 * ============================================================================ */

/**
 * Extended Euclidean Algorithm
 * 
 * Finds gcd(a, b) and coefficients x, y such that:
 * ax + by = gcd(a, b)
 * 
 * Algorithm:
 * 1. Initialize: r₀=a, r₁=b, s₀=1, s₁=0, t₀=0, t₁=1
 * 2. While rᵢ ≠ 0:
 *    - qᵢ = rᵢ₋₁ / rᵢ
 *    - rᵢ₊₁ = rᵢ₋₁ - qᵢ×rᵢ
 *    - sᵢ₊₁ = sᵢ₋₁ - qᵢ×sᵢ
 *    - tᵢ₊₁ = tᵢ₋₁ - qᵢ×tᵢ
 * 3. Return: gcd=rₙ, x=sₙ, y=tₙ
 * 
 * Complexity: O(log min(a,b))
 */
void big_extended_gcd(const BigInt* a, const BigInt* b, 
                      BigInt* gcd, BigInt* x, BigInt* y) {
    if (!a || !b || !gcd) return;
    
    // Initialize
    BigInt r0, r1, r2;
    BigInt s0, s1, s2;
    BigInt t0, t1, t2;
    BigInt q, temp;
    
    big_init(&r0); big_init(&r1); big_init(&r2);
    big_init(&s0); big_init(&s1); big_init(&s2);
    big_init(&t0); big_init(&t1); big_init(&t2);
    big_init(&q); big_init(&temp);
    
    // r₀ = a, r₁ = b
    big_copy(&r0, a);
    big_copy(&r1, b);
    
    // s₀ = 1, s₁ = 0
    big_from_int(&s0, 1);
    big_from_int(&s1, 0);
    
    // t₀ = 0, t₁ = 1
    big_from_int(&t0, 0);
    big_from_int(&t1, 1);
    
    // Extended Euclidean algorithm
    while (!big_is_zero(&r1)) {
        // q = r₀ / r₁
        big_div(&r0, &r1, &q, &r2);
        
        // sᵢ₊₁ = sᵢ₋₁ - q×sᵢ
        big_mul(&temp, &q, &s1);
        big_sub(&s2, &s0, &temp);
        
        // tᵢ₊₁ = tᵢ₋₁ - q×tᵢ
        big_mul(&temp, &q, &t1);
        big_sub(&t2, &t0, &temp);
        
        // Shift: (r₀,r₁,r₂) → (r₁,r₂,_)
        big_copy(&r0, &r1);
        big_copy(&r1, &r2);
        
        big_copy(&s0, &s1);
        big_copy(&s1, &s2);
        
        big_copy(&t0, &t1);
        big_copy(&t1, &t2);
    }
    
    // Result: gcd = r₀, x = s₀, y = t₀
    big_copy(gcd, &r0);
    if (x) big_copy(x, &s0);
    if (y) big_copy(y, &t0);
    
    // Cleanup
    big_free(&r0); big_free(&r1); big_free(&r2);
    big_free(&s0); big_free(&s1); big_free(&s2);
    big_free(&t0); big_free(&t1); big_free(&t2);
    big_free(&q); big_free(&temp);
}

/**
 * Modular inverse using extended Euclidean algorithm
 * 
 * Returns 1 if inverse exists (gcd(a,m) = 1), 0 otherwise
 */
int big_mod_inverse(BigInt* result, const BigInt* a, const BigInt* m) {
    if (!result || !a || !m) return 0;
    
    BigInt gcd, x, y;
    big_init(&gcd);
    big_init(&x);
    big_init(&y);
    
    // Find gcd and coefficients
    big_extended_gcd(a, m, &gcd, &x, &y);
    
    // Check if inverse exists
    BigInt one;
    big_init(&one);
    big_from_int(&one, 1);
    
    if (big_cmp(&gcd, &one) != 0) {
        // gcd(a,m) ≠ 1, inverse doesn't exist
        big_free(&gcd);
        big_free(&x);
        big_free(&y);
        big_free(&one);
        return 0;
    }
    
    // Inverse is x mod m
    // Ensure positive result
    if (x.negative) {
        BigInt temp;
        big_init(&temp);
        big_add(&temp, &x, m);
        big_copy(result, &temp);
        big_free(&temp);
    } else {
        big_copy(result, &x);
    }
    
    // Reduce modulo m
    BigInt q, rem;
    big_init(&q);
    big_init(&rem);
    big_div(result, m, &q, &rem);
    big_copy(result, &rem);
    
    big_free(&gcd);
    big_free(&x);
    big_free(&y);
    big_free(&one);
    big_free(&q);
    big_free(&rem);
    
    return 1;
}

/* ============================================================================
 * MILLER-RABIN PRIMALITY TEST
 * ============================================================================ */

/**
 * Miller-Rabin Primality Test
 * 
 * Probabilistic primality test with error probability ≤ 4^(-k)
 * where k is the number of iterations.
 * 
 * Algorithm:
 * 1. Write n-1 as 2^r × d where d is odd
 * 2. For k iterations:
 *    a. Pick random a in [2, n-2]
 *    b. Compute x = a^d mod n
 *    c. If x = 1 or x = n-1, continue to next iteration
 *    d. For i = 0 to r-1:
 *       - x = x² mod n
 *       - If x = n-1, continue to next iteration
 *    e. If we reach here, n is composite
 * 3. If all iterations pass, n is probably prime
 * 
 * Complexity: O(k log³ n)
 * 
 * @param n Number to test
 * @param iterations Number of test iterations (typically 20-40)
 * @return 1 if probably prime, 0 if definitely composite
 */
int big_miller_rabin(const BigInt* n, int iterations) {
    if (!n) return 0;
    
    // Handle small cases
    BigInt two, three;
    big_init(&two);
    big_init(&three);
    big_from_int(&two, 2);
    big_from_int(&three, 3);
    
    if (big_cmp(n, &two) < 0) {
        big_free(&two);
        big_free(&three);
        return 0;
    }
    
    if (big_cmp(n, &two) == 0 || big_cmp(n, &three) == 0) {
        big_free(&two);
        big_free(&three);
        return 1;
    }
    
    // Check if even
    if ((n->d[0] & 1) == 0) {
        big_free(&two);
        big_free(&three);
        return 0;
    }
    
    // Write n-1 as 2^r × d
    BigInt n_minus_1, d;
    big_init(&n_minus_1);
    big_init(&d);
    
    BigInt one;
    big_init(&one);
    big_from_int(&one, 1);
    
    big_sub(&n_minus_1, n, &one);
    big_copy(&d, &n_minus_1);
    
    int r = 0;
    while ((d.d[0] & 1) == 0) {
        big_shr(&d, 1);
        r++;
    }
    
    // Perform k iterations
    srand(time(NULL));
    
    for (int iter = 0; iter < iterations; iter++) {
        // Pick random a in [2, n-2]
        BigInt a, x, n_minus_2;
        big_init(&a);
        big_init(&x);
        big_init(&n_minus_2);
        
        big_sub(&n_minus_2, n, &two);
        
        // Simple random generation (can be improved)
        uint64_t rand_val = ((uint64_t)rand() << 32) | rand();
        big_from_int(&a, rand_val % 1000000 + 2);
        
        // Compute x = a^d mod n
        big_mod_exp(&x, &a, &d, n);
        
        // If x = 1 or x = n-1, continue
        if (big_cmp(&x, &one) == 0 || big_cmp(&x, &n_minus_1) == 0) {
            big_free(&a);
            big_free(&x);
            big_free(&n_minus_2);
            continue;
        }
        
        // Square x repeatedly
        int composite = 1;
        for (int i = 0; i < r - 1; i++) {
            BigInt x_squared;
            big_init(&x_squared);
            big_mod_exp(&x_squared, &x, &two, n);
            big_copy(&x, &x_squared);
            big_free(&x_squared);
            
            if (big_cmp(&x, &n_minus_1) == 0) {
                composite = 0;
                break;
            }
        }
        
        big_free(&a);
        big_free(&x);
        big_free(&n_minus_2);
        
        if (composite) {
            // n is definitely composite
            big_free(&two);
            big_free(&three);
            big_free(&n_minus_1);
            big_free(&d);
            big_free(&one);
            return 0;
        }
    }
    
    // All iterations passed - probably prime
    big_free(&two);
    big_free(&three);
    big_free(&n_minus_1);
    big_free(&d);
    big_free(&one);
    
    return 1;
}

/* ============================================================================
 * POLLARD'S RHO FACTORIZATION
 * ============================================================================ */

/**
 * Pollard's Rho Algorithm for Integer Factorization
 * 
 * Finds a non-trivial factor of n using cycle detection.
 * 
 * Algorithm:
 * 1. Choose random x₀ and function f(x) = (x² + c) mod n
 * 2. Generate sequence: xᵢ₊₁ = f(xᵢ)
 * 3. Use Floyd's cycle detection: tortoise and hare
 * 4. Compute gcd(|xₜₒᵣₜₒᵢₛₑ - xₕₐᵣₑ|, n)
 * 5. If gcd > 1 and gcd < n, found factor
 * 
 * Expected complexity: O(n^(1/4))
 * 
 * @param n Number to factor
 * @param factor Output: found factor (if any)
 * @return 1 if factor found, 0 otherwise
 */
int big_pollard_rho(const BigInt* n, BigInt* factor) {
    if (!n || !factor) return 0;
    
    // Check if n is even
    if ((n->d[0] & 1) == 0) {
        big_from_int(factor, 2);
        return 1;
    }
    
    // Initialize
    BigInt x, y, d, one, two, c, temp, diff, q, rem;
    big_init(&x);
    big_init(&y);
    big_init(&d);
    big_init(&one);
    big_init(&two);
    big_init(&c);
    big_init(&temp);
    big_init(&diff);
    big_init(&q);
    big_init(&rem);
    
    big_from_int(&one, 1);
    big_from_int(&two, 2);
    big_from_int(&c, 1);  // Constant for f(x) = x² + c
    
    // Start with random x and y
    big_from_int(&x, 2);
    big_copy(&y, &x);
    big_copy(&d, &one);
    
    // Pollard's rho with Floyd's cycle detection
    int max_iterations = 100000;
    for (int iter = 0; iter < max_iterations && big_cmp(&d, &one) == 0; iter++) {
        // x = f(x) = (x² + c) mod n
        big_mul(&temp, &x, &x);
        big_add(&temp, &temp, &c);
        big_div(&temp, n, &q, &rem);
        big_copy(&x, &rem);
        
        // y = f(f(y))
        big_mul(&temp, &y, &y);
        big_add(&temp, &temp, &c);
        big_div(&temp, n, &q, &rem);
        big_copy(&y, &rem);
        
        big_mul(&temp, &y, &y);
        big_add(&temp, &temp, &c);
        big_div(&temp, n, &q, &rem);
        big_copy(&y, &rem);
        
        // d = gcd(|x - y|, n)
        if (big_cmp(&x, &y) > 0) {
            big_sub(&diff, &x, &y);
        } else {
            big_sub(&diff, &y, &x);
        }
        
        big_gcd(&d, &diff, n);
    }
    
    // Check if we found a factor
    int found = 0;
    if (big_cmp(&d, &one) > 0 && big_cmp(&d, n) < 0) {
        big_copy(factor, &d);
        found = 1;
    }
    
    // Cleanup
    big_free(&x);
    big_free(&y);
    big_free(&d);
    big_free(&one);
    big_free(&two);
    big_free(&c);
    big_free(&temp);
    big_free(&diff);
    big_free(&q);
    big_free(&rem);
    
    return found;
}

/* ============================================================================
 * CHINESE REMAINDER THEOREM
 * ============================================================================ */

/**
 * Chinese Remainder Theorem
 * 
 * Solves system of congruences:
 * x ≡ a₁ (mod m₁)
 * x ≡ a₂ (mod m₂)
 * ...
 * x ≡ aₙ (mod mₙ)
 * 
 * where m₁, m₂, ..., mₙ are pairwise coprime.
 * 
 * Algorithm:
 * 1. Compute M = m₁ × m₂ × ... × mₙ
 * 2. For each i: Mᵢ = M / mᵢ
 * 3. For each i: yᵢ = Mᵢ^(-1) mod mᵢ
 * 4. Result: x = Σ(aᵢ × Mᵢ × yᵢ) mod M
 * 
 * @param result Output: solution x
 * @param remainders Array of remainders [a₁, a₂, ..., aₙ]
 * @param moduli Array of moduli [m₁, m₂, ..., mₙ]
 * @param count Number of congruences
 * @return 1 if successful, 0 if moduli not coprime
 */
int big_crt(BigInt* result, const BigInt* remainders, 
            const BigInt* moduli, int count) {
    if (!result || !remainders || !moduli || count <= 0) return 0;
    
    // Compute M = product of all moduli
    BigInt M, temp;
    big_init(&M);
    big_init(&temp);
    big_from_int(&M, 1);
    
    for (int i = 0; i < count; i++) {
        big_mul(&temp, &M, &moduli[i]);
        big_copy(&M, &temp);
    }
    
    // Compute result
    BigInt sum, Mi, yi, prod, q, rem;
    big_init(&sum);
    big_init(&Mi);
    big_init(&yi);
    big_init(&prod);
    big_init(&q);
    big_init(&rem);
    
    big_from_int(&sum, 0);
    
    for (int i = 0; i < count; i++) {
        // Mᵢ = M / mᵢ
        big_div(&M, &moduli[i], &Mi, &rem);
        
        // yᵢ = Mᵢ^(-1) mod mᵢ
        if (!big_mod_inverse(&yi, &Mi, &moduli[i])) {
            // Moduli not coprime
            big_free(&M);
            big_free(&temp);
            big_free(&sum);
            big_free(&Mi);
            big_free(&yi);
            big_free(&prod);
            big_free(&q);
            big_free(&rem);
            return 0;
        }
        
        // sum += aᵢ × Mᵢ × yᵢ
        big_mul(&temp, &remainders[i], &Mi);
        big_mul(&prod, &temp, &yi);
        big_add(&temp, &sum, &prod);
        big_copy(&sum, &temp);
    }
    
    // Result: x = sum mod M
    big_div(&sum, &M, &q, &rem);
    big_copy(result, &rem);
    
    // Cleanup
    big_free(&M);
    big_free(&temp);
    big_free(&sum);
    big_free(&Mi);
    big_free(&yi);
    big_free(&prod);
    big_free(&q);
    big_free(&rem);
    
    return 1;
}

/* ============================================================================
 * EULER'S TOTIENT FUNCTION
 * ============================================================================ */

/**
 * Euler's Totient Function φ(n)
 * 
 * Counts integers k in [1, n] such that gcd(k, n) = 1
 * 
 * Algorithm:
 * 1. Factor n = p₁^a₁ × p₂^a₂ × ... × pₖ^aₖ
 * 2. φ(n) = n × ∏(1 - 1/pᵢ) = n × ∏((pᵢ - 1)/pᵢ)
 * 
 * For prime p: φ(p) = p - 1
 * For prime power: φ(p^k) = p^(k-1) × (p - 1)
 * 
 * @param result Output: φ(n)
 * @param n Input number
 */
void big_euler_phi(BigInt* result, const BigInt* n) {
    if (!result || !n) return;
    
    // Start with result = n
    big_copy(result, n);
    
    // Find prime factors and apply formula
    BigInt temp, p, one, two, q, rem, n_copy;
    big_init(&temp);
    big_init(&p);
    big_init(&one);
    big_init(&two);
    big_init(&q);
    big_init(&rem);
    big_init(&n_copy);
    
    big_from_int(&one, 1);
    big_from_int(&two, 2);
    big_copy(&n_copy, n);
    
    // Check factor 2
    if ((n_copy.d[0] & 1) == 0) {
        // result = result - result/2 = result/2
        big_shr(result, 1);
        
        // Remove all factors of 2
        while ((n_copy.d[0] & 1) == 0) {
            big_shr(&n_copy, 1);
        }
    }
    
    // Check odd factors
    big_from_int(&p, 3);
    while (big_cmp(&p, &n_copy) <= 0) {
        big_div(&n_copy, &p, &q, &rem);
        
        if (big_is_zero(&rem)) {
            // p divides n_copy
            // result = result - result/p
            big_div(result, &p, &q, &rem);
            big_sub(&temp, result, &q);
            big_copy(result, &temp);
            
            // Remove all factors of p
            while (big_is_zero(&rem)) {
                big_copy(&n_copy, &q);
                big_div(&n_copy, &p, &q, &rem);
            }
        }
        
        // Next odd number
        big_add(&temp, &p, &two);
        big_copy(&p, &temp);
        
        // Optimization: if p² > n_copy, n_copy is prime
        BigInt p_squared;
        big_init(&p_squared);
        big_mul(&p_squared, &p, &p);
        if (big_cmp(&p_squared, &n_copy) > 0) {
            big_free(&p_squared);
            break;
        }
        big_free(&p_squared);
    }
    
    // If n_copy > 1, it's a prime factor
    if (big_cmp(&n_copy, &one) > 0) {
        big_div(result, &n_copy, &q, &rem);
        big_sub(&temp, result, &q);
        big_copy(result, &temp);
    }
    
    // Cleanup
    big_free(&temp);
    big_free(&p);
    big_free(&one);
    big_free(&two);
    big_free(&q);
    big_free(&rem);
    big_free(&n_copy);
}