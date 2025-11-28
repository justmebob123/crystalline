#ifndef CLLM_SIMPLE_LOSS_H
#define CLLM_SIMPLE_LOSS_H

#include <stdint.h>
#include "cllm.h"  // Need full CLLMModel definition for accessing prime encodings

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Pure Crystalline Loss Function (ASI Design)
 * 
 * Uses deterministic GCD-based similarity with learned prime encodings
 * and lattice positions. This is the core of the crystalline design.
 * 
 * @param model CLLM model with prime encodings
 * @param input_tokens Input token IDs
 * @param target_tokens Target token IDs
 * @param num_tokens Number of tokens
 * @return Average crystalline loss
 */
float cllm_compute_crystalline_loss(
    CLLMModel* model,
    uint32_t* input_tokens,
    uint32_t* target_tokens,
    int num_tokens
);

/**
 * Compute crystalline loss with detailed statistics
 * 
 * @param model CLLM model
 * @param input_tokens Input token IDs
 * @param target_tokens Target token IDs
 * @param num_tokens Number of tokens
 * @param avg_gcd_sim Output: average GCD similarity
 * @param avg_spatial_sim Output: average spatial similarity
 * @return Average crystalline loss
 */
float cllm_compute_crystalline_loss_detailed(
    CLLMModel* model,
    uint32_t* input_tokens,
    uint32_t* target_tokens,
    int num_tokens,
    float* avg_gcd_sim,
    float* avg_spatial_sim
);

/**
 * LEGACY: Standard cross-entropy loss (for comparison only)
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target Target token ID
 * @param vocab_size Vocabulary size
 * @return Cross-entropy loss value
 */
float cllm_compute_cross_entropy_loss(float* logits, uint32_t target, int vocab_size);

/**
 * Compute cross-entropy loss gradient
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target Target token ID
 * @param vocab_size Vocabulary size
 * @param grad_output Output gradients [vocab_size] (caller must allocate)
 */
void cllm_compute_loss_gradient(float* logits, uint32_t target, 
                                int vocab_size, float* grad_output);

#ifdef __cplusplus
}
#endif

#endif // CLLM_SIMPLE_LOSS_H