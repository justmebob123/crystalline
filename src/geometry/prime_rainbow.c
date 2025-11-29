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