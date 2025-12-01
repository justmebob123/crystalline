/**
 * @file optimizers_bigfixed.h
 * @brief BigFixed arbitrary precision optimizers
 */

#ifndef OPTIMIZERS_BIGFIXED_H
#define OPTIMIZERS_BIGFIXED_H

#include "bigfixed_core.h"

/**
 * @brief Adam optimizer step with BigFixed
 * @param weights Model weights
 * @param gradients Gradients
 * @param m First moment estimates
 * @param v Second moment estimates
 * @param num_params Number of parameters
 * @param learning_rate Learning rate
 * @param beta1 First moment decay rate
 * @param beta2 Second moment decay rate
 * @param epsilon Small constant for numerical stability
 * @param precision Precision bits for computation
 */
void adam_step_bigfixed(
    BigFixed** weights,
    BigFixed** gradients,
    BigFixed** m,
    BigFixed** v,
    int num_params,
    float learning_rate,
    float beta1,
    float beta2,
    float epsilon,
    int precision
);

/**
 * @brief SGD optimizer step with BigFixed
 * @param weights Model weights
 * @param gradients Gradients
 * @param num_params Number of parameters
 * @param learning_rate Learning rate
 * @param precision Precision bits for computation
 */
void sgd_step_bigfixed(
    BigFixed** weights,
    BigFixed** gradients,
    int num_params,
    float learning_rate,
    int precision
);

#endif /* OPTIMIZERS_BIGFIXED_H */
