/*
 * BigFixed Array Utilities
 * Helper functions for allocating and managing BigFixed arrays
 */

#include "bigfixed_core.h"
#include <stdlib.h>
#include <string.h>

/*
 * Create an array of BigFixed values
 * Returns: Array of BigFixed* pointers, each initialized to zero
 */
BigFixed** bigfixed_array_create(size_t size, int precision) {
    if (size == 0) return NULL;
    
    BigFixed** array = (BigFixed**)malloc(size * sizeof(BigFixed*));
    if (!array) return NULL;
    
    // Create each BigFixed element
    for (size_t i = 0; i < size; i++) {
        array[i] = big_fixed_create(precision);
        if (!array[i]) {
            // Allocation failed - clean up and return NULL
            for (size_t j = 0; j < i; j++) {
                big_fixed_free(array[j]);
            }
            free(array);
            return NULL;
        }
    }
    
    return array;
}

/*
 * Free an array of BigFixed values
 */
void bigfixed_array_free(BigFixed** array, size_t size) {
    if (!array) return;
    
    for (size_t i = 0; i < size; i++) {
        if (array[i]) {
            big_fixed_free(array[i]);
        }
    }
    free(array);
}

/*
 * Set all elements of a BigFixed array to zero
 */
void bigfixed_array_zero(BigFixed** array, size_t size) {
    if (!array) return;
    
    for (size_t i = 0; i < size; i++) {
        if (array[i]) {
            big_fixed_from_int(array[i], 0);
        }
    }
}

/*
 * Copy a BigFixed array
 */
void bigfixed_array_copy(BigFixed** dest, BigFixed** src, size_t size) {
    if (!dest || !src) return;
    
    for (size_t i = 0; i < size; i++) {
        if (dest[i] && src[i]) {
            big_fixed_assign(dest[i], src[i]);
        }
    }
}

/*
 * Convert float array to BigFixed array
 */
void bigfixed_array_from_float(BigFixed** dest, const float* src, size_t size) {
    if (!dest || !src) return;
    
    for (size_t i = 0; i < size; i++) {
        if (dest[i]) {
            big_fixed_from_double(dest[i], (double)src[i]);
        }
    }
}

/*
 * Convert BigFixed array to float array
 */
void bigfixed_array_to_float(float* dest, BigFixed** src, size_t size) {
    if (!dest || !src) return;
    
    for (size_t i = 0; i < size; i++) {
        if (src[i]) {
            // Convert BigFixed to double, then to float
            // This is a lossy conversion but necessary for compatibility
            char* str = big_fixed_to_string(src[i], 10);
            if (str) {
                dest[i] = (float)atof(str);
                free(str);
            } else {
                dest[i] = 0.0f;
            }
        }
    }
}