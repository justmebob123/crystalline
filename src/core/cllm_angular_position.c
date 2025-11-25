#include "cllm_angular_position.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Speed of sound in air (m/s) for wavelength/frequency conversions
#define SPEED_OF_SOUND 343.0

// ============================================================================
// INDIVIDUAL TERM CALCULATIONS
// ============================================================================

double angular_position_spiral_term(uint64_t prime_index) {
    // k·π(1+√5)
    // Golden ratio: φ = (1+√5)/2, so (1+√5) = 2φ - 1 ≈ 2.236067977...
    double one_plus_sqrt5 = 1.0 + sqrt(5.0);
    return (double)prime_index * M_PI * one_plus_sqrt5;
}

double angular_position_index_term(int dimension) {
    // (n-1)·2π/(12·ln3)
    if (dimension <= 0) {
        return 0.0;
    }
    
    double ln3 = LN_3;
    return (double)(dimension - 1) * (2.0 * M_PI) / (12.0 * ln3);
}

double angular_position_phonetic_term(double phonetic_wavelength) {
    // log₃(ν(λ))
    // ν(λ) = c/λ (frequency from wavelength)
    
    if (phonetic_wavelength <= 0.0) {
        return 0.0;
    }
    
    double frequency = wavelength_to_frequency(phonetic_wavelength);
    
    if (frequency <= 0.0) {
        return 0.0;
    }
    
    // log₃(x) = ln(x) / ln(3)
    double ln3 = LN_3;
    return log(frequency) / ln3;
}

double angular_position_omega_correction(uint64_t prime) {
    // ω(p) = (3/144000)·f(p)
    // where f(p) is a function of distance to 144000
    
    double lambda = cllm_get_einstein_lambda();
    
    // Calculate distance to 144000
    double distance = fabs((double)prime - (double)VECTOR_CULMINATION);
    
    // f(p) = 1 / (1 + distance/144000)
    // This makes the correction stronger near 144000
    double f_p = 1.0 / (1.0 + distance / (double)VECTOR_CULMINATION);
    
    // Apply Einstein's Lambda
    double omega = lambda * f_p;
    
    // Special handling for twin primes
    if (prime == TWIN_PRIME_LOWER || prime == TWIN_PRIME_UPPER) {
        omega *= 2.0;  // Double correction for twin primes
    }
    
    return omega;
}

double angular_position_psi_correction(uint64_t prime) {
    // ψ(p) = Plimpton 322 correction
    
    PlimptonTriple triple;
    int index = find_nearest_plimpton_triple(prime, &triple);
    
    if (index < 0) {
        return 0.0;
    }
    
    return calculate_plimpton_correction_factor(prime, &triple);
}

// ============================================================================
// ANGULAR POSITION CALCULATION
// ============================================================================

void angular_position_calculate(uint64_t prime,
                                uint64_t prime_index,
                                int dimension,
                                double phonetic_wavelength,
                                AngularPosition* result) {
    if (!result) return;
    
    // Zero out structure
    memset(result, 0, sizeof(AngularPosition));
    
    // Set input parameters
    result->prime = prime;
    result->prime_index = prime_index;
    result->dimension = dimension;
    result->phonetic_wavelength = phonetic_wavelength;
    
    // Calculate individual terms
    result->spiral_term = angular_position_spiral_term(prime_index);
    result->index_term = angular_position_index_term(dimension);
    result->phonetic_term = angular_position_phonetic_term(phonetic_wavelength);
    result->omega_correction = angular_position_omega_correction(prime);
    result->psi_correction = angular_position_psi_correction(prime);
    
    // Calculate complete theta
    result->theta = result->spiral_term + 
                   result->index_term + 
                   result->phonetic_term + 
                   result->omega_correction + 
                   result->psi_correction;
    
    // Normalize to [0, 2π)
    result->theta_normalized = angular_position_normalize(result->theta);
    
    // Calculate clock position
    angular_position_to_clock(result->theta_normalized, 
                             &result->clock_hour, 
                             &result->clock_minute);
    
    // Calculate symmetry group
    result->symmetry_group = angular_position_symmetry_group(prime);
    
    // Check boundary information
    result->is_near_144000 = angular_position_is_near_boundary(prime, 
                                                               &result->distance_to_144000);
    result->is_twin_prime = angular_position_is_twin_prime(prime);
}

void angular_position_calculate_bigfixed(uint64_t prime,
                                        uint64_t prime_index,
                                        int dimension,
                                        double phonetic_wavelength,
                                        const MathematicalConstantsBigFixed* constants,
                                        AngularPosition* result) {
    // For now, use double precision version
    // TODO: Implement full BigFixed version for maximum precision
    (void)constants;  // Unused for now
    
    angular_position_calculate(prime, prime_index, dimension, 
                              phonetic_wavelength, result);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

double angular_position_normalize(double theta) {
    // Normalize to [0, 2π)
    double two_pi = 2.0 * M_PI;
    
    // Reduce to [0, 2π) range
    theta = fmod(theta, two_pi);
    
    // Handle negative angles
    if (theta < 0.0) {
        theta += two_pi;
    }
    
    return theta;
}

void angular_position_to_clock(double theta, int* hour, double* minute) {
    if (!hour || !minute) return;
    
    // Normalize theta to [0, 2π)
    theta = angular_position_normalize(theta);
    
    // Convert to 12-hour clock
    // 0 radians = 12 o'clock (top)
    // π/2 radians = 3 o'clock (right)
    // π radians = 6 o'clock (bottom)
    // 3π/2 radians = 9 o'clock (left)
    
    // Convert to hours (0-12)
    double hours = (theta / (2.0 * M_PI)) * 12.0;
    
    // Extract hour and minute
    *hour = (int)hours % 12;
    *minute = (hours - floor(hours)) * 60.0;
}

int angular_position_symmetry_group(uint64_t prime) {
    return (int)(prime % 12);
}

int angular_position_is_near_boundary(uint64_t prime, double* distance) {
    double dist = fabs((double)prime - (double)VECTOR_CULMINATION);
    
    if (distance) {
        *distance = dist;
    }
    
    // Consider "near" as within 100 of 144000
    return (dist <= 100.0) ? 1 : 0;
}

int angular_position_is_twin_prime(uint64_t prime) {
    return (prime == TWIN_PRIME_LOWER || prime == TWIN_PRIME_UPPER) ? 1 : 0;
}

// ============================================================================
// FREQUENCY AND WAVELENGTH CONVERSIONS
// ============================================================================

double wavelength_to_frequency(double wavelength) {
    if (wavelength <= 0.0) {
        return 0.0;
    }
    
    return SPEED_OF_SOUND / wavelength;
}

double frequency_to_wavelength(double frequency) {
    if (frequency <= 0.0) {
        return 0.0;
    }
    
    return SPEED_OF_SOUND / frequency;
}

double get_phonetic_wavelength(char character) {
    // Map characters to approximate phonetic wavelengths
    // Based on formant frequencies of speech sounds
    
    char c = tolower(character);
    
    // Vowels (lower frequencies, longer wavelengths)
    if (c == 'a') return 1.372;  // ~250 Hz
    if (c == 'e') return 0.980;  // ~350 Hz
    if (c == 'i') return 0.686;  // ~500 Hz
    if (c == 'o') return 1.143;  // ~300 Hz
    if (c == 'u') return 1.225;  // ~280 Hz
    
    // Consonants (higher frequencies, shorter wavelengths)
    if (c == 's' || c == 'z') return 0.086;  // ~4000 Hz (sibilants)
    if (c == 'f' || c == 'v') return 0.098;  // ~3500 Hz (fricatives)
    if (c == 't' || c == 'd') return 0.114;  // ~3000 Hz (stops)
    if (c == 'k' || c == 'g') return 0.137;  // ~2500 Hz (velars)
    if (c == 'p' || c == 'b') return 0.171;  // ~2000 Hz (bilabials)
    if (c == 'm' || c == 'n') return 0.343;  // ~1000 Hz (nasals)
    if (c == 'l' || c == 'r') return 0.490;  // ~700 Hz (liquids)
    if (c == 'w' || c == 'y') return 0.686;  // ~500 Hz (glides)
    
    // Default for other characters
    return 0.343;  // ~1000 Hz (neutral)
}

double get_phonetic_frequency(char character) {
    double wavelength = get_phonetic_wavelength(character);
    return wavelength_to_frequency(wavelength);
}

// ============================================================================
// PLIMPTON 322 HELPERS
// ============================================================================

int find_nearest_plimpton_triple(uint64_t prime, PlimptonTriple* triple) {
    int nearest_index = -1;
    uint64_t min_distance = UINT64_MAX;
    
    for (size_t i = 0; i < PLIMPTON_322_TRIPLES_COUNT; i++) {
        const PlimptonTriple* t = &PLIMPTON_322_TRIPLES[i];
        
        // Calculate distance to each element of the triple
        uint64_t dist_a = (prime > t->a) ? (prime - t->a) : (t->a - prime);
        uint64_t dist_b = (prime > t->b) ? (prime - t->b) : (t->b - prime);
        uint64_t dist_c = (prime > t->c) ? (prime - t->c) : (t->c - prime);
        
        // Use minimum distance to any element
        uint64_t dist = dist_a;
        if (dist_b < dist) dist = dist_b;
        if (dist_c < dist) dist = dist_c;
        
        if (dist < min_distance) {
            min_distance = dist;
            nearest_index = (int)i;
            if (triple) {
                *triple = *t;
            }
        }
    }
    
    return nearest_index;
}

double calculate_plimpton_correction_factor(uint64_t prime, 
                                           const PlimptonTriple* triple) {
    if (!triple) return 0.0;
    
    // Calculate correction based on Pythagorean relationship
    // a² + b² = c²
    
    uint64_t a_sq = triple->a * triple->a;
    uint64_t b_sq = triple->b * triple->b;
    uint64_t c_sq = triple->c * triple->c;
    
    // Verify it's a valid Pythagorean triple
    if (a_sq + b_sq != c_sq) {
        return 0.0;
    }
    
    // Calculate correction factor based on prime's relationship to triple
    // ψ = (p mod c) / c
    double psi = (double)(prime % triple->c) / (double)triple->c;
    
    // Scale by 2π to get angular correction
    psi *= 2.0 * M_PI;
    
    // Normalize to [-π, π]
    if (psi > M_PI) {
        psi -= 2.0 * M_PI;
    }
    
    return psi;
}

// ============================================================================
// ANGULAR POSITION UTILITIES
// ============================================================================

void angular_position_print(const AngularPosition* pos) {
    if (!pos) {
        printf("NULL angular position\n");
        return;
    }
    
    printf("=== Angular Position ===\n");
    printf("Prime: %lu (index %lu)\n", 
           (unsigned long)pos->prime, 
           (unsigned long)pos->prime_index);
    printf("Dimension: %d\n", pos->dimension);
    printf("Symmetry Group: %d\n", pos->symmetry_group);
    printf("θ: %.6f rad (%.2f°)\n", pos->theta, pos->theta * 180.0 / M_PI);
    printf("θ (normalized): %.6f rad (%.2f°)\n", 
           pos->theta_normalized, 
           pos->theta_normalized * 180.0 / M_PI);
    printf("Clock Position: %d:%02.0f\n", pos->clock_hour, pos->clock_minute);
    
    if (pos->is_near_144000) {
        printf("⚠ Near 144000 boundary (distance: %.2f)\n", pos->distance_to_144000);
    }
    
    if (pos->is_twin_prime) {
        printf("★ Twin Prime!\n");
    }
    
    printf("========================\n");
}

void angular_position_print_detailed(const AngularPosition* pos) {
    if (!pos) {
        printf("NULL angular position\n");
        return;
    }
    
    printf("\n=== Detailed Angular Position ===\n");
    printf("Input Parameters:\n");
    printf("  Prime (p): %lu\n", (unsigned long)pos->prime);
    printf("  Prime Index (k): %lu\n", (unsigned long)pos->prime_index);
    printf("  Dimension (n): %d\n", pos->dimension);
    printf("  Phonetic Wavelength (λ): %.6f m\n", pos->phonetic_wavelength);
    
    printf("\nIndividual Terms:\n");
    printf("  Spiral Term [k·π(1+√5)]: %.6f rad\n", pos->spiral_term);
    printf("  Index Term [(n-1)·2π/(12·ln3)]: %.6f rad\n", pos->index_term);
    printf("  Phonetic Term [log₃(ν(λ))]: %.6f rad\n", pos->phonetic_term);
    printf("  Omega Correction [ω(p)]: %.6f rad\n", pos->omega_correction);
    printf("  Psi Correction [ψ(p)]: %.6f rad\n", pos->psi_correction);
    
    printf("\nFinal Result:\n");
    printf("  θ (raw): %.6f rad (%.2f°)\n", 
           pos->theta, pos->theta * 180.0 / M_PI);
    printf("  θ (normalized): %.6f rad (%.2f°)\n", 
           pos->theta_normalized, pos->theta_normalized * 180.0 / M_PI);
    
    printf("\nClock Position:\n");
    printf("  Hour: %d\n", pos->clock_hour);
    printf("  Minute: %.2f\n", pos->clock_minute);
    printf("  Display: %d:%02.0f\n", pos->clock_hour, pos->clock_minute);
    
    printf("\nSymmetry & Boundary:\n");
    printf("  Symmetry Group (p mod 12): %d\n", pos->symmetry_group);
    printf("  Near 144000: %s\n", pos->is_near_144000 ? "Yes" : "No");
    if (pos->is_near_144000) {
        printf("  Distance to 144000: %.2f\n", pos->distance_to_144000);
    }
    printf("  Twin Prime: %s\n", pos->is_twin_prime ? "Yes" : "No");
    
    printf("=================================\n\n");
}

int angular_position_validate(const AngularPosition* pos) {
    if (!pos) return 0;
    
    // Check that normalized theta is in [0, 2π)
    if (pos->theta_normalized < 0.0 || pos->theta_normalized >= 2.0 * M_PI) {
        return 0;
    }
    
    // Check clock hour is in [0, 11]
    if (pos->clock_hour < 0 || pos->clock_hour >= 12) {
        return 0;
    }
    
    // Check clock minute is in [0, 60)
    if (pos->clock_minute < 0.0 || pos->clock_minute >= 60.0) {
        return 0;
    }
    
    // Check symmetry group is in [0, 11]
    if (pos->symmetry_group < 0 || pos->symmetry_group >= 12) {
        return 0;
    }
    
    // Check dimension is non-negative
    if (pos->dimension < 0) {
        return 0;
    }
    
    return 1;
}

double angular_position_compare(const AngularPosition* pos1,
                                const AngularPosition* pos2) {
    if (!pos1 || !pos2) return 0.0;
    
    return pos1->theta - pos2->theta;
}

double angular_position_distance(const AngularPosition* pos1,
                                 const AngularPosition* pos2) {
    if (!pos1 || !pos2) return 0.0;
    
    // Calculate angular distance on circle
    double diff = fabs(pos1->theta_normalized - pos2->theta_normalized);
    
    // Take shorter path around circle
    if (diff > M_PI) {
        diff = 2.0 * M_PI - diff;
    }
    
    return diff;
}

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

void angular_position_calculate_batch(const uint64_t* primes,
                                     const uint64_t* prime_indices,
                                     int dimension,
                                     double phonetic_wavelength,
                                     size_t count,
                                     AngularPosition* results) {
    if (!primes || !prime_indices || !results) return;
    
    for (size_t i = 0; i < count; i++) {
        angular_position_calculate(primes[i], prime_indices[i], 
                                  dimension, phonetic_wavelength,
                                  &results[i]);
    }
}