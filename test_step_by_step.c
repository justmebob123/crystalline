#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"
#include "include/cllm_utils.h"

int main() {
    printf("Step-by-step test with detailed logging\n\n");
    
    srand(time(NULL));
    
    printf("[1] Creating tokenizer...\n");
    fflush(stdout);
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(500);
    if (!tokenizer) { printf("FAILED\n"); return 1; }
    printf("    OK\n");
    
    printf("[2] Creating data loader...\n");
    fflush(stdout);
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    if (!loader) { printf("FAILED\n"); return 1; }
    printf("    OK\n");
    
    printf("[3] Loading files...\n");
    fflush(stdout);
    int files = cllm_data_loader_load_directory(loader, "./data/raw");
    printf("    OK: %d files\n", files);
    
    printf("[4] Building vocabulary...\n");
    fflush(stdout);
    cllm_data_loader_build_vocab(loader);
    printf("    OK: %u tokens\n", tokenizer->vocab_size);
    
    printf("[5] Creating dataset...\n");
    fflush(stdout);
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset) { printf("FAILED\n"); return 1; }
    printf("    OK: %zu tokens\n", dataset->num_tokens);
    
    printf("[6] Freeing data loader (keeping tokenizer and dataset)...\n");
    fflush(stdout);
    cllm_data_loader_free(loader);
    printf("    OK\n");
    
    printf("[7] Creating model...\n");
    fflush(stdout);
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
    if (!model) { printf("FAILED\n"); return 1; }
    printf("    OK: %lu params\n", model->num_weights);
    
    printf("[8] Creating training state...\n");
    fflush(stdout);
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 10
    };
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) { printf("FAILED\n"); return 1; }
    printf("    OK\n");
    
    printf("[9] All steps completed successfully!\n");
    
    // Cleanup
    cllm_training_free(training);
    cllm_free_model(model);
    cllm_token_dataset_free(dataset);
    cllm_free_tokenizer(tokenizer);
    
    printf("\n✓ Test passed\n");
    return 0;
}
