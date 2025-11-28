/**
 * CLLM Angular Position-Based Attention
 * 
 * Implements OBJECTIVE 15: Integrate θ(n,k,λ,ω,ψ) Angular Position into Attention
 * 
 * This file replaces standard dot product attention with angular position-based
 * attention using the formula:
 * 
 * attention(Q,K) = cos((θ_Q - θ_K) · φᵢ) · cymatic_resonance(θ_diff)
 * 
 * Where:
 * - θ = k·π(1+√5) + (n-1)·2π/(12·ln3) + log₃(ν(λ)) + ω + ψ
 * - φᵢ = dimensional frequencies [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, ...]
 * - cymatic_resonance uses 432 Hz base frequency
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_angular_position.h"
#include "../include/cllm_mathematical_constants.h"
#include "../include/prime_float_math.h"

/**
 * Compute attention score using angular positions
 * 
 * This is the CORRECT way to compute attention for a crystalline lattice model.
 * Uses θ(n,k,λ,ω,ψ) angular positions and φᵢ dimensional frequencies.
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
                                   uint32_t head_idx) {
    if (!model || query_token >= model->vocab_size || key_token >= model->vocab_size) {
        return 0.0f;
    }
    
    CLLMToken* q_token = &model->tokens[query_token];
    CLLMToken* k_token = &model->tokens[key_token];
    
    // Compute angular positions θ(n,k,λ,ω,ψ)
    AngularPosition q_pos, k_pos;
    
    // Query angular position
    double q_wavelength = get_phonetic_wavelength(q_token->token_str[0]);
    angular_position_calculate(
        q_token->prime_encoding,
        query_token,
        head_idx,
        q_wavelength,
        &q_pos
    );
    
    // Key angular position
    double k_wavelength = get_phonetic_wavelength(k_token->token_str[0]);
    angular_position_calculate(
        k_token->prime_encoding,
        key_token,
        head_idx,
        k_wavelength,
        &k_pos
    );
    
    // Get dimensional frequency φᵢ for this head
    uint64_t phi_i = cllm_get_dimensional_frequency(head_idx % 12);
    
    // Compute attention score: cos((θ_q - θ_k) · φᵢ)
    double theta_diff = q_pos.theta - k_pos.theta;
    double score = prime_cos(theta_diff * (double)phi_i);
    
    // Apply cymatic resonance (432 Hz base frequency)
    // This adds a frequency-domain modulation to the attention
    double cymatic_phase = 2.0 * PRIME_PI * CYMATIC_432_HZ * theta_diff / 1000.0;
    double resonance = prime_cos(cymatic_phase);
    
    // Combine: attention × (0.8 + 0.2 × resonance)
    // This gives 20% modulation from cymatic resonance
    score *= (0.8 + 0.2 * resonance);
    
    return (float)score;
}

/**
 * Compute attention scores for a batch of query-key pairs
 * 
 * Optimized batch version of angular attention.
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
                                         float* scores) {
    if (!model || !query_tokens || !key_tokens || !scores) return;
    
    // Precompute angular positions for all queries
    AngularPosition* q_positions = (AngularPosition*)malloc(num_queries * sizeof(AngularPosition));
    if (!q_positions) return;
    
    for (uint32_t i = 0; i < num_queries; i++) {
        uint32_t token_id = query_tokens[i];
        if (token_id >= model->vocab_size) continue;
        
        CLLMToken* token = &model->tokens[token_id];
        double wavelength = get_phonetic_wavelength(token->token_str[0]);
        angular_position_calculate(
            token->prime_encoding,
            token_id,
            head_idx,
            wavelength,
            &q_positions[i]
        );
    }
    
    // Precompute angular positions for all keys
    AngularPosition* k_positions = (AngularPosition*)malloc(num_keys * sizeof(AngularPosition));
    if (!k_positions) {
        free(q_positions);
        return;
    }
    
    for (uint32_t j = 0; j < num_keys; j++) {
        uint32_t token_id = key_tokens[j];
        if (token_id >= model->vocab_size) continue;
        
        CLLMToken* token = &model->tokens[token_id];
        double wavelength = get_phonetic_wavelength(token->token_str[0]);
        angular_position_calculate(
            token->prime_encoding,
            token_id,
            head_idx,
            wavelength,
            &k_positions[j]
        );
    }
    
    // Get dimensional frequency φᵢ for this head
    uint64_t phi_i = cllm_get_dimensional_frequency(head_idx % 12);
    
    // Compute all attention scores
    for (uint32_t i = 0; i < num_queries; i++) {
        for (uint32_t j = 0; j < num_keys; j++) {
            // Compute attention score: cos((θ_q - θ_k) · φᵢ)
            double theta_diff = q_positions[i].theta - k_positions[j].theta;
            double score = prime_cos(theta_diff * (double)phi_i);
            
            // Apply cymatic resonance
            double cymatic_phase = 2.0 * PRIME_PI * CYMATIC_432_HZ * theta_diff / 1000.0;
            double resonance = prime_cos(cymatic_phase);
            
            // Combine
            score *= (0.8 + 0.2 * resonance);
            
            scores[i * num_keys + j] = (float)score;
        }
    }
    
    free(q_positions);
    free(k_positions);
}

/**
 * Apply softmax to attention scores
 * 
 * @param scores Attention scores [seq_len]
 * @param seq_len Sequence length
 */
void cllm_attention_softmax(float* scores, uint32_t seq_len) {
    if (!scores || seq_len == 0) return;
    
    // Find max for numerical stability
    float max_score = scores[0];
    for (uint32_t i = 1; i < seq_len; i++) {
        if (scores[i] > max_score) {
            max_score = scores[i];
        }
    }
    
    // Compute exp and sum
    double sum = 0.0;
    for (uint32_t i = 0; i < seq_len; i++) {
        scores[i] = (float)prime_exp((double)(scores[i] - max_score));
        sum += scores[i];
    }
    
    // Normalize
    for (uint32_t i = 0; i < seq_len; i++) {
        scores[i] /= (float)sum;
    }
}

/**
 * Compute full attention with angular positions
 * 
 * This replaces the standard attention mechanism with angular position-based attention.
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
                                    float* output) {
    if (!model || !query_tokens || !key_tokens || !value_embeddings || !output) return;
    
    // Allocate attention scores [seq_len x seq_len]
    float* attention_scores = (float*)calloc(seq_len * seq_len, sizeof(float));
    if (!attention_scores) return;
    
    // Compute attention scores using angular positions
    cllm_attention_scores_angular_batch(
        model,
        query_tokens,
        key_tokens,
        seq_len,
        seq_len,
        head_idx,
        attention_scores
    );
    
    // Apply softmax to each row
    for (uint32_t i = 0; i < seq_len; i++) {
        cllm_attention_softmax(&attention_scores[i * seq_len], seq_len);
    }
    
    // Compute weighted sum of values
    memset(output, 0, seq_len * embed_dim * sizeof(float));
    
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            float attn_weight = attention_scores[i * seq_len + j];
            
            for (uint32_t d = 0; d < embed_dim; d++) {
                output[i * embed_dim + d] += 
                    attn_weight * value_embeddings[j * embed_dim + d];
            }
        }
    }
    
    free(attention_scores);
}

/**
 * Compare angular attention to standard dot product attention
 * 
 * Useful for debugging and validation.
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
                                    uint32_t embed_dim) {
    // Angular attention score
    float angular_score = cllm_attention_score_angular(
        model, query_token, key_token, head_idx
    );
    
    // Standard dot product attention score
    float dot_product = 0.0f;
    for (uint32_t i = 0; i < embed_dim; i++) {
        dot_product += query_embedding[i] * key_embedding[i];
    }
    float standard_score = dot_product / prime_sqrtf((float)embed_dim);
    
    printf("Attention comparison (Q=%u, K=%u, head=%u):\n", 
           query_token, key_token, head_idx);
    printf("  Angular:  %.6f\n", angular_score);
    printf("  Standard: %.6f\n", standard_score);
    printf("  Ratio:    %.6f\n", angular_score / (standard_score + 1e-8f));
}

/**
 * Verify attention respects symmetry groups
 * 
 * Tokens in the same symmetry group should have higher attention scores.
 * 
 * @param model CLLM model
 * @param head_idx Attention head index
 */
void cllm_verify_attention_symmetry(CLLMModel* model, uint32_t head_idx) {
    if (!model) return;
    
    printf("\n=== Verifying Attention Symmetry (head %u) ===\n", head_idx);
    
    // Sample tokens from different symmetry groups
    double same_group_scores = 0.0;
    double diff_group_scores = 0.0;
    int same_count = 0;
    int diff_count = 0;
    
    for (uint32_t i = 0; i < model->vocab_size && i < 100; i++) {
        uint32_t group_i = model->tokens[i].symmetry_group;
        
        for (uint32_t j = i + 1; j < model->vocab_size && j < 100; j++) {
            uint32_t group_j = model->tokens[j].symmetry_group;
            
            float score = cllm_attention_score_angular(model, i, j, head_idx);
            
            if (group_i == group_j) {
                same_group_scores += score;
                same_count++;
            } else {
                diff_group_scores += score;
                diff_count++;
            }
        }
    }
    
    if (same_count > 0 && diff_count > 0) {
        double avg_same = same_group_scores / same_count;
        double avg_diff = diff_group_scores / diff_count;
        
        printf("Average attention scores:\n");
        printf("  Same symmetry group: %.6f (%d pairs)\n", avg_same, same_count);
        printf("  Diff symmetry group: %.6f (%d pairs)\n", avg_diff, diff_count);
        printf("  Ratio (same/diff):   %.6f\n", avg_same / (avg_diff + 1e-8));
        
        if (avg_same > avg_diff) {
            printf("✓ Symmetry respected: same-group attention is higher\n");
        } else {
            printf("⚠ Warning: same-group attention is not higher\n");
        }
    }
    
    printf("\n");
}