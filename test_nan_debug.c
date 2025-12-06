/**
 * Minimal test to debug NaN issue
 * NO SHORTCUTS - Find exact source of NaN
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/prime_float_math.h"

// Check for NaN in array
int check_nan(const char* name, double* arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (isnan(arr[i]) || isinf(arr[i])) {
            printf("ERROR: %s[%zu] = %f (NaN or Inf detected!)\n", name, i, arr[i]);
            return 1;
        }
    }
    return 0;
}

int main() {
    printf("\n=== NaN Debug Test ===\n\n");
    
    // Create tiny model
    printf("Creating tiny model (vocab=50, embed=16, layers=2, heads=2)...\n");
    CLLMConfig* config = (CLLMConfig*)calloc(1, sizeof(CLLMConfig));
    config->vocab_size = 50;
    config->embedding_dim = 16;
    config->num_layers = 2;
    config->num_heads = 2;
    config->ff_dim = 32;
    config->max_seq_len = 8;
    
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    model->vocab_size = config->vocab_size;
    model->embedding_dim = config->embedding_dim;
    model->num_layers = config->num_layers;
    
    // Initialize embeddings
    model->embeddings.vocab_size = config->vocab_size;
    model->embeddings.embedding_dim = config->embedding_dim;
    model->embeddings.embeddings = (double*)calloc(config->vocab_size * config->embedding_dim, sizeof(double));
    
    // Initialize with small random values
    for (size_t i = 0; i < config->vocab_size * config->embedding_dim; i++) {
        model->embeddings.embeddings[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    printf("✓ Model created\n");
    printf("  Checking initial embeddings for NaN...\n");
    if (check_nan("embeddings", model->embeddings.embeddings, config->vocab_size * config->embedding_dim)) {
        printf("  ERROR: Initial embeddings contain NaN!\n");
        return 1;
    }
    printf("  ✓ Initial embeddings OK\n\n");
    
    // Create training config
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .num_epochs = 1,
        .sequence_length = 4,
        .save_every = 1,
        .gradient_accumulation_steps = 1
    };
    
    printf("Initializing training...\n");
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        printf("ERROR: Failed to initialize training\n");
        return 1;
    }
    printf("✓ Training initialized\n\n");
    
    // Create simple training data
    uint32_t tokens[] = {1, 2, 3, 4, 5, 6, 7, 8};
    training->tokens = tokens;
    training->num_tokens = 8;
    
    // Allocate logits
    size_t logits_size = train_config.batch_size * train_config.sequence_length * config->vocab_size;
    training->logits = (double*)calloc(logits_size, sizeof(double));
    
    // Initialize logits with small random values
    for (size_t i = 0; i < logits_size; i++) {
        training->logits[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    }
    
    printf("Testing loss computation...\n");
    uint32_t input_tokens[] = {1, 2, 3, 4};
    uint32_t target_tokens[] = {2, 3, 4, 5};
    
    printf("  Checking logits before loss computation...\n");
    if (check_nan("logits", training->logits, logits_size)) {
        printf("  ERROR: Logits contain NaN before loss computation!\n");
        return 1;
    }
    printf("  ✓ Logits OK before loss\n");
    
    float loss = cllm_compute_loss(training, input_tokens, target_tokens, 4);
    printf("  Loss = %f\n", loss);
    
    if (isnan(loss) || isinf(loss)) {
        printf("  ERROR: Loss is NaN or Inf!\n");
        printf("  Checking logits after loss computation...\n");
        check_nan("logits", training->logits, logits_size);
        return 1;
    }
    printf("  ✓ Loss OK\n\n");
    
    // Test multiple iterations
    printf("Testing 10 iterations...\n");
    for (int iter = 0; iter < 10; iter++) {
        // Reinitialize logits
        for (size_t i = 0; i < logits_size; i++) {
            training->logits[i] = ((double)rand() / RAND_MAX - 0.5) * 0.1;
        }
        
        loss = cllm_compute_loss(training, input_tokens, target_tokens, 4);
        printf("  Iteration %d: loss = %f\n", iter + 1, loss);
        
        if (isnan(loss) || isinf(loss)) {
            printf("  ERROR: Loss became NaN at iteration %d!\n", iter + 1);
            return 1;
        }
    }
    
    printf("\n✓ All tests passed - no NaN detected\n");
    
    // Cleanup
    free(training->logits);
    cllm_training_cleanup(training);
    free(model->embeddings.embeddings);
    free(model);
    free(config);
    
    return 0;
}