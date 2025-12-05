/**
 * Complete Pipeline Test with Valgrind Support
 * 
 * Tests training and inference with proper memory checking
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_inference.h"

void print_banner() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM Pipeline Test - Valgrind Ready               ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int main() {
    print_banner();
    
    // Step 1: Create a small model
    printf("Step 1: Creating small model\n");
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        fprintf(stderr, "  ✗ Failed to create model\n");
        return 1;
    }
    printf("  ✓ Model created\n");
    printf("  Vocab size: %lu\n", (unsigned long)model->vocab_size);
    printf("  Embedding dim: %lu\n", (unsigned long)model->embedding_dim);
    printf("  Layers: %d\n", model->num_layers);
    
    // Step 2: Check initial embeddings
    printf("\nStep 2: Checking initial embeddings\n");
    double sum = 0.0;
    double sum_sq = 0.0;
    int count = 0;
    int check_count = 100;
    if (check_count > (int)(model->vocab_size * model->embedding_dim)) {
        check_count = (int)(model->vocab_size * model->embedding_dim);
    }
    
    for (int i = 0; i < check_count; i++) {
        double val = model->embeddings.embeddings[i];
        sum += val;
        sum_sq += val * val;
        count++;
    }
    double mean = sum / count;
    double variance = (sum_sq / count) - (mean * mean);
    double stddev = sqrt(variance);
    printf("  Mean: %.6f\n", mean);
    printf("  StdDev: %.6f\n", stddev);
    
    if (mean == 0.0 && stddev == 0.0) {
        fprintf(stderr, "  ✗ ERROR: All embeddings are zero!\n");
        fprintf(stderr, "  This means prime_encoding was not initialized!\n");
        cllm_free_model(model);
        return 1;
    }
    printf("  ✓ Embeddings initialized properly\n");
    
    // Step 3: Initialize training
    printf("\nStep 3: Initializing training\n");
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 2,
        .max_steps = 5,
        .gradient_clip = 1.0f,
        .weight_decay = 0.01f,
        .warmup_steps = 0,
        .save_every = 10
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "  ✗ Failed to initialize training\n");
        cllm_free_model(model);
        return 1;
    }
    printf("  ✓ Training initialized\n");
    
    // Step 4: Load training data
    printf("\nStep 4: Loading training data from data/tiny.txt\n");
    if (cllm_load_training_data(training, "data/tiny.txt") != 0) {
        fprintf(stderr, "  ✗ Failed to load training data\n");
        cllm_training_free(training);
        cllm_free_model(model);
        return 1;
    }
    printf("  ✓ Training data loaded\n");
    
    // Step 5: Train for a few epochs
    printf("\nStep 5: Training for %d epochs\n", config.num_epochs);
    for (int epoch = 0; epoch < config.num_epochs; epoch++) {
        float loss = cllm_train_epoch(training);
        printf("  Epoch %d: loss = %.4f\n", epoch + 1, loss);
        
        if (isnan(loss) || isinf(loss)) {
            fprintf(stderr, "  ✗ ERROR: Loss is NaN or Inf!\n");
            cllm_training_free(training);
            cllm_free_model(model);
            return 1;
        }
    }
    
    // Step 6: Save model
    printf("\nStep 6: Saving model to models/test_pipeline.cllm\n");
    if (cllm_save_checkpoint(training, "models/test_pipeline.cllm") != 0) {
        fprintf(stderr, "  ✗ Failed to save model\n");
    } else {
        printf("  ✓ Model saved\n");
    }
    
    // Step 7: Check final embeddings
    printf("\nStep 7: Checking final embeddings\n");
    sum = 0.0;
    sum_sq = 0.0;
    count = 0;
    for (int i = 0; i < check_count; i++) {
        double val = model->embeddings.embeddings[i];
        sum += val;
        sum_sq += val * val;
        count++;
    }
    mean = sum / count;
    variance = (sum_sq / count) - (mean * mean);
    stddev = sqrt(variance);
    printf("  Mean: %.6f\n", mean);
    printf("  StdDev: %.6f\n", stddev);
    
    // Step 8: Initialize inference
    printf("\nStep 8: Initializing inference\n");
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "  ✗ Failed to initialize inference\n");
        cllm_training_free(training);
        cllm_free_model(model);
        return 1;
    }
    printf("  ✓ Inference initialized\n");
    
    // Step 9: Test generation
    printf("\nStep 9: Testing text generation\n");
    const char* prompt = "Hello";
    printf("  Prompt: &quot;%s&quot;\n", prompt);
    
    char output[256];
    int result = cllm_generate(inference, prompt, output, sizeof(output));
    
    if (result < 0) {
        fprintf(stderr, "  ✗ Generation failed with code %d\n", result);
    } else {
        printf("  ✓ Generated %d characters\n", result);
        printf("  Output: &quot;%s&quot;\n", output);
    }
    
    // Step 10: Test another prompt
    printf("\nStep 10: Testing another prompt\n");
    const char* prompt2 = "Test";
    printf("  Prompt: &quot;%s&quot;\n", prompt2);
    
    result = cllm_generate(inference, prompt2, output, sizeof(output));
    
    if (result < 0) {
        fprintf(stderr, "  ✗ Generation failed with code %d\n", result);
    } else {
        printf("  ✓ Generated %d characters\n", result);
        printf("  Output: &quot;%s&quot;\n", output);
    }
    
    // Cleanup
    printf("\nStep 11: Cleanup\n");
    cllm_inference_cleanup(inference);
    cllm_training_free(training);
    cllm_free_model(model);
    printf("  ✓ Cleanup complete\n");
    
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("Pipeline test complete!\n");
    printf("═══════════════════════════════════════════════════════════\n\n");
    
    return 0;
}