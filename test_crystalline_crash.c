#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_crystalline_training.h"
#include "cllm_vocab_builder.h"

int main() {
    printf("=== TESTING CRYSTALLINE TRAINING CRASH ===\n");
    
    // Create model
    printf("1. Creating model...\n");
    CLLMModel* model = cllm_create_model(10000, 512, 6, 8, 2048);
    if (!model) {
        printf("ERROR: Failed to create model\n");
        return 1;
    }
    printf("   ✓ Model created\n");
    
    // Build vocabulary
    printf("2. Building vocabulary...\n");
    const char* files[] = {"data/training/large_corpus.txt"};
    int result = cllm_build_vocabulary_from_files(model, files, 1);
    if (result != 0) {
        printf("ERROR: Failed to build vocabulary\n");
        return 1;
    }
    printf("   ✓ Vocabulary built: %d tokens\n", model->vocab_size);
    
    // Load training data
    printf("3. Loading training data...\n");
    result = cllm_load_training_data(model, "data/training/large_corpus.txt");
    if (result != 0) {
        printf("ERROR: Failed to load training data\n");
        return 1;
    }
    printf("   ✓ Training data loaded: %d tokens\n", model->num_tokens);
    
    // Initialize training
    printf("4. Initializing training...\n");
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 4,
        .sequence_length = 32,
        .num_epochs = 1,
        .use_crystalline = 1
    };
    
    CLLMTrainingState* training = cllm_init_training(model, &config);
    if (!training) {
        printf("ERROR: Failed to initialize training\n");
        return 1;
    }
    printf("   ✓ Training initialized\n");
    
    // Check critical pointers
    printf("5. Checking critical pointers...\n");
    printf("   model->tokens = %p\n", (void*)model->tokens);
    printf("   model->num_tokens = %d\n", model->num_tokens);
    printf("   training->tokens = %p\n", (void*)training->tokens);
    printf("   training->num_tokens = %d\n", training->num_tokens);
    printf("   training->backward_buffer = %p\n", (void*)training->backward_buffer);
    printf("   training->embedding_cache = %p\n", (void*)training->embedding_cache);
    
    // Try to get a batch
    printf("6. Getting first batch...\n");
    int* input_ids = (int*)malloc(config.batch_size * config.sequence_length * sizeof(int));
    int* target_ids = (int*)malloc(config.batch_size * config.sequence_length * sizeof(int));
    
    int batch_result = cllm_get_batch(training, 0, input_ids, target_ids);
    printf("   Batch result: %d\n", batch_result);
    
    if (batch_result > 0) {
        printf("   ✓ Got batch with %d sequences\n", batch_result);
        
        // Print first few tokens
        printf("   First input tokens: ");
        for (int i = 0; i < 10 && i < config.sequence_length; i++) {
            printf("%d ", input_ids[i]);
        }
        printf("\n");
    } else {
        printf("   ERROR: Failed to get batch\n");
    }
    
    // Try crystalline training
    printf("7. Testing crystalline training epoch...\n");
    float loss = cllm_train_epoch_crystalline(model, training);
    printf("   Loss: %f\n", loss);
    
    // Cleanup
    printf("8. Cleaning up...\n");
    free(input_ids);
    free(target_ids);
    cllm_free_training(training);
    cllm_free_model(model);
    
    printf("=== TEST COMPLETE ===\n");
    return 0;
}