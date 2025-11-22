/*
 * CLLM Embedding Layer
 * Handles token embeddings and lattice transformations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/prime_float_math.h"

/**
 * Embed a single token into the embedding space
 * 
 * @param inf Inference engine state
 * @param token_id Token ID to embed
 * @param output Output embedding vector [embedding_dim]
 */
void cllm_embed_token(CLLMInference* inf, uint32_t token_id, float* output) {
    if (!inf || !output || token_id >= inf->model->vocab_size) {
        return;
    }
    
    CLLMModel* model = inf->model;
    uint32_t embedding_dim = model->embeddings.embedding_dim;
    
    // Copy embedding from embedding matrix
    // Embeddings are stored as [vocab_size x embedding_dim]
    float* embedding_matrix = model->embeddings.embeddings;
    size_t offset = token_id * embedding_dim;
    
    for (uint32_t i = 0; i < embedding_dim; i++) {
        output[i] = embedding_matrix[offset + i];
    }
}

/**
 * Apply lattice transformation to an embedding
 * 
 * @param embedding Input/output embedding vector [dim]
 * @param transform Transformation matrix [dim x dim]
 * @param dim Embedding dimension
 */
void cllm_apply_lattice_transform(float* embedding, float* transform, int dim) {
    if (!embedding || !transform || dim <= 0) {
        return;
    }
    
    // Allocate temporary buffer for result
    float* result = (float*)calloc(dim, sizeof(float));
    if (!result) return;
    
    // Matrix-vector multiplication: result = transform * embedding
    for (int i = 0; i < dim; i++) {
        float sum = 0.0f;
        for (int j = 0; j < dim; j++) {
            sum += transform[i * dim + j] * embedding[j];
        }
        result[i] = sum;
    }
    
    // Copy result back to embedding
    memcpy(embedding, result, dim * sizeof(float));
    free(result);
}

/**
 * Get embedding with lattice transformation applied
 * 
 * @param inf Inference engine state
 * @param token_id Token ID to embed
 * @param output Output transformed embedding [embedding_dim]
 */
void cllm_get_embedding_transformed(CLLMInference* inf, uint32_t token_id, float* output) {
    if (!inf || !output) return;
    
    // Get base embedding
    cllm_embed_token(inf, token_id, output);
    
    // Apply lattice transformation if available
    if (inf->model->embeddings.lattice_transform) {
        cllm_apply_lattice_transform(output, 
                                     inf->model->embeddings.lattice_transform,
                                     inf->model->embeddings.embedding_dim);
    }
}

/**
 * Batch embed multiple tokens
 * 
 * @param inf Inference engine state
 * @param token_ids Array of token IDs [num_tokens]
 * @param num_tokens Number of tokens
 * @param output Output embeddings [num_tokens x embedding_dim]
 */
void cllm_embed_tokens_batch(CLLMInference* inf, uint32_t* token_ids, 
                             int num_tokens, float* output) {
    if (!inf || !token_ids || !output || num_tokens <= 0) {
        return;
    }
    
    uint32_t embedding_dim = inf->model->embeddings.embedding_dim;
    
    for (int i = 0; i < num_tokens; i++) {
        cllm_get_embedding_transformed(inf, token_ids[i], 
                                      &output[i * embedding_dim]);
    }
}

/**
 * Apply inverse lattice transformation
 * 
 * @param embedding Input/output embedding vector [dim]
 * @param inverse_transform Inverse transformation matrix [dim x dim]
 * @param dim Embedding dimension
 */
void cllm_apply_inverse_lattice_transform(float* embedding, float* inverse_transform, int dim) {
    if (!embedding || !inverse_transform || dim <= 0) {
        return;
    }
    
    cllm_apply_lattice_transform(embedding, inverse_transform, dim);
}

/**
 * Project embedding back to vocabulary space (for output layer)
 * 
 * @param inf Inference engine state
 * @param hidden_state Hidden state vector [embedding_dim]
 * @param logits Output logits [vocab_size]
 */
void cllm_project_to_vocab(CLLMInference* inf, float* hidden_state, float* logits) {
    if (!inf || !hidden_state || !logits) return;
    
    CLLMModel* model = inf->model;
    uint32_t vocab_size = model->vocab_size;
    uint32_t embedding_dim = model->embeddings.embedding_dim;
    float* embedding_matrix = model->embeddings.embeddings;
    
    // Apply inverse transform if available
    float* transformed = (float*)malloc(embedding_dim * sizeof(float));
    if (!transformed) return;
    
    memcpy(transformed, hidden_state, embedding_dim * sizeof(float));
    
    if (model->embeddings.inverse_transform) {
        cllm_apply_inverse_lattice_transform(transformed,
                                            model->embeddings.inverse_transform,
                                            embedding_dim);
    }
    
    // Compute logits as dot product with each embedding
    for (uint32_t i = 0; i < vocab_size; i++) {
        float dot = 0.0f;
        size_t offset = i * embedding_dim;
        
        for (uint32_t j = 0; j < embedding_dim; j++) {
            dot += transformed[j] * embedding_matrix[offset + j];
        }
        
        logits[i] = dot;
    }
    
    free(transformed);
}

/**
 * Compute embedding norm (L2 norm)
 * 
 * @param embedding Embedding vector [dim]
 * @param dim Embedding dimension
 * @return L2 norm of the embedding
 */
float cllm_embedding_norm(float* embedding, int dim) {
    if (!embedding || dim <= 0) return 0.0f;
    
    float sum = 0.0f;
    for (int i = 0; i < dim; i++) {
        sum += embedding[i] * embedding[i];
    }
    
    return prime_sqrt(sum);
}

/**
 * Normalize embedding to unit length
 * 
 * @param embedding Input/output embedding vector [dim]
 * @param dim Embedding dimension
 */
void cllm_normalize_embedding(float* embedding, int dim) {
    if (!embedding || dim <= 0) return;
    
    float norm = cllm_embedding_norm(embedding, dim);
    if (norm > 1e-8f) {
        for (int i = 0; i < dim; i++) {
            embedding[i] /= norm;
        }
    }
}