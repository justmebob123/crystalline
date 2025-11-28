#ifndef CLLM_CYMATIC_TRAINING_H
#define CLLM_CYMATIC_TRAINING_H

#include "../cllm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Apply cymatic resonance to gradients
 * 
 * Modulates gradients using cymatic frequencies (432 Hz, 528 Hz, etc.)
 * and dimensional frequencies φᵢ. Creates smoother convergence.
 * 
 * @param model CLLM model
 * @param gradients Gradient buffer [vocab_size x embedding_dim]
 * @param training_step Current training step
 */
void cllm_apply_cymatic_resonance(CLLMModel* model,
                                  float* gradients,
                                  uint32_t training_step);

/**
 * Compute harmonic series for a base frequency
 * 
 * @param base_freq Base frequency (Hz)
 * @param num_harmonics Number of harmonics to generate
 * @param harmonics Output harmonic frequencies [num_harmonics]
 */
void cllm_compute_harmonics(double base_freq,
                            uint32_t num_harmonics,
                            double* harmonics);

/**
 * Analyze gradient spectrum (frequency domain)
 * 
 * @param gradients Gradient buffer [size]
 * @param size Gradient buffer size
 * @param spectrum Output power spectrum [spectrum_size]
 * @param spectrum_size Number of frequency bins
 */
void cllm_analyze_gradient_spectrum(float* gradients,
                                    uint32_t size,
                                    double* spectrum,
                                    uint32_t spectrum_size);

/**
 * Apply frequency-domain filtering to gradients
 * 
 * @param gradients Gradient buffer [size]
 * @param size Gradient buffer size
 * @param cutoff_freq Cutoff frequency (0.0 to 1.0)
 */
void cllm_filter_gradients_frequency(float* gradients,
                                     uint32_t size,
                                     double cutoff_freq);

/**
 * Get cymatic modulation for a specific training step
 * 
 * @param training_step Current training step
 * @return Modulation value in [-1, 1]
 */
double cllm_get_cymatic_modulation(uint32_t training_step);

/**
 * Print cymatic resonance statistics
 * 
 * @param training_step Current training step
 */
void cllm_print_cymatic_stats(uint32_t training_step);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_CYMATIC_TRAINING_H */