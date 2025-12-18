/**
 * @file worker_functions_geometric.c
 * @brief Worker functions using geometric matrix storage
 * 
 * This file contains updated implementations of worker functions that use
 * geometric matrices instead of flat arrays or placeholders.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hierarchical_threading.h"
#include "thread_parameters_geometric.h"
#include "geometric_matrix.h"
#include "math.h"

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Get double value from CrystallineAbacus
 */
static double get_abacus_value(const CrystallineAbacus* abacus, uint32_t index) {
    (void)index; // For now, we only use index 0
    double value = 0.0;
    abacus_to_double(abacus, &value);
    return value;
}

/**
 * Set double value in CrystallineAbacus
 */
static void set_abacus_value(CrystallineAbacus* abacus, uint32_t index, double value) {
    (void)index; // For now, we only use index 0
    CrystallineAbacus* temp = abacus_from_double(value, 60, 10);
    if (temp) {
        // Copy beads
        abacus->num_beads = temp->num_beads;
        abacus->negative = temp->negative;
        for (size_t i = 0; i < temp->num_beads && i < abacus->capacity; i++) {
            abacus->beads[i] = temp->beads[i];
        }
        abacus_free(temp);
    }
}

// ============================================================================
// EMBEDDING OPERATIONS
// ============================================================================

/**
 * Get embedding for a token using geometric matrix storage
 * 
 * @param thread Thread that owns the embedding
 * @param token_id Token ID
 * @param output Output abacus (pre-allocated)
 * @return 0 on success, -1 on error
 */
int worker_get_embedding_geometric(
    HierarchicalThread* thread,
    uint32_t token_id,
    CrystallineAbacus* output
) {
    if (!thread || !output) {
        fprintf(stderr, "ERROR: Invalid thread or output buffer\n");
        return -1;
    }
    
    // Get embedding matrix (vocab_size × embed_dim)
    GeometricMatrix* embedding = thread_get_parameter_matrix(thread, "embedding", 0);
    if (!embedding) {
        fprintf(stderr, "ERROR: Embedding matrix not found in thread [%d][%d]\n",
                thread->layer, thread->dimension);
        return -1;
    }
    
    // Get embedding dimension
    uint32_t embed_dim = embedding->cols;
    
    // Ensure output has correct capacity
    if (output->capacity < embed_dim) {
        fprintf(stderr, "ERROR: Output abacus capacity too small\n");
        return -1;
    }
    
    // Extract embedding for this token using geometric interpolation
    CrystallineAbacus* temp = abacus_new(60);
    for (uint32_t i = 0; i < embed_dim; i++) {
        if (geometric_matrix_get(embedding, token_id, i, temp) == 0) {
            double value = get_abacus_value(temp, 0);
            set_abacus_value(output, i, value);
        }
    }
    abacus_free(temp);
    
    output->num_beads = embed_dim;
    
    return 0;
}

// ============================================================================
// ATTENTION OPERATIONS
// ============================================================================

/**
 * Compute attention using geometric matrix storage
 * 
 * Computes:
 *   Q = input × W_q
 *   K = input × W_k
 *   V = input × W_v
 *   scores = Q · K^T / sqrt(embed_dim)
 *   attention = softmax(scores)
 *   output = attention × V
 * 
 * @param thread Thread that owns the computation
 * @param input Input abacus
 * @param embedding_dim Embedding dimension
 * @param output Output abacus (pre-allocated)
 * @return 0 on success, -1 on error
 */
int worker_compute_attention_geometric(
    HierarchicalThread* thread,
    const CrystallineAbacus* input,
    uint32_t embedding_dim,
    CrystallineAbacus* output
) {
    if (!thread || !input || !output) {
        fprintf(stderr, "ERROR: Invalid parameters for attention\n");
        return -1;
    }
    
    // Get Q, K, V weight matrices
    GeometricMatrix* W_q = thread_get_parameter_matrix(thread, "W_q", 0);
    GeometricMatrix* W_k = thread_get_parameter_matrix(thread, "W_k", 0);
    GeometricMatrix* W_v = thread_get_parameter_matrix(thread, "W_v", 0);
    
    if (!W_q || !W_k || !W_v) {
        fprintf(stderr, "ERROR: Attention weight matrices not found\n");
        return -1;
    }
    
    // Allocate temporary abacuses for Q, K, V
    CrystallineAbacus* Q = abacus_new(embedding_dim);
    CrystallineAbacus* K = abacus_new(embedding_dim);
    CrystallineAbacus* V = abacus_new(embedding_dim);
    
    if (!Q || !K || !V) {
        fprintf(stderr, "ERROR: Failed to allocate Q, K, V abacuses\n");
        if (Q) abacus_free(Q);
        if (K) abacus_free(K);
        if (V) abacus_free(V);
        return -1;
    }
    
    // Temporary abacus for getting matrix values
    CrystallineAbacus* temp_weight = abacus_new(60);
    
    // Compute Q = input × W_q
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double sum = 0.0;
        for (uint32_t j = 0; j < embedding_dim; j++) {
            double input_val = get_abacus_value(input, j);
            if (geometric_matrix_get(W_q, j, i, temp_weight) == 0) {
                double weight_val = get_abacus_value(temp_weight, 0);
                sum += input_val * weight_val;
            }
        }
        set_abacus_value(Q, i, sum);
    }
    
    // Compute K = input × W_k
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double sum = 0.0;
        for (uint32_t j = 0; j < embedding_dim; j++) {
            double input_val = get_abacus_value(input, j);
            if (geometric_matrix_get(W_k, j, i, temp_weight) == 0) {
                double weight_val = get_abacus_value(temp_weight, 0);
                sum += input_val * weight_val;
            }
        }
        set_abacus_value(K, i, sum);
    }
    
    // Compute V = input × W_v
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double sum = 0.0;
        for (uint32_t j = 0; j < embedding_dim; j++) {
            double input_val = get_abacus_value(input, j);
            if (geometric_matrix_get(W_v, j, i, temp_weight) == 0) {
                double weight_val = get_abacus_value(temp_weight, 0);
                sum += input_val * weight_val;
            }
        }
        set_abacus_value(V, i, sum);
    }
    
    abacus_free(temp_weight);
    
    // Compute attention score: Q · K^T
    double score = 0.0;
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double q_val = get_abacus_value(Q, i);
        double k_val = get_abacus_value(K, i);
        score += q_val * k_val;
    }
    
    // Scale by sqrt(embedding_dim)
    double scale = 1.0 / sqrt((double)embedding_dim);
    score *= scale;
    
    // Apply softmax (simplified for single token)
    // In full implementation, this would be over all tokens
    double attention_weight = exp(score);
    
    // Compute output = attention_weight × V
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double v_val = get_abacus_value(V, i);
        set_abacus_value(output, i, attention_weight * v_val);
    }
    
    output->num_beads = embedding_dim;
    
    // Cleanup
    abacus_free(Q);
    abacus_free(K);
    abacus_free(V);
    
    return 0;
}

// ============================================================================
// FFN OPERATIONS
// ============================================================================

/**
 * Compute FFN using geometric matrix storage
 * 
 * Computes:
 *   hidden = ReLU(input × W_ffn1)
 *   output = hidden × W_ffn2
 * 
 * @param thread Thread that owns the computation
 * @param input Input abacus
 * @param embedding_dim Embedding dimension
 * @param hidden_dim Hidden dimension
 * @param output Output abacus (pre-allocated)
 * @return 0 on success, -1 on error
 */
int worker_compute_ffn_geometric(
    HierarchicalThread* thread,
    const CrystallineAbacus* input,
    uint32_t embedding_dim,
    uint32_t hidden_dim,
    CrystallineAbacus* output
) {
    if (!thread || !input || !output) {
        fprintf(stderr, "ERROR: Invalid parameters for FFN\n");
        return -1;
    }
    
    // Get FFN weight matrices
    GeometricMatrix* W_ffn1 = thread_get_parameter_matrix(thread, "W_ffn1", 0);
    GeometricMatrix* W_ffn2 = thread_get_parameter_matrix(thread, "W_ffn2", 0);
    
    if (!W_ffn1 || !W_ffn2) {
        fprintf(stderr, "ERROR: FFN weight matrices not found\n");
        return -1;
    }
    
    // Allocate temporary hidden layer and weight buffer
    CrystallineAbacus* hidden = abacus_new(hidden_dim);
    CrystallineAbacus* temp_weight = abacus_new(60);
    
    if (!hidden || !temp_weight) {
        fprintf(stderr, "ERROR: Failed to allocate temporary buffers\n");
        if (hidden) abacus_free(hidden);
        if (temp_weight) abacus_free(temp_weight);
        return -1;
    }
    
    // Compute hidden = input × W_ffn1
    for (uint32_t i = 0; i < hidden_dim; i++) {
        double sum = 0.0;
        for (uint32_t j = 0; j < embedding_dim; j++) {
            double input_val = get_abacus_value(input, j);
            if (geometric_matrix_get(W_ffn1, j, i, temp_weight) == 0) {
                double weight_val = get_abacus_value(temp_weight, 0);
                sum += input_val * weight_val;
            }
        }
        // Apply ReLU activation
        sum = (sum > 0.0) ? sum : 0.0;
        set_abacus_value(hidden, i, sum);
    }
    
    // Compute output = hidden × W_ffn2
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double sum = 0.0;
        for (uint32_t j = 0; j < hidden_dim; j++) {
            double hidden_val = get_abacus_value(hidden, j);
            if (geometric_matrix_get(W_ffn2, j, i, temp_weight) == 0) {
                double weight_val = get_abacus_value(temp_weight, 0);
                sum += hidden_val * weight_val;
            }
        }
        set_abacus_value(output, i, sum);
    }
    
    abacus_free(temp_weight);
    
    output->num_beads = embedding_dim;
    
    // Cleanup
    abacus_free(hidden);
    
    return 0;
}

// ============================================================================
// GRADIENT OPERATIONS
// ============================================================================

/**
 * Compute and accumulate gradients using geometric matrix storage
 * 
 * This is a simplified gradient computation that accumulates gradients
 * to the geometric matrix vertices.
 * 
 * @param thread Thread that owns the parameters
 * @param grad_output Gradient from next layer
 * @param input Input that was used in forward pass
 * @param embedding_dim Embedding dimension
 * @return 0 on success, -1 on error
 */
int worker_compute_gradients_geometric(
    HierarchicalThread* thread,
    const CrystallineAbacus* grad_output,
    const CrystallineAbacus* input,
    uint32_t embedding_dim
) {
    if (!thread || !grad_output || !input) {
        fprintf(stderr, "ERROR: Invalid parameters for gradient computation\n");
        return -1;
    }
    
    // Get parameter and gradient matrices
    GeometricMatrix* W_q = thread_get_parameter_matrix(thread, "W_q", 0);
    GeometricMatrix* grad_W_q = thread_get_gradient_matrix(thread, "W_q", 0);
    
    if (!W_q || !grad_W_q) {
        fprintf(stderr, "ERROR: Weight or gradient matrix not found\n");
        return -1;
    }
    
    // Compute gradient: ∂L/∂W_q = grad_output × input^T
    // Accumulate gradient at each position (will be distributed to vertices)
    for (uint32_t i = 0; i < embedding_dim; i++) {
        for (uint32_t j = 0; j < embedding_dim; j++) {
            double grad_val = get_abacus_value(grad_output, i);
            double input_val = get_abacus_value(input, j);
            double gradient = grad_val * input_val;
            
            // Accumulate gradient (this will distribute to nearest vertices)
            geometric_matrix_accumulate_gradient_value(grad_W_q, i, j, gradient);
        }
    }
    
    // TODO: Similar for W_k, W_v, W_ffn1, W_ffn2
    
    return 0;
}