#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_data_loader.h"
#include "cllm_tokenizer.h"

extern CLLMModel* cllm_create_model(CLLMConfig* config);
extern void cllm_free_model(CLLMModel* model);
extern float cllm_train_epoch(CLLMTraining* training);
extern void cllm_precompute_all_embeddings(CLLMModel* model);

int main() {
    printf("=== Testing Weight Updates During Training ===\n\n");
    
    // Create tiny model
    CLLMConfig model_config = {
        .vocab_size = 50,
        .embedding_dim = 32,
        .num_layers = 1,
        .num_heads = 2,
        .ff_dim = 64,
        .max_seq_len = 16,
        .dropout = 0.0f
    };
    
    CLLMModel* model = cllm_create_model(&model_config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    
    // Precompute embeddings
    cllm_precompute_all_embeddings(model);
    
    // Save initial weights
    double initial_weights[10];
    for (int i = 0; i < 10; i++) {
        initial_weights[i] = model->embeddings.embeddings[i];
    }
    
    printf("Initial weights (first 10):\n");
    for (int i = 0; i < 10; i++) {
        printf("  [%d] = %.6f\n", i, initial_weights[i]);
    }
    printf("\n");
    
    // Create training data
    const char* text = "The sky is blue. The grass is green. The sun is yellow. Water is wet. Fire is hot.";
    
    // Tokenize
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(50);
    
    uint32_t num_tokens_u32 = 0;
    uint32_t* tokens = cllm_tokenizer_encode(tokenizer, text, &num_tokens_u32);
    size_t num_tokens = num_tokens_u32;
    
    printf("Tokenized %zu tokens\n\n", num_tokens);
    
    // Create training config
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001,  // Higher LR for testing
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 5,
        .warmup_steps = 0,  // No warmup for testing
        .gradient_accumulation_steps = 1
    };
    strcpy(train_config.lr_scheduler, "none");
    
    // Initialize training
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        fprintf(stderr, "Failed to init training\n");
        cllm_free_model(model);
        free(tokens);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    // Set training data
    training->tokens = tokens;
    training->num_tokens = num_tokens;
    
    printf("Training for 5 epochs...\n");
    float losses[5];
    for (int epoch = 0; epoch < 5; epoch++) {
        training->current_epoch = epoch;
        losses[epoch] = cllm_train_epoch(training);
        printf("Epoch %d loss: %.6f\n", epoch + 1, losses[epoch]);
    }
    printf("\n");
    
    // Check if loss decreased
    printf("Loss progression:\n");
    for (int i = 0; i < 5; i++) {
        printf("  Epoch %d: %.6f", i + 1, losses[i]);
        if (i > 0) {
            float delta = losses[i] - losses[i-1];
            printf(" (delta: %+.6f)", delta);
        }
        printf("\n");
    }
    printf("\n");
    
    // Check if weights changed
    printf("Weights after training (first 10):\n");
    int num_changed = 0;
    for (int i = 0; i < 10; i++) {
        double new_weight = model->embeddings.embeddings[i];
        double delta = new_weight - initial_weights[i];
        printf("  [%d] = %.6f (delta: %+.6f)\n", i, new_weight, delta);
        if (fabs(delta) > 1e-10) num_changed++;
    }
    
    printf("=== RESULT ===\n");
    printf("Weights changed: %d / 10\n", num_changed);
    
    // Check if loss decreased overall
    float first_loss = losses[0];
    float last_loss = losses[4];
    printf("First epoch loss: %.6f\n", first_loss);
    printf("Last epoch loss: %.6f\n", last_loss);
    printf("Total change: %+.6f\n\n", last_loss - first_loss);
    
    if (last_loss < first_loss) {
        printf("✓ SUCCESS: Loss DECREASED over training!\n");
    } else {
        printf("✗ FAILURE: Loss INCREASED over training!\n");
    }
    
    cllm_training_free(training);
    cllm_free_model(model);
    free(tokens);
    cllm_free_tokenizer(tokenizer);
    
    return (num_changed > 0) ? 0 : 1;
}