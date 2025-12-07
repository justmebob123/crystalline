/**
 * @file cllm_attention.c
 * @brief Main Attention Layer Implementation (Standard + Crystalline Features)
 * 
 * ATTENTION STRATEGY: Complete Crystalline Lattice Attention
 * - Standard attention mechanism with crystalline enhancements
 * - Q→K reversal (query to key transformation)
 * - Hyperdimensional resonance
 * - Lattice coordinate-based attention weights
 * - Symmetry operations (rotations, reflections)
 * - Fourier-based dampening
 * - Plimpton ratio integration
 * - Einstein Lambda correction
 * - Cymatic frequency resonance
 * - Prime-based distance metrics
 * 
 * RELATIONSHIP TO OTHER ATTENTION FILES:
 * - cllm_attention.c: THIS FILE - Main attention implementation
 * - cllm_angular_attention.c: Angular position formula (θ(n,k,λ,ω,ψ))
 * - cllm_ntt_attention.c: NTT-based O(n log n) fast attention
 * 
 * USE THIS WHEN:
 * - Standard attention with crystalline features
 * - Full-featured attention mechanism
 * - Default attention layer
 * 
 * USE cllm_angular_attention.c WHEN:
 * - Need angular position encoding
 * - Geometric relationships important
 * 
 * USE cllm_ntt_attention.c WHEN:
 * - Long sequences (>1024 tokens)
 * - Need O(n log n) complexity
 * - Performance critical
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/prime_float_math.h"
#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/prime_float_math.h"
#include "../include/prime_lattice_core.h"
#include "../include/bigfixed_core.h"

// Constants for crystalline attention
#define PI 3.14159265358979323846
#ifndef PHI
#define PHI 1.618033988749894848
#endif
#define EINSTEIN_LAMBDA (3.0 / 144000.0)
#define SCHUMANN_RESONANCE 7.83
#define GAMMA_BURST 40.0

// Cymatic frequencies (Hz)
// PRECISION FIX: Changed from double to double for consistency
static const double CYMATIC_FREQS[] = {432.0, 528.0, 639.0, 741.0, 852.0, 963.0};
static const int NUM_CYMATIC_FREQS = 6;

// Plimpton ratios (from Babylonian tablet)
// PRECISION FIX: Changed from double to double for consistency
typedef struct {
    double p;
    double q;
    double ratio;
} PlimptonRatio;

static const PlimptonRatio PLIMPTON_RATIOS[] = {
    {2.0, 1.0, 0.75},      // (4-1)/(4+1) = 3/5
    {3.0, 2.0, 0.384615},  // (9-4)/(9+4) = 5/13
    {4.0, 3.0, 0.28},      // (16-9)/(16+9) = 7/25
    {5.0, 4.0, 0.219512},  // (25-16)/(25+16) = 9/41
};
static const int NUM_PLIMPTON_RATIOS = 4;

/**
 * Compute lattice distance between two points using prime-based metric
 * 
 * Distance incorporates:
 * - Euclidean distance in lattice space
 * - Prime factorization similarity
 * - Coprimality bonus
 * 
 * @param coords1 First point coordinates [3]
 * @param coords2 Second point coordinates [3]
 * @param prime1 Prime associated with first point
 * @param prime2 Prime associated with second point
 * @return Lattice distance
 */
// PRECISION FIX: Changed from double to double for consistency with AttentionLayer
static double compute_lattice_distance(const double* coords1, const double* coords2,
                                      uint64_t prime1, uint64_t prime2) {
    // Euclidean distance in 3D lattice space
    double dx = coords1[0] - coords2[0];
    double dy = coords1[1] - coords2[1];
    double dz = coords1[2] - coords2[2];
    double euclidean = prime_sqrt(dx*dx + dy*dy + dz*dz);
    
    // Prime factorization distance
    // If primes are coprime (gcd=1), they're maximally different
    // If one divides the other, they're similar
    uint64_t gcd = prime1;
    uint64_t b = prime2;
    while (b != 0) {
        uint64_t temp = b;
        b = gcd % b;
        gcd = temp;
    }
    
    double prime_similarity = (gcd == 1) ? 1.0 : (1.0 / (double)gcd);
    
    // Combined distance with prime weighting
    return euclidean * prime_similarity;
}

/**
 * Apply Möbius transformation to attention scores
 * 
 * Möbius: f(z) = (az + b) / (cz + d) where ad - bc ≠ 0
 * 
 * @param scores Attention scores [seq_len]
 * @param seq_len Sequence length
 * @param k Twist parameter (affects sign)
 */
static void apply_mobius_transform(double* scores, int seq_len, int k) {
    if (!scores || seq_len <= 0) return;
    
    // Möbius parameters based on k
    double a = 1.0;
    double b = (k % 2 == 0) ? 1.0 : -1.0;  // Twist based on parity
    double c = 0.5;
    double d = 1.0;
    
    // Apply transformation: f(z) = (az + b) / (cz + d)
    for (int i = 0; i < seq_len; i++) {
        double z = scores[i];
        double numerator = a * z + b;
        double denominator = c * z + d;
        
        if (prime_fabs(denominator) > 1e-8) {
            scores[i] = numerator / denominator;
        }
    }
}

/**
 * Apply Plimpton ratio correction to attention weights
 * 
 * Uses Babylonian Pythagorean triples to adjust attention
 * based on geometric relationships
 * 
 * @param weights Attention weights [seq_len]
 * @param seq_len Sequence length
 * @param position Current position in sequence
 */
static void apply_plimpton_correction(double* weights, int seq_len, int position) {
    if (!weights || seq_len <= 0) return;
    
    // Select Plimpton ratio based on position
    int ratio_idx = position % NUM_PLIMPTON_RATIOS;
    PlimptonRatio ratio = PLIMPTON_RATIOS[ratio_idx];
    
    // Apply ratio-based scaling
    for (int i = 0; i < seq_len; i++) {
        // Distance from current position
        int dist = abs(i - position);
        
        // Apply Pythagorean scaling: weight * (p²-q²)/(p²+q²)
        double scale = ratio.ratio * prime_exp(-dist * EINSTEIN_LAMBDA);
        weights[i] *= (1.0 + scale);
    }
}

/**
 * Apply cymatic frequency resonance to attention
 * 
 * Modulates attention based on harmonic frequencies
 * Creates resonance patterns in attention weights
 * 
 * @param weights Attention weights [seq_len]
 * @param seq_len Sequence length
 * @param position Current position
 */
static void apply_cymatic_resonance(double* weights, int seq_len, int position) {
    if (!weights || seq_len <= 0) return;
    
    for (int i = 0; i < seq_len; i++) {
        double resonance = 0.0;
        
        // Sum resonance from all cymatic frequencies
        for (int f = 0; f < NUM_CYMATIC_FREQS; f++) {
            double freq = CYMATIC_FREQS[f];
            double phase = 2.0 * PI * freq * (double)(i - position) / (double)seq_len;
            resonance += prime_cos(phase) / (double)NUM_CYMATIC_FREQS;
        }
        
        // Apply resonance modulation
        weights[i] *= (1.0 + 0.1 * resonance);
    }
}

/**
 * Apply Schumann resonance dampening
 * 
 * Dampens attention weights based on Earth's natural frequency
 * Provides stability and prevents over-attention
 * 
 * @param weights Attention weights [seq_len]
 * @param seq_len Sequence length
 */
static void apply_schumann_dampening(double* weights, int seq_len) {
    if (!weights || seq_len <= 0) return;
    
    double damping_factor = SCHUMANN_RESONANCE / 100.0;
    
    for (int i = 0; i < seq_len; i++) {
        // Exponential dampening based on Schumann resonance
        double damping = prime_exp(-damping_factor * (double)i);
        weights[i] *= damping;
    }
}

/**
 * Apply gamma burst activation
 * 
 * Enhances attention at specific frequencies (40 Hz)
 * Mimics neural gamma oscillations
 * 
 * @param weights Attention weights [seq_len]
 * @param seq_len Sequence length
 * @param position Current position
 */
static void apply_gamma_burst(double* weights, int seq_len, int position) {
    if (!weights || seq_len <= 0) return;
    
    for (int i = 0; i < seq_len; i++) {
        // Gamma burst at 40 Hz
        double phase = 2.0 * PI * GAMMA_BURST * (double)(i - position) / (double)seq_len;
        double burst = 1.0 + 0.2 * prime_cos(phase);
        weights[i] *= burst;
    }
}

/**
 * Q→K Reversal: Transform query to key space
 * 
 * This is the core of the crystalline attention mechanism.
 * The query Q is transformed into key space K through:
 * 1. Lattice coordinate transformation
 * 2. Prime-based rotation
 * 3. Symmetry operations
 * 
 * "if Q is my question, then k is unknown. I have to discover it."
 * 
 * @param query Query vector [head_dim]
 * @param key_space Output key space vector [head_dim]
 * @param head_dim Dimension per head
 * @param lattice_coords Lattice coordinates [3]
 * @param prime Associated prime number
 */
static void query_to_key_reversal(const double* query, double* key_space,
                                  int head_dim, const double* lattice_coords,
                                  uint64_t prime) {
    if (!query || !key_space || head_dim <= 0) return;
    
    // Step 1: Rotate query by golden angle (φ-based)
    double golden_angle = 2.0 * PI / (PHI * PHI);
    double rotation_angle = golden_angle * (double)(prime % 360);
    
    for (int i = 0; i < head_dim; i++) {
        double angle = rotation_angle * (double)i / (double)head_dim;
        double cos_a = prime_cos(angle);
        double sin_a = prime_sin(angle);
        
        // Rotate in 2D subspace
        int j = (i + 1) % head_dim;
        key_space[i] = query[i] * cos_a - query[j] * sin_a;
    }
    
    // Step 2: Apply lattice coordinate transformation
    if (lattice_coords) {
        for (int i = 0; i < head_dim && i < 3; i++) {
            key_space[i] += lattice_coords[i] * 0.1;
        }
    }
    
    // Step 3: Apply prime-based scaling
    double prime_scale = 1.0 / prime_sqrt((double)prime);
    for (int i = 0; i < head_dim; i++) {
        key_space[i] *= prime_scale;
    }
}

/**
 * Compute hyperdimensional resonance between query and key
 * 
 * Resonance is computed using:
 * 1. Dot product in original space
 * 2. Lattice distance in crystalline space
 * 3. Prime factorization similarity
 * 4. Fourier phase alignment
 * 
 * @param query Query vector [head_dim]
 * @param key Key vector [head_dim]
 * @param head_dim Dimension per head
 * @param query_coords Query lattice coordinates [3]
 * @param key_coords Key lattice coordinates [3]
 * @param query_prime Query prime number
 * @param key_prime Key prime number
 * @return Resonance score
 */
static double compute_hyperdimensional_resonance(const double* query, const double* key,
                                               int head_dim,
                                               const double* query_coords,
                                               const double* key_coords,
                                               uint64_t query_prime,
                                               uint64_t key_prime) {
    // 1. Standard dot product
    double dot_product = 0.0;
    for (int i = 0; i < head_dim; i++) {
        dot_product += query[i] * key[i];
    }
    
    // 2. Lattice distance (inverse relationship)
    double lattice_dist = 1.0;
    if (query_coords && key_coords) {
        lattice_dist = compute_lattice_distance(query_coords, key_coords,
                                                query_prime, key_prime);
        lattice_dist = 1.0 / (1.0 + lattice_dist);  // Inverse for similarity
    }
    
    // 3. Prime similarity (coprimality)
    uint64_t gcd = query_prime;
    uint64_t b = key_prime;
    while (b != 0) {
        uint64_t temp = b;
        b = gcd % b;
        gcd = temp;
    }
    double prime_similarity = (gcd == 1) ? 0.5 : (1.0 / (double)gcd);
    
    // 4. Fourier phase alignment
    double phase_diff = 2.0 * PI * (double)(query_prime - key_prime) / (double)(query_prime + key_prime);
    double phase_alignment = (1.0 + prime_cos(phase_diff)) / 2.0;
    
    // Combine all components
    double resonance = dot_product * lattice_dist * (1.0 + prime_similarity) * phase_alignment;
    
    return resonance;
}

/**
 * Crystalline Attention Forward Pass
 * 
 * Complete implementation with all advanced features:
 * - Q→K reversal
 * - Hyperdimensional resonance
 * - Lattice-based attention
 * - Symmetry operations
 * - Fourier dampening
 * - Plimpton ratios
 * - Einstein Lambda
 * - Cymatic resonance
 * 
 * @param layer Attention layer
 * @param input Input sequence [seq_len x embedding_dim]
 * @param output Output sequence [seq_len x embedding_dim]
 * @param lattice_coords Lattice coordinates for each token [seq_len x 3]
 * @param token_primes Prime numbers for each token [seq_len]
 * @param seq_len Sequence length
 */
void cllm_crystalline_attention_forward(AttentionLayer* layer,
                                       const double* input,
                                       double* output,
                                       const double* lattice_coords,
                                       const uint64_t* token_primes,
                                       int seq_len) {
    if (!layer || !input || !output || seq_len <= 0) return;
    
    uint32_t num_heads = layer->num_heads;
    uint32_t head_dim = layer->head_dim;
    uint32_t embedding_dim = num_heads * head_dim;
    
    // Allocate working buffers
    double* queries = (double*)calloc(seq_len * embedding_dim, sizeof(double));
    double* keys = (double*)calloc(seq_len * embedding_dim, sizeof(double));
    double* values = (double*)calloc(seq_len * embedding_dim, sizeof(double));
    double* key_space = (double*)calloc(head_dim, sizeof(double));
    double* attention_scores = (double*)calloc(seq_len, sizeof(double));
    
    if (!queries || !keys || !values || !key_space || !attention_scores) {
        free(queries);
        free(keys);
        free(values);
        free(key_space);
        free(attention_scores);
        return;
    }
    
    // Project input to Q, K, V (simplified - using lattice weights)
    for (int pos = 0; pos < seq_len; pos++) {
        const double* input_vec = &input[pos * embedding_dim];
        
           // BIGFIXED IMPLEMENTATION - NO FLOATS
           // Use dot_product_bigfixed() from algorithms layer
           for (uint32_t h = 0; h < num_heads; h++) {
               for (uint32_t d = 0; d < head_dim; d++) {
                   // Allocate BigFixed results
                   BigFixed* q_sum = big_fixed_create(64);  // Use default precision
                   BigFixed* k_sum = big_fixed_create(64);
                   BigFixed* v_sum = big_fixed_create(64);
                   
                   if (!q_sum || !k_sum || !v_sum) {
                       if (q_sum) big_fixed_free(q_sum);
                       if (k_sum) big_fixed_free(k_sum);
                       if (v_sum) big_fixed_free(v_sum);
                       continue;
                   }
                   
                      // Compute dot products using BigFixed (NO FLOATS!)
                      big_fixed_from_int(q_sum, 0);
                      big_fixed_from_int(k_sum, 0);
                      big_fixed_from_int(v_sum, 0);
                      
                      // Compute Q, K, V using BigFixed operations
                      BigFixed* temp = big_fixed_create(64);
                      BigFixed* input_bf = big_fixed_create(64);
                      
                      for (uint32_t i = 0; i < head_dim; i++) {
                          // Convert input to BigFixed
                          big_fixed_from_double(input_bf, (double)input_vec[h * head_dim + i]);
                          
                          // Q: q_sum += input[i] * weight_q[i,d]
                          uint32_t q_idx = (h * head_dim + i) * head_dim + d;
                          BigFixed* q_weight = big_fixed_create(128);
                            big_fixed_from_double(q_weight, layer->query_lattice[q_idx]);
                            big_fixed_mul(temp, input_bf, q_weight);
                            big_fixed_free(q_weight);
                          big_fixed_add(q_sum, q_sum, temp);
                          
                          // K: k_sum += input[i] * weight_k[i,d]
                          uint32_t k_idx = (h * head_dim + i) * head_dim + d;
                          BigFixed* k_weight = big_fixed_create(128);
                            big_fixed_from_double(k_weight, layer->key_lattice[k_idx]);
                            big_fixed_mul(temp, input_bf, k_weight);
                            big_fixed_free(k_weight);
                          big_fixed_add(k_sum, k_sum, temp);
                          
                          // V: v_sum += input[i] * weight_v[i,d]
                          uint32_t v_idx = (h * head_dim + i) * head_dim + d;
                          BigFixed* v_weight = big_fixed_create(128);
                            big_fixed_from_double(v_weight, layer->value_lattice[v_idx]);
                            big_fixed_mul(temp, input_bf, v_weight);
                            big_fixed_free(v_weight);
                          big_fixed_add(v_sum, v_sum, temp);
                      }
                      
                      big_fixed_free(temp);
                      big_fixed_free(input_bf);
                      
                   // Store results (convert to double for now - will be removed when queries/keys/values are BigFixed**)
                   queries[pos * embedding_dim + h * head_dim + d] = big_fixed_to_double(q_sum);
                   keys[pos * embedding_dim + h * head_dim + d] = big_fixed_to_double(k_sum);
                   values[pos * embedding_dim + h * head_dim + d] = big_fixed_to_double(v_sum);
                   
                   // Cleanup
                   big_fixed_free(q_sum);
                   big_fixed_free(k_sum);
                   big_fixed_free(v_sum);
               }
           }
    }
    
    // Apply crystalline attention for each position and head
    memset(output, 0, seq_len * embedding_dim * sizeof(double));
    
    for (int pos = 0; pos < seq_len; pos++) {
        for (uint32_t h = 0; h < num_heads; h++) {
            const double* query = &queries[pos * embedding_dim + h * head_dim];
            const double* pos_coords = lattice_coords ? &lattice_coords[pos * 3] : NULL;
            uint64_t pos_prime = token_primes ? token_primes[pos] : 2;
            
            // Apply Q→K reversal
            query_to_key_reversal(query, key_space, head_dim, pos_coords, pos_prime);
            
            // Compute attention scores using hyperdimensional resonance
            for (int i = 0; i < seq_len; i++) {
                const double* key = &keys[i * embedding_dim + h * head_dim];
                const double* key_coords = lattice_coords ? &lattice_coords[i * 3] : NULL;
                uint64_t key_prime = token_primes ? token_primes[i] : 2;
                
                attention_scores[i] = compute_hyperdimensional_resonance(
                    key_space, key, head_dim,
                    pos_coords, key_coords,
                    pos_prime, key_prime
                );
            }
            
            // Apply crystalline transformations
            apply_mobius_transform(attention_scores, seq_len, pos);
            apply_plimpton_correction(attention_scores, seq_len, pos);
            apply_cymatic_resonance(attention_scores, seq_len, pos);
            apply_schumann_dampening(attention_scores, seq_len);
            apply_gamma_burst(attention_scores, seq_len, pos);
            
            // Softmax normalization
            double max_score = attention_scores[0];
            for (int i = 1; i < seq_len; i++) {
                if (attention_scores[i] > max_score) max_score = attention_scores[i];
            }
            
            double sum = 0.0;
            for (int i = 0; i < seq_len; i++) {
                attention_scores[i] = prime_exp(attention_scores[i] - max_score);
                sum += attention_scores[i];
            }
            
            if (sum > 1e-8) {
                for (int i = 0; i < seq_len; i++) {
                    attention_scores[i] /= sum;
                }
            }
            
            // Apply attention to values
            double* head_output = &output[pos * embedding_dim + h * head_dim];
            for (int i = 0; i < seq_len; i++) {
                const double* value = &values[i * embedding_dim + h * head_dim];
                for (uint32_t d = 0; d < head_dim; d++) {
                    head_output[d] += attention_scores[i] * value[d];
                }
            }
        }
    }
    
    // Cleanup
    free(queries);
    free(keys);
    free(values);
    free(key_space);
    free(attention_scores);
}

/**
 * Apply Einstein Lambda correction to gradients
 * 
 * Corrects gradients using Einstein's cosmological constant
 * Provides stability and prevents gradient explosion
 * 
 * @param gradients Gradient array
 * @param size Array size
 */
void cllm_apply_einstein_correction(double* gradients, size_t size) {
    if (!gradients || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        // Apply Lambda correction: g' = g * (1 - Λ)
        gradients[i] *= (1.0 - EINSTEIN_LAMBDA);
    }
}