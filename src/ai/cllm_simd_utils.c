/**
 * SIMD Utilities for CLLM Training
 * 
 * Provides vectorized operations using AVX2 for significant speedup
 */

#include <immintrin.h>
#include <stdint.h>
#include <string.h>

/**
 * AVX2 dot product (8 floats at a time)
 * Requires: n must be multiple of 8
 */
float dot_product_avx2(const float* a, const float* b, int n) {
    __m256 sum = _mm256_setzero_ps();
    
    for (int i = 0; i < n; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        sum = _mm256_fmadd_ps(va, vb, sum);
    }
    
    // Horizontal sum
    __m128 sum_high = _mm256_extractf128_ps(sum, 1);
    __m128 sum_low = _mm256_castps256_ps128(sum);
    __m128 sum128 = _mm_add_ps(sum_high, sum_low);
    
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    
    return _mm_cvtss_f32(sum128);
}

/**
 * Fallback for non-multiple-of-8 sizes
 */
float dot_product_scalar(const float* a, const float* b, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

/**
 * Adaptive dot product (uses AVX2 when possible)
 * This is the main function to use - automatically chooses best implementation
 */
float dot_product(const float* a, const float* b, int n) {
    int n_vec = (n / 8) * 8;  // Round down to multiple of 8
    
    float sum = 0.0f;
    if (n_vec > 0) {
        sum = dot_product_avx2(a, b, n_vec);
    }
    
    // Handle remainder
    for (int i = n_vec; i < n; i++) {
        sum += a[i] * b[i];
    }
    
    return sum;
}

/**
 * Vectorized element-wise multiplication and accumulation
 * result[i] += a[i] * b[i] for all i
 */
void vector_multiply_accumulate(float* result, const float* a, const float* b, int n) {
    int n_vec = (n / 8) * 8;
    
    // Vectorized part
    for (int i = 0; i < n_vec; i += 8) {
        __m256 vr = _mm256_loadu_ps(&result[i]);
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        vr = _mm256_fmadd_ps(va, vb, vr);
        _mm256_storeu_ps(&result[i], vr);
    }
    
    // Scalar remainder
    for (int i = n_vec; i < n; i++) {
        result[i] += a[i] * b[i];
    }
}

/**
 * Vectorized element-wise addition
 * result[i] = a[i] + b[i] for all i
 */
void vector_add(float* result, const float* a, const float* b, int n) {
    int n_vec = (n / 8) * 8;
    
    // Vectorized part
    for (int i = 0; i < n_vec; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        __m256 vr = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(&result[i], vr);
    }
    
    // Scalar remainder
    for (int i = n_vec; i < n; i++) {
        result[i] = a[i] + b[i];
    }
}

/**
 * Vectorized scalar multiplication
 * result[i] = a[i] * scalar for all i
 */
void vector_scale(float* result, const float* a, float scalar, int n) {
    int n_vec = (n / 8) * 8;
    __m256 vscalar = _mm256_set1_ps(scalar);
    
    // Vectorized part
    for (int i = 0; i < n_vec; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vr = _mm256_mul_ps(va, vscalar);
        _mm256_storeu_ps(&result[i], vr);
    }
    
    // Scalar remainder
    for (int i = n_vec; i < n; i++) {
        result[i] = a[i] * scalar;
    }
}