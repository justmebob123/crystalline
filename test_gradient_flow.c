#include <stdio.h>
#include <stdlib.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_vocab_builder.h"

int main() {
    printf("=== TESTING GRADIENT FLOW ===\n");
    
    // Create small model
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    model->vocab_size = 611;
    model->embedding_dim = 512;
    model->num_layers = 6;
    
    // Allocate embeddings
    model->embeddings.embeddings = (float*)calloc(611 * 512, sizeof(float));
    
    // Initialize with small random values
    for (int i = 0; i < 611 * 512; i++) {
        model->embeddings.embeddings[i] = ((float)rand() / RAND_MAX) * 0.01f;
    }
    
    printf("Initial embedding[0][0] = %.6f\n", model->embeddings.embeddings[0]);
    printf("Initial embedding[1][0] = %.6f\n", model->embeddings.embeddings[512]);
    
    // Create training config
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 32,
        .num_epochs = 1
    };
    strcpy(config.optimizer, "sgd");
    
    // Initialize training
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        printf("ERROR: Failed to initialize training\n");
        return 1;
    }
    
    printf("\nChecking gradient buffer...\n");
    printf("training->gradients = %p\n", (void*)training->gradients);
    
    // Manually set some gradients
    if (training->gradients) {
        training->gradients[0] = 1.0f;
        training->gradients[512] = 2.0f;
        printf("Set gradient[0] = 1.0\n");
        printf("Set gradient[512] = 2.0\n");
        
        // Call optimizer
        printf("\nCalling optimizer...\n");
        cllm_optimizer_step(training);
        
        // Check if embeddings changed
        printf("\nAfter optimizer step:\n");
        printf("embedding[0][0] = %.6f (should be %.6f)\n", 
               model->embeddings.embeddings[0],
               model->embeddings.embeddings[0] + 0.001f * 1.0f);
        printf("embedding[1][0] = %.6f (should be %.6f)\n",
               model->embeddings.embeddings[512],
               model->embeddings.embeddings[512] + 0.001f * 2.0f);
        
        float diff0 = model->embeddings.embeddings[0] - (model->embeddings.embeddings[0] + 0.001f * 1.0f);
        float diff1 = model->embeddings.embeddings[512] - (model->embeddings.embeddings[512] + 0.001f * 2.0f);
        
        if (diff0 < 0.0001f && diff1 < 0.0001f) {
            printf("\n✓ Optimizer IS updating embeddings!\n");
        } else {
            printf("\n✗ Optimizer NOT updating embeddings!\n");
        }
    }
    
    cllm_training_free(training);
    
    return 0;
}
