#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cllm_training.h"

// Simple test to check if gradients are non-zero
int main() {
    printf("=== GRADIENT CHECK TEST ===\n\n");
    
    // Create minimal model
    CLLMConfig config = {
        .vocab_size = 100,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_hidden_dim = 256,
        .max_seq_length = 32,
        .dropout = 0.0f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    
    // Create training context
    CLLMTrainingConfig train_config = {
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .learning_rate = 0.001f,
        .save_every = 1
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        printf("Failed to create training context\n");
        cllm_free_model(model);
        return 1;
    }
    
    // Create simple training data
    uint32_t input_tokens[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t target_tokens[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    
    printf("Running forward pass...\n");
    float loss_before = cllm_forward_training(training, input_tokens);
    printf("Loss before: %.6f\n", loss_before);
    
    printf("\nRunning backward pass...\n");
    cllm_backward_training(training, target_tokens);
    
    // Check if gradients are non-zero
    printf("\nChecking gradients:\n");
    
    // Check embedding gradients
    int nonzero_embed = 0;
    float max_embed_grad = 0.0f;
    for (size_t i = 0; i < 100 * 64; i++) {
        if (training->gradients[i] != 0.0f) {
            nonzero_embed++;
            float abs_grad = fabsf(training->gradients[i]);
            if (abs_grad > max_embed_grad) max_embed_grad = abs_grad;
        }
    }
    printf("  Embedding gradients: %d non-zero (max: %.6f)\n", nonzero_embed, max_embed_grad);
    
    // Check attention gradients
    int nonzero_attn = 0;
    float max_attn_grad = 0.0f;
    for (uint32_t layer = 0; layer < 2; layer++) {
        size_t size = 64 * 64;
        for (size_t i = 0; i < size; i++) {
            if (training->attention_grads[layer].query_lattice[i] != 0.0f) {
                nonzero_attn++;
                float abs_grad = fabsf(training->attention_grads[layer].query_lattice[i]);
                if (abs_grad > max_attn_grad) max_attn_grad = abs_grad;
            }
        }
    }
    printf("  Attention gradients: %d non-zero (max: %.6f)\n", nonzero_attn, max_attn_grad);
    
    // Check FF gradients
    int nonzero_ff = 0;
    float max_ff_grad = 0.0f;
    for (uint32_t layer = 0; layer < 2; layer++) {
        size_t size = 64 * 256;
        for (size_t i = 0; i < size; i++) {
            if (training->ff_grads[layer].w1_lattice[i] != 0.0f) {
                nonzero_ff++;
                float abs_grad = fabsf(training->ff_grads[layer].w1_lattice[i]);
                if (abs_grad > max_ff_grad) max_ff_grad = abs_grad;
            }
        }
    }
    printf("  FeedForward gradients: %d non-zero (max: %.6f)\n", nonzero_ff, max_ff_grad);
    
    // Apply optimizer step
    printf("\nApplying optimizer step...\n");
    cllm_optimizer_step(training);
    
    // Run forward again to see if loss changed
    printf("\nRunning forward pass again...\n");
    float loss_after = cllm_forward_training(training, input_tokens);
    printf("Loss after: %.6f\n", loss_after);
    printf("Loss change: %.6f\n", loss_after - loss_before);
    
    if (fabsf(loss_after - loss_before) < 1e-6f) {
        printf("\n❌ PROBLEM: Loss did not change!\n");
    } else {
        printf("\n✓ Loss changed (expected behavior)\n");
    }
    
    cllm_training_cleanup(training);
    cllm_free_model(model);
    
    return 0;
}
