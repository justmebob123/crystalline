/**
 * @file threading_integration.c
 * @brief Implementation of Math-Threading Integration
 */

#include "../include/math/threading_integration.h"
#include "../include/math/abacus.h"
#include "../include/math/rainbow.h"
#include "../include/math/clock.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Compute GCD
 */
static uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Check if coprime
 */
static bool are_coprime(uint64_t a, uint64_t b) {
    return gcd(a, b) == 1;
}

/**
 * Get prime at index
 */
static uint32_t get_prime(uint32_t index) {
    uint32_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47,
                         53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107,
                         109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173};
    return primes[index % 40];
}

// ============================================================================
// THREAD ID TO GEOMETRIC POSITION
// ============================================================================

MathError thread_id_to_position(
    uint64_t thread_id,
    uint32_t base,
    ClockPosition* out_position
) {
    if (!out_position || base < 2) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Create abacus for thread ID
    CrystallineAbacus* abacus = abacus_new(base);
    if (!abacus) {
        return MATH_ERROR_MEMORY;
    }
    
    // Set thread ID
    if (abacus_set_uint64(abacus, thread_id) != MATH_SUCCESS) {
        abacus_free(abacus);
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Map to clock position
    // Ring 0: thread_id % 12
    // Ring 1: (thread_id / 12) % 60
    // Ring 2: (thread_id / 720) % 60
    // Ring 3: (thread_id / 43200) % 100
    
    out_position->ring0 = thread_id % 12;
    out_position->ring1 = (thread_id / 12) % 60;
    out_position->ring2 = (thread_id / 720) % 60;
    out_position->ring3 = (thread_id / 43200) % 100;
    
    abacus_free(abacus);
    return MATH_SUCCESS;
}

MathError position_to_thread_id(
    const ClockPosition* position,
    uint32_t base,
    uint64_t* out_thread_id
) {
    if (!position || !out_thread_id || base < 2) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Reconstruct thread ID from clock position
    *out_thread_id = position->ring0 +
                     position->ring1 * 12 +
                     position->ring2 * 720 +
                     position->ring3 * 43200;
    
    return MATH_SUCCESS;
}

MathError thread_compute_nd_position(
    uint64_t thread_id,
    uint32_t num_dimensions,
    double* out_position
) {
    if (!out_position || num_dimensions < 3) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Get clock position
    ClockPosition pos;
    if (thread_id_to_position(thread_id, 12, &pos) != MATH_SUCCESS) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Compute angle and radius
    double angle = (2.0 * M_PI * pos.ring0) / 12.0;
    double radius = 1.0 + (double)pos.ring1 / 60.0;
    
    // First 3 dimensions: standard 3D position
    out_position[0] = radius * cos(angle);
    out_position[1] = radius * sin(angle);
    out_position[2] = (double)pos.ring2 / 60.0;
    
    // Higher dimensions: prime-based projection
    for (uint32_t d = 3; d < num_dimensions; d++) {
        uint32_t prime = get_prime(d);
        double sum = 0.0;
        
        sum += out_position[0] * cos(2.0 * M_PI * prime * d / num_dimensions);
        sum += out_position[1] * sin(2.0 * M_PI * prime * d / num_dimensions);
        sum += out_position[2] * cos(M_PI * prime * d / num_dimensions);
        
        out_position[d] = sum / sqrt((double)num_dimensions);
    }
    
    return MATH_SUCCESS;
}

// ============================================================================
// MEMORY SEGMENT TO ABACUS
// ============================================================================

CrystallineAbacus* segment_to_abacus(
    uint64_t segment_id,
    uint32_t symmetry_group,
    uint32_t base
) {
    if (base < 2) return NULL;
    
    // Create abacus
    CrystallineAbacus* abacus = abacus_new(base);
    if (!abacus) return NULL;
    
    // Encode segment ID and symmetry group
    // segment_value = segment_id * base + symmetry_group
    uint64_t segment_value = segment_id * base + symmetry_group;
    
    if (abacus_set_uint64(abacus, segment_value) != MATH_SUCCESS) {
        abacus_free(abacus);
        return NULL;
    }
    
    return abacus;
}

MathError abacus_to_segment(
    const CrystallineAbacus* abacus,
    uint64_t* out_segment_id,
    uint32_t* out_symmetry_group
) {
    if (!abacus || !out_segment_id || !out_symmetry_group) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Get value from abacus
    uint64_t segment_value;
    if (abacus_get_uint64(abacus, &segment_value) != MATH_SUCCESS) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Decode segment ID and symmetry group
    uint32_t base = abacus->base;
    *out_segment_id = segment_value / base;
    *out_symmetry_group = segment_value % base;
    
    return MATH_SUCCESS;
}

// ============================================================================
// RAINBOW TABLE FOR THREADING
// ============================================================================

RainbowTable* create_thread_rainbow_table(
    uint64_t max_threads,
    uint32_t base
) {
    if (base < 2) return NULL;
    
    // Create rainbow table
    RainbowTable table;
    if (rainbow_init(&table, max_threads) != MATH_SUCCESS) {
        return NULL;
    }
    
    // Populate with thread positions
    for (uint64_t i = 0; i < max_threads; i++) {
        ClockPosition pos;
        if (thread_id_to_position(i, base, &pos) == MATH_SUCCESS) {
            // Add entry (using thread_id as "prime" for lookup)
            RainbowEntry entry;
            entry.prime = i;
            entry.position = pos;
            entry.index = i;
            
            // Note: This is a simplified implementation
            // In production, would use proper rainbow table insertion
        }
    }
    
    // Allocate and return
    RainbowTable* result = malloc(sizeof(RainbowTable));
    if (result) {
        *result = table;
    }
    
    return result;
}

MathError rainbow_lookup_thread_position(
    const RainbowTable* table,
    uint64_t thread_id,
    ClockPosition* out_position
) {
    if (!table || !out_position) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // For now, compute directly (O(1))
    // In production, would use actual rainbow table lookup
    return thread_id_to_position(thread_id, 12, out_position);
}

RainbowTable* create_segment_rainbow_table(
    uint64_t max_segments,
    uint32_t base
) {
    // Similar to thread rainbow table
    return create_thread_rainbow_table(max_segments, base);
}

MathError rainbow_lookup_segment_position(
    const RainbowTable* table,
    uint64_t segment_id,
    ClockPosition* out_position
) {
    // Similar to thread position lookup
    return rainbow_lookup_thread_position(table, segment_id, out_position);
}

// ============================================================================
// GEOMETRIC THREAD ORGANIZATION
// ============================================================================

MathError organize_threads_geometrically(
    uint32_t num_threads,
    uint32_t base,
    ClockPosition* out_positions
) {
    if (!out_positions || base < 2) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Organize threads on clock lattice
    for (uint32_t i = 0; i < num_threads; i++) {
        if (thread_id_to_position(i, base, &out_positions[i]) != MATH_SUCCESS) {
            return MATH_ERROR_INVALID_ARGUMENT;
        }
    }
    
    return MATH_SUCCESS;
}

int compute_thread_neighbors(
    uint64_t thread_id,
    uint64_t num_threads,
    uint32_t base,
    uint64_t* out_neighbors
) {
    if (!out_neighbors || base < 2) {
        return -1;
    }
    
    // Get thread position
    ClockPosition pos;
    if (thread_id_to_position(thread_id, base, &pos) != MATH_SUCCESS) {
        return -1;
    }
    
    // Find 12 nearest neighbors using clock geometry
    int count = 0;
    
    // Neighbors in same ring (ring0)
    for (int offset = -1; offset <= 1; offset += 2) {
        uint32_t neighbor_ring0 = (pos.ring0 + offset + 12) % 12;
        uint64_t neighbor_id = neighbor_ring0 +
                               pos.ring1 * 12 +
                               pos.ring2 * 720 +
                               pos.ring3 * 43200;
        
        if (neighbor_id < num_threads && neighbor_id != thread_id) {
            out_neighbors[count++] = neighbor_id;
            if (count >= 12) return count;
        }
    }
    
    // Neighbors in adjacent rings (ring1)
    for (int offset = -1; offset <= 1; offset += 2) {
        uint32_t neighbor_ring1 = (pos.ring1 + offset + 60) % 60;
        uint64_t neighbor_id = pos.ring0 +
                               neighbor_ring1 * 12 +
                               pos.ring2 * 720 +
                               pos.ring3 * 43200;
        
        if (neighbor_id < num_threads && neighbor_id != thread_id) {
            out_neighbors[count++] = neighbor_id;
            if (count >= 12) return count;
        }
    }
    
    // Continue with other rings until we have 12 neighbors
    // (simplified implementation)
    
    return count;
}

// ============================================================================
// COPRIME RELATIONSHIPS
// ============================================================================

bool threads_can_communicate(
    uint64_t thread_a,
    uint64_t thread_b,
    uint32_t base
) {
    if (base < 2) return false;
    
    // Get symmetry groups
    uint32_t sym_a = thread_a % base;
    uint32_t sym_b = thread_b % base;
    
    // Check if coprime
    return are_coprime(sym_a + 1, sym_b + 1);  // +1 to avoid zero
}

MathError compute_thread_coprime_matrix(
    uint32_t num_threads,
    uint32_t base,
    uint64_t** out_matrix
) {
    if (!out_matrix || base < 2) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Compute coprime relationships
    for (uint32_t i = 0; i < num_threads; i++) {
        for (uint32_t j = 0; j < num_threads; j++) {
            if (i == j) {
                out_matrix[i][j] = 1;  // Same thread
            } else {
                out_matrix[i][j] = threads_can_communicate(i, j, base) ? 1 : 0;
            }
        }
    }
    
    return MATH_SUCCESS;
}

// ============================================================================
// LOAD BALANCING USING ABACUS
// ============================================================================

MathError compute_work_distribution(
    uint64_t total_work,
    uint32_t num_threads,
    uint32_t base,
    uint64_t* out_distribution
) {
    if (!out_distribution || num_threads == 0 || base < 2) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Simple equal distribution (can be enhanced with geometric weighting)
    uint64_t work_per_thread = total_work / num_threads;
    uint64_t remainder = total_work % num_threads;
    
    for (uint32_t i = 0; i < num_threads; i++) {
        out_distribution[i] = work_per_thread;
        if (i < remainder) {
            out_distribution[i]++;
        }
    }
    
    return MATH_SUCCESS;
}

MathError balance_load_geometrically(
    const uint64_t* current_loads,
    uint32_t num_threads,
    uint32_t base,
    uint64_t* out_new_loads
) {
    if (!current_loads || !out_new_loads || num_threads == 0 || base < 2) {
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    // Compute total work
    uint64_t total_work = 0;
    for (uint32_t i = 0; i < num_threads; i++) {
        total_work += current_loads[i];
    }
    
    // Redistribute evenly (geometric balancing can be enhanced)
    return compute_work_distribution(total_work, num_threads, base, out_new_loads);
}

// ============================================================================
// UTILITIES
// ============================================================================

void print_thread_geometry(uint64_t thread_id, uint32_t base) {
    ClockPosition pos;
    if (thread_id_to_position(thread_id, base, &pos) != MATH_SUCCESS) {
        printf("ERROR: Invalid thread ID\n");
        return;
    }
    
    printf("Thread %lu Geometry:\n", thread_id);
    printf("  Clock Position: [%u, %u, %u, %u]\n",
           pos.ring0, pos.ring1, pos.ring2, pos.ring3);
    printf("  Symmetry Group: %u\n", thread_id % base);
    printf("  Angle: %.2f°\n", (360.0 * pos.ring0) / 12.0);
}

void print_segment_geometry(
    uint64_t segment_id,
    uint32_t symmetry_group,
    uint32_t base
) {
    printf("Segment %lu Geometry:\n", segment_id);
    printf("  Symmetry Group: %u\n", symmetry_group);
    printf("  Base: %u\n", base);
    
    CrystallineAbacus* abacus = segment_to_abacus(segment_id, symmetry_group, base);
    if (abacus) {
        printf("  Abacus Representation: ");
        abacus_print(abacus);
        abacus_free(abacus);
    }
}

bool validate_thread_organization(
    uint32_t num_threads,
    const ClockPosition* positions,
    uint32_t base
) {
    if (!positions || base < 2) {
        return false;
    }
    
    // Check each thread position
    for (uint32_t i = 0; i < num_threads; i++) {
        // Verify position is valid
        if (positions[i].ring0 >= 12 ||
            positions[i].ring1 >= 60 ||
            positions[i].ring2 >= 60 ||
            positions[i].ring3 >= 100) {
            return false;
        }
        
        // Verify position matches thread ID
        ClockPosition expected;
        if (thread_id_to_position(i, base, &expected) != MATH_SUCCESS) {
            return false;
        }
        
        if (positions[i].ring0 != expected.ring0 ||
            positions[i].ring1 != expected.ring1 ||
            positions[i].ring2 != expected.ring2 ||
            positions[i].ring3 != expected.ring3) {
            return false;
        }
    }
    
    return true;
}