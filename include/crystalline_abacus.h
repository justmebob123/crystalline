#ifndef CRYSTALLINE_ABACUS_H
#define CRYSTALLINE_ABACUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include "clock_lattice.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Crystalline Lattice Abacus - The Core Mathematical Foundation
 * 
 * This is the SINGLE SOURCE OF TRUTH for all prime numbers in the system.
 * All prime generation, access, and operations go through this structure.
 * 
 * Key Features:
 * - Integrated with Babylonian Clock Lattice mapping
 * - Stores sphere coordinates for each prime
 * - Hierarchical structure for threading (12-fold symmetry)
 * - Fast crystalline sieve generation
 * - Thread-safe operations
 */

/**
 * Crystalline Abacus Structure
 * 
 * The core structure that holds all primes and their geometric mappings.
 */
typedef struct CrystallineAbacus {
    // Prime storage
    uint64_t* primes;                          // Array of prime numbers
    uint32_t count;                            // Number of primes currently stored
    uint32_t capacity;                         // Allocated capacity
    
    // Clock lattice integration (REQUIRED)
    BabylonianClockPosition* clock_positions;  // One per prime
    SphereCoord* sphere_coords;                // One per prime
    
    // Hierarchical structure (for threading)
    struct CrystallineAbacus* parent;          // Parent abacus (NULL for global)
    struct CrystallineAbacus* children[12];    // Child abacuses (one per symmetry group)
    uint32_t symmetry_group;                   // 0-11 (partition for this abacus)
    bool is_hierarchical;                      // true if this is a child abacus
    
    // Thread safety
    pthread_mutex_t mutex;                     // Mutex for thread-safe operations
    
    // Statistics
    uint64_t total_generated;                  // Total primes generated
    uint64_t cache_hits;                       // Number of cache hits
    uint64_t cache_misses;                     // Number of cache misses
    
} CrystallineAbacus;

/**
 * Create global abacus
 * 
 * Creates the root abacus that serves as the single source of truth
 * for all primes in the system.
 * 
 * @param initial_capacity Initial capacity (recommend 10000)
 * @return Allocated abacus, or NULL on error
 */
CrystallineAbacus* crystalline_abacus_create(uint32_t initial_capacity);

/**
 * Free abacus
 * 
 * Frees the abacus and all associated memory.
 * Does NOT free parent (not owned).
 * DOES free all children recursively.
 * 
 * @param abacus Abacus to free
 */
void crystalline_abacus_free(CrystallineAbacus* abacus);

/**
 * Load important primes (Stage 1)
 * 
 * Loads hard-coded important primes into the abacus.
 * These include: first 20 primes, Mersenne primes, Sophie Germain primes,
 * twin primes, Fibonacci primes, and sacred primes.
 * 
 * This is INSTANT (no generation needed).
 * 
 * @param abacus Abacus to load into
 * @return Number of primes loaded, or -1 on error
 */
int crystalline_abacus_load_important_primes(CrystallineAbacus* abacus);

/**
 * Generate primes using fast crystalline sieve (Stage 2)
 * 
 * Generates primes up to the specified count using the crystalline sieve
 * with 12-fold symmetry filtering.
 * 
 * This is FAST (~10ms for 10,000 primes) and NON-BLOCKING.
 * 
 * @param abacus Abacus to generate into
 * @param target_count Target number of primes
 * @return Number of primes generated, or -1 on error
 */
int crystalline_abacus_generate_primes_fast(CrystallineAbacus* abacus, uint32_t target_count);

/**
 * Add a single prime to the abacus
 * 
 * Adds a prime and computes its clock position and sphere coordinates.
 * This is the ONLY way primes should be added to the abacus.
 * 
 * @param abacus Abacus to add to
 * @param prime Prime number to add
 * @return 0 on success, -1 on error
 */
int crystalline_abacus_add_prime(CrystallineAbacus* abacus, uint64_t prime);

/**
 * Get prime by index
 * 
 * Returns the nth prime (0-indexed).
 * Thread-safe.
 * 
 * @param abacus Abacus to query
 * @param index Index of prime (0-based)
 * @return Prime number, or 0 if index out of bounds
 */
uint64_t crystalline_abacus_get_prime(const CrystallineAbacus* abacus, uint32_t index);

/**
 * Get clock position for prime
 * 
 * Returns the Babylonian clock position for the nth prime.
 * Thread-safe.
 * 
 * @param abacus Abacus to query
 * @param index Index of prime (0-based)
 * @return Clock position, or invalid position if index out of bounds
 */
BabylonianClockPosition crystalline_abacus_get_clock_position(
    const CrystallineAbacus* abacus, uint32_t index);

/**
 * Get sphere coordinates for prime
 * 
 * Returns the 3D sphere coordinates for the nth prime.
 * Thread-safe.
 * 
 * @param abacus Abacus to query
 * @param index Index of prime (0-based)
 * @return Sphere coordinates, or invalid coordinates if index out of bounds
 */
SphereCoord crystalline_abacus_get_sphere_coord(
    const CrystallineAbacus* abacus, uint32_t index);

/**
 * Get number of primes in abacus
 * 
 * Thread-safe.
 * 
 * @param abacus Abacus to query
 * @return Number of primes
 */
uint32_t crystalline_abacus_get_count(const CrystallineAbacus* abacus);

/**
 * Create hierarchical abacus (for threading)
 * 
 * Creates a child abacus that filters primes by symmetry group.
 * The child abacus references the parent for read-only access.
 * 
 * @param parent Parent abacus (must not be NULL)
 * @param symmetry_group Symmetry group (0-11)
 * @return Allocated child abacus, or NULL on error
 */
CrystallineAbacus* crystalline_abacus_create_hierarchical(
    CrystallineAbacus* parent, uint32_t symmetry_group);

/**
 * Check if prime matches symmetry group
 * 
 * Checks if a prime belongs to the specified symmetry group.
 * Primes > 3 can only be in groups {1, 5, 7, 11} (mod 12).
 * 
 * @param prime Prime to check
 * @param symmetry_group Symmetry group (0-11)
 * @return true if prime matches group, false otherwise
 */
bool crystalline_abacus_matches_symmetry_group(uint64_t prime, uint32_t symmetry_group);

/**
 * Compute clock positions for all primes
 * 
 * Computes and stores clock positions and sphere coordinates for all
 * primes currently in the abacus.
 * 
 * This should be called after loading/generating primes.
 * 
 * @param abacus Abacus to process
 * @return 0 on success, -1 on error
 */
int crystalline_abacus_compute_clock_positions(CrystallineAbacus* abacus);

/**
 * Get statistics
 * 
 * @param abacus Abacus to query
 * @param out_generated Output: total primes generated
 * @param out_cache_hits Output: cache hits
 * @param out_cache_misses Output: cache misses
 */
void crystalline_abacus_get_stats(const CrystallineAbacus* abacus,
                                  uint64_t* out_generated,
                                  uint64_t* out_cache_hits,
                                  uint64_t* out_cache_misses);

/**
 * Print abacus info (for debugging)
 * 
 * @param abacus Abacus to print
 * @param name Optional name
 */
void crystalline_abacus_print_info(const CrystallineAbacus* abacus, const char* name);

/**
 * Validate abacus
 * 
 * Checks for consistency and correctness.
 * 
 * @param abacus Abacus to validate
 * @return true if valid, false otherwise
 */
bool crystalline_abacus_validate(const CrystallineAbacus* abacus);

#ifdef __cplusplus
}
#endif

#endif /* CRYSTALLINE_ABACUS_H */