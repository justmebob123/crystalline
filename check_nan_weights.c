#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "math/include/math/arithmetic.h"

int main() {
    printf("=== Checking for NaN in Weights ===\n\n");
    
    // Load model
    CLLMModel* model = cllm_read_model("test_checkpoints/final_model.cllm");
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    
    uint32_t embed_dim = model->embedding_dim;
    uint32_t hidden_dim = model->hidden_dim;
    
    printf("Checking w1 weights for NaN:\n");
    int nan_count_w1 = 0;
    size_t w1_size = embed_dim * hidden_dim;
    for (size_t i = 0; i < w1_size; i++) {
        if (math_is_nan(model->layers[0].ffn_w1[i])) {
            nan_count_w1++;
            if (nan_count_w1 <= 10) {
                printf("  w1[%zu] = NaN\n", i);
                // Find which hidden unit this corresponds to
                size_t hidden_idx = i % hidden_dim;
                size_t embed_idx = i / hidden_dim;
                printf("    (embed_idx=%zu, hidden_idx=%zu)\n", embed_idx, hidden_idx);
            }
        }
    }
    printf("  Total NaN in w1: %d / %zu\n\n", nan_count_w1, w1_size);
    
    printf("Checking w2 weights for NaN:\n");
    int nan_count_w2 = 0;
    size_t w2_size = hidden_dim * embed_dim;
    for (size_t i = 0; i < w2_size; i++) {
        if (math_is_nan(model->layers[0].ffn_w2[i])) {
            nan_count_w2++;
            if (nan_count_w2 <= 10) {
                printf("  w2[%zu] = NaN\n", i);
            }
        }
    }
    printf("  Total NaN in w2: %d / %zu\n\n", nan_count_w2, w2_size);
    
    printf("Checking query_weights for NaN:\n");
    int nan_count_q = 0;
    size_t q_size = embed_dim * embed_dim;
    for (size_t i = 0; i < q_size; i++) {
        if (math_is_nan(model->layers[0].query_weights[i])) {
            nan_count_q++;
        }
    }
    printf("  Total NaN in query_weights: %d / %zu\n\n", nan_count_q, q_size);
    
    printf("Checking embeddings for NaN:\n");
    int nan_count_emb = 0;
    size_t emb_size = model->vocab_size * embed_dim;
    for (size_t i = 0; i < emb_size; i++) {
        if (math_is_nan(model->embeddings[i])) {
            nan_count_emb++;
        }
    }
    printf("  Total NaN in embeddings: %d / %zu\n\n", nan_count_emb, emb_size);
    
    cllm_free_model(model);
    return 0;
}
