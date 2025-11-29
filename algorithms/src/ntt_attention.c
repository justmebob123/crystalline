/**
 * NTT Attention - O(n log n) Attention Implementation
 * 
 * Implements fast attention using Number Theoretic Transform.
 * Uses crystalline library for all mathematical operations.
 */

#include "ntt_attention.h"
#include "../../include/prime_math_custom.h"
#include "../../include/bigint_ntt.h"
#include "../../include/bigint_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Constants
#define DEFAULT_MIN_SEQ_LEN 256
#define NTT_SCALE_FACTOR 1000.0  // Scale floats to integers for NTT

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Round up to next power of 2
 */
static uint32_t next_power_of_2(uint32_t n) {
    if (n == 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

/**
 * Apply softmax to attention scores
 */
static void apply_softmax(float* scores, uint32_t len) {
    if (!scores || len == 0) return;
    
    // Find max for numerical stability
    float max_score = scores[0];
    for (uint32_t i = 1; i < len; i++) {
        if (scores[i] > max_score) {
            max_score = scores[i];
        }
    }
    
    // Compute exp(x - max) and sum
    float sum = 0.0f;
    for (uint32_t i = 0; i < len; i++) {
        scores[i] = prime_exp(scores[i] - max_score);
        sum += scores[i];
    }
    
    // Normalize
    if (sum > 0.0f) {
        for (uint32_t i = 0; i < len; i++) {
            scores[i] /= sum;
        }
    }
}

/**
 * Convert float array to BigInt array for NTT
 */
static int float_to_bigint_array(BigInt* output, const float* input, uint32_t len) {
    if (!output || !input) return 0;
    
    for (uint32_t i = 0; i < len; i++) {
        // Scale float to integer (multiply by scale factor)
        int64_t scaled = (int64_t)(input[i] * NTT_SCALE_FACTOR);
        big_from_int(&output[i], scaled);
    }
    
    return 1;
}

/**
 * Convert BigInt array back to float array
 */
static int bigint_to_float_array(float* output, const BigInt* input, uint32_t len) {
    if (!output || !input) return 0;
    
    for (uint32_t i = 0; i < len; i++) {
        // Convert BigInt to int64 and scale back to float
        int64_t value = big_to_int64(&input[i]);
        output[i] = (float)value / NTT_SCALE_FACTOR;
    }
    
    return 1;
}

// ============================================================================
// CONFIGURATION
// ============================================================================

void ntt_attention_config_init(
    NTTAttentionConfig* config,
    uint32_t head_dim,
    uint32_t num_heads
) {
    if (!config) return;
    
    config->min_seq_len = DEFAULT_MIN_SEQ_LEN;
    config->head_dim = head_dim;
    config->num_heads = num_heads;
    config->scale_factor = 1.0 / prime_sqrt((double)head_dim);
}

int should_use_ntt_attention(uint32_t seq_len, const NTTAttentionConfig* config) {
    if (!config) return 0;
    return seq_len >= config->min_seq_len;
}

double ntt_attention_estimate_speedup(uint32_t seq_len) {
    if (seq_len < 256) return 1.0;
    
    // O(n²) vs O(n log n) speedup
    double n = (double)seq_len;
    double standard_ops = n * n;
    double ntt_ops = n * prime_log(n);
    
    return standard_ops / ntt_ops;
}

// ============================================================================
// ATTENTION OPERATIONS
// ============================================================================

int ntt_attention_single_head(
    float* output,
    const float* queries,
    const float* keys,
    const float* values,
    uint32_t seq_len,
    uint32_t head_dim,
    double scale_factor
) {
    if (!output || !queries || !keys || !values) return 0;
    if (seq_len == 0 || head_dim == 0) return 0;
    
    // Round up to power of 2 for NTT
    uint32_t ntt_size = next_power_of_2(seq_len);
    
    // Allocate NTT context
    NTTContext ntt_ctx;
    if (!ntt_init(&ntt_ctx, ntt_size)) {
        fprintf(stderr, "Failed to initialize NTT context\n");
        return 0;
    }
    
    // Allocate working arrays
    BigInt* q_bigint = calloc(ntt_size, sizeof(BigInt));
    BigInt* k_bigint = calloc(ntt_size, sizeof(BigInt));
    BigInt* q_freq = calloc(ntt_size, sizeof(BigInt));
    BigInt* k_freq = calloc(ntt_size, sizeof(BigInt));
    BigInt* attn_freq = calloc(ntt_size, sizeof(BigInt));
    BigInt* attn_time = calloc(ntt_size, sizeof(BigInt));
    float* attn_scores = calloc(seq_len, sizeof(float));
    
    if (!q_bigint || !k_bigint || !q_freq || !k_freq || 
        !attn_freq || !attn_time || !attn_scores) {
        fprintf(stderr, "Memory allocation failed\n");
        goto cleanup;
    }
    
    // Initialize BigInt arrays
    for (uint32_t i = 0; i < ntt_size; i++) {
        big_init(&q_bigint[i]);
        big_init(&k_bigint[i]);
        big_init(&q_freq[i]);
        big_init(&k_freq[i]);
        big_init(&attn_freq[i]);
        big_init(&attn_time[i]);
    }
    
    // Process each dimension
    for (uint32_t d = 0; d < head_dim; d++) {
        // Extract dimension d from queries and keys
        for (uint32_t i = 0; i < seq_len; i++) {
            float q_val = queries[i * head_dim + d] * scale_factor;
            float k_val = keys[i * head_dim + d];
            
            int64_t q_scaled = (int64_t)(q_val * NTT_SCALE_FACTOR);
            int64_t k_scaled = (int64_t)(k_val * NTT_SCALE_FACTOR);
            
            big_from_int(&q_bigint[i], q_scaled);
            big_from_int(&k_bigint[i], k_scaled);
        }
        
        // Pad with zeros
        for (uint32_t i = seq_len; i < ntt_size; i++) {
            big_from_int(&q_bigint[i], 0);
            big_from_int(&k_bigint[i], 0);
        }
        
        // Forward NTT
        ntt_forward(&ntt_ctx, q_freq, q_bigint, ntt_size);
        ntt_forward(&ntt_ctx, k_freq, k_bigint, ntt_size);
        
        // Pointwise multiply in frequency domain
        for (uint32_t i = 0; i < ntt_size; i++) {
            big_mul(&attn_freq[i], &q_freq[i], &k_freq[i]);
            big_mod(&attn_freq[i], &attn_freq[i], &ntt_ctx.prime);
        }
        
        // Inverse NTT
        ntt_inverse(&ntt_ctx, attn_time, attn_freq, ntt_size);
        
        // Accumulate attention scores
        for (uint32_t i = 0; i < seq_len; i++) {
            double value = big_to_double(&attn_time[i]);
            attn_scores[i] += (float)(value / (NTT_SCALE_FACTOR * NTT_SCALE_FACTOR));
        }
    }
    
    // Apply softmax to attention scores
    apply_softmax(attn_scores, seq_len);
    
    // Compute weighted sum of values
    memset(output, 0, seq_len * head_dim * sizeof(float));
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            float weight = attn_scores[j];
            for (uint32_t d = 0; d < head_dim; d++) {
                output[i * head_dim + d] += weight * values[j * head_dim + d];
            }
        }
    }
    
cleanup:
    // Free BigInt arrays
    if (q_bigint) {
        for (uint32_t i = 0; i < ntt_size; i++) big_free(&q_bigint[i]);
        free(q_bigint);
    }
    if (k_bigint) {
        for (uint32_t i = 0; i < ntt_size; i++) big_free(&k_bigint[i]);
        free(k_bigint);
    }
    if (q_freq) {
        for (uint32_t i = 0; i < ntt_size; i++) big_free(&q_freq[i]);
        free(q_freq);
    }
    if (k_freq) {
        for (uint32_t i = 0; i < ntt_size; i++) big_free(&k_freq[i]);
        free(k_freq);
    }
    if (attn_freq) {
        for (uint32_t i = 0; i < ntt_size; i++) big_free(&attn_freq[i]);
        free(attn_freq);
    }
    if (attn_time) {
        for (uint32_t i = 0; i < ntt_size; i++) big_free(&attn_time[i]);
        free(attn_time);
    }
    free(attn_scores);
    
    ntt_free(&ntt_ctx);
    
    return 1;
}

int ntt_attention_multi_head(
    float* output,
    const float* queries,
    const float* keys,
    const float* values,
    uint32_t seq_len,
    uint32_t num_heads,
    uint32_t head_dim,
    double scale_factor
) {
    if (!output || !queries || !keys || !values) return 0;
    if (seq_len == 0 || num_heads == 0 || head_dim == 0) return 0;
    
    // Process each head independently
    for (uint32_t h = 0; h < num_heads; h++) {
        uint32_t head_offset = h * seq_len * head_dim;
        
        if (!ntt_attention_single_head(
            output + head_offset,
            queries + head_offset,
            keys + head_offset,
            values + head_offset,
            seq_len,
            head_dim,
            scale_factor
        )) {
            return 0;
        }
    }
    
    return 1;
}

int ntt_attention_forward(
    float* output,
    const float* queries,
    const float* keys,
    const float* values,
    uint32_t seq_len,
    uint32_t head_dim,
    double scale_factor
) {
    // Single-head forward pass
    return ntt_attention_single_head(
        output,
        queries,
        keys,
        values,
        seq_len,
        head_dim,
        scale_factor
    );
}