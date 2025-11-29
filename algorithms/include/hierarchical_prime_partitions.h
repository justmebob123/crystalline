#ifndef HIERARCHICAL_PRIME_PARTITIONS_H
#define HIERARCHICAL_PRIME_PARTITIONS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>
#include "prime_types.h"
#include "lattice_sphere_positions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Hierarchical Abacus System
 * 
 * Each sphere in the hierarchy has its own abacus for prime generation
 * within its assigned partition. The abacus can reference its parent's
 * abacus for read-only access, enabling efficient prime lookup without
 * duplication.
 * 
 * Key Features:
 * - Parent abacus read-only reference (no copying)
 * - Prime caching within partition boundaries
 * - Modulo 12 filtering for symmetry groups
 * - Statistics tracking (generations, hits, misses)
 * - Thread-safe operations
 */

/**
 * Hierarchical Abacus
 * 
 * Maintains prime generation state for a specific lattice partition.
 */
typedef struct HierarchicalAbacus {
    // Local abacus for this sphere
    CrystalAbacus* local_abacus;
    
    // Parent abacus (read-only reference, not owned)
    const CrystalAbacus* parent_abacus;
    
    // Partition information
    LatticePartition* partition;
    
    // Prime cache (within partition)
    BigInt* cached_primes;
    size_t cache_size;
    size_t cache_capacity;
    
    // Current state
    BigInt current_candidate;
    uint64_t primes_generated;
    
    // Statistics (atomic for thread safety)
    _Atomic(uint64_t) total_primes_generated;
    _Atomic(uint64_t) cache_hits;
    _Atomic(uint64_t) cache_misses;
    _Atomic(uint64_t) parent_lookups;
    
    // Filtering
    int symmetry_group;          // 0-11 (which mod 12 class)
    bool filter_by_symmetry;     // If true, only generate primes ≡ symmetry_group (mod 12)
    
} HierarchicalAbacus;

/**
 * Create hierarchical abacus
 * 
 * @param partition Lattice partition (defines range and symmetry group)
 * @param parent Parent abacus (can be NULL for root)
 * @return Allocated abacus, or NULL on error
 */
HierarchicalAbacus* hierarchical_abacus_create(const LatticePartition* partition,
                                                const CrystalAbacus* parent);

/**
 * Free hierarchical abacus
 * 
 * Note: Does NOT free parent abacus (not owned)
 * 
 * @param abacus Abacus to free
 */
void hierarchical_abacus_free(HierarchicalAbacus* abacus);

/**
 * Get next prime in partition
 * 
 * Generates the next prime within the partition boundaries that
 * matches the symmetry group (if filtering enabled).
 * 
 * @param abacus Abacus to use
 * @param out_prime Output: next prime (caller must initialize)
 * @return 0 on success, -1 if no more primes in partition
 */
int hierarchical_abacus_next_prime(HierarchicalAbacus* abacus, BigInt* out_prime);

/**
 * Check if number is prime
 * 
 * First checks local cache, then parent cache, then performs primality test.
 * 
 * @param abacus Abacus to use
 * @param n Number to test
 * @return true if prime, false otherwise
 */
bool hierarchical_abacus_is_prime(HierarchicalAbacus* abacus, const BigInt* n);

/**
 * Check if prime is in partition
 * 
 * Verifies that prime is within partition boundaries and matches
 * symmetry group (if filtering enabled).
 * 
 * @param abacus Abacus to use
 * @param prime Prime to check
 * @return true if in partition, false otherwise
 */
bool hierarchical_abacus_in_partition(const HierarchicalAbacus* abacus, 
                                      const BigInt* prime);

/**
 * Add prime to cache
 * 
 * Adds a prime to the local cache for faster future lookups.
 * 
 * @param abacus Abacus to use
 * @param prime Prime to cache
 * @return 0 on success, -1 on error
 */
int hierarchical_abacus_cache_prime(HierarchicalAbacus* abacus, const BigInt* prime);

/**
 * Lookup prime in cache
 * 
 * Searches local cache, then parent cache (if available).
 * 
 * @param abacus Abacus to use
 * @param prime Prime to lookup
 * @return true if found in cache, false otherwise
 */
bool hierarchical_abacus_cache_lookup(HierarchicalAbacus* abacus, const BigInt* prime);

/**
 * Get statistics
 * 
 * @param abacus Abacus to query
 * @param out_generated Output: total primes generated
 * @param out_cache_hits Output: cache hits
 * @param out_cache_misses Output: cache misses
 * @param out_parent_lookups Output: parent lookups
 */
void hierarchical_abacus_get_stats(const HierarchicalAbacus* abacus,
                                   uint64_t* out_generated,
                                   uint64_t* out_cache_hits,
                                   uint64_t* out_cache_misses,
                                   uint64_t* out_parent_lookups);

/**
 * Reset statistics
 * 
 * @param abacus Abacus to reset
 */
void hierarchical_abacus_reset_stats(HierarchicalAbacus* abacus);

/**
 * Print abacus info (for debugging)
 * 
 * @param abacus Abacus to print
 * @param name Optional name
 */
void hierarchical_abacus_print_info(const HierarchicalAbacus* abacus, 
                                    const char* name);

/**
 * Validate abacus
 * 
 * Checks for consistency and correctness.
 * 
 * @param abacus Abacus to validate
 * @return true if valid, false otherwise
 */
bool hierarchical_abacus_validate(const HierarchicalAbacus* abacus);

/**
 * Get cache efficiency
 * 
 * Returns cache hit rate as a percentage (0.0 to 100.0).
 * 
 * @param abacus Abacus to query
 * @return Cache hit rate percentage
 */
double hierarchical_abacus_cache_efficiency(const HierarchicalAbacus* abacus);

/**
 * Prefill cache with primes
 * 
 * Generates and caches primes up to a limit within the partition.
 * Useful for initialization.
 * 
 * @param abacus Abacus to use
 * @param max_primes Maximum number of primes to generate
 * @return Number of primes generated
 */
int hierarchical_abacus_prefill_cache(HierarchicalAbacus* abacus, int max_primes);

#ifdef __cplusplus
}
#endif

#endif /* HIERARCHICAL_PRIME_PARTITIONS_H */