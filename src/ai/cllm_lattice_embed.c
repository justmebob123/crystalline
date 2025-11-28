/*
 * CLLM Lattice Embeddings
 * Generates embeddings based on prime lattice structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/clock_lattice.h"
#include "../include/prime_float_math.h"
#include "../include/prime_math.h"

#define PI 3.14159265358979323846
// Use PHI from prime_types.h
#define SYMMETRY_ORDER 12

/**
 * Check if a number is prime (simple trial division)
 * 
 * @param n Number to check
 * @return 1 if prime, 0 otherwise
 */
static int is_prime(uint64_t n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    
    uint64_t sqrt_n = (uint64_t)prime_sqrt((double)n);
    for (uint64_t i = 3; i <= sqrt_n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

/**
 * Get the nth prime number
 * 
 * @param n Index (0-based)
 * @return The nth prime number
 */
static uint64_t get_nth_prime(uint32_t n) {
    if (n == 0) return 2;
    
    uint64_t count = 1;  // Start with 2 as first prime
    uint64_t candidate = 3;
    
    while (count <= n) {
        if (is_prime(candidate)) {
            if (count == n) return candidate;
            count++;
        }
        candidate += 2;
    }
    
    return candidate;
}

/**
 * Compute spiral position for a prime number
 * Uses Ulam spiral mapping
 * 
 * @param prime Prime number
 * @param angle Output: spiral angle in radians
 * @param radius Output: radial distance from center
 */
void cllm_compute_spiral_position(uint64_t prime, float* angle, float* radius) {
    if (!angle || !radius) return;
    
    // Find prime index
    uint32_t prime_index = 0;
    uint64_t p = 2;
    while (p < prime) {
        if (is_prime(p)) prime_index++;
        p++;
    }
    
    // Ulam spiral: radius grows with square root of index
    *radius = prime_sqrt((float)prime_index);
    
    // Angle based on golden angle for optimal packing
    // Golden angle = 2π / φ² ≈ 137.5°
    float golden_angle = 2.0f * PI / (PHI * PHI);
    *angle = golden_angle * (float)prime_index;
    
    // Normalize angle to [0, 2π)
    while (*angle >= 2.0f * PI) {
        *angle -= 2.0f * PI;
    }
}

/**
 * Map token to 3D lattice coordinates
 * 
 * @param token_id Token ID
 * @param prime Associated prime number
 * @param coords Output: 3D coordinates [x, y, z]
 */
void cllm_map_token_to_lattice(uint32_t token_id, uint64_t prime, float* coords) {
    if (!coords) return;
    
    // Use clock-based mapping instead of spiral
    // prime_index = token_id + 1 (1-based indexing for Babylonian structure)
    map_token_to_clock_lattice(token_id, prime, token_id + 1, coords);
}

/**
 * Compute symmetry group for a prime
 * 
 * @param prime Prime number
 * @return Symmetry group (0 to SYMMETRY_ORDER-1)
 */
static uint32_t cllm_compute_symmetry_group_internal(uint64_t prime) {
    // NOTE: Cannot use prime % 12 because primes > 3 are only congruent to 1, 5, 7, 11 (mod 12)
    // This would leave 8 out of 12 worker threads idle!
    // Instead, we use a hash of the prime to distribute evenly across all 12 groups
    return (uint32_t)((prime * 2654435761ULL) % SYMMETRY_ORDER);
}

/**
 * Generate lattice-based embedding for a token
 * 
 * @param token_id Token ID
 * @param prime Associated prime number
 * @param embedding_dim Embedding dimension
 * @param output Output embedding vector [embedding_dim]
 */
void cllm_generate_lattice_embedding(uint32_t token_id, uint64_t prime,
                                     uint32_t embedding_dim, float* output) {
    if (!output || embedding_dim == 0) return;
    
    // Get lattice coordinates
    float coords[3];
    cllm_map_token_to_lattice(token_id, prime, coords);
    
    // Get symmetry group
    uint32_t symmetry = cllm_compute_symmetry_group_internal(prime);
    
    // Generate embedding using Fourier features
    // This creates a smooth, continuous embedding space
    for (uint32_t i = 0; i < embedding_dim; i++) {
        float freq = (float)(i + 1);
        
        // Combine spatial coordinates with different frequencies
        float spatial = prime_sin(freq * coords[0] / 10.0f) * 0.3f +
                       prime_cos(freq * coords[1] / 10.0f) * 0.3f +
                       prime_sin(freq * coords[2] / 10.0f) * 0.3f;
        
        // Add symmetry-based component
        float symmetry_phase = 2.0f * PI * (float)symmetry / (float)SYMMETRY_ORDER;
        float symmetry_component = prime_cos(freq * symmetry_phase) * 0.1f;
        
        output[i] = spatial + symmetry_component;
    }
    
    // Normalize to unit length
    float norm = 0.0f;
    for (uint32_t i = 0; i < embedding_dim; i++) {
        norm += output[i] * output[i];
    }
    norm = prime_sqrt(norm);
    
    if (norm > 1e-8f) {
        for (uint32_t i = 0; i < embedding_dim; i++) {
            output[i] /= norm;
        }
    }
}

/**
 * Generate lattice embeddings for entire vocabulary
 * 
 * @param model CLLM model
 */
void cllm_generate_lattice_embeddings(CLLMModel* model) {
    if (!model || !model->embeddings.embeddings) return;
    
    uint32_t vocab_size = model->embeddings.vocab_size;
    uint32_t embedding_dim = model->embeddings.embedding_dim;
    
    printf("Generating lattice embeddings for %u tokens...\n", vocab_size);
    
    for (uint32_t token_id = 0; token_id < vocab_size; token_id++) {
        // Get or assign prime for this token
        uint64_t prime = get_nth_prime(token_id);
        
        // Generate lattice-based embedding
        float* embedding = &model->embeddings.embeddings[token_id * embedding_dim];
        cllm_generate_lattice_embedding(token_id, prime, embedding_dim, embedding);
        
        // Store prime encoding in token metadata if available
        if (model->tokens && token_id < model->vocab_size) {
            model->tokens[token_id].prime_encoding = prime;
            
            // Store lattice coordinates
            float coords[3];
            cllm_map_token_to_lattice(token_id, prime, coords);
            model->tokens[token_id].lattice_coords[0] = coords[0];
            model->tokens[token_id].lattice_coords[1] = coords[1];
            model->tokens[token_id].lattice_coords[2] = coords[2];
            
            // Store symmetry group
            // Store symmetry group - use token_id for even distribution
            // (prime % 12 only gives 1,5,7,11 for primes > 3, leaving 8 threads idle)
            model->tokens[token_id].symmetry_group = token_id % SYMMETRY_ORDER;
        }
        
        if ((token_id + 1) % 1000 == 0) {
            printf("  Generated %u/%u embeddings\n", token_id + 1, vocab_size);
        }
    }
    
    printf("Lattice embeddings generation complete!\n");
}

/**
 * Generate lattice transformation matrix
 * Creates a rotation matrix based on golden ratio and prime structure
 * 
 * @param transform Output transformation matrix [dim x dim]
 * @param dim Embedding dimension
 */
void cllm_generate_lattice_transform(float* transform, int dim) {
    if (!transform || dim <= 0) return;
    
    // Initialize to identity
    memset(transform, 0, dim * dim * sizeof(float));
    for (int i = 0; i < dim; i++) {
        transform[i * dim + i] = 1.0f;
    }
    
    // Apply golden ratio-based rotations
    // This creates a transformation that preserves lattice structure
    for (int i = 0; i < dim - 1; i++) {
        float angle = 2.0f * PI * PHI * (float)i / (float)dim;
        float cos_a = prime_cos(angle);
        float sin_a = prime_sin(angle);
        
        // Apply Givens rotation in plane (i, i+1)
        float temp_ii = transform[i * dim + i];
        float temp_i_ip1 = transform[i * dim + (i + 1)];
        float temp_ip1_i = transform[(i + 1) * dim + i];
        float temp_ip1_ip1 = transform[(i + 1) * dim + (i + 1)];
        
        transform[i * dim + i] = cos_a * temp_ii - sin_a * temp_ip1_i;
        transform[i * dim + (i + 1)] = cos_a * temp_i_ip1 - sin_a * temp_ip1_ip1;
        transform[(i + 1) * dim + i] = sin_a * temp_ii + cos_a * temp_ip1_i;
        transform[(i + 1) * dim + (i + 1)] = sin_a * temp_i_ip1 + cos_a * temp_ip1_ip1;
    }
}

/**
 * Compute lattice distance between two tokens
 * 
 * @param token1_id First token ID
 * @param prime1 First token's prime
 * @param token2_id Second token ID
 * @param prime2 Second token's prime
 * @return Euclidean distance in lattice space
 */
float cllm_lattice_token_distance(uint32_t token1_id, uint64_t prime1,
                           uint32_t token2_id, uint64_t prime2) {
    float coords1[3], coords2[3];
    
    cllm_map_token_to_lattice(token1_id, prime1, coords1);
    cllm_map_token_to_lattice(token2_id, prime2, coords2);
    
    float dx = coords1[0] - coords2[0];
    float dy = coords1[1] - coords2[1];
    float dz = coords1[2] - coords2[2];
    
    return prime_sqrt(dx * dx + dy * dy + dz * dz);
}

/**
 * Find nearest neighbors in lattice space
 * 
 * @param token_id Query token ID
 * @param prime Query token's prime
 * @param all_tokens Array of all token IDs
 * @param all_primes Array of all primes
 * @param num_tokens Total number of tokens
 * @param k Number of neighbors to find
 * @param neighbors Output: k nearest neighbor token IDs
 */
void cllm_find_lattice_neighbors(uint32_t token_id, uint64_t prime,
                                uint32_t* all_tokens, uint64_t* all_primes,
                                uint32_t num_tokens, int k, uint32_t* neighbors) {
    if (!all_tokens || !all_primes || !neighbors || k <= 0) return;
    
    // Allocate distance array
    float* distances = (float*)malloc(num_tokens * sizeof(float));
    if (!distances) return;
    
    // Compute distances to all tokens
    for (uint32_t i = 0; i < num_tokens; i++) {
        if (all_tokens[i] == token_id) {
            distances[i] = 1e9f;  // Exclude self
        } else {
            distances[i] = cllm_lattice_token_distance(token_id, prime,
                                                all_tokens[i], all_primes[i]);
        }
    }
    
    // Find k smallest distances (simple selection)
    for (int i = 0; i < k && i < (int)num_tokens; i++) {
        int min_idx = 0;
        float min_dist = distances[0];
        
        for (uint32_t j = 1; j < num_tokens; j++) {
            if (distances[j] < min_dist) {
                min_dist = distances[j];
                min_idx = j;
            }
        }
        
        neighbors[i] = all_tokens[min_idx];
        distances[min_idx] = 1e9f;  // Mark as used
    }
    
    free(distances);
}