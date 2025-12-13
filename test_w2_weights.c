#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "math/include/math/arithmetic.h"

int main() {
    printf("=== Testing W2 Weights ===\n\n");
    
    // Load model
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    printf("✓ Model loaded\n");
    printf("  embed_dim=%u, hidden_dim=%u\n\n", model->embedding_dim, model->hidden_dim);
    
    // Check w2 weights
    printf("Checking first layer w2 weights:\n");
    printf("  w2 pointer: %p\n", (void*)model->layers[0].ffn_w2);
    
    if (model->layers[0].ffn_w2) {
        int nan_count = 0;
        int total_checked = 100;
        
        printf("\n  First 20 w2 values:\n");
        for (int i = 0; i < 20; i++) {
            double val = model->layers[0].ffn_w2[i];
            printf("    w2[%d] = %.6f %s\n", i, val,
                   math_is_nan(val) ? "(NaN!)" : "");
            if (math_is_nan(val)) nan_count++;
        }
        
        // Check more values
        for (int i = 20; i < total_checked; i++) {
            if (math_is_nan(model->layers[0].ffn_w2[i])) {
                nan_count++;
            }
        }
        
        printf("\n  NaN count in first %d values: %d\n", total_checked, nan_count);
        
        // Expected size
        size_t expected_size = model->hidden_dim * model->embedding_dim;
        printf("  Expected w2 size: %zu (hidden_dim=%u * embed_dim=%u)\n", 
               expected_size, model->hidden_dim, model->embedding_dim);
    }
    
    cllm_free_model(model);
    return 0;
}