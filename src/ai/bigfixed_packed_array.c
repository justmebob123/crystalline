/*
 * BigFixed Packed Array - Memory-Efficient Storage
 * 
 * CRITICAL FIX: Instead of creating millions of individual BigFixed structures,
 * we create ONE packed array with contiguous memory.
 * 
 * Memory savings:
 * - Old way: 22M × 208 bytes = 4.6 GB per array
 * - New way: 22M × 16 bytes = 352 MB per array (13x reduction!)
 */

#include "bigfixed_packed_array.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Create a packed BigFixed array
 * 
 * Instead of BigFixed**, we use a single contiguous buffer:
 * - Each element is just 2 int64_t values (integer + fractional)
 * - Total: size × 16 bytes
 */
BigFixedPackedArray* bigfixed_packed_array_create(size_t size, int precision) {
    if (size == 0) return NULL;
    
    // Allocate the container
    BigFixedPackedArray* array = (BigFixedPackedArray*)malloc(sizeof(BigFixedPackedArray));
    if (!array) return NULL;
    
    // Allocate ONE contiguous buffer for all data
    // Each element: 2 × int64_t = 16 bytes
    array->data = (int64_t*)calloc(size * 2, sizeof(int64_t));
    if (!array->data) {
        free(array);
        return NULL;
    }
    
    array->size = size;
    array->precision = precision;
    array->scale_factor = (1LL << precision);  // 2^precision
    
    printf("✓ Packed array allocated: %zu elements × 16 bytes = %.2f MB\n",
           size, (size * 16.0) / (1024.0 * 1024.0));
    
    return array;
}

/*
 * Free a packed array
 */
void bigfixed_packed_array_free(BigFixedPackedArray* array) {
    if (!array) return;
    if (array->data) free(array->data);
    free(array);
}

/*
 * Get value at index (returns as double for convenience)
 */
double bigfixed_packed_array_get(const BigFixedPackedArray* array, size_t index) {
    if (!array || index >= array->size) return 0.0;
    
    int64_t integer = array->data[index * 2];
    int64_t fractional = array->data[index * 2 + 1];
    
    return (double)integer + ((double)fractional / (double)array->scale_factor);
}

/*
 * Set value at index (from double)
 */
void bigfixed_packed_array_set(BigFixedPackedArray* array, size_t index, double value) {
    if (!array || index >= array->size) return;
    
    int64_t integer = (int64_t)value;
    double frac = value - (double)integer;
    int64_t fractional = (int64_t)(frac * (double)array->scale_factor);
    
    array->data[index * 2] = integer;
    array->data[index * 2 + 1] = fractional;
}

/*
 * Set value at index (from int64_t, for zero initialization)
 */
void bigfixed_packed_array_set_int(BigFixedPackedArray* array, size_t index, int64_t value) {
    if (!array || index >= array->size) return;
    
    array->data[index * 2] = value;
    array->data[index * 2 + 1] = 0;
}

/*
 * Add two packed arrays: result = a + b
 */
void bigfixed_packed_array_add(BigFixedPackedArray* result, 
                                const BigFixedPackedArray* a,
                                const BigFixedPackedArray* b) {
    if (!result || !a || !b) return;
    if (result->size != a->size || result->size != b->size) return;
    
    for (size_t i = 0; i < result->size; i++) {
        int64_t a_int = a->data[i * 2];
        int64_t a_frac = a->data[i * 2 + 1];
        int64_t b_int = b->data[i * 2];
        int64_t b_frac = b->data[i * 2 + 1];
        
        // Add fractional parts
        int64_t frac_sum = a_frac + b_frac;
        int64_t carry = 0;
        if (frac_sum >= result->scale_factor) {
            carry = 1;
            frac_sum -= result->scale_factor;
        }
        
        // Add integer parts with carry
        int64_t int_sum = a_int + b_int + carry;
        
        result->data[i * 2] = int_sum;
        result->data[i * 2 + 1] = frac_sum;
    }
}

/*
 * Multiply packed array by scalar: result = array * scalar
 */
void bigfixed_packed_array_scale(BigFixedPackedArray* result,
                                  const BigFixedPackedArray* array,
                                  double scalar) {
    if (!result || !array) return;
    if (result->size != array->size) return;
    
    for (size_t i = 0; i < result->size; i++) {
        double value = bigfixed_packed_array_get(array, i);
        bigfixed_packed_array_set(result, i, value * scalar);
    }
}

/*
 * Zero out all elements
 */
void bigfixed_packed_array_zero(BigFixedPackedArray* array) {
    if (!array || !array->data) return;
    memset(array->data, 0, array->size * 2 * sizeof(int64_t));
}

/*
 * Copy array: dest = src
 */
void bigfixed_packed_array_copy(BigFixedPackedArray* dest,
                                 const BigFixedPackedArray* src) {
    if (!dest || !src || !dest->data || !src->data) return;
    if (dest->size != src->size) return;
    
    memcpy(dest->data, src->data, src->size * 2 * sizeof(int64_t));
}

/*
 * Get memory usage in bytes
 */
size_t bigfixed_packed_array_memory_usage(const BigFixedPackedArray* array) {
    if (!array) return 0;
    return sizeof(BigFixedPackedArray) + (array->size * 2 * sizeof(int64_t));
}