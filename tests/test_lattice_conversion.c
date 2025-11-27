/*
 * Test Lattice Conversion Utilities
 * 
 * Tests float <-> BigFixed conversion accuracy and performance.
 */

#include "include/cllm_lattice_conversion.h"
#include "include/bigfixed_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

void test_basic_conversion() {
    printf("=== Test 1: Basic Conversion ===\n");
    
    float input[] = {1.0f, 2.5f, -3.7f, 0.123456f, 999.999f};
    int n = 5;
    
    BigFixed* bigfixed = (BigFixed*)malloc(n * sizeof(BigFixed));
    float* output = (float*)malloc(n * sizeof(float));
    
    // Convert float -> BigFixed -> float
    cllm_float_to_bigfixed(bigfixed, input, n, 128);
    cllm_bigfixed_to_float(output, bigfixed, n);
    
    // Check accuracy
    printf("Input -> BigFixed -> Output:\n");
    for (int i = 0; i < n; i++) {
        float error = fabsf(output[i] - input[i]);
        printf("  %.6f -> %.6f (error: %.10e)\n", input[i], output[i], error);
    }
    
    float max_error = cllm_test_conversion_accuracy(input, n, 128);
    printf("Maximum error: %.10e\n", max_error);
    
    if (max_error < 1e-6f) {
        printf("✓ PASS: Excellent accuracy\n");
    } else {
        printf("✗ FAIL: Poor accuracy\n");
    }
    
    free(bigfixed);
    free(output);
    printf("\n");
}

void test_embedding_conversion() {
    printf("=== Test 2: Embedding Matrix Conversion ===\n");
    
    int vocab_size = 10;
    int embedding_dim = 8;
    
    // Create test embeddings
    float* embeddings = (float*)malloc(vocab_size * embedding_dim * sizeof(float));
    for (int i = 0; i < vocab_size * embedding_dim; i++) {
        embeddings[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    }
    
    // Allocate BigFixed basis
    BigFixed** basis = cllm_alloc_bigfixed_basis(vocab_size, embedding_dim);
    if (!basis) {
        printf("✗ FAIL: Could not allocate basis\n");
        free(embeddings);
        return;
    }
    
    // Convert embeddings -> basis -> embeddings
    cllm_embeddings_to_basis(basis, embeddings, vocab_size, embedding_dim, 128);
    
    float* output = (float*)malloc(vocab_size * embedding_dim * sizeof(float));
    cllm_basis_to_embeddings(output, basis, vocab_size, embedding_dim);
    
    // Check accuracy
    float max_error = 0.0f;
    for (int i = 0; i < vocab_size * embedding_dim; i++) {
        float error = fabsf(output[i] - embeddings[i]);
        if (error > max_error) {
            max_error = error;
        }
    }
    
    printf("Matrix size: %d x %d\n", vocab_size, embedding_dim);
    printf("Maximum error: %.10e\n", max_error);
    
    if (max_error < 1e-6f) {
        printf("✓ PASS: Excellent accuracy\n");
    } else {
        printf("✗ FAIL: Poor accuracy\n");
    }
    
    // Cleanup
    cllm_free_bigfixed_basis(basis, vocab_size);
    free(embeddings);
    free(output);
    printf("\n");
}

void test_conversion_performance() {
    printf("=== Test 3: Conversion Performance ===\n");
    
    int vocab_size = 1000;
    int embedding_dim = 128;
    int total_elements = vocab_size * embedding_dim;
    
    // Create test data
    float* embeddings = (float*)malloc(total_elements * sizeof(float));
    for (int i = 0; i < total_elements; i++) {
        embeddings[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    }
    
    BigFixed** basis = cllm_alloc_bigfixed_basis(vocab_size, embedding_dim);
    float* output = (float*)malloc(total_elements * sizeof(float));
    
    // Benchmark conversion
    clock_t start, end;
    
    // Float -> BigFixed
    start = clock();
    cllm_embeddings_to_basis(basis, embeddings, vocab_size, embedding_dim, 128);
    end = clock();
    double to_bigfixed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // BigFixed -> Float
    start = clock();
    cllm_basis_to_embeddings(output, basis, vocab_size, embedding_dim);
    end = clock();
    double to_float_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Matrix size: %d x %d (%d elements)\n", vocab_size, embedding_dim, total_elements);
    printf("Float -> BigFixed: %.6f seconds (%.2f M elements/sec)\n", 
           to_bigfixed_time, total_elements / to_bigfixed_time / 1e6);
    printf("BigFixed -> Float: %.6f seconds (%.2f M elements/sec)\n",
           to_float_time, total_elements / to_float_time / 1e6);
    printf("Total round-trip: %.6f seconds\n", to_bigfixed_time + to_float_time);
    
    // Check accuracy
    float max_error = 0.0f;
    for (int i = 0; i < total_elements; i++) {
        float error = fabsf(output[i] - embeddings[i]);
        if (error > max_error) {
            max_error = error;
        }
    }
    printf("Maximum error: %.10e\n", max_error);
    
    if (max_error < 1e-6f && to_bigfixed_time < 1.0 && to_float_time < 1.0) {
        printf("✓ PASS: Good performance and accuracy\n");
    } else {
        printf("⚠ WARNING: Check performance or accuracy\n");
    }
    
    // Cleanup
    cllm_free_bigfixed_basis(basis, vocab_size);
    free(embeddings);
    free(output);
    printf("\n");
}

void test_precision_levels() {
    printf("=== Test 4: Different Precision Levels ===\n");
    
    float input[] = {1.23456789f, 0.00001f, 999.999f, -0.123456f};
    int n = 4;
    
    int precisions[] = {32, 64, 128, 256};
    int num_precisions = 4;
    
    printf("Testing different precision levels:\n");
    for (int p = 0; p < num_precisions; p++) {
        int precision = precisions[p];
        float max_error = cllm_test_conversion_accuracy(input, n, precision);
        printf("  %3d bits: max error = %.10e\n", precision, max_error);
    }
    
    printf("✓ PASS: Precision test complete\n");
    printf("\n");
}

void test_edge_cases() {
    printf("=== Test 5: Edge Cases ===\n");
    
    float input[] = {0.0f, -0.0f, 1e-10f, 1e10f, -1e10f};
    int n = 5;
    
    BigFixed* bigfixed = (BigFixed*)malloc(n * sizeof(BigFixed));
    float* output = (float*)malloc(n * sizeof(float));
    
    cllm_float_to_bigfixed(bigfixed, input, n, 128);
    cllm_bigfixed_to_float(output, bigfixed, n);
    
    printf("Edge case conversions:\n");
    int pass = 1;
    for (int i = 0; i < n; i++) {
        float error = fabsf(output[i] - input[i]);
        printf("  %.10e -> %.10e (error: %.10e)\n", input[i], output[i], error);
        if (error > 1e-6f && fabsf(input[i]) > 1e-6f) {
            pass = 0;
        }
    }
    
    if (pass) {
        printf("✓ PASS: Edge cases handled correctly\n");
    } else {
        printf("⚠ WARNING: Some edge cases have high error\n");
    }
    
    free(bigfixed);
    free(output);
    printf("\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     CLLM Lattice Conversion Utilities Test Suite          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    srand(time(NULL));
    
    test_basic_conversion();
    test_embedding_conversion();
    test_conversion_performance();
    test_precision_levels();
    test_edge_cases();
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    All Tests Complete                      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}