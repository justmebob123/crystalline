#ifndef CLLM_NTT_ATTENTION_ADAPTER_H
#define CLLM_NTT_ATTENTION_ADAPTER_H

/**
 * @file cllm_ntt_attention_adapter.h
 * @brief CLLM NTT Attention Adapter
 * 
 * Provides integration between CLLM and the algorithm library's NTT attention.
 * Enables O(n log n) attention computation for long sequences.
 * 
 * Features:
 * - Automatic switching between standard and NTT attention based on sequence length
 * - Integration with CLLM's hierarchical threading system
 * - Support for both single and multi-head attention
 * - Double precision for training stability
 */

#include "cllm_algorithm_adapter.h"
#include "../../algorithms/include/ntt_attention.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONFIGURATION
// ============================================================================

/**
 * NTT Attention Configuration for CLLM
 * 
 * Controls when and how to use NTT attention in CLLM training.
 */
typedef struct {
    uint32_t min_seq_len;        // Minimum sequence length to use NTT (default: 256)
    uint32_t head_dim;           // Attention head dimension
    uint32_t num_heads;          // Number of attention heads
    double scale_factor;         // Attention scaling factor (1/sqrt(head_dim))
    bool enabled;                // Whether NTT attention is enabled
    bool force_ntt;              // Force NTT even for short sequences (for testing)
} CLLMNTTAttentionConfig;

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Initialize NTT attention for CLLM adapter
 * 
 * Sets up NTT attention configuration for a CLLM adapter.
 * 
 * @param adapter CLLM adapter instance
 * @param head_dim Attention head dimension
 * @param num_heads Number of attention heads
 * @param min_seq_len Minimum sequence length to use NTT (0 = auto)
 * @return 0 on success, -1 on error
 */
int cllm_adapter_init_ntt_attention(
    CLLMAlgorithmAdapter* adapter,
    uint32_t head_dim,
    uint32_t num_heads,
    uint32_t min_seq_len
);

/**
 * Free NTT attention resources
 * 
 * @param adapter CLLM adapter instance
 */
void cllm_adapter_free_ntt_attention(CLLMAlgorithmAdapter* adapter);

// ============================================================================
// ATTENTION OPERATIONS
// ============================================================================

/**
 * Compute attention using NTT (single head, double precision)
 * 
 * Automatically chooses between standard and NTT attention based on
 * sequence length and configuration.
 * 
 * @param adapter CLLM adapter instance
 * @param output Output attention values [seq_len x head_dim]
 * @param queries Query vectors [seq_len x head_dim]
 * @param keys Key vectors [seq_len x head_dim]
 * @param values Value vectors [seq_len x head_dim]
 * @param seq_len Sequence length
 * @return 0 on success, -1 on error
 */
int cllm_adapter_compute_attention_single_head(
    CLLMAlgorithmAdapter* adapter,
    double* output,
    const double* queries,
    const double* keys,
    const double* values,
    uint32_t seq_len
);

/**
 * Compute multi-head attention using NTT (double precision)
 * 
 * Processes multiple attention heads in parallel.
 * 
 * @param adapter CLLM adapter instance
 * @param output Output attention values [num_heads x seq_len x head_dim]
 * @param queries Query matrix [num_heads x seq_len x head_dim]
 * @param keys Key matrix [num_heads x seq_len x head_dim]
 * @param values Value matrix [num_heads x seq_len x head_dim]
 * @param seq_len Sequence length
 * @return 0 on success, -1 on error
 */
int cllm_adapter_compute_attention_multi_head(
    CLLMAlgorithmAdapter* adapter,
    double* output,
    const double* queries,
    const double* keys,
    const double* values,
    uint32_t seq_len
);

// ============================================================================
// CONFIGURATION MANAGEMENT
// ============================================================================

/**
 * Get NTT attention configuration
 * 
 * @param adapter CLLM adapter instance
 * @return Configuration pointer, or NULL if not initialized
 */
const CLLMNTTAttentionConfig* cllm_adapter_get_ntt_config(
    const CLLMAlgorithmAdapter* adapter
);

/**
 * Update NTT attention configuration
 * 
 * @param adapter CLLM adapter instance
 * @param config New configuration
 * @return 0 on success, -1 on error
 */
int cllm_adapter_update_ntt_config(
    CLLMAlgorithmAdapter* adapter,
    const CLLMNTTAttentionConfig* config
);

/**
 * Enable/disable NTT attention
 * 
 * @param adapter CLLM adapter instance
 * @param enabled Whether to enable NTT attention
 */
void cllm_adapter_set_ntt_enabled(
    CLLMAlgorithmAdapter* adapter,
    bool enabled
);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * NTT Attention Statistics
 */
typedef struct {
    uint64_t total_attention_calls;     // Total attention computations
    uint64_t ntt_attention_calls;       // Calls using NTT
    uint64_t standard_attention_calls;  // Calls using standard attention
    uint64_t total_tokens_processed;    // Total tokens processed
    double avg_sequence_length;         // Average sequence length
    double ntt_speedup_factor;          // Average speedup from NTT
} CLLMNTTAttentionStats;

/**
 * Get NTT attention statistics
 * 
 * @param adapter CLLM adapter instance
 * @param stats Output statistics structure
 * @return 0 on success, -1 on error
 */
int cllm_adapter_get_ntt_stats(
    const CLLMAlgorithmAdapter* adapter,
    CLLMNTTAttentionStats* stats
);

/**
 * Reset NTT attention statistics
 * 
 * @param adapter CLLM adapter instance
 */
void cllm_adapter_reset_ntt_stats(CLLMAlgorithmAdapter* adapter);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_NTT_ATTENTION_ADAPTER_H */