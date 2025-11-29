// crystalline_abacus.c - Crystalline Lattice Abacus Implementation
// PURE MATHEMATICS - NO THREADING
//
// This is part of the crystalline math library and provides ONLY pure
// mathematical functions for prime generation and clock lattice mapping.
//
// IMPORTANT: NO pthread, NO threading, NO synchronization primitives.
// Threading belongs in the algorithms/CLLM layers, not here.
//
// The abacus provides:
// - Prime generation with crystalline sieve
// - Clock lattice position computation
// - Sphere coordinate computation
// - Hierarchical structure (parent/child relationships, NO threading)

#include "crystalline_abacus.h"
#include "cllm_crystalline_sieve.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// IMPORTANT PRIMES (STAGE 1) - Hard-coded for instant loading
// ============================================================================

static const uint64_t IMPORTANT_PRIMES[] = {
    // First 20 primes
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    
    // Mersenne primes (2^p - 1)
    127, 8191, 131071, 524287,
    
    // Sophie Germain primes (p and 2p+1 both prime)
    2, 3, 5, 11, 23, 29, 41, 53, 83, 89, 113, 131,
    
    // Twin primes (p and p+2 both prime)
    3, 5, 11, 17, 29, 41, 59, 71, 101, 107, 137, 149,
    
    // Fibonacci primes
    2, 3, 5, 13, 89, 233, 1597, 28657,
    
    // Primes of form 4k+1 (Gaussian primes)
    5, 13, 17, 29, 37, 41, 53, 61, 73, 89, 97, 101,
    
    // Primes of form 4k+3
    3, 7, 11, 19, 23, 31, 43, 47, 59, 67, 71, 79,
    
    // Sacred primes (12-fold symmetry)
    // Primes ≡ 1 (mod 12)
    13, 37, 61, 73, 97, 109, 157, 181, 193,
    // Primes ≡ 5 (mod 12)
    5, 17, 29, 41, 53, 89, 101, 113, 137,
    // Primes ≡ 7 (mod 12)
    7, 19, 31, 43, 67, 79, 103, 127, 139,
    // Primes ≡ 11 (mod 12)
    11, 23, 47, 59, 71, 83, 107, 131, 167,
};

static const uint32_t NUM_IMPORTANT_PRIMES = sizeof(IMPORTANT_PRIMES) / sizeof(IMPORTANT_PRIMES[0]);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Remove duplicates from important primes list
 */
static uint32_t remove_duplicates(uint64_t* primes, uint32_t count) {
    if (count <= 1) return count;
    
    // Sort first (simple bubble sort for small array)
    for (uint32_t i = 0; i < count - 1; i++) {
        for (uint32_t j = 0; j < count - i - 1; j++) {
            if (primes[j] > primes[j + 1]) {
                uint64_t temp = primes[j];
                primes[j] = primes[j + 1];
                primes[j + 1] = temp;
            }
        }
    }
    
    // Remove duplicates
    uint32_t unique_count = 1;
    for (uint32_t i = 1; i < count; i++) {
        if (primes[i] != primes[unique_count - 1]) {
            primes[unique_count++] = primes[i];
        }
    }
    
    return unique_count;
}

/**
 * Expand capacity if needed
 */
static int expand_capacity(CrystallineAbacus* abacus, uint32_t new_capacity) {
    if (new_capacity <= abacus->capacity) {
        return 0; // Already sufficient
    }
    
    // Reallocate primes array
    uint64_t* new_primes = (uint64_t*)realloc(abacus->primes, 
                                               new_capacity * sizeof(uint64_t));
    if (!new_primes) return -1;
    abacus->primes = new_primes;
    
    // Reallocate clock positions array
    BabylonianClockPosition* new_positions = (BabylonianClockPosition*)realloc(
        abacus->clock_positions, 
        new_capacity * sizeof(BabylonianClockPosition));
    if (!new_positions) return -1;
    abacus->clock_positions = new_positions;
    
    // Reallocate sphere coords array
    SphereCoord* new_coords = (SphereCoord*)realloc(
        abacus->sphere_coords, 
        new_capacity * sizeof(SphereCoord));
    if (!new_coords) return -1;
    abacus->sphere_coords = new_coords;
    
    abacus->capacity = new_capacity;
    return 0;
}

// ============================================================================
// PUBLIC API IMPLEMENTATION
// ============================================================================

CrystallineAbacus* crystalline_abacus_create(uint32_t initial_capacity) {
    CrystallineAbacus* abacus = (CrystallineAbacus*)calloc(1, sizeof(CrystallineAbacus));
    if (!abacus) return NULL;
    
    // Allocate arrays
    abacus->primes = (uint64_t*)malloc(initial_capacity * sizeof(uint64_t));
    abacus->clock_positions = (BabylonianClockPosition*)malloc(
        initial_capacity * sizeof(BabylonianClockPosition));
    abacus->sphere_coords = (SphereCoord*)malloc(
        initial_capacity * sizeof(SphereCoord));
    
    if (!abacus->primes || !abacus->clock_positions || !abacus->sphere_coords) {
        crystalline_abacus_free(abacus);
        return NULL;
    }
    
    abacus->count = 0;
    abacus->capacity = initial_capacity;
    abacus->parent = NULL;
    abacus->symmetry_group = 0;
    abacus->is_hierarchical = false;
    
    // Initialize children to NULL
    for (int i = 0; i < 12; i++) {
        abacus->children[i] = NULL;
    }
    
    // Initialize statistics
    abacus->total_generated = 0;
    abacus->cache_hits = 0;
    abacus->cache_misses = 0;
    
    return abacus;
}

void crystalline_abacus_free(CrystallineAbacus* abacus) {
    if (!abacus) return;
    
    // Free children recursively
    for (int i = 0; i < 12; i++) {
        if (abacus->children[i]) {
            crystalline_abacus_free(abacus->children[i]);
        }
    }
    
    // Free arrays
    if (abacus->primes) free(abacus->primes);
    if (abacus->clock_positions) free(abacus->clock_positions);
    if (abacus->sphere_coords) free(abacus->sphere_coords);
    
    free(abacus);
}

int crystalline_abacus_load_important_primes(CrystallineAbacus* abacus) {
    if (!abacus) return -1;
    
    // Copy important primes to temporary array
    uint64_t* temp_primes = (uint64_t*)malloc(NUM_IMPORTANT_PRIMES * sizeof(uint64_t));
    if (!temp_primes) {
        return -1;
    }
    
    memcpy(temp_primes, IMPORTANT_PRIMES, NUM_IMPORTANT_PRIMES * sizeof(uint64_t));
    
    // Remove duplicates
    uint32_t unique_count = remove_duplicates(temp_primes, NUM_IMPORTANT_PRIMES);
    
    // Ensure capacity
    if (expand_capacity(abacus, unique_count) != 0) {
        free(temp_primes);
        return -1;
    }
    
    // Add each unique prime
    for (uint32_t i = 0; i < unique_count; i++) {
        abacus->primes[abacus->count] = temp_primes[i];
        
        // Compute clock position
        abacus->clock_positions[abacus->count] = map_prime_index_to_clock(abacus->count);
        
        // Compute sphere coordinates
        abacus->sphere_coords[abacus->count] = fold_clock_to_sphere(
            abacus->clock_positions[abacus->count]);
        
        abacus->count++;
    }
    
    free(temp_primes);
    
    abacus->total_generated += unique_count;
    
    return unique_count;
}

int crystalline_abacus_generate_primes_fast(CrystallineAbacus* abacus, uint32_t target_count) {
    if (!abacus) return -1;
    if (target_count <= abacus->count) return 0; // Already have enough
    
    // Ensure capacity
    if (expand_capacity(abacus, target_count) != 0) {
        return -1;
    }
    
    // Use crystalline sieve to generate primes
    uint64_t* temp_cache = (uint64_t*)malloc(target_count * sizeof(uint64_t));
    if (!temp_cache) {
        return -1;
    }
    
    // Generate using fast crystalline sieve
    crystalline_init_prime_cache_fast(temp_cache, target_count);
    
    // Add new primes (skip ones we already have)
    uint32_t added = 0;
    for (uint32_t i = abacus->count; i < target_count; i++) {
        abacus->primes[abacus->count] = temp_cache[i];
        
        // Compute clock position
        abacus->clock_positions[abacus->count] = map_prime_index_to_clock(abacus->count);
        
        // Compute sphere coordinates
        abacus->sphere_coords[abacus->count] = fold_clock_to_sphere(
            abacus->clock_positions[abacus->count]);
        
        abacus->count++;
        added++;
    }
    
    free(temp_cache);
    
    abacus->total_generated += added;
    
    return added;
}

int crystalline_abacus_add_prime(CrystallineAbacus* abacus, uint64_t prime) {
    if (!abacus) return -1;
    
    // Ensure capacity
    if (abacus->count >= abacus->capacity) {
        if (expand_capacity(abacus, abacus->capacity * 2) != 0) {
            return -1;
        }
    }
    
    // Add prime
    abacus->primes[abacus->count] = prime;
    
    // Compute clock position
    abacus->clock_positions[abacus->count] = map_prime_index_to_clock(abacus->count);
    
    // Compute sphere coordinates
    abacus->sphere_coords[abacus->count] = fold_clock_to_sphere(
        abacus->clock_positions[abacus->count]);
    
    abacus->count++;
    abacus->total_generated++;
    
    return 0;
}

uint64_t crystalline_abacus_get_prime(const CrystallineAbacus* abacus, uint32_t index) {
    if (!abacus || index >= abacus->count) return 0;
    return abacus->primes[index];
}

BabylonianClockPosition crystalline_abacus_get_clock_position(
    const CrystallineAbacus* abacus, uint32_t index) {
    
    BabylonianClockPosition invalid = {-1, -1, 0.0, 0.0};
    if (!abacus || index >= abacus->count) return invalid;
    
    return abacus->clock_positions[index];
}

SphereCoord crystalline_abacus_get_sphere_coord(
    const CrystallineAbacus* abacus, uint32_t index) {
    
    SphereCoord invalid = {0.0, 0.0, 0.0};
    if (!abacus || index >= abacus->count) return invalid;
    
    return abacus->sphere_coords[index];
}

uint32_t crystalline_abacus_get_count(const CrystallineAbacus* abacus) {
    if (!abacus) return 0;
    return abacus->count;
}

bool crystalline_abacus_matches_symmetry_group(uint64_t prime, uint32_t symmetry_group) {
    if (prime == 2) return (symmetry_group == 2);
    if (prime == 3) return (symmetry_group == 3);
    
    // Primes > 3 can only be in {1, 5, 7, 11} (mod 12)
    uint32_t mod12 = prime % 12;
    
    // Check if this prime belongs to the symmetry group
    if (symmetry_group == 1 && mod12 == 1) return true;
    if (symmetry_group == 5 && mod12 == 5) return true;
    if (symmetry_group == 7 && mod12 == 7) return true;
    if (symmetry_group == 11 && mod12 == 11) return true;
    
    return false;
}

CrystallineAbacus* crystalline_abacus_create_hierarchical(
    CrystallineAbacus* parent, uint32_t symmetry_group) {
    
    if (!parent || symmetry_group >= 12) return NULL;
    
    // Create child abacus with smaller initial capacity
    CrystallineAbacus* child = crystalline_abacus_create(1000);
    if (!child) return NULL;
    
    child->parent = parent;
    child->symmetry_group = symmetry_group;
    child->is_hierarchical = true;
    
    // Filter parent's primes by symmetry group (NO LOCKING - pure math)
    for (uint32_t i = 0; i < parent->count; i++) {
        if (crystalline_abacus_matches_symmetry_group(parent->primes[i], symmetry_group)) {
            crystalline_abacus_add_prime(child, parent->primes[i]);
        }
    }
    
    // Register child with parent
    parent->children[symmetry_group] = child;
    
    return child;
}

int crystalline_abacus_compute_clock_positions(CrystallineAbacus* abacus) {
    if (!abacus) return -1;
    
    for (uint32_t i = 0; i < abacus->count; i++) {
        abacus->clock_positions[i] = map_prime_index_to_clock(i);
        abacus->sphere_coords[i] = fold_clock_to_sphere(abacus->clock_positions[i]);
    }
    
    return 0;
}

void crystalline_abacus_get_stats(const CrystallineAbacus* abacus,
                                  uint64_t* out_generated,
                                  uint64_t* out_cache_hits,
                                  uint64_t* out_cache_misses) {
    if (!abacus) return;
    
    if (out_generated) *out_generated = abacus->total_generated;
    if (out_cache_hits) *out_cache_hits = abacus->cache_hits;
    if (out_cache_misses) *out_cache_misses = abacus->cache_misses;
}

void crystalline_abacus_print_info(const CrystallineAbacus* abacus, const char* name) {
    if (!abacus) return;
    
    printf("=== Crystalline Abacus: %s ===\n", name ? name : "Unnamed");
    printf("Count: %u / %u\n", abacus->count, abacus->capacity);
    printf("Hierarchical: %s\n", abacus->is_hierarchical ? "Yes" : "No");
    if (abacus->is_hierarchical) {
        printf("Symmetry Group: %u\n", abacus->symmetry_group);
        printf("Parent: %p\n", (void*)abacus->parent);
    }
    printf("Total Generated: %lu\n", abacus->total_generated);
    printf("Cache Hits: %lu\n", abacus->cache_hits);
    printf("Cache Misses: %lu\n", abacus->cache_misses);
    
    if (abacus->count > 0) {
        printf("First prime: %lu\n", abacus->primes[0]);
        printf("Last prime: %lu\n", abacus->primes[abacus->count - 1]);
    }
    
    printf("Children: ");
    int child_count = 0;
    for (int i = 0; i < 12; i++) {
        if (abacus->children[i]) child_count++;
    }
    printf("%d\n", child_count);
    printf("===========================\n");
}

bool crystalline_abacus_validate(const CrystallineAbacus* abacus) {
    if (!abacus) return false;
    if (!abacus->primes || !abacus->clock_positions || !abacus->sphere_coords) return false;
    if (abacus->count > abacus->capacity) return false;
    
    // Check primes are in ascending order
    for (uint32_t i = 1; i < abacus->count; i++) {
        if (abacus->primes[i] <= abacus->primes[i-1]) return false;
    }
    
    // Check hierarchical structure
    if (abacus->is_hierarchical) {
        if (!abacus->parent) return false;
        if (abacus->symmetry_group >= 12) return false;
    }
    
    return true;
}