#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cllm.h"
#include "cllm_inference.h"

int main() {
    AttentionLayer layer;
    uint32_t num_heads = 2;
    uint32_t head_dim = 32;
    uint32_t seq_len = 4;
    uint32_t embedding_dim = num_heads * head_dim;
    
    cllm_attention_init(&layer, num_heads, head_dim);
    
    // Initialize weights to identity
    size_t weight_size = num_heads * head_dim * head_dim;
    for (size_t i = 0; i < weight_size; i++) {
        size_t row = i / head_dim;
        size_t col = i % head_dim;
        float val = (row == col) ? 1.0f : 0.0f;
        layer.query_lattice[i] = val;
        layer.key_lattice[i] = val;
        layer.value_lattice[i] = val;
    }
    
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* key_cache = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* value_cache = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    // Set input to simple values
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        input[i] = 1.0f;
    }
    
    printf("Before attention:\n");
    printf("  input[0] = %f\n", input[0]);
    printf("  key_cache[0] = %f\n", key_cache[0]);
    printf("  value_cache[0] = %f\n", value_cache[0]);
    
    cllm_attention_forward(&layer, input, output, key_cache, value_cache, seq_len);
    
    printf("\nAfter attention:\n");
    printf("  output[0] = %f\n", output[0]);
    printf("  key_cache[0] = %f\n", key_cache[0]);
    printf("  value_cache[0] = %f\n", value_cache[0]);
    
    float key_sum = 0.0f, value_sum = 0.0f;
    for (uint32_t i = 0; i < seq_len * embedding_dim; i++) {
        key_sum += fabsf(key_cache[i]);
        value_sum += fabsf(value_cache[i]);
    }
    printf("\nCache sums:\n");
    printf("  key_sum = %f\n", key_sum);
    printf("  value_sum = %f\n", value_sum);
    
    free(input);
    free(output);
    free(key_cache);
    free(value_cache);
    cllm_attention_free(&layer);
    
    return 0;
}
