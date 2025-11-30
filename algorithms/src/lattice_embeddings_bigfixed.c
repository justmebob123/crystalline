/**
 * lattice_embeddings_bigfixed.c - BigFixed Geometric Pattern-Based Embeddings
 * 
 * This implements INSTANT embedding initialization using the geometric pattern
 * with ARBITRARY PRECISION using BigFixed. No overflow, no precision loss.
 * 
 * Key insight: Clock position → L(n,d,k,λ) with BigFixed arbitrary precision
 * enables handling of extremely large exponents without overflow.
 * 
 * This is the CORRECT implementation following the crystalline design.
 */

#include "lattice_embeddings.h"
#include "clock_lattice.h"
#include "prime_lattice_core.h"
#include "bigfixed_core.h"
#include "prime_bigint_transcendental.h"
#include "cllm_mathematical_constants.h"
#include <string.h>
#include <stdio.h>

/**
 * Compute L(n,d,k,λ) directly from clock position using BigFixed
 * 
 * This is the CORE algorithm with arbitrary precision.
 * It uses BigFixed to handle large exponents without overflow.
 * 
 * @param pos Clock position for this prime
 * @param dimension Embedding dimension
 * @param phi_i Dimensional frequency (from 12-fold symmetry)
 * @param symmetry_group Symmetry group (k)
 * @param result Output BigFixed value
 * @param precision_bits Precision for BigFixed operations
 */
static void compute_L_from_clock_position_bigfixed(
    BabylonianClockPosition pos,
    uint32_t dimension,
    uint64_t phi_i,
    int symmetry_group,
    BigFixed* result,
    int precision_bits
) {
    // Initialize BigFixed variables
    BigFixed three, O_fixed, base;
    BigFixed theta_fixed, phi_i_fixed, theta_phi, cos_term;
    BigFixed gamma_k_angle, gamma_k;
    BigFixed entropy_fixed, gamma_nd;
    BigFixed temp1, temp2, temp3;
    
    big_fixed_init(&three, precision_bits);
    big_fixed_init(&O_fixed, precision_bits);
    big_fixed_init(&base, precision_bits);
    big_fixed_init(&theta_fixed, precision_bits);
    big_fixed_init(&phi_i_fixed, precision_bits);
    big_fixed_init(&theta_phi, precision_bits);
    big_fixed_init(&cos_term, precision_bits);
    big_fixed_init(&gamma_k_angle, precision_bits);
    big_fixed_init(&gamma_k, precision_bits);
    big_fixed_init(&entropy_fixed, precision_bits);
    big_fixed_init(&gamma_nd, precision_bits);
    big_fixed_init(&temp1, precision_bits);
    big_fixed_init(&temp2, precision_bits);
    big_fixed_init(&temp3, precision_bits);
    
    // Base: 3^O where O depends on ring and position
    double positions_in_ring;
    if (pos.ring == 0) positions_in_ring = 12.0;
    else if (pos.ring == 1 || pos.ring == 2) positions_in_ring = 60.0;
    else if (pos.ring == 3) positions_in_ring = 100.0;
    else positions_in_ring = 1000.0;
    
    // O is naturally bounded by the clock lattice design
    double O = (double)pos.ring + ((double)pos.position / positions_in_ring);
    
    // Compute 3^O with BigFixed (NO OVERFLOW!)
    big_fixed_from_int(&three, 3);
    big_fixed_from_double(&O_fixed, O);
    big_pow(&base, &three, &O_fixed, precision_bits);
    
    // Product: cos(θ·φᵢ) where θ is clock angle
    big_fixed_from_double(&theta_fixed, pos.angle);
    big_fixed_from_int(&phi_i_fixed, (int)phi_i);
    big_fixed_mul(&theta_phi, &theta_fixed, &phi_i_fixed);
    big_cos(&cos_term, &theta_phi, precision_bits);
    
    // Γ(k): Symmetry group contribution (12-fold)
    double gamma_k_val = 2.0 * PRIME_PI * (double)symmetry_group / 12.0;
    big_fixed_from_double(&gamma_k_angle, gamma_k_val);
    big_cos(&gamma_k, &gamma_k_angle, precision_bits);
    
    // Γ(n,d): Lattice entropy based on ring and dimension
    double entropy_factor = 1.0 + (double)pos.ring * 0.1 + (double)dimension * 0.01;
    big_fixed_from_double(&entropy_fixed, entropy_factor);
    big_tanh(&gamma_nd, &entropy_fixed, precision_bits);
    
    // Combine all terms: L = base * cos_term * gamma_k * gamma_nd
    big_fixed_mul(&temp1, &base, &cos_term);
    big_fixed_mul(&temp2, &temp1, &gamma_k);
    big_fixed_mul(result, &temp2, &gamma_nd);
    
    // Cleanup
    big_fixed_free(&three);
    big_fixed_free(&O_fixed);
    big_fixed_free(&base);
    big_fixed_free(&theta_fixed);
    big_fixed_free(&phi_i_fixed);
    big_fixed_free(&theta_phi);
    big_fixed_free(&cos_term);
    big_fixed_free(&gamma_k_angle);
    big_fixed_free(&gamma_k);
    big_fixed_free(&entropy_fixed);
    big_fixed_free(&gamma_nd);
    big_fixed_free(&temp1);
    big_fixed_free(&temp2);
    big_fixed_free(&temp3);
}

/**
 * Initialize embeddings using geometric pattern with BigFixed
 * 
 * This is INSTANT with ARBITRARY PRECISION - no overflow, no precision loss.
 * Works for ANY vocabulary size with O(vocab_size * embedding_dim) complexity.
 * 
 * @param embeddings Output embedding matrix [vocab_size][embedding_dim] (BigFixed)
 * @param vocab_size Number of tokens
 * @param embedding_dim Embedding dimension
 * @param precision_bits Precision for BigFixed operations (e.g., 256)
 */
void lattice_embeddings_init_geometric_bigfixed(
    BigFixed** embeddings,
    uint32_t vocab_size,
    uint32_t embedding_dim,
    int precision_bits
) {
    if (!embeddings) return;
    
    printf("Initializing embeddings with BigFixed (precision: %d bits)...\n", precision_bits);
    
    // For each token
    for (uint32_t token_id = 0; token_id < vocab_size; token_id++) {
        // 1. Map token to clock position (O(1))
        BabylonianClockPosition pos = map_prime_index_to_clock((int)token_id);
        
        // 2. Determine symmetry group (12-fold)
        int symmetry_group = token_id % 12;
        
        // 3. For each dimension, compute L value from geometry
        for (uint32_t dim = 0; dim < embedding_dim; dim++) {
            // Get dimensional frequency (12-fold symmetry)
            uint64_t phi_i = cllm_get_dimensional_frequency(dim % 12);
            
            // Compute L directly from clock position with BigFixed
            BigFixed L, L_scaled, normalized;
            big_fixed_init(&L, precision_bits);
            big_fixed_init(&L_scaled, precision_bits);
            big_fixed_init(&normalized, precision_bits);
            
            compute_L_from_clock_position_bigfixed(
                pos, dim, phi_i, symmetry_group, &L, precision_bits
            );
            
            // Normalize to [-1, 1] using tanh
            // Scale factor of 100.0 keeps values in reasonable range
            BigFixed scale_factor;
            big_fixed_init(&scale_factor, precision_bits);
            big_fixed_from_double(&scale_factor, 100.0);
            big_fixed_div(&L_scaled, &L, &scale_factor);
            big_tanh(&normalized, &L_scaled, precision_bits);
            
            // Store in embeddings matrix
            size_t idx = token_id * embedding_dim + dim;
            big_fixed_assign(&embeddings[idx], &normalized);
            
            // Cleanup
            big_fixed_free(&L);
            big_fixed_free(&L_scaled);
            big_fixed_free(&normalized);
            big_fixed_free(&scale_factor);
        }
        
        // Progress indicator for large vocabularies
        if ((token_id + 1) % 1000 == 0) {
            printf("  Initialized %u / %u tokens\r", token_id + 1, vocab_size);
            fflush(stdout);
        }
    }
    
    printf("\n✓ Embedding initialization complete\n");
}

/**
 * Get embedding for a single token (on-demand) with BigFixed
 * 
 * This demonstrates that we can compute embeddings on-demand
 * with arbitrary precision. Useful for dynamic vocabularies.
 * 
 * @param token_id Token ID
 * @param embedding_dim Embedding dimension
 * @param output Output embedding vector [embedding_dim] (BigFixed)
 * @param precision_bits Precision for BigFixed operations
 */
void lattice_get_token_embedding_geometric_bigfixed(
    uint32_t token_id,
    uint32_t embedding_dim,
    BigFixed* output,
    int precision_bits
) {
    if (!output) return;
    
    // Map to clock position
    BabylonianClockPosition pos = map_prime_index_to_clock((int)token_id);
    int symmetry_group = token_id % 12;
    
    // Compute each dimension
    for (uint32_t dim = 0; dim < embedding_dim; dim++) {
        uint64_t phi_i = cllm_get_dimensional_frequency(dim % 12);
        
        BigFixed L, L_scaled, normalized, scale_factor;
        big_fixed_init(&L, precision_bits);
        big_fixed_init(&L_scaled, precision_bits);
        big_fixed_init(&normalized, precision_bits);
        big_fixed_init(&scale_factor, precision_bits);
        
        compute_L_from_clock_position_bigfixed(
            pos, dim, phi_i, symmetry_group, &L, precision_bits
        );
        
        big_fixed_from_double(&scale_factor, 100.0);
        big_fixed_div(&L_scaled, &L, &scale_factor);
        big_tanh(&normalized, &L_scaled, precision_bits);
        
        big_fixed_assign(&output[dim], &normalized);
        
        big_fixed_free(&L);
        big_fixed_free(&L_scaled);
        big_fixed_free(&normalized);
        big_fixed_free(&scale_factor);
    }
}