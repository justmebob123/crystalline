/*
 * prime_lowlevel.c - Low-level mathematical functions with NO external dependencies
 * 
 * This file implements basic mathematical operations from scratch without using math.h
 * All functions use only integer arithmetic and are designed to avoid circular dependencies.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../include/prime_lowlevel.h"

/* ============================================================================
 * BASIC INTEGER OPERATIONS (No dependencies)
 * ============================================================================ */

/**
 * Integer square root using Newton's method
 * Returns floor(sqrt(n))
 */
uint64_t prime_sqrt_int(uint64_t n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    
    // Initial guess: use bit manipulation for fast approximation
    uint64_t x = n;
    uint64_t y = (x + 1) / 2;
    
    // Newton's method: x_{n+1} = (x_n + n/x_n) / 2
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    
    return x;
}

/**
 * Integer logarithm base 2 using bit manipulation
 * Returns floor(log2(n))
 */
uint32_t prime_log2_int(uint64_t n) {
    if (n == 0) return 0;
    
    uint32_t log = 0;
    while (n >>= 1) {
        log++;
    }
    return log;
}

/**
 * Integer logarithm with arbitrary base using binary search
 * Returns floor(log_base(n))
 */
uint64_t prime_log_int(uint64_t n, uint64_t base) {
    if (n == 0 || base <= 1) return 0;
    if (n == 1) return 0;
    if (n < base) return 0;
    
    uint64_t result = 0;
    uint64_t power = 1;
    
    // Find the largest k such that base^k <= n
    while (power <= n / base) {  // Avoid overflow
        power *= base;
        result++;
    }
    
    return result;
}

/**
 * Integer power using repeated squaring
 * Returns base^exp
 */
uint64_t prime_pow_int(uint64_t base, uint64_t exp) {
    if (exp == 0) return 1;
    if (exp == 1) return base;
    if (base == 0) return 0;
    if (base == 1) return 1;
    
    uint64_t result = 1;
    uint64_t current_base = base;
    
    while (exp > 0) {
        if (exp & 1) {
            result *= current_base;
        }
        current_base *= current_base;
        exp >>= 1;
    }
    
    return result;
}

/**
 * Modular exponentiation: (base^exp) mod m
 * Uses repeated squaring to avoid overflow
 */
uint64_t prime_powmod_int(uint64_t base, uint64_t exp, uint64_t m) {
    if (m == 1) return 0;
    
    uint64_t result = 1;
    base = base % m;
    
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % m;
        }
        exp >>= 1;
        base = (base * base) % m;
    }
    
    return result;
}

/**
 * Greatest Common Divisor using Euclidean algorithm
 */
uint64_t prime_gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Least Common Multiple
 */
uint64_t prime_lcm(uint64_t a, uint64_t b) {
    if (a == 0 || b == 0) return 0;
    return (a / prime_gcd(a, b)) * b;
}

/**
 * Extended Euclidean Algorithm
 * Returns gcd(a, b) and finds x, y such that ax + by = gcd(a, b)
 */
uint64_t prime_gcd_extended(uint64_t a, uint64_t b, int64_t *x, int64_t *y) {
    if (b == 0) {
        *x = 1;
        *y = 0;
        return a;
    }
    
    int64_t x1, y1;
    uint64_t gcd = prime_gcd_extended(b, a % b, &x1, &y1);
    
    *x = y1;
    *y = x1 - (a / b) * y1;
    
    return gcd;
}

/**
 * Modular multiplicative inverse
 * Returns x such that (a * x) mod m = 1
 * Returns 0 if inverse doesn't exist
 */
uint64_t prime_modinv(uint64_t a, uint64_t m) {
    int64_t x, y;
    uint64_t g = prime_gcd_extended(a, m, &x, &y);
    
    if (g != 1) return 0;  // Inverse doesn't exist
    
    // Make x positive
    int64_t result = (x % (int64_t)m + (int64_t)m) % (int64_t)m;
    return (uint64_t)result;
}

/* ============================================================================
 * FIXED-POINT ARITHMETIC FOR TRANSCENDENTAL FUNCTIONS
 * ============================================================================ */

/**
 * Fixed-point representation: value = integer / FIXED_POINT_SCALE
 * This allows us to do fractional arithmetic using only integers
 */
// FIXED_POINT_SCALE defined in prime_types.h

/**
 * Multiply two fixed-point numbers
 */
uint64_t prime_fixedpoint_mul(uint64_t a, uint64_t b) {
    // Use 128-bit intermediate to avoid overflow
    __uint128_t result = (__uint128_t)a * (__uint128_t)b;
    return (uint64_t)(result / FIXED_POINT_SCALE);
}

/**
 * Divide two fixed-point numbers
 */
uint64_t prime_fixedpoint_div(uint64_t a, uint64_t b) {
    if (b == 0) return 0;
    __uint128_t result = ((__uint128_t)a * FIXED_POINT_SCALE) / b;
    return (uint64_t)result;
}

/**
 * Square root of fixed-point number using Newton's method
 */
uint64_t prime_fixedpoint_sqrt(uint64_t n) {
    if (n == 0) return 0;
    
    uint64_t x = n;
    uint64_t y = (x + FIXED_POINT_SCALE) / 2;
    
    while (y < x) {
        x = y;
        y = (x + prime_fixedpoint_div(n, x)) / 2;
    }
    
    return x;
}

/* ============================================================================
 * CONSTANTS (Computed at compile time or initialization)
 * ============================================================================ */

// PI with fixed-point precision: 3.141592653589793
static const uint64_t PRIME_PI_FIXED = 3141592653ULL;

// PHI (Golden Ratio) with fixed-point precision: 1.618033988749895
static const uint64_t PRIME_PHI_FIXED = 1618033988ULL;

// E (Euler's number) with fixed-point precision: 2.718281828459045
static const uint64_t PRIME_E_FIXED = 2718281828ULL;

// SQRT(5) with fixed-point precision: 2.23606797749979
static const uint64_t PRIME_SQRT5_FIXED = 2236067977ULL;

/**
 * Get PI as fixed-point number
 */
uint64_t prime_pi_fixed(void) {
    return PRIME_PI_FIXED;
}

/**
 * Get PHI (Golden Ratio) as fixed-point number
 */
uint64_t prime_phi_fixed(void) {
    return PRIME_PHI_FIXED;
}

/**
 * Get E (Euler's number) as fixed-point number
 */
uint64_t prime_e_fixed(void) {
    return PRIME_E_FIXED;
}

/**
 * Get SQRT(5) as fixed-point number
 */
uint64_t prime_sqrt5_fixed(void) {
    return PRIME_SQRT5_FIXED;
}

/* ============================================================================
 * TRIGONOMETRIC FUNCTIONS USING CORDIC ALGORITHM
 * ============================================================================ */

/**
 * CORDIC algorithm for sine and cosine
 * Input: angle in fixed-point radians
 * Output: sin and cos in fixed-point
 */
void prime_cordic_sincos(uint64_t angle, int64_t *sin_result, int64_t *cos_result) {
    // CORDIC iteration count for precision
    const int iterations = 32;
    
    // CORDIC angles (arctangent table in fixed-point)
    static const uint64_t cordic_angles[] = {
        785398163,   // atan(2^0)
        463647609,   // atan(2^-1)
        244978663,   // atan(2^-2)
        124354995,   // atan(2^-3)
        62418810,    // atan(2^-4)
        31239833,    // atan(2^-5)
        15623729,    // atan(2^-6)
        7812341,     // atan(2^-7)
        3906230,     // atan(2^-8)
        1953123,     // atan(2^-9)
        976562,      // atan(2^-10)
        488281,      // atan(2^-11)
        244140,      // atan(2^-12)
        122070,      // atan(2^-13)
        61035,       // atan(2^-14)
        30517,       // atan(2^-15)
        15259,       // atan(2^-16)
        7629,        // atan(2^-17)
        3815,        // atan(2^-18)
        1907,        // atan(2^-19)
        954,         // atan(2^-20)
        477,         // atan(2^-21)
        238,         // atan(2^-22)
        119,         // atan(2^-23)
        60,          // atan(2^-24)
        30,          // atan(2^-25)
        15,          // atan(2^-26)
        7,           // atan(2^-27)
        4,           // atan(2^-28)
        2,           // atan(2^-29)
        1,           // atan(2^-30)
        0            // atan(2^-31)
    };
    
    // CORDIC gain factor K ≈ 0.6072529350088812561694
    const uint64_t K = 607252935ULL;
    
    // Normalize angle to [0, 2π)
    uint64_t two_pi = 2 * PRIME_PI_FIXED;
    angle = angle % two_pi;
    
    // Determine quadrant and adjust
    int quadrant = 0;
    uint64_t half_pi = PRIME_PI_FIXED / 2;
    
    if (angle >= PRIME_PI_FIXED) {
        angle -= PRIME_PI_FIXED;
        quadrant = 2;
    }
    if (angle >= half_pi) {
        angle = PRIME_PI_FIXED - angle;
        quadrant++;
    }
    
    // Initialize CORDIC
    int64_t x = K;
    int64_t y = 0;
    uint64_t z = angle;
    
    // CORDIC iterations
    for (int i = 0; i < iterations && i < 32; i++) {
        int64_t d = ((int64_t)z >= 0) ? 1 : -1;
        int64_t x_new = x - d * (y >> i);
        int64_t y_new = y + d * (x >> i);
        uint64_t z_new = z - d * cordic_angles[i];
        
        x = x_new;
        y = y_new;
        z = z_new;
    }
    
    // Adjust for quadrant
    switch (quadrant) {
        case 0: *cos_result = x; *sin_result = y; break;
        case 1: *cos_result = -y; *sin_result = x; break;
        case 2: *cos_result = -x; *sin_result = -y; break;
        case 3: *cos_result = y; *sin_result = -x; break;
    }
}

/**
 * Sine function using CORDIC
 */
int64_t prime_sin_fixed(uint64_t angle) {
    int64_t sin_val, cos_val;
    prime_cordic_sincos(angle, &sin_val, &cos_val);
    return sin_val;
}

/**
 * Cosine function using CORDIC
 */
int64_t prime_cos_fixed(uint64_t angle) {
    int64_t sin_val, cos_val;
    prime_cordic_sincos(angle, &sin_val, &cos_val);
    return cos_val;
}

/* ============================================================================
 * EXPONENTIAL AND LOGARITHM USING TAYLOR SERIES
 * ============================================================================ */

/**
 * Natural exponential function e^x using Taylor series
 * Input: x in fixed-point
 * Output: e^x in fixed-point
 */
uint64_t prime_exp_fixed(int64_t x) {
    // For large x, use exp(x) = exp(x/2)^2 to reduce range
    if (x > 2 * (int64_t)FIXED_POINT_SCALE) {
        uint64_t half = prime_exp_fixed(x / 2);
        return prime_fixedpoint_mul(half, half);
    }
    if (x < -2 * (int64_t)FIXED_POINT_SCALE) {
        uint64_t half = prime_exp_fixed(-x / 2);
        uint64_t denom = prime_fixedpoint_mul(half, half);
        return prime_fixedpoint_div(FIXED_POINT_SCALE, denom);
    }
    
    // Taylor series: e^x = 1 + x + x^2/2! + x^3/3! + ...
    int64_t result = FIXED_POINT_SCALE;  // Start with 1
    int64_t term = FIXED_POINT_SCALE;    // Current term
    
    for (int n = 1; n < 50; n++) {
        term = (term * x) / (int64_t)FIXED_POINT_SCALE;
        term = term / n;
        result += term;
        
        // Stop when term becomes negligible
        if (term < 1000 && term > -1000) break;
    }
    
    return (uint64_t)result;
}

/**
 * Natural logarithm ln(x) using series expansion
 * Input: x in fixed-point (must be > 0)
 * Output: ln(x) in fixed-point
 */
int64_t prime_log_fixed(uint64_t x) {
    if (x == 0) return INT64_MIN;  // -infinity
    if (x == FIXED_POINT_SCALE) return 0;  // ln(1) = 0
    
    // For x > 2, use ln(x) = ln(x/e) + 1
    if (x > 2 * FIXED_POINT_SCALE) {
        uint64_t reduced = prime_fixedpoint_div(x, PRIME_E_FIXED);
        return prime_log_fixed(reduced) + (int64_t)FIXED_POINT_SCALE;
    }
    
    // For x < 0.5, use ln(x) = -ln(1/x)
    if (x < FIXED_POINT_SCALE / 2) {
        uint64_t inv = prime_fixedpoint_div(FIXED_POINT_SCALE, x);
        return -prime_log_fixed(inv);
    }
    
    // Use series: ln(x) = 2 * (y + y^3/3 + y^5/5 + ...) where y = (x-1)/(x+1)
    int64_t y_num = (int64_t)x - (int64_t)FIXED_POINT_SCALE;
    int64_t y_den = (int64_t)x + (int64_t)FIXED_POINT_SCALE;
    int64_t y = (y_num * (int64_t)FIXED_POINT_SCALE) / y_den;
    
    int64_t y_squared = (y * y) / (int64_t)FIXED_POINT_SCALE;
    int64_t term = y;
    int64_t result = term;
    
    for (int n = 1; n < 50; n++) {
        term = (term * y_squared) / (int64_t)FIXED_POINT_SCALE;
        result += term / (2 * n + 1);
        
        if (term < 1000 && term > -1000) break;
    }
    
    return 2 * result;
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * Absolute value
 */
uint64_t prime_abs_int(int64_t x) {
    return (x < 0) ? (uint64_t)(-x) : (uint64_t)x;
}

/**
 * Minimum of two integers
 */
uint64_t prime_min_int(uint64_t a, uint64_t b) {
    return (a < b) ? a : b;
}

/**
 * Maximum of two integers
 */
uint64_t prime_max_int(uint64_t a, uint64_t b) {
    return (a > b) ? a : b;
}

/**
 * Clamp value between min and max
 */
uint64_t prime_clamp_int(uint64_t value, uint64_t min, uint64_t max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * Check if number is power of 2
 */
int prime_is_power_of_2(uint64_t n) {
    return n != 0 && (n & (n - 1)) == 0;
}

/**
 * Round up to next power of 2
 */
uint64_t prime_next_power_of_2(uint64_t n) {
    if (n == 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;
    return n;
}