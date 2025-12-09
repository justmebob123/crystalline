/**
 * @file test_platonic_recovery.c
 * @brief Test High-Dimensional Platonic Solid Recovery
 * 
 * This demonstrates the FULL OBJECTIVE 28 implementation:
 * - High-dimensional Platonic solids (13+ dimensions)
 * - Large vertex counts (2^11+)
 * - Real tetration towers (depth 29-59)
 * - Spatial AND temporal oscillation detection
 * - Dynamic scaling when needed
 * - Persistent model files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "platonic_model.h"

void print_separator() {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
}

int main(int argc, char** argv) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                                                           ║\n");
    printf("║  OBJECTIVE 28: Universal Blind Recovery                  ║\n");
    printf("║  High-Dimensional Platonic Solid Test                    ║\n");
    printf("║                                                           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    
    print_separator();
    
    // Parse command line arguments
    uint32_t initial_dimensions = 13;
    uint32_t initial_vertices = 2048;  // 2^11
    uint64_t max_iterations = 100;
    double convergence_threshold = 0.001;
    const char* save_path = "models/test_recovery.platonic";
    
    if (argc > 1) initial_dimensions = atoi(argv[1]);
    if (argc > 2) initial_vertices = atoi(argv[2]);
    if (argc > 3) max_iterations = atol(argv[3]);
    if (argc > 4) convergence_threshold = atof(argv[4]);
    if (argc > 5) save_path = argv[5];
    
    printf("Configuration:\n");
    printf("  Initial dimensions: %u\n", initial_dimensions);
    printf("  Initial vertices: %u\n", initial_vertices);
    printf("  Max iterations: %lu\n", max_iterations);
    printf("  Convergence threshold: %.6f\n", convergence_threshold);
    printf("  Save path: %s\n", save_path);
    
    print_separator();
    
    // Create high-dimensional Platonic solid
    printf("Creating high-dimensional Platonic solid...\n");
    PlatonicModel* model = platonic_model_create(
        PLATONIC_ICOSAHEDRON,  // Best for sphere packing
        initial_dimensions,
        initial_vertices
    );
    
    if (!model) {
        printf("❌ Failed to create model\n");
        return 1;
    }
    
    print_separator();
    
    // Simulate corruption (corrupt 25% of vertices)
    printf("Simulating corruption (25%% of vertices)...\n");
    uint32_t num_corrupted = model->num_vertices / 4;
    
    srand(time(NULL));
    for (uint32_t i = 0; i < num_corrupted; i++) {
        uint32_t v = rand() % model->num_vertices;
        model->corruption_mask[v] = true;
        model->confidence_scores[v] = 0.0;
        
        // Corrupt vertex positions
        for (uint32_t d = 0; d < model->num_dimensions; d++) {
            model->vertex_positions[v * model->num_dimensions + d] = 
                ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        }
    }
    
    printf("✓ Corrupted %u / %u vertices (%.1f%%)\n", 
           num_corrupted, model->num_vertices,
           100.0 * num_corrupted / model->num_vertices);
    
    print_separator();
    
    // Run recovery
    printf("Running recovery algorithm...\n");
    bool converged = platonic_recover(model, max_iterations, convergence_threshold);
    
    print_separator();
    
    // Validate results
    printf("Validating recovered model...\n");
    bool euler_valid = platonic_model_validate_euler(model);
    bool coprime_valid = platonic_model_validate_coprime(model);
    bool geometry_valid = platonic_model_validate_geometry(model);
    
    printf("\nValidation Results:\n");
    printf("  Euler's formula: %s\n", euler_valid ? "✓ Valid" : "❌ Invalid");
    printf("  Coprime relationships: %s\n", coprime_valid ? "✓ Valid" : "❌ Invalid");
    printf("  Geometric constraints: %s\n", geometry_valid ? "✓ Valid" : "❌ Invalid");
    
    print_separator();
    
    // Save model
    printf("Saving model to disk...\n");
    if (platonic_model_save(model, save_path)) {
        printf("✓ Model saved to: %s\n", save_path);
    } else {
        printf("❌ Failed to save model\n");
    }
    
    print_separator();
    
    // Test load
    printf("Testing model load...\n");
    PlatonicModel* loaded_model = platonic_model_load(save_path);
    if (loaded_model) {
        printf("✓ Model loaded successfully\n");
        printf("  Dimensions match: %s\n", 
               loaded_model->num_dimensions == model->num_dimensions ? "Yes" : "No");
        printf("  Vertices match: %s\n",
               loaded_model->num_vertices == model->num_vertices ? "Yes" : "No");
        platonic_model_free(loaded_model);
    } else {
        printf("❌ Failed to load model\n");
    }
    
    print_separator();
    
    // Final summary
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║  TEST SUMMARY                                             ║\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║  Status: %s\n", converged ? "CONVERGED ✓" : "NOT CONVERGED ❌");
    printf("║  Final dimensions: %u (scaled %u times)\n", 
           model->num_dimensions, model->num_dimension_scales);
    printf("║  Final vertices: %u (scaled %u times)\n",
           model->num_vertices, model->num_vertex_scales);
    printf("║  Iterations: %lu\n", model->total_iterations);
    printf("║  Final oscillation: %.6f\n", model->final_oscillation_amplitude);
    printf("║  Validation: %s\n", 
           (euler_valid && coprime_valid && geometry_valid) ? "PASSED ✓" : "FAILED ❌");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    
    // Cleanup
    platonic_model_free(model);
    
    return converged ? 0 : 1;
}