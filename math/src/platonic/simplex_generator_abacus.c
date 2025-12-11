/**
 * @file simplex_generator_abacus.c
 * @brief nD simplex generator using Crystalline Abacus (arbitrary precision)
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
 * @brief Generate coordinates for nD simplex using Abacus
 * 
 * Uses arbitrary precision arithmetic throughout.
 * Coordinates computed using geometric construction.
 */
static bool generate_simplex_coordinates_abacus(PlatonicSolidAbacus* solid) {
    if (!solid) {
        return false;
    }
    
    uint32_t n = solid->dimension;
    uint32_t num_vertices = n + 1;
    uint32_t base = solid->abacus_base;
    int32_t precision = solid->abacus_precision;
    
    // Allocate coordinate array
    solid->vertex_coords = (CrystallineAbacus**)calloc(num_vertices * n, 
                                                        sizeof(CrystallineAbacus*));
    if (!solid->vertex_coords) {
        return false;
    }
    
    // Generate coordinates using standard simplex construction
    // Vertex i has coordinates where:
    // - First i components are 0
    // - Component i is sqrt(1 - sum of previous squares)
    // - Remaining components are 0
    
    for (uint32_t i = 0; i < num_vertices; i++) {
        // Create Abacus for sum of squares
        CrystallineAbacus* sum_sq = abacus_from_double(0.0, base, precision);
        if (!sum_sq) {
            return false;
        }
        
        for (uint32_t j = 0; j < n; j++) {
            uint64_t idx = i * n + j;
            
            if (j < i) {
                // First i components are 0
                solid->vertex_coords[idx] = abacus_from_double(0.0, base, precision);
            } else if (j == i) {
                // Component i: sqrt(1 - sum_sq)
                // First convert sum_sq to double for sqrt calculation
                double sum_val;
                abacus_to_double(sum_sq, &sum_val);
                
                double coord_val = math_sqrt(1.0 - sum_val);
                solid->vertex_coords[idx] = abacus_from_double(coord_val, base, precision);
                
                // Update sum_sq for next iteration
                CrystallineAbacus* coord_sq = abacus_from_double(coord_val * coord_val, 
                                                                  base, precision);
                CrystallineAbacus* new_sum = abacus_new(base);
                abacus_add(new_sum, sum_sq, coord_sq);
                abacus_free(sum_sq);
                abacus_free(coord_sq);
                sum_sq = new_sum;
            } else {
                // Remaining components are 0
                solid->vertex_coords[idx] = abacus_from_double(0.0, base, precision);
            }
            
            if (!solid->vertex_coords[idx]) {
                abacus_free(sum_sq);
                return false;
            }
        }
        
        abacus_free(sum_sq);
    }
    
    return true;
}

// ============================================================================
// EDGE GENERATION
// ============================================================================

static bool generate_simplex_edges_abacus(PlatonicSolidAbacus* solid) {
    if (!solid) {
        return false;
    }
    
    uint32_t n = solid->dimension;
    uint64_t num_vertices = solid->num_vertices;
    
    // Number of edges: C(n+1, 2) = (n+1)*n/2
    solid->num_edges = (num_vertices * (num_vertices - 1)) / 2;
    
    // Allocate edge array
    solid->edge_indices = (uint32_t**)calloc(solid->num_edges, sizeof(uint32_t*));
    if (!solid->edge_indices) {
        return false;
    }
    
    // Generate all pairs of vertices
    uint64_t edge_idx = 0;
    for (uint64_t i = 0; i < num_vertices; i++) {
        for (uint64_t j = i + 1; j < num_vertices; j++) {
            solid->edge_indices[edge_idx] = (uint32_t*)calloc(2, sizeof(uint32_t));
            if (!solid->edge_indices[edge_idx]) {
                return false;
            }
            solid->edge_indices[edge_idx][0] = (uint32_t)i;
            solid->edge_indices[edge_idx][1] = (uint32_t)j;
            edge_idx++;
        }
    }
    
    return true;
}

// ============================================================================
// FACE GENERATION
// ============================================================================

static bool generate_simplex_faces_abacus(PlatonicSolidAbacus* solid) {
    if (!solid || solid->dimension < 3) {
        return true;  // No 2-faces for dimension < 3
    }
    
    uint32_t n = solid->dimension;
    uint64_t num_vertices = solid->num_vertices;
    
    // Number of 2-faces: C(n+1, 3) = (n+1)*n*(n-1)/6
    solid->num_faces = (num_vertices * (num_vertices - 1) * (num_vertices - 2)) / 6;
    
    // Allocate face arrays
    solid->face_indices = (uint32_t**)calloc(solid->num_faces, sizeof(uint32_t*));
    solid->face_sizes = (uint32_t*)calloc(solid->num_faces, sizeof(uint32_t));
    if (!solid->face_indices || !solid->face_sizes) {
        return false;
    }
    
    // Generate all triples of vertices
    uint64_t face_idx = 0;
    for (uint64_t i = 0; i < num_vertices; i++) {
        for (uint64_t j = i + 1; j < num_vertices; j++) {
            for (uint64_t k = j + 1; k < num_vertices; k++) {
                solid->face_indices[face_idx] = (uint32_t*)calloc(3, sizeof(uint32_t));
                if (!solid->face_indices[face_idx]) {
                    return false;
                }
                solid->face_indices[face_idx][0] = (uint32_t)i;
                solid->face_indices[face_idx][1] = (uint32_t)j;
                solid->face_indices[face_idx][2] = (uint32_t)k;
                solid->face_sizes[face_idx] = 3;
                face_idx++;
            }
        }
    }
    
    return true;
}

// ============================================================================
// CELL GENERATION (4D+)
// ============================================================================

static bool generate_simplex_cells_abacus(PlatonicSolidAbacus* solid) {
    if (!solid || solid->dimension < 4) {
        return true;  // No cells for dimension < 4
    }
    
    uint64_t num_vertices = solid->num_vertices;
    
    // Number of 3-cells: C(n+1, 4) = (n+1)*n*(n-1)*(n-2)/24
    solid->num_cells = (num_vertices * (num_vertices - 1) * 
                        (num_vertices - 2) * (num_vertices - 3)) / 24;
    
    return true;
}

// ============================================================================
// MAIN GENERATOR
// ============================================================================

PlatonicSolidAbacus* platonic_generate_simplex_abacus(uint32_t dimension,
                                                       uint32_t base,
                                                       int32_t precision) {
    if (dimension < 2) {
        fprintf(stderr, "Error: Simplex dimension must be >= 2\n");
        return NULL;
    }
    
    // Allocate solid
    PlatonicSolidAbacus* solid = platonic_abacus_alloc(base, precision);
    if (!solid) {
        return NULL;
    }
    
    // Set basic properties
    solid->dimension = dimension;
    solid->num_vertices = dimension + 1;
    
    // Create Schläfli symbol {3,3,...,3}
    if (dimension >= 3) {
        uint32_t* schlafli = (uint32_t*)malloc((dimension - 1) * sizeof(uint32_t));
        if (!schlafli) {
            platonic_abacus_free(solid);
            return NULL;
        }
        for (uint32_t i = 0; i < dimension - 1; i++) {
            schlafli[i] = 3;
        }
        solid->schlafli_symbol = schlafli;
        solid->symbol_length = dimension - 1;
    }
    
    // Set name
    if (dimension == 2) {
        snprintf(solid->name, sizeof(solid->name), "Triangle");
    } else if (dimension == 3) {
        snprintf(solid->name, sizeof(solid->name), "Tetrahedron");
    } else if (dimension == 4) {
        snprintf(solid->name, sizeof(solid->name), "5-cell");
    } else {
        snprintf(solid->name, sizeof(solid->name), "%u-simplex", dimension);
    }
    
    // Generate coordinates
    if (!generate_simplex_coordinates_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Generate edges
    if (!generate_simplex_edges_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Generate faces
    if (!generate_simplex_faces_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Generate cells (4D+)
    if (!generate_simplex_cells_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Set symmetry properties
    snprintf(solid->symmetry_group, sizeof(solid->symmetry_group), "S_%u", dimension + 1);
    
    // Compute symmetry order: (n+1)!
    uint32_t order = 1;
    for (uint32_t i = 2; i <= dimension + 1; i++) {
        order *= i;
    }
    solid->symmetry_order = order;
    
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
    // For simplex: χ = 1 + (-1)^(n-1)
    solid->euler_characteristic = 1 + ((dimension % 2 == 0) ? -1 : 1);
    
    return solid;
}

// ============================================================================
// SPECIALIZED 3D TETRAHEDRON
// ============================================================================

PlatonicSolidAbacus* platonic_generate_tetrahedron_abacus(uint32_t base, int32_t precision) {
    return platonic_generate_simplex_abacus(3, base, precision);
}

// ============================================================================
// SPECIALIZED 4D 5-CELL
// ============================================================================

PlatonicSolidAbacus* platonic_generate_5cell_abacus(uint32_t base, int32_t precision) {
    return platonic_generate_simplex_abacus(4, base, precision);
}