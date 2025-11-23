#ifndef CLLM_TRAINING_PROPER_H
#define CLLM_TRAINING_PROPER_H

#include "cllm.h"
#include "cllm_training.h"

/**
 * Forward pass for training with activation storage
 */
void cllm_forward_pass_training(CLLMTraining* training, uint32_t* input_tokens,
                               int batch_size, int seq_len, float* output_logits);

/**
 * Compute cross-entropy loss from logits (batch version)
 */
float cllm_compute_cross_entropy_loss_batch(float* logits, uint32_t* target_tokens,
                                           int num_tokens, uint32_t vocab_size);

/**
 * Train one epoch with proper forward/backward pass
 */
float cllm_train_epoch_proper(CLLMTraining* training);

#endif // CLLM_TRAINING_PROPER_H