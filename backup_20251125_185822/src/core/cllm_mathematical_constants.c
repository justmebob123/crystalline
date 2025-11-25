#include "cllm_mathematical_constants.h"
#include "prime_math_custom.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// DIMENSIONAL FREQUENCIES
// ============================================================================

const uint64_t DIMENSIONAL_FREQUENCIES[] = {
    3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 37,
    41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89
};

const size_t DIMENSIONAL_FREQUENCIES_COUNT = 
    sizeof(DIMENSIONAL_FREQUENCIES) / sizeof(DIMENSIONAL_FREQUENCIES[0]);

// ============================================================================
// CYMATIC FREQUENCIES
// ============================================================================

const double CYMATIC_FREQUENCIES[] = {
    174.0,  // Pain reduction
    285.0,  // Tissue regeneration
    396.0,  // Liberation from fear
    417.0,  // Transformation
    432.0,  // Universal frequency
    528.0,  // DNA repair
    639.0,  // Connection
    741.0,  // Awakening
    852.0,  // Intuition
    963.0   // Divine connection
};

const size_t CYMATIC_FREQUENCIES_COUNT = 
    sizeof(CYMATIC_FREQUENCIES) / sizeof(CYMATIC_FREQUENCIES[0]);

// ============================================================================
// PLIMPTON 322 TRIPLES
// ============================================================================

const PlimptonTriple PLIMPTON_322_TRIPLES[] = {
    {119, 120, 169},
    {3367, 3456, 4825},
    {4601, 4800, 6649},
    {12709, 13500, 18541},
    {65, 72, 97},
    {319, 360, 481},
    {2291, 2700, 3541},
    {799, 960, 1249},
    {481, 600, 769},
    {4961, 6480, 8161},
    {45, 60, 75},
    {1679, 2400, 2929},
    {161, 240, 289},
    {1771, 2700, 3229},
    {56, 90, 106}
};

const size_t PLIMPTON_322_TRIPLES_COUNT = 
    sizeof(PLIMPTON_322_TRIPLES) / sizeof(PLIMPTON_322_TRIPLES[0]);

// ============================================================================
// BIGFIXED INITIALIZATION
// ============================================================================

void cllm_init_mathematical_constants(MathematicalConstantsBigFixed* constants) {
    if (!constants) return;
    
    // Initialize Einstein's Lambda: Λ = 3/144000
    constants->einstein_lambda = big_fixed_create(128);
    big_fixed_from_int(constants->einstein_lambda, EINSTEIN_LAMBDA_NUMERATOR);
    BigFixed* denominator = big_fixed_create(128);
    big_fixed_from_int(denominator, EINSTEIN_LAMBDA_DENOMINATOR);
    big_fixed_div(constants->einstein_lambda, constants->einstein_lambda, denominator);
    big_fixed_free(denominator);
    
    // Initialize Golden Ratio: φ = (1+√5)/2
    constants->golden_ratio = big_fixed_create(128);
    big_fixed_from_double(constants->golden_ratio, GOLDEN_RATIO);
    
    // Initialize ln(3)
    constants->ln_3 = big_fixed_create(128);
    big_fixed_from_double(constants->ln_3, LN_3);
    
    // Initialize π
    constants->pi = big_fixed_create(128);
    big_fixed_from_double(constants->pi, PRIME_PI);
    
    // Initialize 2π
    constants->two_pi = big_fixed_create(128);
    big_fixed_from_double(constants->two_pi, 2.0 * PRIME_PI);
    
    // Initialize π/12
    constants->pi_over_12 = big_fixed_create(128);
    big_fixed_from_double(constants->pi_over_12, PRIME_PI / 12.0);
    
    // Initialize 2π/(12·ln3)
    constants->two_pi_over_12_ln3 = big_fixed_create(128);
    big_fixed_from_double(constants->two_pi_over_12_ln3, (2.0 * PRIME_PI) / (12.0 * LN_3));
}

void cllm_free_mathematical_constants(MathematicalConstantsBigFixed* constants) {
    if (!constants) return;
    
    big_fixed_free(constants->einstein_lambda);
    big_fixed_free(constants->golden_ratio);
    big_fixed_free(constants->ln_3);
    big_fixed_free(constants->pi);
    big_fixed_free(constants->two_pi);
    big_fixed_free(constants->pi_over_12);
    big_fixed_free(constants->two_pi_over_12_ln3);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

int cllm_is_near_144000_boundary(uint64_t prime) {
    // Define "near" as within 100 of the boundary
    const uint64_t boundary_range = 100;
    
    if (prime >= VECTOR_CULMINATION - boundary_range &&
        prime <= VECTOR_CULMINATION + boundary_range) {
        return 1;
    }
    
    // Also check if it's one of the twin primes
    if (prime == TWIN_PRIME_LOWER || prime == TWIN_PRIME_UPPER) {
        return 1;
    }
    
    return 0;
}

uint64_t cllm_get_dimensional_frequency(size_t dimension) {
    if (dimension >= DIMENSIONAL_FREQUENCIES_COUNT) {
        // For dimensions beyond our table, return a reasonable default
        // In practice, this should be extended or computed
        return DIMENSIONAL_FREQUENCIES[DIMENSIONAL_FREQUENCIES_COUNT - 1];
    }
    return DIMENSIONAL_FREQUENCIES[dimension];
}

uint64_t cllm_product_dimensional_frequencies(size_t max_dimension) {
    uint64_t product = 1;
    
    for (size_t i = 0; i <= max_dimension && i < DIMENSIONAL_FREQUENCIES_COUNT; i++) {
        // Check for overflow
        if (product > UINT64_MAX / DIMENSIONAL_FREQUENCIES[i]) {
            fprintf(stderr, "Warning: Dimensional frequency product overflow at dimension %zu\n", i);
            return UINT64_MAX;
        }
        product *= DIMENSIONAL_FREQUENCIES[i];
    }
    
    return product;
}

int cllm_validate_mathematical_constants(void) {
    int valid = 1;
    
    // Validate Vector Culmination
    if (VECTOR_CULMINATION != 144000) {
        fprintf(stderr, "ERROR: VECTOR_CULMINATION corrupted! Expected 144000, got %llu\n",
                (unsigned long long)VECTOR_CULMINATION);
        valid = 0;
    }
    
    // Validate Twin Primes
    if (TWIN_PRIME_LOWER != 143999) {
        fprintf(stderr, "ERROR: TWIN_PRIME_LOWER corrupted! Expected 143999, got %llu\n",
                (unsigned long long)TWIN_PRIME_LOWER);
        valid = 0;
    }
    
    if (TWIN_PRIME_UPPER != 144001) {
        fprintf(stderr, "ERROR: TWIN_PRIME_UPPER corrupted! Expected 144001, got %llu\n",
                (unsigned long long)TWIN_PRIME_UPPER);
        valid = 0;
    }
    
    // Validate Einstein's Lambda
    double lambda = (double)EINSTEIN_LAMBDA_NUMERATOR / (double)EINSTEIN_LAMBDA_DENOMINATOR;
    double expected_lambda = 3.0 / 144000.0;
    double lambda_error = prime_fabs(lambda - expected_lambda);
    
    if (lambda_error > 1e-10) {
        fprintf(stderr, "ERROR: Einstein's Lambda corrupted! Expected %.15f, got %.15f\n",
                expected_lambda, lambda);
        valid = 0;
    }
    
    // Validate Symmetry Group Count
    if (SYMMETRY_GROUP_COUNT != 12) {
        fprintf(stderr, "ERROR: SYMMETRY_GROUP_COUNT corrupted! Expected 12, got %d\n",
                SYMMETRY_GROUP_COUNT);
        valid = 0;
    }
    
    // Validate Recursive Base
    if (RECURSIVE_BASE != 3) {
        fprintf(stderr, "ERROR: RECURSIVE_BASE corrupted! Expected 3, got %d\n",
                RECURSIVE_BASE);
        valid = 0;
    }
    
    // Validate Kissing Spheres Count
    if (KISSING_SPHERES_COUNT != 12) {
        fprintf(stderr, "ERROR: KISSING_SPHERES_COUNT corrupted! Expected 12, got %d\n",
                KISSING_SPHERES_COUNT);
        valid = 0;
    }
    
    // Validate first few dimensional frequencies
    if (DIMENSIONAL_FREQUENCIES[0] != 3 ||
        DIMENSIONAL_FREQUENCIES[1] != 7 ||
        DIMENSIONAL_FREQUENCIES[2] != 31) {
        fprintf(stderr, "ERROR: DIMENSIONAL_FREQUENCIES corrupted!\n");
        valid = 0;
    }
    
    if (valid) {
        printf("✓ All mathematical constants validated successfully\n");
    } else {
        fprintf(stderr, "✗ Mathematical constant validation FAILED\n");
    }
    
    return valid;
}