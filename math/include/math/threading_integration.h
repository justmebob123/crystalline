/**
 * @file threading_integration.h
 * @brief Integration between Math Library and Threading System
 * 
 * Provides functions to integrate abacus and rainbow table with
 * hierarchical threading and memory systems.
 * 
 * Key Features:
 * - Thread ID to geometric position mapping
 * - Memory segment to abacus representation
 * - Rainbow table for O(1) lookups
 * - Clock lattice for thread organization
 */

#ifndef MATH_THREADING_INTEGRATION_H
#define MATH_THREADING_INTEGRATION_H

#include "types.h"
#include "abacus.h"
#include "rainbow.h"
#include "clock.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// THREAD ID TO GEOMETRIC POSITION
// ============================================================================

/**
 * Map thread ID to geometric position on clock lattice
 * 
 * Uses abacus representation to compute geometric position.
 * 
 * @param thread_id Thread ID
 * @param base Number base (typically 12 for 12-fold symmetry)
 * @param out_position Output clock position
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError thread_id_to_position(
    uint64_t thread_id,
    uint32_t base,
    ClockPosition* out_position
);

/**
 * Map geometric position to thread ID
 * 
 * Inverse of thread_id_to_position.
 * 
 * @param position Clock position
 * @param base Number base
 * @param out_thread_id Output thread ID
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError position_to_thread_id(
    const ClockPosition* position,
    uint32_t base,
    uint64_t* out_thread_id
);

/**
 * Compute N-dimensional position for thread
 * 
 * Projects thread position into N-dimensional space using
 * prime-based geometric projection.
 * 
 * @param thread_id Thread ID
 * @param num_dimensions Number of dimensions
 * @param out_position Output position [num_dimensions]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError thread_compute_nd_position(
    uint64_t thread_id,
    uint32_t num_dimensions,
    double* out_position
);

// ============================================================================
// MEMORY SEGMENT TO ABACUS
// ============================================================================

/**
 * Map memory segment to abacus representation
 * 
 * Converts segment ID and properties to abacus for geometric operations.
 * 
 * @param segment_id Segment ID
 * @param symmetry_group Symmetry group
 * @param base Number base
 * @return Abacus representation, or NULL on error
 */
CrystallineAbacus* segment_to_abacus(
    uint64_t segment_id,
    uint32_t symmetry_group,
    uint32_t base
);

/**
 * Map abacus to memory segment ID
 * 
 * Inverse of segment_to_abacus.
 * 
 * @param abacus Abacus representation
 * @param out_segment_id Output segment ID
 * @param out_symmetry_group Output symmetry group
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError abacus_to_segment(
    const CrystallineAbacus* abacus,
    uint64_t* out_segment_id,
    uint32_t* out_symmetry_group
);

// ============================================================================
// RAINBOW TABLE FOR THREADING
// ============================================================================

/**
 * Create rainbow table for thread IDs
 * 
 * Maps thread IDs to geometric positions for O(1) lookup.
 * 
 * @param max_threads Maximum number of threads
 * @param base Number base
 * @return Rainbow table, or NULL on error
 */
RainbowTable* create_thread_rainbow_table(
    uint64_t max_threads,
    uint32_t base
);

/**
 * Lookup thread position in rainbow table
 * 
 * O(1) lookup of thread's geometric position.
 * 
 * @param table Rainbow table
 * @param thread_id Thread ID
 * @param out_position Output clock position
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError rainbow_lookup_thread_position(
    const RainbowTable* table,
    uint64_t thread_id,
    ClockPosition* out_position
);

/**
 * Create rainbow table for memory segments
 * 
 * Maps segment IDs to geometric positions for O(1) lookup.
 * 
 * @param max_segments Maximum number of segments
 * @param base Number base
 * @return Rainbow table, or NULL on error
 */
RainbowTable* create_segment_rainbow_table(
    uint64_t max_segments,
    uint32_t base
);

/**
 * Lookup segment position in rainbow table
 * 
 * O(1) lookup of segment's geometric position.
 * 
 * @param table Rainbow table
 * @param segment_id Segment ID
 * @param out_position Output clock position
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError rainbow_lookup_segment_position(
    const RainbowTable* table,
    uint64_t segment_id,
    ClockPosition* out_position
);

// ============================================================================
// GEOMETRIC THREAD ORGANIZATION
// ============================================================================

/**
 * Organize threads geometrically on clock lattice
 * 
 * Computes optimal geometric positions for N threads using
 * 12-fold symmetry and clock lattice structure.
 * 
 * @param num_threads Number of threads
 * @param base Number base (typically 12)
 * @param out_positions Output positions [num_threads]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError organize_threads_geometrically(
    uint32_t num_threads,
    uint32_t base,
    ClockPosition* out_positions
);

/**
 * Compute thread neighbors using clock geometry
 * 
 * Finds 12 nearest neighbors for a thread using clock lattice structure.
 * 
 * @param thread_id Thread ID
 * @param num_threads Total number of threads
 * @param base Number base
 * @param out_neighbors Output neighbor IDs [12]
 * @return Number of neighbors found (up to 12)
 */
int compute_thread_neighbors(
    uint64_t thread_id,
    uint64_t num_threads,
    uint32_t base,
    uint64_t* out_neighbors
);

// ============================================================================
// COPRIME RELATIONSHIPS
// ============================================================================

/**
 * Check if two threads can communicate (coprime check)
 * 
 * Uses coprime relationships to determine if threads can communicate.
 * 
 * @param thread_a First thread ID
 * @param thread_b Second thread ID
 * @param base Number base
 * @return true if threads can communicate
 */
bool threads_can_communicate(
    uint64_t thread_a,
    uint64_t thread_b,
    uint32_t base
);

/**
 * Compute coprime matrix for threads
 * 
 * Computes coprime relationships between all threads.
 * 
 * @param num_threads Number of threads
 * @param base Number base
 * @param out_matrix Output matrix [num_threads * num_threads]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError compute_thread_coprime_matrix(
    uint32_t num_threads,
    uint32_t base,
    uint64_t** out_matrix
);

// ============================================================================
// LOAD BALANCING USING ABACUS
// ============================================================================

/**
 * Compute optimal work distribution using abacus
 * 
 * Uses geometric principles to distribute work optimally across threads.
 * 
 * @param total_work Total work units
 * @param num_threads Number of threads
 * @param base Number base
 * @param out_distribution Output work per thread [num_threads]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError compute_work_distribution(
    uint64_t total_work,
    uint32_t num_threads,
    uint32_t base,
    uint64_t* out_distribution
);

/**
 * Balance load using geometric transformation
 * 
 * Redistributes work using geometric transformations on clock lattice.
 * 
 * @param current_loads Current load per thread [num_threads]
 * @param num_threads Number of threads
 * @param base Number base
 * @param out_new_loads Output new loads [num_threads]
 * @return MATH_SUCCESS on success, error code otherwise
 */
MathError balance_load_geometrically(
    const uint64_t* current_loads,
    uint32_t num_threads,
    uint32_t base,
    uint64_t* out_new_loads
);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Print thread geometric information
 * 
 * @param thread_id Thread ID
 * @param base Number base
 */
void print_thread_geometry(uint64_t thread_id, uint32_t base);

/**
 * Print segment geometric information
 * 
 * @param segment_id Segment ID
 * @param symmetry_group Symmetry group
 * @param base Number base
 */
void print_segment_geometry(
    uint64_t segment_id,
    uint32_t symmetry_group,
    uint32_t base
);

/**
 * Validate thread organization
 * 
 * Checks if thread organization follows geometric principles.
 * 
 * @param num_threads Number of threads
 * @param positions Thread positions [num_threads]
 * @param base Number base
 * @return true if valid
 */
bool validate_thread_organization(
    uint32_t num_threads,
    const ClockPosition* positions,
    uint32_t base
);

#ifdef __cplusplus
}
#endif

#endif // MATH_THREADING_INTEGRATION_H