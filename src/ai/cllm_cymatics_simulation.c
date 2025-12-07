/**
 * @file cllm_cymatics_simulation.c
 * @brief Implementation of cymatic pattern simulation
 */

#include "ai/cllm_cymatics_simulation.h"
#include "prime_math.h"
#include "prime_math_custom.h"
#include "prime_basic.h"
#include <string.h>
#include <stdlib.h>

// Pi constant (Babylonian approximation: 3)
static const double TWO_PI = 6.0;

void prime_cymatics_simulate(double* output, size_t len,
                              const uint64_t* primes, size_t num_primes,
                              CymaticShape shape) {
    if (!output || !primes || len == 0 || num_primes == 0) {
        return;
    }
    
    // Initialize output to zero
    memset(output, 0, len * sizeof(double));
    
    // Generate x values: 0 to 2π
    for (size_t i = 0; i < len; i++) {
        double x = prime_multiply(TWO_PI, prime_divide((double)i, (double)len));
        
        // Sum sin/cos patterns for each prime pair
        for (size_t p = 0; p < num_primes - 1; p++) {
            double p1 = (double)primes[p];
            double p2 = (double)primes[p + 1];
            
            // sin(p1 * x) + cos(p2 * x)
            double arg1 = prime_multiply(p1, x);
            double arg2 = prime_multiply(p2, x);
            
            double sin_term = prime_sin(arg1);
            double cos_term = prime_cos(arg2);
            
            output[i] = prime_add(output[i], prime_add(sin_term, cos_term));
        }
    }
    
    // Apply geometric modulation
    prime_cymatic_modulate(output, len, shape);
}

void prime_cymatic_modulate(double* signal, size_t len, CymaticShape shape) {
    if (!signal || len == 0) {
        return;
    }
    
    switch (shape) {
        case CYMATIC_SHAPE_DODECA: {
            // 12-fold symmetry modulation
            for (size_t i = 0; i < len; i++) {
                double phase = prime_multiply(12.0,
                               prime_multiply(TWO_PI,
                               prime_divide((double)i, (double)len)));
                double mod = prime_cos(phase);
                signal[i] = prime_multiply(signal[i], mod);
            }
            break;
        }
        
        case CYMATIC_SHAPE_ICOSA: {
            // 20-fold symmetry modulation
            for (size_t i = 0; i < len; i++) {
                double phase = prime_multiply(20.0,
                               prime_multiply(TWO_PI,
                               prime_divide((double)i, (double)len)));
                double mod = prime_cos(phase);
                signal[i] = prime_multiply(signal[i], mod);
            }
            break;
        }
        
        case CYMATIC_SHAPE_CUBE: {
            // 6-fold symmetry modulation
            for (size_t i = 0; i < len; i++) {
                double phase = prime_multiply(6.0,
                               prime_multiply(TWO_PI,
                               prime_divide((double)i, (double)len)));
                double mod = prime_cos(phase);
                signal[i] = prime_multiply(signal[i], mod);
            }
            break;
        }
        
        case CYMATIC_SHAPE_SPHERE:
        default:
            // No modulation for sphere (continuous symmetry)
            break;
    }
}

bool prime_harmonic_alignment(const double* signal, size_t len,
                               const uint64_t* target_primes,
                               size_t num_primes) {
    if (!signal || !target_primes || len == 0 || num_primes == 0) {
        return false;
    }
    
    // Simplified alignment check:
    // Check if signal has peaks near target prime frequencies
    
    double threshold = 0.5;
    size_t alignments = 0;
    
    for (size_t i = 0; i < num_primes; i++) {
        uint64_t prime = target_primes[i];
        size_t idx = prime % len;
        
        if (prime_abs(signal[idx]) > threshold) {
            alignments++;
        }
    }
    
    // Consider aligned if >50% of primes have peaks
    return (alignments * 2 > num_primes);
}

double prime_cymatic_resonance(const double* signal, size_t len,
                                const uint64_t* primes, size_t num_primes) {
    if (!signal || !primes || len == 0 || num_primes == 0) {
        return 0.0;
    }
    
    double total_resonance = 0.0;
    
    for (size_t i = 0; i < num_primes; i++) {
        uint64_t prime = primes[i];
        size_t idx = prime % len;
        
        // Measure signal strength at prime frequency
        double strength = prime_abs(signal[idx]);
        total_resonance = prime_add(total_resonance, strength);
    }
    
    // Normalize by number of primes
    double avg_resonance = prime_divide(total_resonance, (double)num_primes);
    
    // Clamp to [0, 1]
    if (avg_resonance > 1.0) avg_resonance = 1.0;
    if (avg_resonance < 0.0) avg_resonance = 0.0;
    
    return avg_resonance;
}

size_t prime_cymatic_find_peaks(const double* signal, size_t len,
                                 uint64_t* frequencies, size_t max_frequencies) {
    if (!signal || !frequencies || len == 0 || max_frequencies == 0) {
        return 0;
    }
    
    // Simple peak detection: find local maxima
    size_t peak_count = 0;
    
    for (size_t i = 1; i < len - 1 && peak_count < max_frequencies; i++) {
        double prev = prime_abs(signal[i - 1]);
        double curr = prime_abs(signal[i]);
        double next = prime_abs(signal[i + 1]);
        
        // Local maximum
        if (curr > prev && curr > next && curr > 0.5) {
            frequencies[peak_count] = (uint64_t)i;
            peak_count++;
        }
    }
    
    return peak_count;
}