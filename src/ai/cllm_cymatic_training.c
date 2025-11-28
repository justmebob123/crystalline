/**
 * CLLM Cymatic Frequency Training
 * 
 * Implements OBJECTIVE 18: Apply Cymatic Frequency Resonance to Training
 * 
 * This file applies cymatic frequencies (432 Hz, 528 Hz, etc.) to modulate
 * gradients during training. This creates smoother convergence and better
 * final loss by aligning training dynamics with natural frequency patterns.
 * 
 * Cymatic frequencies:
 * - 432 Hz: Universal base frequency
 * - 528 Hz: DNA repair frequency
 * - 639 Hz: Connection frequency
 * - 741 Hz: Awakening frequency
 * - 852 Hz: Intuition frequency
 * - 963 Hz: Divine frequency
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_mathematical_constants.h"
#include "../include/prime_float_math.h"

/**
 * Apply cymatic resonance to gradients
 * 
 * Modulates gradients using cymatic frequencies and dimensional frequencies.
 * This creates a frequency-domain modulation that smooths convergence.
 * 
 * @param model CLLM model
 * @param gradients Gradient buffer [vocab_size x embedding_dim]
 * @param training_step Current training step
 */
void cllm_apply_cymatic_resonance(CLLMModel* model,
                                  float* gradients,
                                  uint32_t training_step) {
    if (!model || !gradients) return;
    
    // Cymatic frequencies (Hz)
    double frequencies[] = {
        CYMATIC_432_HZ,  // Universal base
        CYMATIC_528_HZ,  // DNA repair
        CYMATIC_639_HZ,  // Connection
        CYMATIC_741_HZ,  // Awakening
        CYMATIC_852_HZ,  // Intuition
        CYMATIC_963_HZ   // Divine
    };
    int num_freqs = 6;
    
    // Compute global phase based on training step
    // This creates a slowly varying modulation over training
    double global_phase = 2.0 * PRIME_PI * (double)training_step / 1000.0;
    
    uint32_t vocab_size = model->vocab_size;
    uint32_t embedding_dim = model->embeddings.embedding_dim;
    
    // Apply to each token
    for (uint32_t token_id = 0; token_id < vocab_size; token_id++) {
        CLLMToken* token = &model->tokens[token_id];
        
        // Compute resonance for this token
        double resonance = 0.0;
        for (int f = 0; f < num_freqs; f++) {
            // Phase for this frequency (normalized to 432 Hz base)
            double freq_phase = global_phase * frequencies[f] / CYMATIC_432_HZ;
            
            // Add harmonic contribution
            resonance += prime_cos(freq_phase) / (double)num_freqs;
        }
        
        // Get dimensional frequency for token's symmetry group
        uint64_t phi_i = cllm_get_dimensional_frequency(token->symmetry_group);
        
        // Modulate resonance with φᵢ
        double modulation = prime_cos(2.0 * PRIME_PI * (double)phi_i * resonance / 100.0);
        
        // Apply to gradients (10% modulation strength)
        double scale = 1.0 + 0.1 * modulation;
        
        for (uint32_t dim = 0; dim < embedding_dim; dim++) {
            gradients[token_id * embedding_dim + dim] *= (float)scale;
        }
    }
}

/**
 * Compute harmonic series for a base frequency
 * 
 * Generates harmonics with golden ratio damping: f, 2f, 3f, ... × φ^(-i)
 * 
 * @param base_freq Base frequency (Hz)
 * @param num_harmonics Number of harmonics to generate
 * @param harmonics Output harmonic frequencies [num_harmonics]
 */
void cllm_compute_harmonics(double base_freq,
                            uint32_t num_harmonics,
                            double* harmonics) {
    if (!harmonics || num_harmonics == 0) return;
    
    for (uint32_t i = 0; i < num_harmonics; i++) {
        // Harmonic series: f, 2f, 3f, 4f, ...
        harmonics[i] = base_freq * (double)(i + 1);
        
        // Apply golden ratio damping: φ^(-i)
        // This makes higher harmonics progressively weaker
        double damping = prime_pow(LATTICE_PHI, -(double)i);
        harmonics[i] *= damping;
    }
}

/**
 * Analyze gradient spectrum (frequency domain)
 * 
 * Performs simple DFT to analyze gradient frequencies.
 * Useful for understanding training dynamics.
 * 
 * @param gradients Gradient buffer [size]
 * @param size Gradient buffer size
 * @param spectrum Output power spectrum [spectrum_size]
 * @param spectrum_size Number of frequency bins
 */
void cllm_analyze_gradient_spectrum(float* gradients,
                                    uint32_t size,
                                    double* spectrum,
                                    uint32_t spectrum_size) {
    if (!gradients || !spectrum || size == 0 || spectrum_size == 0) return;
    
    // Simple DFT for analysis (not for training - too slow)
    for (uint32_t k = 0; k < spectrum_size; k++) {
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

/**
 * Apply frequency-domain filtering to gradients
 * 
 * Removes high-frequency noise while preserving low-frequency trends.
 * 
 * @param gradients Gradient buffer [size]
 * @param size Gradient buffer size
 * @param cutoff_freq Cutoff frequency (0.0 to 1.0)
 */
void cllm_filter_gradients_frequency(float* gradients,
                                     uint32_t size,
                                     double cutoff_freq) {
    if (!gradients || size == 0) return;
    
    // Allocate spectrum
    double* spectrum = calloc(size, sizeof(double));
    if (!spectrum) return;
    
    // Analyze spectrum
    cllm_analyze_gradient_spectrum(gradients, size, spectrum, size);
    
    // Find cutoff index
    size_t cutoff_idx = (size_t)(cutoff_freq * size);
    
    // Zero out high frequencies
    for (size_t i = cutoff_idx; i < size; i++) {
        spectrum[i] = 0.0;
    }
    
    // Apply inverse DFT (reconstruct filtered gradients)
    for (uint32_t n = 0; n < size; n++) {
        double filtered = 0.0;
        
        for (uint32_t k = 0; k < size; k++) {
            if (k >= cutoff_idx) continue;  // Skip zeroed frequencies
            
            double angle = 2.0 * PRIME_PI * (double)k * (double)n / (double)size;
            filtered += prime_sqrt(spectrum[k]) * prime_cos(angle) / (double)size;
        }
        
        gradients[n] = (float)filtered;
    }
    
    free(spectrum);
}

/**
 * Get cymatic modulation for a specific training step
 * 
 * Returns the current cymatic modulation value.
 * 
 * @param training_step Current training step
 * @return Modulation value in [-1, 1]
 */
double cllm_get_cymatic_modulation(uint32_t training_step) {
    double frequencies[] = {
        CYMATIC_432_HZ,
        CYMATIC_528_HZ,
        CYMATIC_639_HZ,
        CYMATIC_741_HZ,
        CYMATIC_852_HZ,
        CYMATIC_963_HZ
    };
    int num_freqs = 6;
    
    double global_phase = 2.0 * PRIME_PI * (double)training_step / 1000.0;
    
    double resonance = 0.0;
    for (int f = 0; f < num_freqs; f++) {
        double freq_phase = global_phase * frequencies[f] / CYMATIC_432_HZ;
        resonance += prime_cos(freq_phase) / (double)num_freqs;
    }
    
    return resonance;
}

/**
 * Print cymatic resonance statistics
 * 
 * @param training_step Current training step
 */
void cllm_print_cymatic_stats(uint32_t training_step) {
    printf("\n=== Cymatic Resonance Statistics (step %u) ===\n", training_step);
    
    double modulation = cllm_get_cymatic_modulation(training_step);
    printf("Current modulation: %.6f\n", modulation);
    
    // Show individual frequency contributions
    double frequencies[] = {
        CYMATIC_432_HZ,
        CYMATIC_528_HZ,
        CYMATIC_639_HZ,
        CYMATIC_741_HZ,
        CYMATIC_852_HZ,
        CYMATIC_963_HZ
    };
    const char* names[] = {
        "Universal (432 Hz)",
        "DNA Repair (528 Hz)",
        "Connection (639 Hz)",
        "Awakening (741 Hz)",
        "Intuition (852 Hz)",
        "Divine (963 Hz)"
    };
    
    double global_phase = 2.0 * PRIME_PI * (double)training_step / 1000.0;
    
    printf("\nIndividual frequency contributions:\n");
    for (int f = 0; f < 6; f++) {
        double freq_phase = global_phase * frequencies[f] / CYMATIC_432_HZ;
        double contribution = prime_cos(freq_phase);
        printf("  %s: %.6f\n", names[f], contribution);
    }
    
    printf("\n");
}