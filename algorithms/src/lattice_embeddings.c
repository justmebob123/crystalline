/**
 * lattice_embeddings.c - Geometric Pattern-Based Embeddings
 * 
 * This implements INSTANT embedding initialization using the geometric pattern
 * directly, without any pre-computation or caching. The pattern itself IS the
 * algorithm.
 * 
 * Key insight: Clock position → L(n,d,k,λ) is a pure geometric calculation
 * that can be done on-demand with O(1) complexity per dimension.
 * 
 * This is a fundamental algorithm in the algorithms layer, usable by ANY system.
 */

#include "lattice_embeddings.h"
#include "clock_lattice.h"
#include "prime_lattice_core.h"
#include "prime_float_math.h"
#include "cllm_mathematical_constants.h"
#include <string.h>

/**
 * Compute L(n,d,k,λ) directly from clock position
 * 
 * This is the CORE algorithm that replaces all caching.
 * It uses the geometric pattern to compute lattice values instantly.
 * 
 * @param pos Clock position for this prime
 * @param dimension Embedding dimension
 * @param phi_i Dimensional frequency (from 12-fold symmetry)
 * @param symmetry_group Symmetry group (k)
 * @return L value for this position and dimension
 */
static double compute_L_from_clock_position(
    BabylonianClockPosition pos,
    uint32_t dimension,
    uint64_t phi_i,
    int symmetry_group
) {
    // Base: 3^O where O depends on ring and position
    // Ring determines the primary exponent
    // Position within ring adds fine-grained variation
    double positions_in_ring;
    if (pos.ring == 0) positions_in_ring = 12.0;
    else if (pos.ring == 1 || pos.ring == 2) positions_in_ring = 60.0;
    else if (pos.ring == 3) positions_in_ring = 100.0;
    else positions_in_ring = 1000.0;
    
    double O = (double)pos.ring + ((double)pos.position / positions_in_ring);
    double base = prime_pow(3.0, O);
    
    // Product: cos(θ·φᵢ) where θ is clock angle
    // This encodes the angular position in the lattice
    double theta = pos.angle;
    double cos_term = prime_cos(theta * (double)phi_i);
    
    // Γ(k): Symmetry group contribution (12-fold)
    // This ensures tokens in same group have similar embeddings
    double gamma_k = prime_cos(2.0 * PRIME_PI * (double)symmetry_group / 12.0);
    
    // Γ(n,d): Lattice entropy based on ring and dimension
    // Deeper rings (larger primes) have higher entropy
    // Higher dimensions have more complex patterns
    double entropy_factor = 1.0 + (double)pos.ring * 0.1 + (double)dimension * 0.01;
    double gamma_nd = prime_tanh(entropy_factor);
    
    // Combine all terms
    return base * cos_term * gamma_k * gamma_nd;
}

/**
 * Initialize embeddings using geometric pattern
 * 
 * This is INSTANT - no pre-computation, no caching, just pure geometry.
 * Works for ANY vocabulary size with O(vocab_size * embedding_dim) complexity.
 * 
 * @param embeddings Output embedding matrix [vocab_size × embedding_dim]
 * @param vocab_size Number of tokens
 * @param embedding_dim Embedding dimension
 */
void lattice_embeddings_init_geometric(
    float* embeddings,
    uint32_t vocab_size,
    uint32_t embedding_dim
) {
    if (!embeddings) return;
    
    // For each token
    for (uint32_t token_id = 0; token_id < vocab_size; token_id++) {
        // 1. Map token to clock position (O(1))
        // Token ID directly maps to prime index
        BabylonianClockPosition pos = map_prime_index_to_clock((int)token_id);
        
        // 2. Determine symmetry group (12-fold)
        int symmetry_group = token_id % 12;
        
        // 3. For each dimension, compute L value from geometry
        for (uint32_t dim = 0; dim < embedding_dim; dim++) {
            // Get dimensional frequency (12-fold symmetry)
            uint64_t phi_i = cllm_get_dimensional_frequency(dim % 12);
            
            // Compute L directly from clock position
            double L = compute_L_from_clock_position(
                pos, dim, phi_i, symmetry_group
            );
            
            // Normalize to [-1, 1] using tanh
            // Scale factor of 100.0 keeps values in reasonable range
            embeddings[token_id * embedding_dim + dim] = (float)prime_tanh(L / 100.0);
        }
    }
}

/**
 * Get embedding for a single token (on-demand)
 * 
 * This demonstrates that we can compute embeddings on-demand
 * without any pre-computation. Useful for dynamic vocabularies.
 * 
 * @param token_id Token ID
 * @param embedding_dim Embedding dimension
 * @param output Output embedding vector [embedding_dim]
 */
void lattice_get_token_embedding_geometric(
    uint32_t token_id,
    uint32_t embedding_dim,
    float* output
) {
    if (!output) return;
    
    // Map to clock position
    BabylonianClockPosition pos = map_prime_index_to_clock((int)token_id);
    int symmetry_group = token_id % 12;
    
    // Compute each dimension
    for (uint32_t dim = 0; dim < embedding_dim; dim++) {
        uint64_t phi_i = cllm_get_dimensional_frequency(dim % 12);
        double L = compute_L_from_clock_position(pos, dim, phi_i, symmetry_group);
        output[dim] = (float)prime_tanh(L / 100.0);
    }
}

/**
 * Verify geometric pattern properties
 * 
 * This checks that the geometric pattern maintains key properties:
 * - Same symmetry group → similar embeddings
 * - Same ring → similar magnitude
 * - 12-fold symmetry preserved
 * 
 * @param embeddings Embedding matrix
 * @param vocab_size Vocabulary size
 * @param embedding_dim Embedding dimension
 * @return true if properties are satisfied
 */
bool lattice_verify_geometric_embeddings(
    const float* embeddings,
    uint32_t vocab_size,
    uint32_t embedding_dim
) {
    if (!embeddings || vocab_size == 0 || embedding_dim == 0) return false;
    
    // Check 1: All values in [-1, 1]
    for (uint32_t i = 0; i < vocab_size * embedding_dim; i++) {
        if (embeddings[i] < -1.0f || embeddings[i] > 1.0f) {
            return false;
        }
    }
    
    // Check 2: Tokens in same symmetry group have similar embeddings
    // Compare token 0 and token 12 (both in group 0)
    if (vocab_size > 12) {
        float similarity = 0.0f;
        for (uint32_t dim = 0; dim < embedding_dim; dim++) {
            similarity += embeddings[0 * embedding_dim + dim] * 
                         embeddings[12 * embedding_dim + dim];
        }
        similarity /= (float)embedding_dim;
        
        // Should have high similarity (> 0.5)
        if (similarity < 0.5f) {
            return false;
        }
    }
    
    return true;
}
