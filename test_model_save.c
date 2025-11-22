#include "cllm.h"
#include "cllm_format.h"
#include "cllm_utils.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Creating model...\n");
    
    CLLMConfig config = {
        .vocab_size = 10000,
        .embedding_dim = 512,
        .num_layers = 6,
        .num_heads = 8,
        .ff_dim = 2048,
        .max_seq_len = 512,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    
    printf("Model created successfully\n");
    printf("Model weights pointer: %p\n", (void*)model->weights);
    printf("Model num_weights: %lu\n", model->num_weights);
    
    // Create models directory
    system("mkdir -p models");
    
    printf("Saving model to models/test_model.cllm...\n");
    int result = cllm_write_model(model, "models/test_model.cllm");
    
    if (result == 0) {
        printf("Model saved successfully\n");
    } else {
        printf("Model save failed\n");
    }
    
    return 0;
}