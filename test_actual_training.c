#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_tokenizer.h"
#include "include/cllm_data_loader.h"
#include "include/cllm_inference.h"
#include "include/cllm_utils.h"

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM Training Test - Full Pipeline                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    srand(time(NULL));
    
    // Load data
    printf("Step 1: Loading training data\n");
    printf("─────────────────────────────────────────────────────────────\n");
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(1000);
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    int files = cllm_data_loader_load_directory(loader, "./data/raw");
    printf("✓ Loaded %d files\n\n", files);
    
    // Build vocabulary
    printf("Step 2: Building vocabulary\n");
    printf("─────────────────────────────────────────────────────────────\n");
    cllm_data_loader_build_vocab(loader);
    cllm_data_loader_print_stats(loader);
    
    // Create dataset
    printf("Step 3: Creating training dataset\n");
    printf("─────────────────────────────────────────────────────────────\n");
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    printf("✓ Dataset: %zu tokens\n\n", dataset->num_tokens);
    
    cllm_data_loader_free(loader);
    
    // Create model
    printf("Step 4: Creating model\n");
    printf("─────────────────────────────────────────────────────────────\n");
    CLLMConfig model_config = {
        .vocab_size = tokenizer->vocab_size,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 64,
        .dropout = 0.1f
    };
    
    printf("Configuration:\n");
    printf("  Vocab size:     %u\n", model_config.vocab_size);
    printf("  Embedding dim:  %u\n", model_config.embedding_dim);
    printf("  Num layers:     %u\n", model_config.num_layers);
    printf("  Num heads:      %u\n", model_config.num_heads);
    printf("  FF dim:         %u\n", model_config.ff_dim);
    printf("  Max seq len:    %u\n", model_config.max_seq_len);
    
    CLLMModel* model = cllm_create_model(&model_config);
    printf("✓ Model created: %lu parameters\n\n", model->num_weights);
    
    // Setup training
    printf("Step 5: Initializing training\n");
    printf("─────────────────────────────────────────────────────────────\n");
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 16,
        .num_epochs = 10,
        .max_steps = 1000
    };
    
    printf("Training configuration:\n");
    printf("  Learning rate:  %.6f\n", train_config.learning_rate);
    printf("  Batch size:     %d\n", train_config.batch_size);
    printf("  Sequence len:   %d\n", train_config.sequence_length);
    printf("  Num epochs:     %d\n", train_config.num_epochs);
    
    CLLMTraining* training = cllm_training_init(model, &train_config);
    training->tokens = dataset->tokens;
    training->num_tokens = dataset->num_tokens;
    training->total_batches = dataset->num_tokens / (train_config.batch_size * train_config.sequence_length);
    
    printf("✓ Training initialized: %d batches per epoch\n\n", training->total_batches);
    
    // Train
    printf("Step 6: Training model\n");
    printf("─────────────────────────────────────────────────────────────\n");
    
    time_t start_time = time(NULL);
    
    for (int epoch = 0; epoch < train_config.num_epochs; epoch++) {
        training->current_epoch = epoch;
        
        printf("\nEpoch %d/%d:\n", epoch + 1, train_config.num_epochs);
        
        float epoch_loss = cllm_train_epoch(training);
        
        printf("  Average loss: %.4f\n", epoch_loss);
        printf("  Best loss:    %.4f\n", training->best_loss);
        printf("  Steps:        %d\n", training->current_step);
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    printf("\n");
    printf("Step 7: Training complete\n");
    printf("─────────────────────────────────────────────────────────────\n");
    printf("Training statistics:\n");
    printf("  Total time:     %.1f seconds\n", elapsed);
    printf("  Total steps:    %d\n", training->current_step);
    printf("  Final loss:     %.4f\n", training->current_loss);
    printf("  Best loss:      %.4f\n", training->best_loss);
    printf("  Steps/second:   %.1f\n", training->current_step / elapsed);
    printf("\n");
    
    // Test inference
    printf("Step 8: Testing inference\n");
    printf("─────────────────────────────────────────────────────────────\n");
    
    CLLMInference* inference = cllm_inference_init(model);
    if (inference) {
        cllm_set_temperature(inference, 0.8f);
        cllm_set_top_p(inference, 0.9f);
        cllm_set_max_tokens(inference, 30);
        
        const char* prompts[] = {
            "artificial intelligence",
            "machine learning",
            "programming"
        };
        
        for (int i = 0; i < 3; i++) {
            printf("\nPrompt: %s\n", prompts[i]);
            
            char output[1024];
            int tokens = cllm_generate(inference, prompts[i], output, sizeof(output));
            
            if (tokens > 0) {
                printf("Generated: %s\n", output);
                printf("Tokens: %d\n", tokens);
            } else {
                printf("Generation failed\n");
            }
        }
        
        cllm_inference_cleanup(inference);
    }
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                  Test Complete!                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    // Cleanup
    training->tokens = NULL;
    cllm_training_free(training);
    cllm_free_model(model);
    cllm_token_dataset_free(dataset);
    cllm_free_tokenizer(tokenizer);
    
    return 0;
}
