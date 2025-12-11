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
static MathError multiply_by_digit(CrystallineAbacus* result, const CrystallineAbacus* a, uint32_t digit);

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
 * @brief Find quotient digit for division (trial division)
 * @param dividend Current dividend
 * @param divisor Divisor
 * @return Quotient digit (0 to base-1)
 * 
 * Finds the largest digit q such that divisor * q <= dividend
 */
static uint32_t find_quotient_digit(const CrystallineAbacus* dividend,
                                    const CrystallineAbacus* divisor) {
    /* Binary search for quotient digit */
    uint32_t low = 0;
    uint32_t high = dividend->base - 1;
    uint32_t result = 0;
    
    CrystallineAbacus* temp = abacus_new(dividend->base);
    if (!temp) return 0;
    
    while (low <= high) {
        uint32_t mid = (low + high) / 2;
        
        /* Calculate divisor * mid */
        multiply_by_digit(temp, divisor, mid);
        
        /* Compare with dividend */
        int cmp = compare_magnitude(temp, dividend);
        
        if (cmp <= 0) {
            /* divisor * mid <= dividend, try larger */
            result = mid;
            low = mid + 1;
        } else {
            /* divisor * mid > dividend, try smaller */
            if (mid == 0) break;
            high = mid - 1;
        }
    }
    
    abacus_free(temp);
    return result;
}

/**
 * @brief Multiply abacus by a single digit (PURE GEOMETRIC)
 * @param result Output abacus (must be pre-allocated)
 * @param a Input abacus
 * @param digit Single digit to multiply by
 * @return MATH_SUCCESS or error code
 */
static MathError multiply_by_digit(CrystallineAbacus* result,
                                   const CrystallineAbacus* a,
                                   uint32_t digit) {
    if (digit == 0) {
        /* Result is zero */
        result->num_beads = 1;
        result->beads[0].value = 0;
        result->beads[0].weight = 1;
        result->negative = false;
        if (map_digit_to_position(0, result->base, &result->beads[0].position) != MATH_SUCCESS) {
            return MATH_ERROR_INVALID_ARG;
        }
        return MATH_SUCCESS;
    }
    
    if (digit == 1) {
        /* Result is a copy of a */
        if (abacus_ensure_capacity(result, a->num_beads) != MATH_SUCCESS) {
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        memcpy(result->beads, a->beads, a->num_beads * sizeof(AbacusBead));
        result->num_beads = a->num_beads;
        result->negative = a->negative;
        return MATH_SUCCESS;
    }
    
    /* Ensure capacity for result (may need one extra bead for carry) */
    if (abacus_ensure_capacity(result, a->num_beads + 1) != MATH_SUCCESS) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    uint32_t carry = 0;
    size_t i = 0;
    
    for (i = 0; i < a->num_beads; i++) {
        uint64_t product = (uint64_t)a->beads[i].value * digit + carry;
        uint32_t digit_value = product % result->base;
        carry = product / result->base;
        
        result->beads[i].value = digit_value;
        result->beads[i].weight = (i == 0) ? 1 : result->beads[i-1].weight * result->base;
        
        if (map_digit_to_position(digit_value, result->base, &result->beads[i].position) != MATH_SUCCESS) {
            return MATH_ERROR_INVALID_ARG;
        }
    }
    
    /* Handle final carry */
    if (carry > 0) {
        result->beads[i].value = carry;
        result->beads[i].weight = result->beads[i-1].weight * result->base;
        if (map_digit_to_position(carry, result->base, &result->beads[i].position) != MATH_SUCCESS) {
            return MATH_ERROR_INVALID_ARG;
        }
        result->num_beads = i + 1;
    } else {
        result->num_beads = i;
    }
    
    result->negative = a->negative;
    
    return MATH_SUCCESS;
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
    
    /* Bases must match */
    if (a->base != b->base || result->base != a->base) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* PURE GEOMETRIC MULTIPLICATION - School Algorithm */
    /* Algorithm: For each digit in b, multiply a by that digit and add to result */
    
    /* Handle zero cases */
    if (abacus_is_zero(a) || abacus_is_zero(b)) {
        result->num_beads = 1;
        result->beads[0].value = 0;
        result->beads[0].weight = 1;
        result->negative = false;
        if (map_digit_to_position(0, result->base, &result->beads[0].position) != MATH_SUCCESS) {
            return MATH_ERROR_INVALID_ARG;
        }
        return MATH_SUCCESS;
    }
    
    /* Initialize result to zero */
    if (abacus_init_zero(result) != MATH_SUCCESS) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* For each digit in b (multiplier) */
    for (size_t i = 0; i < b->num_beads; i++) {
        /* Skip if digit is zero (optimization) */
        if (b->beads[i].value == 0) {
            continue;
        }
        
        /* Multiply a by b[i] */
        CrystallineAbacus* temp = abacus_new(result->base);
        if (!temp) {
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        MathError err = multiply_by_digit(temp, a, b->beads[i].value);
        if (err != MATH_SUCCESS) {
            abacus_free(temp);
            return err;
        }
        
        /* Shift temp left by i positions (multiply by base^i) */
        if (i > 0) {
            CrystallineAbacus* shifted = abacus_new(result->base);
            if (!shifted) {
                abacus_free(temp);
                return MATH_ERROR_OUT_OF_MEMORY;
            }
            
            err = abacus_shift_left(shifted, temp, i);
            if (err != MATH_SUCCESS) {
                abacus_free(temp);
                abacus_free(shifted);
                return err;
            }
            
            abacus_free(temp);
            temp = shifted;
        }
        
        /* Add to result (accumulate) */
        CrystallineAbacus* new_result = abacus_new(result->base);
        if (!new_result) {
            abacus_free(temp);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        err = abacus_add(new_result, result, temp);
        if (err != MATH_SUCCESS) {
            abacus_free(temp);
            abacus_free(new_result);
            return err;
        }
        
        /* Copy new_result to result */
        if (abacus_ensure_capacity(result, new_result->num_beads) != MATH_SUCCESS) {
            abacus_free(temp);
            abacus_free(new_result);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        memcpy(result->beads, new_result->beads, new_result->num_beads * sizeof(AbacusBead));
        result->num_beads = new_result->num_beads;
        result->negative = new_result->negative;
        
        abacus_free(temp);
        abacus_free(new_result);
    }
    
    /* Handle sign: negative if signs differ */
    result->negative = (a->negative != b->negative);
    
    return abacus_normalize(result);
}

MathError abacus_div(CrystallineAbacus* quotient, CrystallineAbacus* remainder,
                     const CrystallineAbacus* a, const CrystallineAbacus* b) {
    if (!quotient || !a || !b) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Bases must match */
    if (a->base != b->base || quotient->base != a->base) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    if (remainder && remainder->base != a->base) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Check for division by zero */
    if (abacus_is_zero(b)) {
        return MATH_ERROR_DIVISION_BY_ZERO;
    }
    
    /* PURE GEOMETRIC DIVISION - Long Division Algorithm */
    
    /* Handle zero dividend */
    if (abacus_is_zero(a)) {
        abacus_init_zero(quotient);
        if (remainder) {
            abacus_init_zero(remainder);
        }
        return MATH_SUCCESS;
    }
    
    /* Compare magnitudes */
    int cmp = compare_magnitude(a, b);
    
    if (cmp < 0) {
        /* |a| < |b|: quotient = 0, remainder = a */
        abacus_init_zero(quotient);
        if (remainder) {
            CrystallineAbacus* temp = abacus_copy(a);
            if (!temp) return MATH_ERROR_OUT_OF_MEMORY;
            
            if (abacus_ensure_capacity(remainder, temp->num_beads) == MATH_SUCCESS) {
                memcpy(remainder->beads, temp->beads, temp->num_beads * sizeof(AbacusBead));
                remainder->num_beads = temp->num_beads;
                remainder->negative = temp->negative;
            }
            abacus_free(temp);
        }
        return MATH_SUCCESS;
    }
    
    if (cmp == 0) {
        /* |a| == |b|: quotient = 1, remainder = 0 */
        quotient->num_beads = 1;
        quotient->beads[0].value = 1;
        quotient->beads[0].weight = 1;
        quotient->negative = (a->negative != b->negative);
        if (map_digit_to_position(1, quotient->base, &quotient->beads[0].position) != MATH_SUCCESS) {
            return MATH_ERROR_INVALID_ARG;
        }
        
        if (remainder) {
            abacus_init_zero(remainder);
        }
        return MATH_SUCCESS;
    }
    
    /* Long division: |a| > |b| */
    /* Work with magnitudes, handle sign at the end */
    
    CrystallineAbacus* current = abacus_new(a->base);
    CrystallineAbacus* temp_quotient = abacus_new(a->base);
    
    if (!current || !temp_quotient) {
        abacus_free(current);
        abacus_free(temp_quotient);
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Initialize quotient to zero */
    abacus_init_zero(temp_quotient);
    
    /* Initialize current to zero */
    abacus_init_zero(current);
    
    /* Process each digit of dividend from most significant to least */
    for (int i = (int)a->num_beads - 1; i >= 0; i--) {
        /* Shift current left by 1 position (multiply by base) */
        CrystallineAbacus* shifted = abacus_new(a->base);
        if (!shifted) {
            abacus_free(current);
            abacus_free(temp_quotient);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        MathError err = abacus_shift_left(shifted, current, 1);
        if (err != MATH_SUCCESS) {
            abacus_free(current);
            abacus_free(temp_quotient);
            abacus_free(shifted);
            return err;
        }
        
        /* Add next digit from dividend */
        shifted->beads[0].value = a->beads[i].value;
        if (map_digit_to_position(a->beads[i].value, shifted->base, &shifted->beads[0].position) != MATH_SUCCESS) {
            abacus_free(current);
            abacus_free(temp_quotient);
            abacus_free(shifted);
            return MATH_ERROR_INVALID_ARG;
        }
        
        /* Normalize shifted */
        abacus_normalize(shifted);
        
        /* Copy shifted to current */
        if (abacus_ensure_capacity(current, shifted->num_beads) != MATH_SUCCESS) {
            abacus_free(current);
            abacus_free(temp_quotient);
            abacus_free(shifted);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        memcpy(current->beads, shifted->beads, shifted->num_beads * sizeof(AbacusBead));
        current->num_beads = shifted->num_beads;
        current->negative = false;  /* Work with magnitudes */
        
        abacus_free(shifted);
        
        /* Find quotient digit */
        uint32_t q_digit = find_quotient_digit(current, b);
        
        /* Shift quotient left and add new digit */
        CrystallineAbacus* q_shifted = abacus_new(a->base);
        if (!q_shifted) {
            abacus_free(current);
            abacus_free(temp_quotient);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        err = abacus_shift_left(q_shifted, temp_quotient, 1);
        if (err != MATH_SUCCESS) {
            abacus_free(current);
            abacus_free(temp_quotient);
            abacus_free(q_shifted);
            return err;
        }
        
        q_shifted->beads[0].value = q_digit;
        if (map_digit_to_position(q_digit, q_shifted->base, &q_shifted->beads[0].position) != MATH_SUCCESS) {
            abacus_free(current);
            abacus_free(temp_quotient);
            abacus_free(q_shifted);
            return MATH_ERROR_INVALID_ARG;
        }
        
        abacus_normalize(q_shifted);
        
        /* Copy to temp_quotient */
        if (abacus_ensure_capacity(temp_quotient, q_shifted->num_beads) != MATH_SUCCESS) {
            abacus_free(current);
            abacus_free(temp_quotient);
            abacus_free(q_shifted);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        memcpy(temp_quotient->beads, q_shifted->beads, q_shifted->num_beads * sizeof(AbacusBead));
        temp_quotient->num_beads = q_shifted->num_beads;
        
        abacus_free(q_shifted);
        
        /* Subtract b * q_digit from current */
        CrystallineAbacus* product = abacus_new(a->base);
        if (!product) {
            abacus_free(current);
            abacus_free(temp_quotient);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        err = multiply_by_digit(product, b, q_digit);
        if (err != MATH_SUCCESS) {
            abacus_free(current);
            abacus_free(temp_quotient);
            abacus_free(product);
            return err;
        }
        
        CrystallineAbacus* new_current = abacus_new(a->base);
        if (!new_current) {
            abacus_free(current);
            abacus_free(temp_quotient);
            abacus_free(product);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        err = abacus_sub(new_current, current, product);
        if (err != MATH_SUCCESS) {
            abacus_free(current);
            abacus_free(temp_quotient);
            abacus_free(product);
            abacus_free(new_current);
            return err;
        }
        
        abacus_free(product);
        
        /* Copy new_current to current */
        if (abacus_ensure_capacity(current, new_current->num_beads) != MATH_SUCCESS) {
            abacus_free(current);
            abacus_free(temp_quotient);
            abacus_free(new_current);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        memcpy(current->beads, new_current->beads, new_current->num_beads * sizeof(AbacusBead));
        current->num_beads = new_current->num_beads;
        current->negative = false;  /* Keep magnitude */
        
        abacus_free(new_current);
    }
    
    /* Copy quotient */
    if (abacus_ensure_capacity(quotient, temp_quotient->num_beads) != MATH_SUCCESS) {
        abacus_free(current);
        abacus_free(temp_quotient);
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    memcpy(quotient->beads, temp_quotient->beads, temp_quotient->num_beads * sizeof(AbacusBead));
    quotient->num_beads = temp_quotient->num_beads;
    quotient->negative = (a->negative != b->negative);
    
    /* Copy remainder */
    if (remainder) {
        if (abacus_ensure_capacity(remainder, current->num_beads) != MATH_SUCCESS) {
            abacus_free(current);
            abacus_free(temp_quotient);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        memcpy(remainder->beads, current->beads, current->num_beads * sizeof(AbacusBead));
        remainder->num_beads = current->num_beads;
        remainder->negative = a->negative;  /* Remainder has same sign as dividend */
    }
    
    abacus_free(current);
    abacus_free(temp_quotient);
    
    abacus_normalize(quotient);
    if (remainder) {
        abacus_normalize(remainder);
    }
    
    return MATH_SUCCESS;
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