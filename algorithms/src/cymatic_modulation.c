/**
 * Cymatic Frequency Modulation Algorithm (General Purpose)
 * 
 * This is a GENERAL algorithm that applies cymatic frequency resonance
 * to modulate gradients. It can be used by ANY training system.
 * 
 * Cymatic frequencies create smoother convergence by aligning training
 * dynamics with natural frequency patterns.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/cymatic_modulation.h"
#include "../../include/prime_float_math.h"
#include "../../include/cllm_mathematical_constants.h"

/**
 * Apply cymatic resonance modulation to gradients
 */
void apply_cymatic_modulation(
    float* gradients,
    const uint32_t* symmetry_groups,
    uint32_t num_elements,
    uint32_t element_dim,
    uint32_t training_step,
    const double* cymatic_freqs,
    uint32_t num_freqs,
    const uint64_t* dimensional_freqs,
    double modulation_strength
) {
    if (!gradients || !symmetry_groups || !cymatic_freqs || !dimensional_freqs) return;
    if (num_elements == 0 || element_dim == 0 || num_freqs == 0) return;
    
    // Compute global phase based on training step
    // This creates a slowly varying modulation over training
    double global_phase = 2.0 * PRIME_PI * (double)training_step / 1000.0;
    
    // Apply to each element
    for (uint32_t elem_id = 0; elem_id < num_elements; elem_id++) {
        uint32_t symmetry_group = symmetry_groups[elem_id];
        
        // Compute resonance for this element
        double resonance = 0.0;
        for (uint32_t f = 0; f < num_freqs; f++) {
            // Phase for this frequency (normalized to first frequency as base)
            double freq_phase = global_phase * cymatic_freqs[f] / cymatic_freqs[0];
            
            // Add harmonic contribution
            resonance += prime_cos(freq_phase) / (double)num_freqs;
        }
        
        // Get dimensional frequency for element's symmetry group
        uint64_t phi_i = dimensional_freqs[symmetry_group % 12];
        
        // Modulate resonance with φᵢ
        double modulation = prime_cos(2.0 * PRIME_PI * (double)phi_i * resonance / 100.0);
        
        // Apply to gradients with specified modulation strength
        double scale = 1.0 + modulation_strength * modulation;
        
        for (uint32_t dim = 0; dim < element_dim; dim++) {
            gradients[elem_id * element_dim + dim] *= (float)scale;
        }
    }
}

/**
 * Compute harmonic series with golden ratio damping
 */
void compute_cymatic_harmonics(
    double base_freq,
    uint32_t num_harmonics,
    double* harmonics,
    double phi
) {
    if (!harmonics || num_harmonics == 0) return;
    
    for (uint32_t i = 0; i < num_harmonics; i++) {
        // Harmonic series: f, 2f, 3f, 4f, ...
        harmonics[i] = base_freq * (double)(i + 1);
        
        // Apply golden ratio damping: φ^(-i)
        // This makes higher harmonics progressively weaker
        double damping = prime_pow(phi, -(double)i);
        harmonics[i] *= damping;
    }
}

/**
 * Analyze gradient spectrum (simple DFT)
 */
void analyze_gradient_spectrum(
    const float* gradients,
    uint32_t size,
    double* spectrum,
    uint32_t num_bins
) {
    if (!gradients || !spectrum || size == 0 || num_bins == 0) return;
    
    // Simple DFT for frequency analysis
    // spectrum[k] = |Σ gradients[n] * e^(-2πikn/N)|²
    
    for (uint32_t k = 0; k < num_bins; k++) {
        double real = 0.0;
        double imag = 0.0;
        
        for (uint32_t n = 0; n < size; n++) {
            double angle = -2.0 * PRIME_PI * (double)k * (double)n / (double)size;
            real += (double)gradients[n] * prime_cos(angle);
            imag += (double)gradients[n] * prime_sin(angle);
        }
        
        // Power spectrum: |X[k]|²
        spectrum[k] = real * real + imag * imag;
    }
}