/**
 * Sphere Position System - Implementation
 * 
 * Manages sphere positions in the crystalline lattice with special
 * handling for the 144000 boundary region.
 */

#include "lattice_sphere_positions.h"
#include "bigint_core.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "prime_math_custom.h"
#include <stdatomic.h>
#include <limits.h>

// Global sphere ID counter (thread-safe)
static _Atomic(uint64_t) next_sphere_id = 1;

/**
 * Helper: Convert BigInt to uint64_t (approximation for large numbers)
 */
static uint64_t bigint_to_uint64_approx(const BigInt* n) {
    if (!n || n->len == 0) {
        return 0;
    }
    
    // For small numbers, return exact value
    if (n->len == 1) {
        return n->d[0];
    }
    
    // For larger numbers, return approximation from first two digits
    if (n->len >= 2) {
        return ((uint64_t)n->d[1] << 32) | n->d[0];
    }
    
    return n->d[0];
}

/**
 * Helper: Convert BigInt to int (for small values)
 */
static int bigint_to_int_approx(const BigInt* n) {
    if (!n || n->len == 0) {
        return 0;
    }
    
    if (n->d[0] > INT_MAX) {
        return INT_MAX;
    }
    
    return (int)n->d[0];
}

/**
 * Helper: Print BigInt (simple version)
 */
static void bigint_print_simple(const BigInt* n) {
    if (!n) {
        printf("NULL");
        return;
    }
    
    char* str = big_to_string(n);
    if (str) {
        printf("%s", str);
        free(str);
    } else {
        printf("(error)");
    }
}

/**
 * Get next sphere ID
 */
uint64_t get_next_sphere_id(void) {
    return atomic_fetch_add(&next_sphere_id, 1);
}

/**
 * Calculate clock position from symmetry group
 */
ClockPosition calculate_clock_position(int symmetry_group) {
    ClockPosition pos;
    memset(&pos, 0, sizeof(ClockPosition));
    
    if (symmetry_group < 0 || symmetry_group >= 12) {
        return pos;
    }
    
    // Map symmetry group to clock position
    pos.position = symmetry_group;
    pos.clock_pos = (uint8_t)symmetry_group;
    
    // Calculate angle in radians (0 = 12 o'clock, clockwise)
    pos.angle_radians = (symmetry_group * 2.0 * PRIME_PI) / 12.0;
    pos.theta = pos.angle_radians;
    
    // Calculate degree (0-360)
    pos.degree = (symmetry_group * 30) % 360; // 360/12 = 30 degrees per hour
    
    // Calculate quadrant (1-4)
    if (symmetry_group >= 0 && symmetry_group < 3) {
        pos.quadrant = 1;
    } else if (symmetry_group >= 3 && symmetry_group < 6) {
        pos.quadrant = 2;
    } else if (symmetry_group >= 6 && symmetry_group < 9) {
        pos.quadrant = 3;
    } else {
        pos.quadrant = 4;
    }
    
    // Check if on 3 o'clock boundary
    pos.on_boundary = (symmetry_group == 3);
    
    // Radial distance (all positions on unit circle)
    pos.r = 1.0;
    
    return pos;
}

/**
 * Create boundary region
 */
BoundaryRegion* create_boundary_region(void) {
    BoundaryRegion* boundary = (BoundaryRegion*)malloc(sizeof(BoundaryRegion));
    if (!boundary) {
        return NULL;
    }
    
    // Initialize BigInts
    big_init(&boundary->center);
    big_init(&boundary->lower_twin);
    big_init(&boundary->upper_twin);
    
    // Set values
    big_from_int(&boundary->center, VECTOR_CULMINATION);
    big_from_int(&boundary->lower_twin, TWIN_PRIME_LOWER);
    big_from_int(&boundary->upper_twin, TWIN_PRIME_UPPER);
    
    // Einstein's Λ correction
    boundary->omega_correction = (double)EINSTEIN_LAMBDA_NUMERATOR / 
                                 (double)EINSTEIN_LAMBDA_DENOMINATOR;
    
    boundary->requires_special_handling = true;
    boundary->is_boundary_region = true;
    
    return boundary;
}

/**
 * Free boundary region
 */
void free_boundary_region(BoundaryRegion* boundary) {
    if (!boundary) {
        return;
    }
    
    big_free(&boundary->center);
    big_free(&boundary->lower_twin);
    big_free(&boundary->upper_twin);
    
    free(boundary);
}

/**
 * Check if range intersects 144000 boundary
 */
bool check_144000_boundary(const BigInt* range_start, const BigInt* range_end) {
    if (!range_start || !range_end) {
        return false;
    }
    
    BigInt lower_twin, upper_twin;
    big_init(&lower_twin);
    big_init(&upper_twin);
    
    big_from_int(&lower_twin, TWIN_PRIME_LOWER);
    big_from_int(&upper_twin, TWIN_PRIME_UPPER);
    
    // Check if range includes 143999 or 144001
    bool includes_lower = (big_cmp(range_start, &lower_twin) <= 0 &&
                          big_cmp(range_end, &lower_twin) >= 0);
    
    bool includes_upper = (big_cmp(range_start, &upper_twin) <= 0 &&
                          big_cmp(range_end, &upper_twin) >= 0);
    
    big_free(&lower_twin);
    big_free(&upper_twin);
    
    return includes_lower || includes_upper;
}

/**
 * Estimate prime count using prime number theorem
 */
uint64_t estimate_partition_prime_count(const LatticePartition* partition) {
    if (!partition) {
        return 0;
    }
    
    // Convert BigInt to uint64_t for estimation
    uint64_t start = bigint_to_uint64_approx(&partition->range_start);
    uint64_t end = bigint_to_uint64_approx(&partition->range_end);
    
    if (start >= end || end <= 2) {
        return 0;
    }
    
    // Prime number theorem: π(x) ≈ x / ln(x)
    double pi_end = (double)end / prime_log((double)end);
    double pi_start = (start > 2) ? (double)start / prime_log((double)start) : 0;
    
    double total_primes = pi_end - pi_start;
    
    // Distribute among 12 symmetry groups (rough approximation)
    double primes_in_group = total_primes / 12.0;
    
    return (uint64_t)primes_in_group;
}

/**
 * Create lattice partition
 */
LatticePartition* create_lattice_partition(int symmetry_group,
                                           const BigInt* range_start,
                                           const BigInt* range_end) {
    if (symmetry_group < 0 || symmetry_group >= 12) {
        fprintf(stderr, "Error: Invalid symmetry group %d (must be 0-11)\n", symmetry_group);
        return NULL;
    }
    
    if (!range_start || !range_end) {
        fprintf(stderr, "Error: NULL range boundaries\n");
        return NULL;
    }
    
    if (big_cmp(range_start, range_end) >= 0) {
        fprintf(stderr, "Error: Invalid range (start >= end)\n");
        return NULL;
    }
    
    LatticePartition* partition = (LatticePartition*)malloc(sizeof(LatticePartition));
    if (!partition) {
        return NULL;
    }
    
    partition->symmetry_group = symmetry_group;
    partition->modulo_class = symmetry_group;
    
    // Copy range boundaries
    big_init(&partition->range_start);
    big_init(&partition->range_end);
    big_copy(&partition->range_start, range_start);
    big_copy(&partition->range_end, range_end);
    
    // Check for 144000 boundary
    partition->contains_twin_boundary = check_144000_boundary(range_start, range_end);
    
    // Create boundary region if needed
    if (partition->contains_twin_boundary) {
        partition->boundary = create_boundary_region();
    } else {
        partition->boundary = NULL;
    }
    
    // Estimate prime count
    partition->expected_prime_count = estimate_partition_prime_count(partition);
    
    // Calculate density
    BigInt range_size;
    big_init(&range_size);
    big_sub(range_end, range_start, &range_size);
    uint64_t size = bigint_to_uint64_approx(&range_size);
    big_free(&range_size);
    
    if (size > 0) {
        partition->prime_density = (double)partition->expected_prime_count / (double)size;
    } else {
        partition->prime_density = 0.0;
    }
    
    return partition;
}

/**
 * Free lattice partition
 */
void free_lattice_partition(LatticePartition* partition) {
    if (!partition) {
        return;
    }
    
    big_free(&partition->range_start);
    big_free(&partition->range_end);
    
    if (partition->boundary) {
        free_boundary_region(partition->boundary);
    }
    
    free(partition);
}

/**
 * Check if prime is in symmetry group
 */
bool is_prime_in_symmetry_group(const BigInt* prime, int symmetry_group) {
    if (!prime || symmetry_group < 0 || symmetry_group >= 12) {
        return false;
    }
    
    // Calculate prime mod 12
    BigInt twelve, remainder;
    big_init(&twelve);
    big_init(&remainder);
    
    big_from_int(&twelve, 12);
    big_mod(prime, &twelve, &remainder);
    
    int mod_value = bigint_to_int_approx(&remainder);
    
    big_free(&twelve);
    big_free(&remainder);
    
    return mod_value == symmetry_group;
}

/**
 * Create sphere position
 */
SpherePosition* sphere_position_create(int symmetry_group, 
                                       const BigInt* center,
                                       const BigInt* radius,
                                       int depth) {
    if (symmetry_group < 0 || symmetry_group >= 12) {
        fprintf(stderr, "Error: Invalid symmetry group %d\n", symmetry_group);
        return NULL;
    }
    
    SpherePosition* pos = (SpherePosition*)malloc(sizeof(SpherePosition));
    if (!pos) {
        return NULL;
    }
    
    pos->symmetry_group = symmetry_group;
    pos->depth = depth;
    pos->sphere_id = get_next_sphere_id();
    
    // Initialize BigInts
    big_init(&pos->prime_center);
    big_init(&pos->prime_radius);
    
    // Set center and radius
    if (center) {
        big_copy(&pos->prime_center, center);
    } else {
        // Default: center at 1000 * (symmetry_group + 1)
        big_from_int(&pos->prime_center, 1000 * (symmetry_group + 1));
    }
    
    if (radius) {
        big_copy(&pos->prime_radius, radius);
    } else {
        // Default: radius of 1000
        big_from_int(&pos->prime_radius, 1000);
    }
    
    // Calculate clock position
    pos->clock_pos = calculate_clock_position(symmetry_group);
    
    // Create partition
    BigInt range_start, range_end;
    big_init(&range_start);
    big_init(&range_end);
    
    // Range: [center - radius, center + radius]
    big_sub(&pos->prime_center, &pos->prime_radius, &range_start);
    big_add(&pos->prime_center, &pos->prime_radius, &range_end);
    
    // Ensure range_start >= 2 (smallest prime)
    BigInt two;
    big_init(&two);
    big_from_int(&two, 2);
    if (big_cmp(&range_start, &two) < 0) {
        big_copy(&range_start, &two);
    }
    big_free(&two);
    
    pos->partition = create_lattice_partition(symmetry_group, &range_start, &range_end);
    
    big_free(&range_start);
    big_free(&range_end);
    
    if (!pos->partition) {
        big_free(&pos->prime_center);
        big_free(&pos->prime_radius);
        free(pos);
        return NULL;
    }
    
    return pos;
}

/**
 * Free sphere position
 */
void sphere_position_free(SpherePosition* pos) {
    if (!pos) {
        return;
    }
    
    big_free(&pos->prime_center);
    big_free(&pos->prime_radius);
    
    if (pos->partition) {
        free_lattice_partition(pos->partition);
    }
    
    free(pos);
}

/**
 * Check if prime is in sphere's partition
 */
bool sphere_position_contains_prime(const SpherePosition* pos, const BigInt* prime) {
    if (!pos || !pos->partition || !prime) {
        return false;
    }
    
    // Check if prime is in range
    if (big_cmp(prime, &pos->partition->range_start) < 0 ||
        big_cmp(prime, &pos->partition->range_end) > 0) {
        return false;
    }
    
    // Check if prime is in symmetry group
    return is_prime_in_symmetry_group(prime, pos->symmetry_group);
}

/**
 * Get Einstein's Λ correction
 */
double get_einstein_lambda_correction(const SpherePosition* pos) {
    if (!pos || !pos->partition) {
        return 0.0;
    }
    
    if (pos->partition->contains_twin_boundary && pos->partition->boundary) {
        return pos->partition->boundary->omega_correction;
    }
    
    return 0.0;
}

/**
 * Validate sphere position
 */
bool validate_sphere_position(const SpherePosition* pos) {
    if (!pos) {
        fprintf(stderr, "Validation failed: NULL position\n");
        return false;
    }
    
    if (pos->symmetry_group < 0 || pos->symmetry_group >= 12) {
        fprintf(stderr, "Validation failed: Invalid symmetry group %d\n", pos->symmetry_group);
        return false;
    }
    
    if (!pos->partition) {
        fprintf(stderr, "Validation failed: NULL partition\n");
        return false;
    }
    
    if (pos->partition->symmetry_group != pos->symmetry_group) {
        fprintf(stderr, "Validation failed: Partition symmetry group mismatch\n");
        return false;
    }
    
    if (pos->depth < 0) {
        fprintf(stderr, "Validation failed: Negative depth\n");
        return false;
    }
    
    if (pos->sphere_id == 0) {
        fprintf(stderr, "Validation failed: Invalid sphere ID\n");
        return false;
    }
    
    return true;
}

/**
 * Print sphere position
 */
void print_sphere_position(const SpherePosition* pos) {
    if (!pos) {
        return;
    }
    
    printf("=== Sphere Position ===\n");
    printf("Sphere ID: %lu\n", pos->sphere_id);
    printf("Depth: %d\n", pos->depth);
    printf("Symmetry group: %d\n", pos->symmetry_group);
    printf("Clock position: %d o'clock\n", pos->clock_pos.position);
    
    printf("Prime center: ");
    bigint_print_simple(&pos->prime_center);
    printf("\n");
    
    printf("Prime radius: ");
    bigint_print_simple(&pos->prime_radius);
    printf("\n");
    
    if (pos->partition) {
        printf("\nPartition:\n");
        printf("  Range: [");
        bigint_print_simple(&pos->partition->range_start);
        printf(", ");
        bigint_print_simple(&pos->partition->range_end);
        printf("]\n");
        
        printf("  Expected primes: %lu\n", pos->partition->expected_prime_count);
        printf("  Prime density: %.6f\n", pos->partition->prime_density);
        printf("  Modulo class: %d\n", pos->partition->modulo_class);
        
        if (pos->partition->contains_twin_boundary) {
            printf("  *** CONTAINS 144000 BOUNDARY ***\n");
            if (pos->partition->boundary) {
                printf("  Einstein's Λ correction: %.10f\n", 
                       pos->partition->boundary->omega_correction);
            }
        }
    }
    
    printf("\n");
}