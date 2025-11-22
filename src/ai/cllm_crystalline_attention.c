/*
 * CLLM Crystalline Attention - Advanced Implementation
 * 
 * Implements the complete crystalline lattice attention mechanism with:
 * - Q→K reversal (query to key transformation)
 * - Hyperdimensional resonance
 * - Lattice coordinate-based attention weights
 * - Symmetry operations (rotations, reflections)
 * - Fourier-based dampening
 * - Plimpton ratio integration
 * - Einstein Lambda correction
 * - Cymatic frequency resonance
 * - Prime-based distance metrics
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/prime_float_math.h"
#include "../include/prime_lattice_core.h"

// Constants for crystalline attention
#define PI 3.14159265358979323846
#ifndef PHI
#define PHI 1.618033988749894848
#endif
#define EINSTEIN_LAMBDA (3.0 / 144000.0)
#define SCHUMANN_RESONANCE 7.83
#define GAMMA_BURST 40.0

// Cymatic frequencies (Hz)
static const float CYMATIC_FREQS[] = {432.0f, 528.0f, 639.0f, 741.0f, 852.0f, 963.0f};
static const int NUM_CYMATIC_FREQS = 6;

// Plimpton ratios (from Babylonian tablet)
typedef struct {
    float p;
    float q;
    float ratio;
} PlimptonRatio;

static const PlimptonRatio PLIMPTON_RATIOS[] = {
    {2.0f, 1.0f, 0.75f},      // (4-1)/(4+1) = 3/5
    {3.0f, 2.0f, 0.384615f},  // (9-4)/(9+4) = 5/13
    {4.0f, 3.0f, 0.28f},      // (16-9)/(16+9) = 7/25
    {5.0f, 4.0f, 0.219512f},  // (25-16)/(25+16) = 9/41
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
static float compute_lattice_distance(const float* coords1, const float* coords2,
                                      uint64_t prime1, uint64_t prime2) {
    // Euclidean distance in 3D lattice space
    float dx = coords1[0] - coords2[0];
    float dy = coords1[1] - coords2[1];
    float dz = coords1[2] - coords2[2];
    float euclidean = prime_sqrt(dx*dx + dy*dy + dz*dz);
    
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
    
    float prime_similarity = (gcd == 1) ? 1.0f : (1.0f / (float)gcd);
    
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
static void apply_mobius_transform(float* scores, int seq_len, int k) {
    if (!scores || seq_len <= 0) return;
    
    // Möbius parameters based on k
    float a = 1.0f;
    float b = (k % 2 == 0) ? 1.0f : -1.0f;  // Twist based on parity
    float c = 0.5f;
    float d = 1.0f;
    
    // Apply transformation: f(z) = (az + b) / (cz + d)
    for (int i = 0; i < seq_len; i++) {
        float z = scores[i];
        float numerator = a * z + b;
        float denominator = c * z + d;
        
        if (fabs(denominator) > 1e-8f) {
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
static void apply_plimpton_correction(float* weights, int seq_len, int position) {
    if (!weights || seq_len <= 0) return;
    
    // Select Plimpton ratio based on position
    int ratio_idx = position % NUM_PLIMPTON_RATIOS;
    PlimptonRatio ratio = PLIMPTON_RATIOS[ratio_idx];
    
    // Apply ratio-based scaling
    for (int i = 0; i < seq_len; i++) {
        // Distance from current position
        int dist = abs(i - position);
        
        // Apply Pythagorean scaling: weight * (p²-q²)/(p²+q²)
        float scale = ratio.ratio * prime_exp(-dist * EINSTEIN_LAMBDA);
        weights[i] *= (1.0f + scale);
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
static void apply_cymatic_resonance(float* weights, int seq_len, int position) {
    if (!weights || seq_len <= 0) return;
    
    for (int i = 0; i < seq_len; i++) {
        float resonance = 0.0f;
        
        // Sum resonance from all cymatic frequencies
        for (int f = 0; f < NUM_CYMATIC_FREQS; f++) {
            float freq = CYMATIC_FREQS[f];
            float phase = 2.0f * PI * freq * (float)(i - position) / (float)seq_len;
            resonance += prime_cos(phase) / (float)NUM_CYMATIC_FREQS;
        }
        
        // Apply resonance modulation
        weights[i] *= (1.0f + 0.1f * resonance);
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
static void apply_schumann_dampening(float* weights, int seq_len) {
    if (!weights || seq_len <= 0) return;
    
    float damping_factor = SCHUMANN_RESONANCE / 100.0f;
    
    for (int i = 0; i < seq_len; i++) {
        // Exponential dampening based on Schumann resonance
        float damping = prime_exp(-damping_factor * (float)i);
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
static void apply_gamma_burst(float* weights, int seq_len, int position) {
    if (!weights || seq_len <= 0) return;
    
    for (int i = 0; i < seq_len; i++) {
        // Gamma burst at 40 Hz
        float phase = 2.0f * PI * GAMMA_BURST * (float)(i - position) / (float)seq_len;
        float burst = 1.0f + 0.2f * prime_cos(phase);
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
static void query_to_key_reversal(const float* query, float* key_space,
                                  int head_dim, const float* lattice_coords,
                                  uint64_t prime) {
    if (!query || !key_space || head_dim <= 0) return;
    
    // Step 1: Rotate query by golden angle (φ-based)
    float golden_angle = 2.0f * PI / (PHI * PHI);
    float rotation_angle = golden_angle * (float)(prime % 360);
    
    for (int i = 0; i < head_dim; i++) {
        float angle = rotation_angle * (float)i / (float)head_dim;
        float cos_a = prime_cos(angle);
        float sin_a = prime_sin(angle);
        
        // Rotate in 2D subspace
        int j = (i + 1) % head_dim;
        key_space[i] = query[i] * cos_a - query[j] * sin_a;
    }
    
    // Step 2: Apply lattice coordinate transformation
    if (lattice_coords) {
        for (int i = 0; i < head_dim && i < 3; i++) {
            key_space[i] += lattice_coords[i] * 0.1f;
        }
    }
    
    // Step 3: Apply prime-based scaling
    float prime_scale = 1.0f / prime_sqrt((float)prime);
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
static float compute_hyperdimensional_resonance(const float* query, const float* key,
                                               int head_dim,
                                               const float* query_coords,
                                               const float* key_coords,
                                               uint64_t query_prime,
                                               uint64_t key_prime) {
    // 1. Standard dot product
    float dot_product = 0.0f;
    for (int i = 0; i < head_dim; i++) {
        dot_product += query[i] * key[i];
    }
    
    // 2. Lattice distance (inverse relationship)
    float lattice_dist = 1.0f;
    if (query_coords && key_coords) {
        lattice_dist = compute_lattice_distance(query_coords, key_coords,
                                                query_prime, key_prime);
        lattice_dist = 1.0f / (1.0f + lattice_dist);  // Inverse for similarity
    }
    
    // 3. Prime similarity (coprimality)
    uint64_t gcd = query_prime;
    uint64_t b = key_prime;
    while (b != 0) {
        uint64_t temp = b;
        b = gcd % b;
        gcd = temp;
    }
    float prime_similarity = (gcd == 1) ? 0.5f : (1.0f / (float)gcd);
    
    // 4. Fourier phase alignment
    float phase_diff = 2.0f * PI * (float)(query_prime - key_prime) / (float)(query_prime + key_prime);
    float phase_alignment = (1.0f + prime_cos(phase_diff)) / 2.0f;
    
    // Combine all components
    float resonance = dot_product * lattice_dist * (1.0f + prime_similarity) * phase_alignment;
    
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
                                       const float* input,
                                       float* output,
                                       const float* lattice_coords,
                                       const uint64_t* token_primes,
                                       int seq_len) {
    if (!layer || !input || !output || seq_len <= 0) return;
    
    uint32_t num_heads = layer->num_heads;
    uint32_t head_dim = layer->head_dim;
    uint32_t embedding_dim = num_heads * head_dim;
    
    // Allocate working buffers
    float* queries = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* keys = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* values = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* key_space = (float*)calloc(head_dim, sizeof(float));
    float* attention_scores = (float*)calloc(seq_len, sizeof(float));
    
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
        const float* input_vec = &input[pos * embedding_dim];
        
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                float q_sum = 0.0f, k_sum = 0.0f, v_sum = 0.0f;
                
                for (uint32_t i = 0; i < head_dim; i++) {
                    size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                    q_sum += layer->query_lattice[weight_idx] * input_vec[h * head_dim + i];
                    k_sum += layer->key_lattice[weight_idx] * input_vec[h * head_dim + i];
                    v_sum += layer->value_lattice[weight_idx] * input_vec[h * head_dim + i];
                }
                
                queries[pos * embedding_dim + h * head_dim + d] = q_sum;
                keys[pos * embedding_dim + h * head_dim + d] = k_sum;
                values[pos * embedding_dim + h * head_dim + d] = v_sum;
            }
        }
    }
    
    // Apply crystalline attention for each position and head
    memset(output, 0, seq_len * embedding_dim * sizeof(float));
    
    for (int pos = 0; pos < seq_len; pos++) {
        for (uint32_t h = 0; h < num_heads; h++) {
            const float* query = &queries[pos * embedding_dim + h * head_dim];
            const float* pos_coords = lattice_coords ? &lattice_coords[pos * 3] : NULL;
            uint64_t pos_prime = token_primes ? token_primes[pos] : 2;
            
            // Apply Q→K reversal
            query_to_key_reversal(query, key_space, head_dim, pos_coords, pos_prime);
            
            // Compute attention scores using hyperdimensional resonance
            for (int i = 0; i < seq_len; i++) {
                const float* key = &keys[i * embedding_dim + h * head_dim];
                const float* key_coords = lattice_coords ? &lattice_coords[i * 3] : NULL;
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
            float max_score = attention_scores[0];
            for (int i = 1; i < seq_len; i++) {
                if (attention_scores[i] > max_score) max_score = attention_scores[i];
            }
            
            float sum = 0.0f;
            for (int i = 0; i < seq_len; i++) {
                attention_scores[i] = prime_exp(attention_scores[i] - max_score);
                sum += attention_scores[i];
            }
            
            if (sum > 1e-8f) {
                for (int i = 0; i < seq_len; i++) {
                    attention_scores[i] /= sum;
                }
            }
            
            // Apply attention to values
            float* head_output = &output[pos * embedding_dim + h * head_dim];
            for (int i = 0; i < seq_len; i++) {
                const float* value = &values[i * embedding_dim + h * head_dim];
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
void cllm_apply_einstein_correction(float* gradients, size_t size) {
    if (!gradients || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        // Apply Lambda correction: g' = g * (1 - Λ)
        gradients[i] *= (1.0f - EINSTEIN_LAMBDA);
    }
}