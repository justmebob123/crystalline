#include "cllm_bigfixed.h"
#include "cllm.h"
#include "bigfixed_core.h"
#include "bigfixed_math_wrappers.h"
#include "numerical_bigfixed.h"
#include <stdlib.h>

/**
 * Feedforward forward pass using BigFixed arbitrary precision
 */
void cllm_feedforward_forward_bigfixed(
    FeedForwardLayer* layer,
    BigFixed** input,
    BigFixed** output,
    int batch_size,
    int precision
) {
    if (!layer || !input || !output || batch_size <= 0) return;
    
    uint32_t input_dim = layer->input_dim;
    uint32_t hidden_dim = layer->hidden_dim;
    
    // Allocate hidden layer buffer
    BigFixed** hidden = (BigFixed**)calloc(batch_size * hidden_dim, sizeof(BigFixed*));
    if (!hidden) return;
    
    for (int i = 0; i < batch_size * hidden_dim; i++) {
        hidden[i] = big_fixed_create(precision);
    }
    
    // First linear transformation: hidden = input * W1 + bias1
    matrix_multiply_bigfixed(
        input,
        layer->w1_lattice,
        hidden,
        batch_size,
        input_dim,
        hidden_dim,
        precision
    );
    
    // Add bias1
    for (int b = 0; b < batch_size; b++) {
        for (uint32_t h = 0; h < hidden_dim; h++) {
            int idx = b * hidden_dim + h;
            big_fixed_add(hidden[idx], hidden[idx], layer->bias1[h]);
        }
    }
    
    // Apply activation (tanh)
    for (int i = 0; i < batch_size * hidden_dim; i++) {
        bigfixed_tanh(hidden[i], hidden[i], precision);
    }
    
    // Second linear transformation: output = hidden * W2 + bias2
    matrix_multiply_bigfixed(
        hidden,
        layer->w2_lattice,
        output,
        batch_size,
        hidden_dim,
        input_dim,
        precision
    );
    
    // Add bias2
    for (int b = 0; b < batch_size; b++) {
        for (uint32_t d = 0; d < input_dim; d++) {
            int idx = b * input_dim + d;
            big_fixed_add(output[idx], output[idx], layer->bias2[d]);
        }
    }
    
    // Cleanup
    for (int i = 0; i < batch_size * hidden_dim; i++) {
        big_fixed_free(hidden[i]);
    }
    free(hidden);
}
