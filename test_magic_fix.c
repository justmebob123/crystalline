#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cllm.h"
#include "cllm_format.h"
#include "cllm_model_manager.h"

int main() {
    printf("Testing model save/load with correct magic number...\n\n");
    
    // Create a simple model
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) {
        fprintf(stderr, "Failed to allocate model\n");
        return 1;
    }
    
    // Initialize basic parameters
    model->vocab_size = 100;
    model->embedding_dim = 64;
    model->num_layers = 2;
    model->num_weights = model->vocab_size * model->embedding_dim;
    
    // Allocate embeddings
    model->embeddings.vocab_size = model->vocab_size;
    model->embeddings.embedding_dim = model->embedding_dim;
    model->embeddings.embeddings = (double*)calloc(model->vocab_size * model->embedding_dim, sizeof(double));
    
    // Initialize with some test values
    for (size_t i = 0; i < model->vocab_size * model->embedding_dim; i++) {
        model->embeddings.embeddings[i] = (double)i / 1000.0;
    }
    
    // Save the model
    const char* test_file = "test_magic_number.cllm";
    printf("Saving model to %s...\n", test_file);
    if (cllm_write_model(model, test_file) != 0) {
        fprintf(stderr, "Failed to save model\n");
        free(model->embeddings.embeddings);
        free(model);
        return 1;
    }
    printf("✓ Model saved successfully\n\n");
    
    // Check the magic number in the file
    FILE* f = fopen(test_file, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open saved file\n");
        free(model->embeddings.embeddings);
        free(model);
        return 1;
    }
    
    unsigned char magic[8];
    fread(magic, 1, 8, f);
    fclose(f);
    
    printf("Magic number in file: ");
    for (int i = 0; i < 8; i++) {
        printf("%02X ", magic[i]);
    }
    printf("\n");
    
    printf("Expected magic:       43 4C 4C 4D 01 00 00 00\n\n");
    
    // Verify magic number
    if (memcmp(magic, "CLLM\x01\x00\x00\x00", 8) == 0) {
        printf("✓ Magic number is CORRECT!\n\n");
    } else {
        printf("✗ Magic number is INCORRECT!\n\n");
        free(model->embeddings.embeddings);
        free(model);
        return 1;
    }
    
    // Now try to load it with model_manager
    printf("Testing model_manager_load...\n");
    ManagedModel* managed = model_manager_load("test_magic_number", test_file);
    if (!managed) {
        fprintf(stderr, "✗ Failed to load model with model_manager\n");
        free(model->embeddings.embeddings);
        free(model);
        return 1;
    }
    printf("✓ Model loaded successfully with model_manager!\n\n");
    
    // Verify the loaded model
    if (managed->model) {
        printf("Loaded model details:\n");
        printf("  vocab_size: %lu\n", managed->model->vocab_size);
        printf("  embedding_dim: %lu\n", managed->model->embedding_dim);
        printf("  num_layers: %u\n", managed->model->num_layers);
        printf("\n✓ All tests passed!\n");
    }
    
    // Cleanup
    free(model->embeddings.embeddings);
    free(model);
    
    return 0;
}
