/**
 * Gradient Debug Test
 * Checks if gradients are being computed correctly
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "include/cllm.h"
#include "include/cllm_training.h"

// Forward declarations
extern CLLMModel* cllm_create_small_model(void);
extern void cllm_free_model(CLLMModel* model);

int main() {
    printf("\n=== Gradient Debug Test ===\n\n");
    
    // Create model
    printf("Step 1: Creating model\n");
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("  Model created: vocab=%lu, dim=%lu, layers=%u\n", 
           (unsigned long)model->vocab_size,
           (unsigned long)model->embedding_dim,
           model->num_layers);
    
    // Initialize training
    printf("\nStep 2: Initializing training\n");
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 1,  // Single batch for debugging
        .sequence_length = 4,  // Short sequence
        .num_epochs = 1,
        .max_steps = 1,
        .gradient_clip = 1.0f,
        .weight_decay = 0.01f,
        .warmup_steps = 0,
        .save_every = 10
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "Failed to initialize training\n");
        cllm_free_model(model);
        return 1;
    }
    printf("  Training initialized\n");
    
    // Check if gradients are allocated
    printf("\nStep 3: Checking gradient allocation\n");
    if (training->gradients) {
        printf("  ✓ Gradients allocated\n");
    } else {
        printf("  ✗ Gradients NOT allocated!\n");
        cllm_training_free(training);
        cllm_free_model(model);
        return 1;
    }
    
    if (training->attention_grads) {
        printf("  ✓ Attention gradients allocated\n");
        if (training->attention_grads[0].query_lattice) {
            printf("  ✓ Query gradients allocated\n");
        } else {
            printf("  ✗ Query gradients NOT allocated!\n");
        }
    } else {
        printf("  ✗ Attention gradients NOT allocated!\n");
    }
    
    // Load minimal training data
    printf("\nStep 4: Loading training data\n");
    if (cllm_load_training_data(training, "data/tiny.txt") != 0) {
        fprintf(stderr, "Failed to load training data\n");
        cllm_training_free(training);
        cllm_free_model(model);
        return 1;
    }
    printf("  Training data loaded\n");
    
    // Create simple synthetic batch
    printf("\nStep 5: Creating synthetic batch\n");
    uint32_t input_tokens[4] = {0, 1, 2, 3};
    uint32_t target_tokens[4] = {1, 2, 3, 0};
    printf("  Input: [0, 1, 2, 3]\n");
    printf("  Target: [1, 2, 3, 0]\n");
    
    // Forward pass
    printf("\nStep 6: Running forward pass\n");
    extern float cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens);
    float loss = cllm_forward_training(training, input_tokens);
    printf("  Forward pass complete, loss = %.4f\n", loss);
    
    // Check embeddings before backward
    printf("\nStep 7: Checking embeddings before backward\n");
    double sum_before = 0.0;
    for (int i = 0; i < 10; i++) {
        sum_before += fabs(model->embeddings.embeddings[i]);
    }
    printf("  Sum of first 10 embeddings: %.6f\n", sum_before);
    
    // Backward pass
    printf("\nStep 8: Running backward pass\n");
    extern void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens, double* gradient_buffer);
    cllm_backward_training(training, target_tokens, NULL);
    printf("  Backward pass complete\n");
    
    // Check gradients after backward
    printf("\nStep 9: Checking gradients after backward\n");
    int nonzero_grads = 0;
    double max_grad = 0.0;
    double sum_grad = 0.0;
    
    size_t grad_size = model->vocab_size * model->embedding_dim;
    for (size_t i = 0; i < grad_size && i < 1000; i++) {
        double g = fabs(training->gradients[i]);
        if (g > 1e-10) {
            nonzero_grads++;
            sum_grad += g;
            if (g > max_grad) max_grad = g;
        }
    }
    
    printf("  Nonzero gradients: %d / 1000\n", nonzero_grads);
    printf("  Max gradient: %.6e\n", max_grad);
    printf("  Avg gradient: %.6e\n", nonzero_grads > 0 ? sum_grad / nonzero_grads : 0.0);
    
    if (nonzero_grads == 0) {
        printf("\n  ✗ ERROR: All gradients are ZERO!\n");
        printf("  This means the backward pass is not working correctly.\n");
        
        // Additional diagnostics
        printf("\nStep 10: Additional diagnostics\n");
        
        // Check if logits were computed
        if (training->logits) {
            printf("  ✓ Logits allocated\n");
            double sum_logits = 0.0;
            for (int i = 0; i < 10; i++) {
                sum_logits += fabs(training->logits[i]);
            }
            printf("  Sum of first 10 logits: %.6f\n", sum_logits);
        } else {
            printf("  ✗ Logits NOT allocated!\n");
        }
        
        // Check if final_hidden was computed
        if (training->final_hidden) {
            printf("  ✓ Final hidden allocated\n");
            double sum_hidden = 0.0;
            for (int i = 0; i < 10; i++) {
                sum_hidden += fabs(training->final_hidden[i]);
            }
            printf("  Sum of first 10 hidden: %.6f\n", sum_hidden);
        } else {
            printf("  ✗ Final hidden NOT allocated!\n");
        }
        
    } else {
        printf("\n  ✓ SUCCESS: Gradients are being computed!\n");
    }
    
    // Cleanup
    printf("\nStep 11: Cleanup\n");
    cllm_training_free(training);
    cllm_free_model(model);
    printf("  Cleanup complete\n");
    
    printf("\n=== Test Complete ===\n\n");
    
    return (nonzero_grads == 0) ? 1 : 0;
}