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
#include "math/validation.h"  // For math_is_nan, math_is_inf, math_is_finite
#include <stdint.h>

/* Forward declarations for internal helpers */
// Forward declaration removed - now in header

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

// math_sign() removed - now implemented as static inline in validation.h for O(1) performance
// This avoids duplicate definitions and improves performance

bool math_approx_equal(double a, double b, double epsilon) {
    return math_abs(a - b) < epsilon;
}

// math_is_nan(), math_is_inf(), math_is_finite() removed
// Now implemented as static inline in validation.h for O(1) performance
// This avoids duplicate definitions and improves performance

// Rounding functions
double math_floor(double x) {
    if (math_is_nan(x) || math_is_inf(x)) {
        return x;
    }
    
    int64_t i = (int64_t)x;
    if (x < 0.0 && x != (double)i) {
        return (double)(i - 1);
    }
    return (double)i;
}

double math_ceil(double x) {
    if (math_is_nan(x) || math_is_inf(x)) {
        return x;
    }
    
    int64_t i = (int64_t)x;
    if (x > 0.0 && x != (double)i) {
        return (double)(i + 1);
    }
    return (double)i;
}

double math_round(double x) {
    if (math_is_nan(x) || math_is_inf(x)) {
        return x;
    }
    
    if (x >= 0.0) {
        return math_floor(x + 0.5);
    } else {
        return math_ceil(x - 0.5);
    }
}

double math_trunc(double x) {
    if (math_is_nan(x) || math_is_inf(x)) {
        return x;
    }
    
    return (double)((int64_t)x);
}

/* ============================================================================
 * HELPER FUNCTIONS (internal use)
 * ============================================================================
 */

// Note: math_trunc is already defined above as a public function