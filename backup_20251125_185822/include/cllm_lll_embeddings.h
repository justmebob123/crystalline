#ifndef CLLM_LLL_EMBEDDINGS_H
#define CLLM_LLL_EMBEDDINGS_H

#include "cllm_training.h"

/**
 * LLL Embedding Reducer
 * Opaque structure for LLL-based dimensionality reduction
 */
typedef struct LLLEmbeddingReducer LLLEmbeddingReducer;

/**
 * Create LLL embedding reducer
 * 
 * Computes covariance matrix and applies LLL reduction to find
 * optimal reduced basis for embeddings.
 * 
 * @param model Model with embeddings to reduce
 * @param target_dim Target reduced dimension (e.g., 64 or 32)
 * @return Reducer instance or NULL on failure
 */
LLLEmbeddingReducer* lll_reducer_create(CLLMModel* model, int target_dim);

/**
 * Free LLL reducer
 */
void lll_reducer_free(LLLEmbeddingReducer* reducer);

/**
 * Project embedding to reduced space
 * 
 * @param reducer LLL reducer
 * @param embedding Original embedding (original_dim)
 * @param reduced Output reduced embedding (reduced_dim)
 */
void lll_project_embedding(LLLEmbeddingReducer* reducer, const float* embedding, float* reduced);

/**
 * Reconstruct embedding from reduced space
 * 
 * @param reducer LLL reducer
 * @param reduced Reduced embedding (reduced_dim)
 * @param embedding Output reconstructed embedding (original_dim)
 */
void lll_reconstruct_embedding(LLLEmbeddingReducer* reducer, const float* reduced, float* embedding);

/**
 * Project all model embeddings to reduced space
 * 
 * @param reducer LLL reducer
 * @param model Model with embeddings
 * @return Array of reduced embeddings (vocab_size × reduced_dim)
 */
float* lll_project_all_embeddings(LLLEmbeddingReducer* reducer, CLLMModel* model);

/**
 * Integrate LLL reduction into training
 * 
 * Reduces embedding dimensionality and updates training state.
 * This provides 2-4x speedup by reducing parameter count.
 * 
 * @param training Training state
 * @param target_dim Target reduced dimension (e.g., 64 or 32)
 */
void lll_integrate_training(CLLMTraining* training, int target_dim);

#endif // CLLM_LLL_EMBEDDINGS_H