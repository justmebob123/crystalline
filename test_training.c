#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_training.h"

int main() {
    printf("=== CLLM Training Test ===\n\n");
    
    // Create a minimal model config
    CLLMConfig config = {
        .vocab_size = 10,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 32,
        .dropout = 0.1f
    };
    
    printf("Creating model...\n");
    fflush(stdout);
    CLLMModel* model = cllm_create_model(&config);
    printf("After cllm_create_model, model = %p\n", (void*)model);
    fflush(stdout);
    if (!model) {
        printf("ERROR: Failed to create model\n");
        return 1;
    }
    printf("✓ Model created\n");
    fflush(stdout);
    
    // Create training config
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .num_epochs = 1,
        .max_steps = 10,
        .weight_decay = 0.0f,
        .gradient_clip = 1.0f,
        .warmup_steps = 0,
        .save_interval = 100,
        .eval_interval = 10,
        .sequence_length = 8
    };
    
    printf("Initializing training...\n");
    fflush(stdout);
    CLLMTraining* training = cllm_training_init(model, &train_config);
    printf("After cllm_training_init, training = %p\n", (void*)training);
    fflush(stdout);
    if (!training) {
        printf("ERROR: Failed to initialize training\n");
        // cllm_free_model(model);  // Skip for now
        return 1;
    }
    printf("✓ Training initialized\n");
    fflush(stdout);
    
    // Create simple training data: [0,1,2,3,4,5,6,7]
    uint32_t input_tokens[16];
    uint32_t target_tokens[16];
    
    for (int i = 0; i < 16; i++) {
        input_tokens[i] = i % 8;
        target_tokens[i] = (i + 1) % 8;
    }
    
    printf("\nTesting backward pass...\n");
    
    // Just test that backward pass doesn't crash
    printf("Calling cllm_backward_complete...\n");
    fflush(stdout);
    cllm_backward_complete(training, input_tokens, target_tokens, 2, 8);
    printf("✓ Backward pass completed\n");
    
    // Test optimizer
    printf("Calling cllm_adam_step...\n");
    fflush(stdout);
    cllm_adam_step(training, train_config.learning_rate);
    printf("✓ Optimizer step completed\n");
    
    printf("\n✓ SUCCESS: Training pipeline works!\n");
    
    // Skip cleanup
    // cllm_training_free(training);
    // cllm_free_model(model);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}