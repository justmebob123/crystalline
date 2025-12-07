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
static double gelu(double x) {
    const double sqrt_2_over_pi = 0.7978845608; // sqrt(2/π)
    const double coeff = 0.044715;
    
    double x_cubed = x * x * x;
    double inner = sqrt_2_over_pi * (x + coeff * x_cubed);
    
    // tanh approximation
    double tanh_val;
    if (inner > 5.0) {
        tanh_val = 1.0;
    } else if (inner < -5.0) {
        tanh_val = -1.0;
    } else {
        double exp_2x = prime_exp(2.0 * inner);
        tanh_val = (exp_2x - 1.0) / (exp_2x + 1.0);
    }
    
    return 0.5 * x * (1.0 + tanh_val);
}

/**
 * Apply GELU activation to array
 * 
 * @param x Input/output array
 * @param size Array size
 */
void cllm_activation_gelu(double* x, int size) {
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
void cllm_activation_relu(double* x, int size) {
    if (!x || size <= 0) return;
    
    for (int i = 0; i < size; i++) {
        if (x[i] < 0.0) {
            x[i] = 0.0;
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
void cllm_feedforward_inplace(FeedForwardLayer* layer, double* data) {
    if (!layer || !data) return;
    
    if (layer->input_dim != layer->output_dim) {
        // Cannot do in-place if dimensions don't match
        return;
    }
    
    double* temp = (double*)malloc(layer->input_dim * sizeof(double));
    if (!temp) return;
    
    memcpy(temp, data, layer->input_dim * sizeof(double));
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
void cllm_feedforward_batch(FeedForwardLayer* layer, double* input, 
                            double* output, int batch_size) {
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
    
    layer->w1_lattice = (double*)malloc(w1_size * sizeof(double));
    layer->w2_lattice = (double*)malloc(w2_size * sizeof(double));
    layer->bias1 = (double*)malloc(hidden_dim * sizeof(double));
    layer->bias2 = (double*)malloc(output_dim * sizeof(double));
    
    if (!layer->w1_lattice || !layer->w2_lattice || 
        !layer->bias1 || !layer->bias2) {
        cllm_feedforward_free(layer);
        return;
    }
    
    // Initialize weights to small random values
    for (size_t i = 0; i < w1_size; i++) {
        layer->w1_lattice[i] = ((double)rand() / RAND_MAX) * 0.02 - 0.01;
    }
    for (size_t i = 0; i < w2_size; i++) {
        layer->w2_lattice[i] = ((double)rand() / RAND_MAX) * 0.02 - 0.01;
    }
    for (size_t i = 0; i < hidden_dim; i++) {
        layer->bias1[i] = 0.0;
    }
    for (size_t i = 0; i < output_dim; i++) {
        layer->bias2[i] = 0.0;
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