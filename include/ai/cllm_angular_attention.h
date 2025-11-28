#ifndef CLLM_ANGULAR_ATTENTION_H
#define CLLM_ANGULAR_ATTENTION_H

#include "../cllm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute attention score using angular positions
 * 
 * Uses θ(n,k,λ,ω,ψ) angular positions and φᵢ dimensional frequencies.
 * Formula: attention(Q,K) = cos((θ_Q - θ_K) · φᵢ) · cymatic_resonance(θ_diff)
 * 
 * @param model CLLM model
 * @param query_token Query token ID
 * @param key_token Key token ID
 * @param head_idx Attention head index
 * @return Attention score in [-1, 1]
 */
float cllm_attention_score_angular(CLLMModel* model,
                                   uint32_t query_token,
                                   uint32_t key_token,
                                   uint32_t head_idx);

/**
 * Compute attention scores for a batch of query-key pairs
 * 
 * @param model CLLM model
 * @param query_tokens Array of query token IDs [num_queries]
 * @param key_tokens Array of key token IDs [num_keys]
 * @param num_queries Number of query tokens
 * @param num_keys Number of key tokens
 * @param head_idx Attention head index
 * @param scores Output attention scores [num_queries x num_keys]
 */
void cllm_attention_scores_angular_batch(CLLMModel* model,
                                         uint32_t* query_tokens,
                                         uint32_t* key_tokens,
                                         uint32_t num_queries,
                                         uint32_t num_keys,
                                         uint32_t head_idx,
                                         float* scores);

/**
 * Apply softmax to attention scores
 * 
 * @param scores Attention scores [seq_len]
 * @param seq_len Sequence length
 */
void cllm_attention_softmax(float* scores, uint32_t seq_len);

/**
 * Compute full attention with angular positions
 * 
 * @param model CLLM model
 * @param query_tokens Query token IDs [seq_len]
 * @param key_tokens Key token IDs [seq_len]
 * @param value_embeddings Value embeddings [seq_len x embed_dim]
 * @param seq_len Sequence length
 * @param embed_dim Embedding dimension
 * @param head_idx Attention head index
 * @param output Output embeddings [seq_len x embed_dim]
 */
void cllm_attention_forward_angular(CLLMModel* model,
                                    uint32_t* query_tokens,
                                    uint32_t* key_tokens,
                                    float* value_embeddings,
                                    uint32_t seq_len,
                                    uint32_t embed_dim,
                                    uint32_t head_idx,
                                    float* output);

/**
 * Compare angular attention to standard dot product attention
 * 
 * @param model CLLM model
 * @param query_token Query token ID
 * @param key_token Key token ID
 * @param head_idx Attention head index
 * @param query_embedding Query embedding vector [embed_dim]
 * @param key_embedding Key embedding vector [embed_dim]
 * @param embed_dim Embedding dimension
 */
void cllm_compare_attention_methods(CLLMModel* model,
                                    uint32_t query_token,
                                    uint32_t key_token,
                                    uint32_t head_idx,
                                    float* query_embedding,
                                    float* key_embedding,
                                    uint32_t embed_dim);

/**
 * Verify attention respects symmetry groups
 * 
 * @param model CLLM model
 * @param head_idx Attention head index
 */
void cllm_verify_attention_symmetry(CLLMModel* model, uint32_t head_idx);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_ANGULAR_ATTENTION_H */