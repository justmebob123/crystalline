/**
 * @file numerical.c
 * @brief Implementation of numerical analysis functions
 */

#include "numerical.h"
#include "prime_math_custom.h"
#include "prime_float_math.h"
#include <string.h>

/* ============================================================================
 * Softmax and Related Functions
 * ============================================================================ */

void numerical_softmax(const double* input, double* output, size_t size) {
    if (!input || !output || size == 0) return;
    
    // Find max for numerical stability
    double max_val = input[0];
    for (size_t i = 1; i < size; i++) {
        if (input[i] > max_val) {
            max_val = input[i];
        }
    }
    
    // Compute exp(x - max) and sum
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        output[i] = prime_exp(input[i] - max_val);
        sum += output[i];
    }
    
    // Normalize
    for (size_t i = 0; i < size; i++) {
        output[i] /= sum;
    }
}

void numerical_log_softmax(const double* input, double* output, size_t size) {
    if (!input || !output || size == 0) return;
    
    // Compute log-sum-exp
    double lse = numerical_log_sum_exp(input, size);
    
    // Compute log-softmax
    for (size_t i = 0; i < size; i++) {
        output[i] = input[i] - lse;
    }
}

double numerical_log_sum_exp(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    // Find max for numerical stability
    double max_val = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] > max_val) {
            max_val = values[i];
        }
    }
    
    // Compute sum(exp(x - max))
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += prime_exp(values[i] - max_val);
    }
    
    // Return max + log(sum)
    return max_val + prime_log(sum);
}

void numerical_softmax_2d(const double* input, double* output, 
                         size_t batch_size, size_t num_classes) {
    for (size_t i = 0; i < batch_size; i++) {
        numerical_softmax(
            input + i * num_classes,
            output + i * num_classes,
            num_classes
        );
    }
}

void numerical_log_softmax_2d(const double* input, double* output,
                             size_t batch_size, size_t num_classes) {
    for (size_t i = 0; i < batch_size; i++) {
        numerical_log_softmax(
            input + i * num_classes,
            output + i * num_classes,
            num_classes
        );
    }
}

/* ============================================================================
 * Safe Mathematical Operations
 * ============================================================================ */

double numerical_safe_log(double x, double epsilon) {
    return prime_log(x + epsilon);
}

double numerical_safe_exp(double x, double max_exp) {
    if (x > max_exp) return prime_exp(max_exp);
    return prime_exp(x);
}

double numerical_safe_divide(double numerator, double denominator, double epsilon) {
    return numerator / (denominator + epsilon);
}

double numerical_safe_sqrt(double x, double epsilon) {
    if (x < 0.0) x = 0.0;
    return prime_sqrt(x + epsilon);
}

/* ============================================================================
 * Numerical Stability Checks
 * ============================================================================ */

bool numerical_is_nan(double x) {
    return x != x;
}

bool numerical_is_inf(double x) {
    return x == INFINITY || x == -INFINITY;
}

bool numerical_is_finite(double x) {
    return !numerical_is_nan(x) && !numerical_is_inf(x);
}

void numerical_check_array(const double* values, size_t size,
                          bool* has_nan, bool* has_inf) {
    if (!values || size == 0) return;
    
    *has_nan = false;
    *has_inf = false;
    
    for (size_t i = 0; i < size; i++) {
        if (numerical_is_nan(values[i])) {
            *has_nan = true;
        }
        if (numerical_is_inf(values[i])) {
            *has_inf = true;
        }
        if (*has_nan && *has_inf) break;
    }
}

double numerical_clip(double x, double min_value, double max_value) {
    if (x < min_value) return min_value;
    if (x > max_value) return max_value;
    return x;
}

void numerical_clip_array(double* values, size_t size,
                         double min_value, double max_value) {
    if (!values || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        values[i] = numerical_clip(values[i], min_value, max_value);
    }
}

/* ============================================================================
 * Array Operations
 * ============================================================================ */

double numerical_max(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double max_val = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] > max_val) {
            max_val = values[i];
        }
    }
    return max_val;
}

double numerical_min(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double min_val = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] < min_val) {
            min_val = values[i];
        }
    }
    return min_val;
}

double numerical_sum(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += values[i];
    }
    return sum;
}

double numerical_mean(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    return numerical_sum(values, size) / (double)size;
}

double numerical_variance(const double* values, size_t size,
                         double mean, bool compute_mean) {
    if (!values || size == 0) return 0.0;
    
    if (compute_mean) {
        mean = numerical_mean(values, size);
    }
    
    double var = 0.0;
    for (size_t i = 0; i < size; i++) {
        double diff = values[i] - mean;
        var += diff * diff;
    }
    
    return var / (double)size;
}

double numerical_std_dev(const double* values, size_t size,
                        double mean, bool compute_mean) {
    double var = numerical_variance(values, size, mean, compute_mean);
    return prime_sqrt(var);
}

/* ============================================================================
 * Normalization Functions
 * ============================================================================ */

void numerical_normalize_01(double* values, size_t size) {
    if (!values || size == 0) return;
    
    double min_val = numerical_min(values, size);
    double max_val = numerical_max(values, size);
    double range = max_val - min_val;
    
    if (range < 1e-10) return; // Avoid division by zero
    
    for (size_t i = 0; i < size; i++) {
        values[i] = (values[i] - min_val) / range;
    }
}

void numerical_standardize(double* values, size_t size) {
    if (!values || size == 0) return;
    
    double mean = numerical_mean(values, size);
    double std = numerical_std_dev(values, size, mean, false);
    
    if (std < 1e-10) return; // Avoid division by zero
    
    for (size_t i = 0; i < size; i++) {
        values[i] = (values[i] - mean) / std;
    }
}

void numerical_l2_normalize(double* values, size_t size) {
    if (!values || size == 0) return;
    
    double norm = 0.0;
    for (size_t i = 0; i < size; i++) {
        norm += values[i] * values[i];
    }
    norm = prime_sqrt(norm);
    
    if (norm < 1e-10) return; // Avoid division by zero
    
    for (size_t i = 0; i < size; i++) {
        values[i] /= norm;
    }
}

/* ============================================================================
 * Distance and Similarity Functions
 * ============================================================================ */

double numerical_l2_distance(const double* a, const double* b, size_t size) {
    if (!a || !b || size == 0) return 0.0;
    
    double dist = 0.0;
    for (size_t i = 0; i < size; i++) {
        double diff = a[i] - b[i];
        dist += diff * diff;
    }
    return prime_sqrt(dist);
}

double numerical_l1_distance(const double* a, const double* b, size_t size) {
    if (!a || !b || size == 0) return 0.0;
    
    double dist = 0.0;
    for (size_t i = 0; i < size; i++) {
        double diff = a[i] - b[i];
        dist += (diff < 0) ? -diff : diff;
    }
    return dist;
}

double numerical_cosine_similarity(const double* a, const double* b, size_t size) {
    if (!a || !b || size == 0) return 0.0;
    
    double dot = numerical_dot_product(a, b, size);
    
    double norm_a = 0.0, norm_b = 0.0;
    for (size_t i = 0; i < size; i++) {
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    norm_a = prime_sqrt(norm_a);
    norm_b = prime_sqrt(norm_b);
    
    if (norm_a < 1e-10 || norm_b < 1e-10) return 0.0;
    
    return dot / (norm_a * norm_b);
}

double numerical_dot_product(const double* a, const double* b, size_t size) {
    if (!a || !b || size == 0) return 0.0;
    
    double dot = 0.0;
    for (size_t i = 0; i < size; i++) {
        dot += a[i] * b[i];
    }
    return dot;
}