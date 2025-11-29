#ifndef LATTICE_SPHERE_POSITIONS_H
#define LATTICE_SPHERE_POSITIONS_H

#include <stdint.h>
#include <stdbool.h>
#include "prime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Lattice Sphere Position System
 * 
 * Manages sphere positions in the crystalline lattice with special
 * handling for the 144000 boundary region.
 * 
 * This module handles thread-safe sphere positioning and partitioning
 * in the crystalline lattice hierarchy.
 */

// Mathematical constants - use definitions from prime_types.h
#ifndef VECTOR_CULMINATION
#define VECTOR_CULMINATION 144000
#endif
#ifndef TWIN_PRIME_LOWER
#define TWIN_PRIME_LOWER 143999
#endif
#ifndef TWIN_PRIME_UPPER
#define TWIN_PRIME_UPPER 144001
#endif
#define EINSTEIN_LAMBDA_NUMERATOR 3
#define EINSTEIN_LAMBDA_DENOMINATOR 144000

/**
 * Boundary Region (144000 special handling)
 * 
 * The region around 144000 requires special processing due to:
 * - Einstein's Λ correction (ω = 3/144000)
 * - Twin primes at boundaries (143999, 144001)
 * - Vector culmination point
 */
typedef struct {
    BigInt center;                 // 144000
    BigInt lower_twin;             // 143999
    BigInt upper_twin;             // 144001
    
    // Einstein's Λ correction
    double omega_correction;       // 3/144000
    
    // Flags
    bool requires_special_handling;
    bool is_boundary_region;
    
} BoundaryRegion;

/**
 * Lattice Partition
 * 
 * Defines a partition of the lattice assigned to a sphere.
 * Partitions are based on symmetry groups (mod 12).
 */
typedef struct {
    int symmetry_group;            // 0-11 (which residue class mod 12)
    
    // Range boundaries
    BigInt range_start;            // Start of range (inclusive)
    BigInt range_end;              // End of range (inclusive)
    
    // Prime characteristics
    uint64_t expected_prime_count; // Estimated primes in range
    double prime_density;          // Actual density (primes / range)
    
    // Mathematical properties
    int modulo_class;              // symmetry_group (mod 12)
    bool contains_twin_boundary;   // True if range includes 143999-144001
    
    // Boundary region (if applicable)
    BoundaryRegion* boundary;      // NULL if not in boundary region
    
} LatticePartition;

/**
 * Sphere Position
 * 
 * Complete position information for a sphere in the hierarchy.
 */
typedef struct {
    // Lattice position
    BigInt prime_center;           // Central prime for this sphere
    BigInt prime_radius;           // Radius in prime space
    
    // Symmetry
    int symmetry_group;            // 0-11
    ClockPosition clock_pos;       // Position on 12-hour clock face
    
    // Partition
    LatticePartition* partition;   // Lattice partition owned by this sphere
    
    // Hierarchy
    int depth;                     // Depth in hierarchy (0 = root)
    uint64_t sphere_id;            // Unique sphere identifier
    
} SpherePosition;

/**
 * Create sphere position
 * 
 * @param symmetry_group Symmetry group (0-11)
 * @param center Central prime (can be NULL for auto-calculation)
 * @param radius Radius in prime space (can be NULL for auto-calculation)
 * @param depth Depth in hierarchy
 * @return Allocated position, or NULL on error
 */
SpherePosition* sphere_position_create(int symmetry_group, 
                                       const BigInt* center,
                                       const BigInt* radius,
                                       int depth);

/**
 * Free sphere position
 * 
 * @param pos Position to free
 */
void sphere_position_free(SpherePosition* pos);

/**
 * Check if prime is in sphere's partition
 * 
 * @param pos Sphere position
 * @param prime Prime to check
 * @return true if prime is in partition, false otherwise
 */
bool sphere_position_contains_prime(const SpherePosition* pos, const BigInt* prime);

/**
 * Check if range intersects 144000 boundary
 * 
 * @param range_start Start of range
 * @param range_end End of range
 * @return true if range includes 143999-144001, false otherwise
 */
bool check_144000_boundary(const BigInt* range_start, const BigInt* range_end);

/**
 * Create boundary region for 144000
 * 
 * @return Allocated boundary region, or NULL on error
 */
BoundaryRegion* create_boundary_region(void);

/**
 * Free boundary region
 * 
 * @param boundary Boundary to free
 */
void free_boundary_region(BoundaryRegion* boundary);

/**
 * Get Einstein's Λ correction for position
 * 
 * Returns ω = 3/144000 if in boundary region, 0.0 otherwise.
 * 
 * @param pos Sphere position
 * @return Einstein's Λ correction value
 */
double get_einstein_lambda_correction(const SpherePosition* pos);

/**
 * Create lattice partition
 * 
 * @param symmetry_group Symmetry group (0-11)
 * @param range_start Start of range
 * @param range_end End of range
 * @return Allocated partition, or NULL on error
 */
LatticePartition* create_lattice_partition(int symmetry_group,
                                           const BigInt* range_start,
                                           const BigInt* range_end);

/**
 * Free lattice partition
 * 
 * @param partition Partition to free
 */
void free_lattice_partition(LatticePartition* partition);

/**
 * Estimate prime count in partition
 * 
 * Uses prime number theorem to estimate primes in range.
 * 
 * @param partition Partition to estimate
 * @return Estimated prime count
 */
uint64_t estimate_partition_prime_count(const LatticePartition* partition);

/**
 * Validate sphere position
 * 
 * Checks for consistency and correctness.
 * 
 * @param pos Position to validate
 * @return true if valid, false otherwise
 */
bool validate_sphere_position(const SpherePosition* pos);

/**
 * Print sphere position info (for debugging)
 * 
 * @param pos Position to print
 */
void print_sphere_position(const SpherePosition* pos);

/**
 * Calculate clock position from symmetry group
 * 
 * Maps symmetry group (0-11) to clock position.
 * 
 * @param symmetry_group Symmetry group (0-11)
 * @return Clock position
 */
ClockPosition calculate_clock_position(int symmetry_group);

/**
 * Check if prime is in symmetry group
 * 
 * @param prime Prime to check
 * @param symmetry_group Symmetry group (0-11)
 * @return true if prime ≡ symmetry_group (mod 12), false otherwise
 */
bool is_prime_in_symmetry_group(const BigInt* prime, int symmetry_group);

/**
 * Get next sphere ID
 * 
 * Thread-safe generation of unique sphere IDs.
 * 
 * @return Unique sphere ID
 */
uint64_t get_next_sphere_id(void);

#ifdef __cplusplus
}
#endif

#endif /* LATTICE_SPHERE_POSITIONS_H */