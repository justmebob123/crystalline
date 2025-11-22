#include <stdio.h>
#include "include/cllm.h"
#include "include/cllm_utils.h"

int main() {
    printf("Testing layer norms initialization...\n\n");
    
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
    printf("  num_layers: %u\n", model->num_layers);
    printf("  layer_norms pointer: %p\n", (void*)model->layer_norms);
    
    if (model->layer_norms) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            printf("\nLayer %u:\n", i);
            printf("  layer_id: %u\n", model->layer_norms[i].layer_id);
            printf("  dim: %u\n", model->layer_norms[i].dim);
            printf("  epsilon: %f\n", model->layer_norms[i].epsilon);
            printf("  gamma: %p\n", (void*)model->layer_norms[i].gamma);
            printf("  beta: %p\n", (void*)model->layer_norms[i].beta);
        }
    } else {
        printf("ERROR: layer_norms is NULL\n");
    }
    
    cllm_free_model(model);
    printf("\n✓ Test complete\n");
    
    return 0;
}
