/*
 * CLLM Root Word Modeling
 * 
 * Implements linguistic root extraction and morphological analysis
 * using prime factorization and crystalline lattice structure.
 * 
 * Core Concept:
 * - Prime numbers represent linguistic roots
 * - Composite numbers are variations (tense, plurality, case, etc.)
 * - Factorization reveals morphological structure
 * - Coprime relationships indicate semantic distance
 * 
 * Example:
 * - "run" (prime 5) is a root
 * - "running" (5 × 2) is progressive form
 * - "runs" (5 × 3) is third person singular
 * - "ran" (5 × 7) is past tense
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/cllm_crystalline_attention.h"
#include "../include/prime_float_math.h"

// Prime cache for fast lookup
#define PRIME_CACHE_SIZE 1000
static uint64_t prime_cache[PRIME_CACHE_SIZE];
static int prime_cache_initialized = 0;

/**
 * Initialize prime cache
 */
static void init_prime_cache(void) {
    if (prime_cache_initialized) return;
    
    prime_cache[0] = 2;
    prime_cache[1] = 3;
    
    int count = 2;
    uint64_t candidate = 5;
    
    while (count < PRIME_CACHE_SIZE) {
        int is_prime = 1;
        uint64_t sqrt_cand = (uint64_t)prime_sqrt((double)candidate);
        
        for (int i = 0; i < count && prime_cache[i] <= sqrt_cand; i++) {
            if (candidate % prime_cache[i] == 0) {
                is_prime = 0;
                break;
            }
        }
        
        if (is_prime) {
            prime_cache[count++] = candidate;
        }
        
        candidate += 2;
    }
    
    prime_cache_initialized = 1;
}

/**
 * Check if number is prime
 */
static int is_prime(uint64_t n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    
    uint64_t sqrt_n = (uint64_t)prime_sqrt((double)n);
    for (uint64_t i = 3; i <= sqrt_n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

/**
 * Get nth prime number
 */
__attribute__((unused))
static uint64_t get_nth_prime(uint32_t n) {
    init_prime_cache();
    
    if (n < PRIME_CACHE_SIZE) {
        return prime_cache[n];
    }
    
    // For larger n, compute on the fly
    uint64_t count = PRIME_CACHE_SIZE;
    uint64_t candidate = prime_cache[PRIME_CACHE_SIZE - 1] + 2;
    
    while (count <= n) {
        if (is_prime(candidate)) {
            if (count == n) return candidate;
            count++;
        }
        candidate += 2;
    }
    
    return candidate;
}

/**
 * Get prime number for token
 * 
 * Mapping strategy:
 * - Small token IDs (< 1000): Direct prime mapping
 * - Larger token IDs: Hash to prime range
 * - Special tokens: Reserved primes (2, 3, 5, 7, 11)
 */
uint64_t cllm_get_token_prime(uint32_t token_id) {
    init_prime_cache();
    
    // Special tokens
    if (token_id == 0) return 2;   // PAD
    if (token_id == 1) return 3;   // UNK
    if (token_id == 2) return 5;   // BOS
    if (token_id == 3) return 7;   // EOS
    if (token_id == 4) return 11;  // MASK
    
    // Direct mapping for small IDs
    if (token_id < PRIME_CACHE_SIZE) {
        return prime_cache[token_id % PRIME_CACHE_SIZE];
    }
    
    // Hash to prime range for larger IDs
    uint32_t prime_idx = token_id % PRIME_CACHE_SIZE;
    return prime_cache[prime_idx];
}

/**
 * Compute lattice coordinates for a token
 */
void cllm_compute_token_lattice_coords(uint32_t token_id, uint64_t prime, float* coords) {
    if (!coords) return;
    
    init_prime_cache();
    
    // Find prime index
    uint32_t prime_index = 0;
    for (int i = 0; i < PRIME_CACHE_SIZE; i++) {
        if (prime_cache[i] == prime) {
            prime_index = i;
            break;
        }
    }
    
    // Ulam spiral: radius grows with square root of index
    float radius = prime_sqrt((float)prime_index + 1.0f);
    
    // Golden angle for optimal packing
    const float PHI = 1.618033988749894848f;
    const float PI = 3.14159265358979323846f;
    float golden_angle = 2.0f * PI / (PHI * PHI);
    float angle = golden_angle * (float)prime_index;
    
    // Normalize angle
    while (angle >= 2.0f * PI) {
        angle -= 2.0f * PI;
    }
    
    // Convert to 3D coordinates
    coords[0] = radius * prime_cos(angle);
    coords[1] = radius * prime_sin(angle);
    coords[2] = prime_log((float)prime + 1.0f);
    
    // Add token-specific perturbation
    float token_phase = 2.0f * PI * (float)token_id / 1000.0f;
    coords[0] += 0.1f * prime_cos(token_phase);
    coords[1] += 0.1f * prime_sin(token_phase);
    coords[2] += 0.1f * prime_sin(token_phase * PHI);
}

/**
 * Compute GCD using Euclidean algorithm
 */
static uint64_t compute_gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Compute semantic similarity using prime factorization
 */
float cllm_compute_prime_similarity(uint64_t prime1, uint64_t prime2) {
    if (prime1 == prime2) return 1.0f;
    
    // Compute GCD
    uint64_t gcd = compute_gcd(prime1, prime2);
    
    // Coprime (gcd = 1): Maximally different
    if (gcd == 1) {
        return 0.0f;
    }
    
    // Share factors: Similarity based on GCD
    float similarity = (float)gcd / (float)(prime1 < prime2 ? prime1 : prime2);
    return similarity;
}

/**
 * Extract root word from token
 * 
 * For composite numbers, extract the smallest prime factor (root).
 * For prime numbers, the token itself is the root.
 */
uint32_t cllm_extract_root_word(uint32_t token_id, uint64_t prime) {
    init_prime_cache();
    
    // If prime, it's already a root
    if (is_prime(prime)) {
        return token_id;
    }
    
    // Find smallest prime factor
    for (int i = 0; i < PRIME_CACHE_SIZE; i++) {
        if (prime % prime_cache[i] == 0) {
            // Find token with this prime
            for (uint32_t tid = 0; tid < 10000; tid++) {
                if (cllm_get_token_prime(tid) == prime_cache[i]) {
                    return tid;
                }
            }
            break;
        }
    }
    
    return token_id;  // Fallback
}

/**
 * Compute morphological relationship between tokens
 * 
 * Returns:
 * 0 = Unrelated (coprime)
 * 1 = Related (share factors)
 * 2 = Derived (one divides other)
 * 3 = Same (identical primes)
 */
int cllm_compute_morphological_relationship(uint64_t token1_prime,
                                           uint64_t token2_prime) {
    if (token1_prime == token2_prime) {
        return 3;  // Same
    }
    
    uint64_t gcd = compute_gcd(token1_prime, token2_prime);
    
    if (gcd == 1) {
        return 0;  // Unrelated (coprime)
    }
    
    // Check if one divides the other
    if (token1_prime % token2_prime == 0 || token2_prime % token1_prime == 0) {
        return 2;  // Derived
    }
    
    return 1;  // Related (share factors)
}

/**
 * Compute hyperdimensional distance
 */
float cllm_compute_hyperdimensional_distance(const float* coords1,
                                             const float* coords2,
                                             uint64_t prime1,
                                             uint64_t prime2) {
    if (!coords1 || !coords2) return 0.0f;
    
    // Euclidean distance in 3D
    float dx = coords1[0] - coords2[0];
    float dy = coords1[1] - coords2[1];
    float dz = coords1[2] - coords2[2];
    float euclidean = prime_sqrt(dx*dx + dy*dy + dz*dz);
    
    // Prime distance
    uint64_t gcd = compute_gcd(prime1, prime2);
    float prime_dist = (gcd == 1) ? 1.0f : (1.0f / (float)gcd);
    
    // Combined distance
    return euclidean * prime_dist;
}

/**
 * Apply symmetry operation to attention weights
 */
void cllm_apply_symmetry_operation(float* weights, int seq_len, int symmetry_type) {
    if (!weights || seq_len <= 0) return;
    
    int operation = symmetry_type % 24;
    
    if (operation < 12) {
        // Rotation (12-fold symmetry)
        float angle = 2.0f * 3.14159265358979323846f * (float)operation / 12.0f;
        
        for (int i = 0; i < seq_len; i++) {
            float phase = angle * (float)i / (float)seq_len;
            float rotation = (1.0f + prime_cos(phase)) / 2.0f;
            weights[i] *= rotation;
        }
    } else {
        // Reflection (mirror planes)
        int mirror_axis = operation - 12;
        (void)mirror_axis; /* Reserved for future axis-specific reflections */
        
        for (int i = 0; i < seq_len; i++) {
            int reflected_i = seq_len - 1 - i;
            if (i < reflected_i) {
                float temp = weights[i];
                weights[i] = weights[reflected_i];
                weights[reflected_i] = temp;
            }
        }
    }
}

/**
 * Compute Fourier transform of attention pattern
 * 
 * Simple DFT implementation for attention analysis
 */
void cllm_compute_attention_fourier(const float* attention_weights,
                                    float* fourier_output,
                                    int seq_len) {
    if (!attention_weights || !fourier_output || seq_len <= 0) return;
    
    const float PI = 3.14159265358979323846f;
    
    for (int k = 0; k < seq_len; k++) {
        float real = 0.0f;
        float imag = 0.0f;
        
        for (int n = 0; n < seq_len; n++) {
            float angle = -2.0f * PI * (float)k * (float)n / (float)seq_len;
            real += attention_weights[n] * prime_cos(angle);
            imag += attention_weights[n] * prime_sin(angle);
        }
        
        // Magnitude
        fourier_output[k] = prime_sqrt(real * real + imag * imag);
    }
}

/**
 * Apply Fourier-based dampening to attention
 */
void cllm_apply_fourier_dampening(float* attention_weights,
                                  int seq_len,
                                  float cutoff_freq) {
    if (!attention_weights || seq_len <= 0) return;
    
    // Compute Fourier transform
    float* fourier = (float*)malloc(seq_len * sizeof(float));
    if (!fourier) return;
    
    cllm_compute_attention_fourier(attention_weights, fourier, seq_len);
    
    // Apply low-pass filter
    for (int i = 0; i < seq_len; i++) {
        float freq = (float)i / (float)seq_len;
        if (freq > cutoff_freq) {
            fourier[i] *= prime_exp(-(freq - cutoff_freq) * 10.0f);
        }
    }
    
    // Inverse transform (simplified - just scale by filtered magnitudes)
    for (int i = 0; i < seq_len; i++) {
        float scale = fourier[i] / (fourier[0] + 1e-8f);
        attention_weights[i] *= scale;
    }
    
    free(fourier);
}