#ifndef OPTIMIZERS_BIGFIXED_H
#define OPTIMIZERS_BIGFIXED_H

#include "bigfixed_core.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SGD optimizer step with BigFixed precision
 */
void sgd_step_bigfixed(
    BigFixed** weights,
    BigFixed** gradients,
    double learning_rate,
    uint32_t size,
    int precision
);

/**
 * Adam optimizer step with BigFixed precision
 */
void adam_step_bigfixed(
    BigFixed** weights,
    BigFixed** gradients,
    BigFixed** m,
    BigFixed** v,
    double learning_rate,
    double beta1,
    double beta2,
    double epsilon,
    int step,
    uint32_t size,
    int precision
);

#ifdef __cplusplus
}
#endif

#endif // OPTIMIZERS_BIGFIXED_H