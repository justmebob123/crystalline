/*
 * CLLM Feed-Forward Network
 * Implements position-wise feed-forward networks for transformer layers
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/prime_float_math.h"
#include "../include/cllm_simd_utils.h"
#include "bigfixed_core.h"
#include "bigfixed_array_utils.h"

// Forward declaration
void cllm_feedforward_free(FeedForwardLayer* layer);

/**
 * GELU activation function
 * GELU(x) = x * Phi(x) where Phi is the cumulative distribution function of the standard normal
 * Approximation: GELU(x) ≈ 0.5 * x * (1 + tanh(sqrt(2/π) * (x + 0.044715 * x^3)))
 * 
 * @param x Input value
 * @return GELU(x)
 */
static float gelu(float x) {
    const float sqrt_2_over_pi = 0.7978845608f; // sqrt(2/π)
    const float coeff = 0.044715f;
    
    float x_cubed = x * x * x;
    float inner = sqrt_2_over_pi * (x + coeff * x_cubed);
    
    // tanh approximation
    float tanh_val;
    if (inner > 5.0f) {
        tanh_val = 1.0f;
    } else if (inner < -5.0f) {
        tanh_val = -1.0f;
    } else {
        float exp_2x = prime_exp(2.0f * inner);
        tanh_val = (exp_2x - 1.0f) / (exp_2x + 1.0f);
    }
    
    return 0.5f * x * (1.0f + tanh_val);
}

/**
 * Apply GELU activation to array
 * 
 * @param x Input/output array
 * @param size Array size
 */
void cllm_activation_gelu(float* x, int size) {
    if (!x || size <= 0) return;
    
    for (int i = 0; i < size; i++) {
        x[i] = gelu(x[i]);
    }
}

/**
 * ReLU activation function
 * 
 * @param x Input/output array
 * @param size Array size
 */
void cllm_activation_relu(float* x, int size) {
    if (!x || size <= 0) return;
    
    for (int i = 0; i < size; i++) {
        if (x[i] < 0.0f) {
            x[i] = 0.0f;
        }
    }
}

/**
 * Feed-forward network forward pass
 * 
 * FFN(x) = W2 * GELU(W1 * x + b1) + b2
 * 
 * @param layer Feed-forward layer parameters
 * @param input Input vector [input_dim]
 * @param output Output vector [output_dim]
 */

/**
 * Feed-forward network forward pass (in-place)
 * Note: Only works when input_dim == output_dim
 * 
 * @param layer Feed-forward layer parameters
 * @param data Input/output vector [input_dim]
 */
void cllm_feedforward_inplace(FeedForwardLayer* layer, float* data) {
    if (!layer || !data) return;
    
    if (layer->input_dim != layer->output_dim) {
        // Cannot do in-place if dimensions don't match
        return;
    }
    
    float* temp = (float*)malloc(layer->input_dim * sizeof(float));
    if (!temp) return;
    
    memcpy(temp, data, layer->input_dim * sizeof(float));
    // TODO: Implement proper feedforward
    
    free(temp);
}

/**
 * Batch feed-forward processing
 * 
 * @param layer Feed-forward layer parameters
 * @param input Input matrix [batch_size x input_dim]
 * @param output Output matrix [batch_size x output_dim]
 * @param batch_size Number of vectors
 */
void cllm_feedforward_batch(FeedForwardLayer* layer, float* input, 
                            float* output, int batch_size) {
    if (!layer || !input || !output || batch_size <= 0) return;
    
    // Suppress unused variable warnings - these will be used when batch processing is implemented
    (void)layer->input_dim;
    (void)layer->output_dim;
    
    for (int b = 0; b < batch_size; b++) {
        // TODO: Implement proper feedforward batch
    }
}

/**
 * Initialize feed-forward layer
 * 
 * @param layer Feed-forward layer to initialize
 * @param input_dim Input dimension
 * @param hidden_dim Hidden layer dimension
 * @param output_dim Output dimension
 */
void cllm_feedforward_init(FeedForwardLayer* layer, uint32_t input_dim,
                           uint32_t hidden_dim, uint32_t output_dim) {
    if (!layer || input_dim == 0 || hidden_dim == 0 || output_dim == 0) return;
    
    layer->input_dim = input_dim;
    layer->hidden_dim = hidden_dim;
    layer->output_dim = output_dim;
    
    // Allocate weight matrices and biases using standard malloc
    size_t w1_size = input_dim * hidden_dim;
    size_t w2_size = hidden_dim * output_dim;
    
    layer->w1_lattice = (float*)malloc(w1_size * sizeof(float));
    layer->w2_lattice = (float*)malloc(w2_size * sizeof(float));
    layer->bias1 = (float*)malloc(hidden_dim * sizeof(float));
    layer->bias2 = (float*)malloc(output_dim * sizeof(float));
    
    if (!layer->w1_lattice || !layer->w2_lattice || 
        !layer->bias1 || !layer->bias2) {
        cllm_feedforward_free(layer);
        return;
    }
    
    // Initialize weights to small random values
    for (size_t i = 0; i < w1_size; i++) {
        layer->w1_lattice[i] = ((float)rand() / RAND_MAX) * 0.02f - 0.01f;
    }
    for (size_t i = 0; i < w2_size; i++) {
        layer->w2_lattice[i] = ((float)rand() / RAND_MAX) * 0.02f - 0.01f;
    }
    for (size_t i = 0; i < hidden_dim; i++) {
        layer->bias1[i] = 0.0f;
    }
    for (size_t i = 0; i < output_dim; i++) {
        layer->bias2[i] = 0.0f;
    }
}

/**
 * Free feed-forward layer
 * 
 * @param layer Feed-forward layer to free
 */
void cllm_feedforward_free(FeedForwardLayer* layer) {
    if (!layer) return;
    
    if (layer->w1_lattice) {
        free(layer->w1_lattice);
        layer->w1_lattice = NULL;
    }
    
    if (layer->w2_lattice) {
        free(layer->w2_lattice);
        layer->w2_lattice = NULL;
    }
    
    if (layer->bias1) {
        free(layer->bias1);
        layer->bias1 = NULL;
    }
    
    if (layer->bias2) {
        free(layer->bias2);
        layer->bias2 = NULL;
    }
}