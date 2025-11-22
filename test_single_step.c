#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing single training step...\n\n");
    
    srand(time(NULL));
    
    // Setup
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(500);
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    cllm_data_loader_load_directory(loader, "./data/raw");
    cllm_data_loader_build_vocab(loader);
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    cllm_data_loader_free(loader);
    
    CLLMConfig model_config = {
        .vocab_size = tokenizer->vocab_size,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 32,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&model_config);
    
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 1
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    training->tokens = dataset->tokens;
    training->num_tokens = dataset->num_tokens;
    training->total_batches = 1;
    
    printf("Setup complete. Running single step...\n");
    
    // Allocate batch
    printf("[1] Allocating batch buffers...\n");
    fflush(stdout);
    uint32_t* input_tokens = (uint32_t*)malloc(train_config.batch_size * train_config.sequence_length * sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)malloc(train_config.batch_size * train_config.sequence_length * sizeof(uint32_t));
    printf("    OK\n");
    
    // Get batch
    printf("[2] Getting batch...\n");
    fflush(stdout);
    training->current_batch_offset = 0;
    int tokens = cllm_get_batch(training, input_tokens, target_tokens);
    printf("    OK: %d tokens\n", tokens);
    
    // Compute loss
    printf("[3] Computing loss...\n");
    fflush(stdout);
    float loss = cllm_compute_loss(training, input_tokens, target_tokens, tokens);
    printf("    OK: loss = %.4f\n", loss);
    
    // Backward pass
    printf("[4] Running backward pass...\n");
    fflush(stdout);
    cllm_backward(training, input_tokens, target_tokens, tokens);
    printf("    OK\n");
    
    // Optimizer step
    printf("[5] Running optimizer step...\n");
    fflush(stdout);
    cllm_optimizer_step(training);
    printf("    OK\n");
    
    printf("\n✓ Single step completed successfully!\n");
    
    // Cleanup
    free(input_tokens);
    free(target_tokens);
    training->tokens = NULL;
    cllm_training_free(training);
    cllm_free_model(model);
    cllm_token_dataset_free(dataset);
    cllm_free_tokenizer(tokenizer);
    
    return 0;
}
