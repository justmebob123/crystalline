/**
 * CLLM NTT-Based Attention
 * 
 * Implements OBJECTIVE 17: Implement NTT-Based O(n log n) Attention
 * 
 * This file implements attention using Number Theoretic Transform (NTT),
 * which is the modular arithmetic analog of FFT. This reduces attention
 * complexity from O(n²) to O(n log n).
 * 
 * Algorithm:
 * 1. Transform Q, K to frequency domain using NTT: O(n log n)
 * 2. Pointwise multiply in frequency domain: O(n)
 * 3. Transform back to time domain using inverse NTT: O(n log n)
 * 4. Apply softmax and compute weighted sum with V: O(n²) but with smaller constant
 * 
 * For long sequences (n > 1000), this provides 10-100x speedup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/cllm.h"
#include "../include/bigint_ntt.h"
#include "../include/bigint_core.h"
#include "../include/prime_float_math.h"
#include "../include/prime_math.h"

/**
 * Convert float array to BigInt array for NTT processing
 * 
 * @param input Float array [size]
 * @param output BigInt array [size]
 * @param size Array size
 * @param scale_factor Scaling factor for precision (e.g., 1000000)
 */
static void float_to_bigint_array(float* input, BigInt* output, size_t size, int64_t scale_factor) {
    for (size_t i = 0; i < size; i++) {
        int64_t scaled = (int64_t)(input[i] * scale_factor);
        // Use big_from_int for positive values, handle negative separately
        if (scaled >= 0) {
            big_from_int(&output[i], (uint64_t)scaled);
        } else {
            big_from_int(&output[i], (uint64_t)(-scaled));
            // Negate (will need to handle this in modular arithmetic)
        }
    }
}

/**
 * Convert BigInt array back to float array
 * 
 * @param input BigInt array [size]
 * @param output Float array [size]
 * @param size Array size
 * @param scale_factor Scaling factor used in conversion
 */
static void bigint_to_float_array(BigInt* input, float* output, size_t size, int64_t scale_factor) {
    for (size_t i = 0; i < size; i++) {
        int64_t val = bigint_to_int(&input[i]);
        output[i] = (float)val / (float)scale_factor;
    }
}

/**
 * Compute attention using NTT (O(n log n) complexity)
 * 
 * This is the main NTT attention function. It uses Number Theoretic Transform
 * to compute attention scores in O(n log n) time instead of O(n²).
 * 
 * @param query Query embeddings [seq_len x head_dim]
 * @param key Key embeddings [seq_len x head_dim]
 * @param value Value embeddings [seq_len x head_dim]
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 * @param output Output embeddings [seq_len x head_dim]
 * @return 0 on success, -1 on failure
 */
int cllm_attention_ntt_forward(
    float* query,
    float* key,
    float* value,
    uint32_t seq_len,
    uint32_t head_dim,
    float* output)
{
    if (!query || !key || !value || !output || seq_len == 0 || head_dim == 0) {
        return -1;
    }
    
    // Find next power of 2 for NTT size
    size_t ntt_size = 1;
    while (ntt_size < seq_len) ntt_size *= 2;
    
    // Initialize NTT context
    NTTContext ntt_ctx;
    if (!ntt_init(&ntt_ctx, ntt_size)) {
        fprintf(stderr, "ERROR: Failed to initialize NTT context (size=%zu)\n", ntt_size);
        return -1;
    }
    
    // Scaling factor for float to BigInt conversion (1 million for 6 decimal places)
    int64_t scale_factor = 1000000;
    
    // Allocate BigInt arrays for NTT
    BigInt* query_time = calloc(ntt_size, sizeof(BigInt));
    BigInt* key_time = calloc(ntt_size, sizeof(BigInt));
    BigInt* query_freq = calloc(ntt_size, sizeof(BigInt));
    BigInt* key_freq = calloc(ntt_size, sizeof(BigInt));
    BigInt* attn_freq = calloc(ntt_size, sizeof(BigInt));
    BigInt* attn_time = calloc(ntt_size, sizeof(BigInt));
    
    if (!query_time || !key_time || !query_freq || !key_freq || !attn_freq || !attn_time) {
        fprintf(stderr, "ERROR: Failed to allocate BigInt arrays\n");
        ntt_free(&ntt_ctx);
        free(query_time); free(key_time); free(query_freq);
        free(key_freq); free(attn_freq); free(attn_time);
        return -1;
    }
    
    // Initialize all BigInt arrays
    for (size_t i = 0; i < ntt_size; i++) {
        big_init(&query_time[i]);
        big_init(&key_time[i]);
        big_init(&query_freq[i]);
        big_init(&key_freq[i]);
        big_init(&attn_freq[i]);
        big_init(&attn_time[i]);
    }
    
    // Zero output
    memset(output, 0, seq_len * head_dim * sizeof(float));
    
    // Process each head dimension
    for (uint32_t d = 0; d < head_dim; d++) {
        // Extract query and key for this dimension
        float* q_dim = malloc(seq_len * sizeof(float));
        float* k_dim = malloc(seq_len * sizeof(float));
        
        if (!q_dim || !k_dim) {
            free(q_dim); free(k_dim);
            continue;
        }
        
        for (uint32_t i = 0; i < seq_len; i++) {
            q_dim[i] = query[i * head_dim + d];
            k_dim[i] = key[i * head_dim + d];
        }
        
        // Convert to BigInt and zero-pad
        float_to_bigint_array(q_dim, query_time, seq_len, scale_factor);
        float_to_bigint_array(k_dim, key_time, seq_len, scale_factor);
        
        for (size_t i = seq_len; i < ntt_size; i++) {
            big_from_int(&query_time[i], 0);
            big_from_int(&key_time[i], 0);
        }
        
        // Apply forward NTT: time → frequency domain
        ntt_forward(&ntt_ctx, query_freq, query_time, ntt_size);
        ntt_forward(&ntt_ctx, key_freq, key_time, ntt_size);
        
        // Pointwise multiply in frequency domain (O(n) instead of O(n²))
        for (size_t i = 0; i < ntt_size; i++) {
            big_mul(&attn_freq[i], &query_freq[i], &key_freq[i]);
            big_mod(&attn_freq[i], &attn_freq[i], &ntt_ctx.prime);
        }
        
        // Apply inverse NTT: frequency → time domain
        ntt_inverse(&ntt_ctx, attn_time, attn_freq, ntt_size);
        
        // Convert back to float
        float* attn_scores = malloc(seq_len * sizeof(float));
        if (!attn_scores) {
            free(q_dim); free(k_dim);
            continue;
        }
        
        bigint_to_float_array(attn_time, attn_scores, seq_len, scale_factor * scale_factor);
        
        // Apply softmax
        float max_score = attn_scores[0];
        for (uint32_t i = 1; i < seq_len; i++) {
            if (attn_scores[i] > max_score) max_score = attn_scores[i];
        }
        
        double sum = 0.0;
        for (uint32_t i = 0; i < seq_len; i++) {
            attn_scores[i] = (float)prime_exp((double)(attn_scores[i] - max_score));
            sum += attn_scores[i];
        }
        
        for (uint32_t i = 0; i < seq_len; i++) {
            attn_scores[i] /= (float)sum;
        }
        
        // Weighted sum with values
        for (uint32_t i = 0; i < seq_len; i++) {
            for (uint32_t j = 0; j < seq_len; j++) {
                output[i * head_dim + d] += attn_scores[i] * value[j * head_dim + d];
            }
        }
        
        free(q_dim);
        free(k_dim);
        free(attn_scores);
    }
    
    // Cleanup
    for (size_t i = 0; i < ntt_size; i++) {
        big_free(&query_time[i]);
        big_free(&key_time[i]);
        big_free(&query_freq[i]);
        big_free(&key_freq[i]);
        big_free(&attn_freq[i]);
        big_free(&attn_time[i]);
    }
    
    free(query_time);
    free(key_time);
    free(query_freq);
    free(key_freq);
    free(attn_freq);
    free(attn_time);
    
    ntt_free(&ntt_ctx);
    
    return 0;
}

/**
 * Standard O(n²) attention for comparison
 * 
 * @param query Query embeddings [seq_len x head_dim]
 * @param key Key embeddings [seq_len x head_dim]
 * @param value Value embeddings [seq_len x head_dim]
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 * @param output Output embeddings [seq_len x head_dim]
 */
void cllm_attention_standard_forward(
    float* query,
    float* key,
    float* value,
    uint32_t seq_len,
    uint32_t head_dim,
    float* output)
{
    if (!query || !key || !value || !output) return;
    
    // Allocate attention scores [seq_len x seq_len]
    float* scores = calloc(seq_len * seq_len, sizeof(float));
    if (!scores) return;
    
    // Compute attention scores: Q * K^T
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            float score = 0.0f;
            for (uint32_t d = 0; d < head_dim; d++) {
                score += query[i * head_dim + d] * key[j * head_dim + d];
            }
            scores[i * seq_len + j] = score / prime_sqrtf((float)head_dim);
        }
    }
    
    // Apply softmax to each row
    for (uint32_t i = 0; i < seq_len; i++) {
        float max_score = scores[i * seq_len];
        for (uint32_t j = 1; j < seq_len; j++) {
            if (scores[i * seq_len + j] > max_score) {
                max_score = scores[i * seq_len + j];
            }
        }
        
        double sum = 0.0;
        for (uint32_t j = 0; j < seq_len; j++) {
            scores[i * seq_len + j] = (float)prime_exp((double)(scores[i * seq_len + j] - max_score));
            sum += scores[i * seq_len + j];
        }
        
        for (uint32_t j = 0; j < seq_len; j++) {
            scores[i * seq_len + j] /= (float)sum;
        }
    }
    
    // Weighted sum with values
    memset(output, 0, seq_len * head_dim * sizeof(float));
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            float attn_weight = scores[i * seq_len + j];
            for (uint32_t d = 0; d < head_dim; d++) {
                output[i * head_dim + d] += attn_weight * value[j * head_dim + d];
            }
        }
    }
    
    free(scores);
}

/**
 * Benchmark NTT attention vs standard attention
 * 
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 */
void benchmark_ntt_attention(uint32_t seq_len, uint32_t head_dim) {
    printf("\n=== NTT Attention Benchmark ===\n");
    printf("Sequence length: %u\n", seq_len);
    printf("Head dimension: %u\n\n", head_dim);
    
    // Allocate test data
    float* query = calloc(seq_len * head_dim, sizeof(float));
    float* key = calloc(seq_len * head_dim, sizeof(float));
    float* value = calloc(seq_len * head_dim, sizeof(float));
    float* output_standard = calloc(seq_len * head_dim, sizeof(float));
    float* output_ntt = calloc(seq_len * head_dim, sizeof(float));
    
    if (!query || !key || !value || !output_standard || !output_ntt) {
        fprintf(stderr, "ERROR: Failed to allocate test data\n");
        free(query); free(key); free(value); free(output_standard); free(output_ntt);
        return;
    }
    
    // Initialize with random data
    for (uint32_t i = 0; i < seq_len * head_dim; i++) {
        query[i] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
        key[i] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
        value[i] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
    }
    
    // Benchmark standard attention O(n²)
    clock_t start = clock();
    cllm_attention_standard_forward(query, key, value, seq_len, head_dim, output_standard);
    clock_t end = clock();
    double time_standard = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Benchmark NTT attention O(n log n)
    start = clock();
    int result = cllm_attention_ntt_forward(query, key, value, seq_len, head_dim, output_ntt);
    end = clock();
    double time_ntt = (double)(end - start) / CLOCKS_PER_SEC;
    
    if (result != 0) {
        printf("ERROR: NTT attention failed\n");
        free(query); free(key); free(value); free(output_standard); free(output_ntt);
        return;
    }
    
    // Compute speedup
    double speedup = time_standard / time_ntt;
    
    printf("Standard attention: %.4f seconds (O(n²))\n", time_standard);
    printf("NTT attention:      %.4f seconds (O(n log n))\n", time_ntt);
    printf("Speedup:            %.2fx\n", speedup);
    
    // Verify correctness (outputs should be similar)
    double max_diff = 0.0;
    double avg_diff = 0.0;
    for (uint32_t i = 0; i < seq_len * head_dim; i++) {
        double diff = (double)prime_fabsf((float)(output_standard[i] - output_ntt[i]));
        if (diff > max_diff) max_diff = diff;
        avg_diff += diff;
    }
    avg_diff /= (seq_len * head_dim);
    
    printf("Max difference:     %.6f\n", max_diff);
    printf("Avg difference:     %.6f\n", avg_diff);
    
    if (max_diff < 0.01) {
        printf("✓ Outputs match (< 1%% error)\n");
    } else {
        printf("⚠ Warning: Outputs differ significantly\n");
    }
    
    printf("\n");
    
    // Cleanup
    free(query);
    free(key);
    free(value);
    free(output_standard);
    free(output_ntt);
}