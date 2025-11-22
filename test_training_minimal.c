#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"
#include "include/cllm_utils.h"

int main() {
    printf("=== Minimal Training Test ===\n\n");
    
    srand(time(NULL));
    
    // Step 1: Create tokenizer and load data
    printf("Step 1: Loading data...\n");
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(500);
    if (!tokenizer) {
        printf("Failed to create tokenizer\n");
        return 1;
    }
    
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    if (!loader) {
        printf("Failed to create loader\n");
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    int files = cllm_data_loader_load_directory(loader, "./data/raw");
    printf("Loaded %d files\n", files);
    
    if (files == 0) {
        printf("No files found\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    // Step 2: Build vocabulary
    printf("\nStep 2: Building vocabulary...\n");
    cllm_data_loader_build_vocab(loader);
    printf("Vocabulary size: %u\n", tokenizer->vocab_size);
    
    // Step 3: Create dataset
    printf("\nStep 3: Creating dataset...\n");
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset) {
        printf("Failed to create dataset\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("Dataset: %zu tokens\n", dataset->num_tokens);
    
    // Step 4: Create model with SMALL config
    printf("\nStep 4: Creating model...\n");
    CLLMConfig model_config = {
        .vocab_size = tokenizer->vocab_size,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 32,
        .dropout = 0.1f
    };
    
    printf("Model config:\n");
    printf("  vocab_size: %u\n", model_config.vocab_size);
    printf("  embedding_dim: %u\n", model_config.embedding_dim);
    printf("  num_layers: %u\n", model_config.num_layers);
    printf("  num_heads: %u\n", model_config.num_heads);
    printf("  ff_dim: %u\n", model_config.ff_dim);
    
    CLLMModel* model = cllm_create_model(&model_config);
    if (!model) {
        printf("Failed to create model\n");
        cllm_token_dataset_free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("✓ Model created: %lu params\n", model->num_weights);
    
    // Step 5: Create training state
    printf("\nStep 5: Creating training state...\n");
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 3,
        .max_steps = 100
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        printf("Failed to create training state\n");
        cllm_free_model(model);
        cllm_token_dataset_free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("✓ Training state created\n");
    
    // Set training data
    training->tokens = dataset->tokens;
    training->num_tokens = dataset->num_tokens;
    
    int tokens_per_batch = train_config.batch_size * train_config.sequence_length;
    training->total_batches = dataset->num_tokens / tokens_per_batch;
    
    printf("  Tokens per batch: %d\n", tokens_per_batch);
    printf("  Total batches: %d\n", training->total_batches);
    
    // Step 6: Train for a few steps
    printf("\nStep 6: Training...\n");
    time_t start = time(NULL);
    
    for (int epoch = 0; epoch < train_config.num_epochs; epoch++) {
        printf("\nEpoch %d/%d:\n", epoch + 1, train_config.num_epochs);
        training->current_epoch = epoch;
        
        float epoch_loss = cllm_train_epoch(training);
        
        printf("  Avg loss: %.4f\n", epoch_loss);
        printf("  Best loss: %.4f\n", training->best_loss);
    }
    
    time_t end = time(NULL);
    double elapsed = difftime(end, start);
    
    printf("\n✓ Training complete!\n");
    printf("  Time: %.1f seconds\n", elapsed);
    printf("  Final loss: %.4f\n", training->current_loss);
    printf("  Best loss: %.4f\n", training->best_loss);
    printf("  Total steps: %d\n", training->current_step);
    
    // Cleanup
    training->tokens = NULL;  // Don't free, belongs to dataset
    cllm_training_free(training);
    cllm_free_model(model);
    cllm_token_dataset_free(dataset);
    cllm_data_loader_free(loader);
    cllm_free_tokenizer(tokenizer);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
