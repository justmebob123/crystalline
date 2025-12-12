/**
 * @file compact_vector.h
 * @brief Compact Vector Representation for Memory Hopping Architecture
 * 
 * Revolutionary memory optimization using vector-based representation
 * with on-demand reconstruction via O(1) prime generation formula.
 * 
 * Key Innovation: Store only (sphere_id, phase_angle, magnitude_offset)
 * instead of full number arrays. Reduces memory by 10-625x.
 */

#ifndef MATH_COMPACT_VECTOR_H
#define MATH_COMPACT_VECTOR_H

#include "math/types.h"
#include "math/clock.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * COMPACT VECTOR STRUCTURES
 * ============================================================================
 */

/**
 * @brief Platonic solid types for geometric framework
 */
typedef enum {
    SOLID_TETRAHEDRON,    /**< 4 vertices, 6 edges, 4 faces */
    SOLID_CUBE,           /**< 8 vertices, 12 edges, 6 faces */
    SOLID_OCTAHEDRON,     /**< 6 vertices, 12 edges, 8 faces */
    SOLID_DODECAHEDRON,   /**< 20 vertices, 30 edges, 12 faces */
    SOLID_ICOSAHEDRON,    /**< 12 vertices, 30 edges, 20 faces */
    SOLID_5_CELL,         /**< 5 vertices (4D simplex) */
    SOLID_TESSERACT,      /**< 16 vertices (4D hypercube) */
    SOLID_16_CELL,        /**< 8 vertices (4D cross-polytope) */
    SOLID_24_CELL,        /**< 24 vertices (4D) */
    SOLID_120_CELL,       /**< 600 vertices (4D) */
    SOLID_600_CELL,       /**< 120 vertices (4D) */
    SOLID_SIMPLEX_ND,     /**< nD simplex (dynamic) */
    SOLID_HYPERCUBE_ND,   /**< nD hypercube (dynamic) */
    SOLID_CROSS_POLYTOPE_ND /**< nD cross-polytope (dynamic) */
} PlatonicSolidType;

/**
 * @brief Compact vector representation (16 bytes)
 * 
 * Replaces full bead storage with geometric position.
 * Enables O(1) reconstruction using prime generation formula.
 */
typedef struct {
    uint32_t sphere_id;        /**< Sphere in hierarchy (4 bytes) */
    float phase_angle;         /**< Precise angle 0-360° (4 bytes) */
    int32_t magnitude_offset;  /**< Offset from base magnitude (4 bytes) */
    float phase_offset;        /**< Prime alignment offset (4 bytes) */
} CompactVector;

/**
 * @brief Extended compact vector with solid mapping (32 bytes)
 */
typedef struct {
    CompactVector base;        /**< Base vector (16 bytes) */
    PlatonicSolidType solid;   /**< Geometric framework (4 bytes) */
    uint32_t vertex_id;        /**< Position on solid (4 bytes) */
    uint32_t ring;             /**< Clock ring (0-3) (4 bytes) */
    uint32_t reserved;         /**< Reserved for alignment (4 bytes) */
} ExtendedCompactVector;

/**
 * @brief Compact number representation
 * 
 * Stores number as array of significant vectors instead of full beads.
 * Memory usage: O(log n) instead of O(n).
 */
typedef struct {
    CompactVector* vectors;    /**< Array of significant vectors */
    size_t num_vectors;        /**< Number of vectors */
    size_t capacity;           /**< Allocated capacity */
    uint32_t base;             /**< Number base (12, 60, 100) */
    uint32_t precision;        /**< Precision in digits */
    bool negative;             /**< Sign */
    ClockContext* clock_ctx;   /**< Clock context for reconstruction */
} CompactNumber;

/**
 * @brief Triangle for triangulation-based operations
 */
typedef struct {
    CompactVector p0;          /**< Origin point */
    CompactVector p1;          /**< First operand */
    CompactVector p2;          /**< Second operand */
} Triangle;

/**
 * @brief Precise clock position with 360° precision
 */
typedef struct {
    uint32_t ring;             /**< Ring number (0-3) */
    double precise_angle;      /**< Precise angle 0-360° */
    uint64_t magnitude;        /**< Magnitude value */
    double phase_offset;       /**< Phase offset for prime alignment */
} PreciseClockPosition;

/* ============================================================================
 * COMPACT VECTOR OPERATIONS
 * ============================================================================
 */

/**
 * @brief Create a compact vector
 * 
 * @param sphere_id Sphere in hierarchy
 * @param phase_angle Phase angle (0-360°)
 * @param magnitude_offset Magnitude offset
 * @param vector Output vector
 * @return MATH_SUCCESS or error code
 */
MathError compact_vector_create(
    uint32_t sphere_id,
    float phase_angle,
    int32_t magnitude_offset,
    CompactVector* vector
);

/**
 * @brief Calculate distance between two vectors
 * 
 * @param v1 First vector
 * @param v2 Second vector
 * @param distance Output distance
 * @return MATH_SUCCESS or error code
 */
MathError compact_vector_distance(
    const CompactVector* v1,
    const CompactVector* v2,
    double* distance
);

/**
 * @brief Calculate angle between two vectors
 * 
 * @param v1 First vector
 * @param v2 Second vector
 * @param angle Output angle in degrees
 * @return MATH_SUCCESS or error code
 */
MathError compact_vector_angle(
    const CompactVector* v1,
    const CompactVector* v2,
    double* angle
);

/**
 * @brief Compare two vectors
 * 
 * @param v1 First vector
 * @param v2 Second vector
 * @return -1 if v1 < v2, 0 if equal, 1 if v1 > v2
 */
int compact_vector_compare(
    const CompactVector* v1,
    const CompactVector* v2
);

/* ============================================================================
 * COMPACT NUMBER OPERATIONS
 * ============================================================================
 */

/**
 * @brief Create a compact number
 * 
 * @param base Number base (12, 60, or 100)
 * @param precision Precision in digits
 * @param clock_ctx Clock context for reconstruction
 * @param number Output number
 * @return MATH_SUCCESS or error code
 */
MathError compact_number_create(
    uint32_t base,
    uint32_t precision,
    ClockContext* clock_ctx,
    CompactNumber** number
);

/**
 * @brief Free a compact number
 * 
 * @param number Number to free
 */
void compact_number_free(CompactNumber* number);

/**
 * @brief Create compact number from uint64
 * 
 * @param value Input value
 * @param base Number base
 * @param precision Precision in digits
 * @param clock_ctx Clock context
 * @param number Output number
 * @return MATH_SUCCESS or error code
 */
MathError compact_number_from_uint64(
    uint64_t value,
    uint32_t base,
    uint32_t precision,
    ClockContext* clock_ctx,
    CompactNumber** number
);

/**
 * @brief Convert compact number to uint64
 * 
 * @param number Input number
 * @param value Output value
 * @return MATH_SUCCESS or error code
 */
MathError compact_number_to_uint64(
    const CompactNumber* number,
    uint64_t* value
);

/**
 * @brief Get digit at specific exponent
 * 
 * Reconstructs digit on-demand using O(1) formula.
 * 
 * @param number Input number
 * @param exponent Exponent position
 * @param digit Output digit
 * @return MATH_SUCCESS or error code
 */
MathError compact_number_get_digit(
    const CompactNumber* number,
    int32_t exponent,
    uint32_t* digit
);

/**
 * @brief Convert compact number to string
 * 
 * @param number Input number
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return MATH_SUCCESS or error code
 */
MathError compact_number_to_string(
    const CompactNumber* number,
    char* buffer,
    size_t buffer_size
);

/* ============================================================================
 * TRIANGULATION OPERATIONS
 * ============================================================================
 */

/**
 * @brief Triangulate addition
 * 
 * Uses spherical geometry to compute A + B from triangle (0, A, B).
 * 
 * @param tri Input triangle
 * @param result Output vector
 * @return MATH_SUCCESS or error code
 */
MathError triangulate_addition(
    const Triangle* tri,
    CompactVector* result
);

/**
 * @brief Triangulate subtraction
 * 
 * Uses spherical geometry to compute A - B from triangle (0, A, -B).
 * 
 * @param tri Input triangle
 * @param result Output vector
 * @return MATH_SUCCESS or error code
 */
MathError triangulate_subtraction(
    const Triangle* tri,
    CompactVector* result
);

/**
 * @brief Triangulate multiplication
 * 
 * Uses rotation composition to compute A × B.
 * 
 * @param tri Input triangle
 * @param result Output vector
 * @return MATH_SUCCESS or error code
 */
MathError triangulate_multiplication(
    const Triangle* tri,
    CompactVector* result
);

/* ============================================================================
 * PRECISE CLOCK OPERATIONS
 * ============================================================================
 */

/**
 * @brief Get precise clock position with 360° precision
 * 
 * Uses factor-based angle refinement for full precision.
 * 
 * @param number Input number
 * @param position Output position
 * @return MATH_SUCCESS or error code
 */
MathError get_precise_clock_position(
    uint64_t number,
    PreciseClockPosition* position
);

/**
 * @brief Calculate precise angle with factor refinement
 * 
 * @param number Input number
 * @param factors Array of factors
 * @param num_factors Number of factors
 * @param angle Output angle (0-360°)
 * @return MATH_SUCCESS or error code
 */
MathError calculate_precise_angle_with_factors(
    uint64_t number,
    const uint64_t* factors,
    size_t num_factors,
    double* angle
);

/**
 * @brief Find phase offset for prime alignment
 * 
 * Finds phase offset where position becomes prime.
 * 
 * @param position Clock position
 * @param magnitude Magnitude value
 * @param phase_offset Output phase offset
 * @return MATH_SUCCESS or error code
 */
MathError find_prime_phase_offset(
    uint32_t position,
    uint64_t magnitude,
    double* phase_offset
);

/* ============================================================================
 * MEMORY STATISTICS
 * ============================================================================
 */

/**
 * @brief Get memory usage of compact number
 * 
 * @param number Input number
 * @return Memory usage in bytes
 */
size_t compact_number_memory_usage(const CompactNumber* number);

/**
 * @brief Get memory reduction ratio
 * 
 * Compares compact representation to traditional abacus.
 * 
 * @param number Input number
 * @param traditional_size Traditional size in bytes
 * @return Reduction ratio (e.g., 12.5 for 12.5x reduction)
 */
double compact_number_reduction_ratio(
    const CompactNumber* number,
    size_t traditional_size
);

#ifdef __cplusplus
}
#endif

#endif /* MATH_COMPACT_VECTOR_H */