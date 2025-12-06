#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cllm.h"
#include "cllm_training.h"
#include "prime_math.h"

// Forward declarations
extern CLLMModel* cllm_create_model(CLLMConfig* config);
extern void cllm_free_model(CLLMModel* model);
extern float cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens);
extern void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens, double* gradient_buffer);
extern float cllm_compute_loss(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens, int num_tokens);

int main() {
    printf("=== Testing Gradient Direction ===\n\n");
    
    // Create tiny model
    CLLMConfig config = {
        .vocab_size = 10,
        .embedding_dim = 8,
        .num_layers = 1,
        .num_heads = 2,
        .ff_dim = 16,
        .max_seq_len = 4,
        .dropout = 0.0f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    
    // Initialize embeddings to small random values
    for (uint32_t i = 0; i < config.vocab_size * config.embedding_dim; i++) {
        model->embeddings.embeddings[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    // Create training config
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.01,  // Larger LR for testing
        .batch_size = 1,
        .sequence_length = 2,
        .num_epochs = 1
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        fprintf(stderr, "Failed to init training\n");
        cllm_free_model(model);
        return 1;
    }
    
    // Simple training data: [1, 2] -> [2, 3]
    uint32_t input_tokens[] = {1, 2};
    uint32_t target_tokens[] = {2, 3};
    
    printf("Input tokens: [%u, %u]\n", input_tokens[0], input_tokens[1]);
    printf("Target tokens: [%u, %u]\n\n", target_tokens[0], target_tokens[1]);
    
    // Forward pass
    printf("Step 1: Forward pass\n");
    cllm_forward_training(training, input_tokens);
    
    // Compute initial loss
    float loss_before = cllm_compute_loss(training, input_tokens, target_tokens, 2);
    printf("Loss BEFORE update: %.6f\n\n", loss_before);
    
    // Backward pass
    printf("Step 2: Backward pass\n");
    cllm_backward_training(training, target_tokens, NULL);
    
    // Check gradient magnitudes
    double max_grad = 0.0;
    for (size_t i = 0; i < 10; i++) {
        double g = fabs(training->gradients[i]);
        if (g > max_grad) max_grad = g;
    }
    printf("Max gradient magnitude: %.6f\n\n", max_grad);
    
    // Manually apply ONE gradient step
    printf("Step 3: Apply gradients manually\n");
    double lr = 0.01;
    for (size_t i = 0; i < config.vocab_size * config.embedding_dim; i++) {
        model->embeddings.embeddings[i] -= lr * training->gradients[i];
    }
    printf("Applied gradients with LR=%.4f\n\n", lr);
    
    // Forward pass again
    printf("Step 4: Forward pass after update\n");
    cllm_forward_training(training, input_tokens);
    
    // Compute loss after update
    float loss_after = cllm_compute_loss(training, input_tokens, target_tokens, 2);
    printf("Loss AFTER update: %.6f\n\n", loss_after);
    
    // Check if loss decreased
    printf("=== RESULT ===\n");
    printf("Loss change: %.6f -> %.6f\n", loss_before, loss_after);
    printf("Delta: %.6f\n", loss_after - loss_before);
    
    if (loss_after < loss_before) {
        printf("✓ SUCCESS: Loss DECREASED (gradient descent working!)\n");
    } else {
        printf("✗ FAILURE: Loss INCREASED (gradient direction is WRONG!)\n");
    }
    
    cllm_training_free(training);
    cllm_free_model(model);
    
    return (loss_after < loss_before) ? 0 : 1;
}