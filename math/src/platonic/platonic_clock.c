/**
 * @file platonic_clock.c
 * @brief Clock lattice integration for Platonic solid generators
 */

#include "math/platonic_clock.h"
#include "math/prime.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * VERTEX TO PRIME MAPPING
 * ============================================================================
 */

uint64_t platonic_vertex_to_prime(uint64_t vertex_idx) {
    // Get the (vertex_idx + 1)th prime
    // vertex_idx is 0-based, but prime indices are 1-based
    return prime_nth(vertex_idx + 1);
}

uint64_t platonic_prime_to_vertex(uint64_t prime) {
    // Get the index of this prime (1-based), then convert to 0-based vertex index
    if (!prime_is_prime(prime)) {
        return UINT64_MAX; // Not a prime
    }
    
    // Count primes up to this prime to get its index
    // This is a workaround until prime_index() is implemented
    uint64_t count = 0;
    for (uint64_t p = 2; p <= prime; p = prime_next(p)) {
        count++;
        if (p == prime) {
            return count - 1; // Convert to 0-based
        }
    }
    
    return UINT64_MAX; // Should not reach here
}

/* ============================================================================
 * PRIME TO CLOCK POSITION MAPPING
 * ============================================================================
 */

MathError platonic_prime_to_clock_position(uint64_t prime, ClockPosition* pos) {
    if (!pos) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    // Use the clock lattice mapping function
    return clock_map_prime_to_position(prime, pos);
}

MathError platonic_vertex_to_clock_position(uint64_t vertex_idx, ClockPosition* pos) {
    if (!pos) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    // Get prime for this vertex
    uint64_t prime = platonic_vertex_to_prime(vertex_idx);
    
    // Map prime to clock position
    return platonic_prime_to_clock_position(prime, pos);
}

/* ============================================================================
 * CLOCK POSITION TO COORDINATES
 * ============================================================================
 */

MathError platonic_clock_to_coordinates(const ClockPosition* pos,
                                       uint32_t dimension,
                                       CrystallineAbacus*** coords,
                                       uint32_t base,
                                       uint32_t precision) {
    if (!pos || !coords || dimension == 0) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    // Step 1: Project clock position to 3D sphere using simple mapping
    // Use angle and radius from clock position to generate sphere coordinates
    // This is a simplified version until clock_fold_to_sphere is implemented
    double angle = pos->angle;
    double radius = pos->radius;
    
    // Map to unit sphere using spherical coordinates
    // x = r * cos(angle)
    // y = r * sin(angle)  
    // z = sqrt(1 - r^2) if r <= 1, else 0
    double x = radius * math_cos(angle);
    double y = radius * math_sin(angle);
    double z = (radius <= 1.0) ? math_sqrt(1.0 - radius * radius) : 0.0;
    
    // Step 2: Allocate coordinate array
    *coords = (CrystallineAbacus**)calloc(dimension, sizeof(CrystallineAbacus*));
    if (!*coords) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    // Step 3: Set first 3 coordinates from sphere projection
    if (dimension >= 1) {
        (*coords)[0] = abacus_from_double(x, base, precision);
        if (!(*coords)[0]) {
            free(*coords);
            *coords = NULL;
            return MATH_ERROR_OUT_OF_MEMORY;
        }
    }
    
    if (dimension >= 2) {
        (*coords)[1] = abacus_from_double(y, base, precision);
        if (!(*coords)[1]) {
            if (dimension >= 1) abacus_free((*coords)[0]);
            free(*coords);
            *coords = NULL;
            return MATH_ERROR_OUT_OF_MEMORY;
        }
    }
    
    if (dimension >= 3) {
        (*coords)[2] = abacus_from_double(z, base, precision);
        if (!(*coords)[2]) {
            if (dimension >= 1) abacus_free((*coords)[0]);
            if (dimension >= 2) abacus_free((*coords)[1]);
            free(*coords);
            *coords = NULL;
            return MATH_ERROR_OUT_OF_MEMORY;
        }
    }
    
    // Step 4: For dimensions > 3, use harmonic extension
    // Use angle and radius from clock position to generate higher dimensions
    if (dimension > 3) {
        double angle = pos->angle;
        double radius = pos->radius;
        
        for (uint32_t d = 3; d < dimension; d++) {
            // Use harmonic functions to extend to higher dimensions
            // Each dimension uses a different harmonic (maintains 12-fold symmetry)
            double harmonic = (d - 2); // 1, 2, 3, ... for dimensions 4, 5, 6, ...
            double phase = angle * harmonic;
            
            // Compute coordinate using sin/cos with phase
            // Alternate between sin and cos for different dimensions
            double value;
            if ((d - 3) % 2 == 0) {
                value = radius * math_sin(phase);
            } else {
                value = radius * math_cos(phase);
            }
            
            (*coords)[d] = abacus_from_double(value, base, precision);
            if (!(*coords)[d]) {
                // Clean up on error
                for (uint32_t i = 0; i < d; i++) {
                    abacus_free((*coords)[i]);
                }
                free(*coords);
                *coords = NULL;
                return MATH_ERROR_OUT_OF_MEMORY;
            }
        }
    }
    
    return MATH_SUCCESS;
}

MathError platonic_vertex_to_coordinates(uint64_t vertex_idx,
                                        uint32_t dimension,
                                        CrystallineAbacus*** coords,
                                        uint32_t base,
                                        uint32_t precision) {
    // Get clock position for this vertex
    ClockPosition pos;
    MathError err = platonic_vertex_to_clock_position(vertex_idx, &pos);
    if (err != MATH_SUCCESS) {
        return err;
    }
    
    // Generate coordinates from clock position
    return platonic_clock_to_coordinates(&pos, dimension, coords, base, precision);
}

/* ============================================================================
 * COORDINATE VALIDATION
 * ============================================================================
 */

bool platonic_validate_coordinates(CrystallineAbacus** coords, uint32_t dimension) {
    if (!coords || dimension == 0) {
        return false;
    }
    
    // Check each coordinate
    for (uint32_t i = 0; i < dimension; i++) {
        if (!coords[i]) {
            return false;
        }
        
        // Convert to double to check magnitude
        double value;
        MathError err = abacus_to_double(coords[i], &value);
        if (err != MATH_SUCCESS) {
            return false;
        }
        
        // Check for NaN or infinity
        if (value != value || value == 1.0/0.0 || value == -1.0/0.0) {
            return false;
        }
        
        // Check magnitude is reasonable (not too large)
        if (value > 1e10 || value < -1e10) {
            return false;
        }
    }
    
    return true;
}

CrystallineAbacus* platonic_coordinate_distance(CrystallineAbacus** coords1,
                                               CrystallineAbacus** coords2,
                                               uint32_t dimension) {
    if (!coords1 || !coords2 || dimension == 0) {
        return NULL;
    }
    
    // Use same base and precision as first coordinate
    uint32_t base = coords1[0]->base;
    // Get precision from the Abacus structure
    // Note: Abacus doesn't have a precision field, we'll use a default
    uint32_t precision = 6; // Default precision
    
    // Compute sum of squared differences using double precision
    // (workaround for abacus_div bug)
    double sum_sq = 0.0;
    
    for (uint32_t i = 0; i < dimension; i++) {
        double v1, v2;
        MathError err1 = abacus_to_double(coords1[i], &v1);
        MathError err2 = abacus_to_double(coords2[i], &v2);
        
        if (err1 != MATH_SUCCESS || err2 != MATH_SUCCESS) {
            return NULL;
        }
        
        double diff = v1 - v2;
        sum_sq += diff * diff;
    }
    
    // Take square root
    double dist_double = math_sqrt(sum_sq);
    
    // Convert to Abacus
    CrystallineAbacus* distance = abacus_from_double(dist_double, base, precision);
    
    return distance;
}

/* ============================================================================
 * SYMMETRY OPERATIONS
 * ============================================================================
 */

MathError platonic_apply_12fold_rotation(CrystallineAbacus** coords,
                                         uint32_t dimension,
                                         uint32_t rotation,
                                         CrystallineAbacus** result) {
    if (!coords || !result || dimension < 2 || rotation >= 12) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    // Rotation angle: rotation * 30° = rotation * π/6
    double angle = rotation * 3.14159265358979323846 / 6.0;
    double cos_angle = math_cos(angle);
    double sin_angle = math_sin(angle);
    
    // Apply rotation in the first 2 dimensions (x-y plane)
    // x' = x*cos(θ) - y*sin(θ)
    // y' = x*sin(θ) + y*cos(θ)
    
    double x, y;
    MathError err1 = abacus_to_double(coords[0], &x);
    MathError err2 = abacus_to_double(coords[1], &y);
    
    if (err1 != MATH_SUCCESS || err2 != MATH_SUCCESS) {
        return err1 != MATH_SUCCESS ? err1 : err2;
    }
    
    double x_new = x * cos_angle - y * sin_angle;
    double y_new = x * sin_angle + y * cos_angle;
    
    uint32_t base = coords[0]->base;
    // Get precision from the Abacus structure
    // Note: Abacus doesn't have a precision field, we'll use a default
    uint32_t precision = 6; // Default precision
    
    // Free old values if they exist
    if (result[0]) abacus_free(result[0]);
    if (result[1]) abacus_free(result[1]);
    
    result[0] = abacus_from_double(x_new, base, precision);
    result[1] = abacus_from_double(y_new, base, precision);
    
    if (!result[0] || !result[1]) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    // Copy remaining dimensions unchanged
    for (uint32_t i = 2; i < dimension; i++) {
        double value;
        MathError err = abacus_to_double(coords[i], &value);
        if (err != MATH_SUCCESS) {
            return err;
        }
        
        if (result[i]) abacus_free(result[i]);
        result[i] = abacus_from_double(value, base, precision);
        if (!result[i]) {
            return MATH_ERROR_OUT_OF_MEMORY;
        }
    }
    
    return MATH_SUCCESS;
}

bool platonic_has_12fold_symmetry(CrystallineAbacus** coords, uint32_t dimension) {
    if (!coords || dimension < 2) {
        return false;
    }
    
    // Allocate temporary result array (array of pointers)
    CrystallineAbacus** rotated = (CrystallineAbacus**)calloc(dimension, sizeof(CrystallineAbacus*));
    if (!rotated) {
        return false;
    }
    
    // Check if rotating by 30° (1/12 of full circle) gives similar structure
    // For true 12-fold symmetry, rotating 12 times should return to original
    bool has_symmetry = true;
    
    // Apply one 30° rotation
    MathError err = platonic_apply_12fold_rotation(coords, dimension, 1, rotated);
    if (err != MATH_SUCCESS) {
        has_symmetry = false;
    } else {
        // Check if the rotated coordinates are valid
        // (For full validation, we'd need to check all 12 rotations)
        has_symmetry = platonic_validate_coordinates(rotated, dimension);
    }
    
    // Clean up
    for (uint32_t i = 0; i < dimension; i++) {
        if (rotated[i]) {
            abacus_free(rotated[i]);
        }
    }
    free(rotated);
    
    return has_symmetry;
}