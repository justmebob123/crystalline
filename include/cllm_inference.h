#ifndef CLLM_INFERENCE_H
#define CLLM_INFERENCE_H

/*
 * cllm_inference.h - Auto-generated header file
 * Source: cllm_inference.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "prime_types.h"

/* Local includes */
#include "cllm.h"

/* Type definitions */
typedef struct { int idx; float prob; } IndexProb;

/*
 * CLLM Inference Engine - Runtime inference state
 */
typedef struct {
    CLLMModel* model;            // Pointer to the model
    float temperature;           // Sampling temperature
    float top_p;                 // Nucleus sampling parameter
    int top_k;                   // Top-k sampling parameter
    int max_tokens;              // Maximum tokens to generate
    float repetition_penalty;    // Repetition penalty factor
    
    // KV cache for attention
    int kv_cache_size;           // Size of KV cache
    int kv_cache_used;           // Number of cached positions
    float* key_cache;            // Cached keys
    float* value_cache;          // Cached values
    
    // Working buffers
    float* hidden_states;        // Hidden state buffer
    float* logits;               // Output logits buffer
} CLLMInference;

/* Function declarations */
CLLMInference* cllm_inference_init(CLLMModel* model);
int cllm_tokenize(CLLMInference* inference, const char* text, uint32_t* tokens, int max_tokens);
void cllm_detokenize(CLLMInference* inference, uint32_t* tokens, int num_tokens, char* output, int max_length);
void cllm_get_embedding(CLLMInference* inference, uint32_t token_id, float* embedding);
void cllm_apply_positional_encoding(CLLMInference* inference, float* embedding, int position);

/* Embedding layer functions */
void cllm_embed_token(CLLMInference* inf, uint32_t token_id, float* output);
void cllm_apply_lattice_transform(float* embedding, float* transform, int dim);
void cllm_get_embedding_transformed(CLLMInference* inf, uint32_t token_id, float* output);
void cllm_embed_tokens_batch(CLLMInference* inf, uint32_t* token_ids, int num_tokens, float* output);
void cllm_project_to_vocab(CLLMInference* inf, float* hidden_state, float* logits);
float cllm_embedding_norm(float* embedding, int dim);
void cllm_normalize_embedding(float* embedding, int dim);

/* Attention functions */
void cllm_attention_forward(AttentionLayer* layer, float* input, float* output,
                           float* key_cache, float* value_cache, int seq_len);
void cllm_multi_head_attention(CLLMInference* inf, int layer_idx, float* input, float* output, int seq_len);
void cllm_attention_init(AttentionLayer* layer, uint32_t num_heads, uint32_t head_dim);
void cllm_attention_free(AttentionLayer* layer);

/* Feed-forward functions */
void cllm_feedforward(FeedForwardLayer* layer, float* input, float* output);
void cllm_activation_gelu(float* x, int size);
void cllm_activation_relu(float* x, int size);
void cllm_feedforward_inplace(FeedForwardLayer* layer, float* data);
void cllm_feedforward_batch(FeedForwardLayer* layer, float* input, float* output, int batch_size);
void cllm_feedforward_init(FeedForwardLayer* layer, uint32_t input_dim, uint32_t hidden_dim, uint32_t output_dim);
void cllm_feedforward_free(FeedForwardLayer* layer);

/* Layer normalization functions */
void cllm_layer_norm(CLLMLayerNorm* ln, float* input, float* output);
void cllm_layer_norm_inplace(CLLMLayerNorm* ln, float* data);
void cllm_layer_norm_batch(CLLMLayerNorm* ln, float* input, float* output, int batch_size);
void cllm_layer_norm_init(CLLMLayerNorm* ln, uint32_t dim, float epsilon);
void cllm_layer_norm_free(CLLMLayerNorm* ln);
void cllm_layer_norm_stats(float* input, uint32_t dim, float* mean, float* variance);

/* Lattice embedding functions */
void cllm_compute_spiral_position(uint64_t prime, float* angle, float* radius);
void cllm_map_token_to_lattice(uint32_t token_id, uint64_t prime, float* coords);
void cllm_generate_lattice_embedding(uint32_t token_id, uint64_t prime, uint32_t embedding_dim, float* output);
void cllm_generate_lattice_embeddings(CLLMModel* model);
void cllm_generate_lattice_transform(float* transform, int dim);
float cllm_lattice_token_distance(uint32_t token1_id, uint64_t prime1, uint32_t token2_id, uint64_t prime2);
void cllm_find_lattice_neighbors(uint32_t token_id, uint64_t prime, uint32_t* all_tokens, uint64_t* all_primes, uint32_t num_tokens, int k, uint32_t* neighbors);

/* Positional encoding functions */
void cllm_generate_spiral_encoding(PositionalEncoding* pos_enc);
void cllm_generate_clock_encoding(PositionalEncoding* pos_enc);
void cllm_generate_prime_encoding(PositionalEncoding* pos_enc);
void cllm_initialize_learned_encoding(PositionalEncoding* pos_enc);
void cllm_apply_positional_encoding_complete(CLLMInference* inf, float* embedding, int position);
void cllm_generate_all_positional_encodings(CLLMModel* model);
void cllm_free_positional_encoding(PositionalEncoding* pos_enc);

/* Symmetry functions */
uint32_t cllm_compute_symmetry_group(uint64_t prime);
void cllm_apply_symmetry_transform(float* embedding, int symmetry_group, int dim);
void cllm_apply_inverse_symmetry_transform(float* embedding, int symmetry_group, int dim);
void cllm_compute_symmetry_invariants(float* embedding, int dim, float* features, int num_features);
void cllm_apply_equivariant_transform(float* embedding, int symmetry_group, float* transform_matrix, int dim);
float cllm_symmetry_compatibility(float* embedding1, int symmetry1, float* embedding2, int symmetry2, int dim);
void cllm_generate_symmetry_attention_mask(int* symmetry_groups, int seq_len, float* mask);

/* Forward pass and generation */
void cllm_forward(CLLMInference* inference, uint32_t* tokens, int num_tokens);
void cllm_transformer_layer_forward(CLLMInference* inf, int layer_idx, float* input, float* output, int seq_len);
void cllm_forward_complete(CLLMInference* inf, uint32_t* tokens, int seq_len, float* output_logits);
uint32_t cllm_generate_next_token(CLLMInference* inf, uint32_t* tokens, int seq_len);
int cllm_generate_sequence(CLLMInference* inf, uint32_t* prompt, int prompt_len, uint32_t* output, int max_length);
CLLMInference* cllm_initialize_inference(CLLMModel* model);
void cllm_compute_logits(CLLMInference* inf, float* hidden_state);
int cllm_sample_token(CLLMInference* inf, float* logits);
void cllm_apply_temperature(float* logits, int vocab_size, float temperature);
void cllm_softmax(float* logits, int vocab_size);
uint32_t cllm_sample_top_k(float* probs, int vocab_size, int k);
uint32_t cllm_sample_top_p(float* probs, int vocab_size, float p);
int cllm_generate(CLLMInference* inference, const char* prompt, char* output, int max_output_length);

/* Configuration */
void cllm_set_temperature(CLLMInference* inference, float temperature);
void cllm_set_top_p(CLLMInference* inference, float top_p);
void cllm_set_top_k(CLLMInference* inference, int top_k);
void cllm_set_max_tokens(CLLMInference* inference, int max_tokens);
void cllm_inference_cleanup(CLLMInference* inference);

#endif /* CLLM_INFERENCE_H */