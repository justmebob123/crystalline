/**
 * @file cllm_ntt_attention.c
 * @brief NTT-Based O(n log n) Fast Attention
 * 
 * ATTENTION STRATEGY: Number Theoretic Transform (NTT)
 * - O(n log n) complexity vs O(n²) standard attention
 * - Uses NTT for fast convolution
 * - Ideal for long sequences (>1024 tokens)
 * - Maintains attention quality with better performance
 * 
 * RELATIONSHIP TO OTHER ATTENTION FILES:
 * - cllm_ntt_attention.c: THIS FILE - Fast O(n log n) attention
 * - cllm_attention.c: Main attention (O(n²) but full-featured)
 * - cllm_angular_attention.c: Angular position formula
 * 
 * USE THIS WHEN:
 * - Long sequences (>1024 tokens)
 * - Performance is critical
 * - Need O(n log n) complexity
 * - Can trade some features for speed
 * 
 * USE cllm_attention.c WHEN:
 * - Short sequences (<1024 tokens)
 * - Need all crystalline features
 * - Performance not critical
 * 
 * CLLM-specific wrapper around the general NTT attention algorithm.
 * 
 * Layer Architecture:
 * - Layer 1 (Crystalline): Pure math (NTT transforms, prime operations)
 * - Layer 2 (Algorithms): General NTT attention algorithm
 * - Layer 3 (CLLM): This wrapper - extracts CLLM data, calls Layer 2
 * 
 * This file implements OBJECTIVE 17: Implement NTT-Based O(n log n) Attention
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/cllm.h"
#include "../include/ai/cllm_ntt_attention.h"
#include "../../algorithms/include/ntt_attention.h"
#include "../include/prime_float_math.h"
#include "../include/bigfixed_core.h"
#include "../include/bigfixed_array_utils.h"

/**
 * Compute attention using NTT (O(n log n) complexity)
 * 
 * CLLM wrapper that calls the general NTT attention from algorithms layer.
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
    
    // Calculate scale factor (1/sqrt(head_dim))
    double scale_factor = 1.0 / prime_sqrt((double)head_dim);
    
    // Call general NTT attention from algorithms layer
    int result = ntt_attention_forward(
        output,
        query,
        key,
        value,
        seq_len,
        head_dim,
        scale_factor
    );
    
    return result ? 0 : -1;
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
    if (!query || !key || !value || !output || seq_len == 0 || head_dim == 0) {
        return;
    }
    
    // Calculate scale factor
    double scale = 1.0 / prime_sqrt((double)head_dim);
    
    // Allocate attention scores
    double* scores = calloc(seq_len * seq_len, sizeof(double));
    if (!scores) return;
    
    // Compute attention scores: Q * K^T / sqrt(d_k)
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            double score = 0.0;
            for (uint32_t d = 0; d < head_dim; d++) {
                score += query[i * head_dim + d] * key[j * head_dim + d];
            }
            scores[i * seq_len + j] = score * scale;
        }
    }
    
    // Apply softmax to each row
    for (uint32_t i = 0; i < seq_len; i++) {
        double* row = &scores[i * seq_len];
        
        // Find max for numerical stability
        double max_score = row[0];
        for (uint32_t j = 1; j < seq_len; j++) {
            if (row[j] > max_score) max_score = row[j];
        }
        
        // Compute exp and sum
        double sum = 0.0;
        for (uint32_t j = 0; j < seq_len; j++) {
            row[j] = prime_exp(row[j] - max_score);
            sum += row[j];
        }
        
        // Normalize
        if (sum > 0.0) {
            for (uint32_t j = 0; j < seq_len; j++) {
                row[j] /= sum;
            }
        }
    }
    
    // Compute weighted sum: scores * V
    memset(output, 0, seq_len * head_dim * sizeof(float));
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            double weight = scores[i * seq_len + j];
            for (uint32_t d = 0; d < head_dim; d++) {
                output[i * head_dim + d] += weight * value[j * head_dim + d];
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
void benchmark_ntt_attention(uint32_t seq_len, uint32_t head_dim)
{
    printf("Benchmarking NTT Attention vs Standard Attention\n");
    printf("Sequence Length: %u, Head Dimension: %u\n\n", seq_len, head_dim);
    
    // Allocate test data
    float* query = calloc(seq_len * head_dim, sizeof(float));
    float* key = calloc(seq_len * head_dim, sizeof(float));
    float* value = calloc(seq_len * head_dim, sizeof(float));
    float* output_ntt = calloc(seq_len * head_dim, sizeof(float));
    float* output_std = calloc(seq_len * head_dim, sizeof(float));
    
    if (!query || !key || !value || !output_ntt || !output_std) {
        printf("ERROR: Memory allocation failed\n");
        goto cleanup;
    }
    
    // Initialize with random data
    for (uint32_t i = 0; i < seq_len * head_dim; i++) {
        query[i] = (float)rand() / RAND_MAX - 0.5;
        key[i] = (float)rand() / RAND_MAX - 0.5;
        value[i] = (float)rand() / RAND_MAX - 0.5;
    }
    
    // Benchmark standard attention
    clock_t start = clock();
    cllm_attention_standard_forward(query, key, value, seq_len, head_dim, output_std);
    clock_t end = clock();
    double time_std = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Benchmark NTT attention
    start = clock();
    cllm_attention_ntt_forward(query, key, value, seq_len, head_dim, output_ntt);
    end = clock();
    double time_ntt = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Calculate speedup
    double speedup = time_std / time_ntt;
    
    // Estimate theoretical speedup
    double theoretical_speedup = ntt_attention_estimate_speedup(seq_len);
    
    printf("Results:\n");
    printf("  Standard Attention: %.6f seconds\n", time_std);
    printf("  NTT Attention:      %.6f seconds\n", time_ntt);
    printf("  Actual Speedup:     %.2fx\n", speedup);
    printf("  Theoretical:        %.2fx\n", theoretical_speedup);
    printf("\n");
    
    // Verify correctness (outputs should be similar)
    double max_diff = 0.0;
    for (uint32_t i = 0; i < seq_len * head_dim; i++) {
        double diff = prime_fabsf(output_ntt[i] - output_std[i]);
        if (diff > max_diff) max_diff = diff;
    }
    printf("  Max Difference:     %.6f\n", max_diff);
    
    if (max_diff < 0.01) {
        printf("  ✓ Correctness verified (difference < 0.01)\n");
    } else {
        printf("  ⚠ Large difference detected (may need tuning)\n");
    }
    
cleanup:
    free(query);
    free(key);
    free(value);
    free(output_ntt);
    free(output_std);
}
/**
 * NTT Attention with BigFixed Support
 * 
 * Wrapper that converts BigFixed** to float*, applies NTT attention,
 * and converts back to BigFixed**.
 * 
 * @param query Query matrix [seq_len x head_dim] (BigFixed**)
 * @param key Key matrix [seq_len x head_dim] (BigFixed**)
 * @param value Value matrix [seq_len x head_dim] (BigFixed**)
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 * @param output Output matrix [seq_len x head_dim] (BigFixed**)
 * @param precision Precision bits for BigFixed
 * @return 0 on success, -1 on failure
 */
int cllm_attention_ntt_forward_bigfixed(
    BigFixed** query,
    BigFixed** key,
    BigFixed** value,
    uint32_t seq_len,
    uint32_t head_dim,
    BigFixed** output,
    int precision)
{
    (void)precision;  // Reserved for future precision-aware conversion
    
    if (!query || !key || !value || !output || seq_len == 0 || head_dim == 0) {
        return -1;
    }
    
    size_t total_size = seq_len * head_dim;
    
    // Allocate float arrays
    float* query_f = (float*)calloc(total_size, sizeof(float));
    float* key_f = (float*)calloc(total_size, sizeof(float));
    float* value_f = (float*)calloc(total_size, sizeof(float));
    float* output_f = (float*)calloc(total_size, sizeof(float));
    
    if (!query_f || !key_f || !value_f || !output_f) {
        free(query_f);
        free(key_f);
        free(value_f);
        free(output_f);
        return -1;
    }
    
    // Convert BigFixed** to float*
    bigfixed_array_to_float(query_f, query, total_size);
    bigfixed_array_to_float(key_f, key, total_size);
    bigfixed_array_to_float(value_f, value, total_size);
    
    // Call NTT attention (O(n log n))
    int result = cllm_attention_ntt_forward(query_f, key_f, value_f, 
                                            seq_len, head_dim, output_f);
    
    if (result == 0) {
        // Convert back to BigFixed**
        bigfixed_array_from_float(output, output_f, total_size);
    }
    
    // Cleanup
    free(query_f);
    free(key_f);
    free(value_f);
    free(output_f);
    
    return result;
}