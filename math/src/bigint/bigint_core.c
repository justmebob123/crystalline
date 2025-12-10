/**
 * @file bigint_core.c
 * @brief BigInt core operations implementation
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * Arbitrary precision integer arithmetic using dynamic arrays of 32-bit digits.
 * Little-endian representation: digits[0] is least significant.
 */

#include "math/bigint.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Initial capacity for digit array */
#define BIGINT_INITIAL_CAPACITY 4

/* Growth factor for reallocation */
#define BIGINT_GROWTH_FACTOR 2

/* ============================================================================
 * INTERNAL HELPERS
 * ============================================================================
 */

/**
 * @brief Ensure BigInt has enough capacity
 * @param bi BigInt to resize
 * @param min_capacity Minimum required capacity
 * @return 0 on success, -1 on error
 */
static int bigint_ensure_capacity(BigInt* bi, size_t min_capacity) {
    if (bi->capacity >= min_capacity) {
        return 0;
    }
    
    size_t new_capacity = bi->capacity * BIGINT_GROWTH_FACTOR;
    if (new_capacity < min_capacity) {
        new_capacity = min_capacity;
    }
    
    uint32_t* new_digits = (uint32_t*)realloc(bi->digits, new_capacity * sizeof(uint32_t));
    if (!new_digits) {
        return -1;
    }
    
    bi->digits = new_digits;
    bi->capacity = new_capacity;
    
    return 0;
}

/**
 * @brief Remove leading zeros from BigInt
 * @param bi BigInt to normalize
 */
static void bigint_normalize(BigInt* bi) {
    while (bi->length > 1 && bi->digits[bi->length - 1] == 0) {
        bi->length--;
    }
    
    /* Zero is always positive */
    if (bi->length == 1 && bi->digits[0] == 0) {
        bi->negative = false;
    }
}

/* ============================================================================
 * CREATION AND DESTRUCTION
 * ============================================================================
 */

BigInt* bigint_new(void) {
    BigInt* bi = (BigInt*)malloc(sizeof(BigInt));
    if (!bi) {
        return NULL;
    }
    
    bi->digits = (uint32_t*)calloc(BIGINT_INITIAL_CAPACITY, sizeof(uint32_t));
    if (!bi->digits) {
        free(bi);
        return NULL;
    }
    
    bi->length = 1;
    bi->capacity = BIGINT_INITIAL_CAPACITY;
    bi->negative = false;
    
    return bi;
}

void bigint_free(BigInt* bi) {
    if (bi) {
        if (bi->digits) {
            free(bi->digits);
        }
        free(bi);
    }
}

BigInt* bigint_copy(const BigInt* bi) {
    if (!bi) {
        return NULL;
    }
    
    BigInt* copy = bigint_new();
    if (!copy) {
        return NULL;
    }
    
    if (bigint_ensure_capacity(copy, bi->length) != 0) {
        bigint_free(copy);
        return NULL;
    }
    
    memcpy(copy->digits, bi->digits, bi->length * sizeof(uint32_t));
    copy->length = bi->length;
    copy->negative = bi->negative;
    
    return copy;
}

void bigint_init(BigInt* bi) {
    if (bi) {
        bi->digits = (uint32_t*)calloc(BIGINT_INITIAL_CAPACITY, sizeof(uint32_t));
        bi->length = 1;
        bi->capacity = BIGINT_INITIAL_CAPACITY;
        bi->negative = false;
    }
}

void bigint_clear(BigInt* bi) {
    if (bi && bi->digits) {
        free(bi->digits);
        bi->digits = NULL;
        bi->length = 0;
        bi->capacity = 0;
    }
}

/* ============================================================================
 * CONVERSION FUNCTIONS
 * ============================================================================
 */

BigInt* bigint_from_int64(int64_t value) {
    BigInt* bi = bigint_new();
    if (!bi) {
        return NULL;
    }
    
    if (value < 0) {
        bi->negative = true;
        value = -value;
    }
    
    uint64_t uvalue = (uint64_t)value;
    
    if (uvalue == 0) {
        bi->digits[0] = 0;
        bi->length = 1;
    } else {
        bi->digits[0] = (uint32_t)(uvalue & 0xFFFFFFFF);
        uint32_t high = (uint32_t)(uvalue >> 32);
        
        if (high != 0) {
            if (bigint_ensure_capacity(bi, 2) != 0) {
                bigint_free(bi);
                return NULL;
            }
            bi->digits[1] = high;
            bi->length = 2;
        } else {
            bi->length = 1;
        }
    }
    
    return bi;
}

int64_t bigint_to_int64(const BigInt* bi) {
    if (!bi || bi->length == 0) {
        return 0;
    }
    
    uint64_t value = bi->digits[0];
    if (bi->length > 1) {
        value |= ((uint64_t)bi->digits[1]) << 32;
    }
    
    /* Truncate if too large */
    if (value > (uint64_t)INT64_MAX) {
        value = INT64_MAX;
    }
    
    int64_t result = (int64_t)value;
    return bi->negative ? -result : result;
}

BigInt* bigint_from_uint64(uint64_t value) {
    BigInt* bi = bigint_new();
    if (!bi) {
        return NULL;
    }
    
    if (value == 0) {
        bi->digits[0] = 0;
        bi->length = 1;
    } else {
        bi->digits[0] = (uint32_t)(value & 0xFFFFFFFF);
        uint32_t high = (uint32_t)(value >> 32);
        
        if (high != 0) {
            if (bigint_ensure_capacity(bi, 2) != 0) {
                bigint_free(bi);
                return NULL;
            }
            bi->digits[1] = high;
            bi->length = 2;
        } else {
            bi->length = 1;
        }
    }
    
    return bi;
}

uint64_t bigint_to_uint64(const BigInt* bi) {
    if (!bi || bi->length == 0) {
        return 0;
    }
    
    uint64_t value = bi->digits[0];
    if (bi->length > 1) {
        value |= ((uint64_t)bi->digits[1]) << 32;
    }
    
    return value;
}

BigInt* bigint_from_hex(const char* hex) {
    if (!hex) {
        return NULL;
    }
    
    /* Skip leading "0x" if present */
    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        hex += 2;
    }
    
    size_t len = strlen(hex);
    if (len == 0) {
        return bigint_new();  /* Return zero */
    }
    
    /* Each hex digit is 4 bits, so we need len*4 bits total */
    /* That's (len*4 + 31) / 32 = (len + 7) / 8 digits */
    size_t num_digits = (len + 7) / 8;
    
    BigInt* bi = bigint_new();
    if (!bi) {
        return NULL;
    }
    
    if (bigint_ensure_capacity(bi, num_digits) != 0) {
        bigint_free(bi);
        return NULL;
    }
    
    /* Parse from right to left */
    size_t digit_idx = 0;
    uint32_t current_digit = 0;
    int shift = 0;
    
    for (int i = (int)len - 1; i >= 0; i--) {
        char c = hex[i];
        uint32_t nibble;
        
        if (c >= '0' && c <= '9') {
            nibble = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            nibble = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            nibble = c - 'A' + 10;
        } else {
            /* Invalid character */
            bigint_free(bi);
            return NULL;
        }
        
        current_digit |= (nibble << shift);
        shift += 4;
        
        if (shift == 32) {
            bi->digits[digit_idx++] = current_digit;
            current_digit = 0;
            shift = 0;
        }
    }
    
    if (shift > 0) {
        bi->digits[digit_idx++] = current_digit;
    }
    
    bi->length = digit_idx;
    bigint_normalize(bi);
    
    return bi;
}

char* bigint_to_hex(const BigInt* bi) {
    if (!bi || bi->length == 0) {
        char* result = (char*)malloc(2);
        if (result) {
            result[0] = '0';
            result[1] = '\0';
        }
        return result;
    }
    
    /* Each digit is 8 hex characters, plus sign, plus null terminator */
    size_t max_len = bi->length * 8 + 2;
    char* result = (char*)malloc(max_len);
    if (!result) {
        return NULL;
    }
    
    char* ptr = result;
    if (bi->negative) {
        *ptr++ = '-';
    }
    
    /* Find most significant non-zero digit */
    int start_digit = (int)bi->length - 1;
    
    /* Print first digit without leading zeros */
    ptr += sprintf(ptr, "%x", bi->digits[start_digit]);
    
    /* Print remaining digits with leading zeros */
    for (int i = start_digit - 1; i >= 0; i--) {
        ptr += sprintf(ptr, "%08x", bi->digits[i]);
    }
    
    return result;
}

/* ============================================================================
 * COMPARISON
 * ============================================================================
 */

int bigint_cmp(const BigInt* a, const BigInt* b) {
    if (!a || !b) {
        return 0;
    }
    
    /* Handle signs */
    if (a->negative != b->negative) {
        return a->negative ? -1 : 1;
    }
    
    /* Same sign - compare magnitudes */
    if (a->length != b->length) {
        int mag_cmp = (a->length > b->length) ? 1 : -1;
        return a->negative ? -mag_cmp : mag_cmp;
    }
    
    /* Same length - compare digits from most significant */
    for (int i = (int)a->length - 1; i >= 0; i--) {
        if (a->digits[i] != b->digits[i]) {
            int mag_cmp = (a->digits[i] > b->digits[i]) ? 1 : -1;
            return a->negative ? -mag_cmp : mag_cmp;
        }
    }
    
    return 0;  /* Equal */
}

bool bigint_is_zero(const BigInt* bi) {
    return bi && bi->length == 1 && bi->digits[0] == 0;
}

bool bigint_is_negative(const BigInt* bi) {
    return bi && bi->negative && !bigint_is_zero(bi);
}

bool bigint_is_positive(const BigInt* bi) {
    return bi && !bi->negative && !bigint_is_zero(bi);
}

bool bigint_is_even(const BigInt* bi) {
    return bi && (bi->digits[0] & 1) == 0;
}

bool bigint_is_odd(const BigInt* bi) {
    return bi && (bi->digits[0] & 1) == 1;
}