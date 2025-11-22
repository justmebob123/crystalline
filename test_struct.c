#include <stdio.h>
#include "include/cllm.h"
#include "include/cllm_utils.h"

int main() {
    CLLMConfig config = {
        .vocab_size = 10,
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
    
    printf("Model created at %p\n", (void*)model);
    
    // Try to access fields one by one
    printf("Trying to access header...\n");
    printf("  header.magic[0] = %c\n", model->header.magic[0]);
    
    printf("Trying to access tokens...\n");
    printf("  tokens = %p\n", (void*)model->tokens);
    
    printf("Trying to access vocab_size...\n");
    printf("  vocab_size = %lu\n", (unsigned long)model->vocab_size);
    
    printf("Trying to access num_weights...\n");
    printf("  num_weights = %lu\n", (unsigned long)model->num_weights);
    
    printf("Trying to access embedding_dim...\n");
    printf("  embedding_dim = %lu\n", (unsigned long)model->embedding_dim);
    
    printf("All fields accessible!\n");
    
    return 0;
}