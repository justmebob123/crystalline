#ifndef CLLM_SIMD_UTILS_H
#define CLLM_SIMD_UTILS_H

/**
 * SIMD Utilities for CLLM Training
 * 
 * Provides vectorized operations using AVX2 for significant speedup
 */

/**
 * Compute dot product of two vectors
 * Uses AVX2 when possible, falls back to scalar for remainder
 * 
 * @param a First vector
 * @param b Second vector
 * @param n Length of vectors
 * @return Dot product a·b
 */
float dot_product(const float* a, const float* b, int n);

/**
 * AVX2-optimized dot product (requires n to be multiple of 8)
 * 
 * @param a First vector
 * @param b Second vector
 * @param n Length of vectors (must be multiple of 8)
 * @return Dot product a·b
 */
float dot_product_avx2(const float* a, const float* b, int n);

/**
 * Scalar fallback dot product
 * 
 * @param a First vector
 * @param b Second vector
 * @param n Length of vectors
 * @return Dot product a·b
 */
float dot_product_scalar(const float* a, const float* b, int n);

/**
 * Vectorized multiply-accumulate: result[i] += a[i] * b[i]
 * 
 * @param result Result vector (modified in-place)
 * @param a First input vector
 * @param b Second input vector
 * @param n Length of vectors
 */
void vector_multiply_accumulate(float* result, const float* a, const float* b, int n);

/**
 * Vectorized addition: result[i] = a[i] + b[i]
 * 
 * @param result Result vector
 * @param a First input vector
 * @param b Second input vector
 * @param n Length of vectors
 */
void vector_add(float* result, const float* a, const float* b, int n);

/**
 * Vectorized scalar multiplication: result[i] = a[i] * scalar
 * 
 * @param result Result vector
 * @param a Input vector
 * @param scalar Scalar multiplier
 * @param n Length of vectors
 */
void vector_scale(float* result, const float* a, float scalar, int n);

#endif /* CLLM_SIMD_UTILS_H */