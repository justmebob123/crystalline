#ifndef CLLM_ANGULAR_POSITION_H
#define CLLM_ANGULAR_POSITION_H

#include <stdint.h>
#include "bigfixed_core.h"
#include "cllm_mathematical_constants.h"

/**
 * Angular Position System
 * 
 * Complete implementation of the angular position formula with all corrections:
 * 
 * θ(p, k, n, λ) = k·π(1+√5) + (n-1)·2π/(12·ln3) + log₃(ν(λ)) + ω(p) + ψ(p)
 * 
 * Where:
 * - k = prime index in sequence
 * - p = prime value
 * - n = dimension index
 * - λ = phonetic wavelength
 * - ω(p) = Einstein's Λ correction = (3/144000)·f(p)
 * - ψ(p) = Plimpton 322 correction based on Pythagorean triples
 * 
 * This formula determines the angular position of a prime in the lattice,
 * incorporating spiral geometry, dimensional indexing, phonetic resonance,
 * and mathematical corrections.
 */

// ============================================================================
// ANGULAR POSITION STRUCTURE
// ============================================================================

/**
 * Angular Position
 * 
 * Complete angular position with all terms and corrections.
 */
typedef struct {
    // Input parameters
    uint64_t prime;             // Prime value (p)
    uint64_t prime_index;       // Prime index in sequence (k)
    int dimension;              // Dimension index (n)
    double phonetic_wavelength; // Phonetic wavelength (λ)
    
    // Individual terms
    double spiral_term;         // k·π(1+√5)
    double index_term;          // (n-1)·2π/(12·ln3)
    double phonetic_term;       // log₃(ν(λ))
    double omega_correction;    // ω(p) = Einstein's Λ correction
    double psi_correction;      // ψ(p) = Plimpton 322 correction
    
    // Final result
    double theta;               // Complete angular position
    double theta_normalized;    // Normalized to [0, 2π)
    
    // Clock position (12-hour face)
    int clock_hour;             // Hour position (0-11)
    double clock_minute;        // Minute position (0.0-60.0)
    
    // Symmetry group
    int symmetry_group;         // p mod 12
    
    // Boundary information
    int is_near_144000;         // 1 if near 144000 boundary
    int is_twin_prime;          // 1 if twin prime (143999 or 144001)
    double distance_to_144000;  // Distance to 144000
    
} AngularPosition;

// ============================================================================
// ANGULAR POSITION CALCULATION
// ============================================================================

/**
 * Calculate complete angular position
 * 
 * Computes all terms and corrections for the angular position formula.
 * 
 * @param prime Prime value (p)
 * @param prime_index Prime index in sequence (k)
 * @param dimension Dimension index (n)
 * @param phonetic_wavelength Phonetic wavelength (λ)
 * @param result Pointer to AngularPosition structure to fill
 */
void angular_position_calculate(uint64_t prime,
                                uint64_t prime_index,
                                int dimension,
                                double phonetic_wavelength,
                                AngularPosition* result);

/**
 * Calculate angular position with BigFixed precision
 * 
 * High-precision version using BigFixed arithmetic.
 * 
 * @param prime Prime value
 * @param prime_index Prime index
 * @param dimension Dimension index
 * @param phonetic_wavelength Phonetic wavelength
 * @param constants Mathematical constants (BigFixed)
 * @param result Pointer to AngularPosition structure to fill
 */
void angular_position_calculate_bigfixed(uint64_t prime,
                                        uint64_t prime_index,
                                        int dimension,
                                        double phonetic_wavelength,
                                        const MathematicalConstantsBigFixed* constants,
                                        AngularPosition* result);

// ============================================================================
// INDIVIDUAL TERM CALCULATIONS
// ============================================================================

/**
 * Calculate spiral term: k·π(1+√5)
 * 
 * This term represents the golden spiral component.
 * 
 * @param prime_index Prime index (k)
 * @return Spiral term value
 */
double angular_position_spiral_term(uint64_t prime_index);

/**
 * Calculate index term: (n-1)·2π/(12·ln3)
 * 
 * This term represents the dimensional indexing.
 * 
 * @param dimension Dimension index (n)
 * @return Index term value
 */
double angular_position_index_term(int dimension);

/**
 * Calculate phonetic term: log₃(ν(λ))
 * 
 * This term represents the phonetic resonance.
 * ν(λ) is the frequency corresponding to wavelength λ.
 * 
 * @param phonetic_wavelength Wavelength (λ)
 * @return Phonetic term value
 */
double angular_position_phonetic_term(double phonetic_wavelength);

/**
 * Calculate omega correction: ω(p) = (3/144000)·f(p)
 * 
 * Einstein's Λ correction based on distance to 144000.
 * 
 * @param prime Prime value (p)
 * @return Omega correction value
 */
double angular_position_omega_correction(uint64_t prime);

/**
 * Calculate psi correction: ψ(p)
 * 
 * Plimpton 322 correction based on Pythagorean triples.
 * 
 * @param prime Prime value (p)
 * @return Psi correction value
 */
double angular_position_psi_correction(uint64_t prime);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Normalize angle to [0, 2π)
 * 
 * @param theta Angle in radians
 * @return Normalized angle
 */
double angular_position_normalize(double theta);

/**
 * Convert angle to clock position
 * 
 * Maps angle to 12-hour clock face.
 * 
 * @param theta Angle in radians
 * @param hour Pointer to store hour (0-11)
 * @param minute Pointer to store minute (0.0-60.0)
 */
void angular_position_to_clock(double theta, int* hour, double* minute);

/**
 * Get symmetry group from prime
 * 
 * @param prime Prime value
 * @return Symmetry group (p mod 12)
 */
int angular_position_symmetry_group(uint64_t prime);

/**
 * Check if prime is near 144000 boundary
 * 
 * @param prime Prime value
 * @param distance Pointer to store distance (optional)
 * @return 1 if near boundary, 0 otherwise
 */
int angular_position_is_near_boundary(uint64_t prime, double* distance);

/**
 * Check if prime is a twin prime (143999 or 144001)
 * 
 * @param prime Prime value
 * @return 1 if twin prime, 0 otherwise
 */
int angular_position_is_twin_prime(uint64_t prime);

// ============================================================================
// FREQUENCY AND WAVELENGTH CONVERSIONS
// ============================================================================

/**
 * Convert wavelength to frequency
 * 
 * ν = c/λ where c is speed of sound in air (343 m/s)
 * 
 * @param wavelength Wavelength in meters
 * @return Frequency in Hz
 */
double wavelength_to_frequency(double wavelength);

/**
 * Convert frequency to wavelength
 * 
 * λ = c/ν where c is speed of sound in air (343 m/s)
 * 
 * @param frequency Frequency in Hz
 * @return Wavelength in meters
 */
double frequency_to_wavelength(double frequency);

/**
 * Get phonetic wavelength for character
 * 
 * Maps characters to phonetic wavelengths based on
 * acoustic properties of speech sounds.
 * 
 * @param character Character to map
 * @return Phonetic wavelength in meters
 */
double get_phonetic_wavelength(char character);

/**
 * Get phonetic frequency for character
 * 
 * @param character Character to map
 * @return Phonetic frequency in Hz
 */
double get_phonetic_frequency(char character);

// ============================================================================
// PLIMPTON 322 HELPERS
// ============================================================================

/**
 * Find nearest Plimpton 322 triple for prime
 * 
 * Finds the Pythagorean triple from Plimpton 322 that is
 * closest to the given prime value.
 * 
 * @param prime Prime value
 * @param triple Pointer to store triple (optional)
 * @return Index of nearest triple, or -1 if none found
 */
int find_nearest_plimpton_triple(uint64_t prime, PlimptonTriple* triple);

/**
 * Calculate Plimpton 322 correction factor
 * 
 * Computes the correction factor based on the relationship
 * between the prime and the nearest Pythagorean triple.
 * 
 * @param prime Prime value
 * @param triple Plimpton 322 triple
 * @return Correction factor
 */
double calculate_plimpton_correction_factor(uint64_t prime, 
                                           const PlimptonTriple* triple);

// ============================================================================
// ANGULAR POSITION UTILITIES
// ============================================================================

/**
 * Print angular position information
 * 
 * @param pos Angular position to print
 */
void angular_position_print(const AngularPosition* pos);

/**
 * Print detailed angular position breakdown
 * 
 * Shows all individual terms and corrections.
 * 
 * @param pos Angular position to print
 */
void angular_position_print_detailed(const AngularPosition* pos);

/**
 * Validate angular position
 * 
 * Checks that all values are within expected ranges.
 * 
 * @param pos Angular position to validate
 * @return 1 if valid, 0 otherwise
 */
int angular_position_validate(const AngularPosition* pos);

/**
 * Compare two angular positions
 * 
 * @param pos1 First position
 * @param pos2 Second position
 * @return Difference in theta (pos1 - pos2)
 */
double angular_position_compare(const AngularPosition* pos1,
                                const AngularPosition* pos2);

/**
 * Calculate angular distance between two positions
 * 
 * Returns the shortest angular distance on the circle.
 * 
 * @param pos1 First position
 * @param pos2 Second position
 * @return Angular distance in radians [0, π]
 */
double angular_position_distance(const AngularPosition* pos1,
                                 const AngularPosition* pos2);

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

/**
 * Calculate angular positions for multiple primes
 * 
 * More efficient than calling angular_position_calculate multiple times.
 * 
 * @param primes Array of prime values
 * @param prime_indices Array of prime indices
 * @param dimension Dimension index (same for all)
 * @param phonetic_wavelength Phonetic wavelength (same for all)
 * @param count Number of primes
 * @param results Array to store results
 */
void angular_position_calculate_batch(const uint64_t* primes,
                                     const uint64_t* prime_indices,
                                     int dimension,
                                     double phonetic_wavelength,
                                     size_t count,
                                     AngularPosition* results);

#endif // CLLM_ANGULAR_POSITION_H