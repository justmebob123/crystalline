#include "cllm_bigfixed.h"
#include "cllm.h"
#include "bigfixed_core.h"
#include "prime_bigint_transcendental.h"
#include "bigfixed_math_wrappers.h"
#include "numerical_bigfixed.h"
#include "loss_functions_bigfixed.h"
#include "prime_float_math.h"
#include <stdlib.h>
#include <string.h>

/**
 * Attention forward pass using BigFixed arbitrary precision
 * This eliminates NaN errors and overflow issues
 */
void cllm_attention_forward_bigfixed(
    AttentionLayer* layer,
    BigFixed** input,
    BigFixed** output,
    int seq_len,
    int precision
) {
    if (!layer || !input || !output || seq_len <= 0) return;
    
    uint32_t num_heads = layer->num_heads;
    uint32_t head_dim = layer->head_dim;
    uint32_t embedding_dim = num_heads * head_dim;
    
    // Allocate BigFixed buffers
    BigFixed** queries = (BigFixed**)calloc(seq_len * embedding_dim, sizeof(BigFixed*));
    BigFixed** keys = (BigFixed**)calloc(seq_len * embedding_dim, sizeof(BigFixed*));
    BigFixed** values = (BigFixed**)calloc(seq_len * embedding_dim, sizeof(BigFixed*));
    BigFixed** attention_scores = (BigFixed**)calloc(seq_len, sizeof(BigFixed*));
    
    if (!queries || !keys || !values || !attention_scores) {
        free(queries);
        free(keys);
        free(values);
        free(attention_scores);
        return;
    }
    
    // Initialize BigFixed arrays
    for (int i = 0; i < seq_len * embedding_dim; i++) {
        queries[i] = big_fixed_create(precision);
        keys[i] = big_fixed_create(precision);
        values[i] = big_fixed_create(precision);
    }
    
    for (int i = 0; i < seq_len; i++) {
        attention_scores[i] = big_fixed_create(precision);
    }
    
    // Project input to Q, K, V using matrix multiplication
    // Q = input * W_q, K = input * W_k, V = input * W_v
    for (int pos = 0; pos < seq_len; pos++) {
        BigFixed** input_vec = &input[pos * embedding_dim];
        
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                // Compute Q[pos, h, d] = dot(input, W_q)
                BigFixed** weight_q = &layer->query_lattice[h * head_dim * head_dim + d * head_dim];
                BigFixed** input_head = &input_vec[h * head_dim];
                dot_product_bigfixed(
                    weight_q,
                    input_head,
                    queries[pos * embedding_dim + h * head_dim + d],
                    head_dim,
                    precision
                );
                
                // Compute K[pos, h, d] = dot(input, W_k)
                BigFixed** weight_k = &layer->key_lattice[h * head_dim * head_dim + d * head_dim];
                dot_product_bigfixed(
                    weight_k,
                    input_head,
                    keys[pos * embedding_dim + h * head_dim + d],
                    head_dim,
                    precision
                );
                
                // Compute V[pos, h, d] = dot(input, W_v)
                BigFixed** weight_v = &layer->value_lattice[h * head_dim * head_dim + d * head_dim];
                dot_product_bigfixed(
                    weight_v,
                    input_head,
                    values[pos * embedding_dim + h * head_dim + d],
                    head_dim,
                    precision
                );
            }
        }
    }
    
    // Initialize output to zero
    for (int i = 0; i < seq_len * embedding_dim; i++) {
        big_fixed_from_int(output[i], 0);
    }
    
    // Compute attention for each position and head
    for (int pos = 0; pos < seq_len; pos++) {
        for (uint32_t h = 0; h < num_heads; h++) {
            BigFixed** query = &queries[pos * embedding_dim + h * head_dim];
            
            // Compute attention scores: score[i] = dot(query, key[i]) / sqrt(head_dim)
            BigFixed* scale = big_fixed_create(precision);
            BigFixed* sqrt_d = big_fixed_create(precision);
            big_fixed_from_int(sqrt_d, head_dim);
            bigfixed_sqrt(sqrt_d, sqrt_d, precision);
            
            for (int i = 0; i < seq_len; i++) {
                BigFixed** key = &keys[i * embedding_dim + h * head_dim];
                
                // score = dot(query, key)
                dot_product_bigfixed(
                    query,
                    key,
                    attention_scores[i],
                    head_dim,
                    precision
                );
                
                // score = score / sqrt(head_dim)
                big_fixed_div(attention_scores[i], attention_scores[i], sqrt_d);
            }
            
            // Apply softmax to attention scores
            softmax_bigfixed(attention_scores, seq_len, precision);
            
            // Apply attention to values: output = sum(attention_scores[i] * values[i])
            BigFixed** head_output = &output[pos * embedding_dim + h * head_dim];
            
            for (int i = 0; i < seq_len; i++) {
                BigFixed** value = &values[i * embedding_dim + h * head_dim];
                
                for (uint32_t d = 0; d < head_dim; d++) {
                    BigFixed* weighted = big_fixed_create(precision);
                    big_fixed_mul(weighted, attention_scores[i], value[d]);
                    big_fixed_add(head_output[d], head_output[d], weighted);
                    big_fixed_free(weighted);
                }
            }
            
            big_fixed_free(scale);
            big_fixed_free(sqrt_d);
        }
    }
    
    // Cleanup
    for (int i = 0; i < seq_len * embedding_dim; i++) {
        big_fixed_free(queries[i]);
        big_fixed_free(keys[i]);
        big_fixed_free(values[i]);
    }
    
    for (int i = 0; i < seq_len; i++) {
        big_fixed_free(attention_scores[i]);
    }
    
    free(queries);
    free(keys);
    free(values);
    free(attention_scores);
}
