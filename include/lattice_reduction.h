#ifndef LATTICE_REDUCTION_H
#define LATTICE_REDUCTION_H

#include "bigint_core.h"
#include "bigfixed_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * lattice_reduction.h - Crystalline Lattice Reduction Algorithms
 * 
 * Core algorithms for the crystalline lattice mathematics:
 * - Gram-Schmidt orthogonalization
 * - LLL (Lenstra-Lenstra-Lovász) lattice reduction
 * - Basis quality metrics
 * 
 * These are FUNDAMENTAL to the crystalline lattice framework:
 * - Find optimal lattice bases
 * - Compress embeddings efficiently
 * - Solve closest vector problems
 * - Enable lattice-based cryptography
 * 
 * All operations use BigFixed for arbitrary precision.
 */

/* ============================================================================
 * LATTICE BASIS STRUCTURE
 * ============================================================================ */

/**
 * Lattice Basis - represents a basis for a lattice in n-dimensional space
 */
typedef struct {
    BigFixed** vectors;        // Array of basis vectors (each is array of coordinates)
    size_t num_vectors;        // Number of basis vectors (rank)
    size_t dimension;          // Dimension of each vector
    int precision;             // Precision for BigFixed arithmetic
    int is_orthogonal;         // Flag: 1 if basis is orthogonal
    int is_reduced;            // Flag: 1 if basis is LLL-reduced
} LatticeBasis;

/* ============================================================================
 * BASIS INITIALIZATION & CLEANUP
 * ============================================================================ */

/**
 * Initialize lattice basis
 * 
 * @param basis Basis to initialize
 * @param num_vectors Number of basis vectors
 * @param dimension Dimension of each vector
 * @param precision Precision for BigFixed arithmetic
 * @return 1 if successful, 0 if failed
 */
int lattice_basis_init(LatticeBasis* basis, size_t num_vectors, 
                       size_t dimension, int precision);

/**
 * Free lattice basis
 * 
 * @param basis Basis to free
 */
void lattice_basis_free(LatticeBasis* basis);

/**
 * Copy lattice basis
 * 
 * @param dest Destination basis
 * @param src Source basis
 * @return 1 if successful, 0 if failed
 */
int lattice_basis_copy(LatticeBasis* dest, const LatticeBasis* src);

/**
 * Set basis vector from array
 * 
 * @param basis Lattice basis
 * @param index Vector index
 * @param coords Array of coordinates
 * @param len Length of coordinate array
 * @return 1 if successful, 0 if failed
 */
int lattice_basis_set_vector(LatticeBasis* basis, size_t index, 
                             const double* coords, size_t len);

/* ============================================================================
 * GRAM-SCHMIDT ORTHOGONALIZATION
 * ============================================================================ */

/**
 * Classical Gram-Schmidt Orthogonalization
 * 
 * Converts a basis into an orthogonal basis using the classical algorithm.
 * 
 * Algorithm:
 * For each vector v_i:
 *   u_i = v_i - Σ(j=0 to i-1) proj_{u_j}(v_i)
 *   where proj_{u_j}(v_i) = (<v_i, u_j> / <u_j, u_j>) * u_j
 * 
 * Properties:
 * - Produces orthogonal vectors
 * - Preserves span of original basis
 * - Can be numerically unstable for ill-conditioned bases
 * 
 * Complexity: O(n² × d) where n = num_vectors, d = dimension
 * 
 * @param orthogonal Output: orthogonal basis
 * @param basis Input: basis to orthogonalize
 * @return 1 if successful, 0 if failed
 */
int gram_schmidt_classical(LatticeBasis* orthogonal, const LatticeBasis* basis);

/**
 * Modified Gram-Schmidt Orthogonalization
 * 
 * More numerically stable version of Gram-Schmidt.
 * 
 * Algorithm:
 * For each vector v_i:
 *   u_i = v_i
 *   For j = 0 to i-1:
 *     u_i = u_i - proj_{u_j}(u_i)
 * 
 * Advantages over classical:
 * - Better numerical stability
 * - Reduces accumulation of rounding errors
 * - Recommended for practical use
 * 
 * Complexity: O(n² × d)
 * 
 * @param orthogonal Output: orthogonal basis
 * @param basis Input: basis to orthogonalize
 * @return 1 if successful, 0 if failed
 */
int gram_schmidt_modified(LatticeBasis* orthogonal, const LatticeBasis* basis);

/**
 * Compute Gram-Schmidt coefficients (μ values)
 * 
 * Computes μ_{i,j} = <b_i, b*_j> / <b*_j, b*_j>
 * where b* is the Gram-Schmidt orthogonalization of b
 * 
 * These coefficients are used in LLL algorithm.
 * 
 * @param mu Output: matrix of coefficients (num_vectors × num_vectors)
 * @param basis Input basis
 * @param orthogonal Gram-Schmidt orthogonalization of basis
 * @return 1 if successful, 0 if failed
 */
int gram_schmidt_coefficients(BigFixed** mu, const LatticeBasis* basis,
                               const LatticeBasis* orthogonal);

/* ============================================================================
 * LLL LATTICE REDUCTION
 * ============================================================================ */

/**
 * LLL Lattice Reduction Algorithm
 * 
 * Finds a "reduced" basis that is nearly orthogonal and has short vectors.
 * This is THE core algorithm for crystalline lattice mathematics.
 * 
 * Algorithm (simplified):
 * 1. Compute Gram-Schmidt orthogonalization
 * 2. For each vector, perform size reduction
 * 3. Check Lovász condition: ||b*_i||² ≥ (δ - μ²_{i,i-1}) ||b*_{i-1}||²
 * 4. If violated, swap vectors and repeat
 * 5. Continue until all conditions satisfied
 * 
 * Properties:
 * - Produces basis with short, nearly orthogonal vectors
 * - First vector is at most 2^((n-1)/2) times shortest vector
 * - Polynomial time: O(n⁴ × d × log B) where B is max entry size
 * 
 * Applications in CLLM:
 * - Optimal embedding compression
 * - Fast nearest neighbor search
 * - Lattice-based quantization
 * - Cryptographic operations
 * 
 * @param reduced Output: LLL-reduced basis
 * @param basis Input: basis to reduce
 * @param delta Reduction parameter (typically 0.75)
 * @return 1 if successful, 0 if failed
 */
int lll_reduce(LatticeBasis* reduced, const LatticeBasis* basis, double delta);

/**
 * LLL reduction with crystalline lattice optimizations
 * 
 * Enhanced LLL that leverages:
 * - Prime factorization structure
 * - Lattice symmetries
 * - Plimpton ratios
 * - Hyperdimensional packing
 * 
 * @param reduced Output: LLL-reduced basis
 * @param basis Input: basis to reduce
 * @param delta Reduction parameter
 * @return 1 if successful, 0 if failed
 */
int lll_reduce_crystalline(LatticeBasis* reduced, const LatticeBasis* basis, 
                           double delta);

/**
 * Size reduction step in LLL
 * 
 * Reduces vector b_i by subtracting integer multiples of previous vectors
 * to minimize Gram-Schmidt coefficients.
 * 
 * @param basis Basis to reduce (modified in place)
 * @param i Index of vector to reduce
 * @param mu Gram-Schmidt coefficients
 * @return 1 if successful, 0 if failed
 */
int lll_size_reduce(LatticeBasis* basis, size_t i, BigFixed** mu);

/**
 * Check Lovász condition
 * 
 * Tests if ||b*_i||² ≥ (δ - μ²_{i,i-1}) ||b*_{i-1}||²
 * 
 * @param orthogonal Gram-Schmidt orthogonalization
 * @param mu Gram-Schmidt coefficients
 * @param i Index to check
 * @param delta Reduction parameter
 * @return 1 if condition satisfied, 0 otherwise
 */
int lll_lovasz_condition(const LatticeBasis* orthogonal, BigFixed** mu,
                         size_t i, double delta);

/* ============================================================================
 * BASIS QUALITY METRICS
 * ============================================================================ */

/**
 * Compute basis orthogonality defect
 * 
 * Measures how far basis is from being orthogonal.
 * 
 * Defect = (∏ ||b_i||) / |det(B)|
 * 
 * Properties:
 * - Defect = 1 for orthogonal basis
 * - Defect > 1 for non-orthogonal basis
 * - Lower is better
 * 
 * @param basis Lattice basis
 * @return Orthogonality defect
 */
double lattice_orthogonality_defect(const LatticeBasis* basis);

/**
 * Compute Hermite factor
 * 
 * Measures quality of shortest vector in basis.
 * 
 * Hermite factor = ||b_1|| / (det(B))^(1/n)
 * 
 * Properties:
 * - Lower is better
 * - LLL guarantees factor ≤ 2^((n-1)/4)
 * 
 * @param basis Lattice basis
 * @return Hermite factor
 */
double lattice_hermite_factor(const LatticeBasis* basis);

/**
 * Compute basis determinant
 * 
 * Determinant = |det(B)| = ∏ ||b*_i||
 * 
 * @param basis Lattice basis
 * @param det Output: determinant
 * @return 1 if successful, 0 if failed
 */
int lattice_determinant(BigFixed* det, const LatticeBasis* basis);

/**
 * Check if basis is LLL-reduced
 * 
 * Verifies all LLL conditions are satisfied.
 * 
 * @param basis Basis to check
 * @param delta Reduction parameter
 * @return 1 if LLL-reduced, 0 otherwise
 */
int lattice_is_lll_reduced(const LatticeBasis* basis, double delta);

/* ============================================================================
 * VECTOR OPERATIONS
 * ============================================================================ */

/**
 * Compute inner product of two vectors
 * 
 * <u, v> = Σ u_i × v_i
 * 
 * @param result Output: inner product
 * @param u First vector
 * @param v Second vector
 * @param dimension Vector dimension
 * @param precision Arithmetic precision
 * @return 1 if successful, 0 if failed
 */
int vector_inner_product(BigFixed* result, const BigFixed* u, const BigFixed* v,
                         size_t dimension, int precision);

/**
 * Compute vector norm (length)
 * 
 * ||v|| = sqrt(<v, v>)
 * 
 * @param result Output: norm
 * @param v Vector
 * @param dimension Vector dimension
 * @param precision Arithmetic precision
 * @return 1 if successful, 0 if failed
 */
int vector_norm(BigFixed* result, const BigFixed* v, size_t dimension, 
                int precision);

/**
 * Compute squared norm (avoids square root)
 * 
 * ||v||² = <v, v>
 * 
 * @param result Output: squared norm
 * @param v Vector
 * @param dimension Vector dimension
 * @param precision Arithmetic precision
 * @return 1 if successful, 0 if failed
 */
int vector_norm_squared(BigFixed* result, const BigFixed* v, size_t dimension,
                        int precision);

/**
 * Project vector u onto vector v
 * 
 * proj_v(u) = (<u, v> / <v, v>) × v
 * 
 * @param result Output: projection (dimension-length array)
 * @param u Vector to project
 * @param v Vector to project onto
 * @param dimension Vector dimension
 * @param precision Arithmetic precision
 * @return 1 if successful, 0 if failed
 */
int vector_project(BigFixed* result, const BigFixed* u, const BigFixed* v,
                   size_t dimension, int precision);

/**
 * Subtract vectors: result = u - v
 * 
 * @param result Output: difference
 * @param u First vector
 * @param v Second vector
 * @param dimension Vector dimension
 * @return 1 if successful, 0 if failed
 */
int vector_subtract(BigFixed* result, const BigFixed* u, const BigFixed* v,
                    size_t dimension);

/**
 * Add vectors: result = u + v
 * 
 * @param result Output: sum
 * @param u First vector
 * @param v Second vector
 * @param dimension Vector dimension
 * @return 1 if successful, 0 if failed
 */
int vector_add(BigFixed* result, const BigFixed* u, const BigFixed* v,
               size_t dimension);

/**
 * Scale vector: result = scalar × v
 * 
 * @param result Output: scaled vector
 * @param scalar Scaling factor
 * @param v Vector to scale
 * @param dimension Vector dimension
 * @param precision Arithmetic precision
 * @return 1 if successful, 0 if failed
 */
int vector_scale(BigFixed* result, const BigFixed* scalar, const BigFixed* v,
                 size_t dimension, int precision);

/* ============================================================================
 * CRYSTALLINE LATTICE SPECIFIC
 * ============================================================================ */

/**
 * Create lattice basis from prime coordinates
 * 
 * Converts CLLM prime-based coordinates to lattice basis.
 * 
 * @param basis Output: lattice basis
 * @param primes Array of prime numbers
 * @param num_primes Number of primes
 * @param dimension Target dimension
 * @param precision Arithmetic precision
 * @return 1 if successful, 0 if failed
 */
int lattice_from_primes(LatticeBasis* basis, const uint64_t* primes,
                        size_t num_primes, size_t dimension, int precision);

/**
 * Apply lattice symmetry operations
 * 
 * Uses crystalline symmetries to optimize basis.
 * 
 * @param basis Basis to optimize (modified in place)
 * @return 1 if successful, 0 if failed
 */
int lattice_apply_symmetries(LatticeBasis* basis);

/**
 * Optimize basis using Plimpton ratios
 * 
 * Leverages Plimpton 322 ratios for optimal basis structure.
 * 
 * @param basis Basis to optimize (modified in place)
 * @return 1 if successful, 0 if failed
 */
int lattice_optimize_plimpton(LatticeBasis* basis);

#ifdef __cplusplus
}
#endif

#endif // LATTICE_REDUCTION_H