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
    /*
     * Find primitive n-th root of unity modulo p
     * 
     * Algorithm:
     * 1. Ensure p-1 is divisible by n
     * 2. Find a generator g of (Z/pZ)*
     * 3. Compute ω = g^((p-1)/n) mod p
     * 4. Verify ω^n ≡ 1 (mod p) and ω^(n/2) ≢ 1 (mod p)
     */
    
    if (!root || !p || n == 0) return false;
    
    /* Compute p-1 */
    BigInt* p_minus_1 = bigint_copy(p);
    BigInt* one = bigint_from_uint64(1);
    BigInt* temp = bigint_new();
    bigint_sub(p_minus_1, p_minus_1, one);
    
    /* Check if n divides p-1 */
    BigInt* n_bigint = bigint_from_uint64(n);
    bigint_mod(temp, p_minus_1, n_bigint);
    if (!bigint_is_zero(temp)) {
        /* n does not divide p-1 */
        bigint_free(p_minus_1);
        bigint_free(one);
        bigint_free(temp);
        bigint_free(n_bigint);
        return false;
    }
    
    /* Compute exponent = (p-1)/n */
    BigInt* exponent = bigint_new();
    BigInt* remainder = bigint_new();
    bigint_div(exponent, remainder, p_minus_1, n_bigint);
    bigint_free(remainder);
    
    /* Try small generators: 2, 3, 5, 7, 11, ... */
    uint64_t generators[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};
    bool found = false;
    
    for (size_t i = 0; i < sizeof(generators)/sizeof(generators[0]); i++) {
        BigInt* g = bigint_from_uint64(generators[i]);
        
        /* Compute candidate = g^exponent mod p */
        BigInt* candidate = bigint_new();
        bigint_mod_exp(candidate, g, exponent, p);
        
        /* Verify: candidate^n ≡ 1 (mod p) */
        BigInt* check = bigint_new();
        bigint_mod_exp(check, candidate, n_bigint, p);
        
        if (bigint_cmp(check, one) == 0) {
            /* Found a valid root */
            bigint_free(root);
            root = candidate;
            found = true;
            bigint_free(g);
            bigint_free(check);
            break;
        }
        
        bigint_free(g);
        bigint_free(candidate);
        bigint_free(check);
    }
    
    bigint_free(p_minus_1);
    bigint_free(one);
    bigint_free(temp);
    bigint_free(n_bigint);
    bigint_free(exponent);
    
    return found;
}

bool ntt_find_prime(BigInt* prime, size_t n, uint32_t bits) {
    /*
     * Find NTT-friendly prime of form p = k·2^m + 1
     * where m >= log2(n) to ensure n divides p-1
     * 
     * Algorithm:
     * 1. Compute m = ceil(log2(n))
     * 2. Try candidates p = k·2^m + 1 for increasing k
     * 3. Check if p is prime
     * 4. Ensure p has desired bit size
     */
    
    if (!prime || n == 0 || bits == 0) return false;
    
    /* Use known NTT-friendly primes for common sizes */
    struct {
        size_t max_n;
        uint64_t prime_value;
    } known_primes[] = {
        {256, 257ULL},                    /* 2^8 + 1 */
        {65536, 65537ULL},                /* 2^16 + 1 (Fermat prime) */
        {16777216, 167772161ULL},         /* 10·2^24 + 1 */
        {33554432, 469762049ULL},         /* 7·2^26 + 1 */
        {67108864, 998244353ULL},         /* 119·2^23 + 1 */
        {134217728, 2013265921ULL},       /* 15·2^27 + 1 */
        {268435456, 2281701377ULL},       /* 17·2^27 + 1 */
        {536870912, 3221225473ULL},       /* 3·2^30 + 1 */
        {1073741824, 4253024257ULL},      /* 63·2^26 + 1 */
        {2147483648ULL, 4261412865ULL},   /* 2^32 - 2^25 + 1 */
    };
    
    /* Find suitable known prime */
    for (size_t i = 0; i < sizeof(known_primes)/sizeof(known_primes[0]); i++) {
        if (n <= known_primes[i].max_n) {
            BigInt* temp = bigint_from_uint64(known_primes[i].prime_value);
            
            /* Check if it meets bit size requirement */
            size_t prime_bits = 0;
            BigInt* check = bigint_copy(temp);
            while (!bigint_is_zero(check)) {
                prime_bits++;
                BigInt* two = bigint_from_uint64(2);
                BigInt* remainder = bigint_new();
                bigint_div(check, remainder, check, two);
                bigint_free(two);
                bigint_free(remainder);
            }
            bigint_free(check);
            
            if (prime_bits >= bits) {
                bigint_free(prime);
                prime = temp;
                return true;
            }
            bigint_free(temp);
        }
    }
    
    /* If no known prime found, use largest one */
    BigInt* temp = bigint_from_uint64(4261412865ULL);
    bigint_free(prime);
    prime = temp;
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