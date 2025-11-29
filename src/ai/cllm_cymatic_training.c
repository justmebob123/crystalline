/**
 * CLLM Cymatic Frequency Training (CLLM-Specific Wrapper)
 * 
 * Implements OBJECTIVE 18: Apply Cymatic Frequency Resonance to Training
 * 
 * This file is a THIN WRAPPER around the general cymatic modulation algorithm
 * in the algorithms layer. It extracts CLLM-specific data and calls the
 * general algorithm.
 * 
 * ARCHITECTURE:
 * - Layer 1 (Crystalline): Cymatic frequency constants - pure math
 * - Layer 2 (Algorithms): apply_cymatic_modulation() - general algorithm
 * - Layer 3 (CLLM): cllm_apply_cymatic_resonance() - CLLM wrapper (THIS FILE)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_mathematical_constants.h"
#include "../include/prime_float_math.h"
#include "../../algorithms/include/cymatic_modulation.h"

/**
 * Apply cymatic resonance to gradients (CLLM Wrapper)
 * 
 * This is a THIN WRAPPER that extracts CLLM-specific data and calls
 * the general apply_cymatic_modulation() from the algorithms layer.
 * 
 * @param model CLLM model
 * @param gradients Gradient buffer [vocab_size x embedding_dim]
 * @param training_step Current training step
 */
void cllm_apply_cymatic_resonance(CLLMModel* model,
                                  float* gradients,
                                  uint32_t training_step) {
    if (!model || !gradients) return;
    
    // Cymatic frequencies (Hz) - from Layer 1 (crystalline)
    double frequencies[] = {
        CYMATIC_432_HZ,  // Universal base
        CYMATIC_528_HZ,  // DNA repair
        CYMATIC_639_HZ,  // Connection
        CYMATIC_741_HZ,  // Awakening
        CYMATIC_852_HZ,  // Intuition
        CYMATIC_963_HZ   // Divine
    };
    uint32_t num_freqs = 6;
    
    // Extract symmetry groups from CLLM tokens
    uint32_t vocab_size = model->vocab_size;
    uint32_t* symmetry_groups = (uint32_t*)malloc(vocab_size * sizeof(uint32_t));
    if (!symmetry_groups) return;
    
    for (uint32_t i = 0; i < vocab_size; i++) {
        symmetry_groups[i] = model->tokens[i].symmetry_group;
    }
    
    // Get dimensional frequencies - from Layer 1 (crystalline)
    uint64_t dimensional_freqs[12];
    for (uint32_t i = 0; i < 12; i++) {
        dimensional_freqs[i] = cllm_get_dimensional_frequency(i);
    }
    
    // Call Layer 2 (algorithms) general cymatic modulation
    apply_cymatic_modulation(
        gradients,
        symmetry_groups,
        vocab_size,
        model->embeddings.embedding_dim,
        training_step,
        frequencies,
        num_freqs,
        dimensional_freqs,
        0.1  // 10% modulation strength
    );
    
    free(symmetry_groups);
}

/**
 * Compute harmonic series for a base frequency (CLLM Wrapper)
 * 
 * This is a THIN WRAPPER that calls the general compute_cymatic_harmonics()
 * from the algorithms layer.
 * 
 * @param base_freq Base frequency (Hz)
 * @param num_harmonics Number of harmonics to generate
 * @param harmonics Output harmonic frequencies [num_harmonics]
 */
void cllm_compute_harmonics(double base_freq,
                            uint32_t num_harmonics,
                            double* harmonics) {
    // Call Layer 2 (algorithms) general harmonic computation
    compute_cymatic_harmonics(base_freq, num_harmonics, harmonics, LATTICE_PHI);
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
    // Call Layer 2 (algorithms) general spectrum analysis
    analyze_gradient_spectrum(gradients, size, spectrum, spectrum_size);
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