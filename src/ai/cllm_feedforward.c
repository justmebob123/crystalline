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
 * Matrix-vector multiplication: output = matrix * input + bias
 * Uses SIMD for vectorization
 * 
 * @param matrix Weight matrix [output_dim x input_dim]
 * @param input Input vector [input_dim]
 * @param bias Bias vector [output_dim]
 * @param output Output vector [output_dim]
 * @param input_dim Input dimension
 * @param output_dim Output dimension
 */
static void matmul_add_bias(float* matrix, float* input, float* bias, 
                           float* output, int input_dim, int output_dim) {
    // Use SIMD matrix-vector multiply
    simd_simd_matrix_vector_multiply(output, matrix, input, output_dim, input_dim);
    
    // Add bias if present
    if (bias) {
        vector_add(output, output, bias, output_dim);
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
void cllm_feedforward(FeedForwardLayer* layer, float* input, float* output) {
    if (!layer || !input || !output) return;
    
    uint32_t input_dim = layer->input_dim;
    uint32_t hidden_dim = layer->hidden_dim;
    uint32_t output_dim = layer->output_dim;
    
    // Allocate hidden layer buffer
    float* hidden = (float*)malloc(hidden_dim * sizeof(float));
    if (!hidden) return;
    
    // First linear layer: hidden = W1 * input + b1
    matmul_add_bias(layer->w1_lattice, input, layer->bias1, 
                   hidden, input_dim, hidden_dim);
    
    // Apply GELU activation
    cllm_activation_gelu(hidden, hidden_dim);
    
    // Second linear layer: output = W2 * hidden + b2
    matmul_add_bias(layer->w2_lattice, hidden, layer->bias2,
                   output, hidden_dim, output_dim);
    
    free(hidden);
}

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
    cllm_feedforward(layer, temp, data);
    
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
    
    uint32_t input_dim = layer->input_dim;
    uint32_t output_dim = layer->output_dim;
    
    for (int b = 0; b < batch_size; b++) {
        cllm_feedforward(layer, &input[b * input_dim], &output[b * output_dim]);
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
    
    // Allocate weight matrices and biases
    size_t w1_size = input_dim * hidden_dim;
    size_t w2_size = hidden_dim * output_dim;
    
    layer->w1_lattice = (float*)calloc(w1_size, sizeof(float));
    layer->w2_lattice = (float*)calloc(w2_size, sizeof(float));
    layer->bias1 = (float*)calloc(hidden_dim, sizeof(float));
    layer->bias2 = (float*)calloc(output_dim, sizeof(float));
    
    if (!layer->w1_lattice || !layer->w2_lattice || 
        !layer->bias1 || !layer->bias2) {
        cllm_feedforward_free(layer);
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