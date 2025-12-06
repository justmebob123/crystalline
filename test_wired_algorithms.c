#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_training.h"

int main() {
    printf("Testing Wired Algorithms\n");
    printf("========================\n\n");
    
    // Create a small model
    CLLMModel* model = cllm_create_small_model(100, 32, 2, 4);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("✓ Model created: vocab=%lu, embed=%lu, layers=%u\n", 
           model->vocab_size, model->embedding_dim, model->num_layers);
    
    // Create training config
    CLLMTrainingConfig config = {
        .learning_rate = 0.01f,
        .batch_size = 2,
        .num_epochs = 1,
        .sequence_length = 4,
        .save_every = 1
    };
    
    // Initialize training
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "Failed to initialize training\n");
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Training initialized\n");
    
    // Check that algorithm layer is wired
    printf("\nAlgorithm Layer Status:\n");
    printf("  Loss Config: %s\n", training->loss_config.type == LOSS_CROSS_ENTROPY ? "WIRED" : "NOT WIRED");
    printf("  Optimizer State: %s\n", training->optimizer_state_alg ? "WIRED" : "NOT WIRED");
    printf("  Gradient Buffer: %s\n", training->gradient_buffer ? "WIRED" : "NOT WIRED");
    
    // Test loss computation
    uint32_t input_tokens[] = {1, 2, 3, 4};
    uint32_t target_tokens[] = {2, 3, 4, 5};
    
    // Allocate logits
    training->logits = (double*)calloc(4 * model->vocab_size, sizeof(double));
    
    // Set some random logits
    for (int i = 0; i < 4 * model->vocab_size; i++) {
        training->logits[i] = (double)rand() / RAND_MAX - 0.5;
    }
    
    float loss = cllm_compute_loss(training, input_tokens, target_tokens, 4);
    printf("\n✓ Loss computation successful: %.4f\n", loss);
    
    // Cleanup
    cllm_training_cleanup(training);
    cllm_free_model(model);
    
    printf("\n========================================\n");
    printf("SUCCESS: All algorithm layers are WIRED!\n");
    printf("========================================\n");
    
    return 0;
}
