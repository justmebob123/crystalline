/**
 * @file arithmetic.c
 * @brief Core arithmetic operations implementation
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * Self-contained double-precision arithmetic.
 * No dependencies on math.h or external libraries.
 */

#include "math/arithmetic.h"
#include <stdint.h>

/* Forward declarations for internal helpers */
static double math_trunc(double x);

/* ============================================================================
 * BASIC ARITHMETIC
 * ============================================================================
 */

double math_add(double a, double b) {
    return a + b;
}

double math_sub(double a, double b) {
    return a - b;
}

double math_mul(double a, double b) {
    return a * b;
}

double math_div(double a, double b) {
    if (b == 0.0) {
        // Return NaN for division by zero
        return 0.0 / 0.0;
    }
    return a / b;
}

double math_mod(double a, double b) {
    if (b == 0.0) {
        return 0.0 / 0.0;  // NaN
    }
    
    // C99 fmod behavior: result has same sign as dividend (a)
    double quotient = math_trunc(a / b);
    double result = a - b * quotient;
    
    return result;
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================
 */

double math_abs(double x) {
    return (x < 0.0) ? -x : x;
}

double math_min(double a, double b) {
    return (a < b) ? a : b;
}

double math_max(double a, double b) {
    return (a > b) ? a : b;
}

double math_clamp(double x, double min_val, double max_val) {
    if (x < min_val) return min_val;
    if (x > max_val) return max_val;
    return x;
}

int math_sign(double x) {
    if (x < 0.0) return -1;
    if (x > 0.0) return 1;
    return 0;
}

bool math_approx_equal(double a, double b, double epsilon) {
    return math_abs(a - b) < epsilon;
}

bool math_is_nan(double x) {
    // NaN is the only value that doesn't equal itself
    return x != x;
}

bool math_is_inf(double x) {
    // Check for infinity by comparing with a very large number
    // Infinity is larger than any finite number
    return (x > 1.7976931348623157e+308) || (x < -1.7976931348623157e+308);
}

bool math_is_finite(double x) {
    return !math_is_nan(x) && !math_is_inf(x);
}

/* ============================================================================
 * HELPER FUNCTIONS (internal use)
 * ============================================================================
 */

/**
 * @brief Truncate to integer (internal helper)
 * @param x Input value
 * @return Integer part (round toward zero)
 */
static double math_trunc(double x) {
    if (x >= 0.0) {
        // Positive: cast to int64 and back
        if (x < 9007199254740992.0) {  // 2^53
            return (double)(int64_t)x;
        }
        // For very large numbers, already an integer
        return x;
    } else {
        // Negative: cast to int64 and back
        if (x > -9007199254740992.0) {  // -2^53
            return (double)(int64_t)x;
        }
        // For very large negative numbers, already an integer
        return x;
    }
}