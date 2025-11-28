#ifndef CLOCK_LATTICE_H
#define CLOCK_LATTICE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Babylonian clock position structure
 * Represents a position on the Babylonian clock with rings
 */
typedef struct {
    int ring;           // 0=hours, 1=minutes, 2=seconds, 3=milliseconds
    int position;       // Position on that ring (1-based)
    double angle;       // Angle in radians (12 o'clock = -π/2, 3 o'clock = 0)
    double radius;      // Normalized radius (0.0 to 1.0)
} BabylonianClockPosition;

/**
 * Modular arithmetic relationships
 * Encodes prime's relationship to clock divisions
 */
typedef struct {
    uint64_t prime;
    int mod_12;         // prime % 12 (hours)
    int mod_60;         // prime % 60 (minutes/seconds)
    int mod_100;        // prime % 100 (milliseconds)
} PrimeModular;

/**
 * 3D sphere coordinates
 * Result of folding clock into sphere
 */
typedef struct {
    float x, y, z;
} SphereCoord;

/**
 * Map prime index to clock position
 * 
 * @param prime_index 1-based prime index
 * @return Clock position
 */
BabylonianClockPosition map_prime_index_to_clock(int prime_index);

/**
 * Get modular arithmetic relationships for a prime
 * 
 * @param prime Prime number
 * @return Modular relationships
 */
PrimeModular get_prime_modular(uint64_t prime);

/**
 * Fold clock position into 3D sphere
 * 
 * @param clock_pos Clock position
 * @return 3D sphere coordinates
 */
SphereCoord fold_clock_to_sphere(BabylonianClockPosition clock_pos);

/**
 * Check if position is sacred
 * 
 * @param pos Clock position
 * @return true if sacred (π, 12 o'clock, or related to 144,000)
 */
bool is_sacred_position(BabylonianClockPosition pos);

/**
 * Map token to clock-based lattice coordinates
 * 
 * @param token_id Token ID
 * @param prime Prime number for this token
 * @param prime_index 1-based index of this prime
 * @param coords Output: 3D coordinates [x, y, z]
 */
void map_token_to_clock_lattice(uint32_t token_id, uint64_t prime, 
                                int prime_index, float* coords);

/**
 * Get ring radius for visualization
 * 
 * @param ring Ring number (0-3)
 * @param base_radius Base radius for scaling
 * @return Radius for this ring
 */
double get_ring_radius_for_visualization(int ring, double base_radius);

#ifdef __cplusplus
}
#endif

#endif // CLOCK_LATTICE_H