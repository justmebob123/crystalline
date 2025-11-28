/**
 * CLLM Kissing Spheres Initialization and Processing
 * 
 * Implements OBJECTIVE 16: Initialize and Process 12 Kissing Sphere Neighbors
 * 
 * Each lattice point should have exactly 12 neighbors (one per symmetry group),
 * forming a kissing spheres configuration. Neighbors are processed using the
 * L(n,d,k,λ) formula for interaction strength.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "../include/cllm.h"
#include "../include/cllm_angular_position.h"
#include "../include/cllm_mathematical_constants.h"
#include "../include/prime_lattice_core.h"
#include "../include/prime_float_math.h"

/**
 * Compute lattice distance between two tokens using angular positions
 * 
 * @param model CLLM model
 * @param token_id1 First token ID
 * @param token_id2 Second token ID
 * @return Distance in angular space
 */
static double compute_lattice_distance(CLLMModel* model, uint32_t token_id1, uint32_t token_id2) {
    if (!model || token_id1 >= model->vocab_size || token_id2 >= model->vocab_size) {
        return DBL_MAX;
    }
    
    CLLMToken* token1 = &model->tokens[token_id1];
    CLLMToken* token2 = &model->tokens[token_id2];
    
    // Compute angular positions
    AngularPosition pos1, pos2;
    
    double wavelength1 = get_phonetic_wavelength(token1->token_str[0]);
    angular_position_calculate(token1->prime_encoding, token_id1, 0, wavelength1, &pos1);
    
    double wavelength2 = get_phonetic_wavelength(token2->token_str[0]);
    angular_position_calculate(token2->prime_encoding, token_id2, 0, wavelength2, &pos2);
    
    // Euclidean distance in angular space
    double dtheta = pos1.theta - pos2.theta;
    double dr = pos1.distance_to_144000 - pos2.distance_to_144000;
    
    return prime_sqrt(dtheta * dtheta + dr * dr);
}

/**
 * Find nearest token in a specific symmetry group
 * 
 * @param model CLLM model
 * @param point_id Reference point ID
 * @param target_group Target symmetry group (0-11)
 * @return Token ID of nearest token in target group, or UINT32_MAX if not found
 */
static uint32_t find_nearest_in_group(CLLMModel* model, uint32_t point_id, uint32_t target_group) {
    if (!model || point_id >= model->vocab_size) {
        return UINT32_MAX;
    }
    
    uint32_t nearest_id = UINT32_MAX;
    double min_distance = DBL_MAX;
    
    for (uint32_t candidate = 0; candidate < model->vocab_size; candidate++) {
        if (candidate == point_id) continue;
        
        CLLMToken* cand_token = &model->tokens[candidate];
        if (cand_token->symmetry_group != target_group) continue;
        
        double distance = compute_lattice_distance(model, point_id, candidate);
        
        if (distance < min_distance) {
            min_distance = distance;
            nearest_id = candidate;
        }
    }
    
    return nearest_id;
}

/**
 * Initialize kissing spheres (12 neighbors per point)
 * 
 * Each lattice point gets exactly 12 neighbors, one from each symmetry group.
 * This creates the kissing spheres configuration.
 * 
 * @param model CLLM model with lattice points
 */
void cllm_initialize_kissing_spheres(CLLMModel* model) {
    if (!model || !model->lattice_points) {
        fprintf(stderr, "ERROR: Invalid model or no lattice points\n");
        return;
    }
    
    printf("\n=== Initializing Kissing Spheres (12 neighbors per point) ===\n");
    printf("Number of lattice points: %lu\n", (unsigned long)model->num_lattice_points);
    printf("Finding nearest neighbor in each symmetry group...\n\n");
    
    uint32_t total_neighbors = 0;
    uint32_t points_with_full_neighbors = 0;
    
    for (uint32_t point_id = 0; point_id < model->num_lattice_points; point_id++) {
        CLLMLatticePoint* point = &model->lattice_points[point_id];
        
        // Reset neighbor count
        point->num_neighbors = 0;
        
        // Find nearest neighbor in each of the 12 symmetry groups
        for (uint32_t group = 0; group < 12; group++) {
            uint32_t nearest_id = find_nearest_in_group(model, point_id, group);
            
            if (nearest_id != UINT32_MAX) {
                point->neighbors[point->num_neighbors] = nearest_id;
                point->num_neighbors++;
                total_neighbors++;
            }
        }
        
        if (point->num_neighbors == 12) {
            points_with_full_neighbors++;
        }
        
        // Progress indicator
        if ((point_id + 1) % 100 == 0 || point_id == model->num_lattice_points - 1) {
            printf("  Initialized %lu/%lu points (%.1f%%)\r", 
                   (unsigned long)(point_id + 1), 
                   (unsigned long)model->num_lattice_points,
                   100.0 * (point_id + 1) / model->num_lattice_points);
            fflush(stdout);
        }
    }
    
    printf("\n\n");
    
    // Statistics
    printf("=== Kissing Spheres Statistics ===\n");
    printf("Total neighbors assigned: %u\n", total_neighbors);
    printf("Average neighbors per point: %.2f\n", 
           (double)total_neighbors / model->num_lattice_points);
    printf("Points with 12 neighbors: %u (%.1f%%)\n",
           points_with_full_neighbors,
           100.0 * points_with_full_neighbors / model->num_lattice_points);
    
    if (points_with_full_neighbors == model->num_lattice_points) {
        printf("✓ All points have complete kissing spheres configuration\n");
    } else {
        printf("⚠ Some points have incomplete neighbor sets\n");
    }
    
    printf("\n");
}

/**
 * Process lattice point with its 12 kissing sphere neighbors
 * 
 * Uses L(n,d,k,λ) formula to compute interaction strength with each neighbor.
 * Applies interactions to gradients.
 * 
 * @param point Lattice point to process
 * @param model CLLM model
 * @param local_gradients Gradient buffer [vocab_size x embedding_dim]
 * @param gradient_size Total gradient buffer size
 */
void cllm_process_kissing_spheres(CLLMLatticePoint* point,
                                  CLLMModel* model,
                                  float* local_gradients,
                                  size_t gradient_size) {
    if (!point || !model || !local_gradients) return;
    
    uint32_t center_id = point->point_id;
    if (center_id >= model->vocab_size) return;
    
    CLLMToken* center_token = &model->tokens[center_id];
    uint32_t embedding_dim = model->embeddings.embedding_dim;
    
    // Process each of the 12 kissing sphere neighbors
    for (uint32_t i = 0; i < point->num_neighbors; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        if (neighbor_id >= model->vocab_size) continue;
        
        CLLMToken* neighbor_token = &model->tokens[neighbor_id];
        (void)neighbor_token;  // Reserved for future use in interaction computation
        
        // Get dimensional frequency φᵢ for this neighbor
        uint64_t phi_i = cllm_get_dimensional_frequency(i);
        
        // Compute interaction strength using L(n,d,k,λ)
        // Use phi_i as the 'q' parameter for dimensional frequency modulation
        double interaction = L_lattice(
            center_token->prime_encoding,     // n: center prime
            i,                                // d: neighbor index (0-11)
            center_token->symmetry_group,     // k: symmetry group
            center_token->token_str,          // λ: phonetic layer
            3,                                // ω: Einstein's Λ
            center_token->prime_encoding,     // p: for Plimpton
            phi_i                             // q: dimensional frequency
        );
        
        // Normalize interaction strength to reasonable range
        double normalized_interaction = prime_tanh(interaction / 10.0);
        
        // Apply interaction to gradients
        size_t center_offset = center_id * embedding_dim;
        size_t neighbor_offset = neighbor_id * embedding_dim;
        
        for (uint32_t dim = 0; dim < embedding_dim; dim++) {
            if (center_offset + dim >= gradient_size) break;
            if (neighbor_offset + dim >= gradient_size) break;
            
            // Get neighbor's embedding value
            float neighbor_value = model->embeddings.embeddings[neighbor_offset + dim];
            
            // Add weighted contribution to gradient (10% neighbor influence)
            local_gradients[center_offset + dim] += 
                (float)normalized_interaction * neighbor_value * 0.1f;
        }
    }
}

/**
 * Verify kissing spheres configuration
 * 
 * Checks that all points have proper neighbor assignments.
 * 
 * @param model CLLM model
 * @return 1 if valid, 0 if issues found
 */
int cllm_verify_kissing_spheres(CLLMModel* model) {
    if (!model || !model->lattice_points) {
        fprintf(stderr, "ERROR: Invalid model or no lattice points\n");
        return 0;
    }
    
    printf("\n=== Verifying Kissing Spheres Configuration ===\n");
    
    int issues_found = 0;
    int group_distribution[12] = {0};
    
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        CLLMLatticePoint* point = &model->lattice_points[i];
        
        // Check number of neighbors
        if (point->num_neighbors < 10 || point->num_neighbors > 12) {
            printf("WARNING: Point %u has %u neighbors (expected 10-12)\n",
                   i, point->num_neighbors);
            issues_found++;
        }
        
        // Check neighbor symmetry groups
        int neighbor_groups[12] = {0};
        for (uint32_t j = 0; j < point->num_neighbors; j++) {
            uint32_t neighbor_id = point->neighbors[j];
            if (neighbor_id >= model->vocab_size) {
                printf("WARNING: Point %u has invalid neighbor ID %u\n", i, neighbor_id);
                issues_found++;
                continue;
            }
            
            uint32_t group = model->tokens[neighbor_id].symmetry_group;
            neighbor_groups[group]++;
            group_distribution[group]++;
        }
        
        // Verify each group has at most 1 neighbor
        for (int g = 0; g < 12; g++) {
            if (neighbor_groups[g] > 1) {
                printf("WARNING: Point %u has %d neighbors in group %d (expected 0-1)\n",
                       i, neighbor_groups[g], g);
                issues_found++;
            }
        }
    }
    
    printf("\nSymmetry group distribution (total neighbors):\n");
    for (int g = 0; g < 12; g++) {
        printf("  Group %2d: %5d neighbors\n", g, group_distribution[g]);
    }
    
    if (issues_found == 0) {
        printf("\n✓ Kissing spheres configuration is valid\n");
        return 1;
    } else {
        printf("\n⚠ Found %d issues in kissing spheres configuration\n", issues_found);
        return 0;
    }
}

/**
 * Get statistics about kissing spheres configuration
 * 
 * @param model CLLM model
 */
void cllm_kissing_spheres_stats(CLLMModel* model) {
    if (!model || !model->lattice_points) return;
    
    printf("\n=== Kissing Spheres Statistics ===\n");
    
    uint32_t min_neighbors = UINT32_MAX;
    uint32_t max_neighbors = 0;
    uint64_t total_neighbors = 0;
    
    int neighbor_histogram[13] = {0};  // 0-12 neighbors
    
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        CLLMLatticePoint* point = &model->lattice_points[i];
        uint32_t n = point->num_neighbors;
        
        if (n < min_neighbors) min_neighbors = n;
        if (n > max_neighbors) max_neighbors = n;
        total_neighbors += n;
        
        if (n <= 12) {
            neighbor_histogram[n]++;
        }
    }
    
    double avg_neighbors = (double)total_neighbors / model->num_lattice_points;
    
    printf("Total lattice points: %lu\n", (unsigned long)model->num_lattice_points);
    printf("Total neighbors: %lu\n", (unsigned long)total_neighbors);
    printf("Average neighbors per point: %.2f\n", avg_neighbors);
    printf("Min neighbors: %u\n", min_neighbors);
    printf("Max neighbors: %u\n", max_neighbors);
    
    printf("\nNeighbor count distribution:\n");
    for (int n = 0; n <= 12; n++) {
        if (neighbor_histogram[n] > 0) {
            printf("  %2d neighbors: %5d points (%.1f%%)\n",
                   n, neighbor_histogram[n],
                   100.0 * neighbor_histogram[n] / model->num_lattice_points);
        }
    }
    
    printf("\n");
}