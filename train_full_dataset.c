/**
 * Train CLLM on Full Dataset
 * 
 * Simple training script for the complete training dataset
 */

#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_format.h"
#include "include/cllm_data_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    printf("=== CLLM Full Dataset Training ===\n\n");
    
    const char* data_file = "data/full_training/complete_training.txt";
    const char* output_model = "checkpoints/full_trained_model.cllm";
    int epochs = 50;
    float learning_rate = 0.001f;
    
    if (argc > 1) data_file = argv[1];
    if (argc > 2) output_model = argv[2];
    if (argc > 3) epochs = atoi(argv[3]);
    if (argc > 4) learning_rate = atof(argv[4]);
    
    printf("Configuration:\n");
    printf("  Data file: %s\n", data_file);
    printf("  Output model: %s\n", output_model);
    printf("  Epochs: %d\n", epochs);
    printf("  Learning rate: %.6f\n\n", learning_rate);
    
    // Create model configuration
    CLLMConfig config = {
        .vocab_size = 500,
        .embedding_dim = 128,
        .num_layers = 4,
        .num_heads = 8,
        .ff_dim = 512,
        .max_seq_len = 128,
        .dropout = 0.1f
    };
    
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("✓ Model created\n\n");
    
    // Load training data
    printf("Loading training data...\n");
    CLLMDataset* dataset = cllm_load_dataset(data_file, model->vocab_size);
    if (!dataset) {
        fprintf(stderr, "Failed to load dataset\n");
        cllm_free(model);
        return 1;
    }
    printf("✓ Dataset loaded: %u sequences\n\n", dataset->num_sequences);
    
    // Create training context
    printf("Initializing training...\n");
    CLLMTraining* training = cllm_training_init(model, dataset);
    if (!training) {
        fprintf(stderr, "Failed to initialize training\n");
        cllm_free_dataset(dataset);
        cllm_free(model);
        return 1;
    }
    
    // Set training parameters
    training->learning_rate = learning_rate;
    training->num_epochs = epochs;
    training->batch_size = 8;
    training->checkpoint_interval = 10;
    
    printf("✓ Training initialized\n");
    printf("  Batch size: %u\n", training->batch_size);
    printf("  Checkpoint interval: %u epochs\n\n", training->checkpoint_interval);
    
    // Train the model
    printf("Starting training...\n");
    printf("========================================\n\n");
    
    int result = cllm_train(training);
    
    printf("\n========================================\n");
    if (result == 0) {
        printf("✓ Training completed successfully!\n\n");
        
        // Save the final model
        printf("Saving model to %s...\n", output_model);
        if (cllm_write_model(model, output_model) == 0) {
            printf("✓ Model saved successfully\n");
        } else {
            fprintf(stderr, "Failed to save model\n");
        }
    } else {
        fprintf(stderr, "Training failed with error code: %d\n", result);
    }
    
    // Cleanup
    cllm_training_cleanup(training);
    cllm_free_dataset(dataset);
    cllm_free(model);
    
    printf("\n=== Training Complete ===\n");
    return result;
}