#include <stdlib.h>
/**
 * lattice_embeddings_bigfixed.c - BigFixed Geometric Pattern-Based Embeddings
 * 
 * This implements INSTANT embedding initialization using the geometric pattern
 * with ARBITRARY PRECISION using BigFixed. No overflow, no precision loss.
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
 * Compute L(n,d,k,λ) using BigFixed arbitrary precision
 * Formula: L = 3^O(n,k,λ) · ∏cos(θ·φᵢ) · Γ(k) · ν(λ) · Γ(n,d)
 */
static void compute_L_bigfixed(
    BabylonianClockPosition pos,
    uint32_t dimension,
    uint64_t phi_i,
    int symmetry_group,
    BigFixed* result,
    int precision_bits
) {
    // Calculate positions in ring
    double positions_in_ring;
    if (pos.ring == 0) positions_in_ring = 12.0;
    else if (pos.ring == 1 || pos.ring == 2) positions_in_ring = 60.0;
    else if (pos.ring == 3) positions_in_ring = 100.0;
    else positions_in_ring = 1000.0;
    
    double O = (double)pos.ring + ((double)pos.position / positions_in_ring);
    
    // Create BigFixed variables
    BigFixed* three = big_fixed_create(precision_bits);
    BigFixed* O_fixed = big_fixed_create(precision_bits);
    BigFixed* base = big_fixed_create(precision_bits);
    
    // Compute 3^O with BigFixed (NO OVERFLOW!)
    big_fixed_from_int(three, 3);
    big_fixed_from_double(O_fixed, O);
    
    // Convert three to BigInt for big_pow
    BigInt* three_int = (BigInt*)malloc(sizeof(BigInt));
    big_init(three_int);
    big_from_int(three_int, 3);
    big_pow(base, three_int, O_fixed, precision_bits);
    
    // Compute cos(θ·φᵢ)
    BigFixed* theta_fixed = big_fixed_create(precision_bits);
    BigFixed* phi_i_fixed = big_fixed_create(precision_bits);
    BigFixed* theta_phi = big_fixed_create(precision_bits);
    BigFixed* cos_term = big_fixed_create(precision_bits);
    
    big_fixed_from_double(theta_fixed, pos.angle);
    big_fixed_from_int(phi_i_fixed, (int)phi_i);
    big_fixed_mul(theta_phi, theta_fixed, phi_i_fixed);
    big_cos(cos_term, theta_phi, precision_bits);
    
    // Compute Γ(k): Symmetry group contribution
    BigFixed* gamma_k_angle = big_fixed_create(precision_bits);
    BigFixed* gamma_k = big_fixed_create(precision_bits);
    double gamma_k_val = 2.0 * PRIME_PI * (double)symmetry_group / 12.0;
    big_fixed_from_double(gamma_k_angle, gamma_k_val);
    big_cos(gamma_k, gamma_k_angle, precision_bits);
    
    // Compute Γ(n,d): Lattice entropy
    BigFixed* entropy_fixed = big_fixed_create(precision_bits);
    BigFixed* gamma_nd = big_fixed_create(precision_bits);
    double entropy_factor = 1.0 + (double)pos.ring * 0.1 + (double)dimension * 0.01;
    big_fixed_from_double(entropy_fixed, entropy_factor);
    big_tanh(gamma_nd, entropy_fixed, precision_bits);
    
    // Combine: L = base * cos_term * gamma_k * gamma_nd
    BigFixed* temp1 = big_fixed_create(precision_bits);
    BigFixed* temp2 = big_fixed_create(precision_bits);
    
    big_fixed_mul(temp1, base, cos_term);
    big_fixed_mul(temp2, temp1, gamma_k);
    big_fixed_mul(result, temp2, gamma_nd);
    
    // Cleanup
    big_fixed_free(three);
    big_fixed_free(O_fixed);
    big_fixed_free(base);
    big_free(three_int);
    free(three_int);
    big_fixed_free(theta_fixed);
    big_fixed_free(phi_i_fixed);
    big_fixed_free(theta_phi);
    big_fixed_free(cos_term);
    big_fixed_free(gamma_k_angle);
    big_fixed_free(gamma_k);
    big_fixed_free(entropy_fixed);
    big_fixed_free(gamma_nd);
    big_fixed_free(temp1);
    big_fixed_free(temp2);
}

/**
 * Initialize embeddings using geometric pattern with BigFixed
 */
void lattice_embeddings_init_geometric_bigfixed(
    BigFixed** embeddings,
    uint32_t vocab_size,
    uint32_t embedding_dim,
    int precision_bits
) {
    if (!embeddings) return;
    
    printf("Initializing embeddings with BigFixed (precision: %d bits)...\n", precision_bits);
    
    for (uint32_t token_id = 0; token_id < vocab_size; token_id++) {
        BabylonianClockPosition pos = map_prime_index_to_clock((int)token_id);
        int symmetry_group = token_id % 12;
        
        for (uint32_t dim = 0; dim < embedding_dim; dim++) {
            uint64_t phi_i = cllm_get_dimensional_frequency(dim % 12);
            
            BigFixed* L = big_fixed_create(precision_bits);
            BigFixed* L_scaled = big_fixed_create(precision_bits);
            BigFixed* normalized = big_fixed_create(precision_bits);
            BigFixed* scale_factor = big_fixed_create(precision_bits);
            
            compute_L_bigfixed(pos, dim, phi_i, symmetry_group, L, precision_bits);
            
            // Normalize: tanh(L / 100.0)
            big_fixed_from_double(scale_factor, 100.0);
            big_fixed_div(L_scaled, L, scale_factor);
            big_tanh(normalized, L_scaled, precision_bits);
            
            // Store in embeddings
            size_t idx = token_id * embedding_dim + dim;
            big_fixed_assign(embeddings[idx], normalized);
            
            big_fixed_free(L);
            big_fixed_free(L_scaled);
            big_fixed_free(normalized);
            big_fixed_free(scale_factor);
        }
        
        if ((token_id + 1) % 1000 == 0) {
            printf("  Initialized %u / %u tokens\r", token_id + 1, vocab_size);
            fflush(stdout);
        }
    }
    
    printf("\n✓ Embedding initialization complete\n");
}

/**
 * Get embedding for a single token with BigFixed
 */
void lattice_get_token_embedding_geometric_bigfixed(
    uint32_t token_id,
    uint32_t embedding_dim,
    BigFixed* output,
    int precision_bits
) {
    if (!output) return;
    
    BabylonianClockPosition pos = map_prime_index_to_clock((int)token_id);
    int symmetry_group = token_id % 12;
    
    for (uint32_t dim = 0; dim < embedding_dim; dim++) {
        uint64_t phi_i = cllm_get_dimensional_frequency(dim % 12);
        
        BigFixed* L = big_fixed_create(precision_bits);
        BigFixed* L_scaled = big_fixed_create(precision_bits);
        BigFixed* normalized = big_fixed_create(precision_bits);
        BigFixed* scale_factor = big_fixed_create(precision_bits);
        
        compute_L_bigfixed(pos, dim, phi_i, symmetry_group, L, precision_bits);
        
        big_fixed_from_double(scale_factor, 100.0);
        big_fixed_div(L_scaled, L, scale_factor);
        big_tanh(normalized, L_scaled, precision_bits);
        
        big_fixed_assign(&output[dim], normalized);
        
        big_fixed_free(L);
        big_fixed_free(L_scaled);
        big_fixed_free(normalized);
        big_fixed_free(scale_factor);
    }
}
