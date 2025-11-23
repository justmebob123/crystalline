#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_training.h"

int main() {
    printf("=== TESTING BACKWARD PASS GRADIENTS ===\n");
    
    // Load model
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    FILE* f = fopen("models/saved_model.cllm", "rb");
    if (!f) {
        printf("ERROR: Could not open model\n");
        return 1;
    }
    
    uint32_t magic, version;
    fread(&magic, sizeof(uint32_t), 1, f);
    fread(&version, sizeof(uint32_t), 1, f);
    fread(&model->vocab_size, sizeof(uint32_t), 1, f);
    fread(&model->embedding_dim, sizeof(uint64_t), 1, f);
    fread(&model->num_layers, sizeof(uint32_t), 1, f);
    
    model->embeddings.embeddings = (float*)malloc(model->vocab_size * model->embedding_dim * sizeof(float));
    fread(model->embeddings.embeddings, sizeof(float), model->vocab_size * model->embedding_dim, f);
    fclose(f);
    
    printf("Loaded model: vocab=%u, embed=%lu, layers=%u\n",
           model->vocab_size, model->embedding_dim, model->num_layers);
    
    // Create training
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 32,
        .num_epochs = 1
    };
    strcpy(config.optimizer, "sgd");
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        printf("ERROR: Failed to init training\n");
        return 1;
    }
    
    // Create fake batch
    uint32_t input_tokens[4] = {1, 2, 3, 4};
    uint32_t target_tokens[4] = {2, 3, 4, 5};
    
    printf("\nBefore backward pass:\n");
    printf("gradient[0] = %.6f\n", training->gradients[0]);
    printf("gradient[512] = %.6f\n", training->gradients[512]);
    
    // Call backward
    printf("\nCalling cllm_backward...\n");
    cllm_backward(training, input_tokens, target_tokens, 4);
    
    printf("\nAfter backward pass:\n");
    printf("gradient[0] = %.6f\n", training->gradients[0]);
    printf("gradient[512] = %.6f\n", training->gradients[512]);
    
    // Check if any gradients are non-zero
    int nonzero = 0;
    for (int i = 0; i < 100; i++) {
        if (training->gradients[i] != 0.0f) {
            nonzero++;
        }
    }
    
    printf("\nNon-zero gradients in first 100: %d\n", nonzero);
    
    if (nonzero > 0) {
        printf("✓ Backward pass IS computing gradients!\n");
    } else {
        printf("✗ Backward pass NOT computing gradients!\n");
    }
    
    cllm_training_free(training);
    
    return 0;
}
