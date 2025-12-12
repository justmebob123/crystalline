/**
 * @file uint64_arithmetic.c
 * @brief Integer arithmetic operations for uint64_t
 * 
 * INDEPENDENT IMPLEMENTATION - NO dependencies on OLD library
 * Pure mathematical implementations using only standard integer operations
 */

#include "math/arithmetic.h"
#include "math/types.h"
#include <stdint.h>

/**
 * @brief Modular exponentiation using binary method
 * 
 * Computes (base^exponent) mod modulus efficiently using
 * the binary exponentiation algorithm (square-and-multiply).
 * 
 * Time complexity: O(log exponent)
 */
uint64_t uint64_powmod(uint64_t base, uint64_t exponent, uint64_t modulus) {
    if (modulus == 0) return 0;
    if (modulus == 1) return 0;
    if (exponent == 0) return 1;
    
    uint64_t result = 1;
    base = base % modulus;
    
    while (exponent > 0) {
        // If exponent is odd, multiply base with result
        if (exponent & 1) {
            // Use 128-bit arithmetic to avoid overflow
            __uint128_t temp = ((__uint128_t)result * (__uint128_t)base) % modulus;
            result = (uint64_t)temp;
        }
        
        // Square the base
        exponent >>= 1;
        if (exponent > 0) {
            __uint128_t temp = ((__uint128_t)base * (__uint128_t)base) % modulus;
            base = (uint64_t)temp;
        }
    }
    
    return result;
}

/**
 * @brief Greatest common divisor using Euclidean algorithm
 * 
 * Classic recursive GCD algorithm.
 * Time complexity: O(log(min(a, b)))
 */
uint64_t uint64_gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * @brief Least common multiple
 * 
 * LCM(a, b) = (a * b) / GCD(a, b)
 */
uint64_t uint64_lcm(uint64_t a, uint64_t b) {
    if (a == 0 || b == 0) return 0;
    
    uint64_t gcd = uint64_gcd(a, b);
    
    // Avoid overflow by dividing first
    return (a / gcd) * b;
}

/**
 * @brief Modular inverse using extended Euclidean algorithm
 * 
 * Finds x such that (a * x) mod modulus = 1
 * Returns MATH_SUCCESS if inverse exists (when GCD(a, modulus) = 1)
 */
MathError uint64_modinv(uint64_t a, uint64_t modulus, uint64_t* inverse) {
    if (!inverse) return MATH_ERROR_NULL_POINTER;
    if (modulus <= 1) return MATH_ERROR_INVALID_ARGUMENT;
    
    a = a % modulus;
    if (a == 0) return MATH_ERROR_INVALID_ARGUMENT;
    
    // Extended Euclidean algorithm
    int64_t t = 0, newt = 1;
    uint64_t r = modulus, newr = a;
    
    while (newr != 0) {
        uint64_t quotient = r / newr;
        
        int64_t temp_t = t;
        t = newt;
        newt = temp_t - (int64_t)quotient * newt;
        
        uint64_t temp_r = r;
        r = newr;
        newr = temp_r - quotient * newr;
    }
    
    if (r > 1) {
        // a is not invertible
        return MATH_ERROR_INVALID_ARGUMENT;
    }
    
    if (t < 0) {
        t = t + (int64_t)modulus;
    }
    
    *inverse = (uint64_t)t;
    return MATH_SUCCESS;
}

/**
 * @brief Integer power (no modulus)
 * 
 * WARNING: Can overflow! Use uint64_powmod for large values.
 */
uint64_t uint64_pow(uint64_t base, uint64_t exponent) {
    if (exponent == 0) return 1;
    if (base == 0) return 0;
    if (base == 1) return 1;
    
    uint64_t result = 1;
    
    while (exponent > 0) {
        if (exponent & 1) {
            result *= base;
        }
        base *= base;
        exponent >>= 1;
    }
    
    return result;
}

/**
 * @brief Integer square root using Newton's method
 * 
 * Returns floor(sqrt(n))
 */
uint64_t uint64_sqrt(uint64_t n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    
    // Initial guess using bit manipulation
    uint64_t x = n;
    uint64_t y = (x + 1) / 2;
    
    // Newton's method: x_new = (x + n/x) / 2
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    
    return x;
}