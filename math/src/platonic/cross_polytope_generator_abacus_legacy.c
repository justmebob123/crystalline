/**
 * @file cross_polytope_generator_abacus.c
 * @brief nD cross-polytope generator using Crystalline Abacus (arbitrary precision)
 * 
 * Pure Crystalline mathematics - NO math.h, uses Abacus throughout
 */

#include "math/platonic_generator_abacus.h"
#include "math/polytope_abacus.h"
#include "math/abacus.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// COORDINATE GENERATION WITH ABACUS
// ============================================================================

/**
 * @brief Generate coordinates for nD cross-polytope using Abacus
 * 
 * Vertices are at ±1 on each axis.
 * Uses arbitrary precision arithmetic.
 */
static bool generate_cross_polytope_coordinates_abacus(PlatonicSolidAbacus* solid) {
    if (!solid) {
        return false;
    }
    
    uint32_t n = solid->dimension;
    uint64_t num_vertices = 2 * n;
    uint32_t base = solid->abacus_base;
    int32_t precision = solid->abacus_precision;
    
    // Allocate coordinate array
    solid->vertex_coords = (CrystallineAbacus**)calloc(num_vertices * n,
                                                        sizeof(CrystallineAbacus*));
    if (!solid->vertex_coords) {
        return false;
    }
    
    // Generate vertices: ±1 on each axis
    // First n vertices: +1 on axis i, 0 elsewhere
    // Next n vertices: -1 on axis i, 0 elsewhere
    for (uint64_t i = 0; i < num_vertices; i++) {
        uint32_t axis = i % n;
        bool positive = (i < n);
        
        for (uint32_t j = 0; j < n; j++) {
            uint64_t idx = i * n + j;
            
            if (j == axis) {
                // On the axis: ±1
                double value = positive ? 1.0 : -1.0;
                solid->vertex_coords[idx] = abacus_from_double(value, base, precision);
            } else {
                // Off the axis: 0
                solid->vertex_coords[idx] = abacus_from_double(0.0, base, precision);
            }
            
            if (!solid->vertex_coords[idx]) {
                return false;
            }
        }
    }
    
    return true;
}

// ============================================================================
// EDGE GENERATION
// ============================================================================

static bool generate_cross_polytope_edges_abacus(PlatonicSolidAbacus* solid) {
    if (!solid) {
        return false;
    }
    
    uint32_t n = solid->dimension;
    uint64_t num_vertices = solid->num_vertices;
    
    // Number of edges: 2n(n-1)
    solid->num_edges = 2 * n * (n - 1);
    
    // Allocate edge array
    solid->edge_indices = (uint32_t**)calloc(solid->num_edges, sizeof(uint32_t*));
    if (!solid->edge_indices) {
        return false;
    }
    
    // Generate edges: connect vertices on different axes
    uint64_t edge_idx = 0;
    for (uint64_t i = 0; i < num_vertices; i++) {
        for (uint64_t j = i + 1; j < num_vertices; j++) {
            uint32_t axis_i = i % n;
            uint32_t axis_j = j % n;
            
            // Connect if on different axes
            if (axis_i != axis_j) {
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

static bool generate_cross_polytope_faces_abacus(PlatonicSolidAbacus* solid) {
    if (!solid || solid->dimension < 3) {
        return true;  // No 2-faces for dimension < 3
    }
    
    uint32_t n = solid->dimension;
    
    // Number of 2-faces
    // For 3D: 8 faces = 2^3
    // For 4D+: 2^(n-1) × n
    if (n == 3) {
        solid->num_faces = 1ULL << n;  // 2^3 = 8
    } else {
        solid->num_faces = (1ULL << (n - 1)) * n;  // 2^(n-1) × n
    }
    
    // Allocate face arrays
    solid->face_indices = (uint32_t**)calloc(solid->num_faces, sizeof(uint32_t*));
    solid->face_sizes = (uint32_t*)calloc(solid->num_faces, sizeof(uint32_t));
    if (!solid->face_indices || !solid->face_sizes) {
        return false;
    }
    
    // For now, just set the sizes (all triangular)
    for (uint64_t i = 0; i < solid->num_faces; i++) {
        solid->face_sizes[i] = 3;
    }
    
    return true;
}

// ============================================================================
// CELL GENERATION (4D+)
// ============================================================================

static bool generate_cross_polytope_cells_abacus(PlatonicSolidAbacus* solid) {
    if (!solid || solid->dimension < 4) {
        return true;  // No cells for dimension < 4
    }
    
    uint32_t n = solid->dimension;
    
    // Number of 3-cells: 2^n
    solid->num_cells = 1ULL << n;
    
    return true;
}

// ============================================================================
// MAIN GENERATOR
// ============================================================================

PlatonicSolidAbacus* platonic_generate_cross_polytope_abacus(uint32_t dimension,
                                                              uint32_t base,
                                                              int32_t precision) {
    if (dimension < 2) {
        fprintf(stderr, "Error: Cross-polytope dimension must be >= 2\n");
        return NULL;
    }
    
    // Allocate solid
    PlatonicSolidAbacus* solid = platonic_abacus_alloc(base, precision);
    if (!solid) {
        return NULL;
    }
    
    // Set basic properties
    solid->dimension = dimension;
    solid->num_vertices = 2 * dimension;
    
    // Create Schläfli symbol {3,3,...,4}
    if (dimension >= 3) {
        uint32_t* schlafli = (uint32_t*)malloc((dimension - 1) * sizeof(uint32_t));
        if (!schlafli) {
            platonic_abacus_free(solid);
            return NULL;
        }
        for (uint32_t i = 0; i < dimension - 2; i++) {
            schlafli[i] = 3;
        }
        schlafli[dimension - 2] = 4;
        solid->schlafli_symbol = schlafli;
        solid->symbol_length = dimension - 1;
    }
    
    // Set name
    if (dimension == 2) {
        snprintf(solid->name, sizeof(solid->name), "Square");
    } else if (dimension == 3) {
        snprintf(solid->name, sizeof(solid->name), "Octahedron");
    } else if (dimension == 4) {
        snprintf(solid->name, sizeof(solid->name), "16-cell");
    } else {
        snprintf(solid->name, sizeof(solid->name), "%u-orthoplex", dimension);
    }
    
    // Generate coordinates
    if (!generate_cross_polytope_coordinates_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Generate edges
    if (!generate_cross_polytope_edges_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Generate faces
    if (!generate_cross_polytope_faces_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Generate cells (4D+)
    if (!generate_cross_polytope_cells_abacus(solid)) {
        platonic_abacus_free(solid);
        return NULL;
    }
    
    // Set symmetry properties
    if (dimension == 3) {
        snprintf(solid->symmetry_group, sizeof(solid->symmetry_group), "O_h");
        solid->symmetry_order = 48;
    } else if (dimension == 4) {
        snprintf(solid->symmetry_group, sizeof(solid->symmetry_group), "S_5");
        solid->symmetry_order = 120;
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
    // For cross-polytope: χ = 0 for even dimensions, 2 for odd dimensions
    solid->euler_characteristic = (dimension % 2 == 0) ? 0 : 2;
    
    return solid;
}

// ============================================================================
// SPECIALIZED 3D OCTAHEDRON
// ============================================================================

PlatonicSolidAbacus* platonic_generate_octahedron_abacus(uint32_t base, int32_t precision) {
    return platonic_generate_cross_polytope_abacus(3, base, precision);
}

// ============================================================================
// SPECIALIZED 4D 16-CELL
// ============================================================================

PlatonicSolidAbacus* platonic_generate_16cell_abacus(uint32_t base, int32_t precision) {
    return platonic_generate_cross_polytope_abacus(4, base, precision);
}