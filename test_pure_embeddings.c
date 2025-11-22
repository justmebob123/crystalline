/*
 * Test suite for Pure Crystalline Embeddings
 */

#include "cllm_pure_crystalline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// Test colors
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

void print_test_result(const char* test_name, bool passed) {
    if (passed) {
        printf("%s✓ PASS%s: %s\n", GREEN, RESET, test_name);
    } else {
        printf("%s✗ FAIL%s: %s\n", RED, RESET, test_name);
    }
}

/*
 * Test 1: Embeddings Creation
 */
bool test_embeddings_creation() {
    printf("\n=== Test 1: Embeddings Creation ===\n");
    
    // Create embeddings with vocab_size=100, lattice_dim=3
    CrystallineEmbeddings* embeddings = crystalline_embeddings_create(100, 3);
    
    if (!embeddings) {
        printf("Failed to create embeddings\n");
        return false;
    }
    
    // Verify structure initialization
    assert(embeddings->vocab_size == 100);
    assert(embeddings->lattice_dim == 3);
    assert(embeddings->basis_optimized == false);
    assert(embeddings->optimization_epoch == 0);
    assert(embeddings->lattice_basis != NULL);
    assert(embeddings->inverse_basis != NULL);
    assert(embeddings->tokens != NULL);
    assert(embeddings->token_positions != NULL);
    assert(embeddings->token_primes != NULL);
    assert(embeddings->morphology_graph != NULL);
    assert(embeddings->morphology_counts != NULL);
    
    printf("Embeddings created successfully:\n");
    printf("  vocab_size: %u\n", embeddings->vocab_size);
    printf("  lattice_dim: %u\n", embeddings->lattice_dim);
    printf("  basis_optimized: %s\n", embeddings->basis_optimized ? "true" : "false");
    
    // Cleanup
    crystalline_embeddings_free(embeddings);
    
    return true;
}

/*
 * Test 2: Token Addition
 */
bool test_token_addition() {
    printf("\n=== Test 2: Token Addition ===\n");
    
    // Create embeddings
    CrystallineEmbeddings* embeddings = crystalline_embeddings_create(10, 3);
    if (!embeddings) return false;
    
    // Create tokens with primes 2,3,5,7,11
    uint64_t primes[] = {2, 3, 5, 7, 11};
    const char* words[] = {"the", "a", "is", "of", "to"};
    
    for (int i = 0; i < 5; i++) {
        CrystallineToken* token = crystalline_token_create(i, words[i], primes[i]);
        if (!token) {
            printf("Failed to create token %d\n", i);
            crystalline_embeddings_free(embeddings);
            return false;
        }
        
        bool added = crystalline_embeddings_add_token(embeddings, token);
        if (!added) {
            printf("Failed to add token %d\n", i);
            crystalline_token_free(token);
            crystalline_embeddings_free(embeddings);
            return false;
        }
        
        printf("Added token %d: '%s' (prime=%lu)\n", i, words[i], primes[i]);
    }
    
    // Verify tokens can be retrieved
    for (int i = 0; i < 5; i++) {
        CrystallineToken* token = crystalline_embeddings_get_token(embeddings, i);
        if (!token) {
            printf("Failed to retrieve token %d\n", i);
            crystalline_embeddings_free(embeddings);
            return false;
        }
        
        assert(token->token_id == (uint32_t)i);
        assert(token->prime == primes[i]);
        assert(strcmp(token->token_str, words[i]) == 0);
        
        printf("Retrieved token %d: '%s' (prime=%lu)\n", i, token->token_str, token->prime);
    }
    
    // Cleanup (tokens are freed by their owners, not by embeddings)
    for (int i = 0; i < 5; i++) {
        CrystallineToken* token = crystalline_embeddings_get_token(embeddings, i);
        crystalline_token_free(token);
    }
    crystalline_embeddings_free(embeddings);
    
    return true;
}

/*
 * Test 3: Basis Initialization
 */
bool test_basis_initialization() {
    printf("\n=== Test 3: Basis Initialization ===\n");
    
    // Create embeddings
    CrystallineEmbeddings* embeddings = crystalline_embeddings_create(10, 3);
    if (!embeddings) return false;
    
    // Initialize basis to identity
    bool initialized = crystalline_initialize_basis(embeddings);
    if (!initialized) {
        printf("Failed to initialize basis\n");
        crystalline_embeddings_free(embeddings);
        return false;
    }
    
    printf("Basis initialized to identity matrix\n");
    
    // Verify basis is identity matrix
    for (uint32_t i = 0; i < embeddings->lattice_dim; i++) {
        for (uint32_t j = 0; j < embeddings->lattice_dim; j++) {
            BigFixed* elem = &embeddings->lattice_basis[i][j];
            
            if (i == j) {
                // Diagonal should be 1.0
                double int_val = big_to_double(elem->integer_part);
                double frac_val = big_to_double(elem->fractional_part);
                if (int_val != 1 || frac_val != 0) {
                    printf("Diagonal element [%u][%u] is not 1.0\n", i, j);
                    crystalline_embeddings_free(embeddings);
                    return false;
                }
            } else {
                // Off-diagonal should be 0.0
                double int_val = big_to_double(elem->integer_part);
                double frac_val = big_to_double(elem->fractional_part);
                if (int_val != 0 || frac_val != 0) {
                    printf("Off-diagonal element [%u][%u] is not 0.0\n", i, j);
                    crystalline_embeddings_free(embeddings);
                    return false;
                }
            }
        }
    }
    
    printf("Basis matrix:\n");
    for (uint32_t i = 0; i < embeddings->lattice_dim; i++) {
        printf("  [");
        for (uint32_t j = 0; j < embeddings->lattice_dim; j++) {
            double val = big_to_double(embeddings->lattice_basis[i][j].integer_part);
            printf("%.0f", val);
            if (j < embeddings->lattice_dim - 1) printf(", ");
        }
        printf("]\n");
    }
    
    // Cleanup
    crystalline_embeddings_free(embeddings);
    
    return true;
}

/*
 * Test 4: Token Position Computation
 */
bool test_token_position() {
    printf("\n=== Test 4: Token Position Computation ===\n");
    
    // Create embeddings
    CrystallineEmbeddings* embeddings = crystalline_embeddings_create(10, 3);
    if (!embeddings) return false;
    
    // Create and add a token
    CrystallineToken* token = crystalline_token_create(0, "test", 5);
    if (!token) {
        crystalline_embeddings_free(embeddings);
        return false;
    }
    
    crystalline_embeddings_add_token(embeddings, token);
    
    // Compute position
    BigFixed position[3];
    for (int i = 0; i < 3; i++) {
        position[i].integer_part = (BigInt*)malloc(sizeof(BigInt));
        position[i].fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(position[i].integer_part);
        big_init(position[i].fractional_part);
        position[i].scale_bits = 256;
        position[i].negative = 0;
    }
    
    crystalline_compute_token_position(embeddings, 0, position);
    
    printf("Token position computed:\n");
    for (int i = 0; i < 3; i++) {
        double val = big_fixed_to_double(&position[i]);
        printf("  coord[%d] = %.6f\n", i, val);
    }
    
    // Verify position matches token's lattice_coords
    for (int i = 0; i < 3; i++) {
        double pos_val = big_fixed_to_double(&position[i]);
        double token_val = big_fixed_to_double(&token->lattice_coords[i]);
        
        if (fabs(pos_val - token_val) > 0.0001) {
            printf("Position mismatch at coord %d: %.6f vs %.6f\n", i, pos_val, token_val);
            // Cleanup
            for (int j = 0; j < 3; j++) {
                big_free(position[j].integer_part);
                free(position[j].integer_part);
                big_free(position[j].fractional_part);
                free(position[j].fractional_part);
            }
            crystalline_token_free(token);
            crystalline_embeddings_free(embeddings);
            return false;
        }
    }
    
    // Cleanup
    for (int i = 0; i < 3; i++) {
        big_free(position[i].integer_part);
        free(position[i].integer_part);
        big_free(position[i].fractional_part);
        free(position[i].fractional_part);
    }
    crystalline_token_free(token);
    crystalline_embeddings_free(embeddings);
    
    return true;
}

/*
 * Main test runner
 */
int main() {
    printf("===========================================\n");
    printf("Pure Crystalline Embeddings Test Suite\n");
    printf("Phase 2 - Day 1: Core Structures & Basic Operations\n");
    printf("===========================================\n");
    
    bool all_passed = true;
    
    // Run tests
    bool test1 = test_embeddings_creation();
    print_test_result("Embeddings Creation", test1);
    all_passed = all_passed && test1;
    
    bool test2 = test_token_addition();
    print_test_result("Token Addition", test2);
    all_passed = all_passed && test2;
    
    bool test3 = test_basis_initialization();
    print_test_result("Basis Initialization", test3);
    all_passed = all_passed && test3;
    
    bool test4 = test_token_position();
    print_test_result("Token Position Computation", test4);
    all_passed = all_passed && test4;
    
    // Summary
    printf("\n===========================================\n");
    if (all_passed) {
        printf("%s✓ ALL TESTS PASSED%s\n", GREEN, RESET);
        printf("===========================================\n");
        return 0;
    } else {
        printf("%s✗ SOME TESTS FAILED%s\n", RED, RESET);
        printf("===========================================\n");
        return 1;
    }
}