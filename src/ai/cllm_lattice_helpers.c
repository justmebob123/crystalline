/**
 * @file cllm_lattice_helpers.c
 * @brief Helper functions for CLLM lattice operations
 * 
 * Implements application-specific functions using NEW math library.
 */

#include "math/angular_position.h"
#include "math/types.h"
#include "phonetic_values.h"

/**
 * @brief Get dimensional frequency for a given dimension
 * 
 * Based on cymatic frequencies and dimensional indexing.
 * Uses 432 Hz as base frequency (universal cymatic frequency).
 * 
 * @param dimension Dimension index (0-based)
 * @return Frequency in Hz
 */
double cllm_get_dimensional_frequency(int dimension) {
    // Base frequency: 432 Hz (universal cymatic frequency)
    double base_freq = 432.0;
    
    // Each dimension adds a harmonic
    // frequency = base_freq * (dimension + 1)
    return base_freq * (dimension + 1);
}

/**
 * @brief Check if prime is near 144000 boundary
 * 
 * Uses angular_position function from NEW math library.
 * 
 * @param prime Prime value
 * @return 1 if near boundary, 0 otherwise
 */
int cllm_is_near_144000_boundary(uint64_t prime) {
    double distance;
    return angular_position_is_near_boundary(prime, &distance);
}