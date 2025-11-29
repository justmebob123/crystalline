// prime_rainbow.c - Prime Rainbow Table (Crystalline Lattice Visualization)
// Integrated from: prime_rainbow_duplicates.c, prime_rainbow_minimal.c
// Part of the Prime Mathematics Library - Crystalline Lattice Architecture

#include "prime_rainbow.h"
#include "crystal_abacus.h"
#include <stdlib.h>
#include "clock_lattice.h"
#include "../include/prime_math_custom.h"

// Global rainbow table
static PrimeRainbowTable g_rainbow_table = {0};
static bool g_rainbow_initialized = false;

// ═══════════════════════════════════════════════════════════════════════════
// RAINBOW TABLE INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════

void rainbow_table_init(void) {
    if (g_rainbow_initialized) return;
    
    g_rainbow_table.root = NULL;
    g_rainbow_table.count = 0;
    g_rainbow_table.is_stable = true;
    
    // Initialize fold progression
    for (int i = 0; i < RAINBOW_LAYERS; i++) {
        g_rainbow_table.fold_progression[i] = 0.0;
    }
    
    // Initialize negative space
    for (int i = 0; i < 10; i++) {
        g_rainbow_table.negative_space[i] = 0.0;
    }
    
    g_rainbow_initialized = true;
}

void rainbow_table_cleanup(void) {
    if (!g_rainbow_initialized) return;
    
    // Recursive function to free tree nodes
    void free_node_recursive(PrimeRainbowNode* node) {
        if (!node) return;
        
        // Free children first
        if (node->children) {
            for (int i = 0; i < node->child_count; i++) {
                free_node_recursive(node->children[i]);
            }
            free(node->children);
        }
        
        // Free the prime
        if (node->entry.prime) {
            big_free(node->entry.prime);
            free(node->entry.prime);
        }
        
        // Free the node itself
        free(node);
    }
    
    // Free the tree starting from root
    free_node_recursive(g_rainbow_table.root);
    
    g_rainbow_table.root = NULL;
    g_rainbow_table.count = 0;
    g_rainbow_table.is_stable = false;
    
    g_rainbow_initialized = false;
}

PrimeRainbowTable* rainbow_table_get(void) {
    if (!g_rainbow_initialized) {
        rainbow_table_init();
    }
    return &g_rainbow_table;
}

// ═══════════════════════════════════════════════════════════════════════════
// FAST PRIME COORDINATE FUNCTIONS (Integer)
// ═══════════════════════════════════════════════════════════════════════════

// CORRECTED: Use clock lattice mapping instead of spiral formulas
   double fast_prime_angle(int prime_index) {
       // Get clock position from lattice mapping
       BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
       return pos.angle;
}

double fast_prime_radius(int prime_index) {
       // Get radius from clock lattice (0.25 to 1.0, counting INWARD)
       BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
       return pos.radius;
}

double fast_prime_frequency(int prime_index) {
       // Frequency based on clock position, not prime value
       BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
       
       // Base frequency on ring (deeper rings = higher frequency)
       double base_freq = 432.0;  // A4 tuning
       double ring_multiplier = 1.0 + (pos.ring * 0.5);  // Ring 0=1.0, Ring 3=2.5
       
       // Modulate by position within ring
       double position_factor = (double)pos.position / 12.0;  // Normalized
       
       return base_freq * ring_multiplier * (1.0 + position_factor * PHI);
}

int fast_prime_layer(int prime_index) {
       // Get ring from clock lattice (0-3 for first 232 primes)
       BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
       return pos.ring;
}

void fast_prime_fold_coords(int prime_index, double* x, double* y, double* z) {
       if (!x || !y || !z) return;
       
       // Get clock position
       BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
       
       // Fold to 3D sphere
       SphereCoord sphere = fold_clock_to_sphere(pos);
       
       *x = sphere.x;
       *y = sphere.y;
       *z = sphere.z;
}


   // Get modular relationships for a prime
   PrimeModular fast_prime_modular(uint64_t prime) {
       return get_prime_modular(prime);
   }

   // Check if position is sacred (π, 12 o'clock, etc.)
   bool fast_prime_is_sacred(int prime_index) {
       BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
       return is_sacred_position(pos);
   }

   // Get position on clock ring
   int fast_prime_position(int prime_index) {
       BabylonianClockPosition pos = map_prime_index_to_clock(prime_index);
       return pos.position;
   }
// ═══════════════════════════════════════════════════════════════════════════
// FAST PRIME COORDINATE FUNCTIONS (BigInt)
// ═══════════════════════════════════════════════════════════════════════════

double big_fast_prime_angle(BigInt *prime) {
       if (!prime || prime->len == 0) return 0.0;
       
       // Convert to prime index (approximation for large primes)
       uint64_t prime_index = prime->d[0];
       return fast_prime_angle((int)prime_index);
}

double big_fast_prime_radius(BigInt *prime) {
       if (!prime || prime->len == 0) return 0.0;
       
       uint64_t prime_index = prime->d[0];
       return fast_prime_radius((int)prime_index);
}

int big_fast_prime_layer(BigInt *prime) {
    if (!prime || prime->len == 0) return 1;
    
    uint64_t val = prime->d[0];
    return fast_prime_layer((int)val);
}

// ═══════════════════════════════════════════════════════════════════════════

// ════════════════════════════════════════════════════════════════════════════
// PRIME STORAGE FUNCTIONS - Make Rainbow Table the Complete Abacus
// ════════════════════════════════════════════════════════════════════════════

/**
 * Add a prime to the rainbow table
 * 
 * This makes the rainbow table the SINGLE SOURCE OF TRUTH for all primes.
 * Each prime is stored with its complete geometric representation.
 * 
 * @param prime Prime number to add (BigInt)
 * @return 0 on success, -1 on error
 */
int rainbow_table_add_prime(BigInt* prime) {
    if (!g_rainbow_initialized) {
        rainbow_table_init();
    }
    
    if (!prime) return -1;
    
    // Create new entry
    RainbowEntry entry;
    entry.prime = (BigInt*)malloc(sizeof(BigInt));
    if (!entry.prime) return -1;
    
    // Copy the prime
    big_init(entry.prime);
    big_copy(entry.prime, prime);
    
    // Create new node
    PrimeRainbowNode* node = (PrimeRainbowNode*)malloc(sizeof(PrimeRainbowNode));
    if (!node) {
        big_free(entry.prime);
        free(entry.prime);
        return -1;
    }
    
    node->entry = entry;
    node->children = NULL;
    node->child_count = 0;
    
    // Add to tree (simple: just add as child of root for now)
    // TODO: Organize by symmetry group in tree structure
    if (!g_rainbow_table.root) {
        g_rainbow_table.root = node;
    } else {
        // Add as child of root
        PrimeRainbowNode** new_children = (PrimeRainbowNode**)realloc(
            g_rainbow_table.root->children,
            (g_rainbow_table.root->child_count + 1) * sizeof(PrimeRainbowNode*)
        );
        if (!new_children) {
            big_free(entry.prime);
            free(entry.prime);
            free(node);
            return -1;
        }
        g_rainbow_table.root->children = new_children;
        g_rainbow_table.root->children[g_rainbow_table.root->child_count] = node;
        g_rainbow_table.root->child_count++;
    }
    
    g_rainbow_table.count++;
    
    return 0;
}

/**
 * Get the nth prime from the rainbow table
 * 
 * @param index Prime index (0-based)
 * @return Prime at index, or NULL if out of bounds
 */
BigInt* rainbow_table_get_prime(int index) {
    if (!g_rainbow_initialized) return NULL;
    if (index < 0 || index >= g_rainbow_table.count) return NULL;
    
    // Simple traversal for now (TODO: optimize with indexing)
    if (!g_rainbow_table.root) return NULL;
    
    if (index == 0) {
        return g_rainbow_table.root->entry.prime;
    }
    
    // Traverse children
    int current_index = 1;
    for (int i = 0; i < g_rainbow_table.root->child_count; i++) {
        if (current_index == index) {
            return g_rainbow_table.root->children[i]->entry.prime;
        }
        current_index++;
    }
    
    return NULL;
}

/**
 * Get prime count in rainbow table
 */
int rainbow_table_get_count(void) {
    if (!g_rainbow_initialized) return 0;
    return g_rainbow_table.count;
}

/**
 * Generate primes using crystalline sieve and add to rainbow table
 * 
 * This integrates the fast crystalline sieve with the rainbow table,
 * making the rainbow table the complete abacus structure.
 * 
 * @param target_count Number of primes to generate
 * @return Number of primes generated, or -1 on error
 */
int rainbow_table_generate_primes(int target_count) {
    if (!g_rainbow_initialized) {
        rainbow_table_init();
    }
    
    if (target_count <= 0) return 0;
    
    // Use crystalline sieve to generate primes
    // For now, use simple generation (TODO: integrate cllm_crystalline_sieve)
    
    // Start with first few primes
    uint64_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};
    int initial_count = sizeof(primes) / sizeof(primes[0]);
    
    int count = 0;
    for (int i = 0; i < initial_count && count < target_count; i++) {
        BigInt prime;
        big_init(&prime);
        big_from_int(&prime, primes[i]);
        
        if (rainbow_table_add_prime(&prime) == 0) {
            count++;
        }
        
        big_free(&prime);
    }
    
    return count;
}

/**
 * Load important primes into rainbow table
 * 
 * Loads sacred primes, Mersenne primes, etc.
 * 
 * @return Number of primes loaded, or -1 on error
 */
int rainbow_table_load_important_primes(void) {
    if (!g_rainbow_initialized) {
        rainbow_table_init();
    }
    
    // Important primes (sacred, Mersenne, etc.)
    uint64_t important[] = {
        // First 20 primes
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        // Mersenne primes
        127, 8191, 131071, 524287,
        // Sacred primes (12-fold symmetry)
        73, 97, 109, 157, 181, 193
    };
    
    int count = sizeof(important) / sizeof(important[0]);
    int loaded = 0;
    
    for (int i = 0; i < count; i++) {
        BigInt prime;
        big_init(&prime);
        big_from_int(&prime, important[i]);
        
        if (rainbow_table_add_prime(&prime) == 0) {
            loaded++;
        }
        
        big_free(&prime);
    }
    
    return loaded;
}

// RAINBOW TABLE ANALYSIS
// ═══════════════════════════════════════════════════════════════════════════

double rainbow_table_check_stability(PrimeRainbowTable* table) {
    (void)table;
    // Stability metric for crystalline lattice
    return 1.0; // Assume stable
}

double rainbow_table_self_similarity(PrimeRainbowTable* table) {
    (void)table;
    // Self-similarity metric for fractal structure
    return PHI; // Golden ratio self-similarity
}