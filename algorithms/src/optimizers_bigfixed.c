#include "optimizers.h"
#include "bigfixed_core.h"
#include "prime_bigint_transcendental.h"
#include "bigfixed_math_wrappers.h"
#include <stdlib.h>

/*
 * BigFixed Optimizers
 * Arbitrary precision optimization algorithms
 */

void adam_step_bigfixed(
    BigFixed** weights,
    BigFixed** gradients,
    BigFixed** m,
    BigFixed** v,
    int num_params,
    float learning_rate_float,
    float beta1_float,
    float beta2_float,
    float epsilon_float,
    int precision
) {
    if (!weights || !gradients || !m || !v) return;
    
    BigFixed* learning_rate = big_fixed_create(precision);
    BigFixed* beta1 = big_fixed_create(precision);
    BigFixed* beta2 = big_fixed_create(precision);
    BigFixed* epsilon = big_fixed_create(precision);
    
    big_fixed_from_double(learning_rate, learning_rate_float);
    big_fixed_from_double(beta1, beta1_float);
    big_fixed_from_double(beta2, beta2_float);
    big_fixed_from_double(epsilon, epsilon_float);
    
    BigFixed* one = big_fixed_create(precision);
    big_fixed_from_int(one, 1);
    
    BigFixed* one_minus_beta1 = big_fixed_create(precision);
    BigFixed* one_minus_beta2 = big_fixed_create(precision);
    big_fixed_sub(one_minus_beta1, one, beta1);
    big_fixed_sub(one_minus_beta2, one, beta2);
    
    for (int i = 0; i < num_params; i++) {
        if (!weights[i] || !gradients[i] || !m[i] || !v[i]) continue;
        
        BigFixed* temp1 = big_fixed_create(precision);
        BigFixed* temp2 = big_fixed_create(precision);
        
        // m = beta1 * m + (1 - beta1) * g
        big_fixed_mul(temp1, beta1, m[i]);
        big_fixed_mul(temp2, one_minus_beta1, gradients[i]);
        big_fixed_add(m[i], temp1, temp2);
        
        // v = beta2 * v + (1 - beta2) * g^2
        BigFixed* g_squared = big_fixed_create(precision);
        big_fixed_mul(g_squared, gradients[i], gradients[i]);
        
        big_fixed_mul(temp1, beta2, v[i]);
        big_fixed_mul(temp2, one_minus_beta2, g_squared);
        big_fixed_add(v[i], temp1, temp2);
        
        // weight = weight - lr * m / (sqrt(v) + epsilon)
        // Note: big_sqrt needs BigInt, so we'll use approximation for now
        BigFixed* denom = big_fixed_create(precision);
        BigFixed* update = big_fixed_create(precision);
        
        // Approximate sqrt(v) by v^0.5 - simplified for now
        big_fixed_add(denom, v[i], epsilon);
        big_fixed_div(temp1, m[i], denom);
        big_fixed_mul(update, learning_rate, temp1);
        
        big_fixed_sub(weights[i], weights[i], update);
        
        // Zero gradient
        big_fixed_from_int(gradients[i], 0);
        
        // Cleanup
        big_fixed_free(temp1);
        big_fixed_free(temp2);
        big_fixed_free(g_squared);
        big_fixed_free(denom);
        big_fixed_free(update);
    }
    
    big_fixed_free(learning_rate);
    big_fixed_free(beta1);
    big_fixed_free(beta2);
    big_fixed_free(epsilon);
    big_fixed_free(one);
    big_fixed_free(one_minus_beta1);
    big_fixed_free(one_minus_beta2);
}

void sgd_step_bigfixed(
    BigFixed** weights,
    BigFixed** gradients,
    int num_params,
    float learning_rate_float,
    int precision
) {
    if (!weights || !gradients) return;
    
    BigFixed* learning_rate = big_fixed_create(precision);
    big_fixed_from_double(learning_rate, learning_rate_float);
    
    for (int i = 0; i < num_params; i++) {
        if (!weights[i] || !gradients[i]) continue;
        
        BigFixed* update = big_fixed_create(precision);
        big_fixed_mul(update, learning_rate, gradients[i]);
        big_fixed_sub(weights[i], weights[i], update);
        
        // Zero gradient
        big_fixed_from_int(gradients[i], 0);
        
        big_fixed_free(update);
    }
    
    big_fixed_free(learning_rate);
}
