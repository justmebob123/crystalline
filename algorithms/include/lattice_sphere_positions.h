#ifndef LATTICE_SPHERE_POSITIONS_H
#define LATTICE_SPHERE_POSITIONS_H

#include <stdint.h>
#include <stdbool.h>
#include "math/types.h"  // Use NEW math library types

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Lattice Sphere Position System
 * 
 * REDESIGNED: Uses NEW math library (Crystalline Abacus)
 * - Removed ALL OLD library dependencies
 * - Simplified structures
 * - Pure geometric approach
 */

// Forward declarations to avoid type conflicts
typedef struct BigInt BigInt;
typedef struct CrystalAbacus CrystalAbacus;

/**
 * Lattice Partition (simplified)
 */
typedef struct {
    int symmetry_group;
    uint64_t range_start;
    uint64_t range_end;
} LatticePartition;

/**
 * Sphere Position (simplified)
 */
typedef struct {
    uint64_t sphere_id;
    int symmetry_group;
    ClockPosition clock_pos;
    LatticePartition* partition;
} SpherePosition;

/**
 * Get next sphere ID
 */
uint64_t get_next_sphere_id(void);

/**
 * Calculate clock position from symmetry group
 */
ClockPosition calculate_clock_position(int symmetry_group);

/**
 * Create lattice partition
 */
LatticePartition* create_lattice_partition(int symmetry_group,
                                          const BigInt* range_start,
                                          const BigInt* range_end);

/**
 * Free lattice partition
 */
void free_lattice_partition(LatticePartition* partition);

/**
 * Create sphere position
 */
SpherePosition* create_sphere_position(int symmetry_group,
                                       const BigInt* prime_start,
                                       const BigInt* prime_end);

/**
 * Free sphere position
 */
void free_sphere_position(SpherePosition* position);

/**
 * Check if prime is in partition
 */
bool is_prime_in_partition(const LatticePartition* partition, const BigInt* prime);

/**
 * Get partition size
 */
uint64_t get_partition_size(const LatticePartition* partition);

/**
 * Print partition info
 */
void print_partition_info(const LatticePartition* partition);

/**
 * Print sphere position
 */
void print_sphere_position(const SpherePosition* position);

/**
 * Create 12 sphere positions (12-fold symmetry)
 */
SpherePosition** create_12_sphere_positions(const BigInt* total_range_start,
                                           const BigInt* total_range_end);

/**
 * Free 12 sphere positions
 */
void free_12_sphere_positions(SpherePosition** positions);

#ifdef __cplusplus
}
#endif

#endif // LATTICE_SPHERE_POSITIONS_H