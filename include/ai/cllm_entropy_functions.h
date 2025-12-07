/**
 * @file cllm_entropy_functions.h
 * @brief Entropy calculation and reduction functions for crystalline CLLM
 * 
 * Implements Shannon entropy, recursive entropy reduction, and related
 * functions using ONLY prime_* mathematics (NO math.h).
 */

#ifndef CLLM_ENTROPY_FUNCTIONS_H
#define CLLM_ENTROPY_FUNCTIONS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate Shannon entropy of a signal
 * 
 * Computes H = -Σ p(i) * log2(p(i)) using prime_* functions only.
 * 
 * @param signal Input signal array
 * @param len Length of signal
 * @return Shannon entropy value
 */
double prime_shannon_entropy(const double* signal, size_t len);

/**
 * @brief Recursive entropy reduction
 * 
 * Reduces entropy by applying random cuts in range [cut_min, cut_max]
 * for specified number of steps. Used for search space reduction.
 * 
 * @param initial_bits Initial entropy in bits
 * @param steps Number of reduction steps
 * @param cut_min Minimum cut fraction (e.g., 0.18 = 18%)
 * @param cut_max Maximum cut fraction (e.g., 0.45 = 45%)
 * @return Reduced entropy in bits
 */
double prime_entropy_reduction(double initial_bits, uint32_t steps,
                                double cut_min, double cut_max);

/**
 * @brief Compute entropy residuals from tetrated stack
 * 
 * Calculates layer-wise entropy using modular probabilities,
 * then folds residuals into golden ratio bounds.
 * 
 * @param tetrated_stack Array of tetration layer values
 * @param stack_len Length of tetrated stack
 * @param primes Array of primes for modular calculations
 * @param num_primes Number of primes
 * @param phi_scale Golden ratio scale factor
 * @return Entropy residual value
 */
double prime_compute_entropy_residuals(const uint64_t* tetrated_stack,
                                        size_t stack_len,
                                        const uint64_t* primes,
                                        size_t num_primes,
                                        double phi_scale);

/**
 * @brief Validate ethics constraints
 * 
 * Ensures residual < threshold for low divergence.
 * Used to prevent harmful divergence in recursive algorithms.
 * 
 * @param residual Residual value to check
 * @param threshold Maximum allowed residual (e.g., 0.05)
 * @return true if residual passes ethics check
 */
bool prime_validate_ethics_residual(double residual, double threshold);

/**
 * @brief Calculate probability distribution from modular values
 * 
 * Computes normalized probabilities: p[i] = (value % prime[i]) / prime[i]
 * 
 * @param value Input value
 * @param primes Array of primes
 * @param num_primes Number of primes
 * @param prob_dist Output probability distribution (must be pre-allocated)
 * @return Sum of probabilities (for validation)
 */
double prime_modular_probability_dist(uint64_t value,
                                       const uint64_t* primes,
                                       size_t num_primes,
                                       double* prob_dist);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_ENTROPY_FUNCTIONS_H */