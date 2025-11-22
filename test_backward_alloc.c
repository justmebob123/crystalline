#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing backward pass allocation sizes...\n\n");
    
    CLLMConfig config = {
        .vocab_size = 381,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 32,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    printf("Model created\n");
    printf("  embedding_dim (uint64_t): %lu\n", model->embedding_dim);
    printf("  embedding_dim (as size_t): %zu\n", (size_t)model->embedding_dim);
    
    int batch_size = 2;
    int seq_len = 8;
    
    uint64_t embed_dim = model->embedding_dim;
    size_t activation_size = batch_size * seq_len * embed_dim;
    
    printf("\nAllocation calculations:\n");
    printf("  batch_size: %d\n", batch_size);
    printf("  seq_len: %d\n", seq_len);
    printf("  embed_dim: %lu\n", embed_dim);
    printf("  activation_size: %zu\n", activation_size);
    printf("  activation_size in bytes: %zu\n", activation_size * sizeof(float));
    printf("  activation_size in MB: %.2f\n", (activation_size * sizeof(float)) / (1024.0 * 1024.0));
    
    printf("\nAttempting allocations...\n");
    
    printf("  Allocating embeddings (%zu floats)...\n", activation_size);
    float* embeddings = (float*)calloc(activation_size, sizeof(float));
    if (!embeddings) {
        printf("    FAILED\n");
        return 1;
    }
    printf("    OK\n");
    
    printf("  Allocating grad_output (%zu floats)...\n", activation_size);
    float* grad_output = (float*)calloc(activation_size, sizeof(float));
    if (!grad_output) {
        printf("    FAILED\n");
        free(embeddings);
        return 1;
    }
    printf("    OK\n");
    
    printf("  Allocating layer_input (%lu floats)...\n", embed_dim);
    float* layer_input = (float*)calloc(embed_dim, sizeof(float));
    if (!layer_input) {
        printf("    FAILED\n");
        free(embeddings);
        free(grad_output);
        return 1;
    }
    printf("    OK\n");
    
    printf("\n✓ All allocations successful\n");
    
    free(embeddings);
    free(grad_output);
    free(layer_input);
    cllm_free_model(model);
    
    return 0;
}
