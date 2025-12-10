/**
 * @file bigint_arithmetic.c
 * @brief BigInt arithmetic operations
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 */

#include "math/bigint.h"
#include <stdlib.h>
#include <string.h>

/* Forward declarations for internal helpers */
static int bigint_ensure_capacity(BigInt* bi, size_t min_capacity);
static void bigint_normalize(BigInt* bi);
static int bigint_add_magnitude(BigInt* result, const BigInt* a, const BigInt* b);
static int bigint_sub_magnitude(BigInt* result, const BigInt* a, const BigInt* b);
static int bigint_cmp_magnitude(const BigInt* a, const BigInt* b);

/* ============================================================================
 * BASIC ARITHMETIC
 * ============================================================================
 */

MathError bigint_add(BigInt* result, const BigInt* a, const BigInt* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Handle signs */
    if (a->negative == b->negative) {
        /* Same sign: add magnitudes, keep sign */
        if (bigint_add_magnitude(result, a, b) != 0) {
            return MATH_ERROR_INVALID_ARG;
        }
        result->negative = a->negative;
    } else {
        /* Different signs: subtract magnitudes */
        int cmp = bigint_cmp_magnitude(a, b);
        if (cmp >= 0) {
            if (bigint_sub_magnitude(result, a, b) != 0) {
                return MATH_ERROR_INVALID_ARG;
            }
            result->negative = a->negative;
        } else {
            if (bigint_sub_magnitude(result, b, a) != 0) {
                return MATH_ERROR_INVALID_ARG;
            }
            result->negative = b->negative;
        }
    }
    
    bigint_normalize(result);
    return MATH_SUCCESS;
}

MathError bigint_sub(BigInt* result, const BigInt* a, const BigInt* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* a - b = a + (-b) */
    if (a->negative != b->negative) {
        /* Different signs: add magnitudes */
        if (bigint_add_magnitude(result, a, b) != 0) {
            return MATH_ERROR_INVALID_ARG;
        }
        result->negative = a->negative;
    } else {
        /* Same sign: subtract magnitudes */
        int cmp = bigint_cmp_magnitude(a, b);
        if (cmp >= 0) {
            if (bigint_sub_magnitude(result, a, b) != 0) {
                return MATH_ERROR_INVALID_ARG;
            }
            result->negative = a->negative;
        } else {
            if (bigint_sub_magnitude(result, b, a) != 0) {
                return MATH_ERROR_INVALID_ARG;
            }
            result->negative = !a->negative;
        }
    }
    
    bigint_normalize(result);
    return MATH_SUCCESS;
}

MathError bigint_mul(BigInt* result, const BigInt* a, const BigInt* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Handle zero */
    if (bigint_is_zero(a) || bigint_is_zero(b)) {
        result->digits[0] = 0;
        result->length = 1;
        result->negative = false;
        return MATH_SUCCESS;
    }
    
    size_t result_len = a->length + b->length;
    if (bigint_ensure_capacity(result, result_len) != 0) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Clear result */
    memset(result->digits, 0, result_len * sizeof(uint32_t));
    
    /* Multiply digit by digit */
    for (size_t i = 0; i < a->length; i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < b->length; j++) {
            uint64_t product = (uint64_t)a->digits[i] * (uint64_t)b->digits[j];
            uint64_t sum = (uint64_t)result->digits[i + j] + product + carry;
            result->digits[i + j] = (uint32_t)(sum & 0xFFFFFFFF);
            carry = sum >> 32;
        }
        if (carry > 0) {
            result->digits[i + b->length] = (uint32_t)carry;
        }
    }
    
    result->length = result_len;
    result->negative = (a->negative != b->negative);
    
    bigint_normalize(result);
    return MATH_SUCCESS;
}

MathError bigint_div(BigInt* quotient, BigInt* remainder, const BigInt* a, const BigInt* b) {
    if (!quotient || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Check for division by zero */
    if (bigint_is_zero(b)) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Handle zero dividend */
    if (bigint_is_zero(a)) {
        quotient->digits[0] = 0;
        quotient->length = 1;
        quotient->negative = false;
        if (remainder) {
            remainder->digits[0] = 0;
            remainder->length = 1;
            remainder->negative = false;
        }
        return MATH_SUCCESS;
    }
    
    /* Simple case: divisor is larger than dividend */
    int cmp = bigint_cmp_magnitude(a, b);
    if (cmp < 0) {
        quotient->digits[0] = 0;
        quotient->length = 1;
        quotient->negative = false;
        if (remainder) {
            /* remainder = a */
            if (bigint_ensure_capacity(remainder, a->length) != 0) {
                return MATH_ERROR_INVALID_ARG;
            }
            memcpy(remainder->digits, a->digits, a->length * sizeof(uint32_t));
            remainder->length = a->length;
            remainder->negative = a->negative;
        }
        return MATH_SUCCESS;
    }
    
    /* Simple case: divisor equals dividend */
    if (cmp == 0) {
        quotient->digits[0] = 1;
        quotient->length = 1;
        quotient->negative = (a->negative != b->negative);
        if (remainder) {
            remainder->digits[0] = 0;
            remainder->length = 1;
            remainder->negative = false;
        }
        return MATH_SUCCESS;
    }
    
    /* Long division algorithm */
    /* For now, implement a simple repeated subtraction for small numbers */
    /* TODO: Implement efficient long division for large numbers */
    
    BigInt* temp_remainder = bigint_copy(a);
    if (!temp_remainder) {
        return MATH_ERROR_INVALID_ARG;
    }
    temp_remainder->negative = false;  /* Work with magnitudes */
    
    BigInt* temp_divisor = bigint_copy(b);
    if (!temp_divisor) {
        bigint_free(temp_remainder);
        return MATH_ERROR_INVALID_ARG;
    }
    temp_divisor->negative = false;
    
    /* Initialize quotient to zero */
    quotient->digits[0] = 0;
    quotient->length = 1;
    
    /* Count how many times we can subtract */
    uint64_t count = 0;
    while (bigint_cmp_magnitude(temp_remainder, temp_divisor) >= 0) {
        bigint_sub_magnitude(temp_remainder, temp_remainder, temp_divisor);
        count++;
        
        /* Prevent infinite loops for very large numbers */
        if (count > UINT32_MAX) {
            bigint_free(temp_remainder);
            bigint_free(temp_divisor);
            return MATH_ERROR_INVALID_ARG;
        }
    }
    
    /* Set quotient */
    if (count <= UINT32_MAX) {
        quotient->digits[0] = (uint32_t)count;
        quotient->length = 1;
    } else {
        quotient->digits[0] = (uint32_t)(count & 0xFFFFFFFF);
        if (bigint_ensure_capacity(quotient, 2) != 0) {
            bigint_free(temp_remainder);
            bigint_free(temp_divisor);
            return MATH_ERROR_INVALID_ARG;
        }
        quotient->digits[1] = (uint32_t)(count >> 32);
        quotient->length = 2;
    }
    quotient->negative = (a->negative != b->negative);
    
    /* Set remainder */
    if (remainder) {
        if (bigint_ensure_capacity(remainder, temp_remainder->length) != 0) {
            bigint_free(temp_remainder);
            bigint_free(temp_divisor);
            return MATH_ERROR_INVALID_ARG;
        }
        memcpy(remainder->digits, temp_remainder->digits, temp_remainder->length * sizeof(uint32_t));
        remainder->length = temp_remainder->length;
        remainder->negative = a->negative;
        bigint_normalize(remainder);
    }
    
    bigint_free(temp_remainder);
    bigint_free(temp_divisor);
    bigint_normalize(quotient);
    
    return MATH_SUCCESS;
}

MathError bigint_mod(BigInt* result, const BigInt* a, const BigInt* m) {
    if (!result || !a || !m) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    return bigint_div(result, result, a, m);
}

/* ============================================================================
 * INTERNAL HELPER FUNCTIONS
 * ============================================================================
 */

static int bigint_add_magnitude(BigInt* result, const BigInt* a, const BigInt* b) {
    size_t max_len = (a->length > b->length) ? a->length : b->length;
    
    if (bigint_ensure_capacity(result, max_len + 1) != 0) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    uint64_t carry = 0;
    size_t i;
    
    for (i = 0; i < max_len; i++) {
        uint64_t digit_a = (i < a->length) ? a->digits[i] : 0;
        uint64_t digit_b = (i < b->length) ? b->digits[i] : 0;
        uint64_t sum = digit_a + digit_b + carry;
        
        result->digits[i] = (uint32_t)(sum & 0xFFFFFFFF);
        carry = sum >> 32;
    }
    
    if (carry > 0) {
        result->digits[i] = (uint32_t)carry;
        result->length = max_len + 1;
    } else {
        result->length = max_len;
    }
    
    return MATH_SUCCESS;
}

static int bigint_sub_magnitude(BigInt* result, const BigInt* a, const BigInt* b) {
    /* Assumes a >= b in magnitude */
    
    if (bigint_ensure_capacity(result, a->length) != 0) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    int64_t borrow = 0;
    
    for (size_t i = 0; i < a->length; i++) {
        int64_t digit_a = a->digits[i];
        int64_t digit_b = (i < b->length) ? b->digits[i] : 0;
        int64_t diff = digit_a - digit_b - borrow;
        
        if (diff < 0) {
            diff += 0x100000000LL;
            borrow = 1;
        } else {
            borrow = 0;
        }
        
        result->digits[i] = (uint32_t)diff;
    }
    
    result->length = a->length;
    return MATH_SUCCESS;
}

static int bigint_cmp_magnitude(const BigInt* a, const BigInt* b) {
    if (a->length != b->length) {
        return (a->length > b->length) ? 1 : -1;
    }
    
    for (int i = (int)a->length - 1; i >= 0; i--) {
        if (a->digits[i] != b->digits[i]) {
            return (a->digits[i] > b->digits[i]) ? 1 : -1;
        }
    }
    
    return MATH_SUCCESS;
}

/* Stub implementations for capacity and normalize (defined in bigint_core.c) */
static int bigint_ensure_capacity(BigInt* bi, size_t min_capacity) {
    if (bi->capacity >= min_capacity) {
        return MATH_SUCCESS;
    }
    
    size_t new_capacity = bi->capacity * 2;
    if (new_capacity < min_capacity) {
        new_capacity = min_capacity;
    }
    
    uint32_t* new_digits = (uint32_t*)realloc(bi->digits, new_capacity * sizeof(uint32_t));
    if (!new_digits) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    bi->digits = new_digits;
    bi->capacity = new_capacity;
    
    return MATH_SUCCESS;
}

static void bigint_normalize(BigInt* bi) {
    while (bi->length > 1 && bi->digits[bi->length - 1] == 0) {
        bi->length--;
    }
    
    if (bi->length == 1 && bi->digits[0] == 0) {
        bi->negative = false;
    }
}