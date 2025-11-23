#ifndef CLLM_FORWARD_BACKWARD_PROPER_H
#define CLLM_FORWARD_BACKWARD_PROPER_H

#include "cllm_training.h"
#include "cllm_activations.h"

/**
 * Forward pass with activation storage
 */
float cllm_forward_with_activations(
    CLLMTraining* training,
    uint32_t* input_tokens,
    ForwardActivations* activations
);

/**
 * Compute loss from stored activations
 */
float cllm_compute_loss_from_activations(
    ForwardActivations* activations,
    uint32_t* target_tokens
);

/**
 * Backward pass for cross-entropy loss
 */
void cllm_backward_cross_entropy(
    CLLMTraining* training,
    ForwardActivations* activations,
    uint32_t* target_tokens
);

#endif // CLLM_FORWARD_BACKWARD_PROPER_H