/*
 * Crystalline Abacus Array Utilities
 * Helper functions for allocating and managing CrystallineAbacus arrays
 * 
 * MIGRATED FROM: BigFixed array utilities
 * NEW API: Uses CrystallineAbacus from NEW math library
 */

#include "math/abacus.h"
#include "math/types.h"
#include "math/clock.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Create an array of CrystallineAbacus values
 * Returns: Array of CrystallineAbacus* pointers, each initialized to zero
 */
CrystallineAbacus** abacus_array_create(size_t size, uint32_t base, ClockContext* ctx) {
    if (size == 0 || !ctx) return NULL;
    
    CrystallineAbacus** array = (CrystallineAbacus**)malloc(size * sizeof(CrystallineAbacus*));
    if (!array) return NULL;
    
    // Create each Abacus element
    for (size_t i = 0; i < size; i++) {
        array[i] = abacus_create_from_uint64(0, base, ctx);
        if (!array[i]) {
            // Allocation failed - clean up and return NULL
            for (size_t j = 0; j < i; j++) {
                abacus_free(array[j]);
            }
            free(array);
            return NULL;
        }
    }
    
    return array;
}

/*
 * Free an array of CrystallineAbacus values
 */
void abacus_array_free(CrystallineAbacus** array, size_t size) {
    if (!array) return;
    
    for (size_t i = 0; i < size; i++) {
        if (array[i]) {
            abacus_free(array[i]);
        }
    }
    free(array);
}

/*
 * Set all elements of a CrystallineAbacus array to zero
 */
void abacus_array_zero(CrystallineAbacus** array, size_t size) {
    if (!array) return;
    
    for (size_t i = 0; i < size; i++) {
        if (array[i]) {
            abacus_set_zero(array[i]);
        }
    }
}

/*
 * Copy a CrystallineAbacus array
 */
void abacus_array_copy(CrystallineAbacus** dest, CrystallineAbacus** src, size_t size) {
    if (!dest || !src) {
        fprintf(stderr, "ERROR: abacus_array_copy called with NULL pointer (dest=%p, src=%p)\n", 
                (void*)dest, (void*)src);
        return;
    }
    
    for (size_t i = 0; i < size; i++) {
        if (!dest[i] || !src[i]) {
            // Skip NULL entries but don't crash
            if (i < 10) {  // Only log first 10 to avoid spam
                fprintf(stderr, "WARNING: abacus_array_copy skipping NULL at index %zu (dest[i]=%p, src[i]=%p)\n",
                        i, (void*)dest[i], (void*)src[i]);
            }
            continue;
        }
        abacus_copy(dest[i], src[i]);
    }
}

/*
 * Convert float array to CrystallineAbacus array
 */
void abacus_array_from_float(CrystallineAbacus** dest, const float* src, size_t size, 
                             uint32_t base, int32_t precision, ClockContext* ctx) {
    if (!dest || !src || !ctx) return;
    
    for (size_t i = 0; i < size; i++) {
        if (dest[i]) {
            MathError err = abacus_from_double((double)src[i], base, precision, ctx, dest[i]);
            if (err != MATH_SUCCESS) {
                fprintf(stderr, "WARNING: abacus_array_from_float failed at index %zu with error %d\n", i, err);
            }
        }
    }
}

/*
 * Convert CrystallineAbacus array to float array
 */
void abacus_array_to_float(float* dest, CrystallineAbacus** src, size_t size) {
    if (!dest || !src) return;
    
    for (size_t i = 0; i < size; i++) {
        if (src[i]) {
            // Convert Abacus to double, then to float
            // This is a lossy conversion but necessary for compatibility
            dest[i] = (float)abacus_to_double(src[i]);
        } else {
            dest[i] = 0.0f;
        }
    }
}

/* ============================================================================
 * LEGACY COMPATIBILITY WRAPPERS (for gradual migration)
 * These will be removed once all code is migrated
 * ============================================================================
 */

// Temporary typedefs for compatibility
typedef CrystallineAbacus BigFixed;

BigFixed** bigfixed_array_create(size_t size, int precision) {
    // Use default base 12 and global context
    static ClockContext ctx;
    static bool ctx_initialized = false;
    
    if (!ctx_initialized) {
        clock_init(&ctx);
        ctx_initialized = true;
    }
    
    return abacus_array_create(size, 12, &ctx);
}

void bigfixed_array_free(BigFixed** array, size_t size) {
    abacus_array_free(array, size);
}

void bigfixed_array_zero(BigFixed** array, size_t size) {
    abacus_array_zero(array, size);
}

void bigfixed_array_copy(BigFixed** dest, BigFixed** src, size_t size) {
    abacus_array_copy(dest, src, size);
}

void bigfixed_array_from_float(BigFixed** dest, const float* src, size_t size) {
    static ClockContext ctx;
    static bool ctx_initialized = false;
    
    if (!ctx_initialized) {
        clock_init(&ctx);
        ctx_initialized = true;
    }
    
    abacus_array_from_float(dest, src, size, 12, -6, &ctx);
}

void bigfixed_array_to_float(float* dest, BigFixed** src, size_t size) {
    abacus_array_to_float(dest, src, size);
}