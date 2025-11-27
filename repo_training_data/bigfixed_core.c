/*
 * bigfixed_core.c - Complete Fixed-Point Arbitrary Precision Arithmetic
 * 
 * Clean implementation using the correct BigInt API from crystal_abacus.h
 * 
 * BigInt API Pattern:
 * - BigInt must be allocated by caller
 * - big_init(BigInt *n) initializes
 * - big_from_int(BigInt *n, uint64_t val) sets value
 * - big_free(BigInt *n) frees contents (not pointer)
 * - Operations: big_add(a, b, result), big_sub(a, b, result), etc.
 * 
 * Representation: value = (integer_part + fractional_part / 2^scale_bits) * sign
 */

#include "bigfixed_core.h"
#include "prime_lowlevel.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * CREATION AND DESTRUCTION
 * ============================================================================ */

BigFixed* big_fixed_create(int scale_bits) {
    if (scale_bits < 1) scale_bits = FIXED_POINT_BITS_DEFAULT;
    
    BigFixed *f = (BigFixed*)malloc(sizeof(BigFixed));
    if (!f) return NULL;
    
    // Allocate BigInt structures
    f->integer_part = (BigInt*)malloc(sizeof(BigInt));
    f->fractional_part = (BigInt*)malloc(sizeof(BigInt));
    
    if (!f->integer_part || !f->fractional_part) {
        if (f->integer_part) free(f->integer_part);
        if (f->fractional_part) free(f->fractional_part);
        free(f);
        return NULL;
    }
    
    // Initialize BigInts
    big_init(f->integer_part);
    big_init(f->fractional_part);
    
    f->scale_bits = scale_bits;
    f->negative = false;
    
    return f;
}

void big_fixed_free(BigFixed *f) {
    if (!f) return;
    
    if (f->integer_part) {
        big_free(f->integer_part);
        free(f->integer_part);
    }
    
    if (f->fractional_part) {
        big_free(f->fractional_part);
        free(f->fractional_part);
    }
    
    free(f);
}

BigFixed* big_fixed_copy(const BigFixed *f) {
    if (!f) return NULL;
    
    BigFixed *result = big_fixed_create(f->scale_bits);
    if (!result) return NULL;
    
    big_copy(result->integer_part, f->integer_part);
    big_copy(result->fractional_part, f->fractional_part);
    result->negative = f->negative;
    
    return result;
}

void big_fixed_assign(BigFixed *dest, const BigFixed *src) {
    if (!dest || !src) return;
    
    // Copy integer part
    big_copy(dest->integer_part, src->integer_part);
    
    // Copy fractional part
    big_copy(dest->fractional_part, src->fractional_part);
    
    // Copy properties
    dest->negative = src->negative;
    dest->scale_bits = src->scale_bits;
}

/* ============================================================================
 * CONVERSION FUNCTIONS
 * ============================================================================ */

void big_fixed_from_bigint(BigFixed *result, const BigInt *n) {
    if (!result || !n) return;
    
    big_copy(result->integer_part, n);
    big_from_int(result->fractional_part, 0);
    result->negative = n->negative;
}

void big_fixed_from_int(BigFixed *result, int64_t n) {
    if (!result) return;
    
    uint64_t abs_n = (n < 0) ? (uint64_t)(-n) : (uint64_t)n;
    big_from_int(result->integer_part, abs_n);
    big_from_int(result->fractional_part, 0);
    result->negative = (n < 0);
}

void big_fixed_to_bigint(BigInt *result, const BigFixed *f) {
    if (!result || !f) return;
    
    // Truncate: just take integer part
    big_copy(result, f->integer_part);
    result->negative = f->negative;
}

void big_fixed_to_bigint_rounded(BigInt *result, const BigFixed *f) {
    if (!result || !f) return;
    
    // Round to nearest integer
    big_copy(result, f->integer_part);
    
    // Check if fractional part >= 0.5 (i.e., >= 2^(scale_bits-1))
    BigInt half, one;
    big_init(&half);
    big_init(&one);
    
    big_from_int(&half, 1);
    big_shl(&half, f->scale_bits - 1);
    
    if (big_cmp(f->fractional_part, &half) >= 0) {
        // Round up
        big_from_int(&one, 1);
        big_add(result, &one, result);
    }
    
    result->negative = f->negative;
    
    big_free(&half);
    big_free(&one);
}

/* ============================================================================
 * COMPARISON
 * ============================================================================ */

int big_fixed_cmp(const BigFixed *a, const BigFixed *b) {
    if (!a || !b) return 0;
    
    // Handle signs
    if (a->negative && !b->negative) return -1;
    if (!a->negative && b->negative) return 1;
    
    // Same sign: compare magnitudes
    int int_cmp = big_cmp(a->integer_part, b->integer_part);
    if (int_cmp != 0) {
        return a->negative ? -int_cmp : int_cmp;
    }
    
    // Integer parts equal, compare fractional
    int frac_cmp = big_cmp(a->fractional_part, b->fractional_part);
    return a->negative ? -frac_cmp : frac_cmp;
}

bool big_fixed_is_zero(const BigFixed *f) {
    if (!f) return true;
    return (big_is_zero(f->integer_part) && big_is_zero(f->fractional_part));
}

bool big_fixed_is_negative(const BigFixed *f) {
    if (!f) return false;
    return f->negative && !big_fixed_is_zero(f);
}

/* ============================================================================
 * NORMALIZATION
 * ============================================================================ */

static void big_fixed_normalize(BigFixed *f) {
    if (!f) return;
    
    // Handle fractional overflow: if frac >= 2^scale_bits, carry to integer
    BigInt scale, one;
    big_init(&scale);
    big_init(&one);
    
    big_from_int(&scale, 1);
    big_shl(&scale, f->scale_bits);
    
    while (big_cmp(f->fractional_part, &scale) >= 0) {
        BigInt temp;
        big_init(&temp);
        big_sub(f->fractional_part, &scale, &temp);
        big_copy(f->fractional_part, &temp);
        big_free(&temp);
        
        big_from_int(&one, 1);
        big_init(&temp);
        big_add(f->integer_part, &one, &temp);
        big_copy(f->integer_part, &temp);
        big_free(&temp);
    }
    
    // Handle zero case
    if (big_fixed_is_zero(f)) {
        f->negative = false;
    }
    
    big_free(&scale);
    big_free(&one);
}

/* ============================================================================
 * RESCALING
 * ============================================================================ */

static void big_fixed_rescale(BigFixed *result, const BigFixed *f, int new_scale_bits) {
    if (!result || !f) return;
    
    big_copy(result->integer_part, f->integer_part);
    result->negative = f->negative;
    
    if (new_scale_bits == (int)f->scale_bits) {
        big_copy(result->fractional_part, f->fractional_part);
    } else if (new_scale_bits > (int)f->scale_bits) {
        // Scale up: shift left
        int shift = new_scale_bits - f->scale_bits;
        big_copy(result->fractional_part, f->fractional_part);
        big_shl(result->fractional_part, shift);
    } else {
        // Scale down: shift right (loses precision)
        int shift = f->scale_bits - new_scale_bits;
        big_copy(result->fractional_part, f->fractional_part);
        big_shr(result->fractional_part, shift);
    }
    
    result->scale_bits = new_scale_bits;
}

/* ============================================================================
 * ADDITION
 * ============================================================================ */

void big_fixed_add(BigFixed *result, const BigFixed *a, const BigFixed *b) {
    if (!result || !a || !b) return;
    
    // Ensure same scale
    int target_scale = (a->scale_bits > b->scale_bits) ? a->scale_bits : b->scale_bits;
    
    BigFixed *a_scaled = big_fixed_create(target_scale);
    BigFixed *b_scaled = big_fixed_create(target_scale);
    big_fixed_rescale(a_scaled, a, target_scale);
    big_fixed_rescale(b_scaled, b, target_scale);
    
    result->scale_bits = target_scale;
    
    // Handle signs
    if (a_scaled->negative == b_scaled->negative) {
        // Same sign: add magnitudes
        big_add(a_scaled->integer_part, b_scaled->integer_part, result->integer_part);
        big_add(a_scaled->fractional_part, b_scaled->fractional_part, result->fractional_part);
        result->negative = a_scaled->negative;
        big_fixed_normalize(result);
    } else {
        // Different signs: subtract magnitudes
        // Compare absolute values
        int cmp = big_cmp(a_scaled->integer_part, b_scaled->integer_part);
        if (cmp == 0) {
            cmp = big_cmp(a_scaled->fractional_part, b_scaled->fractional_part);
        }
        
        if (cmp >= 0) {
            // |a| >= |b|: result = a - b
            big_sub(a_scaled->integer_part, b_scaled->integer_part, result->integer_part);
            
            if (big_cmp(a_scaled->fractional_part, b_scaled->fractional_part) >= 0) {
                big_sub(a_scaled->fractional_part, b_scaled->fractional_part, result->fractional_part);
            } else {
                // Borrow from integer part
                BigInt scale, temp;
                big_init(&scale);
                big_init(&temp);
                
                big_from_int(&scale, 1);
                big_shl(&scale, target_scale);
                big_add(a_scaled->fractional_part, &scale, &temp);
                big_sub(&temp, b_scaled->fractional_part, result->fractional_part);
                
                BigInt one;
                big_init(&one);
                big_from_int(&one, 1);
                big_sub(result->integer_part, &one, &temp);
                big_copy(result->integer_part, &temp);
                
                big_free(&scale);
                big_free(&temp);
                big_free(&one);
            }
            
            result->negative = a_scaled->negative;
        } else {
            // |b| > |a|: result = b - a
            big_sub(b_scaled->integer_part, a_scaled->integer_part, result->integer_part);
            
            if (big_cmp(b_scaled->fractional_part, a_scaled->fractional_part) >= 0) {
                big_sub(b_scaled->fractional_part, a_scaled->fractional_part, result->fractional_part);
            } else {
                // Borrow from integer part
                BigInt scale, temp;
                big_init(&scale);
                big_init(&temp);
                
                big_from_int(&scale, 1);
                big_shl(&scale, target_scale);
                big_add(b_scaled->fractional_part, &scale, &temp);
                big_sub(&temp, a_scaled->fractional_part, result->fractional_part);
                
                BigInt one;
                big_init(&one);
                big_from_int(&one, 1);
                big_sub(result->integer_part, &one, &temp);
                big_copy(result->integer_part, &temp);
                
                big_free(&scale);
                big_free(&temp);
                big_free(&one);
            }
            
            result->negative = b_scaled->negative;
        }
        
        big_fixed_normalize(result);
    }
    
    big_fixed_free(a_scaled);
    big_fixed_free(b_scaled);
}

/* ============================================================================
 * SUBTRACTION
 * ============================================================================ */

void big_fixed_sub(BigFixed *result, const BigFixed *a, const BigFixed *b) {
    if (!result || !a || !b) return;
    
    // Negate b and add
    BigFixed *neg_b = big_fixed_copy(b);
    neg_b->negative = !b->negative;
    
    big_fixed_add(result, a, neg_b);
    big_fixed_free(neg_b);
}

/* ============================================================================
 * MULTIPLICATION
 * ============================================================================ */

void big_fixed_mul(BigFixed *result, const BigFixed *a, const BigFixed *b) {
    if (!result || !a || !b) return;
    
    // (a_int + a_frac/2^s) * (b_int + b_frac/2^s)
    // = a_int*b_int + (a_int*b_frac + a_frac*b_int)/2^s + a_frac*b_frac/2^(2s)
    
    int target_scale = (a->scale_bits > b->scale_bits) ? a->scale_bits : b->scale_bits;
    result->scale_bits = target_scale;
    
    BigInt term1, term2, term3_int, term3_frac;
    BigInt cross1, cross2, cross_sum;
    BigInt term3_full, frac_mask, one;
    
    big_init(&term1);
    big_init(&term2);
    big_init(&term3_int);
    big_init(&term3_frac);
    big_init(&cross1);
    big_init(&cross2);
    big_init(&cross_sum);
    big_init(&term3_full);
    big_init(&frac_mask);
    big_init(&one);
    
    // Term 1: a_int * b_int
    big_mul(a->integer_part, b->integer_part, &term1);
    
    // Term 2: (a_int * b_frac + a_frac * b_int) / 2^s
    big_mul(a->integer_part, b->fractional_part, &cross1);
    big_mul(a->fractional_part, b->integer_part, &cross2);
    big_add(&cross1, &cross2, &cross_sum);
    big_copy(&term2, &cross_sum);
    big_shr(&term2, target_scale);
    
    // Term 3: a_frac * b_frac / 2^(2s)
    big_mul(a->fractional_part, b->fractional_part, &term3_full);
    
    // Split term3 into integer and fractional contributions
    big_copy(&term3_int, &term3_full);
    big_shr(&term3_int, 2 * target_scale);
    
    big_copy(&term3_frac, &term3_full);
    big_shr(&term3_frac, target_scale);
    
    // Mask to keep only fractional bits
    big_from_int(&frac_mask, 1);
    big_shl(&frac_mask, target_scale);
    big_from_int(&one, 1);
    BigInt temp;
    big_init(&temp);
    big_sub(&frac_mask, &one, &temp);
    big_copy(&frac_mask, &temp);
    big_free(&temp);
    
    // Apply mask (note: we don't have big_and, so we'll use modulo)
    big_mod(&term3_frac, &frac_mask, &term3_frac);
    
    // Combine integer parts
    big_init(&temp);
    big_add(&term1, &term2, &temp);
    big_add(&temp, &term3_int, result->integer_part);
    big_free(&temp);
    
    // Combine fractional parts
    BigInt frac_sum;
    big_init(&frac_sum);
    big_copy(&frac_sum, &cross_sum);
    big_mod(&frac_sum, &frac_mask, &frac_sum);
    big_add(&frac_sum, &term3_frac, result->fractional_part);
    big_free(&frac_sum);
    
    // Handle sign
    result->negative = (a->negative != b->negative);
    
    // Normalize
    big_fixed_normalize(result);
    
    // Cleanup
    big_free(&term1);
    big_free(&term2);
    big_free(&term3_int);
    big_free(&term3_frac);
    big_free(&cross1);
    big_free(&cross2);
    big_free(&cross_sum);
    big_free(&term3_full);
    big_free(&frac_mask);
    big_free(&one);
}

/* ============================================================================
 * DIVISION
 * ============================================================================ */

void big_fixed_div(BigFixed *result, const BigFixed *a, const BigFixed *b) {
    if (!result || !a || !b) return;
    
    // Check for division by zero
    if (big_fixed_is_zero(b)) {
        // Set result to zero (or could set error flag)
        big_from_int(result->integer_part, 0);
        big_from_int(result->fractional_part, 0);
        result->negative = false;
        return;
    }
    
    int target_scale = (a->scale_bits > b->scale_bits) ? a->scale_bits : b->scale_bits;
    result->scale_bits = target_scale;
    
    // Convert to single BigInt: value = int_part * 2^scale + frac_part
    BigInt a_full, b_full, quotient, remainder, scale_val;
    big_init(&a_full);
    big_init(&b_full);
    big_init(&quotient);
    big_init(&remainder);
    big_init(&scale_val);
    
    big_copy(&a_full, a->integer_part);
    big_shl(&a_full, target_scale);
    BigInt temp;
    big_init(&temp);
    big_add(&a_full, a->fractional_part, &temp);
    big_copy(&a_full, &temp);
    big_free(&temp);
    
    big_copy(&b_full, b->integer_part);
    big_shl(&b_full, target_scale);
    big_init(&temp);
    big_add(&b_full, b->fractional_part, &temp);
    big_copy(&b_full, &temp);
    big_free(&temp);
    
    // Scale numerator for precision: a_full * 2^scale
    big_shl(&a_full, target_scale);
    
    // Divide: quotient = (a_full * 2^scale) / b_full
    big_div(&a_full, &b_full, &quotient, &remainder);
    
    // Split quotient into integer and fractional parts
    big_copy(result->integer_part, &quotient);
    big_shr(result->integer_part, target_scale);
    
    big_from_int(&scale_val, 1);
    big_shl(&scale_val, target_scale);
    BigInt one, frac_mask;
    big_init(&one);
    big_init(&frac_mask);
    big_from_int(&one, 1);
    big_sub(&scale_val, &one, &frac_mask);
    big_mod(&quotient, &frac_mask, result->fractional_part);
    
    // Handle sign
    result->negative = (a->negative != b->negative);
    
    // Normalize
    big_fixed_normalize(result);
    
    // Cleanup
    big_free(&a_full);
    big_free(&b_full);
    big_free(&quotient);
    big_free(&remainder);
    big_free(&scale_val);
    big_free(&one);
    big_free(&frac_mask);
}

/* ============================================================================
 * ABSOLUTE VALUE AND NEGATION
 * ============================================================================ */

void big_fixed_abs(BigFixed *result, const BigFixed *f) {
    if (!result || !f) return;
    
    big_copy(result->integer_part, f->integer_part);
    big_copy(result->fractional_part, f->fractional_part);
    result->scale_bits = f->scale_bits;
    result->negative = false;
}

void big_fixed_neg(BigFixed *result, const BigFixed *f) {
    if (!result || !f) return;
    
    big_copy(result->integer_part, f->integer_part);
    big_copy(result->fractional_part, f->fractional_part);
    result->scale_bits = f->scale_bits;
    result->negative = !f->negative;
    
    // Handle zero case
    if (big_fixed_is_zero(result)) {
        result->negative = false;
    }
}

/* ============================================================================
 * SHIFT OPERATIONS
 * ============================================================================ */

void big_fixed_lshift(BigFixed *result, const BigFixed *f, int bits) {
    if (!result || !f || bits < 0) return;
    
    // Left shift: multiply by 2^bits
    big_copy(result->fractional_part, f->fractional_part);
    result->scale_bits = f->scale_bits;
    result->negative = f->negative;
    
    // Shift integer part
    big_copy(result->integer_part, f->integer_part);
    big_shl(result->integer_part, bits);
    
    // Handle fractional overflow
    big_fixed_normalize(result);
}

void big_fixed_rshift(BigFixed *result, const BigFixed *f, int bits) {
    if (!result || !f || bits < 0) return;
    
    // Right shift: divide by 2^bits
    result->scale_bits = f->scale_bits;
    result->negative = f->negative;
    
    // Combine into single value, shift, then split
    BigInt combined, frac_mask, one;
    big_init(&combined);
    big_init(&frac_mask);
    big_init(&one);
    
    big_copy(&combined, f->integer_part);
    big_shl(&combined, f->scale_bits);
    BigInt temp;
    big_init(&temp);
    big_add(&combined, f->fractional_part, &temp);
    big_copy(&combined, &temp);
    big_free(&temp);
    
    big_shr(&combined, bits);
    
    // Split back
    big_copy(result->integer_part, &combined);
    big_shr(result->integer_part, f->scale_bits);
    
    big_from_int(&frac_mask, 1);
    big_shl(&frac_mask, f->scale_bits);
    big_from_int(&one, 1);
    big_init(&temp);
    big_sub(&frac_mask, &one, &temp);
    big_copy(&frac_mask, &temp);
    big_free(&temp);
    
    big_mod(&combined, &frac_mask, result->fractional_part);
    
    big_free(&combined);
    big_free(&frac_mask);
    big_free(&one);
}

/* ============================================================================
 * ROUNDING FUNCTIONS
 * ============================================================================ */

void big_fixed_floor(BigInt *result, const BigFixed *f) {
    if (!result || !f) return;
    
    big_copy(result, f->integer_part);
    
    // If negative and has fractional part, subtract 1
    if (f->negative && !big_is_zero(f->fractional_part)) {
        BigInt one, temp;
        big_init(&one);
        big_init(&temp);
        big_from_int(&one, 1);
        big_sub(result, &one, &temp);
        big_copy(result, &temp);
        big_free(&one);
        big_free(&temp);
    }
    
    result->negative = f->negative;
}

void big_fixed_ceil(BigInt *result, const BigFixed *f) {
    if (!result || !f) return;
    
    big_copy(result, f->integer_part);
    
    // If positive and has fractional part, add 1
    if (!f->negative && !big_is_zero(f->fractional_part)) {
        BigInt one, temp;
        big_init(&one);
        big_init(&temp);
        big_from_int(&one, 1);
        big_add(result, &one, &temp);
        big_copy(result, &temp);
        big_free(&one);
        big_free(&temp);
    }
    
    result->negative = f->negative;
}

void big_fixed_round(BigInt *result, const BigFixed *f) {
    if (!result || !f) return;
    
    big_fixed_to_bigint_rounded(result, f);
}

void big_fixed_trunc(BigInt *result, const BigFixed *f) {
    if (!result || !f) return;
    
    big_fixed_to_bigint(result, f);
}

/* ============================================================================
 * FRACTIONAL PART
 * ============================================================================ */

void big_fixed_frac(BigFixed *result, const BigFixed *f) {
    if (!result || !f) return;
    
    big_from_int(result->integer_part, 0);
    big_copy(result->fractional_part, f->fractional_part);
    result->scale_bits = f->scale_bits;
    result->negative = f->negative;
}

/* ============================================================================
 * STRING CONVERSION (for debugging)
 * ============================================================================ */

char* big_fixed_to_string(const BigFixed *f, int decimal_places) {
    if (!f) return NULL;
    
    // Allocate buffer (generous size)
    char *buffer = (char*)malloc(4096);
    if (!buffer) return NULL;
    
    // Convert integer part
    char *int_str = big_to_string(f->integer_part);
    if (!int_str) {
        free(buffer);
        return NULL;
    }
    
    // Start with sign and integer part
    int pos = 0;
    if (f->negative && !big_fixed_is_zero(f)) {
        buffer[pos++] = '-';
    }
    strcpy(buffer + pos, int_str);
    pos += strlen(int_str);
    free(int_str);
    
    // Add decimal point
    buffer[pos++] = '.';
    
    // Convert fractional part to decimal
    // frac_decimal = frac * 10^decimal_places / 2^scale_bits
    BigInt frac_decimal, ten, temp;
    big_init(&frac_decimal);
    big_init(&ten);
    big_init(&temp);
    
    big_copy(&frac_decimal, f->fractional_part);
    big_from_int(&ten, 10);
    
    for (int i = 0; i < decimal_places; i++) {
        // Multiply by 10
        big_mul(&frac_decimal, &ten, &temp);
        big_copy(&frac_decimal, &temp);
    }
    
    // Divide by 2^scale_bits
    big_shr(&frac_decimal, f->scale_bits);
    
    // Convert to string and pad with zeros
    char *frac_str = big_to_string(&frac_decimal);
    int frac_len = strlen(frac_str);
    
    // Pad with leading zeros if necessary
    for (int i = frac_len; i < decimal_places; i++) {
        buffer[pos++] = '0';
    }
    
    // Copy fractional digits
    strcpy(buffer + pos, frac_str);
    
    free(frac_str);
    big_free(&frac_decimal);
    big_free(&ten);
    big_free(&temp);
    
    return buffer;
}
// Convert BigInt to uint64_t helper
static uint64_t bigint_to_uint64_helper(const BigInt* n) {
    if (!n || !n->d || n->len == 0) return 0;
    
    uint64_t result = 0;
    size_t max_digits = (n->len < 2) ? n->len : 2;
    
    for (size_t i = 0; i < max_digits; i++) {
        result |= ((uint64_t)n->d[i]) << (i * 32);
    }
    
    return result;
}

// Convert BigFixed to double
double big_fixed_to_double(const BigFixed* value) {
    if (!value) return 0.0;
    
    double result = (double)bigint_to_uint64_helper(value->integer_part);
    
    uint64_t frac = bigint_to_uint64_helper(value->fractional_part);
    result += (double)frac / 18446744073709551616.0;
    
    if (value->negative) result = -result;
    
    return result;
}

// Convert double to BigFixed
void big_fixed_from_double(BigFixed* result, double value) {
    if (!result) return;
    
    result->negative = (value < 0);
    if (value < 0) value = -value;
    
    int64_t int_part = (int64_t)value;
    double frac_part = value - (double)int_part;
    
    big_from_int(result->integer_part, (uint64_t)int_part);
    
    uint64_t frac_fixed = (uint64_t)(frac_part * 18446744073709551616.0);
    big_from_int(result->fractional_part, frac_fixed);
}
