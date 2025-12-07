/**
 * @file cllm_qk_mapping.c
 * @brief Implementation of Q→k mapping for AI inference
 */

#include "ai/cllm_qk_mapping.h"
#include "prime_math.h"
#include "prime_basic.h"
#include "prime_math_custom.h"
#include <stdlib.h>
#include <string.h>

// Golden ratio constant (use system-defined PHI from prime_types.h)

QtoK_Mapping* prime_qk_mapper_create(size_t Q_dim, size_t num_candidates) {
    if (Q_dim == 0 || num_candidates == 0) {
        return NULL;
    }
    
    QtoK_Mapping* mapper = (QtoK_Mapping*)calloc(1, sizeof(QtoK_Mapping));
    if (!mapper) {
        return NULL;
    }
    
    mapper->Q_dim = Q_dim;
    mapper->num_candidates = num_candidates;
    mapper->p_mod = 13; // Default prime modulus
    
    mapper->Q_vector = (double*)calloc(Q_dim, sizeof(double));
    mapper->k_candidates = (uint64_t*)calloc(num_candidates, sizeof(uint64_t));
    mapper->distances = (double*)calloc(num_candidates, sizeof(double));
    
    if (!mapper->Q_vector || !mapper->k_candidates || !mapper->distances) {
        prime_qk_mapper_destroy(mapper);
        return NULL;
    }
    
    return mapper;
}

void prime_qk_mapper_destroy(QtoK_Mapping* mapper) {
    if (!mapper) {
        return;
    }
    
    free(mapper->Q_vector);
    free(mapper->k_candidates);
    free(mapper->distances);
    free(mapper);
}

double prime_tensor_distance(const double* Q1, const double* Q2,
                              size_t dim, uint64_t p_mod) {
    if (!Q1 || !Q2 || dim < 2) {
        return 0.0;
    }
    
    // Calculate dx = Q1[0] - Q2[0], dy = Q1[1] - Q2[1]
    double dx = prime_subtract(Q1[0], Q2[0]);
    double dy = prime_subtract(Q1[1], Q2[1]);
    
    // Modular reduction
    dx = prime_fmod(dx, (double)p_mod);
    dy = prime_fmod(dy, (double)p_mod);
    
    // Golden ratio geometry: g = (φ, φ²)
    double gx = prime_fmod(PHI, (double)p_mod);
    double gy = prime_fmod(prime_multiply(PHI, PHI), (double)p_mod);
    
    // Determinant: dx * gy - dy * gx
    double term1 = prime_multiply(dx, gy);
    double term2 = prime_multiply(dy, gx);
    double det = prime_subtract(term1, term2);
    
    // Final modular reduction
    return prime_fmod(det, (double)p_mod);
}

uint64_t prime_qk_find_best(const QtoK_Mapping* mapper,
                             const double* Q_query) {
    if (!mapper || !Q_query) {
        return 0;
    }
    
    double min_distance = 1e308; // Large value
    uint64_t best_k = 0;
    
    for (size_t i = 0; i < mapper->num_candidates; i++) {
        // Create candidate Q vector from k
        // Map k to 2D space using modular arithmetic
        double Q_candidate[2];
        Q_candidate[0] = (double)(mapper->k_candidates[i] % 1000000);
        Q_candidate[1] = (double)(mapper->k_candidates[i] / 1000000);
        
        // Calculate tensor distance
        double dist = prime_tensor_distance(Q_query, Q_candidate,
                                             mapper->Q_dim, mapper->p_mod);
        
        // Track minimum
        if (dist < min_distance) {
            min_distance = dist;
            best_k = mapper->k_candidates[i];
        }
    }
    
    return best_k;
}

size_t prime_qk_find_top_n(const QtoK_Mapping* mapper,
                            const double* Q_query,
                            uint64_t* top_k, size_t N) {
    if (!mapper || !Q_query || !top_k || N == 0) {
        return 0;
    }
    
    // Calculate distances for all candidates
    for (size_t i = 0; i < mapper->num_candidates; i++) {
        double Q_candidate[2];
        Q_candidate[0] = (double)(mapper->k_candidates[i] % 1000000);
        Q_candidate[1] = (double)(mapper->k_candidates[i] / 1000000);
        
        mapper->distances[i] = prime_tensor_distance(Q_query, Q_candidate,
                                                      mapper->Q_dim,
                                                      mapper->p_mod);
    }
    
    // Find top N with smallest distances (simple selection sort)
    size_t found = (N < mapper->num_candidates) ? N : mapper->num_candidates;
    
    for (size_t i = 0; i < found; i++) {
        size_t min_idx = i;
        double min_dist = mapper->distances[i];
        
        for (size_t j = i + 1; j < mapper->num_candidates; j++) {
            if (mapper->distances[j] < min_dist) {
                min_dist = mapper->distances[j];
                min_idx = j;
            }
        }
        
        // Swap
        if (min_idx != i) {
            double temp_dist = mapper->distances[i];
            mapper->distances[i] = mapper->distances[min_idx];
            mapper->distances[min_idx] = temp_dist;
            
            uint64_t temp_k = mapper->k_candidates[i];
            mapper->k_candidates[i] = mapper->k_candidates[min_idx];
            mapper->k_candidates[min_idx] = temp_k;
        }
        
        top_k[i] = mapper->k_candidates[i];
    }
    
    return found;
}

bool prime_qk_set_candidates(QtoK_Mapping* mapper,
                              const uint64_t* candidates,
                              size_t num_candidates) {
    if (!mapper || !candidates || num_candidates == 0) {
        return false;
    }
    
    if (num_candidates > mapper->num_candidates) {
        // Reallocate if needed
        uint64_t* new_candidates = (uint64_t*)realloc(mapper->k_candidates,
                                                       num_candidates * sizeof(uint64_t));
        double* new_distances = (double*)realloc(mapper->distances,
                                                  num_candidates * sizeof(double));
        
        if (!new_candidates || !new_distances) {
            return false;
        }
        
        mapper->k_candidates = new_candidates;
        mapper->distances = new_distances;
        mapper->num_candidates = num_candidates;
    }
    
    memcpy(mapper->k_candidates, candidates, num_candidates * sizeof(uint64_t));
    return true;
}

void prime_qk_generate_anchors(double* anchors, size_t num_anchors,
                                size_t dim, uint64_t seed) {
    if (!anchors || num_anchors == 0 || dim == 0) {
        return;
    }
    
    // Simple PRNG for anchor generation
    uint64_t state = seed;
    
    for (size_t i = 0; i < num_anchors; i++) {
        for (size_t d = 0; d < dim; d++) {
            // Linear congruential generator
            state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
            
            // Normalize to [0, 1] and scale by golden ratio
            double rand_val = prime_divide((double)(state % 10000), 10000.0);
            anchors[i * dim + d] = prime_multiply(rand_val, PHI);
        }
    }
}

uint64_t prime_qk_estimate_from_anchors(const double* Q,
                                         const double* anchors,
                                         size_t num_anchors,
                                         size_t dim,
                                         uint64_t p_mod) {
    if (!Q || !anchors || num_anchors == 0 || dim == 0) {
        return 0;
    }
    
    // Calculate distances to all anchors
    double total_distance = 0.0;
    
    for (size_t i = 0; i < num_anchors; i++) {
        const double* anchor = &anchors[i * dim];
        double dist = prime_tensor_distance(Q, anchor, dim, p_mod);
        total_distance = prime_add(total_distance, dist);
    }
    
    // Average distance as estimate
    double avg_distance = prime_divide(total_distance, (double)num_anchors);
    
    // Convert to k estimate (modular)
    uint64_t k_estimate = (uint64_t)avg_distance % 1000000;
    
    return k_estimate;
}