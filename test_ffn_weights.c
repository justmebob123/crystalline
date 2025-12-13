#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "math/include/math/arithmetic.h"

int main() {
    printf("=== Testing FFN Weights ===\n\n");
    
    // Load model
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    printf("✓ Model loaded\n\n");
    
    // Check FFN biases
    printf("Checking first layer FFN biases:\n");
    printf("  b1 pointer: %p\n", (void*)model->layers[0].ffn_b1);
    printf("  b2 pointer: %p\n", (void*)model->layers[0].ffn_b2);
    
    if (model->layers[0].ffn_b1) {
        printf("\n  b1 (first 10 values):\n");
        for (int i = 0; i < 10; i++) {
            printf("    b1[%d] = %.6f %s\n", i, model->layers[0].ffn_b1[i],
                   math_is_nan(model->layers[0].ffn_b1[i]) ? "(NaN!)" : "");
        }
    }
    
    if (model->layers[0].ffn_b2) {
        printf("\n  b2 (first 10 values):\n");
        for (int i = 0; i < 10; i++) {
            printf("    b2[%d] = %.6f %s\n", i, model->layers[0].ffn_b2[i],
                   math_is_nan(model->layers[0].ffn_b2[i]) ? "(NaN!)" : "");
        }
    }
    
    cllm_free_model(model);
    return 0;
}