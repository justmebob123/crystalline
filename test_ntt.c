/*
 * test_ntt.c - Test Number Theoretic Transform Implementation
 * 
 * Tests NTT-based multiplication and compares with standard multiplication.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/bigint_core.h"
#include "include/bigint_ntt.h"
#include "include/bigint_advanced.h"

void print_test_header(const char* test_name) {
    printf("\n");
    printf("================================================================================\n");
    printf("TEST: %s\n", test_name);
    printf("================================================================================\n");
}

void print_test_result(const char* test_name, int passed) {
    if (passed) {
        printf("✓ %s: PASSED\n", test_name);
    } else {
        printf("✗ %s: FAILED\n", test_name);
    }
}

/**
 * Test 1: Basic NTT initialization
 */
int test_ntt_init() {
    print_test_header("NTT Initialization");
    
    NTTContext ctx;
    int result = ntt_init(&ctx, 8);
    
    if (!result) {
        printf("Failed to initialize NTT context\n");
        return 0;
    }
    
    printf("NTT context initialized successfully\n");
    printf("Transform size: %zu\n", ctx.n);
    printf("Prime modulus: ");
    char* prime_str = big_to_string(&ctx.prime);
    printf("%s\n", prime_str);
    free(prime_str);
    
    printf("Primitive root: ");
    char* root_str = big_to_string(&ctx.root);
    printf("%s\n", root_str);
    free(root_str);
    
    ntt_free(&ctx);
    
    return 1;
}

/**
 * Test 2: Forward and inverse NTT
 */
int test_ntt_transform() {
    print_test_header("NTT Forward/Inverse Transform");
    
    size_t n = 8;
    NTTContext ctx;
    
    if (!ntt_init(&ctx, n)) {
        printf("Failed to initialize NTT context\n");
        return 0;
    }
    
    // Create test input
    BigInt* input = malloc(n * sizeof(BigInt));
    BigInt* transformed = malloc(n * sizeof(BigInt));
    BigInt* recovered = malloc(n * sizeof(BigInt));
    
    printf("Input sequence: ");
    for (size_t i = 0; i < n; i++) {
        big_init(&input[i]);
        big_from_int(&input[i], i + 1);  // [1, 2, 3, 4, 5, 6, 7, 8]
        printf("%zu ", i + 1);
    }
    printf("\n");
    
    // Forward NTT
    ntt_forward(&ctx, transformed, input, n);
    
    printf("After forward NTT: ");
    for (size_t i = 0; i < n; i++) {
        char* str = big_to_string(&transformed[i]);
        printf("%s ", str);
        free(str);
    }
    printf("\n");
    
    // Inverse NTT
    ntt_inverse(&ctx, recovered, transformed, n);
    
    printf("After inverse NTT: ");
    int all_match = 1;
    for (size_t i = 0; i < n; i++) {
        char* str = big_to_string(&recovered[i]);
        printf("%s ", str);
        free(str);
        
        if (big_cmp(&recovered[i], &input[i]) != 0) {
            all_match = 0;
        }
    }
    printf("\n");
    
    // Cleanup
    for (size_t i = 0; i < n; i++) {
        big_free(&input[i]);
        big_free(&transformed[i]);
        big_free(&recovered[i]);
    }
    free(input);
    free(transformed);
    free(recovered);
    
    ntt_free(&ctx);
    
    if (all_match) {
        printf("✓ Forward/Inverse NTT correctly recovers input\n");
    } else {
        printf("✗ Forward/Inverse NTT failed to recover input\n");
    }
    
    return all_match;
}

/**
 * Test 3: NTT-based multiplication correctness
 */
int test_ntt_multiplication() {
    print_test_header("NTT-Based Multiplication Correctness");
    
    // Test with small numbers first
    BigInt a, b, result_standard, result_ntt;
    big_init(&a);
    big_init(&b);
    big_init(&result_standard);
    big_init(&result_ntt);
    
    // Test case 1: 123 × 456
    big_from_int(&a, 123);
    big_from_int(&b, 456);
    
    printf("Test: 123 × 456\n");
    
    // Standard multiplication
    big_mul(&result_standard, &a, &b);
    char* std_str = big_to_string(&result_standard);
    printf("Standard result: %s\n", std_str);
    free(std_str);
    
    // NTT multiplication
    if (big_ntt_multiply(&result_ntt, &a, &b)) {
        char* ntt_str = big_to_string(&result_ntt);
        printf("NTT result: %s\n", ntt_str);
        free(ntt_str);
        
        int match = (big_cmp(&result_standard, &result_ntt) == 0);
        if (match) {
            printf("✓ Results match!\n");
        } else {
            printf("✗ Results don't match!\n");
        }
        
        big_free(&a);
        big_free(&b);
        big_free(&result_standard);
        big_free(&result_ntt);
        
        return match;
    } else {
        printf("✗ NTT multiplication failed\n");
        big_free(&a);
        big_free(&b);
        big_free(&result_standard);
        big_free(&result_ntt);
        return 0;
    }
}

/**
 * Test 4: Performance benchmark
 */
int test_ntt_performance() {
    print_test_header("NTT Performance Benchmark");
    
    // Create large numbers for benchmarking
    BigInt a, b, result;
    big_init(&a);
    big_init(&b);
    big_init(&result);
    
    // Create numbers with many digits
    printf("Creating large test numbers...\n");
    
    // Build a number with 100 digits (approximately)
    for (int i = 0; i < 100; i++) {
        BigInt temp, ten;
        big_init(&temp);
        big_init(&ten);
        big_from_int(&ten, 10);
        
        big_mul(&temp, &a, &ten);
        big_copy(&a, &temp);
        
        BigInt digit;
        big_init(&digit);
        big_from_int(&digit, (i % 9) + 1);
        big_add(&a, &a, &digit);
        
        big_free(&temp);
        big_free(&ten);
        big_free(&digit);
    }
    
    big_copy(&b, &a);
    
    printf("Number size: %zu digits (uint32_t)\n", a.len);
    
    // Benchmark standard multiplication
    clock_t start = clock();
    big_mul(&result, &a, &b);
    clock_t end = clock();
    double time_standard = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    printf("Standard multiplication: %.3f ms\n", time_standard);
    
    // Benchmark NTT multiplication
    big_from_int(&result, 0);  // Reset result
    
    start = clock();
    int ntt_success = big_ntt_multiply(&result, &a, &b);
    end = clock();
    double time_ntt = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    if (ntt_success) {
        printf("NTT multiplication: %.3f ms\n", time_ntt);
        
        if (time_standard > 0) {
            double speedup = time_standard / time_ntt;
            printf("Speedup: %.2fx\n", speedup);
        }
    } else {
        printf("NTT multiplication failed\n");
    }
    
    big_free(&a);
    big_free(&b);
    big_free(&result);
    
    return ntt_success;
}

/**
 * Main test runner
 */
int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                   NTT (Number Theoretic Transform) Tests                  ║\n");
    printf("║                  Crystalline Lattice Math Library                          ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Run tests
    total_tests++;
    if (test_ntt_init()) passed_tests++;
    
    total_tests++;
    if (test_ntt_transform()) passed_tests++;
    
    total_tests++;
    if (test_ntt_multiplication()) passed_tests++;
    
    total_tests++;
    if (test_ntt_performance()) passed_tests++;
    
    // Summary
    printf("\n");
    printf("================================================================================\n");
    printf("TEST SUMMARY\n");
    printf("================================================================================\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    printf("Success rate: %.1f%%\n", (passed_tests * 100.0) / total_tests);
    printf("================================================================================\n");
    
    return (passed_tests == total_tests) ? 0 : 1;
}