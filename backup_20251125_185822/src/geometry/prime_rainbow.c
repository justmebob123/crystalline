// prime_rainbow.c - Prime Rainbow Table (Crystalline Lattice Visualization)
// Integrated from: prime_rainbow_duplicates.c, prime_rainbow_minimal.c
// Part of the Prime Mathematics Library - Crystalline Lattice Architecture

#include "prime_rainbow.h"
#include "crystal_abacus.h"
#include <stdlib.h>
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

double fast_prime_angle(int prime) {
    // Crystalline lattice angle mapping
    return (2.0 * PRIME_PI * prime) / (prime + 1);
}

double fast_prime_radius(int prime) {
    // Logarithmic spiral radius
    return prime_sqrt((double)prime);
}

double fast_prime_frequency(int prime) {
    // Golden ratio frequency modulation
    return prime * PHI;
}

int fast_prime_layer(int prime) {
    // Map to 7 crystalline layers
    return (prime % RAINBOW_LAYERS) + 1;
}

void fast_prime_fold_coords(int prime, double* x, double* y) {
    // Folded coordinate mapping for crystalline structure
    double angle = fast_prime_angle(prime);
    double radius = fast_prime_radius(prime);
    
    *x = radius * prime_cos(angle);
    *y = radius * prime_sin(angle);
}

// ═══════════════════════════════════════════════════════════════════════════
// FAST PRIME COORDINATE FUNCTIONS (BigInt)
// ═══════════════════════════════════════════════════════════════════════════

double big_fast_prime_angle(BigInt *prime) {
    if (!prime || prime->len == 0) return 0.0;
    
    // Convert to int for angle calculation
    uint64_t val = prime->d[0];
    return fast_prime_angle((int)val);
}

double big_fast_prime_radius(BigInt *prime) {
    if (!prime || prime->len == 0) return 0.0;
    
    uint64_t val = prime->d[0];
    return fast_prime_radius((int)val);
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