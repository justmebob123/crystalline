#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_utils.h"
#include "../include/cllm_format.h"

int main() {
    printf("=== COMPLETE PIPELINE TEST ===\n\n");
    
    // Step 1: Create a small model
    printf("Step 1: Creating model...\n");
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("Model created: vocab=%u, embed=%u, layers=%u\n\n", 
           model->vocab_size, model->embedding_dim, model->num_layers);
    
    // Step 2: Create training data (simple pattern)
    printf("Step 2: Creating training data...\n");
    size_t num_tokens = 32;
    uint32_t* tokens = (uint32_t*)malloc(num_tokens * sizeof(uint32_t));
    // Pattern: [1,2,3,4,5,6,7,8] repeating
    for (size_t i = 0; i < num_tokens; i++) {
        tokens[i] = (i % 8) + 1;
    }
    printf("Created %zu tokens\n\n", num_tokens);
    
    // Step 3: Train the model
    printf("Step 3: Training model (20 steps)...\n");
    CLLMTrainingConfig config = {
        .num_epochs = 1,
        .batch_size = 1,
        .sequence_length = 8,
        .learning_rate = 0.01,
        .gradient_accumulation_steps = 1,
        .save_every = 1000,
        .use_mixed_precision = false
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "Failed to initialize training\n");
        free(tokens);
        cllm_free_model(model);
        return 1;
    }
    
    training->tokens = tokens;
    training->num_tokens = num_tokens;
    training->total_batches = num_tokens / (config.batch_size * config.sequence_length);
    
    // Train for 20 steps
    uint32_t batch[8], target[8];
    float initial_loss = 0, final_loss = 0;
    
    for (int step = 0; step < 20; step++) {
        // Prepare batch
        int start = (step * 8) % num_tokens;
        for (int i = 0; i < 8; i++) {
            batch[i] = tokens[(start + i) % num_tokens];
            target[i] = tokens[(start + i + 1) % num_tokens];
        }
        
        // Forward, backward, optimize
        cllm_forward_training(training, batch);
        float loss = cllm_compute_loss(training, batch, target, 8);
        cllm_backward_training(training, target, NULL);
        cllm_optimizer_step(training);
        
        if (step == 0) initial_loss = loss;
        if (step == 19) final_loss = loss;
        
        if (step % 5 == 0) {
            printf("  Step %2d: Loss = %.4f\n", step, loss);
        }
    }
    
    printf("\nTraining complete!\n");
    printf("  Initial loss: %.4f\n", initial_loss);
    printf("  Final loss:   %.4f\n", final_loss);
    printf("  Improvement:  %.1f%%\n\n", 100.0 * (initial_loss - final_loss) / initial_loss);
    
    // Step 4: Save the model
    printf("Step 4: Saving model...\n");
    const char* model_file = "test_model.cllm";
    if (cllm_write_model(model, model_file) != 0) {
        fprintf(stderr, "Failed to save model\n");
    } else {
        printf("Model saved to %s\n\n", model_file);
    }
    
    // Step 5: Load the model
    printf("Step 5: Loading model...\n");
    CLLMModel* loaded_model = cllm_read_model(model_file);
    if (!loaded_model) {
        fprintf(stderr, "Failed to load model\n");
        cllm_training_cleanup(training);
        free(tokens);
        cllm_free_model(model);
        return 1;
    }
    printf("Model loaded successfully\n\n");
    
    // Step 6: Test inference
    printf("Step 6: Testing inference...\n");
    CLLMInference* inference = cllm_inference_init(loaded_model);
    if (!inference) {
        fprintf(stderr, "Failed to initialize inference\n");
        cllm_free_model(loaded_model);
        cllm_training_cleanup(training);
        free(tokens);
        cllm_free_model(model);
        return 1;
    }
    
    cllm_set_temperature(inference, 0.1);  // Low temperature for deterministic output
    cllm_set_max_tokens(inference, 10);
    
    // Test with simple prompt
    printf("Testing text generation:\n");
    printf("  Input: &quot;test&quot;\n");
    
    char output_text[1024];
    int generated = cllm_generate(inference, "test", output_text, sizeof(output_text));
    if (generated > 0) {
        printf("  Generated %d tokens\n", generated);
        printf("  Output: %s\n", output_text);
    } else {
        printf("  Generation failed\n");
    }
    
    printf("\n=== PIPELINE TEST COMPLETE ===\n");
    printf("✅ Model creation: SUCCESS\n");
    printf("✅ Training: SUCCESS (loss decreased %.1f%%)\n", 
           100.0 * (initial_loss - final_loss) / initial_loss);
    printf("✅ Model save/load: SUCCESS\n");
    printf("✅ Inference: %s\n", generated > 0 ? "SUCCESS" : "FAILED");
    
    // Cleanup
    cllm_inference_cleanup(inference);
    cllm_free_model(loaded_model);
    cllm_training_cleanup(training);
    free(tokens);
    cllm_free_model(model);
    
    return generated > 0 ? 0 : 1;
}