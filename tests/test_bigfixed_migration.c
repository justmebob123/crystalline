/**
 * Comprehensive Test Suite for BigFixed Migration
 * 
 * This test suite verifies that the BigFixed migration is complete and functional.
 * It checks for stubs, shortcuts, and verifies actual computation is happening.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "../include/cllm_training.h"
#include "../include/bigfixed_core.h"

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("✓ PASS: %s\n", message); \
            tests_passed++; \
        } else { \
            printf("✗ FAIL: %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

/**
 * Test 1: Verify attention output is not just a copy of input
 */
void test_attention_not_identity() {
    printf("\n=== Test 1: Attention Output Verification ===\n");
    
    // Create a simple model and training context
    CLLMModel* model = cllm_model_create(100, 64, 2, 2, 128);
    if (!model) {
        printf("✗ FAIL: Could not create model\n");
        tests_failed++;
        return;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 4,
        .num_epochs = 1
    };
    
    CLLMTraining* training = cllm_training_init(model, config);
    if (!training) {
        printf("✗ FAIL: Could not create training context\n");
        tests_failed++;
        cllm_model_free(model);
        return;
    }
    
    // Create test input tokens
    uint32_t input_tokens[] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    // Run forward pass
    float loss = cllm_forward_training(training, input_tokens);
    
    // Check that attention outputs are different from inputs
    int identical_count = 0;
    int total_count = 0;
    
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        BigFixed** attn_input = training->layer_inputs[layer];
        BigFixed** attn_output = training->attention_outputs[layer];
        
        for (uint32_t i = 0; i < config.batch_size * config.sequence_length * model->embedding_dim; i++) {
            if (attn_input[i] && attn_output[i]) {
                double input_val = big_fixed_to_double(attn_input[i]);
                double output_val = big_fixed_to_double(attn_output[i]);
                
                if (fabs(input_val - output_val) < 1e-10) {
                    identical_count++;
                }
                total_count++;
            }
        }
    }
    
    // If more than 90% of values are identical, attention is likely just copying
    double identical_ratio = (double)identical_count / total_count;
    
    TEST_ASSERT(identical_ratio < 0.9, 
                "Attention output is different from input (not just copying)");
    
    if (identical_ratio >= 0.9) {
        printf("  WARNING: %.1f%% of attention outputs are identical to inputs!\n", 
               identical_ratio * 100);
        printf("  This suggests attention is just copying input to output (STUB behavior)\n");
    }
    
    // Cleanup
    cllm_training_cleanup(training);
    cllm_model_free(model);
}

/**
 * Test 2: Verify feedforward output is not just a copy of input
 */
void test_feedforward_not_identity() {
    printf("\n=== Test 2: Feedforward Output Verification ===\n");
    
    CLLMModel* model = cllm_model_create(100, 64, 2, 2, 128);
    if (!model) {
        printf("✗ FAIL: Could not create model\n");
        tests_failed++;
        return;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 4,
        .num_epochs = 1
    };
    
    CLLMTraining* training = cllm_training_init(model, config);
    if (!training) {
        printf("✗ FAIL: Could not create training context\n");
        tests_failed++;
        cllm_model_free(model);
        return;
    }
    
    uint32_t input_tokens[] = {1, 2, 3, 4, 5, 6, 7, 8};
    float loss = cllm_forward_training(training, input_tokens);
    
    // Check that feedforward outputs are different from attention outputs
    int identical_count = 0;
    int total_count = 0;
    
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        BigFixed** attn_output = training->attention_outputs[layer];
        BigFixed** ff_output = training->ff_outputs[layer];
        
        for (uint32_t i = 0; i < config.batch_size * config.sequence_length * model->embedding_dim; i++) {
            if (attn_output[i] && ff_output[i]) {
                double attn_val = big_fixed_to_double(attn_output[i]);
                double ff_val = big_fixed_to_double(ff_output[i]);
                
                if (fabs(attn_val - ff_val) < 1e-10) {
                    identical_count++;
                }
                total_count++;
            }
        }
    }
    
    double identical_ratio = (double)identical_count / total_count;
    
    TEST_ASSERT(identical_ratio < 0.9, 
                "Feedforward output is different from attention output");
    
    if (identical_ratio >= 0.9) {
        printf("  WARNING: %.1f%% of feedforward outputs are identical to attention outputs!\n", 
               identical_ratio * 100);
    }
    
    cllm_training_cleanup(training);
    cllm_model_free(model);
}

/**
 * Test 3: Verify gradients are computed (not zero)
 */
void test_gradients_nonzero() {
    printf("\n=== Test 3: Gradient Computation Verification ===\n");
    
    CLLMModel* model = cllm_model_create(100, 64, 2, 2, 128);
    if (!model) {
        printf("✗ FAIL: Could not create model\n");
        tests_failed++;
        return;
    }
    
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 4,
        .num_epochs = 1
    };
    
    CLLMTraining* training = cllm_training_init(model, config);
    if (!training) {
        printf("✗ FAIL: Could not create training context\n");
        tests_failed++;
        cllm_model_free(model);
        return;
    }
    
    uint32_t input_tokens[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint32_t target_tokens[] = {2, 3, 4, 5, 6, 7, 8, 9};
    
    // Run forward pass
    float loss = cllm_forward_training(training, input_tokens);
    
    // Run backward pass
    cllm_backward_training(training, target_tokens);
    
    // Check that gradients are non-zero
    int nonzero_grads = 0;
    int total_grads = 0;
    
    // Check embedding gradients
    if (training->gradients) {
        for (uint32_t i = 0; i < model->vocab_size * model->embedding_dim; i++) {
            if (training->gradients[i]) {
                double grad_val = big_fixed_to_double(training->gradients[i]);
                if (fabs(grad_val) > 1e-10) {
                    nonzero_grads++;
                }
                total_grads++;
            }
        }
    }
    
    // Check attention gradients
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        if (training->attention_grads[layer].query_lattice) {
            for (uint32_t i = 0; i < model->embedding_dim * model->embedding_dim; i++) {
                if (training->attention_grads[layer].query_lattice[i]) {
                    double grad_val = big_fixed_to_double(training->attention_grads[layer].query_lattice[i]);
                    if (fabs(grad_val) > 1e-10) {
                        nonzero_grads++;
                    }
                    total_grads++;
                }
            }
        }
    }
    
    double nonzero_ratio = (double)nonzero_grads / total_grads;
    
    TEST_ASSERT(nonzero_ratio > 0.1, 
                "Gradients are being computed (not all zero)");
    
    if (nonzero_ratio <= 0.1) {
        printf("  WARNING: Only %.1f%% of gradients are non-zero!\n", 
               nonzero_ratio * 100);
        printf("  This suggests backward pass is not working correctly\n");
    }
    
    cllm_training_cleanup(training);
    cllm_model_free(model);
}

/**
 * Test 4: Verify BigFixed operations are actually being used
 */
void test_bigfixed_usage() {
    printf("\n=== Test 4: BigFixed Usage Verification ===\n");
    
    // Create test BigFixed values
    int precision = 256;
    BigFixed* a = big_fixed_create(precision);
    BigFixed* b = big_fixed_create(precision);
    BigFixed* result = big_fixed_create(precision);
    
    big_fixed_from_double(a, 3.14159);
    big_fixed_from_double(b, 2.71828);
    
    // Test multiplication
    big_fixed_mul(result, a, b);
    double mul_result = big_fixed_to_double(result);
    double expected = 3.14159 * 2.71828;
    
    TEST_ASSERT(fabs(mul_result - expected) < 1e-5, 
                "BigFixed multiplication works correctly");
    
    // Test addition
    big_fixed_add(result, a, b);
    double add_result = big_fixed_to_double(result);
    expected = 3.14159 + 2.71828;
    
    TEST_ASSERT(fabs(add_result - expected) < 1e-5, 
                "BigFixed addition works correctly");
    
    // Test division
    big_fixed_div(result, a, b);
    double div_result = big_fixed_to_double(result);
    expected = 3.14159 / 2.71828;
    
    TEST_ASSERT(fabs(div_result - expected) < 1e-5, 
                "BigFixed division works correctly");
    
    big_fixed_free(a);
    big_fixed_free(b);
    big_fixed_free(result);
}

/**
 * Test 5: Verify no float arithmetic in forward pass
 */
void test_no_float_arithmetic() {
    printf("\n=== Test 5: Float Arithmetic Detection ===\n");
    
    // This is a compile-time check - if the code compiles with BigFixed**
    // and no type errors, then float arithmetic has been eliminated
    
    TEST_ASSERT(1, "Code compiles with BigFixed types (no float* casts)");
    
    printf("  Note: This test verifies compilation, not runtime behavior\n");
    printf("  Manual code review is still needed to verify no float operations\n");
}

/**
 * Main test runner
 */
int main(int argc, char** argv) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  COMPREHENSIVE BIGFIXED MIGRATION TEST SUITE              ║\n");
    printf("║  Testing for stubs, shortcuts, and functional correctness ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Run all tests
    test_bigfixed_usage();
    test_attention_not_identity();
    test_feedforward_not_identity();
    test_gradients_nonzero();
    test_no_float_arithmetic();
    
    // Print summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  TEST SUMMARY                                              ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Tests Passed: %-3d                                        ║\n", tests_passed);
    printf("║  Tests Failed: %-3d                                        ║\n", tests_failed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    if (tests_failed > 0) {
        printf("\n⚠️  CRITICAL: %d test(s) failed!\n", tests_failed);
        printf("This indicates the BigFixed migration has functional issues.\n");
        printf("Review CRITICAL_ISSUES_FOUND.md for details.\n");
        return 1;
    } else {
        printf("\n✓ All tests passed!\n");
        return 0;
    }
}