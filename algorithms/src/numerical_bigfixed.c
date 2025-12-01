#include "numerical.h"
#include "bigfixed_core.h"
#include "prime_bigint_transcendental.h"
#include "bigfixed_math_wrappers.h"
#include <stdlib.h>

/*
 * BigFixed Numerical Operations
 * Arbitrary precision numerical computations
 */

void matrix_multiply_bigfixed(
    BigFixed** A,
    BigFixed** B,
    BigFixed** C,
    int m, int n, int k,
    int precision
) {
    if (!A || !B || !C) return;
    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < k; j++) {
            BigFixed* sum = big_fixed_create(precision);
            big_fixed_from_int(sum, 0);
            
            for (int p = 0; p < n; p++) {
                BigFixed* prod = big_fixed_create(precision);
                big_fixed_mul(prod, A[i * n + p], B[p * k + j]);
                big_fixed_add(sum, sum, prod);
                big_fixed_free(prod);
            }
            
            big_fixed_assign(C[i * k + j], sum);
            big_fixed_free(sum);
        }
    }
}

void vector_add_bigfixed(
    BigFixed** a,
    BigFixed** b,
    BigFixed** result,
    int size
) {
    if (!a || !b || !result) return;
    
    for (int i = 0; i < size; i++) {
        big_fixed_add(result[i], a[i], b[i]);
    }
}

void vector_scale_bigfixed(
    BigFixed** vec,
    BigFixed* scalar,
    BigFixed** result,
    int size
) {
    if (!vec || !scalar || !result) return;
    
    for (int i = 0; i < size; i++) {
        big_fixed_mul(result[i], vec[i], scalar);
    }
}

void dot_product_bigfixed(
    BigFixed** a,
    BigFixed** b,
    BigFixed* result,
    int size,
    int precision
) {
    if (!a || !b || !result) return;
    
    big_fixed_from_int(result, 0);
    
    for (int i = 0; i < size; i++) {
        BigFixed* prod = big_fixed_create(precision);
        big_fixed_mul(prod, a[i], b[i]);
        big_fixed_add(result, result, prod);
        big_fixed_free(prod);
    }
}

void layer_norm_bigfixed(
    BigFixed** input,
    BigFixed** output,
    BigFixed** gamma,
    BigFixed** beta,
    int size,
    int precision
) {
    if (!input || !output || !gamma || !beta) return;
    
    // Compute mean
    BigFixed* mean = big_fixed_create(precision);
    big_fixed_from_int(mean, 0);
    
    for (int i = 0; i < size; i++) {
        big_fixed_add(mean, mean, input[i]);
    }
    
    BigFixed* size_fixed = big_fixed_create(precision);
    big_fixed_from_int(size_fixed, size);
    big_fixed_div(mean, mean, size_fixed);
    
    // Compute variance
    BigFixed* var = big_fixed_create(precision);
    big_fixed_from_int(var, 0);
    
    for (int i = 0; i < size; i++) {
        BigFixed* diff = big_fixed_create(precision);
        BigFixed* diff_sq = big_fixed_create(precision);
        
        big_fixed_sub(diff, input[i], mean);
        big_fixed_mul(diff_sq, diff, diff);
        big_fixed_add(var, var, diff_sq);
        
        big_fixed_free(diff);
        big_fixed_free(diff_sq);
    }
    big_fixed_div(var, var, size_fixed);
    
    // Add epsilon (simplified - just use var as-is for now)
    BigFixed* epsilon = big_fixed_create(precision);
    big_fixed_from_double(epsilon, 1e-5);
    big_fixed_add(var, var, epsilon);
    
    // Normalize (without sqrt for now - would need BigInt conversion)
    for (int i = 0; i < size; i++) {
        BigFixed* normalized = big_fixed_create(precision);
        BigFixed* scaled = big_fixed_create(precision);
        
        big_fixed_sub(normalized, input[i], mean);
        big_fixed_mul(scaled, gamma[i], normalized);
        big_fixed_add(output[i], scaled, beta[i]);
        
        big_fixed_free(normalized);
        big_fixed_free(scaled);
    }
    
    big_fixed_free(mean);
    big_fixed_free(var);
    big_fixed_free(epsilon);
    big_fixed_free(size_fixed);
}

void relu_bigfixed(
    BigFixed** input,
    BigFixed** output,
    int size,
    int precision
) {
    if (!input || !output) return;
    
    BigFixed* zero = big_fixed_create(precision);
    big_fixed_from_int(zero, 0);
    
    for (int i = 0; i < size; i++) {
        if (big_fixed_cmp(input[i], zero) > 0) {
            big_fixed_assign(output[i], input[i]);
        } else {
            big_fixed_assign(output[i], zero);
        }
    }
    
    big_fixed_free(zero);
}

void tanh_bigfixed(
    BigFixed** input,
    BigFixed** output,
    int size,
    int precision
) {
    if (!input || !output) return;
    
    for (int i = 0; i < size; i++) {
        bigfixed_tanh(output[i], input[i], precision);
    }
}
