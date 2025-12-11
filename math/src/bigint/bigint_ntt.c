/**
 * @file bigint_ntt.c
 * @brief Number Theoretic Transform (NTT) implementation
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * This file implements the Number Theoretic Transform for BigInt arithmetic.
 * NTT provides O(n log n) multiplication for large integers.
 */

#include "math/ntt.h"
#include "math/bigint.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

bool ntt_is_power_of_2(size_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

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

uint32_t ntt_log2(size_t n) {
    uint32_t log = 0;
    while (n > 1) {
        n >>= 1;
        log++;
    }
    return log;
}

void ntt_bit_reverse(BigInt** array, size_t n) {
    if (!ntt_is_power_of_2(n)) return;
    
    uint32_t log_n = ntt_log2(n);
    
    for (size_t i = 0; i < n; i++) {
        size_t j = 0;
        for (uint32_t bit = 0; bit < log_n; bit++) {
            if (i & (1ULL << bit)) {
                j |= 1ULL << (log_n - 1 - bit);
            }
        }
        
        if (i < j) {
            /* Swap array[i] and array[j] */
            BigInt* temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
}

/* ============================================================================
 * INITIALIZATION & CLEANUP
 * ============================================================================ */

NTTContext* ntt_create(size_t n) {
    if (!ntt_is_power_of_2(n)) {
        return NULL;
    }
    
    NTTContext* ctx = (NTTContext*)calloc(1, sizeof(NTTContext));
    if (!ctx) return NULL;
    
    ctx->n = n;
    ctx->log_n = ntt_log2(n);
    ctx->initialized = false;
    
    /* Find suitable prime */
    ctx->prime = bigint_new();
    if (!ntt_find_prime(ctx->prime, n, 64)) {
        ntt_free(ctx);
        return NULL;
    }
    
    /* Find primitive root */
    ctx->root = bigint_new();
    if (!ntt_find_primitive_root(ctx->root, n, ctx->prime)) {
        ntt_free(ctx);
        return NULL;
    }
    
    /* Allocate root arrays */
    ctx->roots_forward = (BigInt**)calloc(n, sizeof(BigInt*));
    ctx->roots_inverse = (BigInt**)calloc(n, sizeof(BigInt*));
    
    if (!ctx->roots_forward || !ctx->roots_inverse) {
        ntt_free(ctx);
        return NULL;
    }
    
    /* Precompute roots */
    for (size_t i = 0; i < n; i++) {
        ctx->roots_forward[i] = bigint_new();
        ctx->roots_inverse[i] = bigint_new();
        
        /* Forward: ω^i mod p */
        BigInt* exp_i = bigint_from_uint64(i);
        bigint_mod_exp(ctx->roots_forward[i], ctx->root, exp_i, ctx->prime);
        bigint_free(exp_i);
        
        /* Inverse: ω^(-i) mod p = ω^(n-i) mod p */
        if (i == 0) {
            BigInt* one = bigint_from_uint64(1);
            bigint_free(ctx->roots_inverse[i]);
            ctx->roots_inverse[i] = one;
        } else {
            BigInt* exp_ni = bigint_from_uint64(n - i);
            bigint_mod_exp(ctx->roots_inverse[i], ctx->root, exp_ni, ctx->prime);
            bigint_free(exp_ni);
        }
    }
    
    ctx->initialized = true;
    return ctx;
}

void ntt_free(NTTContext* ctx) {
    if (!ctx) return;
    
    if (ctx->prime) bigint_free(ctx->prime);
    if (ctx->root) bigint_free(ctx->root);
    
    if (ctx->roots_forward) {
        for (size_t i = 0; i < ctx->n; i++) {
            if (ctx->roots_forward[i]) {
                bigint_free(ctx->roots_forward[i]);
            }
        }
        free(ctx->roots_forward);
    }
    
    if (ctx->roots_inverse) {
        for (size_t i = 0; i < ctx->n; i++) {
            if (ctx->roots_inverse[i]) {
                bigint_free(ctx->roots_inverse[i]);
            }
        }
        free(ctx->roots_inverse);
    }
    
    free(ctx);
}

bool ntt_init_with_prime(NTTContext* ctx, size_t n, const BigInt* prime) {
    if (!ctx || !prime || !ntt_is_power_of_2(n)) {
        return false;
    }
    
    ctx->n = n;
    ctx->log_n = ntt_log2(n);
    
    ctx->prime = bigint_copy(prime);
    
    ctx->root = bigint_new();
    if (!ntt_find_primitive_root(ctx->root, n, ctx->prime)) {
        return false;
    }
    
    ctx->initialized = true;
    return true;
}

/* ============================================================================
 * PRIMITIVE ROOT FINDING
 * ============================================================================ */

bool ntt_find_primitive_root(BigInt* root, size_t n, const BigInt* p) {
    /* TODO: Implement primitive root finding
     * This is a placeholder that returns a simple value */
    (void)n; (void)p; (void)root;
    return true;
}

bool ntt_find_prime(BigInt* prime, size_t n, uint32_t bits) {
    /* TODO: Implement NTT prime finding
     * For now, use a known NTT-friendly prime */
    (void)n; (void)bits;
    
    /* Example: 2^32 - 2^25 + 1 = 4261412865 (supports n up to 2^25) */
    BigInt* temp = bigint_from_uint64(4261412865ULL);
    bigint_free(prime);
    prime = temp;
    (void)prime; /* Suppress unused warning since we're modifying the pointer */
    return true;
}

bool ntt_is_primitive_root(const BigInt* g, const BigInt* p) {
    /* TODO: Implement primitive root checking */
    (void)g; (void)p;
    return true;
}

/* ============================================================================
 * NTT TRANSFORMS
 * ============================================================================ */

bool ntt_forward(const NTTContext* ctx, BigInt** output, const BigInt** input, size_t n) {
    if (!ctx || !ctx->initialized || !output || !input || n != ctx->n) {
        return false;
    }
    
    /* Copy input to output */
    for (size_t i = 0; i < n; i++) {
        output[i] = bigint_copy(input[i]);
    }
    
    /* Bit-reverse permutation */
    ntt_bit_reverse(output, n);
    
    /* Cooley-Tukey butterfly operations */
    for (size_t len = 2; len <= n; len *= 2) {
        size_t half_len = len / 2;
        size_t step = n / len;
        
        for (size_t i = 0; i < n; i += len) {
            for (size_t j = 0; j < half_len; j++) {
                size_t root_idx = j * step;
                
                BigInt* u = output[i + j];
                BigInt* v = bigint_new();
                
                /* v = output[i + j + half_len] * root^root_idx mod p */
                bigint_mod_mul(v, output[i + j + half_len], 
                              ctx->roots_forward[root_idx], ctx->prime);
                
                /* output[i + j] = u + v mod p */
                BigInt* temp1 = bigint_new();
                bigint_mod_add(temp1, u, v, ctx->prime);
                bigint_free(output[i + j]);
                output[i + j] = temp1;
                
                /* output[i + j + half_len] = u - v mod p */
                BigInt* temp2 = bigint_new();
                bigint_mod_sub(temp2, u, v, ctx->prime);
                bigint_free(output[i + j + half_len]);
                output[i + j + half_len] = temp2;
                
                bigint_free(v);
            }
        }
    }
    
    return true;
}

bool ntt_inverse(const NTTContext* ctx, BigInt** output, const BigInt** input, size_t n) {
    if (!ctx || !ctx->initialized || !output || !input || n != ctx->n) {
        return false;
    }
    
    /* Copy input to output */
    for (size_t i = 0; i < n; i++) {
        output[i] = bigint_copy(input[i]);
    }
    
    /* Bit-reverse permutation */
    ntt_bit_reverse(output, n);
    
    /* Cooley-Tukey butterfly operations with inverse roots */
    for (size_t len = 2; len <= n; len *= 2) {
        size_t half_len = len / 2;
        size_t step = n / len;
        
        for (size_t i = 0; i < n; i += len) {
            for (size_t j = 0; j < half_len; j++) {
                size_t root_idx = j * step;
                
                BigInt* u = output[i + j];
                BigInt* v = bigint_new();
                
                /* v = output[i + j + half_len] * inverse_root^root_idx mod p */
                bigint_mod_mul(v, output[i + j + half_len],
                              ctx->roots_inverse[root_idx], ctx->prime);
                
                /* output[i + j] = u + v mod p */
                BigInt* temp1 = bigint_new();
                bigint_mod_add(temp1, u, v, ctx->prime);
                bigint_free(output[i + j]);
                output[i + j] = temp1;
                
                /* output[i + j + half_len] = u - v mod p */
                BigInt* temp2 = bigint_new();
                bigint_mod_sub(temp2, u, v, ctx->prime);
                bigint_free(output[i + j + half_len]);
                output[i + j + half_len] = temp2;
                
                bigint_free(v);
            }
        }
    }
    
    /* Scale by 1/n mod p */
    BigInt* n_inv = bigint_new();
    BigInt* n_bigint = bigint_from_uint64(n);
    bigint_mod_inverse(n_inv, n_bigint, ctx->prime);
    
    for (size_t i = 0; i < n; i++) {
        BigInt* temp = bigint_new();
        bigint_mod_mul(temp, output[i], n_inv, ctx->prime);
        bigint_free(output[i]);
        output[i] = temp;
    }
    
    bigint_free(n_inv);
    bigint_free(n_bigint);
    
    return true;
}

/* ============================================================================
 * NTT-BASED MULTIPLICATION
 * ============================================================================ */

bool ntt_multiply(BigInt* result, const BigInt* a, const BigInt* b) {
    /* TODO: Implement NTT-based multiplication
     * This is a complex algorithm that requires:
     * 1. Converting BigInts to coefficient arrays
     * 2. Padding to power of 2
     * 3. Applying forward NTT
     * 4. Pointwise multiplication
     * 5. Applying inverse NTT
     * 6. Handling carries
     */
    
    /* For now, fall back to standard multiplication */
    return bigint_mul(result, a, b);
}

bool ntt_poly_multiply(BigInt** result, const BigInt** a, size_t n,
                       const BigInt** b, size_t m, const NTTContext* ctx) {
    /* TODO: Implement polynomial multiplication */
    (void)result; (void)a; (void)n; (void)b; (void)m; (void)ctx;
    return false;
}

/* ============================================================================
 * CRYSTALLINE LATTICE OPTIMIZATIONS
 * ============================================================================ */

bool ntt_find_prime_lattice(BigInt* prime, size_t n, uint32_t bits) {
    /* TODO: Use clock lattice to find NTT-friendly primes faster */
    return ntt_find_prime(prime, n, bits);
}

bool ntt_optimize_for_lattice(NTTContext* ctx) {
    /* TODO: Precompute lattice-specific optimizations */
    (void)ctx;
    return true;
}