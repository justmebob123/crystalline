/**
 * @file statistics.h
 * @brief Statistical analysis functions
 * 
 * This library provides pure mathematical implementations of statistical
 * functions used in data analysis, machine learning, and scientific computing.
 * 
 * These are MATHEMATICAL ALGORITHMS for:
 * - Descriptive statistics
 * - Probability distributions
 * - Statistical tests
 * - Data analysis
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#include <stddef.h>
#include <stdbool.h>

/* ============================================================================
 * Descriptive Statistics
 * ============================================================================ */

/**
 * @brief Compute mean (average) of array
 * @param values Array of values
 * @param size Number of values
 * @return Mean value
 */
double stats_mean(const double* values, size_t size);

/**
 * @brief Compute variance of array
 * @param values Array of values
 * @param size Number of values
 * @param sample_variance If true, use n-1 denominator (sample variance)
 * @return Variance
 */
double stats_variance(
    const double* values,
    size_t size,
    bool sample_variance
);

/**
 * @brief Compute standard deviation
 * @param values Array of values
 * @param size Number of values
 * @param sample_variance If true, use n-1 denominator
 * @return Standard deviation
 */
double stats_std_dev(
    const double* values,
    size_t size,
    bool sample_variance
);

/**
 * @brief Compute median of array
 * @param values Array of values (will be sorted)
 * @param size Number of values
 * @return Median value
 */
double stats_median(double* values, size_t size);

/**
 * @brief Compute mode (most frequent value)
 * @param values Array of values
 * @param size Number of values
 * @param tolerance Tolerance for considering values equal
 * @return Mode value
 */
double stats_mode(
    const double* values,
    size_t size,
    double tolerance
);

/**
 * @brief Compute percentile
 * @param values Array of values (will be sorted)
 * @param size Number of values
 * @param percentile Percentile to compute (0-100)
 * @return Percentile value
 */
double stats_percentile(
    double* values,
    size_t size,
    double percentile
);

/* ============================================================================
 * Correlation and Covariance
 * ============================================================================ */

/**
 * @brief Compute covariance between two arrays
 * @param x First array
 * @param y Second array
 * @param size Number of values
 * @param sample_covariance If true, use n-1 denominator
 * @return Covariance
 */
double stats_covariance(
    const double* x,
    const double* y,
    size_t size,
    bool sample_covariance
);

/**
 * @brief Compute Pearson correlation coefficient
 * @param x First array
 * @param y Second array
 * @param size Number of values
 * @return Correlation coefficient [-1, 1]
 */
double stats_correlation(
    const double* x,
    const double* y,
    size_t size
);

/**
 * @brief Compute Spearman rank correlation
 * @param x First array
 * @param y Second array
 * @param size Number of values
 * @return Rank correlation coefficient [-1, 1]
 */
double stats_spearman_correlation(
    const double* x,
    const double* y,
    size_t size
);

/* ============================================================================
 * Distribution Functions
 * ============================================================================ */

/**
 * @brief Compute histogram
 * @param values Array of values
 * @param size Number of values
 * @param bins Output histogram bins (must be pre-allocated)
 * @param num_bins Number of bins
 * @param min_value Minimum value for histogram
 * @param max_value Maximum value for histogram
 */
void stats_histogram(
    const double* values,
    size_t size,
    size_t* bins,
    size_t num_bins,
    double min_value,
    double max_value
);

/**
 * @brief Compute cumulative distribution function (empirical)
 * @param values Array of values (will be sorted)
 * @param size Number of values
 * @param x Point at which to evaluate CDF
 * @return CDF value at x
 */
double stats_empirical_cdf(
    double* values,
    size_t size,
    double x
);

/* ============================================================================
 * Normalization and Standardization
 * ============================================================================ */

/**
 * @brief Z-score normalization (standardization)
 * @param values Array to normalize (modified in place)
 * @param size Number of values
 */
void stats_z_score_normalize(double* values, size_t size);

/**
 * @brief Min-max normalization to [0, 1]
 * @param values Array to normalize (modified in place)
 * @param size Number of values
 */
void stats_min_max_normalize(double* values, size_t size);

/**
 * @brief Robust scaling using median and IQR
 * @param values Array to scale (modified in place)
 * @param size Number of values
 */
void stats_robust_scale(double* values, size_t size);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Find minimum value in array
 * @param values Array of values
 * @param size Number of values
 * @return Minimum value
 */
double stats_min(const double* values, size_t size);

/**
 * @brief Find maximum value in array
 * @param values Array of values
 * @param size Number of values
 * @return Maximum value
 */
double stats_max(const double* values, size_t size);

/**
 * @brief Find range (max - min)
 * @param values Array of values
 * @param size Number of values
 * @return Range
 */
double stats_range(const double* values, size_t size);

/**
 * @brief Compute sum of array
 * @param values Array of values
 * @param size Number of values
 * @return Sum
 */
double stats_sum(const double* values, size_t size);

/**
 * @brief Compute sum of squares
 * @param values Array of values
 * @param size Number of values
 * @return Sum of squares
 */
double stats_sum_of_squares(const double* values, size_t size);

#endif /* STATISTICS_H */