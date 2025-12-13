#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_format.h"

int main() {
    printf("Testing transformer layer initialization...\n\n");
    
    // Load the model
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    
    printf("Model loaded successfully\n");
    printf("Number of layers: %u\n", model->num_layers);
    printf("Embedding dim: %u\n", model->embedding_dim);
    printf("Hidden dim: %u\n", model->hidden_dim);
    
    // Check if layers array exists
    if (model->layers) {
        printf("✓ Layers array allocated\n");
        
        // Check first layer
        printf("\nChecking first layer weights:\n");
        printf("  query_weights: %s\n", model->layers[0].query_weights ? "✓ allocated" : "✗ NULL");
        printf("  key_weights: %s\n", model->layers[0].key_weights ? "✓ allocated" : "✗ NULL");
        printf("  value_weights: %s\n", model->layers[0].value_weights ? "✓ allocated" : "✗ NULL");
        printf("  output_weights: %s\n", model->layers[0].output_weights ? "✓ allocated" : "✗ NULL");
        printf("  ffn_w1: %s\n", model->layers[0].ffn_w1 ? "✓ allocated" : "✗ NULL");
        printf("  ffn_w2: %s\n", model->layers[0].ffn_w2 ? "✓ allocated" : "✗ NULL");
        printf("  ln1_gamma: %s\n", model->layers[0].ln1_gamma ? "✓ allocated" : "✗ NULL");
        printf("  ln1_beta: %s\n", model->layers[0].ln1_beta ? "✓ allocated" : "✗ NULL");
        printf("  ln2_gamma: %s\n", model->layers[0].ln2_gamma ? "✓ allocated" : "✗ NULL");
        printf("  ln2_beta: %s\n", model->layers[0].ln2_beta ? "✓ allocated" : "✗ NULL");
    } else {
        printf("✗ Layers array is NULL\n");
    }
    
    cllm_free_model(model);
    return 0;
}