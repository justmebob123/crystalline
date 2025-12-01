#ifndef LOSS_FUNCTIONS_BIGFIXED_H
#define LOSS_FUNCTIONS_BIGFIXED_H

#include "bigfixed_core.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Cross-entropy loss with BigFixed precision
 */
void cross_entropy_loss_bigfixed(
    BigFixed** predictions,
    uint32_t* targets,
    BigFixed* loss,
    uint32_t batch_size,
    uint32_t num_classes,
    int precision
);

/**
 * Softmax with BigFixed precision
 */
void softmax_bigfixed(
    BigFixed** input,
    BigFixed** output,
    uint32_t size,
    int precision
);

#ifdef __cplusplus
}
#endif

#endif // LOSS_FUNCTIONS_BIGFIXED_H