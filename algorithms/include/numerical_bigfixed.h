/**
 * @file numerical_bigfixed.h
 * @brief BigFixed arbitrary precision numerical operations
 * 
 * This library provides BigFixed versions of numerical operations
 * for arbitrary precision computation without floating point errors.
 */

#ifndef NUMERICAL_BIGFIXED_H
#define NUMERICAL_BIGFIXED_H

#include "bigfixed_core.h"
#include <stddef.h>
#include <stdint.h>

/* ============================================================================
 * Matrix and Vector Operations
 * ============================================================================ */

/**
 * @brief Matrix multiplication with BigFixed
 * C = A * B
 * @param A Input matrix A [m x n]
 * @param B Input matrix B [n x k]
 * @param C Output matrix C [m x k]
 * @param m Number of rows in A
 * @param n Number of columns in A / rows in B
 * @param k Number of columns in B
 * @param precision Precision bits for computation
 */
void matrix_multiply_bigfixed(
    BigFixed** A,
    BigFixed** B,
    BigFixed** C,
    int m, int n, int k,
    int precision
);

/**
 * @brief Vector addition with BigFixed
 * result = a + b
 * @param a Input vector a
 * @param b Input vector b
 * @param result Output vector
 * @param size Vector size
 */
void vector_add_bigfixed(
    BigFixed** a,
    BigFixed** b,
    BigFixed** result,
    int size
);

/**
 * @brief Vector scaling with BigFixed
 * result = scalar * vec
 * @param vec Input vector
 * @param scalar Scalar value
 * @param result Output vector
 * @param size Vector size
 */
void vector_scale_bigfixed(
    BigFixed** vec,
    BigFixed* scalar,
    BigFixed** result,
    int size
);

/**
 * @brief Dot product with BigFixed
 * result = a · b
 * @param a Input vector a
 * @param b Input vector b
 * @param result Output scalar
 * @param size Vector size
 * @param precision Precision bits for computation
 */
void dot_product_bigfixed(
    BigFixed** a,
    BigFixed** b,
    BigFixed* result,
    int size,
    int precision
);

/**
 * @brief Layer normalization with BigFixed
 * output = gamma * (input - mean) / sqrt(variance + epsilon) + beta
 * @param input Input vector
 * @param output Output vector
 * @param gamma Scale parameter
 * @param beta Shift parameter
 * @param size Vector size
 * @param precision Precision bits for computation
 */
void layer_norm_bigfixed(
    BigFixed** input,
    BigFixed** output,
    BigFixed** gamma,
    BigFixed** beta,
    int size,
    int precision
);

#endif /* NUMERICAL_BIGFIXED_H */
