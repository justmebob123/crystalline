/*
 * BigFixed Packed Array - Memory-Efficient Storage
 * 
 * CRITICAL: This is a memory-efficient alternative to BigFixed** arrays.
 * Instead of millions of individual structures, we use ONE contiguous buffer.
 * 
 * Memory comparison for 22M parameters:
 * - BigFixed** array: 22M × 208 bytes = 4.6 GB
 * - Packed array: 22M × 16 bytes = 352 MB (13x reduction!)
 */

#ifndef BIGFIXED_PACKED_ARRAY_H
#define BIGFIXED_PACKED_ARRAY_H

#include <stdint.h>
#include <stddef.h>

/*
 * Packed array structure
 * 
 * Data layout: [int64_t integer, int64_t fractional] repeated for each element
 * Total memory: size × 2 × 8 bytes = size × 16 bytes
 */
typedef struct {
    int64_t* data;          // Contiguous buffer: [int, frac, int, frac, ...]
    size_t size;            // Number of elements
    int precision;          // Fractional precision bits
    int64_t scale_factor;   // 2^precision for conversions
} BigFixedPackedArray;

/* Create/destroy */
BigFixedPackedArray* bigfixed_packed_array_create(size_t size, int precision);
void bigfixed_packed_array_free(BigFixedPackedArray* array);

/* Access operations */
double bigfixed_packed_array_get(const BigFixedPackedArray* array, size_t index);
void bigfixed_packed_array_set(BigFixedPackedArray* array, size_t index, double value);
void bigfixed_packed_array_set_int(BigFixedPackedArray* array, size_t index, int64_t value);

/* Array operations */
void bigfixed_packed_array_add(BigFixedPackedArray* result, 
                                const BigFixedPackedArray* a,
                                const BigFixedPackedArray* b);
void bigfixed_packed_array_scale(BigFixedPackedArray* result,
                                  const BigFixedPackedArray* array,
                                  double scalar);
void bigfixed_packed_array_zero(BigFixedPackedArray* array);
void bigfixed_packed_array_copy(BigFixedPackedArray* dest,
                                 const BigFixedPackedArray* src);

/* Utility */
size_t bigfixed_packed_array_memory_usage(const BigFixedPackedArray* array);

#endif /* BIGFIXED_PACKED_ARRAY_H */