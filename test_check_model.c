#include "cllm.h"
#include "cllm_utils.h"
#include <stdio.h>

int main() {
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
    
    printf("Model pointers:\n");
    printf("  tokens: %p\n", (void*)model->tokens);
    printf("  lattice_points: %p\n", (void*)model->lattice_points);
    printf("  embeddings.embeddings: %p\n", (void*)model->embeddings.embeddings);
    printf("  embeddings.lattice_transform: %p\n", (void*)model->embeddings.lattice_transform);
    printf("  embeddings.inverse_transform: %p\n", (void*)model->embeddings.inverse_transform);
    printf("  attention_layers: %p\n", (void*)model->attention_layers);
    printf("  ff_layers: %p\n", (void*)model->ff_layers);
    printf("  layer_norms: %p\n", (void*)model->layer_norms);
    
    if (model->attention_layers) {
        printf("  attention_layers[0].query_lattice: %p\n", (void*)model->attention_layers[0].query_lattice);
        printf("  attention_layers[0].key_lattice: %p\n", (void*)model->attention_layers[0].key_lattice);
        printf("  attention_layers[0].value_lattice: %p\n", (void*)model->attention_layers[0].value_lattice);
    }
    
    if (model->ff_layers) {
        printf("  ff_layers[0].w1_lattice: %p\n", (void*)model->ff_layers[0].w1_lattice);
        printf("  ff_layers[0].w2_lattice: %p\n", (void*)model->ff_layers[0].w2_lattice);
        printf("  ff_layers[0].bias1: %p\n", (void*)model->ff_layers[0].bias1);
        printf("  ff_layers[0].bias2: %p\n", (void*)model->ff_layers[0].bias2);
    }
    
    if (model->layer_norms) {
        printf("  layer_norms[0].gamma: %p\n", (void*)model->layer_norms[0].gamma);
        printf("  layer_norms[0].beta: %p\n", (void*)model->layer_norms[0].beta);
    }
    
    printf("  pos_encoding.spiral_positions: %p\n", (void*)model->pos_encoding.spiral_positions);
    printf("  pos_encoding.clock_positions: %p\n", (void*)model->pos_encoding.clock_positions);
    printf("  pos_encoding.prime_positions: %p\n", (void*)model->pos_encoding.prime_positions);
    printf("  pos_encoding.learned_positions: %p\n", (void*)model->pos_encoding.learned_positions);
    
    return 0;
}
