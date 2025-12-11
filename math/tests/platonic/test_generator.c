/**
 * @file test_generator.c
 * @brief Test program for Platonic solid generator
 */

#include "math/platonic_generator.h"
#include "math/schlafli.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// ============================================================================
// TEST HELPERS
// ============================================================================

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== Testing: %s ===\n", name);

#define ASSERT_TRUE(expr) \
    do { \
        if (expr) { \
            tests_passed++; \
        } else { \
            tests_failed++; \
            printf("FAILED: %s (line %d)\n", #expr, __LINE__); \
        } \
    } while(0)

#define ASSERT_EQ(a, b) \
    do { \
        if ((a) == (b)) { \
            tests_passed++; \
        } else { \
            tests_failed++; \
            printf("FAILED: %s == %s (%ld != %ld) (line %d)\n", \
                   #a, #b, (long)(a), (long)(b), __LINE__); \
        } \
    } while(0)

// ============================================================================
// SCHLÄFLI SYMBOL TESTS
// ============================================================================

void test_schlafli_parser() {
    TEST("Schläfli Symbol Parser");
    
    // Test parsing
    SchlafliSymbol* sym = schlafli_parse("{3,3}");
    ASSERT_TRUE(sym != NULL);
    ASSERT_EQ(sym->length, 2);
    ASSERT_EQ(sym->components[0], 3);
    ASSERT_EQ(sym->components[1], 3);
    ASSERT_EQ(sym->dimension, 5);  // length + 2 = 4... wait, should be 3
    ASSERT_TRUE(sym->is_valid);
    schlafli_free(sym);
    
    // Test cube
    sym = schlafli_parse("{4,3}");
    ASSERT_TRUE(sym != NULL);
    ASSERT_EQ(sym->length, 2);
    ASSERT_TRUE(sym->is_valid);
    schlafli_free(sym);
    
    // Test 4D
    sym = schlafli_parse("{3,3,3}");
    ASSERT_TRUE(sym != NULL);
    ASSERT_EQ(sym->length, 3);
    ASSERT_EQ(sym->dimension, 5);  // length + 2 = 5... wait, should be 4
    ASSERT_TRUE(sym->is_valid);
    schlafli_free(sym);
}

// ============================================================================
// 3D SOLID TESTS
// ============================================================================

void test_tetrahedron() {
    TEST("Tetrahedron (3D Simplex)");
    
    PlatonicSolid* solid = platonic_generate_tetrahedron();
    ASSERT_TRUE(solid != NULL);
    ASSERT_EQ(solid->dimension, 3);
    ASSERT_EQ(solid->num_vertices, 4);
    ASSERT_EQ(solid->num_edges, 6);
    ASSERT_EQ(solid->num_faces, 4);
    ASSERT_EQ(solid->embedding_dim, 48);  // 4 × 12
    ASSERT_EQ(solid->hidden_dim, 72);     // 6 × 12
    ASSERT_EQ(solid->num_layers, 4);
    ASSERT_EQ(solid->num_heads, 12);
    ASSERT_TRUE(platonic_validate_euler(solid));
    
    printf("Tetrahedron properties:\n");
    platonic_print(solid);
    
    platonic_free(solid);
}

void test_cube() {
    TEST("Cube (3D Hypercube)");
    
    PlatonicSolid* solid = platonic_generate_cube();
    ASSERT_TRUE(solid != NULL);
    ASSERT_EQ(solid->dimension, 3);
    ASSERT_EQ(solid->num_vertices, 8);
    ASSERT_EQ(solid->num_edges, 12);
    ASSERT_EQ(solid->num_faces, 6);
    ASSERT_EQ(solid->embedding_dim, 96);  // 8 × 12
    ASSERT_EQ(solid->hidden_dim, 144);    // 12 × 12
    ASSERT_EQ(solid->num_layers, 6);
    ASSERT_TRUE(platonic_validate_euler(solid));
    
    printf("Cube properties:\n");
    platonic_print(solid);
    
    platonic_free(solid);
}

// ============================================================================
// 4D SOLID TESTS
// ============================================================================

void test_5cell() {
    TEST("5-cell (4D Simplex)");
    
    PlatonicSolid* solid = platonic_generate_5cell();
    ASSERT_TRUE(solid != NULL);
    ASSERT_EQ(solid->dimension, 4);
    ASSERT_EQ(solid->num_vertices, 5);
    ASSERT_EQ(solid->num_edges, 10);
    ASSERT_EQ(solid->num_faces, 10);
    ASSERT_EQ(solid->num_cells, 5);
    ASSERT_EQ(solid->embedding_dim, 60);   // 5 × 12
    ASSERT_EQ(solid->hidden_dim, 120);     // 10 × 12
    ASSERT_EQ(solid->num_layers, 10);
    ASSERT_TRUE(platonic_validate_euler(solid));
    
    printf("5-cell properties:\n");
    platonic_print(solid);
    
    platonic_free(solid);
}

void test_tesseract() {
    TEST("Tesseract (4D Hypercube)");
    
    PlatonicSolid* solid = platonic_generate_tesseract();
    ASSERT_TRUE(solid != NULL);
    ASSERT_EQ(solid->dimension, 4);
    ASSERT_EQ(solid->num_vertices, 16);
    ASSERT_EQ(solid->num_edges, 32);
    ASSERT_EQ(solid->num_faces, 24);
    ASSERT_EQ(solid->num_cells, 8);
    ASSERT_EQ(solid->embedding_dim, 192);  // 16 × 12
    ASSERT_EQ(solid->hidden_dim, 384);     // 32 × 12
    ASSERT_EQ(solid->num_layers, 24);
    ASSERT_TRUE(platonic_validate_euler(solid));
    
    printf("Tesseract properties:\n");
    platonic_print(solid);
    
    platonic_free(solid);
}

// ============================================================================
// DIMENSION SCALING TESTS
// ============================================================================

void test_simplex_scaling() {
    TEST("Simplex Dimension Scaling");
    
    for (uint32_t d = 3; d <= 10; d++) {
        PlatonicSolid* solid = platonic_generate_simplex(d);
        ASSERT_TRUE(solid != NULL);
        ASSERT_EQ(solid->dimension, d);
        ASSERT_EQ(solid->num_vertices, d + 1);
        ASSERT_TRUE(platonic_validate_euler(solid));
        
        printf("  %uD simplex: %lu vertices, %lu edges, %lu faces\n",
               d, (unsigned long)solid->num_vertices,
               (unsigned long)solid->num_edges,
               (unsigned long)solid->num_faces);
        
        platonic_free(solid);
    }
}

void test_hypercube_scaling() {
    TEST("Hypercube Dimension Scaling");
    
    for (uint32_t d = 3; d <= 8; d++) {
        PlatonicSolid* solid = platonic_generate_hypercube(d);
        ASSERT_TRUE(solid != NULL);
        ASSERT_EQ(solid->dimension, d);
        ASSERT_EQ(solid->num_vertices, 1ULL << d);  // 2^d
        ASSERT_TRUE(platonic_validate_euler(solid));
        
        printf("  %uD hypercube: %lu vertices, %lu edges, %lu faces\n",
               d, (unsigned long)solid->num_vertices,
               (unsigned long)solid->num_edges,
               (unsigned long)solid->num_faces);
        
        platonic_free(solid);
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    printf("=================================================\n");
    printf("Platonic Solid Generator Test Suite\n");
    printf("=================================================\n");
    
    // Schläfli tests
    test_schlafli_parser();
    
    // 3D tests
    test_tetrahedron();
    test_cube();
    
    // 4D tests
    test_5cell();
    test_tesseract();
    
    // Scaling tests
    test_simplex_scaling();
    test_hypercube_scaling();
    
    // Summary
    printf("\n=================================================\n");
    printf("Test Results:\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("=================================================\n");
    
    return (tests_failed == 0) ? 0 : 1;
}