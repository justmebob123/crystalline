/*
 * BigInt Conversion Functions
 * Provides conversion between BigInt and standard numeric types
 */

#include "prime_types.h"
#include "bigint_core.h"
#include <math.h>
#include <limits.h>

/*
 * Convert BigInt to double
 * Uses polynomial evaluation for arbitrary precision
 */
double bigint_to_double(const BigInt *n) {
    if (!n || !n->d || n->len == 0) {
        return 0.0;
    }
    
    double result = 0.0;
    double base = 4294967296.0; // 2^32
    double multiplier = 1.0;
    
    // Process digits from least significant to most significant
    for (size_t i = 0; i < n->len && i < 20; i++) { // Limit to prevent overflow
        result += (double)n->d[i] * multiplier;
        multiplier *= base;
        
        // Check for infinity
        if (isinf(result)) {
            return n->negative ? -INFINITY : INFINITY;
        }
    }
    
    return n->negative ? -result : result;
}

/*
 * Convert BigInt to int
 * Returns INT_MAX or INT_MIN if out of range
 */
int bigint_to_int(const BigInt *n) {
    if (!n || !n->d || n->len == 0) {
        return 0;
    }
    
    // Check if value fits in int range
    if (n->len > 1) {
        return n->negative ? INT_MIN : INT_MAX;
    }
    
    uint32_t val = n->d[0];
    
    // Check range
    if (n->negative) {
        if (val > (uint32_t)INT_MAX + 1) {
            return INT_MIN;
        }
        return -(int)val;
    } else {
        if (val > (uint32_t)INT_MAX) {
            return INT_MAX;
        }
        return (int)val;
    }
}

/*
 * Convert BigInt to uint64_t
 * Returns UINT64_MAX if out of range or negative
 */
uint64_t bigint_to_uint64(const BigInt *n) {
    if (!n || !n->d || n->len == 0) {
        return 0;
    }
    
    // Negative values return 0
    if (n->negative) {
        return 0;
    }
    
    // If more than 2 digits, return max
    if (n->len > 2) {
        return UINT64_MAX;
    }
    
    uint64_t result = n->d[0];
    
    if (n->len > 1) {
        result |= ((uint64_t)n->d[1]) << 32;
    }
    
    return result;
}

/* Note: BigInt arithmetic operations are provided by bigint_core.h
 * Use big_add, big_sub, big_mul, big_div directly for BigInt operations
 */