/**
 * @file cllm_cymatics_simulation.h
 * @brief Cymatic pattern simulation using prime frequencies
 * 
 * Cymatics is the study of visible sound and vibration patterns.
 * This module simulates cymatic patterns using prime number frequencies
 * and geometric shapes (dodecahedron, icosahedron) for crystalline AI.
 */

#ifndef CLLM_CYMATICS_SIMULATION_H
#define CLLM_CYMATICS_SIMULATION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Geometric shape for cymatic modulation
 */
typedef enum {
    CYMATIC_SHAPE_DODECA,    /**< Dodecahedron (12 faces) */
    CYMATIC_SHAPE_ICOSA,     /**< Icosahedron (20 faces) */
    CYMATIC_SHAPE_SPHERE,    /**< Sphere (continuous) */
    CYMATIC_SHAPE_CUBE       /**< Cube (6 faces) */
} CymaticShape;

/**
 * @brief Simulate cymatic pattern for prime set
 * 
 * Generates wave pattern by summing sin/cos functions for each prime pair.
 * Pattern: Σ [sin(p[i] * x) + cos(p[i+1] * x)]
 * 
 * @param output Output signal array (must be pre-allocated)
 * @param len Length of output array
 * @param primes Array of prime frequencies
 * @param num_primes Number of primes
 * @param shape Geometric shape for modulation
 */
void prime_cymatics_simulate(double* output, size_t len,
                              const uint64_t* primes, size_t num_primes,
                              CymaticShape shape);

/**
 * @brief Detect harmonic alignment in signal
 * 
 * Checks if signal has frequency peaks aligned with target primes.
 * Uses NTT-based frequency analysis.
 * 
 * @param signal Input signal
 * @param len Signal length
 * @param target_primes Target prime frequencies
 * @param num_primes Number of target primes
 * @return true if harmonically aligned
 */
bool prime_harmonic_alignment(const double* signal, size_t len,
                               const uint64_t* target_primes,
                               size_t num_primes);

/**
 * @brief Calculate cymatic resonance score
 * 
 * Measures how well signal resonates with prime frequencies.
 * Higher score = better resonance.
 * 
 * @param signal Input signal
 * @param len Signal length
 * @param primes Prime frequencies
 * @param num_primes Number of primes
 * @return Resonance score (0.0 to 1.0)
 */
double prime_cymatic_resonance(const double* signal, size_t len,
                                const uint64_t* primes, size_t num_primes);

/**
 * @brief Apply geometric modulation to signal
 * 
 * Modulates signal based on geometric shape symmetry.
 * 
 * @param signal Input/output signal (modified in place)
 * @param len Signal length
 * @param shape Geometric shape
 */
void prime_cymatic_modulate(double* signal, size_t len, CymaticShape shape);

/**
 * @brief Find dominant frequencies in signal
 * 
 * Uses NTT to identify strongest frequency components.
 * 
 * @param signal Input signal
 * @param len Signal length
 * @param frequencies Output array for dominant frequencies (pre-allocated)
 * @param max_frequencies Maximum number of frequencies to return
 * @return Number of frequencies found
 */
size_t prime_cymatic_find_peaks(const double* signal, size_t len,
                                 uint64_t* frequencies, size_t max_frequencies);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_CYMATICS_SIMULATION_H */