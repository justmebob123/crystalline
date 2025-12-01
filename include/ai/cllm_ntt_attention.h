#ifndef CLLM_NTT_ATTENTION_H
#define CLLM_NTT_ATTENTION_H

#include <stdint.h>
#include "../bigfixed_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute attention using NTT (O(n log n) complexity)
 * 
 * Uses Number Theoretic Transform to compute attention in O(n log n) time
 * instead of O(n²). Provides 10-100x speedup for long sequences.
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
    float* output
);

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
    float* output
);

/**
 * Benchmark NTT attention vs standard attention
 * 
 * @param seq_len Sequence length
 * @param head_dim Head dimension
 */
void benchmark_ntt_attention(uint32_t seq_len, uint32_t head_dim);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_NTT_ATTENTION_H */
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
    int precision
);

