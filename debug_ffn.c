#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "include/cllm_inference.h"
#include "math/include/math/arithmetic.h"

int main() {
    printf("=== Debugging FFN Matrix Dimensions ===\n\n");
    
    // Load model
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    
    printf("Model dimensions:\n");
    printf("  embedding_dim = %u\n", model->embedding_dim);
    printf("  hidden_dim = %u\n", model->hidden_dim);
    printf("  vocab_size = %u\n", model->vocab_size);
    printf("\n");
    
    // Check FFN weight dimensions
    printf("FFN Layer 0 expected dimensions:\n");
    printf("  w1: [embed_dim × hidden_dim] = [%u × %u] = %u elements\n", 
           model->embedding_dim, model->hidden_dim, 
           model->embedding_dim * model->hidden_dim);
    printf("  b1: [hidden_dim] = [%u] = %u elements\n", 
           model->hidden_dim, model->hidden_dim);
    printf("  w2: [hidden_dim × embed_dim] = [%u × %u] = %u elements\n", 
           model->hidden_dim, model->embedding_dim,
           model->hidden_dim * model->embedding_dim);
    printf("  b2: [embed_dim] = [%u] = %u elements\n", 
           model->embedding_dim, model->embedding_dim);
    printf("\n");
    
    // Test a simple forward pass through FFN manually
    printf("Testing FFN forward pass manually:\n");
    uint32_t embed_dim = model->embedding_dim;
    uint32_t hidden_dim = model->hidden_dim;
    
    // Create a simple input vector (all 0.1)
    double* input = (double*)malloc(embed_dim * sizeof(double));
    for (uint32_t i = 0; i < embed_dim; i++) {
        input[i] = 0.1;
    }
    
    // First layer: input -> hidden
    double* hidden = (double*)calloc(hidden_dim, sizeof(double));
    const double* w1 = model->layers[0].ffn_w1;
    const double* b1 = model->layers[0].ffn_b1;
    
    printf("  Computing hidden = W1 * input + b1\n");
    for (uint32_t i = 0; i < hidden_dim; i++) {
        hidden[i] = b1[i];
        for (uint32_t j = 0; j < embed_dim; j++) {
            // Check the indexing: w1[j * hidden_dim + i]
            size_t idx = j * hidden_dim + i;
            if (idx >= embed_dim * hidden_dim) {
                printf("  ERROR: Index out of bounds! idx=%zu, max=%u\n", 
                       idx, embed_dim * hidden_dim);
                break;
            }
            hidden[i] += input[j] * w1[idx];
        }
        
        if (i < 5) {
            printf("    hidden[%u] = %.6f %s\n", i, hidden[i],
                   math_is_nan(hidden[i]) ? "(NaN!)" : "");
        }
    }
    
    printf("\n  Checking for NaN in hidden layer:\n");
    int nan_count = 0;
    for (uint32_t i = 0; i < hidden_dim; i++) {
        if (math_is_nan(hidden[i])) {
            nan_count++;
            if (nan_count <= 5) {
                printf("    hidden[%u] = NaN\n", i);
            }
        }
    }
    printf("  Total NaN count: %d / %u\n", nan_count, hidden_dim);
    
    free(input);
    free(hidden);
    cllm_free_model(model);
    return 0;
}
