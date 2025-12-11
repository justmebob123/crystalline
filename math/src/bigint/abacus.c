/**
 * @file abacus.c
 * @brief Crystalline Abacus implementation
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * Implements geometric arithmetic operations using the clock lattice structure.
 */

#include "math/abacus.h"
#include "math/prime.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * INTERNAL HELPERS
 * ============================================================================
 */

/* Forward declarations */
static MathError map_digit_to_position(uint32_t digit, uint32_t base, ClockPosition* pos);

/**
 * @brief Ensure abacus has enough capacity
 */
static MathError abacus_ensure_capacity(CrystallineAbacus* abacus, size_t min_capacity) {
    if (abacus->capacity >= min_capacity) {
        return MATH_SUCCESS;
    }
    
    size_t new_capacity = abacus->capacity * 2;
    if (new_capacity < min_capacity) {
        new_capacity = min_capacity;
    }
    
    AbacusBead* new_beads = (AbacusBead*)realloc(
        abacus->beads,
        new_capacity * sizeof(AbacusBead)
    );
    
    if (!new_beads) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    abacus->beads = new_beads;
    abacus->capacity = new_capacity;
    
    return MATH_SUCCESS;
}

/**
 * @brief Map a digit value to a clock position
 */
/**
 * @brief Compare magnitudes of two abacuses (ignore signs)
 * @return -1 if |a| < |b|, 0 if |a| == |b|, 1 if |a| > |b|
 */
static int compare_magnitude(const CrystallineAbacus* a, const CrystallineAbacus* b) {
    /* Compare number of beads first */
    if (a->num_beads > b->num_beads) return 1;
    if (a->num_beads < b->num_beads) return -1;
    
    /* Same number of beads, compare from most significant */
    for (size_t i = a->num_beads; i > 0; i--) {
        uint32_t digit_a = a->beads[i-1].value;
        uint32_t digit_b = b->beads[i-1].value;
        
        if (digit_a > digit_b) return 1;
        if (digit_a < digit_b) return -1;
    }
    
    return 0; /* Equal */
}

/**
 * @brief Subtract magnitudes (assumes |a| >= |b|)
 * @param result Output abacus
 * @param a Larger magnitude
 * @param b Smaller magnitude
 * @return MATH_SUCCESS or error code
 */
static MathError subtract_magnitude(CrystallineAbacus* result, 
                                    const CrystallineAbacus* a, 
                                    const CrystallineAbacus* b) {
    if (abacus_ensure_capacity(result, a->num_beads) != MATH_SUCCESS) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    int32_t borrow = 0;
    size_t i = 0;
    
    while (i < a->num_beads) {
        int32_t digit_a = (i < a->num_beads) ? a->beads[i].value : 0;
        int32_t digit_b = (i < b->num_beads) ? b->beads[i].value : 0;
        
        int32_t diff = digit_a - digit_b - borrow;
        
        if (diff < 0) {
            diff += result->base;
            borrow = 1;
        } else {
            borrow = 0;
        }
        
        result->beads[i].value = (uint32_t)diff;
        result->beads[i].weight = (i == 0) ? 1 : result->beads[i-1].weight * result->base;
        
        if (map_digit_to_position((uint32_t)diff, result->base, &result->beads[i].position) != MATH_SUCCESS) {
            return MATH_ERROR_INVALID_ARG;
        }
        
        i++;
    }
    
    result->num_beads = i;
    
    return MATH_SUCCESS;
}

static MathError map_digit_to_position(uint32_t digit, uint32_t base, ClockPosition* pos) {
    if (!pos) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Map digit to appropriate ring based on base */
    if (base == 12) {
        pos->ring = 0;  /* Use Ring 0 (12 positions) */
        pos->position = digit % 12;
    } else if (base == 60) {
        pos->ring = 1;  /* Use Ring 1 (60 positions) */
        pos->position = digit % 60;
    } else if (base == 100) {
        pos->ring = 3;  /* Use Ring 3 (100 positions) */
        pos->position = digit % 100;
    } else {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Calculate geometric properties */
    uint32_t ring_size = (pos->ring == 0) ? 12 :
                         (pos->ring == 1) ? 60 :
                         (pos->ring == 2) ? 60 : 100;
    
    pos->angle = (2.0 * MATH_PI * pos->position) / ring_size;
    pos->radius = 1.0 - (pos->ring * 0.25);
    
    return MATH_SUCCESS;
}

/* ============================================================================
 * INITIALIZATION AND CLEANUP
 * ============================================================================
 */

CrystallineAbacus* abacus_new(uint32_t base) {
    /* Validate base */
    if (base != 12 && base != 60 && base != 100) {
        return NULL;
    }
    
    CrystallineAbacus* abacus = (CrystallineAbacus*)malloc(sizeof(CrystallineAbacus));
    if (!abacus) {
        return NULL;
    }
    
    abacus->beads = (AbacusBead*)calloc(8, sizeof(AbacusBead));
    if (!abacus->beads) {
        free(abacus);
        return NULL;
    }
    
    abacus->num_beads = 0;
    abacus->capacity = 8;
    abacus->base = base;
    abacus->negative = false;
    
    return abacus;
}

void abacus_free(CrystallineAbacus* abacus) {
    if (abacus) {
        if (abacus->beads) {
            free(abacus->beads);
        }
        free(abacus);
    }
}

MathError abacus_init_zero(CrystallineAbacus* abacus) {
    if (!abacus) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    abacus->num_beads = 1;
    abacus->beads[0].value = 0;
    abacus->beads[0].weight = 1;
    abacus->negative = false;
    
    return map_digit_to_position(0, abacus->base, &abacus->beads[0].position);
}

/* ============================================================================
 * CONVERSION
 * ============================================================================
 */

CrystallineAbacus* abacus_from_uint64(uint64_t value, uint32_t base) {
    CrystallineAbacus* abacus = abacus_new(base);
    if (!abacus) {
        return NULL;
    }
    
    if (value == 0) {
        abacus_init_zero(abacus);
        return abacus;
    }
    
    /* Convert value to base representation */
    uint64_t temp = value;
    size_t num_digits = 0;
    
    /* Count digits */
    while (temp > 0) {
        num_digits++;
        temp /= base;
    }
    
    /* Ensure capacity */
    if (abacus_ensure_capacity(abacus, num_digits) != MATH_SUCCESS) {
        abacus_free(abacus);
        return NULL;
    }
    
    /* Extract digits (least significant first) */
    temp = value;
    uint32_t weight = 1;
    
    for (size_t i = 0; i < num_digits; i++) {
        uint32_t digit = temp % base;
        
        abacus->beads[i].value = digit;
        abacus->beads[i].weight = weight;
        
        if (map_digit_to_position(digit, base, &abacus->beads[i].position) != MATH_SUCCESS) {
            abacus_free(abacus);
            return NULL;
        }
        
        temp /= base;
        weight *= base;
    }
    
    abacus->num_beads = num_digits;
    abacus->negative = false;
    
    return abacus;
}

MathError abacus_to_uint64(const CrystallineAbacus* abacus, uint64_t* value) {
    if (!abacus || !value) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    *value = 0;
    uint64_t multiplier = 1;
    
    for (size_t i = 0; i < abacus->num_beads; i++) {
        *value += abacus->beads[i].value * multiplier;
        multiplier *= abacus->base;
        
        /* Check for overflow */
        if (multiplier == 0 && i < abacus->num_beads - 1) {
            return MATH_ERROR_OVERFLOW;
        }
    }
    
    return MATH_SUCCESS;
}

CrystallineAbacus* abacus_from_bigint(const BigInt* bi, uint32_t base) {
    if (!bi) {
        return NULL;
    }
    
    /* For now, convert through int64 */
    /* TODO: Implement direct BigInt conversion for large numbers */
    
    int64_t value = bigint_to_int64(bi);
    
    CrystallineAbacus* abacus = abacus_from_uint64(
        (value < 0) ? -value : value,
        base
    );
    
    if (abacus) {
        abacus->negative = (value < 0);
    }
    
    return abacus;
}

BigInt* abacus_to_bigint(const CrystallineAbacus* abacus) {
    if (!abacus) {
        return NULL;
    }
    
    /* For now, convert through uint64 */
    /* TODO: Implement direct BigInt conversion for large numbers */
    
    uint64_t value;
    if (abacus_to_uint64(abacus, &value) != MATH_SUCCESS) {
        return NULL;
    }
    
    BigInt* bi = bigint_from_int64(abacus->negative ? -(int64_t)value : (int64_t)value);
    return bi;
}

/* ============================================================================
 * GEOMETRIC ARITHMETIC OPERATIONS
 * ============================================================================
 */

MathError abacus_add(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Bases must match */
    if (a->base != b->base || result->base != a->base) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Handle signs */
    if (a->negative == b->negative) {
        /* Same sign: add magnitudes */
        size_t max_beads = (a->num_beads > b->num_beads) ? a->num_beads : b->num_beads;
        
        if (abacus_ensure_capacity(result, max_beads + 1) != MATH_SUCCESS) {
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        uint32_t carry = 0;
        size_t i = 0;
        
        while (i < max_beads || carry > 0) {
            uint32_t digit_a = (i < a->num_beads) ? a->beads[i].value : 0;
            uint32_t digit_b = (i < b->num_beads) ? b->beads[i].value : 0;
            
            uint32_t sum = digit_a + digit_b + carry;
            carry = sum / result->base;
            uint32_t digit = sum % result->base;
            
            result->beads[i].value = digit;
            result->beads[i].weight = (i == 0) ? 1 : result->beads[i-1].weight * result->base;
            
            if (map_digit_to_position(digit, result->base, &result->beads[i].position) != MATH_SUCCESS) {
                return MATH_ERROR_INVALID_ARG;
            }
            
            i++;
        }
        
        result->num_beads = i;
        result->negative = a->negative;
        
    } else {
        /* Different signs: subtract magnitudes (PURE GEOMETRIC) */
        int cmp = compare_magnitude(a, b);
        
        if (cmp == 0) {
            /* Equal magnitudes, result is zero */
            result->num_beads = 1;
            result->beads[0].value = 0;
            result->beads[0].weight = 1;
            result->negative = false;
            if (map_digit_to_position(0, result->base, &result->beads[0].position) != MATH_SUCCESS) {
                return MATH_ERROR_INVALID_ARG;
            }
        } else if (cmp > 0) {
            /* |a| > |b|: subtract b from a, keep sign of a */
            MathError err = subtract_magnitude(result, a, b);
            if (err != MATH_SUCCESS) return err;
            result->negative = a->negative;
        } else {
            /* |a| < |b|: subtract a from b, flip sign of a */
            MathError err = subtract_magnitude(result, b, a);
            if (err != MATH_SUCCESS) return err;
            result->negative = !a->negative;
        }
    }
    
    return abacus_normalize(result);
}

MathError abacus_sub(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Bases must match */
    if (a->base != b->base || result->base != a->base) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Subtraction is addition with flipped sign (PURE GEOMETRIC) */
    /* a - b = a + (-b) */
    
    /* Handle signs */
    if (a->negative == b->negative) {
        /* Same sign: subtract magnitudes */
        int cmp = compare_magnitude(a, b);
        
        if (cmp == 0) {
            /* Equal magnitudes, result is zero */
            result->num_beads = 1;
            result->beads[0].value = 0;
            result->beads[0].weight = 1;
            result->negative = false;
            if (map_digit_to_position(0, result->base, &result->beads[0].position) != MATH_SUCCESS) {
                return MATH_ERROR_INVALID_ARG;
            }
        } else if (cmp > 0) {
            /* |a| > |b|: subtract b from a, keep sign of a */
            MathError err = subtract_magnitude(result, a, b);
            if (err != MATH_SUCCESS) return err;
            result->negative = a->negative;
        } else {
            /* |a| < |b|: subtract a from b, flip sign */
            MathError err = subtract_magnitude(result, b, a);
            if (err != MATH_SUCCESS) return err;
            result->negative = !a->negative;
        }
    } else {
        /* Different signs: add magnitudes */
        size_t max_beads = (a->num_beads > b->num_beads) ? a->num_beads : b->num_beads;
        
        if (abacus_ensure_capacity(result, max_beads + 1) != MATH_SUCCESS) {
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        uint32_t carry = 0;
        size_t i = 0;
        
        while (i < max_beads || carry > 0) {
            uint32_t digit_a = (i < a->num_beads) ? a->beads[i].value : 0;
            uint32_t digit_b = (i < b->num_beads) ? b->beads[i].value : 0;
            
            uint32_t sum = digit_a + digit_b + carry;
            carry = sum / result->base;
            uint32_t digit = sum % result->base;
            
            result->beads[i].value = digit;
            result->beads[i].weight = (i == 0) ? 1 : result->beads[i-1].weight * result->base;
            
            if (map_digit_to_position(digit, result->base, &result->beads[i].position) != MATH_SUCCESS) {
                return MATH_ERROR_INVALID_ARG;
            }
            
            i++;
        }
        
        result->num_beads = i;
        result->negative = a->negative;
    }
    
    return abacus_normalize(result);
}

MathError abacus_mul(CrystallineAbacus* result, const CrystallineAbacus* a, const CrystallineAbacus* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* For now, convert to BigInt and back */
    /* TODO: Implement direct geometric multiplication */
    
    BigInt* bi_a = abacus_to_bigint(a);
    BigInt* bi_b = abacus_to_bigint(b);
    BigInt* bi_result = bigint_new();
    
    if (!bi_a || !bi_b || !bi_result) {
        bigint_free(bi_a);
        bigint_free(bi_b);
        bigint_free(bi_result);
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    MathError err = bigint_mul(bi_result, bi_a, bi_b);
    
    if (err == MATH_SUCCESS) {
        CrystallineAbacus* temp = abacus_from_bigint(bi_result, result->base);
        if (temp) {
            if (abacus_ensure_capacity(result, temp->num_beads) == MATH_SUCCESS) {
                memcpy(result->beads, temp->beads, temp->num_beads * sizeof(AbacusBead));
                result->num_beads = temp->num_beads;
                result->negative = temp->negative;
            }
            abacus_free(temp);
        }
    }
    
    bigint_free(bi_a);
    bigint_free(bi_b);
    bigint_free(bi_result);
    
    return err;
}

MathError abacus_div(CrystallineAbacus* quotient, CrystallineAbacus* remainder,
                     const CrystallineAbacus* a, const CrystallineAbacus* b) {
    if (!quotient || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Check for division by zero */
    if (abacus_is_zero(b)) {
        return MATH_ERROR_DIVISION_BY_ZERO;
    }
    
    /* For now, convert to BigInt and back */
    /* TODO: Implement direct geometric division */
    
    BigInt* bi_a = abacus_to_bigint(a);
    BigInt* bi_b = abacus_to_bigint(b);
    BigInt* bi_q = bigint_new();
    BigInt* bi_r = (remainder != NULL) ? bigint_new() : NULL;
    
    if (!bi_a || !bi_b || !bi_q) {
        bigint_free(bi_a);
        bigint_free(bi_b);
        bigint_free(bi_q);
        bigint_free(bi_r);
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    MathError err = bigint_div(bi_q, bi_r, bi_a, bi_b);
    
    if (err == MATH_SUCCESS) {
        CrystallineAbacus* temp_q = abacus_from_bigint(bi_q, quotient->base);
        if (temp_q) {
            if (abacus_ensure_capacity(quotient, temp_q->num_beads) == MATH_SUCCESS) {
                memcpy(quotient->beads, temp_q->beads, temp_q->num_beads * sizeof(AbacusBead));
                quotient->num_beads = temp_q->num_beads;
                quotient->negative = temp_q->negative;
            }
            abacus_free(temp_q);
        }
        
        if (remainder && bi_r) {
            CrystallineAbacus* temp_r = abacus_from_bigint(bi_r, remainder->base);
            if (temp_r) {
                if (abacus_ensure_capacity(remainder, temp_r->num_beads) == MATH_SUCCESS) {
                    memcpy(remainder->beads, temp_r->beads, temp_r->num_beads * sizeof(AbacusBead));
                    remainder->num_beads = temp_r->num_beads;
                    remainder->negative = temp_r->negative;
                }
                abacus_free(temp_r);
            }
        }
    }
    
    bigint_free(bi_a);
    bigint_free(bi_b);
    bigint_free(bi_q);
    bigint_free(bi_r);
    
    return err;
}

MathError abacus_shift_left(CrystallineAbacus* result, const CrystallineAbacus* a, size_t n) {
    if (!result || !a) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    if (n == 0) {
        /* No shift needed, just copy */
        if (result != a) {
            CrystallineAbacus* temp = abacus_copy(a);
            if (!temp) return MATH_ERROR_OUT_OF_MEMORY;
            
            if (abacus_ensure_capacity(result, temp->num_beads) == MATH_SUCCESS) {
                memcpy(result->beads, temp->beads, temp->num_beads * sizeof(AbacusBead));
                result->num_beads = temp->num_beads;
                result->negative = temp->negative;
            }
            abacus_free(temp);
        }
        return MATH_SUCCESS;
    }
    
    /* Shift left by n positions (multiply by base^n) */
    size_t new_size = a->num_beads + n;
    
    if (abacus_ensure_capacity(result, new_size) != MATH_SUCCESS) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Fill lower n positions with zeros */
    for (size_t i = 0; i < n; i++) {
        result->beads[i].value = 0;
        result->beads[i].weight = 1;
        for (size_t j = 0; j < i; j++) {
            result->beads[i].weight *= result->base;
        }
        map_digit_to_position(0, result->base, &result->beads[i].position);
    }
    
    /* Copy shifted digits */
    for (size_t i = 0; i < a->num_beads; i++) {
        result->beads[i + n] = a->beads[i];
        result->beads[i + n].weight = a->beads[i].weight;
        for (size_t j = 0; j < n; j++) {
            result->beads[i + n].weight *= result->base;
        }
    }
    
    result->num_beads = new_size;
    result->negative = a->negative;
    
    return abacus_normalize(result);
}

MathError abacus_shift_right(CrystallineAbacus* result, const CrystallineAbacus* a, size_t n) {
    if (!result || !a) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    if (n >= a->num_beads) {
        /* Shift results in zero */
        return abacus_init_zero(result);
    }
    
    if (n == 0) {
        /* No shift needed, just copy */
        if (result != a) {
            CrystallineAbacus* temp = abacus_copy(a);
            if (!temp) return MATH_ERROR_OUT_OF_MEMORY;
            
            if (abacus_ensure_capacity(result, temp->num_beads) == MATH_SUCCESS) {
                memcpy(result->beads, temp->beads, temp->num_beads * sizeof(AbacusBead));
                result->num_beads = temp->num_beads;
                result->negative = temp->negative;
            }
            abacus_free(temp);
        }
        return MATH_SUCCESS;
    }
    
    /* Shift right by n positions (divide by base^n) */
    size_t new_size = a->num_beads - n;
    
    if (abacus_ensure_capacity(result, new_size) != MATH_SUCCESS) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Copy shifted digits */
    for (size_t i = 0; i < new_size; i++) {
        result->beads[i] = a->beads[i + n];
    }
    
    result->num_beads = new_size;
    result->negative = a->negative;
    
    return abacus_normalize(result);
}

/* ============================================================================
 * COMPARISON
 * ============================================================================
 */

int abacus_compare(const CrystallineAbacus* a, const CrystallineAbacus* b) {
    if (!a || !b) {
        return 0;
    }
    
    /* Handle signs */
    if (a->negative != b->negative) {
        return a->negative ? -1 : 1;
    }
    
    /* Same sign: compare magnitudes */
    if (a->num_beads != b->num_beads) {
        int cmp = (a->num_beads > b->num_beads) ? 1 : -1;
        return a->negative ? -cmp : cmp;
    }
    
    /* Same number of beads: compare digit by digit from most significant */
    for (int i = (int)a->num_beads - 1; i >= 0; i--) {
        if (a->beads[i].value != b->beads[i].value) {
            int cmp = (a->beads[i].value > b->beads[i].value) ? 1 : -1;
            return a->negative ? -cmp : cmp;
        }
    }
    
    return 0;  /* Equal */
}

bool abacus_is_zero(const CrystallineAbacus* abacus) {
    if (!abacus || abacus->num_beads == 0) {
        return true;
    }
    
    for (size_t i = 0; i < abacus->num_beads; i++) {
        if (abacus->beads[i].value != 0) {
            return false;
        }
    }
    
    return true;
}

bool abacus_is_negative(const CrystallineAbacus* abacus) {
    return abacus ? abacus->negative : false;
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================
 */

CrystallineAbacus* abacus_copy(const CrystallineAbacus* src) {
    if (!src) {
        return NULL;
    }
    
    CrystallineAbacus* copy = abacus_new(src->base);
    if (!copy) {
        return NULL;
    }
    
    if (abacus_ensure_capacity(copy, src->num_beads) != MATH_SUCCESS) {
        abacus_free(copy);
        return NULL;
    }
    
    memcpy(copy->beads, src->beads, src->num_beads * sizeof(AbacusBead));
    copy->num_beads = src->num_beads;
    copy->negative = src->negative;
    
    return copy;
}

MathError abacus_normalize(CrystallineAbacus* abacus) {
    if (!abacus) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Remove leading zeros */
    while (abacus->num_beads > 1 && abacus->beads[abacus->num_beads - 1].value == 0) {
        abacus->num_beads--;
    }
    
    /* If all zeros, ensure not negative */
    if (abacus_is_zero(abacus)) {
        abacus->negative = false;
    }
    
    return MATH_SUCCESS;
}

char* abacus_to_string(const CrystallineAbacus* abacus) {
    if (!abacus) {
        return NULL;
    }
    
    /* Estimate size needed */
    size_t size = abacus->num_beads * 4 + 10;  /* Generous estimate */
    char* str = (char*)malloc(size);
    if (!str) {
        return NULL;
    }
    
    char* ptr = str;
    
    if (abacus->negative) {
        *ptr++ = '-';
    }
    
    /* Print digits from most significant to least */
    for (int i = (int)abacus->num_beads - 1; i >= 0; i--) {
        if (i < (int)abacus->num_beads - 1) {
            *ptr++ = ',';
        }
        ptr += sprintf(ptr, "%u", abacus->beads[i].value);
    }
    
    ptr += sprintf(ptr, " (base %u)", abacus->base);
    
    return str;
}

void abacus_print(const CrystallineAbacus* abacus) {
    if (!abacus) {
        printf("NULL\n");
        return;
    }
    
    char* str = abacus_to_string(abacus);
    if (str) {
        printf("%s\n", str);
        free(str);
    }
}