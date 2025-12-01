#include "cllm_bigfixed.h"
#include "cllm.h"
#include "bigfixed_core.h"
#include "numerical_bigfixed.h"
#include <stdlib.h>

/**
 * Layer normalization using BigFixed arbitrary precision
 */
void cllm_layernorm_forward_bigfixed(
    LayerNorm* layer,
    BigFixed** input,
    BigFixed** output,
    int batch_size,
    int precision
) {
    if (!layer || !input || !output || batch_size <= 0) return;
    
    uint32_t size = layer->size;
    
    // Apply layer normalization for each sample in batch
    for (int b = 0; b < batch_size; b++) {
        BigFixed** input_vec = &input[b * size];
        BigFixed** output_vec = &output[b * size];
        
        layer_norm_bigfixed(
            input_vec,
            output_vec,
            layer->gamma,
            layer->beta,
            size,
            precision
        );
    }
}
