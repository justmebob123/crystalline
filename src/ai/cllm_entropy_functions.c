/**
 * @file cllm_entropy_functions.c
 * @brief Implementation of entropy functions using pure crystalline mathematics
 */

#include "ai/cllm_entropy_functions.h"
#include "prime_math.h"
#include "prime_basic.h"
#include "prime_math_custom.h"
#include <stdlib.h>
#include <string.h>

double prime_shannon_entropy(const double* signal, size_t len) {
    if (!signal || len == 0) {
        return 0.0;
    }
    
    // Calculate sum for normalization
    double sum = 0.0;
    for (size_t i = 0; i < len; i++) {
        sum = prime_add(sum, signal[i]);
    }
    
    if (sum < 1e-10) {
        return 0.0;
    }
    
    // Calculate Shannon entropy: H = -Σ p(i) * log2(p(i))
    double entropy = 0.0;
    for (size_t i = 0; i < len; i++) {
        if (signal[i] > 1e-10) {
            // Normalize to probability
            double prob = prime_divide(signal[i], sum);
            
            // log2(prob) = log(prob) / log(2)
            double log_prob = prime_log2(prob);
            
            // -p * log2(p)
            double term = prime_multiply(prob, log_prob);
            entropy = prime_subtract(entropy, term);
        }
    }
    
    return entropy;
}

double prime_entropy_reduction(double initial_bits, uint32_t steps,
                                double cut_min, double cut_max) {
    double remaining = initial_bits;
    
    // Simple PRNG seed based on initial value
    uint32_t seed = (uint32_t)(initial_bits * 1000.0);
    
    for (uint32_t i = 0; i < steps; i++) {
        // Linear congruential generator (prime-based)
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        // Normalize to [0, 1]
        double rand_val = prime_divide((double)(seed % 10000), 10000.0);
        
        // Scale to [cut_min, cut_max]
        double range = prime_subtract(cut_max, cut_min);
        double cut = prime_add(cut_min, prime_multiply(rand_val, range));
        
        // Apply reduction: remaining *= (1 - cut)
        double reduction = prime_subtract(1.0, cut);
        remaining = prime_multiply(remaining, reduction);
    }
    
    // Ensure minimum of 1 bit
    return prime_fmax(1.0, remaining);
}

double prime_modular_probability_dist(uint64_t value,
                                       const uint64_t* primes,
                                       size_t num_primes,
                                       double* prob_dist) {
    if (!primes || !prob_dist || num_primes == 0) {
        return 0.0;
    }
    
    double sum = 0.0;
    
    for (size_t i = 0; i < num_primes; i++) {
        if (primes[i] == 0) {
            prob_dist[i] = 0.0;
            continue;
        }
        
        // p[i] = (value % prime[i]) / prime[i]
        uint64_t mod_val = value % primes[i];
        prob_dist[i] = prime_divide((double)mod_val, (double)primes[i]);
        sum = prime_add(sum, prob_dist[i]);
    }
    
    return sum;
}

double prime_compute_entropy_residuals(const uint64_t* tetrated_stack,
                                        size_t stack_len,
                                        const uint64_t* primes,
                                        size_t num_primes,
                                        double phi_scale) {
    if (!tetrated_stack || !primes || stack_len == 0 || num_primes == 0) {
        return 0.0;
    }
    
    double layer_entropy = 0.0;
    
    // Allocate temporary probability distribution
    double* prob_dist = (double*)calloc(num_primes, sizeof(double));
    if (!prob_dist) {
        return 0.0;
    }
    
    // Calculate entropy for each layer
    for (size_t i = 0; i < stack_len; i++) {
        // Use first (i+1) primes for this layer
        size_t primes_for_layer = (i + 1 < num_primes) ? (i + 1) : num_primes;
        
        // Get probability distribution
        double prob_sum = prime_modular_probability_dist(
            tetrated_stack[i],
            primes,
            primes_for_layer,
            prob_dist
        );
        
        if (prob_sum < 1e-10) {
            continue; // Skip zero-divergence layers
        }
        
        // Calculate Shannon-like entropy for this layer
        for (size_t j = 0; j < primes_for_layer; j++) {
            if (prob_dist[j] > 1e-10) {
                double log_p = prime_log(prime_add(prob_dist[j], 1e-10));
                double term = prime_multiply(prob_dist[j], log_p);
                layer_entropy = prime_subtract(layer_entropy, term);
            }
        }
    }
    
    free(prob_dist);
    
    // Fold residuals into golden bounds: residual = layer_entropy % phi_scale
    double residual = prime_fmod(layer_entropy, phi_scale);
    
    return residual;
}

bool prime_validate_ethics_residual(double residual, double threshold) {
    // Ensure residual < threshold for low divergence
    return prime_abs(residual) < threshold;
}