#ifndef CLLM_BIGFIXED_H
#define CLLM_BIGFIXED_H

#include "cllm.h"
#include "cllm_training.h"
#include "bigfixed_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * BigFixed Forward Pass Functions
 * ============================================================================ */

void cllm_attention_forward_bigfixed(
    AttentionLayer* layer,
    BigFixed** input,
    BigFixed** output,
    int seq_len,
    int precision
);

void cllm_feedforward_forward_bigfixed(
    FeedForwardLayer* layer,
    BigFixed** input,
    BigFixed** output,
    int batch_size,
    int precision
);

void cllm_layernorm_forward_bigfixed(
    LayerNorm* layer,
    BigFixed** input,
    BigFixed** output,
    int batch_size,
    int precision
);

void cllm_forward_bigfixed(
    CLLMModel* model,
    BigFixed** input,
    BigFixed** output,
    int seq_len,
    int precision
);

/* ============================================================================
 * BigFixed Training Functions
 * ============================================================================ */

void cllm_compute_loss_bigfixed(
    BigFixed** logits,
    uint32_t* targets,
    BigFixed* loss,
    int batch_size,
    int num_classes,
    int precision
);

void cllm_optimizer_step_bigfixed(
    CLLMModel* model,
    CLLMTraining* training,
    float learning_rate,
    int precision
);

float cllm_train_step_bigfixed(
    CLLMModel* model,
    CLLMTraining* training,
    uint32_t* input_tokens,
    uint32_t* target_tokens,
    int seq_len,
    float learning_rate
);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_BIGFIXED_H */
