#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "math/include/math/arithmetic.h"

int main() {
    printf("=== Testing Model Weights ===\n\n");
    
    // Load model
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    printf("✓ Model loaded\n\n");
    
    // Check embeddings
    printf("Checking embeddings (first 10 values):\n");
    for (int i = 0; i < 10; i++) {
        printf("  embedding[%d] = %.6f %s\n", i, model->embeddings[i],
               math_is_nan(model->embeddings[i]) ? "(NaN!)" : "");
    }
    
    // Check first layer weights
    printf("\nChecking first layer query weights (first 10 values):\n");
    for (int i = 0; i < 10; i++) {
        printf("  query_weights[%d] = %.6f %s\n", i, model->layers[0].query_weights[i],
               math_is_nan(model->layers[0].query_weights[i]) ? "(NaN!)" : "");
    }
    
    printf("\nChecking first layer ffn_w1 (first 10 values):\n");
    for (int i = 0; i < 10; i++) {
        printf("  ffn_w1[%d] = %.6f %s\n", i, model->layers[0].ffn_w1[i],
               math_is_nan(model->layers[0].ffn_w1[i]) ? "(NaN!)" : "");
    }
    
    printf("\nChecking first layer ln1_gamma (first 10 values):\n");
    for (int i = 0; i < 10; i++) {
        printf("  ln1_gamma[%d] = %.6f %s\n", i, model->layers[0].ln1_gamma[i],
               math_is_nan(model->layers[0].ln1_gamma[i]) ? "(NaN!)" : "");
    }
    
    cllm_free_model(model);
    return 0;
}