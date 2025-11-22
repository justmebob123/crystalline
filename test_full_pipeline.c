#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"
#include "include/cllm_utils.h"

int main() {
    printf("=== Full Pipeline Test ===\n\n");
    
    srand(time(NULL));
    
    // Step 1
    printf("Step 1: Creating tokenizer...\n");
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(500);
    if (!tokenizer) return 1;
    printf("✓ Tokenizer created\n");
    
    // Step 2
    printf("\nStep 2: Creating data loader...\n");
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    if (!loader) {
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("✓ Data loader created\n");
    
    // Step 3
    printf("\nStep 3: Loading files...\n");
    int files = cllm_data_loader_load_directory(loader, "./data/raw");
    printf("✓ Loaded %d files\n", files);
    
    // Step 4
    printf("\nStep 4: Building vocabulary...\n");
    cllm_data_loader_build_vocab(loader);
    printf("✓ Vocabulary: %u tokens\n", tokenizer->vocab_size);
    
    // Step 5
    printf("\nStep 5: Creating dataset...\n");
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset) {
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("✓ Dataset: %zu tokens\n", dataset->num_tokens);
    
    // Step 6
    printf("\nStep 6: Creating model...\n");
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
    if (!model) {
        printf("Failed at model creation\n");
        cllm_token_dataset_free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("✓ Model created: %lu params\n", model->num_weights);
    
    // Step 7
    printf("\nStep 7: Creating training state...\n");
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 10
    };
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        printf("Failed at training init\n");
        cllm_free_model(model);
        cllm_token_dataset_free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("✓ Training state created\n");
    
    // Step 8
    printf("\nStep 8: Setting training data...\n");
    training->tokens = dataset->tokens;
    training->num_tokens = dataset->num_tokens;
    training->total_batches = dataset->num_tokens / (train_config.batch_size * train_config.sequence_length);
    printf("✓ Training data set: %d batches\n", training->total_batches);
    
    // Step 9
    printf("\nStep 9: Running one training step...\n");
    training->current_epoch = 0;
    training->current_batch_offset = 0;
    
    uint32_t* input_tokens = (uint32_t*)malloc(train_config.batch_size * train_config.sequence_length * sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)malloc(train_config.batch_size * train_config.sequence_length * sizeof(uint32_t));
    
    int tokens = cllm_get_batch(training, input_tokens, target_tokens);
    printf("Got batch: %d tokens\n", tokens);
    
    if (tokens > 0) {
        printf("Computing loss...\n");
        float loss = cllm_compute_loss(training, input_tokens, target_tokens, tokens);
        printf("Loss: %.4f\n", loss);
        
        printf("Running backward pass...\n");
        cllm_backward(training, input_tokens, target_tokens, tokens);
        printf("✓ Backward pass complete\n");
        
        printf("Running optimizer step...\n");
        cllm_optimizer_step(training);
        printf("✓ Optimizer step complete\n");
    }
    
    free(input_tokens);
    free(target_tokens);
    
    // Cleanup
    printf("\nCleaning up...\n");
    training->tokens = NULL;
    cllm_training_free(training);
    printf("✓ Training freed\n");
    
    cllm_free_model(model);
    printf("✓ Model freed\n");
    
    cllm_token_dataset_free(dataset);
    printf("✓ Dataset freed\n");
    
    cllm_data_loader_free(loader);
    printf("✓ Loader freed\n");
    
    cllm_free_tokenizer(tokenizer);
    printf("✓ Tokenizer freed\n");
    
    printf("\n=== All Steps Passed ===\n");
    return 0;
}
