/**
 * @file bigfixed_core.c
 * @brief BigFixed core operations implementation
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * BigFixed provides arbitrary precision fixed-point arithmetic.
 * Representation: value = (integer_part + fractional_part / 2^scale_bits) * sign
 * 
 * This allows for exact decimal arithmetic with configurable precision.
 */

#include "math/bigfixed.h"
#include "math/bigint.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * CREATION AND DESTRUCTION
 * ============================================================================
 */

BigFixed* bigfixed_new(size_t scale_bits) {
    BigFixed* bf = (BigFixed*)malloc(sizeof(BigFixed));
    if (!bf) {
        return NULL;
    }
    
    bf->integer_part = bigint_new();
    if (!bf->integer_part) {
        free(bf);
        return NULL;
    }
    
    bf->fractional_part = bigint_new();
    if (!bf->fractional_part) {
        bigint_free(bf->integer_part);
        free(bf);
        return NULL;
    }
    
    bf->scale_bits = scale_bits;
    bf->negative = false;
    
    return bf;
}

void bigfixed_free(BigFixed* bf) {
    if (bf) {
        if (bf->integer_part) {
            bigint_free(bf->integer_part);
        }
        if (bf->fractional_part) {
            bigint_free(bf->fractional_part);
        }
        free(bf);
    }
}

BigFixed* bigfixed_copy(const BigFixed* src) {
    if (!src) {
        return NULL;
    }
    
    BigFixed* copy = bigfixed_new(src->scale_bits);
    if (!copy) {
        return NULL;
    }
    
    bigint_free(copy->integer_part);
    bigint_free(copy->fractional_part);
    
    copy->integer_part = bigint_copy(src->integer_part);
    copy->fractional_part = bigint_copy(src->fractional_part);
    
    if (!copy->integer_part || !copy->fractional_part) {
        bigfixed_free(copy);
        return NULL;
    }
    
    copy->negative = src->negative;
    
    return copy;
}

/* ============================================================================
 * CONVERSION FUNCTIONS
 * ============================================================================
 */

BigFixed* bigfixed_from_double(double value, size_t scale_bits) {
    BigFixed* bf = bigfixed_new(scale_bits);
    if (!bf) {
        return NULL;
    }
    
    /* Handle sign */
    if (value < 0.0) {
        bf->negative = true;
        value = -value;
    }
    
    /* Extract integer part */
    int64_t int_part = (int64_t)value;
    bigint_free(bf->integer_part);
    bf->integer_part = bigint_from_int64(int_part);
    if (!bf->integer_part) {
        bigfixed_free(bf);
        return NULL;
    }
    
    /* Extract fractional part */
    double frac_part = value - (double)int_part;
    
    /* Convert fractional part to fixed-point */
    /* frac_part * 2^scale_bits */
    double scale_factor = 1.0;
    for (size_t i = 0; i < scale_bits; i++) {
        scale_factor *= 2.0;
    }
    
    uint64_t frac_fixed = (uint64_t)(frac_part * scale_factor);
    
    bigint_free(bf->fractional_part);
    bf->fractional_part = bigint_from_uint64(frac_fixed);
    if (!bf->fractional_part) {
        bigfixed_free(bf);
        return NULL;
    }
    
    return bf;
}

double bigfixed_to_double(const BigFixed* bf) {
    if (!bf) {
        return 0.0;
    }
    
    /* Convert integer part */
    double result = (double)bigint_to_int64(bf->integer_part);
    
    /* Convert fractional part */
    uint64_t frac_fixed = bigint_to_uint64(bf->fractional_part);
    
    /* Divide by 2^scale_bits */
    double scale_factor = 1.0;
    for (size_t i = 0; i < bf->scale_bits; i++) {
        scale_factor *= 2.0;
    }
    
    double frac_part = (double)frac_fixed / scale_factor;
    
    result += frac_part;
    
    return bf->negative ? -result : result;
}

BigFixed* bigfixed_from_int64(int64_t value, size_t scale_bits) {
    BigFixed* bf = bigfixed_new(scale_bits);
    if (!bf) {
        return NULL;
    }
    
    if (value < 0) {
        bf->negative = true;
        value = -value;
    }
    
    bigint_free(bf->integer_part);
    bf->integer_part = bigint_from_int64(value);
    if (!bf->integer_part) {
        bigfixed_free(bf);
        return NULL;
    }
    
    /* Fractional part is already zero from bigfixed_new */
    
    return bf;
}

int64_t bigfixed_to_int64(const BigFixed* bf) {
    if (!bf) {
        return 0;
    }
    
    int64_t result = bigint_to_int64(bf->integer_part);
    return bf->negative ? -result : result;
}

BigFixed* bigfixed_from_string(const char* str, size_t scale_bits) {
    if (!str) {
        return NULL;
    }
    
    BigFixed* bf = bigfixed_new(scale_bits);
    if (!bf) {
        return NULL;
    }
    
    /* Parse sign */
    const char* ptr = str;
    if (*ptr == '-') {
        bf->negative = true;
        ptr++;
    } else if (*ptr == '+') {
        ptr++;
    }
    
    /* Parse integer part */
    char int_str[256];
    size_t int_len = 0;
    while (*ptr >= '0' && *ptr <= '9' && int_len < 255) {
        int_str[int_len++] = *ptr++;
    }
    int_str[int_len] = '\0';
    
    if (int_len > 0) {
        int64_t int_val = 0;
        for (size_t i = 0; i < int_len; i++) {
            int_val = int_val * 10 + (int_str[i] - '0');
        }
        bigint_free(bf->integer_part);
        bf->integer_part = bigint_from_int64(int_val);
    }
    
    /* Parse fractional part */
    if (*ptr == '.') {
        ptr++;
        
        char frac_str[256];
        size_t frac_len = 0;
        while (*ptr >= '0' && *ptr <= '9' && frac_len < 255) {
            frac_str[frac_len++] = *ptr++;
        }
        frac_str[frac_len] = '\0';
        
        if (frac_len > 0) {
            /* Convert decimal fraction to binary fixed-point */
            double frac_val = 0.0;
            double divisor = 1.0;
            for (size_t i = 0; i < frac_len; i++) {
                divisor *= 10.0;
                frac_val = frac_val * 10.0 + (frac_str[i] - '0');
            }
            frac_val /= divisor;
            
            /* Convert to fixed-point */
            double scale_factor = 1.0;
            for (size_t i = 0; i < scale_bits; i++) {
                scale_factor *= 2.0;
            }
            
            uint64_t frac_fixed = (uint64_t)(frac_val * scale_factor);
            
            bigint_free(bf->fractional_part);
            bf->fractional_part = bigint_from_uint64(frac_fixed);
        }
    }
    
    return bf;
}

char* bigfixed_to_string(const BigFixed* bf, int decimal_places) {
    if (!bf) {
        return NULL;
    }
    
    /* Convert to double and format */
    double value = bigfixed_to_double(bf);
    
    /* Allocate buffer (enough for sign + integer + decimal + fractional + null) */
    char* buffer = (char*)malloc(256);
    if (!buffer) {
        return NULL;
    }
    
    /* Use provided decimal places, or calculate from scale_bits */
    if (decimal_places <= 0) {
        decimal_places = (int)(bf->scale_bits * 0.301);  /* log10(2) ≈ 0.301 */
    }
    if (decimal_places > 50) {
        decimal_places = 50;
    }
    
    snprintf(buffer, 256, "%.*f", decimal_places, value);
    
    return buffer;
}

/* ============================================================================
 * COMPARISON
 * ============================================================================
 */

int bigfixed_cmp(const BigFixed* a, const BigFixed* b) {
    if (!a || !b) {
        return 0;
    }
    
    /* Handle signs */
    if (a->negative != b->negative) {
        return a->negative ? -1 : 1;
    }
    
    /* Compare integer parts */
    int int_cmp = bigint_cmp(a->integer_part, b->integer_part);
    if (int_cmp != 0) {
        return a->negative ? -int_cmp : int_cmp;
    }
    
    /* Integer parts equal, compare fractional parts */
    int frac_cmp = bigint_cmp(a->fractional_part, b->fractional_part);
    return a->negative ? -frac_cmp : frac_cmp;
}

bool bigfixed_is_zero(const BigFixed* bf) {
    if (!bf) {
        return true;
    }
    
    return bigint_is_zero(bf->integer_part) && bigint_is_zero(bf->fractional_part);
}

bool bigfixed_is_negative(const BigFixed* bf) {
    return bf && bf->negative && !bigfixed_is_zero(bf);
}

bool bigfixed_is_positive(const BigFixed* bf) {
    return bf && !bf->negative && !bigfixed_is_zero(bf);
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================
 */

size_t bigfixed_get_scale_bits(const BigFixed* bf) {
    return bf ? bf->scale_bits : 0;
}

MathError bigfixed_set_precision(BigFixed* bf, size_t new_scale_bits) {
    if (!bf) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* TODO: Implement precision conversion */
    /* For now, just update the scale_bits */
    bf->scale_bits = new_scale_bits;
    
    return MATH_SUCCESS;
}