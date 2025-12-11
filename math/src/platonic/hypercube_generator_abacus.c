/**
 * @file hypercube_generator_abacus.c
 * @brief nD hypercube generator using Crystalline Abacus (arbitrary precision)
 * 
 * Pure Crystalline mathematics - NO math.h, uses Abacus throughout
 */

#include "math/platonic_generator.h"
#include "math/polytope_abacus.h"
#include "math/abacus.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// COORDINATE GENERATION WITH ABACUS
// ============================================================================

/**
 * @brief Generate coordinates for nD hypercube using Abacus
 * 
 * Vertices are at all combinations of ±1 in each dimension.
 * Uses arbitrary precision arithmetic.
 */
static bool generate_hypercube_coordinates_abacus(PlatonicSolidAbacus* solid) {
    if (!solid) {
        return false;
    }
    
    uint32_t n = solid->dimension;
    uint64_t num_vertices = solid->num_vertices;  // 2^n
    uint32_t base = solid->abacus_base;
    int32_t precision = solid->abacus_precision;
    
    // Allocate coordinate array
    solid->vertex_coords = (CrystallineAbacus**)calloc(num_vertices * n,
                                                        sizeof(CrystallineAbacus*));
    if (!solid->vertex_coords) {
        return false;
    }
    
    // Create Abacus values for +1 and -1
    CrystallineAbacus* plus_one = abacus_from_double(1.0, base, precision);
    CrystallineAbacus* minus_one = abacus_from_double(-1.0, base, precision);
    
    if (!plus_one || !minus_one) {
        abacus_free(plus_one);
        abacus_free(minus_one);
        return false;
    }
    
    // Generate all 2^n vertices
    for (uint64_t i = 0; i < num_vertices; i++) {
        for (uint32_t j = 0; j < n; j++) {
            uint64_t idx = i * n + j;
            
            // Check bit j of vertex index i
            if ((i >> j) & 1) {
                // Bit is 1: use +1
                solid->vertex_coords[idx] = abacus_from_double(1.0, base, precision);
            } else {
                // Bit is 0: use -1
                solid->vertex_coords[idx] = abacus_from_double(-1.0, base, precision);
            }
            
            if (!solid->vertex_coords[idx]) {
                abacus_free(plus_one);
                abacus_free(minus_one);
                return false;
            }
        }
    }
    
    abacus_free(plus_one);
    abacus_free(minus_one);
    
    return true;
}

// ============================================================================
// EDGE GENERATION
// ============================================================================

static bool generate_hypercube_edges_abacus(PlatonicSolidAbacus* solid) {
    if (!solid) {
        return false;
    }
    
    uint32_t n = solid->dimension;
    uint64_t num_vertices = solid->num_vertices;
    
    // Number of edges: n * 2^(n-1)
    solid->num_edges = n * (1ULL << (n - 1));
    
    // Allocate edge array
    solid->edge_indices = (uint32_t**)calloc(solid->num_edges, sizeof(uint32_t*));
    if (!solid->edge_indices) {
        return false;
    }
    
    // Generate edges: vertices differ in exactly one bit
    uint64_t edge_idx = 0;
    for (uint64_t i = 0; i < num_vertices; i++) {
        for (uint32_t bit = 0; bit < n; bit++) {
            uint64_t j = i ^ (1ULL << bit);  // Flip bit
            if (j > i) {  // Only count each edge once
                solid->edge_indices[edge_idx] = (uint32_t*)calloc(2, sizeof(uint32_t));
                if (!solid->edge_indices[edge_idx]) {
                    return false;
                }
                solid->edge_indices[edge_idx][0] = (uint32_t)i;
                solid->edge_indices[edge_idx][1] = (uint32_t)j;
                edge_idx++;
            }
        }
    }
    
    return true;
}

// ============================================================================
// FACE GENERATION
// ============================================================================

static bool generate_hypercube_faces_abacus(PlatonicSolidAbacus* solid) {
    if (!solid || solid->dimension < 3) {
        return true;  // No 2-faces for dimension < 3
    }
    
    uint32_t n = solid->dimension;
    
    // Number of 2-faces: C(n,2) * 2^(n-2)
    // For each pair of dimensions, we have 2^(n-2) square faces
    solid->num_faces = (n * (n - 1) / 2) * (1ULL << (n - 2));
    
    // Allocate face arrays
    solid->face_indices = (uint32_t**)calloc(solid->num_faces, sizeof(uint32_t*));
    solid->face_sizes = (uint32_t*)calloc(solid->num_faces, sizeof(uint32_t));
    if (!solid->face_indices || !solid->face_sizes) {
        return false;
    }
    
    // For now, just set the count and sizes
    // Full face generation can be added if needed
    for (uint64_t i = 0; i < solid->num_faces; i++) {
        solid->face_sizes[i] = 4;  // All faces are squares
    }
    
    return true;
}

// ============================================================================
// CELL GENERATION (4D+)
// ============================================================================

static bool generate_hypercube_cells_abacus(PlatonicSolidAbacus* solid) {
    if (!solid || solid->dimension < 4) {
        return true;  // No cells for dimension < 4
    }
    
    uint32_t n = solid->dimension;
    
    // Number of 3-cells: n * 2^(n-3)
    // For each dimension, we have 2^(n-3) cubic cells
    solid->num_cells = n * (1ULL << (n - 3));
    
    return true;
}

// ============================================================================
// MAIN GENERATOR
// ============================================================================

PlatonicSolidAbacus* platonic_generate_hypercube_abacus(uint32_t dimension,
                                                         uint32_t base,
                                                         int32_t precision) {
    if (dimension < 2) {
        fprintf(stderr, "Error: Hypercube dimension must be >= 2\n");
        return NULL;
    }
    
    // Allocate solid
    PlatonicSolidAbacus* solid = platonic_abacus_alloc(base, precision);
    if (!solid) {
        return NULL;
    }
    
    // Set basic properties
    solid->dimension = dimension;
    solid->num_vertices = 1ULL << dimension;  // 2^n
    
    // Create Schläfli symbol {4,3,...,3}
    if (dimension >= 3) {
        uint32_t* schlafli = (uint32_t*)malloc((dimension - 1) * sizeof(uint32_t));
        if (!schlafli) {
            platonic_abacus_free(solid);
            return NULL;
        }
        schlafli[0] = 4;
        for (uint32_t i = 1; i < dimension - 1; i++) {
            schlafli[i] = 3;
        }
        solid->schlafli_symbol = schlafli;
        solid->symbol_length = dimension - 1;
    }
    
    // Set name
    if (dimension == 2) {
        snprintf(solid->name, sizeof(solid->name), "Square");
    } else if (dimension == 3) {
        snprintf(solid->name, sizeof(solid->name), "Cube");
    } else if (dimension == 4) {
        snprintf(solid->name, sizeof(solid->name), "Tesseract");
    } else {
        snprintf(solid->name, sizeof(solid->name), "%u-cube", dimension);
    }
    
    // Generate coordinates
    if (!generate_hypercube_coordinates_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Generate edges
    if (!generate_hypercube_edges_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Generate faces
    if (!generate_hypercube_faces_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Generate cells (4D+)
    if (!generate_hypercube_cells_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Set symmetry properties
    if (dimension == 3) {
        snprintf(solid->symmetry_group, sizeof(solid->symmetry_group), "O_h");
        solid->symmetry_order = 48;
    } else if (dimension == 4) {
        snprintf(solid->symmetry_group, sizeof(solid->symmetry_group), "B_4");
        solid->symmetry_order = 384;
    } else {
        snprintf(solid->symmetry_group, sizeof(solid->symmetry_group), "B_%u", dimension);
        // Order: 2^n * n!
        uint32_t order = 1ULL << dimension;
        for (uint32_t i = 2; i <= dimension; i++) {
            order *= i;
        }
        solid->symmetry_order = order;
    }
    
    // Set properties
    solid->is_regular = true;
    solid->is_convex = true;
    solid->has_golden_ratio = false;
    
    // Compute CLLM properties
    solid->embedding_dim = solid->num_vertices * 12;
    solid->hidden_dim = solid->num_edges * 12;
    solid->num_layers = (uint32_t)solid->num_faces;
    solid->num_heads = 12;
    
    // Compute Euler characteristic
    // For hypercube: χ = 0 for even dimensions, 2 for odd dimensions
    solid->euler_characteristic = (dimension % 2 == 0) ? 0 : 2;
    
    return solid;
}

// ============================================================================
// SPECIALIZED 3D CUBE
// ============================================================================

PlatonicSolidAbacus* platonic_generate_cube_abacus(uint32_t base, int32_t precision) {
    return platonic_generate_hypercube_abacus(3, base, precision);
}

// ============================================================================
// SPECIALIZED 4D TESSERACT
// ============================================================================

PlatonicSolidAbacus* platonic_generate_tesseract_abacus(uint32_t base, int32_t precision) {
    return platonic_generate_hypercube_abacus(4, base, precision);
}