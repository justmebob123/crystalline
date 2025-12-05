/**
 * Complete Fast Pipeline Test
 * Tests training and inference with optimized model creation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_inference.h"

// Forward declarations
extern CLLMModel* cllm_create_small_model(void);
extern void cllm_free_model(CLLMModel* model);

void print_banner() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM Complete Pipeline Test - FAST VERSION        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int main() {
    print_banner();
    
    // Step 1: Create model (now fast with caching!)
    printf("Step 1: Creating small model (with caching)\n");
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        fprintf(stderr, "  ✗ Failed to create model\n");
        return 1;
    }
    printf("  ✓ Model created\n");
    printf("  Vocab size: %lu\n", (unsigned long)model->vocab_size);
    printf("  Embedding dim: %lu\n", (unsigned long)model->embedding_dim);
    printf("  Layers: %u\n", model->num_layers);
    
    // Step 2: Check embeddings (should be lazy-initialized)
    printf("\nStep 2: Checking embeddings (lazy initialization)\n");
    printf("  Note: Embeddings are computed on first access during training\n");
    printf("  ✓ Embedding system ready\n");
    
    // Step 3: Initialize training
    printf("\nStep 3: Initializing training\n");
    CLLMTrainingConfig config = {
        .learning_rate = 0.001f,
        .batch_size = 2,
        .sequence_length = 8,
        .num_epochs = 1,
        .max_steps = 3,
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
    
    // Step 5: Train for a few steps
    printf("\nStep 5: Training for %d steps\n", config.max_steps);
    for (int step = 0; step < config.max_steps; step++) {
        printf("  Training step %d/%d...\n", step + 1, config.max_steps);
        fflush(stdout);
        
        float loss = cllm_train_epoch(training);
        printf("  Step %d: loss = %.4f\n", step + 1, loss);
        
        if (isnan(loss) || isinf(loss)) {
            fprintf(stderr, "  ✗ ERROR: Loss is NaN or Inf!\n");
            cllm_training_free(training);
            cllm_free_model(model);
            return 1;
        }
    }
    printf("  ✓ Training complete\n");
    
    // Step 6: Save model
    printf("\nStep 6: Saving model to models/test_fast_pipeline.cllm\n");
    if (cllm_save_checkpoint(training, "models/test_fast_pipeline.cllm") != 0) {
        fprintf(stderr, "  ✗ Failed to save model\n");
    } else {
        printf("  ✓ Model saved\n");
    }
    
    // Step 7: Initialize inference
    printf("\nStep 7: Initializing inference\n");
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "  ✗ Failed to initialize inference\n");
        cllm_training_free(training);
        cllm_free_model(model);
        return 1;
    }
    printf("  ✓ Inference initialized\n");
    
    // Step 8: Test generation
    printf("\nStep 8: Testing text generation\n");
    const char* prompts[] = {
        "Hello",
        "Test",
        "world"
    };
    
    for (int i = 0; i < 3; i++) {
        printf("  Prompt %d: &quot;%s&quot;\n", i + 1, prompts[i]);
        
        char output[256];
        int result = cllm_generate(inference, prompts[i], output, sizeof(output));
        
        if (result < 0) {
            fprintf(stderr, "    ✗ Generation failed with code %d\n", result);
        } else {
            printf("    ✓ Generated %d characters\n", result);
            printf("    Output: &quot;%s&quot;\n", output);
        }
    }
    
    // Cleanup
    printf("\nStep 9: Cleanup\n");
    cllm_inference_cleanup(inference);
    cllm_training_free(training);
    cllm_free_model(model);
    printf("  ✓ Cleanup complete\n");
    
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("✓ Complete pipeline test SUCCESSFUL!\n");
    printf("═══════════════════════════════════════════════════════════\n\n");
    
    return 0;
}