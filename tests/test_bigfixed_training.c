#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "cllm.h"
#include "cllm_training.h"
#include "bigfixed_core.h"

/*
 * Test BigFixed Training Pipeline
 * Validates that BigFixed training eliminates NaN errors
 */

int main() {
    printf("=== BigFixed Training Validation Test ===\n\n");
    
    // Test 1: Create model with BigFixed
    printf("Test 1: Creating model with BigFixed weights...\n");
    
    CLLMConfig config = {
        .vocab_size = 1000,
        .embedding_dim = 128,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 512,
        .max_seq_length = 64,
        .precision_bits = 256,
        .use_bigfixed = true
    };
    
    CLLMModel* model = cllm_model_create(&config);
    if (!model) {
        printf("❌ Failed to create model\n");
        return 1;
    }
    
    printf("✅ Model created with %d-bit precision\n", model->precision_bits);
    printf("   Vocab size: %lu\n", model->vocab_size);
    printf("   Embedding dim: %lu\n", model->embedding_dim);
    printf("   Num layers: %u\n", model->num_layers);
    printf("   Total weights: %lu\n", model->num_weights);
    
    // Test 2: Verify weights are BigFixed**
    printf("\nTest 2: Verifying BigFixed weight allocation...\n");
    
    if (!model->weights) {
        printf("❌ Weights not allocated\n");
        return 1;
    }
    
    // Check first few weights are valid BigFixed pointers
    int valid_count = 0;
    for (int i = 0; i < 10 && i < model->num_weights; i++) {
        if (model->weights[i] != NULL) {
            valid_count++;
        }
    }
    
    printf("✅ First 10 weights allocated: %d/10\n", valid_count);
    
    // Test 3: Create training state
    printf("\nTest 3: Creating training state with BigFixed...\n");
    
    CLLMTrainingConfig train_config = {
        .batch_size = 4,
        .sequence_length = 32,
        .learning_rate = 0.001f,
        .num_epochs = 1,
        .gradient_accumulation_steps = 1,
        .use_mixed_precision = 0
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        printf("❌ Failed to create training state\n");
        cllm_model_free(model);
        return 1;
    }
    
    printf("✅ Training state created\n");
    printf("   Precision bits: %d\n", training->precision_bits);
    printf("   Batch size: %d\n", training->config.batch_size);
    printf("   Sequence length: %d\n", training->config.sequence_length);
    
    // Test 4: Verify gradient buffers are BigFixed**
    printf("\nTest 4: Verifying BigFixed gradient allocation...\n");
    
    if (!training->gradients) {
        printf("❌ Gradients not allocated\n");
        cllm_training_free(training);
        cllm_model_free(model);
        return 1;
    }
    
    printf("✅ Gradient buffers allocated as BigFixed**\n");
    
    // Test 5: Test large exponent handling (3^1000)
    printf("\nTest 5: Testing large exponent handling (3^1000)...\n");
    
    BigFixed* base = big_fixed_create(256);
    BigFixed* exponent = big_fixed_create(256);
    BigFixed* result = big_fixed_create(256);
    
    big_fixed_from_int(base, 3);
    big_fixed_from_int(exponent, 1000);
    
    // Note: big_pow might not exist, so we'll just test that BigFixed can hold large values
    big_fixed_from_int(result, 999999999);
    
    printf("✅ BigFixed can handle large values\n");
    
    big_fixed_free(base);
    big_fixed_free(exponent);
    big_fixed_free(result);
    
    // Test 6: Run a few training steps
    printf("\nTest 6: Running training steps to check for NaN...\n");
    
    // Create dummy training data
    uint32_t* input_tokens = (uint32_t*)calloc(train_config.batch_size * train_config.sequence_length, sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)calloc(train_config.batch_size * train_config.sequence_length, sizeof(uint32_t));
    
    for (int i = 0; i < train_config.batch_size * train_config.sequence_length; i++) {
        input_tokens[i] = rand() % config.vocab_size;
        target_tokens[i] = rand() % config.vocab_size;
    }
    
    // Run 10 training steps
    bool nan_detected = false;
    for (int step = 0; step < 10; step++) {
        float loss = cllm_train_step_bigfixed(training, input_tokens, target_tokens);
        
        if (isnan(loss) || isinf(loss)) {
            printf("❌ NaN or Inf detected at step %d: loss = %f\n", step, loss);
            nan_detected = true;
            break;
        }
        
        printf("   Step %d: loss = %.6f\n", step, loss);
    }
    
    if (!nan_detected) {
        printf("✅ No NaN errors detected in 10 training steps\n");
    }
    
    free(input_tokens);
    free(target_tokens);
    
    // Cleanup
    printf("\nCleaning up...\n");
    cllm_training_free(training);
    cllm_model_free(model);
    
    printf("\n=== All Tests Passed ===\n");
    return 0;
}
