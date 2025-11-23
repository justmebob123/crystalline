#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"

int main() {
    printf("=== Training Performance Analysis ===\n\n");
    
    // Create minimal model
    CLLMModel* model = cllm_create_model(1000, 128, 2, 4, 32);
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    
    // Create training config
    CLLMTrainingConfig config = {
        .batch_size = 4,
        .sequence_length = 32,
        .learning_rate = 0.001f,
        .num_epochs = 1,
        .save_every = 100
    };
    
    // Initialize training
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        printf("Failed to init training\n");
        cllm_free_model(model);
        return 1;
    }
    
    // Create dummy training data
    training->num_tokens = 1000;
    training->tokens = (uint32_t*)malloc(1000 * sizeof(uint32_t));
    for (int i = 0; i < 1000; i++) {
        training->tokens[i] = i % model->vocab_size;
    }
    training->total_batches = training->num_tokens / (config.batch_size * config.sequence_length);
    
    printf("Model: vocab=%u, embed_dim=%u, layers=%u\n", 
           model->vocab_size, model->embedding_dim, model->num_layers);
    printf("Training: batch_size=%d, seq_len=%d, tokens=%zu\n",
           config.batch_size, config.sequence_length, training->num_tokens);
    printf("Total batches: %d\n\n", training->total_batches);
    
    // Profile one epoch
    printf("Starting epoch profiling...\n");
    clock_t start = clock();
    
    float loss = cllm_train_epoch(training);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nEpoch complete:\n");
    printf("  Time: %.3f seconds\n", elapsed);
    printf("  Loss: %.4f\n", loss);
    printf("  Batches processed: %d\n", training->total_batches);
    printf("  Time per batch: %.3f ms\n", (elapsed * 1000.0) / training->total_batches);
    
    // Cleanup
    cllm_training_cleanup(training);
    cllm_free_model(model);
    
    return 0;
}
