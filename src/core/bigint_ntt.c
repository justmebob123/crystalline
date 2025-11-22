/*
 * bigint_ntt.c - Number Theoretic Transform Implementation
 * 
 * Fast multiplication using NTT instead of FFT.
 * Perfect for crystalline lattice prime-based system.
 * 
 * Key Innovation: Uses modular arithmetic (no complex numbers)
 * Complexity: O(n log n) for multiplication
 */

#include "../../include/bigint_ntt.h"
#include "../../include/bigint_core.h"
#include "../../include/bigint_advanced.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * Check if n is a power of 2
 */
int ntt_is_power_of_2(size_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

/**
 * Compute log2 of n (assuming n is power of 2)
 */
int ntt_log2(size_t n) {
    int log = 0;
    while (n > 1) {
        n >>= 1;
        log++;
    }
    return log;
}

/**
 * Round up to next power of 2
 */
size_t ntt_next_power_of_2(size_t n) {
    if (n == 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

/**
 * Bit-reverse permutation
 */
void ntt_bit_reverse(BigInt* array, size_t n) {
    if (!array || !ntt_is_power_of_2(n)) return;
    
    int bits = ntt_log2(n);
    
    for (size_t i = 0; i < n; i++) {
        size_t j = 0;
        size_t temp = i;
        
        // Reverse bits
        for (int b = 0; b < bits; b++) {
            j = (j << 1) | (temp & 1);
            temp >>= 1;
        }
        
        // Swap if i < j (to avoid double swap)
        if (i < j) {
            BigInt tmp;
            big_init(&tmp);
            big_copy(&tmp, &array[i]);
            big_copy(&array[i], &array[j]);
            big_copy(&array[j], &tmp);
            big_free(&tmp);
        }
    }
}

/* ============================================================================
 * PRIMITIVE ROOT FINDING
 * ============================================================================ */

/**
 * Check if g is a primitive root modulo p
 * 
 * A primitive root g generates all elements of (Z/pZ)*
 */
int ntt_is_primitive_root(const BigInt* g, const BigInt* p) {
    if (!g || !p) return 0;
    
    // Compute p-1
    BigInt p_minus_1, one, result, exponent;
    big_init(&p_minus_1);
    big_init(&one);
    big_init(&result);
    big_init(&exponent);
    
    big_from_int(&one, 1);
    big_copy(&p_minus_1, p);
    big_sub(&p_minus_1, &p_minus_1, &one);
    
    // Factor p-1 to get prime factors
    // For simplicity, we'll test a few small prime divisors
    // In production, use complete factorization
    
    uint64_t small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31};
    int num_primes = sizeof(small_primes) / sizeof(small_primes[0]);
    
    for (int i = 0; i < num_primes; i++) {
        BigInt prime_divisor, quotient, remainder;
        big_init(&prime_divisor);
        big_init(&quotient);
        big_init(&remainder);
        
        big_from_int(&prime_divisor, small_primes[i]);
        
        // Check if this prime divides p-1
        big_div(&p_minus_1, &prime_divisor, &quotient, &remainder);
        
        if (big_is_zero(&remainder)) {
            // Compute g^((p-1)/prime) mod p
            big_mod_exp(&result, g, &quotient, p);
            
            // If result == 1, then g is not a primitive root
            if (big_cmp(&result, &one) == 0) {
                big_free(&p_minus_1);
                big_free(&one);
                big_free(&result);
                big_free(&exponent);
                big_free(&prime_divisor);
                big_free(&quotient);
                big_free(&remainder);
                return 0;
            }
        }
        
        big_free(&prime_divisor);
        big_free(&quotient);
        big_free(&remainder);
    }
    
    big_free(&p_minus_1);
    big_free(&one);
    big_free(&result);
    big_free(&exponent);
    
    return 1;
}

/**
 * Find primitive n-th root of unity modulo prime p
 */
int ntt_find_primitive_root(BigInt* root, size_t n, const BigInt* p) {
    if (!root || !p || n == 0) return 0;
    // Special case for p = 65537 (use known generator 3)
    BigInt p_val;
    big_init(&p_val);
    big_from_int(&p_val, 65537);
    if (big_cmp(p, &p_val) == 0) {
        BigInt p_minus_1, n_bigint, quotient, remainder, generator;
        big_init(&p_minus_1);
        big_init(&n_bigint);
        big_init(&quotient);
        big_init(&remainder);
        big_init(&generator);
        
        BigInt one;
        big_init(&one);
        big_from_int(&one, 1);
        
        big_copy(&p_minus_1, p);
        big_sub(&p_minus_1, &p_minus_1, &one);
        big_from_int(&n_bigint, n);
        
        big_div(&p_minus_1, &n_bigint, &quotient, &remainder);
        
        if (big_is_zero(&remainder)) {
            // Use generator 3 for p = 65537
            big_from_int(&generator, 3);
            big_mod_exp(root, &generator, &quotient, p);
            
            big_free(&p_minus_1);
            big_free(&n_bigint);
            big_free(&quotient);
            big_free(&remainder);
            big_free(&generator);
            big_free(&one);
            big_free(&p_val);
            return 1;
        }
        
        big_free(&p_minus_1);
        big_free(&n_bigint);
        big_free(&quotient);
        big_free(&remainder);
        big_free(&generator);
        big_free(&one);
    }
    big_free(&p_val);

    
    // Ensure n divides p-1
    BigInt p_minus_1, n_bigint, quotient, remainder;
    big_init(&p_minus_1);
    big_init(&n_bigint);
    big_init(&quotient);
    big_init(&remainder);
    
    BigInt one;
    big_init(&one);
    big_from_int(&one, 1);
    
    big_copy(&p_minus_1, p);
    big_sub(&p_minus_1, &p_minus_1, &one);
    big_from_int(&n_bigint, n);
    
    big_div(&p_minus_1, &n_bigint, &quotient, &remainder);
    
    if (!big_is_zero(&remainder)) {
        // n does not divide p-1
        big_free(&p_minus_1);
        big_free(&n_bigint);
        big_free(&quotient);
        big_free(&remainder);
        big_free(&one);
        return 0;
    }
    
    // Try small values as generators
    for (uint64_t g = 2; g < 100; g++) {
        BigInt generator, candidate;
        big_init(&generator);
        big_init(&candidate);
        
        big_from_int(&generator, g);
        
        // Compute ω = g^((p-1)/n) mod p
        big_mod_exp(&candidate, &generator, &quotient, p);
        
        // Check if ω^n ≡ 1 (mod p)
        BigInt test;
        big_init(&test);
        big_mod_exp(&test, &candidate, &n_bigint, p);
        
        if (big_cmp(&test, &one) == 0) {
            // Check if it's a primitive n-th root
            // (i.e., ω^k ≢ 1 for 0 < k < n)
            int is_primitive = 1;
            
            for (size_t k = 1; k < n; k++) {
                if (n % k == 0 && k != n) {
                    BigInt k_bigint, test_k;
                    big_init(&k_bigint);
                    big_init(&test_k);
                    
                    big_from_int(&k_bigint, k);
                    big_mod_exp(&test_k, &candidate, &k_bigint, p);
                    
                    if (big_cmp(&test_k, &one) == 0) {
                        is_primitive = 0;
                        big_free(&k_bigint);
                        big_free(&test_k);
                        break;
                    }
                    
                    big_free(&k_bigint);
                    big_free(&test_k);
                }
            }
            
            if (is_primitive) {
                big_copy(root, &candidate);
                big_free(&generator);
                big_free(&candidate);
                big_free(&test);
                big_free(&p_minus_1);
                big_free(&n_bigint);
                big_free(&quotient);
                big_free(&remainder);
                big_free(&one);
                return 1;
            }
        }
        
        big_free(&generator);
        big_free(&candidate);
        big_free(&test);
    }
    
    big_free(&p_minus_1);
    big_free(&n_bigint);
    big_free(&quotient);
    big_free(&remainder);
    big_free(&one);
    
    return 0;
}

/**
 * Find suitable NTT prime
 * 
 * Finds prime p of form p = k·2^m + 1 where m >= log2(n)
 */
int ntt_find_prime(BigInt* prime, size_t n, int bits) {
    if (!prime || n == 0 || bits < 16) return 0;
    
    // Find m such that 2^m >= n
    int m = ntt_log2(ntt_next_power_of_2(n));
    // For small n, use known NTT-friendly primes
    if (n <= 16 && bits <= 64) {
        // Use a known prime: 65537 = 2^16 + 1 (works for n up to 2^16)
        big_from_int(prime, 65537);
        return 1;
    }

    
    // Start with 2^m
    BigInt two_pow_m, k, candidate, one, two;
    big_init(&two_pow_m);
    big_init(&k);
    big_init(&candidate);
    big_init(&one);
    big_init(&two);
    
    big_from_int(&one, 1);
    big_from_int(&two, 2);
    big_from_int(&two_pow_m, 1);
    
    // Compute 2^m
    for (int i = 0; i < m; i++) {
        BigInt temp;
        big_init(&temp);
        big_mul(&temp, &two_pow_m, &two);
        big_copy(&two_pow_m, &temp);
        big_free(&temp);
    }
    
    // Try k values until we find a prime
    for (uint64_t k_val = 1; k_val < 100000; k_val++) {
        big_from_int(&k, k_val);
        
        // candidate = k * 2^m + 1
        big_mul(&candidate, &k, &two_pow_m);
        big_add(&candidate, &candidate, &one);
        
        // Check if candidate has desired bit size
        if (candidate.len * 32 >= (size_t)bits - 32 && 
            candidate.len * 32 <= (size_t)bits + 32) {
            
            // Test primality with Miller-Rabin
            if (big_miller_rabin(&candidate, 5)) {
                big_copy(prime, &candidate);
                big_free(&two_pow_m);
                big_free(&k);
                big_free(&candidate);
                big_free(&one);
                big_free(&two);
                return 1;
            }
        }
    }
    
    big_free(&two_pow_m);
    big_free(&k);
    big_free(&candidate);
    big_free(&one);
    big_free(&two);
    
    return 0;
}

/* ============================================================================
 * NTT CONTEXT INITIALIZATION
 * ============================================================================ */

/**
 * Initialize NTT context
 */
int ntt_init(NTTContext* ctx, size_t n) {
    if (!ctx || !ntt_is_power_of_2(n)) return 0;
    
    ctx->n = n;
    ctx->initialized = 0;
    
    // Initialize BigInts
    big_init(&ctx->prime);
    big_init(&ctx->root);
    
    // Find suitable prime (use 64-bit prime for efficiency)
    if (!ntt_find_prime(&ctx->prime, n, 64)) {
        fprintf(stderr, "ntt_init: failed to find suitable prime\n");
        return 0;
    }
    
    // Find primitive n-th root of unity
    if (!ntt_find_primitive_root(&ctx->root, n, &ctx->prime)) {
        fprintf(stderr, "ntt_init: failed to find primitive root\n");
        big_free(&ctx->prime);
        big_free(&ctx->root);
        return 0;
    }
    
    // Precompute roots for forward NTT
    ctx->roots_forward = malloc(n * sizeof(BigInt));
    if (!ctx->roots_forward) {
        big_free(&ctx->prime);
        big_free(&ctx->root);
        return 0;
    }
    
    // Compute powers of root: ω^0, ω^1, ω^2, ..., ω^(n-1)
    for (size_t i = 0; i < n; i++) {
        big_init(&ctx->roots_forward[i]);
        
        if (i == 0) {
            big_from_int(&ctx->roots_forward[i], 1);
        } else {
            BigInt i_bigint;
            big_init(&i_bigint);
            big_from_int(&i_bigint, i);
            big_mod_exp(&ctx->roots_forward[i], &ctx->root, &i_bigint, &ctx->prime);
            big_free(&i_bigint);
        }
    }
    
    // Precompute roots for inverse NTT
    ctx->roots_inverse = malloc(n * sizeof(BigInt));
    if (!ctx->roots_inverse) {
        for (size_t i = 0; i < n; i++) {
            big_free(&ctx->roots_forward[i]);
        }
        free(ctx->roots_forward);
        big_free(&ctx->prime);
        big_free(&ctx->root);
        return 0;
    }
    
    // Compute inverse root: ω^(-1) = ω^(n-1) mod p
    BigInt root_inv;
    big_init(&root_inv);
    big_mod_inverse(&root_inv, &ctx->root, &ctx->prime);
    
    // Compute powers of inverse root
    for (size_t i = 0; i < n; i++) {
        big_init(&ctx->roots_inverse[i]);
        
        if (i == 0) {
            big_from_int(&ctx->roots_inverse[i], 1);
        } else {
            BigInt i_bigint;
            big_init(&i_bigint);
            big_from_int(&i_bigint, i);
            big_mod_exp(&ctx->roots_inverse[i], &root_inv, &i_bigint, &ctx->prime);
            big_free(&i_bigint);
        }
    }
    
    big_free(&root_inv);
    
    ctx->initialized = 1;
    return 1;
}

/**
 * Free NTT context
 */
void ntt_free(NTTContext* ctx) {
    if (!ctx || !ctx->initialized) return;
    
    big_free(&ctx->prime);
    big_free(&ctx->root);
    
    if (ctx->roots_forward) {
        for (size_t i = 0; i < ctx->n; i++) {
            big_free(&ctx->roots_forward[i]);
        }
        free(ctx->roots_forward);
    }
    
    if (ctx->roots_inverse) {
        for (size_t i = 0; i < ctx->n; i++) {
            big_free(&ctx->roots_inverse[i]);
        }
        free(ctx->roots_inverse);
    }
    
    ctx->initialized = 0;
}

/* ============================================================================
 * NTT TRANSFORMS
 * ============================================================================ */

/**
 * Forward Number Theoretic Transform
 * 
 * Uses Cooley-Tukey algorithm (iterative, in-place)
 */
void ntt_forward(const NTTContext* ctx, BigInt* output, const BigInt* input, size_t n) {
    if (!ctx || !ctx->initialized || !output || !input || n != ctx->n) return;
    
    // Copy input to output
    for (size_t i = 0; i < n; i++) {
        big_init(&output[i]);
        big_copy(&output[i], &input[i]);
    }
    
    // Bit-reverse permutation
    ntt_bit_reverse(output, n);
    
    // Cooley-Tukey butterfly operations
    for (size_t len = 2; len <= n; len *= 2) {
        size_t half_len = len / 2;
        size_t step = n / len;
        
        for (size_t i = 0; i < n; i += len) {
            for (size_t j = 0; j < half_len; j++) {
                size_t root_idx = j * step;
                
                // Butterfly operation
                BigInt u, v, temp, q, rem;
                big_init(&u);
                big_init(&v);
                big_init(&temp);
                big_init(&q);
                big_init(&rem);
                
                big_copy(&u, &output[i + j]);
                
                // v = output[i + j + half_len] * root
                big_mul(&temp, &output[i + j + half_len], &ctx->roots_forward[root_idx]);
                big_div(&temp, &ctx->prime, &q, &rem);
                big_copy(&v, &rem);
                
                // output[i + j] = (u + v) mod prime
                big_add(&temp, &u, &v);
                big_div(&temp, &ctx->prime, &q, &rem);
                big_copy(&output[i + j], &rem);
                
                // output[i + j + half_len] = (u - v) mod prime
                big_sub(&temp, &u, &v);
                if (temp.negative) {
                    big_add(&temp, &temp, &ctx->prime);
                }
                big_div(&temp, &ctx->prime, &q, &rem);
                big_copy(&output[i + j + half_len], &rem);
                
                big_free(&u);
                big_free(&v);
                big_free(&temp);
                big_free(&q);
                big_free(&rem);
            }
        }
    }
}

/**
 * Inverse Number Theoretic Transform
 */
void ntt_inverse(const NTTContext* ctx, BigInt* output, const BigInt* input, size_t n) {
    if (!ctx || !ctx->initialized || !output || !input || n != ctx->n) return;
    
    // Copy input to output
    for (size_t i = 0; i < n; i++) {
        big_init(&output[i]);
        big_copy(&output[i], &input[i]);
    }
    
    // Bit-reverse permutation
    ntt_bit_reverse(output, n);
    
    // Cooley-Tukey butterfly operations with inverse roots
    for (size_t len = 2; len <= n; len *= 2) {
        size_t half_len = len / 2;
        size_t step = n / len;
        
        for (size_t i = 0; i < n; i += len) {
            for (size_t j = 0; j < half_len; j++) {
                size_t root_idx = j * step;
                
                // Butterfly operation
                BigInt u, v, temp, q, rem;
                big_init(&u);
                big_init(&v);
                big_init(&temp);
                big_init(&q);
                big_init(&rem);
                
                big_copy(&u, &output[i + j]);
                
                // v = output[i + j + half_len] * inverse_root
                big_mul(&temp, &output[i + j + half_len], &ctx->roots_inverse[root_idx]);
                big_div(&temp, &ctx->prime, &q, &rem);
                big_copy(&v, &rem);
                
                // output[i + j] = (u + v) mod prime
                big_add(&temp, &u, &v);
                big_div(&temp, &ctx->prime, &q, &rem);
                big_copy(&output[i + j], &rem);
                
                // output[i + j + half_len] = (u - v) mod prime
                big_sub(&temp, &u, &v);
                if (temp.negative) {
                    big_add(&temp, &temp, &ctx->prime);
                }
                big_div(&temp, &ctx->prime, &q, &rem);
                big_copy(&output[i + j + half_len], &rem);
                
                big_free(&u);
                big_free(&v);
                big_free(&temp);
                big_free(&q);
                big_free(&rem);
            }
        }
    }
    
    // Scale by 1/n
    BigInt n_bigint, n_inv;
    big_init(&n_bigint);
    big_init(&n_inv);
    
    big_from_int(&n_bigint, n);
    big_mod_inverse(&n_inv, &n_bigint, &ctx->prime);
    
    for (size_t i = 0; i < n; i++) {
        BigInt temp, q, rem;
        big_init(&temp);
        big_init(&q);
        big_init(&rem);
        
        big_mul(&temp, &output[i], &n_inv);
        big_div(&temp, &ctx->prime, &q, &rem);
        big_copy(&output[i], &rem);
        
        big_free(&temp);
        big_free(&q);
        big_free(&rem);
    }
    
    big_free(&n_bigint);
    big_free(&n_inv);
}

/* ============================================================================
 * NTT-BASED MULTIPLICATION
 * ============================================================================ */

/**
 * Fast BigInt multiplication using NTT
 */
int big_ntt_multiply(BigInt* result, const BigInt* a, const BigInt* b) {
    if (!result || !a || !b) return 0;
    
    // For small numbers, use standard multiplication
    if (a->len < 32 || b->len < 32) {
        big_mul(result, a, b);
        return 1;
    }
    
    // Determine transform size (next power of 2 >= len(a) + len(b))
    size_t result_len = a->len + b->len;
    size_t n = ntt_next_power_of_2(result_len);
    
    // Initialize NTT context
    NTTContext ctx;
    if (!ntt_init(&ctx, n)) {
        fprintf(stderr, "big_ntt_multiply: failed to initialize NTT context\n");
        return 0;
    }
    
    // Convert BigInts to coefficient arrays (treating each uint32_t as a coefficient)
    BigInt* a_coeffs = malloc(n * sizeof(BigInt));
    BigInt* b_coeffs = malloc(n * sizeof(BigInt));
    
    if (!a_coeffs || !b_coeffs) {
        ntt_free(&ctx);
        if (a_coeffs) free(a_coeffs);
        if (b_coeffs) free(b_coeffs);
        return 0;
    }
    
    // Initialize and copy coefficients
    for (size_t i = 0; i < n; i++) {
        big_init(&a_coeffs[i]);
        big_init(&b_coeffs[i]);
        
        if (i < a->len) {
            big_from_int(&a_coeffs[i], a->d[i]);
        } else {
            big_from_int(&a_coeffs[i], 0);
        }
        
        if (i < b->len) {
            big_from_int(&b_coeffs[i], b->d[i]);
        } else {
            big_from_int(&b_coeffs[i], 0);
        }
    }
    
    // Apply forward NTT
    BigInt* a_ntt = malloc(n * sizeof(BigInt));
    BigInt* b_ntt = malloc(n * sizeof(BigInt));
    
    if (!a_ntt || !b_ntt) {
        for (size_t i = 0; i < n; i++) {
            big_free(&a_coeffs[i]);
            big_free(&b_coeffs[i]);
        }
        free(a_coeffs);
        free(b_coeffs);
        ntt_free(&ctx);
        if (a_ntt) free(a_ntt);
        if (b_ntt) free(b_ntt);
        return 0;
    }
    
    ntt_forward(&ctx, a_ntt, a_coeffs, n);
    ntt_forward(&ctx, b_ntt, b_coeffs, n);
    
    // Pointwise multiplication in frequency domain
    BigInt* product_ntt = malloc(n * sizeof(BigInt));
    if (!product_ntt) {
        for (size_t i = 0; i < n; i++) {
            big_free(&a_coeffs[i]);
            big_free(&b_coeffs[i]);
            big_free(&a_ntt[i]);
            big_free(&b_ntt[i]);
        }
        free(a_coeffs);
        free(b_coeffs);
        free(a_ntt);
        free(b_ntt);
        ntt_free(&ctx);
        return 0;
    }
    
    for (size_t i = 0; i < n; i++) {
        big_init(&product_ntt[i]);
        BigInt temp, q, rem;
        big_init(&temp);
        big_init(&q);
        big_init(&rem);
        
        big_mul(&temp, &a_ntt[i], &b_ntt[i]);
        big_div(&temp, &ctx.prime, &q, &rem);
        big_copy(&product_ntt[i], &rem);
        
        big_free(&temp);
        big_free(&q);
        big_free(&rem);
    }
    
    // Apply inverse NTT
    BigInt* product_coeffs = malloc(n * sizeof(BigInt));
    if (!product_coeffs) {
        for (size_t i = 0; i < n; i++) {
            big_free(&a_coeffs[i]);
            big_free(&b_coeffs[i]);
            big_free(&a_ntt[i]);
            big_free(&b_ntt[i]);
            big_free(&product_ntt[i]);
        }
        free(a_coeffs);
        free(b_coeffs);
        free(a_ntt);
        free(b_ntt);
        free(product_ntt);
        ntt_free(&ctx);
        return 0;
    }
    
    ntt_inverse(&ctx, product_coeffs, product_ntt, n);
    
    // Convert back to BigInt with carry handling
    big_from_int(result, 0);
    BigInt base, power, term;
    big_init(&base);
    big_init(&power);
    big_init(&term);
    
    big_from_int(&base, 1ULL << 32);  // Base is 2^32
    big_from_int(&power, 1);
    
    for (size_t i = 0; i < result_len && i < n; i++) {
        // term = product_coeffs[i] * power
        big_mul(&term, &product_coeffs[i], &power);
        big_add(result, result, &term);
        
        // power *= base
        BigInt temp;
        big_init(&temp);
        big_mul(&temp, &power, &base);
        big_copy(&power, &temp);
        big_free(&temp);
    }
    
    // Handle sign
    result->negative = a->negative ^ b->negative;
    
    // Cleanup
    for (size_t i = 0; i < n; i++) {
        big_free(&a_coeffs[i]);
        big_free(&b_coeffs[i]);
        big_free(&a_ntt[i]);
        big_free(&b_ntt[i]);
        big_free(&product_ntt[i]);
        big_free(&product_coeffs[i]);
    }
    free(a_coeffs);
    free(b_coeffs);
    free(a_ntt);
    free(b_ntt);
    free(product_ntt);
    free(product_coeffs);
    
    big_free(&base);
    big_free(&power);
    big_free(&term);
    
    ntt_free(&ctx);
    
    return 1;
}

/* ============================================================================
 * CRYSTALLINE LATTICE OPTIMIZATIONS
 * ============================================================================ */

/**
 * Find NTT prime using Ulam spiral structure
 * 
 * TODO: Implement lattice-aware prime finding
 * Uses crystalline lattice prime distribution patterns
 */
int ntt_find_prime_lattice(BigInt* prime, size_t n, int bits) {
    // For now, use standard method
    // Future: leverage Ulam spiral patterns for faster prime finding
    return ntt_find_prime(prime, n, bits);
}

/**
 * Optimize NTT for crystalline lattice structure
 * 
 * TODO: Implement lattice-specific optimizations
 */
int ntt_optimize_for_lattice(NTTContext* ctx) {
    if (!ctx || !ctx->initialized) return 0;
    
    // Future optimizations:
    // 1. Use lattice symmetries to reduce computation
    // 2. Leverage prime factorization structure
    // 3. Apply Plimpton ratios for root selection
    
    return 1;
}