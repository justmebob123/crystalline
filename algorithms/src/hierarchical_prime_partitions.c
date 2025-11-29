/**
 * Hierarchical Abacus System - Implementation
 * 
 * Prime generation with parent reference and partition filtering.
 */

#include "../include/hierarchical_prime_partitions.h"
#include "bigint_core.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Create hierarchical abacus
 */
HierarchicalAbacus* hierarchical_abacus_create(const LatticePartition* partition,
                                                const CrystalAbacus* parent) {
    if (!partition) {
        fprintf(stderr, "Error: NULL partition\n");
        return NULL;
    }
    
    HierarchicalAbacus* abacus = (HierarchicalAbacus*)calloc(1, sizeof(HierarchicalAbacus));
    if (!abacus) {
        fprintf(stderr, "Error: Failed to allocate HierarchicalAbacus\n");
        return NULL;
    }
    
    // Create local abacus
    abacus->local_abacus = (CrystalAbacus*)calloc(1, sizeof(CrystalAbacus));
    if (!abacus->local_abacus) {
        fprintf(stderr, "Error: Failed to allocate local abacus\n");
        free(abacus);
        return NULL;
    }
    
    // Initialize local abacus
    abacus->local_abacus->capacity = 1000;
    abacus->local_abacus->primes = (int*)malloc(1000 * sizeof(int));
    if (!abacus->local_abacus->primes) {
        free(abacus->local_abacus);
        free(abacus);
        return NULL;
    }
    abacus->local_abacus->num_primes = 0;
    abacus->local_abacus->candidate = 2;
    
    // Set parent reference (read-only, not owned)
    abacus->parent_abacus = parent;
    
    // Copy partition (we need our own copy)
    abacus->partition = create_lattice_partition(
        partition->symmetry_group,
        &partition->range_start,
        &partition->range_end
    );
    
    if (!abacus->partition) {
        free(abacus->local_abacus->primes);
        free(abacus->local_abacus);
        free(abacus);
        return NULL;
    }
    
    // Initialize cache
    abacus->cache_capacity = 100;
    abacus->cached_primes = (BigInt*)malloc(abacus->cache_capacity * sizeof(BigInt));
    if (!abacus->cached_primes) {
        free_lattice_partition(abacus->partition);
        free(abacus->local_abacus->primes);
        free(abacus->local_abacus);
        free(abacus);
        return NULL;
    }
    
    for (size_t i = 0; i < abacus->cache_capacity; i++) {
        big_init(&abacus->cached_primes[i]);
    }
    
    abacus->cache_size = 0;
    
    // Initialize current candidate to range start
    big_init(&abacus->current_candidate);
    big_copy(&abacus->current_candidate, &partition->range_start);
    
    // Set symmetry group and filtering
    abacus->symmetry_group = partition->symmetry_group;
    abacus->filter_by_symmetry = true;
    
    // Initialize statistics
    atomic_init(&abacus->total_primes_generated, 0);
    atomic_init(&abacus->cache_hits, 0);
    atomic_init(&abacus->cache_misses, 0);
    atomic_init(&abacus->parent_lookups, 0);
    
    abacus->primes_generated = 0;
    
    return abacus;
}

/**
 * Free hierarchical abacus
 */
void hierarchical_abacus_free(HierarchicalAbacus* abacus) {
    if (!abacus) {
        return;
    }
    
    // Free local abacus
    if (abacus->local_abacus) {
        if (abacus->local_abacus->primes) {
            free(abacus->local_abacus->primes);
        }
        free(abacus->local_abacus);
    }
    
    // Free partition
    if (abacus->partition) {
        free_lattice_partition(abacus->partition);
    }
    
    // Free cache
    if (abacus->cached_primes) {
        for (size_t i = 0; i < abacus->cache_capacity; i++) {
            big_free(&abacus->cached_primes[i]);
        }
        free(abacus->cached_primes);
    }
    
    // Free current candidate
    big_free(&abacus->current_candidate);
    
    // Note: Do NOT free parent_abacus (not owned)
    
    free(abacus);
}

/**
 * Check if prime matches symmetry group
 */
static bool matches_symmetry_group(const BigInt* prime, int symmetry_group) {
    BigInt twelve, remainder;
    big_init(&twelve);
    big_init(&remainder);
    
    big_from_int(&twelve, 12);
    big_mod(prime, &twelve, &remainder);
    
    // Convert remainder to int
    int mod_value = (remainder.len > 0) ? (int)remainder.d[0] : 0;
    
    big_free(&twelve);
    big_free(&remainder);
    
    return mod_value == symmetry_group;
}

/**
 * Check if prime is in partition
 */
bool hierarchical_abacus_in_partition(const HierarchicalAbacus* abacus, 
                                      const BigInt* prime) {
    if (!abacus || !abacus->partition || !prime) {
        return false;
    }
    
    // Check range
    if (big_cmp(prime, &abacus->partition->range_start) < 0 ||
        big_cmp(prime, &abacus->partition->range_end) > 0) {
        return false;
    }
    
    // Check symmetry group if filtering enabled
    if (abacus->filter_by_symmetry) {
        return matches_symmetry_group(prime, abacus->symmetry_group);
    }
    
    return true;
}

/**
 * Lookup prime in cache
 */
bool hierarchical_abacus_cache_lookup(HierarchicalAbacus* abacus, const BigInt* prime) {
    if (!abacus || !prime) {
        return false;
    }
    
    // Search local cache
    for (size_t i = 0; i < abacus->cache_size; i++) {
        if (big_cmp(&abacus->cached_primes[i], prime) == 0) {
            atomic_fetch_add(&abacus->cache_hits, 1);
            return true;
        }
    }
    
    // Search parent cache if available
    if (abacus->parent_abacus) {
        atomic_fetch_add(&abacus->parent_lookups, 1);
        
        // Check parent's primes array
        for (size_t i = 0; i < abacus->parent_abacus->num_primes; i++) {
            BigInt parent_prime;
            big_init(&parent_prime);
            big_from_int(&parent_prime, abacus->parent_abacus->primes[i]);
            
            int cmp = big_cmp(&parent_prime, prime);
            big_free(&parent_prime);
            
            if (cmp == 0) {
                atomic_fetch_add(&abacus->cache_hits, 1);
                return true;
            }
        }
    }
    
    atomic_fetch_add(&abacus->cache_misses, 1);
    return false;
}

/**
 * Add prime to cache
 */
int hierarchical_abacus_cache_prime(HierarchicalAbacus* abacus, const BigInt* prime) {
    if (!abacus || !prime) {
        return -1;
    }
    
    // Check if already in cache
    if (hierarchical_abacus_cache_lookup(abacus, prime)) {
        return 0;  // Already cached
    }
    
    // Expand cache if needed
    if (abacus->cache_size >= abacus->cache_capacity) {
        size_t new_capacity = abacus->cache_capacity * 2;
        BigInt* new_cache = (BigInt*)realloc(abacus->cached_primes, 
                                             new_capacity * sizeof(BigInt));
        if (!new_cache) {
            return -1;
        }
        
        // Initialize new entries
        for (size_t i = abacus->cache_capacity; i < new_capacity; i++) {
            big_init(&new_cache[i]);
        }
        
        abacus->cached_primes = new_cache;
        abacus->cache_capacity = new_capacity;
    }
    
    // Add to cache
    big_copy(&abacus->cached_primes[abacus->cache_size], prime);
    abacus->cache_size++;
    
    return 0;
}

/**
 * Check if number is prime
 */
bool hierarchical_abacus_is_prime(HierarchicalAbacus* abacus, const BigInt* n) {
    if (!abacus || !n) {
        return false;
    }
    
    // Check cache first
    if (hierarchical_abacus_cache_lookup(abacus, n)) {
        return true;
    }
    
    // Perform primality test
    int result = big_is_prime(n, 10);  // Miller-Rabin with 10 iterations
    
    // Cache if prime
    if (result) {
        hierarchical_abacus_cache_prime(abacus, n);
    }
    
    return result != 0;
}

/**
 * Get next prime in partition
 */
int hierarchical_abacus_next_prime(HierarchicalAbacus* abacus, BigInt* out_prime) {
    if (!abacus || !out_prime) {
        return -1;
    }
    
    BigInt one;
    big_init(&one);
    big_from_int(&one, 1);
    
    // Start from current candidate
    BigInt candidate;
    big_init(&candidate);
    big_copy(&candidate, &abacus->current_candidate);
    
    // Search for next prime in partition
    while (big_cmp(&candidate, &abacus->partition->range_end) <= 0) {
        // Check if in partition and matches symmetry group
        if (hierarchical_abacus_in_partition(abacus, &candidate)) {
            // Check if prime
            if (hierarchical_abacus_is_prime(abacus, &candidate)) {
                // Found prime
                big_copy(out_prime, &candidate);
                
                // Update current candidate for next call
                big_add(&candidate, &one, &abacus->current_candidate);
                
                // Update statistics
                abacus->primes_generated++;
                atomic_fetch_add(&abacus->total_primes_generated, 1);
                
                big_free(&candidate);
                big_free(&one);
                return 0;
            }
        }
        
        // Move to next candidate
        big_add(&candidate, &one, &candidate);
    }
    
    // No more primes in partition
    big_free(&candidate);
    big_free(&one);
    return -1;
}

/**
 * Get statistics
 */
void hierarchical_abacus_get_stats(const HierarchicalAbacus* abacus,
                                   uint64_t* out_generated,
                                   uint64_t* out_cache_hits,
                                   uint64_t* out_cache_misses,
                                   uint64_t* out_parent_lookups) {
    if (!abacus) {
        return;
    }
    
    if (out_generated) {
        *out_generated = atomic_load(&abacus->total_primes_generated);
    }
    
    if (out_cache_hits) {
        *out_cache_hits = atomic_load(&abacus->cache_hits);
    }
    
    if (out_cache_misses) {
        *out_cache_misses = atomic_load(&abacus->cache_misses);
    }
    
    if (out_parent_lookups) {
        *out_parent_lookups = atomic_load(&abacus->parent_lookups);
    }
}

/**
 * Reset statistics
 */
void hierarchical_abacus_reset_stats(HierarchicalAbacus* abacus) {
    if (!abacus) {
        return;
    }
    
    atomic_store(&abacus->total_primes_generated, 0);
    atomic_store(&abacus->cache_hits, 0);
    atomic_store(&abacus->cache_misses, 0);
    atomic_store(&abacus->parent_lookups, 0);
    abacus->primes_generated = 0;
}

/**
 * Get cache efficiency
 */
double hierarchical_abacus_cache_efficiency(const HierarchicalAbacus* abacus) {
    if (!abacus) {
        return 0.0;
    }
    
    uint64_t hits = atomic_load(&abacus->cache_hits);
    uint64_t misses = atomic_load(&abacus->cache_misses);
    uint64_t total = hits + misses;
    
    if (total == 0) {
        return 0.0;
    }
    
    return (double)hits / (double)total * 100.0;
}

/**
 * Prefill cache
 */
int hierarchical_abacus_prefill_cache(HierarchicalAbacus* abacus, int max_primes) {
    if (!abacus || max_primes <= 0) {
        return 0;
    }
    
    int count = 0;
    BigInt prime;
    big_init(&prime);
    
    for (int i = 0; i < max_primes; i++) {
        if (hierarchical_abacus_next_prime(abacus, &prime) == 0) {
            count++;
        } else {
            break;  // No more primes in partition
        }
    }
    
    big_free(&prime);
    return count;
}

/**
 * Validate abacus
 */
bool hierarchical_abacus_validate(const HierarchicalAbacus* abacus) {
    if (!abacus) {
        fprintf(stderr, "Validation failed: NULL abacus\n");
        return false;
    }
    
    if (!abacus->local_abacus) {
        fprintf(stderr, "Validation failed: NULL local_abacus\n");
        return false;
    }
    
    if (!abacus->partition) {
        fprintf(stderr, "Validation failed: NULL partition\n");
        return false;
    }
    
    if (abacus->symmetry_group < 0 || abacus->symmetry_group >= 12) {
        fprintf(stderr, "Validation failed: Invalid symmetry group %d\n", 
                abacus->symmetry_group);
        return false;
    }
    
    if (abacus->cache_size > abacus->cache_capacity) {
        fprintf(stderr, "Validation failed: cache_size > cache_capacity\n");
        return false;
    }
    
    return true;
}

/**
 * Print abacus info
 */
void hierarchical_abacus_print_info(const HierarchicalAbacus* abacus, 
                                    const char* name) {
    if (!abacus) {
        return;
    }
    
    printf("=== Hierarchical Abacus");
    if (name) {
        printf(": %s", name);
    }
    printf(" ===\n");
    
    printf("Symmetry group: %d\n", abacus->symmetry_group);
    printf("Filter by symmetry: %s\n", abacus->filter_by_symmetry ? "YES" : "NO");
    printf("Has parent: %s\n", abacus->parent_abacus ? "YES" : "NO");
    
    printf("\nCache:\n");
    printf("  Size: %zu\n", abacus->cache_size);
    printf("  Capacity: %zu\n", abacus->cache_capacity);
    printf("  Efficiency: %.2f%%\n", hierarchical_abacus_cache_efficiency(abacus));
    
    printf("\nStatistics:\n");
    printf("  Primes generated: %lu\n", atomic_load(&abacus->total_primes_generated));
    printf("  Cache hits: %lu\n", atomic_load(&abacus->cache_hits));
    printf("  Cache misses: %lu\n", atomic_load(&abacus->cache_misses));
    printf("  Parent lookups: %lu\n", atomic_load(&abacus->parent_lookups));
    
    printf("\n");
}