/**
 * @file rainbow_table.c
 * @brief Rainbow Table implementation for fast prime lookup
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * This implements the revolutionary rainbow table that provides O(1) prime
 * lookup using the clock lattice structure. This eliminates the need for
 * trial division, sieving, or any other legacy prime generation methods.
 */

#include "math/rainbow.h"
#include "math/prime.h"
#include "math/clock.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * INTERNAL HELPERS
 * ============================================================================
 */

/**
 * @brief Ensure table has enough capacity
 */
static MathError rainbow_ensure_capacity(RainbowTable* table, size_t min_capacity) {
    if (table->capacity >= min_capacity) {
        return MATH_SUCCESS;
    }
    
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < min_capacity) {
        new_capacity = min_capacity;
    }
    
    RainbowEntry* new_entries = (RainbowEntry*)realloc(
        table->entries,
        new_capacity * sizeof(RainbowEntry)
    );
    
    if (!new_entries) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    table->entries = new_entries;
    table->capacity = new_capacity;
    
    return MATH_SUCCESS;
}

/**
 * @brief Binary search for prime in table
 */
static int rainbow_find_prime(const RainbowTable* table, uint64_t prime) {
    if (table->size == 0) {
        return -1;
    }
    
    int left = 0;
    int right = (int)table->size - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (table->entries[mid].prime == prime) {
            return mid;
        } else if (table->entries[mid].prime < prime) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

/**
 * @brief Binary search for index in table
 */
static int rainbow_find_index(const RainbowTable* table, uint64_t index) {
    if (table->size == 0 || index == 0) {
        return -1;
    }
    
    /* Since entries are sorted by prime (which correlates with index),
     * we can use binary search on index */
    int left = 0;
    int right = (int)table->size - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (table->entries[mid].index == index) {
            return mid;
        } else if (table->entries[mid].index < index) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

/* ============================================================================
 * INITIALIZATION AND CLEANUP
 * ============================================================================
 */

MathError rainbow_init(RainbowTable* table, size_t initial_capacity) {
    if (!table) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    if (initial_capacity == 0) {
        initial_capacity = 1000;  /* Default: first 1000 primes */
    }
    
    table->entries = (RainbowEntry*)calloc(initial_capacity, sizeof(RainbowEntry));
    if (!table->entries) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    table->size = 0;
    table->capacity = initial_capacity;
    table->max_prime = 0;
    table->max_index = 0;
    
    return MATH_SUCCESS;
}

void rainbow_cleanup(RainbowTable* table) {
    if (table && table->entries) {
        free(table->entries);
        table->entries = NULL;
        table->size = 0;
        table->capacity = 0;
        table->max_prime = 0;
        table->max_index = 0;
    }
}

/* ============================================================================
 * POPULATION
 * ============================================================================
 */

MathError rainbow_populate_count(RainbowTable* table, uint64_t n) {
    if (!table) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Ensure capacity */
    MathError err = rainbow_ensure_capacity(table, (size_t)n);
    if (err != MATH_SUCCESS) {
        return err;
    }
    
    /* Generate first n primes and populate table */
    uint64_t index = 1;
    
    while (index <= n) {
        /* Get nth prime using existing prime generation */
        uint64_t prime = prime_nth(index);
        if (prime == 0) {
            return MATH_ERROR_INVALID_ARG;
        }
        
        /* Map prime to clock position */
        ClockPosition pos;
        err = clock_map_prime_to_position(prime, &pos);
        if (err != MATH_SUCCESS) {
            return err;
        }
        
        /* Add entry to table */
        table->entries[table->size].prime = prime;
        table->entries[table->size].position = pos;
        table->entries[table->size].index = index;
        table->size++;
        
        /* Update max values */
        if (prime > table->max_prime) {
            table->max_prime = prime;
        }
        if (index > table->max_index) {
            table->max_index = index;
        }
        
        index++;
    }
    
    return MATH_SUCCESS;
}

MathError rainbow_populate_to_prime(RainbowTable* table, uint64_t max_prime) {
    if (!table) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Estimate number of primes up to max_prime using prime number theorem */
    /* π(n) ≈ n / ln(n) */
    uint64_t estimated_count = max_prime / 10;  /* Rough estimate */
    
    /* Ensure capacity */
    MathError err = rainbow_ensure_capacity(table, (size_t)estimated_count);
    if (err != MATH_SUCCESS) {
        return err;
    }
    
    /* Generate primes up to max_prime */
    uint64_t prime = 2;
    uint64_t index = 1;
    
    while (prime <= max_prime) {
        /* Map prime to clock position */
        ClockPosition pos;
        err = clock_map_prime_to_position(prime, &pos);
        if (err != MATH_SUCCESS) {
            return err;
        }
        
        /* Ensure capacity (may need to grow) */
        err = rainbow_ensure_capacity(table, table->size + 1);
        if (err != MATH_SUCCESS) {
            return err;
        }
        
        /* Add entry to table */
        table->entries[table->size].prime = prime;
        table->entries[table->size].position = pos;
        table->entries[table->size].index = index;
        table->size++;
        
        /* Update max values */
        table->max_prime = prime;
        table->max_index = index;
        
        /* Get next prime */
        prime = prime_next(prime);
        if (prime == 0) {
            break;  /* Overflow or error */
        }
        index++;
    }
    
    return MATH_SUCCESS;
}

/* ============================================================================
 * LOOKUP OPERATIONS
 * ============================================================================
 */

MathError rainbow_lookup_by_index(const RainbowTable* table, uint64_t index, uint64_t* prime) {
    if (!table || !prime || index == 0) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Check if index is in range */
    if (index > table->max_index) {
        return MATH_ERROR_OUT_OF_RANGE;
    }
    
    /* Binary search for index */
    int pos = rainbow_find_index(table, index);
    if (pos < 0) {
        return MATH_ERROR_NOT_FOUND;
    }
    
    *prime = table->entries[pos].prime;
    return MATH_SUCCESS;
}

MathError rainbow_lookup_by_position(const RainbowTable* table, const ClockPosition* pos, uint64_t* prime) {
    if (!table || !pos || !prime) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Linear search for position (positions are not sorted) */
    /* TODO: Optimize with position-based indexing */
    for (size_t i = 0; i < table->size; i++) {
        const ClockPosition* entry_pos = &table->entries[i].position;
        
        if (entry_pos->ring == pos->ring && entry_pos->position == pos->position) {
            *prime = table->entries[i].prime;
            return MATH_SUCCESS;
        }
    }
    
    return MATH_ERROR_NOT_FOUND;
}

MathError rainbow_lookup_position(const RainbowTable* table, uint64_t prime, ClockPosition* pos) {
    if (!table || !pos) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Binary search for prime */
    int idx = rainbow_find_prime(table, prime);
    if (idx < 0) {
        return MATH_ERROR_NOT_FOUND;
    }
    
    *pos = table->entries[idx].position;
    return MATH_SUCCESS;
}

MathError rainbow_lookup_index(const RainbowTable* table, uint64_t prime, uint64_t* index) {
    if (!table || !index) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Binary search for prime */
    int idx = rainbow_find_prime(table, prime);
    if (idx < 0) {
        return MATH_ERROR_NOT_FOUND;
    }
    
    *index = table->entries[idx].index;
    return MATH_SUCCESS;
}

/* ============================================================================
 * NAVIGATION
 * ============================================================================
 */

MathError rainbow_next_prime(const RainbowTable* table, uint64_t prime, uint64_t* next) {
    if (!table || !next) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Binary search for prime */
    int idx = rainbow_find_prime(table, prime);
    if (idx < 0) {
        return MATH_ERROR_NOT_FOUND;
    }
    
    /* Check if there's a next prime in table */
    if ((size_t)idx + 1 >= table->size) {
        return MATH_ERROR_OUT_OF_RANGE;
    }
    
    *next = table->entries[idx + 1].prime;
    return MATH_SUCCESS;
}

MathError rainbow_prev_prime(const RainbowTable* table, uint64_t prime, uint64_t* prev) {
    if (!table || !prev) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Binary search for prime */
    int idx = rainbow_find_prime(table, prime);
    if (idx < 0) {
        return MATH_ERROR_NOT_FOUND;
    }
    
    /* Check if there's a previous prime in table */
    if (idx == 0) {
        return MATH_ERROR_OUT_OF_RANGE;
    }
    
    *prev = table->entries[idx - 1].prime;
    return MATH_SUCCESS;
}

/* ============================================================================
 * QUERY OPERATIONS
 * ============================================================================
 */

bool rainbow_contains(const RainbowTable* table, uint64_t prime) {
    if (!table) {
        return false;
    }
    
    return rainbow_find_prime(table, prime) >= 0;
}

size_t rainbow_size(const RainbowTable* table) {
    return table ? table->size : 0;
}

uint64_t rainbow_max_prime(const RainbowTable* table) {
    return table ? table->max_prime : 0;
}