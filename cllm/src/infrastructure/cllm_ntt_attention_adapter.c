/**
 * @file cllm_ntt_attention_adapter.c
 * @brief CLLM NTT Attention Adapter Implementation
 * 
 * Integrates the algorithm library's NTT attention with CLLM's
 * hierarchical threading system.
 */

#include "ai/cllm_ntt_attention_adapter.h"
#include "ai/cllm_lattice_hierarchy.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// ============================================================================
// INTERNAL STRUCTURES
// ============================================================================

/**
 * NTT Attention Context
 * 
 * Stored in the adapter's user_data field.
 */
typedef struct {
    CLLMNTTAttentionConfig config;
    CLLMNTTAttentionStats stats;
    bool initialized;
} NTTAttentionContext;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Get NTT attention context from adapter
 */
static NTTAttentionContext* get_ntt_context(CLLMAlgorithmAdapter* adapter) {
    if (!adapter) {
        return NULL;
    }
    return (NTTAttentionContext*)adapter->ntt_attention_context;
}

/**
 * Standard attention fallback (O(n²) implementation)
 * 
 * Used for short sequences or when NTT is disabled.
 */
static int standard_attention_single_head(
    double* output,
    const double* queries,
    const double* keys,
    const double* values,
    uint32_t seq_len,
    uint32_t head_dim,
    double scale_factor
) {
    if (!output || !queries || !keys || !values || seq_len == 0 || head_dim == 0) {
        return -1;
    }
    
    // Allocate attention scores matrix [seq_len x seq_len]
    double* scores = calloc(seq_len * seq_len, sizeof(double));
    if (!scores) {
        return -1;
    }
    
    // Compute Q * K^T
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            double score = 0.0;
            for (uint32_t k = 0; k < head_dim; k++) {
                score += queries[i * head_dim + k] * keys[j * head_dim + k];
            }
            scores[i * seq_len + j] = score * scale_factor;
        }
    }
    
    // Apply softmax to each row
    for (uint32_t i = 0; i < seq_len; i++) {
        double* row = &scores[i * seq_len];
        
        // Find max for numerical stability
        double max_score = row[0];
        for (uint32_t j = 1; j < seq_len; j++) {
            if (row[j] > max_score) {
                max_score = row[j];
            }
        }
        
        // Compute exp and sum
        double sum = 0.0;
        for (uint32_t j = 0; j < seq_len; j++) {
            row[j] = exp(row[j] - max_score);
            sum += row[j];
        }
        
        // Normalize
        for (uint32_t j = 0; j < seq_len; j++) {
            row[j] /= sum;
        }
    }
    
    // Compute attention * V
    memset(output, 0, seq_len * head_dim * sizeof(double));
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            double weight = scores[i * seq_len + j];
            for (uint32_t k = 0; k < head_dim; k++) {
                output[i * head_dim + k] += weight * values[j * head_dim + k];
            }
        }
    }
    
    free(scores);
    return 0;
}

// ============================================================================
// INITIALIZATION
// ============================================================================

int cllm_adapter_init_ntt_attention(
    CLLMAlgorithmAdapter* adapter,
    uint32_t head_dim,
    uint32_t num_heads,
    uint32_t min_seq_len
) {
    if (!adapter) {
        return -1;
    }
    
    // Allocate context
    NTTAttentionContext* ctx = calloc(1, sizeof(NTTAttentionContext));
    if (!ctx) {
        return -1;
    }
    
    // Initialize configuration
    ctx->config.min_seq_len = (min_seq_len > 0) ? min_seq_len : 256;
    ctx->config.head_dim = head_dim;
    ctx->config.num_heads = num_heads;
    ctx->config.scale_factor = 1.0 / sqrt((double)head_dim);
    ctx->config.enabled = true;
    ctx->config.force_ntt = false;
    
    // Initialize statistics
    memset(&ctx->stats, 0, sizeof(CLLMNTTAttentionStats));
    
    ctx->initialized = true;
    
    // Store in adapter
    adapter->ntt_attention_context = ctx;
    
    return 0;
}

void cllm_adapter_free_ntt_attention(CLLMAlgorithmAdapter* adapter) {
    if (!adapter) {
        return;
    }
    
    NTTAttentionContext* ctx = get_ntt_context(adapter);
    if (ctx) {
        free(ctx);
        adapter->ntt_attention_context = NULL;
    }
}

// ============================================================================
// ATTENTION OPERATIONS
// ============================================================================

int cllm_adapter_compute_attention_single_head(
    CLLMAlgorithmAdapter* adapter,
    double* output,
    const double* queries,
    const double* keys,
    const double* values,
    uint32_t seq_len
) {
    if (!adapter || !output || !queries || !keys || !values || seq_len == 0) {
        return -1;
    }
    
    NTTAttentionContext* ctx = get_ntt_context(adapter);
    if (!ctx || !ctx->initialized) {
        fprintf(stderr, "Error: NTT attention not initialized\n");
        return -1;
    }
    
    // Update statistics
    ctx->stats.total_attention_calls++;
    ctx->stats.total_tokens_processed += seq_len;
    
    // Decide whether to use NTT or standard attention
    bool use_ntt = ctx->config.enabled && 
                   (ctx->config.force_ntt || seq_len >= ctx->config.min_seq_len);
    
    int result;
    if (use_ntt) {
        // Use NTT attention (O(n log n))
        result = ntt_attention_single_head_double(
            output,
            queries,
            keys,
            values,
            seq_len,
            ctx->config.head_dim,
            ctx->config.scale_factor
        );
        
        if (result == 0) {
            ctx->stats.ntt_attention_calls++;
        } else {
            // Fall back to standard if NTT fails
            fprintf(stderr, "Warning: NTT attention failed, falling back to standard\n");
            result = standard_attention_single_head(
                output, queries, keys, values,
                seq_len, ctx->config.head_dim, ctx->config.scale_factor
            );
            ctx->stats.standard_attention_calls++;
        }
    } else {
        // Use standard attention (O(n²))
        result = standard_attention_single_head(
            output, queries, keys, values,
            seq_len, ctx->config.head_dim, ctx->config.scale_factor
        );
        ctx->stats.standard_attention_calls++;
    }
    
    // Update average sequence length
    if (ctx->stats.total_attention_calls > 0) {
        ctx->stats.avg_sequence_length = 
            (double)ctx->stats.total_tokens_processed / ctx->stats.total_attention_calls;
    }
    
    // Estimate speedup factor
    if (ctx->stats.ntt_attention_calls > 0) {
        // Theoretical speedup: O(n²) / O(n log n) = n / log n
        double avg_len = ctx->stats.avg_sequence_length;
        if (avg_len > 1) {
            ctx->stats.ntt_speedup_factor = avg_len / log2(avg_len);
        }
    }
    
    return result;
}

int cllm_adapter_compute_attention_multi_head(
    CLLMAlgorithmAdapter* adapter,
    double* output,
    const double* queries,
    const double* keys,
    const double* values,
    uint32_t seq_len
) {
    if (!adapter || !output || !queries || !keys || !values || seq_len == 0) {
        return -1;
    }
    
    NTTAttentionContext* ctx = get_ntt_context(adapter);
    if (!ctx || !ctx->initialized) {
        fprintf(stderr, "Error: NTT attention not initialized\n");
        return -1;
    }
    
    // Update statistics
    ctx->stats.total_attention_calls += ctx->config.num_heads;
    ctx->stats.total_tokens_processed += seq_len * ctx->config.num_heads;
    
    // Decide whether to use NTT or standard attention
    bool use_ntt = ctx->config.enabled && 
                   (ctx->config.force_ntt || seq_len >= ctx->config.min_seq_len);
    
    int result;
    if (use_ntt) {
        // Use NTT multi-head attention
        result = ntt_attention_multi_head_double(
            output,
            queries,
            keys,
            values,
            seq_len,
            ctx->config.head_dim,
            ctx->config.num_heads,
            ctx->config.scale_factor
        );
        
        if (result == 0) {
            ctx->stats.ntt_attention_calls += ctx->config.num_heads;
        } else {
            // Fall back to processing heads individually
            fprintf(stderr, "Warning: Multi-head NTT failed, processing heads individually\n");
            for (uint32_t h = 0; h < ctx->config.num_heads; h++) {
                uint32_t head_offset = h * seq_len * ctx->config.head_dim;
                int head_result = cllm_adapter_compute_attention_single_head(
                    adapter,
                    output + head_offset,
                    queries + head_offset,
                    keys + head_offset,
                    values + head_offset,
                    seq_len
                );
                if (head_result != 0) {
                    return -1;
                }
            }
            result = 0;
        }
    } else {
        // Process each head with standard attention
        for (uint32_t h = 0; h < ctx->config.num_heads; h++) {
            uint32_t head_offset = h * seq_len * ctx->config.head_dim;
            result = standard_attention_single_head(
                output + head_offset,
                queries + head_offset,
                keys + head_offset,
                values + head_offset,
                seq_len,
                ctx->config.head_dim,
                ctx->config.scale_factor
            );
            if (result != 0) {
                return -1;
            }
        }
        ctx->stats.standard_attention_calls += ctx->config.num_heads;
    }
    
    // Update statistics
    if (ctx->stats.total_attention_calls > 0) {
        ctx->stats.avg_sequence_length = 
            (double)ctx->stats.total_tokens_processed / ctx->stats.total_attention_calls;
    }
    
    if (ctx->stats.ntt_attention_calls > 0) {
        double avg_len = ctx->stats.avg_sequence_length;
        if (avg_len > 1) {
            ctx->stats.ntt_speedup_factor = avg_len / log2(avg_len);
        }
    }
    
    return result;
}

// ============================================================================
// CONFIGURATION MANAGEMENT
// ============================================================================

const CLLMNTTAttentionConfig* cllm_adapter_get_ntt_config(
    const CLLMAlgorithmAdapter* adapter
) {
    if (!adapter) {
        return NULL;
    }
    
    NTTAttentionContext* ctx = (NTTAttentionContext*)adapter->ntt_attention_context;
    if (!ctx || !ctx->initialized) {
        return NULL;
    }
    
    return &ctx->config;
}

int cllm_adapter_update_ntt_config(
    CLLMAlgorithmAdapter* adapter,
    const CLLMNTTAttentionConfig* config
) {
    if (!adapter || !config) {
        return -1;
    }
    
    NTTAttentionContext* ctx = get_ntt_context(adapter);
    if (!ctx || !ctx->initialized) {
        return -1;
    }
    
    // Update configuration
    ctx->config = *config;
    
    return 0;
}

void cllm_adapter_set_ntt_enabled(
    CLLMAlgorithmAdapter* adapter,
    bool enabled
) {
    if (!adapter) {
        return;
    }
    
    NTTAttentionContext* ctx = get_ntt_context(adapter);
    if (ctx && ctx->initialized) {
        ctx->config.enabled = enabled;
    }
}

// ============================================================================
// STATISTICS
// ============================================================================

int cllm_adapter_get_ntt_stats(
    const CLLMAlgorithmAdapter* adapter,
    CLLMNTTAttentionStats* stats
) {
    if (!adapter || !stats) {
        return -1;
    }
    
    NTTAttentionContext* ctx = (NTTAttentionContext*)adapter->ntt_attention_context;
    if (!ctx || !ctx->initialized) {
        return -1;
    }
    
    *stats = ctx->stats;
    return 0;
}

void cllm_adapter_reset_ntt_stats(CLLMAlgorithmAdapter* adapter) {
    if (!adapter) {
        return;
    }
    
    NTTAttentionContext* ctx = get_ntt_context(adapter);
    if (ctx && ctx->initialized) {
        memset(&ctx->stats, 0, sizeof(CLLMNTTAttentionStats));
    }
}