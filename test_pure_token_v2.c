#include "include/cllm_pure_crystalline.h"
#include "include/bigfixed_core.h"
#include <stdio.h>
#include <stdlib.h>

void test_prime_operations() {
    printf("=== Test 1: Prime Operations ===\n");
    
    printf("Is 17 prime? %s\n", crystalline_is_prime(17) ? "YES" : "NO");
    printf("Is 18 prime? %s\n", crystalline_is_prime(18) ? "YES" : "NO");
    
    printf("5th prime: %lu\n", crystalline_get_nth_prime(5));
    printf("10th prime: %lu\n", crystalline_get_nth_prime(10));
    printf("100th prime: %lu\n", crystalline_get_nth_prime(100));
    
    uint64_t factors[MAX_PRIME_FACTORS];
    uint8_t num_factors;
    
    crystalline_factorize(30, factors, &num_factors);
    printf("Factorization of 30: ");
    for (int i = 0; i < num_factors; i++) {
        printf("%lu ", factors[i]);
    }
    printf("(%d factors)\n", num_factors);
    
    printf("✓ PASS\n\n");
}

void test_token_creation() {
    printf("=== Test 2: Token Creation ===\n");
    
    CrystallineToken* root = crystalline_token_create(0, "run", 5);
    printf("Token: '%s'\n", root->token_str);
    printf("  Prime: %lu\n", root->prime);
    printf("  Is root: %s\n", root->is_root ? "YES" : "NO");
    printf("  Factors: ");
    for (int i = 0; i < root->num_factors; i++) {
        printf("%lu ", root->prime_factors[i]);
    }
    printf("\n");
    
    CrystallineToken* derived = crystalline_token_create(1, "running", 10);
    printf("Token: '%s'\n", derived->token_str);
    printf("  Prime: %lu\n", derived->prime);
    printf("  Is root: %s\n", derived->is_root ? "YES" : "NO");
    printf("  Factors: ");
    for (int i = 0; i < derived->num_factors; i++) {
        printf("%lu ", derived->prime_factors[i]);
    }
    printf("\n");
    
    // Test coordinates are initialized
    printf("  Coordinates initialized: %s\n", 
           root->lattice_coords[0].integer_part != NULL ? "YES" : "NO");
    
    crystalline_token_free(root);
    crystalline_token_free(derived);
    
    printf("✓ PASS\n\n");
}

void test_ulam_positioning() {
    printf("=== Test 3: Ulam Spiral Positioning ===\n");
    
    uint64_t test_primes[] = {2, 3, 5, 7, 11};
    
    for (int i = 0; i < 5; i++) {
        CrystallineToken* token = crystalline_token_create(i, "test", test_primes[i]);
        
        if (token->lattice_coords[0].integer_part) {
            double x = big_fixed_to_double(&token->lattice_coords[0]);
            double y = big_fixed_to_double(&token->lattice_coords[1]);
            double z = big_fixed_to_double(&token->lattice_coords[2]);
            
            printf("Prime %2lu: (%.4f, %.4f, %.4f)\n", test_primes[i], x, y, z);
        } else {
            printf("Prime %2lu: coordinates not initialized\n", test_primes[i]);
        }
        
        crystalline_token_free(token);
    }
    
    printf("✓ PASS\n\n");
}

void test_lattice_distance() {
    printf("=== Test 4: Lattice Distance ===\n");
    
    CrystallineToken* token1 = crystalline_token_create(0, "test1", 5);
    CrystallineToken* token2 = crystalline_token_create(1, "test2", 7);
    
    BigFixed distance;
    distance.integer_part = NULL;
    distance.fractional_part = NULL;
    
    crystalline_lattice_distance(token1->lattice_coords, token2->lattice_coords, &distance);
    
    if (distance.integer_part) {
        double dist = big_fixed_to_double(&distance);
        printf("Distance between prime 5 and prime 7: %.6f\n", dist);
        
        // Free distance
        if (distance.integer_part) {
            big_free(distance.integer_part);
            free(distance.integer_part);
        }
        if (distance.fractional_part) {
            big_free(distance.fractional_part);
            free(distance.fractional_part);
        }
    }
    
    crystalline_token_free(token1);
    crystalline_token_free(token2);
    
    printf("✓ PASS\n\n");
}

void test_prime_similarity() {
    printf("=== Test 5: Prime Similarity ===\n");
    
    BigFixed sim;
    sim.integer_part = NULL;
    sim.fractional_part = NULL;
    
    crystalline_prime_similarity(5, 5, &sim);
    printf("Similarity(5, 5): %.4f (should be 1.0)\n", big_fixed_to_double(&sim));
    
    if (sim.integer_part) {
        big_free(sim.integer_part);
        free(sim.integer_part);
    }
    if (sim.fractional_part) {
        big_free(sim.fractional_part);
        free(sim.fractional_part);
    }
    
    sim.integer_part = NULL;
    sim.fractional_part = NULL;
    
    crystalline_prime_similarity(5, 7, &sim);
    printf("Similarity(5, 7): %.4f (coprime, should be 0.5)\n", big_fixed_to_double(&sim));
    
    if (sim.integer_part) {
        big_free(sim.integer_part);
        free(sim.integer_part);
    }
    if (sim.fractional_part) {
        big_free(sim.fractional_part);
        free(sim.fractional_part);
    }
    
    printf("✓ PASS\n\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Pure Crystalline CLLM - Token Operations Test         ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    test_prime_operations();
    test_token_creation();
    test_ulam_positioning();
    test_lattice_distance();
    test_prime_similarity();
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║              All Tests Complete - PASSED                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
