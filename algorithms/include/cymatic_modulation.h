#ifndef CYMATIC_MODULATION_H
#define CYMATIC_MODULATION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * General Cymatic Frequency Modulation Algorithm
 * 
 * This is a GENERAL algorithm that can be used by any training system.
 * It applies cymatic frequency resonance to modulate gradients for smoother convergence.
 * 
 * Cymatic frequencies: 432 Hz, 528 Hz, 639 Hz, 741 Hz, 852 Hz, 963 Hz
 */

/**
 * Apply cymatic resonance modulation to gradients
 * 
 * @param gradients Gradient buffer [num_elements x element_dim]
 * @param symmetry_groups Symmetry group for each element [num_elements]
 * @param num_elements Number of elements (e.g., vocab size)
 * @param element_dim Dimension of each element (e.g., embedding dim)
 * @param training_step Current training step (for phase calculation)
 * @param cymatic_freqs Array of cymatic frequencies (Hz)
 * @param num_freqs Number of cymatic frequencies
 * @param dimensional_freqs Array of dimensional frequencies φᵢ
 * @param modulation_strength Modulation strength (0.0 to 1.0, typically 0.1)
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
);

/**
 * Compute harmonic series with golden ratio damping
 * 
 * Generates: f, 2f, 3f, ... × φ^(-i)
 * 
 * @param base_freq Base frequency (Hz)
 * @param num_harmonics Number of harmonics to generate
 * @param harmonics Output harmonic frequencies [num_harmonics]
 * @param phi Golden ratio for damping
 */
void compute_cymatic_harmonics(
    double base_freq,
    uint32_t num_harmonics,
    double* harmonics,
    double phi
);

/**
 * Analyze gradient spectrum (simple DFT)
 * 
 * @param gradients Gradient buffer [size]
 * @param size Gradient buffer size
 * @param spectrum Output power spectrum [size/2]
 * @param num_bins Number of frequency bins (size/2)
 */
void analyze_gradient_spectrum(
    const float* gradients,
    uint32_t size,
    double* spectrum,
    uint32_t num_bins
);

#ifdef __cplusplus
}
#endif

#endif /* CYMATIC_MODULATION_H */