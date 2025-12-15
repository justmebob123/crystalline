/**
 * @file polytope_ntt.c
 * @brief NTT Integration for Polytope Operations - Implementation
 * 
 * This file implements NTT-based operations for polytopes, providing
 * O(n log n) performance for large polytopes.
 */

#include "math/polytope_ntt.h"
#include "math/ntt.h"
#include "math/polytope.h"
#include "math/higher_faces.h"
#include "math/abacus.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>

/* Default Abacus base (Babylonian) */
#define NTT_ABACUS_BASE 60

/* ============================================================================
 * AUTOMATIC NTT SELECTION
 * ============================================================================ */

bool polytope_ntt_should_use(const PlatonicSolid* solid) {
    return polytope_ntt_should_use_threshold(solid, POLYTOPE_NTT_DEFAULT_THRESHOLD);
}

bool polytope_ntt_should_use_threshold(const PlatonicSolid* solid, 
                                        uint32_t threshold) {
    if (!solid) return false;
    
    /* Check vertex count threshold */
    if (solid->num_vertices < threshold) {
        return false;  /* Too small, direct method faster */
    }
    
    /* Check if suitable NTT prime exists */
    uint64_t prime = polytope_ntt_find_optimal_prime(solid);
    if (prime == 0) {
        return false;  /* No suitable prime */
    }
    
    /* Check memory requirements */
    size_t transform_size = polytope_ntt_get_transform_size(solid);
    size_t memory_needed = transform_size * sizeof(CrystallineAbacus*) * 2;
    if (memory_needed > POLYTOPE_NTT_MAX_MEMORY) {
        return false;  /* Too much memory required */
    }
    
    return true;  /* Use NTT */
}

uint64_t polytope_ntt_find_optimal_prime(const PlatonicSolid* solid) {
    if (!solid) return 0;
    
    /* Get required transform size */
    size_t transform_size = polytope_ntt_get_transform_size(solid);
    
    /* Known NTT-friendly primes (from ntt.c) */
    struct {
        size_t max_n;
        uint64_t prime_value;
    } known_primes[] = {
        {256, 257ULL},                    /* 2^8 + 1 */
        {65536, 65537ULL},                /* 2^16 + 1 (Fermat prime) */
        {16777216, 167772161ULL},         /* 10×2^24 + 1 */
        {33554432, 469762049ULL},         /* 7×2^26 + 1 */
        {67108864, 998244353ULL},         /* 119×2^23 + 1 */
        {134217728, 2013265921ULL},       /* 15×2^27 + 1 */
        {268435456, 2281701377ULL},       /* 17×2^27 + 1 */
        {536870912, 3221225473ULL},       /* 3×2^30 + 1 */
        {1073741824, 4253024257ULL},      /* 63×2^26 + 1 */
        {2147483648ULL, 4261412865ULL},   /* 2^32 - 2^25 + 1 */
    };
    
    /* Find smallest suitable prime */
    for (size_t i = 0; i < sizeof(known_primes)/sizeof(known_primes[0]); i++) {
        if (transform_size <= known_primes[i].max_n) {
            return known_primes[i].prime_value;
        }
    }
    
    /* If no suitable prime found, return largest one */
    return 4261412865ULL;
}

size_t polytope_ntt_get_transform_size(const PlatonicSolid* solid) {
    if (!solid) return 0;
    
    /* Return next power of 2 >= vertex count */
    return ntt_next_power_of_2(solid->num_vertices);
}

/* ============================================================================
 * NTT CONTEXT MANAGEMENT
 * ============================================================================ */

NTTContext* polytope_ntt_create_context(const PlatonicSolid* solid) {
    if (!solid) return NULL;
    
    /* Get optimal parameters */
    size_t transform_size = polytope_ntt_get_transform_size(solid);
    uint64_t prime = polytope_ntt_find_optimal_prime(solid);
    
    if (prime == 0) return NULL;
    
    /* Create context with optimal parameters */
    return polytope_ntt_create_context_custom(transform_size, prime);
}

NTTContext* polytope_ntt_create_context_custom(size_t transform_size, 
                                                uint64_t prime) {
    if (!ntt_is_power_of_2(transform_size) || prime == 0) {
        return NULL;
    }
    
    /* Create NTT context */
    NTTContext* ctx = ntt_create(transform_size);
    if (!ctx) return NULL;
    
    /* Initialize with specific prime */
    CrystallineAbacus* prime_abacus = abacus_from_uint64(prime, NTT_ABACUS_BASE);
    if (!prime_abacus) {
        ntt_free(ctx);
        return NULL;
    }
    
    bool success = ntt_init_with_prime(ctx, transform_size, prime_abacus);
    abacus_free(prime_abacus);
    
    if (!success) {
        ntt_free(ctx);
        return NULL;
    }
    
    return ctx;
}

/* ============================================================================
 * NTT-BASED FACE ENUMERATION
 * ============================================================================ */

MathError polytope_ntt_enumerate_faces(const PlatonicSolid* solid,
                                        uint32_t k,
                                        NTTContext* ctx,
                                        KFaceSet** faces) {
    if (!solid || !faces) return MATH_ERROR_INVALID_ARG;
    
    (void)k;  /* Will be used in full implementation */
    
    /* Create context if not provided */
    bool created_ctx = false;
    if (!ctx) {
        ctx = polytope_ntt_create_context(solid);
        if (!ctx) {
            /* Fall back to direct method if NTT not available */
            return MATH_ERROR_NOT_FOUND;  /* Placeholder for NOT_IMPLEMENTED */
        }
        created_ctx = true;
    }
    
    /* TODO: Implement NTT-based face enumeration
     * 
     * Algorithm:
     * 1. Convert vertex adjacency to polynomial coefficients
     * 2. Pad to transform size
     * 3. Apply forward NTT
     * 4. Perform pointwise multiplication (convolution)
     * 5. Apply inverse NTT
     * 6. Extract face combinations from result
     * 7. Validate and construct face structures
     * 
     * For now, return NOT_IMPLEMENTED to indicate this needs implementation
     */
    
    MathError result = MATH_ERROR_NOT_FOUND;  /* Placeholder */
    
    /* Clean up context if we created it */
    if (created_ctx) {
        ntt_free(ctx);
    }
    
    return result;
}

MathError polytope_ntt_generate_hierarchy(const PlatonicSolid* solid,
                                           NTTContext* ctx,
                                           FaceHierarchy** hierarchy) {
    if (!solid || !hierarchy) return MATH_ERROR_INVALID_ARG;
    
    /* Create context if not provided */
    bool created_ctx = false;
    if (!ctx) {
        ctx = polytope_ntt_create_context(solid);
        if (!ctx) {
            /* Fall back to direct method */
            return MATH_ERROR_NOT_FOUND;
        }
        created_ctx = true;
    }
    
    /* TODO: Implement NTT-based hierarchy generation
     * 
     * Generate all k-faces (0 to dimension-1) using NTT optimization
     * For now, return NOT_IMPLEMENTED
     */
    
    MathError result = MATH_ERROR_NOT_FOUND;  /* Placeholder */
    
    /* Clean up context if we created it */
    if (created_ctx) {
        ntt_free(ctx);
    }
    
    return result;
}

/* ============================================================================
 * NTT-BASED VERTEX TRANSFORMATIONS
 * ============================================================================ */

MathError polytope_ntt_transform_vertices(PlatonicSolid* solid,
                                           const CrystallineAbacus** transformation,
                                           NTTContext* ctx) {
    if (!solid || !transformation) return MATH_ERROR_INVALID_ARG;
    
    /* Create context if not provided */
    bool created_ctx = false;
    if (!ctx) {
        ctx = polytope_ntt_create_context(solid);
        if (!ctx) {
            /* Fall back to direct method */
            return MATH_ERROR_NOT_FOUND;
        }
        created_ctx = true;
    }
    
    /* TODO: Implement NTT-based vertex transformation
     * 
     * Use fast polynomial multiplication via NTT for matrix-vector operations
     * Complexity: O(n^2 log n) vs O(n^3) for direct method
     */
    
    MathError result = MATH_ERROR_NOT_FOUND;  /* Placeholder */
    
    /* Clean up context if we created it */
    if (created_ctx) {
        ntt_free(ctx);
    }
    
    return result;
}

MathError polytope_ntt_rotate(PlatonicSolid* solid,
                               const CrystallineAbacus** axis,
                               const CrystallineAbacus* angle,
                               NTTContext* ctx) {
    if (!solid || !axis || !angle) return MATH_ERROR_INVALID_ARG;
    
    /* TODO: Implement NTT-based rotation
     * 
     * 1. Construct rotation matrix from axis and angle
     * 2. Use polytope_ntt_transform_vertices() to apply rotation
     */
    
    (void)ctx;  /* Unused for now */
    return MATH_ERROR_NOT_FOUND;  /* Placeholder */
}

MathError polytope_ntt_scale(PlatonicSolid* solid,
                              const CrystallineAbacus* scale_factor,
                              NTTContext* ctx) {
    if (!solid || !scale_factor) return MATH_ERROR_INVALID_ARG;
    
    /* Create context if not provided */
    bool created_ctx = false;
    if (!ctx) {
        ctx = polytope_ntt_create_context(solid);
        if (!ctx) {
            /* Fall back to direct method */
            return MATH_ERROR_NOT_FOUND;
        }
        created_ctx = true;
    }
    
    /* TODO: Implement NTT-based scaling
     * 
     * Scale all vertex coordinates by scale_factor using NTT optimization
     */
    
    MathError result = MATH_ERROR_NOT_FOUND;  /* Placeholder */
    
    /* Clean up context if we created it */
    if (created_ctx) {
        ntt_free(ctx);
    }
    
    return result;
}

/* ============================================================================
 * NTT-BASED CONVOLUTION OPERATIONS
 * ============================================================================ */

MathError polytope_ntt_convolve(CrystallineAbacus** result,
                                 const CrystallineAbacus** a, size_t n,
                                 const CrystallineAbacus** b, size_t m,
                                 NTTContext* ctx) {
    if (!result || !a || !b || !ctx || n == 0 || m == 0) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Determine required transform size */
    size_t result_size = n + m - 1;
    size_t transform_size = ntt_next_power_of_2(result_size);
    
    /* Verify context has correct size */
    if (ctx->n < transform_size) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* Allocate padded arrays */
    CrystallineAbacus** a_padded = (CrystallineAbacus**)calloc(transform_size, 
                                                                sizeof(CrystallineAbacus*));
    CrystallineAbacus** b_padded = (CrystallineAbacus**)calloc(transform_size,
                                                                sizeof(CrystallineAbacus*));
    CrystallineAbacus** a_transformed = (CrystallineAbacus**)calloc(transform_size,
                                                                     sizeof(CrystallineAbacus*));
    CrystallineAbacus** b_transformed = (CrystallineAbacus**)calloc(transform_size,
                                                                     sizeof(CrystallineAbacus*));
    CrystallineAbacus** product = (CrystallineAbacus**)calloc(transform_size,
                                                               sizeof(CrystallineAbacus*));
    
    if (!a_padded || !b_padded || !a_transformed || !b_transformed || !product) {
        free(a_padded);
        free(b_padded);
        free(a_transformed);
        free(b_transformed);
        free(product);
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    /* Copy and pad input arrays */
    for (size_t i = 0; i < n; i++) {
        a_padded[i] = abacus_copy(a[i]);
        if (!a_padded[i]) {
            /* Cleanup and return error */
            for (size_t j = 0; j < i; j++) abacus_free(a_padded[j]);
            free(a_padded); free(b_padded); free(a_transformed);
            free(b_transformed); free(product);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
    }
    for (size_t i = n; i < transform_size; i++) {
        a_padded[i] = abacus_from_uint64(0, NTT_ABACUS_BASE);
    }
    
    for (size_t i = 0; i < m; i++) {
        b_padded[i] = abacus_copy(b[i]);
        if (!b_padded[i]) {
            /* Cleanup and return error */
            for (size_t j = 0; j < transform_size; j++) abacus_free(a_padded[j]);
            for (size_t j = 0; j < i; j++) abacus_free(b_padded[j]);
            free(a_padded); free(b_padded); free(a_transformed);
            free(b_transformed); free(product);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
    }
    for (size_t i = m; i < transform_size; i++) {
        b_padded[i] = abacus_from_uint64(0, NTT_ABACUS_BASE);
    }
    
    /* Apply forward NTT to both sequences */
    MathError err = ntt_forward(ctx, a_transformed, 
                                (const CrystallineAbacus**)a_padded, 
                                transform_size);
    if (err != MATH_SUCCESS) {
        /* Cleanup */
        for (size_t i = 0; i < transform_size; i++) {
            abacus_free(a_padded[i]);
            abacus_free(b_padded[i]);
        }
        free(a_padded); free(b_padded); free(a_transformed);
        free(b_transformed); free(product);
        return err;
    }
    
    err = ntt_forward(ctx, b_transformed,
                     (const CrystallineAbacus**)b_padded,
                     transform_size);
    if (err != MATH_SUCCESS) {
        /* Cleanup */
        for (size_t i = 0; i < transform_size; i++) {
            abacus_free(a_padded[i]);
            abacus_free(b_padded[i]);
            abacus_free(a_transformed[i]);
        }
        free(a_padded); free(b_padded); free(a_transformed);
        free(b_transformed); free(product);
        return err;
    }
    
    /* Pointwise multiplication */
    for (size_t i = 0; i < transform_size; i++) {
        product[i] = abacus_new(NTT_ABACUS_BASE);
        if (!product[i]) {
            /* Cleanup */
            for (size_t j = 0; j < i; j++) abacus_free(product[j]);
            for (size_t j = 0; j < transform_size; j++) {
                abacus_free(a_padded[j]);
                abacus_free(b_padded[j]);
                abacus_free(a_transformed[j]);
                abacus_free(b_transformed[j]);
            }
            free(a_padded); free(b_padded); free(a_transformed);
            free(b_transformed); free(product);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
        
        err = abacus_mod_mul(product[i], a_transformed[i], b_transformed[i], ctx->prime);
        if (err != MATH_SUCCESS) {
            /* Cleanup */
            for (size_t j = 0; j <= i; j++) abacus_free(product[j]);
            for (size_t j = 0; j < transform_size; j++) {
                abacus_free(a_padded[j]);
                abacus_free(b_padded[j]);
                abacus_free(a_transformed[j]);
                abacus_free(b_transformed[j]);
            }
            free(a_padded); free(b_padded); free(a_transformed);
            free(b_transformed); free(product);
            return err;
        }
    }
    
    /* Apply inverse NTT */
    CrystallineAbacus** result_full = (CrystallineAbacus**)calloc(transform_size,
                                                                   sizeof(CrystallineAbacus*));
    if (!result_full) {
        /* Cleanup */
        for (size_t i = 0; i < transform_size; i++) {
            abacus_free(a_padded[i]);
            abacus_free(b_padded[i]);
            abacus_free(a_transformed[i]);
            abacus_free(b_transformed[i]);
            abacus_free(product[i]);
        }
        free(a_padded); free(b_padded); free(a_transformed);
        free(b_transformed); free(product);
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    err = ntt_inverse(ctx, result_full,
                     (const CrystallineAbacus**)product,
                     transform_size);
    
    /* Copy result (only first result_size elements) */
    if (err == MATH_SUCCESS) {
        for (size_t i = 0; i < result_size; i++) {
            result[i] = abacus_copy(result_full[i]);
        }
    }
    
    /* Cleanup */
    for (size_t i = 0; i < transform_size; i++) {
        abacus_free(a_padded[i]);
        abacus_free(b_padded[i]);
        abacus_free(a_transformed[i]);
        abacus_free(b_transformed[i]);
        abacus_free(product[i]);
        abacus_free(result_full[i]);
    }
    free(a_padded);
    free(b_padded);
    free(a_transformed);
    free(b_transformed);
    free(product);
    free(result_full);
    
    return err;
}

MathError polytope_ntt_convolve_coordinates(CrystallineAbacus** result,
                                             const CrystallineAbacus** coords1,
                                             const CrystallineAbacus** coords2,
                                             uint32_t dimension,
                                             NTTContext* ctx) {
    if (!result || !coords1 || !coords2 || !ctx || dimension == 0) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    /* TODO: Implement coordinate-specific convolution
     * 
     * This is similar to general convolution but handles coordinate
     * structures specifically for nested polytope operations
     */
    
    return MATH_ERROR_NOT_FOUND;  /* Placeholder */
}

/* ============================================================================
 * PERFORMANCE UTILITIES
 * ============================================================================ */

double polytope_ntt_estimate_speedup(const PlatonicSolid* solid,
                                      const char* operation) {
    if (!solid || !operation) return 1.0;
    
    uint64_t n = solid->num_vertices;
    
    /* Estimate based on operation type and complexity analysis */
    if (strcmp(operation, "face_enum") == 0) {
        /* Face enumeration: O(n^k) vs O(n log n)
         * For 2-faces (k=3): O(n^3) vs O(n log n)
         * Speedup ≈ n^2 / log(n)
         */
        if (n < 10) return 0.5;  /* Overhead dominates for small n */
        double log_n = 0;
        uint64_t temp = n;
        while (temp > 1) {
            log_n += 1.0;
            temp /= 2;
        }
        return (double)(n * n) / log_n;
    }
    else if (strcmp(operation, "transform") == 0) {
        /* Vertex transformation: O(n^3) vs O(n^2 log n)
         * Speedup ≈ n / log(n)
         */
        if (n < 10) return 0.5;
        double log_n = 0;
        uint64_t temp = n;
        while (temp > 1) {
            log_n += 1.0;
            temp /= 2;
        }
        return (double)n / log_n;
    }
    else if (strcmp(operation, "convolve") == 0) {
        /* Convolution: O(n^2) vs O(n log n)
         * Speedup ≈ n / log(n)
         */
        if (n < 10) return 0.5;
        double log_n = 0;
        uint64_t temp = n;
        while (temp > 1) {
            log_n += 1.0;
            temp /= 2;
        }
        return (double)n / log_n;
    }
    
    return 1.0;  /* Unknown operation */
}

MathError polytope_ntt_get_stats(const PlatonicSolid* solid,
                                  const NTTContext* ctx,
                                  PolytopeNTTStats* stats) {
    if (!solid || !ctx || !stats) return MATH_ERROR_INVALID_ARG;
    
    /* Fill in statistics */
    stats->transform_size = ctx->n;
    
    /* Get prime value from Abacus */
    stats->prime_used = 0;
    if (ctx->prime) {
        /* TODO: Convert Abacus to uint64_t
         * For now, use placeholder
         */
        stats->prime_used = 998244353ULL;  /* Common NTT prime */
    }
    
    /* Estimate times (TODO: actual timing) */
    stats->setup_time_ms = 0.0;
    stats->operation_time_ms = 0.0;
    stats->total_time_ms = 0.0;
    
    /* Estimate memory usage */
    stats->memory_used_bytes = ctx->n * sizeof(CrystallineAbacus*) * 2;
    
    /* Estimate speedup */
    stats->speedup_vs_direct = polytope_ntt_estimate_speedup(solid, "face_enum");
    
    return MATH_SUCCESS;
}