// Test program to reproduce threading deadlock
#include <stdio.h>
#include <stdlib.h>
#include "cllm.h"
#include "cllm_training.h"

int main() {
    printf("=== Testing Threading Deadlock ===\n\n");
    
    // Create minimal training configuration
    CLLMTrainingConfig config = {
        .vocab_size = 100,
        .embed_dim = 32,
        .num_layers = 2,
        .num_heads = 2,
        .batch_size = 2,
        .sequence_length = 16,
        .learning_rate = 0.0001,
        .num_epochs = 3,  // Test with 3 epochs to trigger deadlock
        .checkpoint_interval = 0
    };
    
    // Create training system
    printf("Creating training system...\n");
    CLLMTraining* training = cllm_training_create(&config);
    if (!training) {
        fprintf(stderr, "Failed to create training\n");
        return 1;
    }
    
    // Create minimal dataset
    printf("Creating dataset...\n");
    uint32_t tokens[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    int num_tokens = 16;
    
    // Train for 3 epochs
    printf("\nStarting training (3 epochs)...\n");
    for (int epoch = 0; epoch < 3; epoch++) {
        printf("\n--- Epoch %d ---\n", epoch + 1);
        
        // This should hang on epoch 2
        float loss = cllm_train_epoch(training);
        
        printf("Epoch %d complete: loss = %.4f\n", epoch + 1, loss);
    }
    
    printf("\n=== Test Complete ===\n");
    printf("If you see this, the deadlock is fixed!\n");
    
    cllm_training_free(training);
    return 0;
}