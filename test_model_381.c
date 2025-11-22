#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing model creation with vocab_size=381...\n");
    
    CLLMConfig config = {
        .vocab_size = 381,
        .embedding_dim = 64,
        .num_layers = 2,
        .num_heads = 4,
        .ff_dim = 128,
        .max_seq_len = 32,
        .dropout = 0.1f
    };
    
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_model(&config);
    
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    
    printf("✓ Model created: %lu params\n", model->num_weights);
    
    cllm_free_model(model);
    printf("✓ Model freed\n");
    
    return 0;
}
