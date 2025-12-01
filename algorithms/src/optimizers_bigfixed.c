#include "optimizers.h"
#include "bigfixed_core.h"
#include "prime_bigint_transcendental.h"
#include <stdlib.h>

/*
 * BigFixed Optimizers
 * Arbitrary precision optimization algorithms
 */

void adam_step_bigfixed(
    BigFixed** weights,
    BigFixed** gradients,
    BigFixed** m,  // First moment
    BigFixed** v,  // Second moment
    int num_params,
    float learning_rate_float,
    float beta1_float,
    float beta2_float,
    float epsilon_float,
    int precision
) {
    if (!weights || !gradients || !m || !v) return;
    
    BigFixed learning_rate, beta1, beta2, epsilon;
    big_fixed_create_init(&learning_rate, precision);
    big_fixed_create_init(&beta1, precision);
    big_fixed_create_init(&beta2, precision);
    big_fixed_create_init(&epsilon, precision);
    
    big_fixed_from_double(&learning_rate, learning_rate_float);
    big_fixed_from_double(&beta1, beta1_float);
    big_fixed_from_double(&beta2, beta2_float);
    big_fixed_from_double(&epsilon, epsilon_float);
    
    BigFixed one;
    big_fixed_create_init(&one, precision);
    big_fixed_from_int(&one, 1);
    
    BigFixed one_minus_beta1, one_minus_beta2;
    big_fixed_create_init(&one_minus_beta1, precision);
    big_fixed_create_init(&one_minus_beta2, precision);
    big_fixed_sub(&one_minus_beta1, &one, &beta1);
    big_fixed_sub(&one_minus_beta2, &one, &beta2);
    
    for (int i = 0; i < num_params; i++) {
        if (!weights[i] || !gradients[i] || !m[i] || !v[i]) continue;
        
        // m = beta1 * m + (1 - beta1) * g
        BigFixed temp1, temp2;
        big_fixed_create_init(&temp1, precision);
        big_fixed_create_init(&temp2, precision);
        
        big_fixed_mul(&temp1, &beta1, m[i]);
        big_fixed_mul(&temp2, &one_minus_beta1, gradients[i]);
        big_fixed_add(m[i], &temp1, &temp2);
        
        // v = beta2 * v + (1 - beta2) * g^2
        BigFixed g_squared;
        big_fixed_create_init(&g_squared, precision);
        big_fixed_mul(&g_squared, gradients[i], gradients[i]);
        
        big_fixed_mul(&temp1, &beta2, v[i]);
        big_fixed_mul(&temp2, &one_minus_beta2, &g_squared);
        big_fixed_add(v[i], &temp1, &temp2);
        
        // weight = weight - lr * m / (sqrt(v) + epsilon)
        BigFixed sqrt_v, denom, update;
        big_fixed_create_init(&sqrt_v, precision);
        big_fixed_create_init(&denom, precision);
        big_fixed_create_init(&update, precision);
        
        big_sqrt(&sqrt_v, v[i], precision);
        big_fixed_add(&denom, &sqrt_v, &epsilon);
        big_fixed_div(&temp1, m[i], &denom);
        big_fixed_mul(&update, &learning_rate, &temp1);
        
        big_fixed_sub(weights[i], weights[i], &update);
        
        // Zero gradient
        big_fixed_from_int(gradients[i], 0);
        
        // Cleanup
        big_fixed_free(&temp1);
        big_fixed_free(&temp2);
        big_fixed_free(&g_squared);
        big_fixed_free(&sqrt_v);
        big_fixed_free(&denom);
        big_fixed_free(&update);
    }
    
    big_fixed_free(&learning_rate);
    big_fixed_free(&beta1);
    big_fixed_free(&beta2);
    big_fixed_free(&epsilon);
    big_fixed_free(&one);
    big_fixed_free(&one_minus_beta1);
    big_fixed_free(&one_minus_beta2);
}

void sgd_step_bigfixed(
    BigFixed** weights,
    BigFixed** gradients,
    int num_params,
    float learning_rate_float,
    int precision
) {
    if (!weights || !gradients) return;
    
    BigFixed learning_rate;
    big_fixed_create_init(&learning_rate, precision);
    big_fixed_from_double(&learning_rate, learning_rate_float);
    
    for (int i = 0; i < num_params; i++) {
        if (!weights[i] || !gradients[i]) continue;
        
        // weight = weight - lr * gradient
        BigFixed update;
        big_fixed_create_init(&update, precision);
        big_fixed_mul(&update, &learning_rate, gradients[i]);
        big_fixed_sub(weights[i], weights[i], &update);
        
        // Zero gradient
        big_fixed_from_int(gradients[i], 0);
        
        big_fixed_free(&update);
    }
    
    big_fixed_free(&learning_rate);
}

void sgd_momentum_step_bigfixed(
    BigFixed** weights,
    BigFixed** gradients,
    BigFixed** velocity,
    int num_params,
    float learning_rate_float,
    float momentum_float,
    int precision
) {
    if (!weights || !gradients || !velocity) return;
    
    BigFixed learning_rate, momentum;
    big_fixed_create_init(&learning_rate, precision);
    big_fixed_create_init(&momentum, precision);
    big_fixed_from_double(&learning_rate, learning_rate_float);
    big_fixed_from_double(&momentum, momentum_float);
    
    for (int i = 0; i < num_params; i++) {
        if (!weights[i] || !gradients[i] || !velocity[i]) continue;
        
        // velocity = momentum * velocity + gradient
        BigFixed temp;
        big_fixed_create_init(&temp, precision);
        big_fixed_mul(&temp, &momentum, velocity[i]);
        big_fixed_add(velocity[i], &temp, gradients[i]);
        
        // weight = weight - lr * velocity
        BigFixed update;
        big_fixed_create_init(&update, precision);
        big_fixed_mul(&update, &learning_rate, velocity[i]);
        big_fixed_sub(weights[i], weights[i], &update);
        
        // Zero gradient
        big_fixed_from_int(gradients[i], 0);
        
        big_fixed_free(&temp);
        big_fixed_free(&update);
    }
    
    big_fixed_free(&learning_rate);
    big_fixed_free(&momentum);
}
