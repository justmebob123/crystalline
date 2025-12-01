#ifndef CLLM_FEEDFORWARD_BIGFIXED_H
#define CLLM_FEEDFORWARD_BIGFIXED_H

#include "cllm.h"
#include "bigfixed_core.h"

/**
 * Feed-forward layer forward pass using BigFixed arithmetic
 * 
 * @param layer Feed-forward layer
 * @param input Input tensor (BigFixed**)
 * @param output Output tensor (BigFixed**)
 * @param precision BigFixed precision
 */
void cllm_feedforward_bigfixed(
    FeedForwardLayer* layer,
    BigFixed** input,
    BigFixed** output,
    int precision
);

#endif // CLLM_FEEDFORWARD_BIGFIXED_H