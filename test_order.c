#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing with model/training created FIRST...\n\n");
    
    srand(time(NULL));
    
    // Create model FIRST
    printf("[1] Creating model...\n");
    CLLMConfig model_config = {
        .vocab_size = 381,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 32,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&model_config);
    printf("    OK: %lu params\n", model->num_weights);
    
    // Create training SECOND
    printf("[2] Creating training...\n");
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 1
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    printf("    OK\n");
    
    // Test backward with dummy data
    printf("[3] Testing backward pass...\n");
    uint32_t input_tokens[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t target_tokens[16] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    
    fflush(stdout);
    cllm_backward(training, input_tokens, target_tokens, 16);
    printf("    OK\n");
    
    printf("\n✓ Test passed\n");
    
    // Cleanup
    cllm_training_free(training);
    cllm_free_model(model);
    
    return 0;
}
