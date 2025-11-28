#ifndef CLLM_SIMPLE_LOSS_H
#define CLLM_SIMPLE_LOSS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute cross-entropy loss for a single prediction
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