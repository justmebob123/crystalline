/**
 * clock_lattice.c - Correct Clock-Based Lattice Mapping
 * 
 * This implements the TRUE crystalline lattice structure based on:
 * - Babylonian clock mathematics (12, 60, 60, 100)
 * - Clock tick positions (not spiral)
 * - Modular arithmetic relationships
 * - Sacred geometry (π at 3 o'clock, 144,000 vector)
 * - Clock-to-sphere folding via stereographic projection
 * 
 * The ancient proverb (0→1→2→3→∞):
 * - 0 begets 1: Outside ring (∞/0) creates center (unity)
 * - 1 begets 2: Unity creates duality (radius/line)
 * - 2 begets 3: Line + point = triangle
 * - 3 leads to all things: Triangle → sphere, π crosses to next sphere
 */

#include "clock_lattice.h"
#include "prime_float_math.h"
#include "prime_types.h"
#include <stdio.h>

/**
 * Map prime index to clock position
 * 
 * Babylonian structure:
 * - Ring 0: Positions 1-12 (hours) - OUTER ring (smaller primes)
 * - Ring 1: Positions 1-60 (minutes)
 * - Ring 2: Positions 1-60 (seconds)
 * - Ring 3: Positions 1-100 (milliseconds) - INNER ring (larger primes)
 * 
 * @param prime_index 1-based prime index (1st prime, 2nd prime, etc.)
 * @return Clock position
 */
BabylonianClockPosition map_prime_index_to_clock(int prime_index) {
    BabylonianClockPosition pos;
    
    if (prime_index < 12) {
        // Ring 0: Hours (12 positions) - OUTER
        pos.ring = 0;
        pos.position = prime_index;
        // 12 o'clock = position 12, 3 o'clock = position 3
        // Angle: position 3 = 0°, position 12 = -90° (top)
        pos.angle = (pos.position - 3) * (2.0 * PRIME_PI / 12.0);
        pos.radius = 0.25;  // Outer ring (25% from center)
        
    } else if (prime_index < 72) {
        // Ring 1: Minutes (60 positions)
        pos.ring = 1;
        pos.position = prime_index - 12;
        // Position 15 = 3 o'clock (0°)
        pos.angle = (pos.position - 15) * (2.0 * PRIME_PI / 60.0);
        pos.radius = 0.50;  // 50% from center
        
    } else if (prime_index < 132) {
        // Ring 2: Seconds (60 positions)
        pos.ring = 2;
        pos.position = prime_index - 72;
        pos.angle = (pos.position - 15) * (2.0 * PRIME_PI / 60.0);
        pos.radius = 0.75;  // 75% from center
        
    } else if (prime_index < 232) {
        // Ring 3: Milliseconds (100 positions) - INNER
        pos.ring = 3;
        pos.position = prime_index - 132;
        // Position 25 = 3 o'clock (0°)
        pos.angle = (pos.position - 25) * (2.0 * PRIME_PI / 100.0);
        pos.radius = 1.00;  // Inner ring (100% from center)
        
    } else {
        // Beyond 232: Continue pattern with larger rings
        pos.ring = 4;
        pos.position = prime_index - 232;
        pos.angle = (pos.position * 2.0 * PRIME_PI) / 1000.0;
        pos.radius = 1.25;
    }
    
    return pos;
}

/**
 * Get modular arithmetic relationships for a prime
 */
PrimeModular get_prime_modular(uint64_t prime) {
    PrimeModular pm;
    pm.prime = prime;
    pm.mod_12 = prime % 12;
    pm.mod_60 = prime % 60;
    pm.mod_100 = prime % 100;
    return pm;
}

/**
 * Fold clock position into 3D sphere
 * 
 * Uses stereographic projection from first quadrant.
 * The clock folds along the sacred triangle (0→1→2→3→∞).
 * 
 * @param clock_pos Clock position
 * @return 3D sphere coordinates
 */
SphereCoord fold_clock_to_sphere(BabylonianClockPosition clock_pos) {
    SphereCoord sphere;
    
    // Normalize angle to [0, 2π]
    double theta = clock_pos.angle;
    double two_pi = 2.0 * PRIME_PI;
    while (theta < 0) theta += two_pi;
    while (theta >= two_pi) theta -= two_pi;
    
    // Fold into first quadrant (mirrored sudoku folding)
    // This maps all 4 quadrants into the sacred triangle
    double quarter_pi = PRIME_PI / 2.0;
    double folded_theta = theta;
    while (folded_theta >= quarter_pi) folded_theta -= quarter_pi;
    
    // Radius determines latitude on sphere
    // Outer rings (small radius) → near north pole
    // Inner rings (large radius) → near equator
    double phi = clock_pos.radius * PRIME_PI;  // 0 to π
    
    // Convert to 3D sphere coordinates (unit sphere)
    sphere.x = prime_sin(phi) * prime_cos(folded_theta);
    sphere.y = prime_sin(phi) * prime_sin(folded_theta);
    sphere.z = prime_cos(phi);
    
    return sphere;
}

/**
 * Check if position is sacred (π, 12 o'clock, or related to 144,000)
 */
bool is_sacred_position(BabylonianClockPosition pos) {
    // 3 o'clock = π position
    if (pos.ring == 0 && pos.position == 3) return true;
    
    // 12 o'clock = top position
    if (pos.ring == 0 && pos.position == 12) return true;
    
    // Positions related to 144,000 = 12 × 12 × 1000
    // Any position divisible by 12 has special significance
    if (pos.position % 12 == 0) return true;
    
    return false;
}

/**
 * Map token to clock-based lattice coordinates
 * 
 * This is the CORRECT mapping that respects:
 * - Babylonian clock structure
 * - Modular arithmetic
 * - Sacred geometry
 * - Clock-to-sphere folding
 * 
 * @param token_id Token ID
 * @param prime Prime number for this token
 * @param prime_index 1-based index of this prime (1st, 2nd, 3rd, etc.)
 * @param coords Output: 3D coordinates [x, y, z]
 */
void map_token_to_clock_lattice(uint32_t token_id __attribute__((unused)), 
                                uint64_t prime, 
                                int prime_index, float* coords) {
    // Get clock position
    BabylonianClockPosition clock_pos = map_prime_index_to_clock(prime_index);
    
    // Fold to 3D sphere
    SphereCoord sphere = fold_clock_to_sphere(clock_pos);
    
    // Store base 3D coordinates
    coords[0] = sphere.x;
    coords[1] = sphere.y;
    coords[2] = sphere.z;
    
    // Add modular arithmetic relationships as small perturbations
    // This encodes the prime's relationship to clock divisions
    PrimeModular pm = get_prime_modular(prime);
    
    // Small perturbation based on modular relationships
    // This creates subtle structure within each clock position
    coords[0] += 0.01f * (pm.mod_12 / 12.0f);
    coords[1] += 0.01f * (pm.mod_60 / 60.0f);
    coords[2] += 0.01f * (pm.mod_100 / 100.0f);
    
    // Normalize to unit sphere
    float norm = prime_sqrtf(coords[0]*coords[0] + coords[1]*coords[1] + coords[2]*coords[2]);
    if (norm > 1e-8f) {
        coords[0] /= norm;
        coords[1] /= norm;
        coords[2] /= norm;
    }
}

/**
 * Get ring radius for visualization
 * 
 * @param ring Ring number (0-3)
 * @param base_radius Base radius for scaling
 * @return Radius for this ring
 */
double get_ring_radius_for_visualization(int ring, double base_radius) {
    switch (ring) {
        case 0: return base_radius * 0.25;  // Hours (outer)
        case 1: return base_radius * 0.50;  // Minutes
        case 2: return base_radius * 0.75;  // Seconds
        case 3: return base_radius * 1.00;  // Milliseconds (inner)
        default: return base_radius * 1.25;
    }
}