#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing training initialization...\n\n");
    
    // Create model
    CLLMConfig config = {
        .vocab_size = 381,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 32,
        .dropout = 0.1f
    };
    
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    printf("✓ Model created: %lu params\n", model->num_weights);
    
    // Create training config
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 3,
        .max_steps = 100
    };
    
    printf("\nInitializing training...\n");
    printf("  batch_size: %d\n", train_config.batch_size);
    printf("  sequence_length: %d\n", train_config.sequence_length);
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        printf("Failed to initialize training\n");
        cllm_free_model(model);
        return 1;
    }
    
    printf("✓ Training initialized\n");
    printf("  Gradient buffer: %p\n", (void*)training->gradients);
    printf("  Attention grads: %p\n", (void*)training->attention_grads);
    printf("  FF grads: %p\n", (void*)training->ff_grads);
    printf("  LN grads: %p\n", (void*)training->ln_grads);
    
    // Cleanup
    cllm_training_free(training);
    printf("✓ Training freed\n");
    
    cllm_free_model(model);
    printf("✓ Model freed\n");
    
    return 0;
}
