/**
 * @file cllm_kissing_spheres_attention.c
 * @brief Kissing Spheres Attention Implementation
 * 
 * Implements attention mechanism where each token attends to its 12 kissing
 * sphere neighbors using the L(n,d,k,λ) lattice formula for attention weights.
 */

#include "cllm_lattice_attention.h"
#include "cllm_inference.h"
#include "prime_lattice_core.h"
#include "cllm_angular_position.h"
#include "prime_float_math.h"
#include "prime_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Softmax function for normalizing attention weights
 */
static void softmax(float* weights, int size) {
    if (!weights || size <= 0) return;
    
    // Find max for numerical stability
    float max_val = weights[0];
    for (int i = 1; i < size; i++) {
        if (weights[i] > max_val) {
            max_val = weights[i];
        }
    }
    
    // Compute exp(x - max) and sum
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        weights[i] = prime_expf(weights[i] - max_val);
        sum += weights[i];
    }
    
    // Normalize
    if (sum > 0.0f) {
        for (int i = 0; i < size; i++) {
            weights[i] /= sum;
        }
    }
}

/**
 * Compute attention weights for kissing sphere neighbors
 */
int cllm_compute_kissing_spheres_weights(
    CLLMModel* model,
    uint32_t token_id,
    float* weights
) {
    if (!model || !weights) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_compute_kissing_spheres_weights\n");
        return -1;
    }
    
    if (token_id >= model->vocab_size) {
        fprintf(stderr, "ERROR: Token ID %u out of range (vocab_size=%lu)\n", 
                token_id, (unsigned long)model->vocab_size);
        return -1;
    }
    
    if (!model->lattice_points || token_id >= model->num_lattice_points) {
        fprintf(stderr, "ERROR: No lattice point for token %u\n", token_id);
        return -1;
    }
    
    CLLMLatticePoint* point = &model->lattice_points[token_id];
    CLLMToken* center_token = &model->tokens[token_id];
    
    // Compute raw attention weights using L(n,d,k,λ) formula
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        
        if (neighbor_id >= model->vocab_size) {
            weights[i] = 0.0f;
            continue;
        }
        
        // Get dimensional frequency φᵢ for this neighbor
        uint64_t phi_i = cllm_get_dimensional_frequency(i);
        
        // Compute interaction strength using L(n,d,k,λ)
        double interaction = L_lattice(
            center_token->prime_encoding,     // n: center prime
            i,                                // d: neighbor index (0-11)
            center_token->symmetry_group,     // k: symmetry group
            center_token->token_str,          // λ: phonetic layer
            3,                                // ω: Einstein's Λ
            center_token->prime_encoding,     // p: for Plimpton
            phi_i                             // q: dimensional frequency
        );
        
        // Convert to attention weight (positive values)
        weights[i] = (float)prime_fabsf((float)interaction);
    }
    
    // Fill remaining weights with zeros if fewer than 12 neighbors
    for (uint32_t i = point->num_neighbors; i < 12; i++) {
        weights[i] = 0.0f;
    }
    
    // Normalize using softmax
    softmax(weights, 12);
    
    return 0;
}

/**
 * Compute attention using kissing spheres neighbors
 */
int cllm_attention_kissing_spheres(
    CLLMModel* model,
    uint32_t token_id,
    const float* query,
    float* output,
    float* attention_weights
) {
    if (!model || !query || !output) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_attention_kissing_spheres\n");
        return -1;
    }
    
    if (token_id >= model->vocab_size) {
        fprintf(stderr, "ERROR: Token ID %u out of range\n", token_id);
        return -1;
    }
    
    if (!model->lattice_points || token_id >= model->num_lattice_points) {
        fprintf(stderr, "ERROR: No lattice point for token %u\n", token_id);
        return -1;
    }
    
    CLLMLatticePoint* point = &model->lattice_points[token_id];
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Compute attention weights
    float weights[12];
    if (cllm_compute_kissing_spheres_weights(model, token_id, weights) != 0) {
        return -1;
    }
    
    // Copy weights to output if requested
    if (attention_weights) {
        memcpy(attention_weights, weights, 12 * sizeof(float));
    }
    
    // Initialize output to zero
    memset(output, 0, embed_dim * sizeof(float));
    
    // Accumulate weighted neighbor embeddings
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        
        if (neighbor_id >= model->vocab_size) continue;
        
        float weight = weights[i];
        double* neighbor_embedding_double = &model->embeddings.embeddings[
            neighbor_id * embed_dim
        ];
        
        // Convert to float for processing
        float neighbor_embedding[embed_dim];
        for (uint32_t d = 0; d < embed_dim; d++) {
            neighbor_embedding[d] = (float)neighbor_embedding_double[d];
        }
        
        // Add weighted neighbor contribution
        for (uint32_t d = 0; d < embed_dim; d++) {
            output[d] += weight * neighbor_embedding[d];
        }
    }
    
    return 0;
}

/**
 * Compute attention for a batch of tokens using kissing spheres
 */
int cllm_attention_kissing_spheres_batch(
    CLLMModel* model,
    const uint32_t* token_ids,
    uint32_t batch_size,
    const float* queries,
    float* outputs
) {
    if (!model || !token_ids || !queries || !outputs) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_attention_kissing_spheres_batch\n");
        return -1;
    }
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Process each token in the batch
    for (uint32_t b = 0; b < batch_size; b++) {
        uint32_t token_id = token_ids[b];
        const float* query = &queries[b * embed_dim];
        float* output = &outputs[b * embed_dim];
        
        if (cllm_attention_kissing_spheres(model, token_id, query, output, NULL) != 0) {
            fprintf(stderr, "ERROR: Failed to compute attention for token %u in batch\n", token_id);
            return -1;
        }
    }
    
    return 0;
}

/**
 * Integrate kissing spheres attention with standard attention layer
 */
int cllm_attention_hybrid_kissing_spheres(
    AttentionLayer* layer,
    CLLMModel* model,
    uint32_t token_id,
    const double* input,
    double* output,
    float ks_weight
) {
    if (!layer || !model || !input || !output) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_attention_hybrid_kissing_spheres\n");
        return -1;
    }
    
    if (ks_weight < 0.0f || ks_weight > 1.0f) {
        fprintf(stderr, "ERROR: ks_weight must be in range [0.0, 1.0]\n");
        return -1;
    }
    
    uint32_t embed_dim = model->embeddings.embedding_dim;
    
    // Compute standard attention
    double* std_output = (double*)calloc(embed_dim, sizeof(double));
    if (!std_output) {
        fprintf(stderr, "ERROR: Failed to allocate memory for standard attention output\n");
        return -1;
    }
    
    cllm_attention_forward(layer, (double*)input, std_output, NULL, NULL, 1);
    
    // Compute kissing spheres attention
    float* ks_output = (float*)calloc(embed_dim, sizeof(float));
    if (!ks_output) {
        fprintf(stderr, "ERROR: Failed to allocate memory for kissing spheres attention output\n");
        free(std_output);
        return -1;
    }
    
    // Convert input to float for kissing spheres attention
    float* query_float = (float*)malloc(embed_dim * sizeof(float));
    if (!query_float) {
        fprintf(stderr, "ERROR: Failed to allocate memory for query conversion\n");
        free(std_output);
        free(ks_output);
        return -1;
    }
    
    for (uint32_t i = 0; i < embed_dim; i++) {
        query_float[i] = (float)input[i];
    }
    
    if (cllm_attention_kissing_spheres(model, token_id, query_float, ks_output, NULL) != 0) {
        free(std_output);
        free(ks_output);
        free(query_float);
        return -1;
    }
    
    // Combine outputs: output = (1 - ks_weight) * std_output + ks_weight * ks_output
    float std_weight = 1.0f - ks_weight;
    for (uint32_t i = 0; i < embed_dim; i++) {
        output[i] = std_weight * std_output[i] + ks_weight * (double)ks_output[i];
    }
    
    free(std_output);
    free(ks_output);
    free(query_float);
    
    return 0;
}

/**
 * Get statistics about kissing spheres attention
 */
int cllm_kissing_spheres_attention_stats(
    CLLMModel* model,
    float* avg_weight,
    float* max_weight,
    float* min_weight
) {
    if (!model || !avg_weight || !max_weight || !min_weight) {
        fprintf(stderr, "ERROR: Invalid parameters to cllm_kissing_spheres_attention_stats\n");
        return -1;
    }
    
    if (!model->lattice_points) {
        fprintf(stderr, "ERROR: No lattice points in model\n");
        return -1;
    }
    
    float sum = 0.0f;
    float max_val = -1.0f;
    float min_val = 2.0f;
    uint32_t count = 0;
    
    // Sample weights from first 100 tokens (or all if fewer)
    uint32_t sample_size = (model->num_lattice_points < 100) ? 
                           model->num_lattice_points : 100;
    
    for (uint32_t i = 0; i < sample_size; i++) {
        float weights[12];
        if (cllm_compute_kissing_spheres_weights(model, i, weights) != 0) {
            continue;
        }
        
        for (int j = 0; j < 12; j++) {
            sum += weights[j];
            if (weights[j] > max_val) max_val = weights[j];
            if (weights[j] < min_val) min_val = weights[j];
            count++;
        }
    }
    
    *avg_weight = (count > 0) ? sum / count : 0.0f;
    *max_weight = max_val;
    *min_weight = min_val;
    
    return 0;
}