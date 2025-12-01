/**
 * @file loss_functions_bigfixed.h
 * @brief BigFixed arbitrary precision loss functions
 */

#ifndef LOSS_FUNCTIONS_BIGFIXED_H
#define LOSS_FUNCTIONS_BIGFIXED_H

#include "bigfixed_core.h"
#include <stdint.h>

/**
 * @brief Cross entropy loss with BigFixed
 * @param logits Model logits [batch_size x num_classes]
 * @param targets Target class indices
 * @param loss Output loss value
 * @param batch_size Number of samples
 * @param num_classes Number of classes
 * @param precision Precision bits for computation
 */
void cross_entropy_loss_bigfixed(
    BigFixed** logits,
    uint32_t* targets,
    BigFixed* loss,
    int batch_size,
    int num_classes,
    int precision
);

/**
 * @brief Softmax activation with BigFixed
 * @param logits Input logits (modified in place)
 * @param size Number of values
 * @param precision Precision bits for computation
 */
void softmax_bigfixed(
    BigFixed** logits,
    int size,
    int precision
);

#endif /* LOSS_FUNCTIONS_BIGFIXED_H */
