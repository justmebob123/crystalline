#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing backward pass with minimal setup...\n\n");
    
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
    
    // Create minimal batch
    uint32_t input_tokens[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t target_tokens[16] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    
    printf("Calling cllm_backward with 16 tokens...\n");
    printf("  batch_size: %d\n", train_config.batch_size);
    printf("  sequence_length: %d\n", train_config.sequence_length);
    printf("  embed_dim: %lu\n", model->embedding_dim);
    
    uint64_t embed_dim = model->embedding_dim;
    size_t activation_size = train_config.batch_size * train_config.sequence_length * embed_dim;
    printf("  activation_size: %zu floats (%zu bytes, %.2f MB)\n",
           activation_size, activation_size * sizeof(float),
           (activation_size * sizeof(float)) / (1024.0 * 1024.0));
    
    fflush(stdout);
    
    cllm_backward(training, input_tokens, target_tokens, 16);
    
    printf("✓ Backward pass completed\n");
    
    // Cleanup (don't free tokens)
    cllm_training_free(training);
    cllm_free_model(model);
    cllm_token_dataset_free(dataset);
    cllm_free_tokenizer(tokenizer);
    
    return 0;
}
