#ifndef NUMERICAL_BIGFIXED_H
#define NUMERICAL_BIGFIXED_H

#include "../../include/bigfixed_core.h"

/**
 * Matrix multiplication using BigFixed precision
 * C = A * B
 * A: m x n matrix
 * B: n x k matrix
 * C: m x k matrix (output)
 */
void matrix_multiply_bigfixed(
    BigFixed** A,
    BigFixed** B,
    BigFixed** C,
    int m, int n, int k,
    int precision
);

/**
 * Dot product using BigFixed precision
 * result = sum(a[i] * b[i])
 */
void dot_product_bigfixed(
    BigFixed** a,
    BigFixed** b,
    BigFixed* result,
    int size,
    int precision
);

#endif // NUMERICAL_BIGFIXED_H