/**
 * @file cllm_embedding_helpers.h
 * @brief Helper Functions for Thread-Based Embedding Access
 */

#ifndef CLLM_EMBEDDING_HELPERS_H
#define CLLM_EMBEDDING_HELPERS_H

#include "cllm.h"
#include "hierarchical_threading.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// THREAD-BASED EMBEDDING ACCESS
// ============================================================================

/**
 * Get embedding from thread's CrystallineAbacus
 */
bool cllm_get_embedding_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t embedding_dim
);

/**
 * Set embedding in thread's CrystallineAbacus
 */
bool cllm_set_embedding_to_thread(
    HierarchicalThread* thread,
    const double* input,
    uint32_t embedding_dim
);

/**
 * Get embedding by token ID from thread storage (convenience function)
 * Renamed to avoid conflict with cllm_inference.h
 */
bool cllm_get_embedding_from_model(
    const CLLMModel* model,
    uint32_t token_id,
    double* output
);

/**
 * Set embedding by token ID (convenience function)
 */
bool cllm_set_embedding(
    CLLMModel* model,
    uint32_t token_id,
    const double* input
);

/**
 * Get pointer to embedding (read-only)
 * 
 * WARNING: Do not store this pointer - it may become invalid
 */
const double* cllm_get_embedding_ptr(
    const CLLMModel* model,
    uint32_t token_id
);

/**
 * Get mutable pointer to embedding
 * 
 * WARNING: Do not store this pointer - it may become invalid
 */
double* cllm_get_embedding_ptr_mut(
    CLLMModel* model,
    uint32_t token_id
);

/**
 * Copy embedding from one token to another
 */
bool cllm_copy_embedding(
    CLLMModel* model,
    uint32_t src_token_id,
    uint32_t dst_token_id
);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_EMBEDDING_HELPERS_H */