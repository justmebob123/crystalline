/**
 * @file statistics.c
 * @brief Implementation of statistical analysis functions
 */

#include "statistics.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static int compare_doubles(const void* a, const void* b) {
    double diff = (*(double*)a - *(double*)b);
    return (diff > 0) ? 1 : ((diff < 0) ? -1 : 0);
}

/* ============================================================================
 * Descriptive Statistics
 * ============================================================================ */

double stats_mean(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += values[i];
    }
    return sum / (double)size;
}

double stats_variance(const double* values, size_t size, bool sample_variance) {
    if (!values || size == 0) return 0.0;
    
    double mean = stats_mean(values, size);
    double var = 0.0;
    
    for (size_t i = 0; i < size; i++) {
        double diff = values[i] - mean;
        var += diff * diff;
    }
    
    // Use n-1 for sample variance, n for population variance
    double denominator = sample_variance ? (double)(size - 1) : (double)size;
    return var / denominator;
}

double stats_std_dev(const double* values, size_t size, bool sample_variance) {
    return sqrt(stats_variance(values, size, sample_variance));
}

double stats_median(double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    // Sort the array
    qsort(values, size, sizeof(double), compare_doubles);
    
    if (size % 2 == 0) {
        // Even number of elements: average of middle two
        return (values[size/2 - 1] + values[size/2]) / 2.0;
    } else {
        // Odd number of elements: middle element
        return values[size/2];
    }
}

double stats_mode(const double* values, size_t size, double tolerance) {
    if (!values || size == 0) return 0.0;
    
    // Simple implementation: find most frequent value within tolerance
    double mode = values[0];
    size_t max_count = 1;
    
    for (size_t i = 0; i < size; i++) {
        size_t count = 0;
        for (size_t j = 0; j < size; j++) {
            if (fabs(values[i] - values[j]) <= tolerance) {
                count++;
            }
        }
        if (count > max_count) {
            max_count = count;
            mode = values[i];
        }
    }
    
    return mode;
}

double stats_percentile(double* values, size_t size, double percentile) {
    if (!values || size == 0) return 0.0;
    if (percentile < 0.0) percentile = 0.0;
    if (percentile > 100.0) percentile = 100.0;
    
    // Sort the array
    qsort(values, size, sizeof(double), compare_doubles);
    
    double index = (percentile / 100.0) * (double)(size - 1);
    size_t lower = (size_t)floor(index);
    size_t upper = (size_t)ceil(index);
    
    if (lower == upper) {
        return values[lower];
    } else {
        double weight = index - (double)lower;
        return values[lower] * (1.0 - weight) + values[upper] * weight;
    }
}

/* ============================================================================
 * Correlation and Covariance
 * ============================================================================ */

double stats_covariance(
    const double* x,
    const double* y,
    size_t size,
    bool sample_covariance
) {
    if (!x || !y || size == 0) return 0.0;
    
    double mean_x = stats_mean(x, size);
    double mean_y = stats_mean(y, size);
    
    double cov = 0.0;
    for (size_t i = 0; i < size; i++) {
        cov += (x[i] - mean_x) * (y[i] - mean_y);
    }
    
    double denominator = sample_covariance ? (double)(size - 1) : (double)size;
    return cov / denominator;
}

double stats_correlation(const double* x, const double* y, size_t size) {
    if (!x || !y || size == 0) return 0.0;
    
    double cov = stats_covariance(x, y, size, true);
    double std_x = stats_std_dev(x, size, true);
    double std_y = stats_std_dev(y, size, true);
    
    if (std_x < 1e-10 || std_y < 1e-10) return 0.0;
    
    return cov / (std_x * std_y);
}

double stats_spearman_correlation(const double* x, const double* y, size_t size) {
    if (!x || !y || size == 0) return 0.0;
    
    // Convert to ranks
    double* rank_x = (double*)malloc(size * sizeof(double));
    double* rank_y = (double*)malloc(size * sizeof(double));
    
    if (!rank_x || !rank_y) {
        free(rank_x);
        free(rank_y);
        return 0.0;
    }
    
    // Simple ranking (could be optimized)
    for (size_t i = 0; i < size; i++) {
        rank_x[i] = 1.0;
        rank_y[i] = 1.0;
        
        for (size_t j = 0; j < size; j++) {
            if (x[j] < x[i]) rank_x[i] += 1.0;
            if (y[j] < y[i]) rank_y[i] += 1.0;
        }
    }
    
    // Compute Pearson correlation of ranks
    double corr = stats_correlation(rank_x, rank_y, size);
    
    free(rank_x);
    free(rank_y);
    
    return corr;
}

/* ============================================================================
 * Distribution Functions
 * ============================================================================ */

void stats_histogram(
    const double* values,
    size_t size,
    size_t* bins,
    size_t num_bins,
    double min_value,
    double max_value
) {
    if (!values || !bins || size == 0 || num_bins == 0) return;
    
    // Initialize bins to zero
    memset(bins, 0, num_bins * sizeof(size_t));
    
    double range = max_value - min_value;
    if (range < 1e-10) return;
    
    for (size_t i = 0; i < size; i++) {
        double val = values[i];
        
        if (val < min_value || val > max_value) continue;
        
        size_t bin = (size_t)((val - min_value) / range * (double)num_bins);
        if (bin >= num_bins) bin = num_bins - 1;
        
        bins[bin]++;
    }
}

double stats_empirical_cdf(double* values, size_t size, double x) {
    if (!values || size == 0) return 0.0;
    
    // Sort the array
    qsort(values, size, sizeof(double), compare_doubles);
    
    // Count values <= x
    size_t count = 0;
    for (size_t i = 0; i < size; i++) {
        if (values[i] <= x) {
            count++;
        } else {
            break;
        }
    }
    
    return (double)count / (double)size;
}

/* ============================================================================
 * Normalization and Standardization
 * ============================================================================ */

void stats_z_score_normalize(double* values, size_t size) {
    if (!values || size == 0) return;
    
    double mean = stats_mean(values, size);
    double std = stats_std_dev(values, size, false);
    
    if (std < 1e-10) return;
    
    for (size_t i = 0; i < size; i++) {
        values[i] = (values[i] - mean) / std;
    }
}

void stats_min_max_normalize(double* values, size_t size) {
    if (!values || size == 0) return;
    
    double min = stats_min(values, size);
    double max = stats_max(values, size);
    double range = max - min;
    
    if (range < 1e-10) return;
    
    for (size_t i = 0; i < size; i++) {
        values[i] = (values[i] - min) / range;
    }
}

void stats_robust_scale(double* values, size_t size) {
    if (!values || size == 0) return;
    
    // Make a copy for computing median and IQR
    double* sorted = (double*)malloc(size * sizeof(double));
    if (!sorted) return;
    
    memcpy(sorted, values, size * sizeof(double));
    
    double median = stats_median(sorted, size);
    double q1 = stats_percentile(sorted, size, 25.0);
    double q3 = stats_percentile(sorted, size, 75.0);
    double iqr = q3 - q1;
    
    free(sorted);
    
    if (iqr < 1e-10) return;
    
    for (size_t i = 0; i < size; i++) {
        values[i] = (values[i] - median) / iqr;
    }
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

double stats_min(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double min = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] < min) min = values[i];
    }
    return min;
}

double stats_max(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double max = values[0];
    for (size_t i = 1; i < size; i++) {
        if (values[i] > max) max = values[i];
    }
    return max;
}

double stats_range(const double* values, size_t size) {
    return stats_max(values, size) - stats_min(values, size);
}

double stats_sum(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += values[i];
    }
    return sum;
}

double stats_sum_of_squares(const double* values, size_t size) {
    if (!values || size == 0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        sum += values[i] * values[i];
    }
    return sum;
}