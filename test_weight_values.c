#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_format.h"

int main() {
    printf("Loading model...\n");
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    
    printf("Model loaded successfully\n");
    printf("Vocab size: %u\n", model->vocab_size);
    printf("Embedding dim: %u\n", model->embedding_dim);
    printf("Hidden dim: %u\n", model->hidden_dim);
    printf("Num layers: %u\n", model->num_layers);
    
    // Check embeddings for NaN
    printf("\nChecking embeddings...\n");
    int nan_count = 0;
    int zero_count = 0;
    for (size_t i = 0; i < model->vocab_size * model->embedding_dim; i++) {
        if (isnan(model->embeddings[i])) {
            nan_count++;
        }
        if (model->embeddings[i] == 0.0) {
            zero_count++;
        }
    }
    printf("Embeddings: %d NaN values, %d zero values out of %zu total\n", 
           nan_count, zero_count, (size_t)(model->vocab_size * model->embedding_dim));
    
    // Check first few embedding values
    printf("First 10 embedding values: ");
    for (int i = 0; i < 10 && i < model->vocab_size * model->embedding_dim; i++) {
        printf("%.6f ", model->embeddings[i]);
    }
    printf("\n");
    
    // Check layer 0 weights
    if (model->num_layers > 0) {
        printf("\nChecking Layer 0 weights...\n");
        
        // Check FFN w1
        nan_count = 0;
        zero_count = 0;
        size_t w1_size = model->embedding_dim * model->hidden_dim;
        for (size_t i = 0; i < w1_size; i++) {
            if (isnan(model->layers[0].ffn_w1[i])) {
                nan_count++;
            }
            if (model->layers[0].ffn_w1[i] == 0.0) {
                zero_count++;
            }
        }
        printf("FFN w1: %d NaN values, %d zero values out of %zu total\n", 
               nan_count, zero_count, w1_size);
        
        printf("First 10 FFN w1 values: ");
        for (int i = 0; i < 10 && i < w1_size; i++) {
            printf("%.6f ", model->layers[0].ffn_w1[i]);
        }
        printf("\n");
        
        // Check FFN w2
        nan_count = 0;
        zero_count = 0;
        size_t w2_size = model->hidden_dim * model->embedding_dim;
        for (size_t i = 0; i < w2_size; i++) {
            if (isnan(model->layers[0].ffn_w2[i])) {
                nan_count++;
            }
            if (model->layers[0].ffn_w2[i] == 0.0) {
                zero_count++;
            }
        }
        printf("FFN w2: %d NaN values, %d zero values out of %zu total\n", 
               nan_count, zero_count, w2_size);
        
        printf("First 10 FFN w2 values: ");
        for (int i = 0; i < 10 && i < w2_size; i++) {
            printf("%.6f ", model->layers[0].ffn_w2[i]);
        }
        printf("\n");
        
        // Check FFN biases
        nan_count = 0;
        zero_count = 0;
        for (size_t i = 0; i < model->hidden_dim; i++) {
            if (isnan(model->layers[0].ffn_b1[i])) {
                nan_count++;
            }
            if (model->layers[0].ffn_b1[i] == 0.0) {
                zero_count++;
            }
        }
        printf("FFN b1: %d NaN values, %d zero values out of %u total\n", 
               nan_count, zero_count, model->hidden_dim);
        
        printf("First 10 FFN b1 values: ");
        for (int i = 0; i < 10 && i < model->hidden_dim; i++) {
            printf("%.6f ", model->layers[0].ffn_b1[i]);
        }
        printf("\n");
        
        // Check layer norm parameters
        nan_count = 0;
        zero_count = 0;
        for (size_t i = 0; i < model->embedding_dim; i++) {
            if (isnan(model->layers[0].ln1_gamma[i])) {
                nan_count++;
            }
            if (model->layers[0].ln1_gamma[i] == 0.0) {
                zero_count++;
            }
        }
        printf("LN1 gamma: %d NaN values, %d zero values out of %u total\n", 
               nan_count, zero_count, model->embedding_dim);
        
        printf("First 10 LN1 gamma values: ");
        for (int i = 0; i < 10 && i < model->embedding_dim; i++) {
            printf("%.6f ", model->layers[0].ln1_gamma[i]);
        }
        printf("\n");
    }
    
    cllm_free_model(model);
    return 0;
}