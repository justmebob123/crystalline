#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing model creation with minimal config...\n\n");
    
    // Start with absolute minimum
    CLLMConfig config = {
        .vocab_size = 100,
        .embedding_dim = 32,
        .num_layers = 1,
        .num_heads = 2,
        .ff_dim = 64,
        .max_seq_len = 16,
        .dropout = 0.1f
    };
    
    printf("Configuration:\n");
    printf("  vocab_size: %u\n", config.vocab_size);
    printf("  embedding_dim: %u\n", config.embedding_dim);
    printf("  num_layers: %u\n", config.num_layers);
    printf("  num_heads: %u\n", config.num_heads);
    printf("  ff_dim: %u\n", config.ff_dim);
    printf("  max_seq_len: %u\n", config.max_seq_len);
    printf("\n");
    
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_model(&config);
    
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    
    printf("✓ Model created successfully!\n");
    printf("  vocab_size: %lu\n", model->vocab_size);
    printf("  embedding_dim: %lu\n", model->embedding_dim);
    printf("  num_layers: %u\n", model->num_layers);
    printf("  num_weights: %lu\n", model->num_weights);
    printf("\n");
    
    // Cleanup
    cllm_free_model(model);
    printf("✓ Model freed successfully\n");
    
    return 0;
}
