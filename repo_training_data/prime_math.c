// prime_math.c - Main library implementation
#include "prime_math.h"
#include <stdlib.h>
#include <string.h>

// Library state
static bool initialized = false;
static int current_lattice_depth = 3;
static PrimeMathStats stats = {0};

// ═══════════════════════════════════════════════════════════════════════
// LIBRARY CONTROL
// ═══════════════════════════════════════════════════════════════════════

void prime_math_init_depth(int depth) {
    if (initialized) {
        prime_math_cleanup();
    }
    
    current_lattice_depth = depth;
    initialized = true;
    
    // Initialize statistics
    memset(&stats, 0, sizeof(PrimeMathStats));
    stats.lattice_depth = depth;
    stats.prime_cache_size = 0;
    stats.rainbow_table_count = 0;
    stats.rainbow_stable = true;
    stats.rainbow_self_similarity = 1.0;
    stats.abacus_primes_generated = 0;
    
    // Initialize sub-systems
    // rainbow_table_init(); // Temporarily disabled - missing implementation
}

void prime_math_init(void) {
    prime_math_init_depth(3);  // Default depth
}

void prime_math_cleanup(void) {
    if (!initialized) return;
    
    // Cleanup sub-systems
    // rainbow_table_cleanup(); // Temporarily disabled - missing implementation
    
    initialized = false;
    memset(&stats, 0, sizeof(PrimeMathStats));
}

bool prime_math_is_initialized(void) {
    return initialized;
}

int prime_math_get_depth(void) {
    return current_lattice_depth;
}

// ═══════════════════════════════════════════════════════════════════════
// LIBRARY INFORMATION
// ═══════════════════════════════════════════════════════════════════════

const char* prime_math_version(void) {
    return "1.0.0-alpha";
}

const char* prime_math_description(void) {
    return "Arbitrary precision mathematics with prime-based lattice operations";
}

// ═══════════════════════════════════════════════════════════════════════
// RUNTIME STATS
// ═══════════════════════════════════════════════════════════════════════

PrimeMathStats prime_math_stats(void) {
    return stats;
}

void prime_math_extend_cache(int new_limit) {
    // Placeholder - extend prime cache
    stats.prime_cache_size = new_limit;
}

const int* prime_math_get_cache(int* size) {
    // Placeholder - return prime cache
    if (size) *size = 0;
    return NULL;
}

void big_prime_math_extend_cache(BigInt *new_limit) {
    (void)new_limit; // Suppress unused parameter warning
    // Placeholder - BigInt cache extension
}

BigInt** big_prime_math_get_cache(int* size) {
    // Placeholder - BigInt cache retrieval
    if (size) *size = 0;
    return NULL;
}