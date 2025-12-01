#ifndef CLLM_LAYERNORM_BIGFIXED_H
#define CLLM_LAYERNORM_BIGFIXED_H

#include "cllm.h"
#include "bigfixed_core.h"

/**
 * Layer normalization using BigFixed arithmetic
 * 
 * @param ln Layer normalization parameters
 * @param input Input tensor (BigFixed**)
 * @param output Output tensor (BigFixed**)
 * @param precision BigFixed precision
 */
void cllm_layer_norm_bigfixed(
    CLLMLayerNorm* ln,
    BigFixed** input,
    BigFixed** output,
    int precision
);

#endif // CLLM_LAYERNORM_BIGFIXED_H