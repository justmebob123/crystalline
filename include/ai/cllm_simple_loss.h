#ifndef CLLM_SIMPLE_LOSS_H
#define CLLM_SIMPLE_LOSS_H

#include <stdint.h>
#include "cllm.h"  // Need full CLLMModel definition for accessing prime encodings

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute Loss (The ONLY Loss Function)
 * 
 * Uses deterministic GCD-based similarity with learned prime encodings
 * and lattice positions. This is the core of the crystalline design.
 * 
 * @param model CLLM model with prime encodings
 * @param input_tokens Input token IDs
 * @param target_tokens Target token IDs
 * @param num_tokens Number of tokens
 * @return Average loss
 */
float cllm_compute_loss(
    CLLMModel* model,
    uint32_t* input_tokens,
    uint32_t* target_tokens,
    int num_tokens
);

/**
 * Compute loss with detailed statistics
 * 
 * @param model CLLM model
 * @param input_tokens Input token IDs
 * @param target_tokens Target token IDs
 * @param num_tokens Number of tokens
 * @param avg_gcd_sim Output: average GCD similarity
 * @param avg_spatial_sim Output: average spatial similarity
 * @return Average loss
 */
float cllm_compute_loss_detailed(
    CLLMModel* model,
    uint32_t* input_tokens,
    uint32_t* target_tokens,
    int num_tokens,
    float* avg_gcd_sim,
    float* avg_spatial_sim
);

/**
 * Compute loss gradient for backpropagation
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target Target token ID
 * @param vocab_size Vocabulary size
 * @param grad_output Output gradients [vocab_size] (caller must allocate)
 */
void cllm_compute_loss_gradient(float* logits, uint32_t target, 
                                int vocab_size, float* grad_output);

/**
 * Compute perplexity from loss
 * 
 * @param loss Loss value
 * @return Perplexity value
 */
float cllm_compute_perplexity(float loss);

/**
 * Compute accuracy
 * 
 * @param logits Predicted logits [batch_size x vocab_size]
 * @param targets Target token IDs [batch_size]
 * @param batch_size Batch size
 * @param vocab_size Vocabulary size
 * @return Accuracy (0 to 1)
 */
float cllm_compute_accuracy(float* logits, uint32_t* targets,
                           int batch_size, int vocab_size);

/**
 * Compute top-k accuracy
 * 
 * @param logits Predicted logits [batch_size x vocab_size]
 * @param targets Target token IDs [batch_size]
 * @param batch_size Batch size
 * @param vocab_size Vocabulary size
 * @param k Top-k value
 * @return Top-k accuracy (0 to 1)
 */
float cllm_compute_top_k_accuracy(float* logits, uint32_t* targets,
                                  int batch_size, int vocab_size, int k);

#ifdef __cplusplus
}
#endif

#endif // CLLM_SIMPLE_LOSS_H