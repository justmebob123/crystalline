/*
 * CLLM Attention Mechanism
 * Implements multi-head self-attention with lattice structure
 * 
 * HYBRID ATTENTION SYSTEM:
 * - When token IDs available: Use angular attention (OBJECTIVE 15)
 * - When token IDs unavailable: Use standard dot product attention
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/prime_float_math.h"
#include "../include/cllm_simd_utils.h"
#include "../include/cllm_cache.h"
#include "../include/ai/cllm_angular_attention.h"
#include "../include/ai/cllm_ntt_attention.h"

// Forward declarations for missing math functions
double prime_exp(double x);
double prime_sqrt(double x);

/**
 * Softmax function
 * 
 * @param x Input/output array
 * @param size Array size
 */
static void softmax(float* x, int size) {
    if (!x || size <= 0) return;
    
    // Find max for numerical stability
    float max_val = x[0];
    for (int i = 1; i < size; i++) {
        if (x[i] > max_val) {
            max_val = x[i];
        }
    }
    
    // Compute exp and sum
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        x[i] = prime_exp(x[i] - max_val);
        sum += x[i];
    }
    
    // Normalize
    if (sum > 1e-8f) {
        for (int i = 0; i < size; i++) {
            x[i] /= sum;
        }
    }
}

/**
 * Scaled dot-product attention for a single head
 * 
 * Attention(Q, K, V) = softmax(Q * K^T / sqrt(d_k)) * V
 * 
 * @param query Query vector [head_dim]
 * @param keys Key matrix [seq_len x head_dim]
 * @param values Value matrix [seq_len x head_dim]
 * @param output Output vector [head_dim]
 * @param head_dim Dimension per head
 * @param seq_len Sequence length
 */
static void scaled_dot_product_attention(float* query, float* keys, float* values,
                                        float* output, int head_dim, int seq_len) {
    if (!query || !keys || !values || !output || head_dim <= 0 || seq_len <= 0) {
        return;
    }
    
    float scale = 1.0f / prime_sqrt((float)head_dim);
    
    // Allocate attention scores
    float* scores = (float*)malloc(seq_len * sizeof(float));
    if (!scores) return;
    
    // Compute attention scores using SIMD dot product: scores[i] = query · keys[i] / sqrt(head_dim)
    for (int i = 0; i < seq_len; i++) {
        // Prefetch next key for better cache utilization
        if (i + 1 < seq_len) {
            prefetch_read(&keys[(i + 1) * head_dim]);
        }
        scores[i] = dot_product(query, &keys[i * head_dim], head_dim) * scale;
    }
    
    // Apply softmax to get attention weights
    softmax(scores, seq_len);
    
    // Compute weighted sum of values using SIMD
    memset(output, 0, head_dim * sizeof(float));
    for (int i = 0; i < seq_len; i++) {
        // Prefetch next value for better cache utilization
        if (i + 1 < seq_len) {
            prefetch_read(&values[(i + 1) * head_dim]);
        }
        
        // output += scores[i] * values[i]
        float score = scores[i];
        int j = 0;
        int j_vec = (head_dim / 8) * 8;
        __m256 vscore = _mm256_set1_ps(score);
        
        // Vectorized accumulation
        for (; j < j_vec; j += 8) {
            __m256 vval = _mm256_loadu_ps(&values[i * head_dim + j]);
            __m256 vout = _mm256_loadu_ps(&output[j]);
            vout = _mm256_fmadd_ps(vscore, vval, vout);
            _mm256_storeu_ps(&output[j], vout);
        }
        
        // Scalar remainder
        for (; j < head_dim; j++) {
            output[j] += score * values[i * head_dim + j];
        }
    }
    
    free(scores);
}

/**
 * Multi-head attention forward pass
 * 
 * @param layer Attention layer parameters
 * @param input Input sequence [seq_len x embedding_dim]
 * @param output Output sequence [seq_len x embedding_dim]
 * @param key_cache Cached keys [seq_len x embedding_dim] (can be NULL)
 * @param value_cache Cached values [seq_len x embedding_dim] (can be NULL)
 * @param seq_len Sequence length
 */
void cllm_attention_forward(AttentionLayer* layer, float* input, float* output,
                           float* key_cache, float* value_cache, int seq_len) {
    if (!layer || !input || !output || seq_len <= 0) return;
    
    uint32_t num_heads = layer->num_heads;
    uint32_t head_dim = layer->head_dim;
    uint32_t embedding_dim = num_heads * head_dim;
    
    // Allocate buffers for Q, K, V projections
    float* queries = (float*)malloc(seq_len * embedding_dim * sizeof(float));
    float* keys = (float*)malloc(seq_len * embedding_dim * sizeof(float));
    float* values = (float*)malloc(seq_len * embedding_dim * sizeof(float));
    
    if (!queries || !keys || !values) {
        if (queries) free(queries);
        if (keys) free(keys);
        if (values) free(values);
        return;
    }
    
    // Project input to Q, K, V
    // For simplicity, using the lattice weights directly
    // In a full implementation, these would be separate projection matrices
    (void)num_heads;  // Used for structure, suppress warning
    (void)head_dim;   // Used for structure, suppress warning
    
    for (int pos = 0; pos < seq_len; pos++) {
        float* input_vec = &input[pos * embedding_dim];
        
        // Query projection
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                float sum = 0.0f;
                for (uint32_t i = 0; i < head_dim; i++) {
                    size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                    sum += layer->query_lattice[weight_idx] * input_vec[h * head_dim + i];
                }
                queries[pos * embedding_dim + h * head_dim + d] = sum;
            }
        }
        
        // Key projection
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                float sum = 0.0f;
                for (uint32_t i = 0; i < head_dim; i++) {
                    size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                    sum += layer->key_lattice[weight_idx] * input_vec[h * head_dim + i];
                }
                keys[pos * embedding_dim + h * head_dim + d] = sum;
            }
        }
        
        // Value projection
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                float sum = 0.0f;
                for (uint32_t i = 0; i < head_dim; i++) {
                    size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                    sum += layer->value_lattice[weight_idx] * input_vec[h * head_dim + i];
                }
                values[pos * embedding_dim + h * head_dim + d] = sum;
            }
        }
    }
    
    // Use cached keys/values if available
    if (key_cache) {
        memcpy(keys, key_cache, seq_len * embedding_dim * sizeof(float));
    }
    if (value_cache) {
        memcpy(values, value_cache, seq_len * embedding_dim * sizeof(float));
    }
    
    // Apply attention for each position and head
    memset(output, 0, seq_len * embedding_dim * sizeof(float));
    
    for (int pos = 0; pos < seq_len; pos++) {
        for (uint32_t h = 0; h < num_heads; h++) {
            float* query = &queries[pos * embedding_dim + h * head_dim];
            float* head_keys = &keys[h * head_dim];
            float* head_values = &values[h * head_dim];
            float* head_output = &output[pos * embedding_dim + h * head_dim];
            
            // Compute attention for this head
            scaled_dot_product_attention(query, head_keys, head_values,
                                        head_output, head_dim, seq_len);
        }
    }
    
    // Update caches if provided
    if (key_cache) {
        memcpy(key_cache, keys, seq_len * embedding_dim * sizeof(float));
    }
    if (value_cache) {
        memcpy(value_cache, values, seq_len * embedding_dim * sizeof(float));
    }
    
    free(queries);
    free(keys);
    free(values);
}

/**
 * Multi-head attention with KV cache (for autoregressive generation)
 * 
 * @param inf Inference engine state
 * @param layer_idx Layer index
 * @param input Input sequence [seq_len x embedding_dim]
 * @param output Output sequence [seq_len x embedding_dim]
 * @param seq_len Sequence length
 */
void cllm_multi_head_attention(CLLMInference* inf, int layer_idx,
                              float* input, float* output, int seq_len) {
    if (!inf || !input || !output || layer_idx < 0 || seq_len <= 0) return;
    
    if (layer_idx >= (int)inf->model->num_layers) return;
    
    AttentionLayer* layer = &inf->model->attention_layers[layer_idx];
    
    // Calculate cache offsets for this layer
    uint32_t embedding_dim = inf->model->embedding_dim;
    size_t cache_offset = layer_idx * inf->kv_cache_size * embedding_dim;
    
    float* key_cache = inf->key_cache ? &inf->key_cache[cache_offset] : NULL;
    float* value_cache = inf->value_cache ? &inf->value_cache[cache_offset] : NULL;
    
    cllm_attention_forward(layer, input, output, key_cache, value_cache, seq_len);
}

/**
 * Initialize attention layer
 * 
 * @param layer Attention layer to initialize
 * @param num_heads Number of attention heads
 * @param head_dim Dimension per head
 */
void cllm_attention_init(AttentionLayer* layer, uint32_t num_heads, uint32_t head_dim) {
    if (!layer || num_heads == 0 || head_dim == 0) return;
    
    layer->num_heads = num_heads;
    layer->head_dim = head_dim;
    
    size_t weight_size = num_heads * head_dim * head_dim;
    
    layer->query_lattice = (float*)calloc(weight_size, sizeof(float));
    layer->key_lattice = (float*)calloc(weight_size, sizeof(float));
    layer->value_lattice = (float*)calloc(weight_size, sizeof(float));
    
    if (!layer->query_lattice || !layer->key_lattice || !layer->value_lattice) {
        cllm_attention_free(layer);
    }
}

/**
 * Free attention layer
 * 
 * @param layer Attention layer to free
 */
void cllm_attention_free(AttentionLayer* layer) {
    if (!layer) return;
    
    if (layer->query_lattice) {
        free(layer->query_lattice);
        layer->query_lattice = NULL;
    }
    
    if (layer->key_lattice) {
        free(layer->key_lattice);
        layer->key_lattice = NULL;
    }
    
    if (layer->value_lattice) {
        free(layer->value_lattice);
        layer->value_lattice = NULL;
    }
}

/**
 * Hybrid Attention Forward Pass with Angular and NTT Attention
 * 
 * This function implements:
 * - OBJECTIVE 15: Angular Attention (θ(n,k,λ,ω,ψ))
 * - OBJECTIVE 17: NTT Attention (O(n log n) for long sequences)
 * 
 * Strategy:
 * - For long sequences (>= 256): Use NTT attention (O(n log n))
 * - For short sequences with token IDs: Use angular attention
 * - Otherwise: Use standard dot product attention
 * 
 * @param model CLLM model (needed for angular attention)
 * @param layer Attention layer
 * @param input Input embeddings [seq_len x embedding_dim]
 * @param output Output embeddings [seq_len x embedding_dim]
 * @param token_ids Token IDs [seq_len] (NULL for standard attention)
 * @param key_cache Optional key cache
 * @param value_cache Optional value cache
 * @param seq_len Sequence length
 */
void cllm_attention_forward_hybrid(
    CLLMModel* model,
    AttentionLayer* layer,
    float* input,
    float* output,
    uint32_t* token_ids,
    float* key_cache,
    float* value_cache,
    int seq_len
) {
    if (!layer || !input || !output || seq_len <= 0) return;
    
    uint32_t num_heads = layer->num_heads;
    uint32_t head_dim = layer->head_dim;
    uint32_t embedding_dim = num_heads * head_dim;
    
    // OBJECTIVE 17: Use NTT attention for long sequences (O(n log n))
    if (seq_len >= 256) {
        // Allocate buffers for Q, K, V projections
        float* queries = (float*)malloc(seq_len * embedding_dim * sizeof(float));
        float* keys = (float*)malloc(seq_len * embedding_dim * sizeof(float));
        float* values = (float*)malloc(seq_len * embedding_dim * sizeof(float));
        
        if (!queries || !keys || !values) {
            free(queries);
            free(keys);
            free(values);
            // Fall back to standard attention
            cllm_attention_forward(layer, input, output, key_cache, value_cache, seq_len);
            return;
        }
        
        // Project input to Q, K, V (simplified - using input directly)
        // In full implementation, these would be separate projections
        memcpy(queries, input, seq_len * embedding_dim * sizeof(float));
        memcpy(keys, input, seq_len * embedding_dim * sizeof(float));
        memcpy(values, input, seq_len * embedding_dim * sizeof(float));
        
        // Use NTT attention for O(n log n) complexity
        int result = cllm_attention_ntt_forward(
            queries,
            keys,
            values,
            seq_len,
            head_dim,
            output
        );
        
        if (result != 0) {
            // NTT failed, fall back to standard
            cllm_attention_forward(layer, input, output, key_cache, value_cache, seq_len);
        }
        
        free(queries);
        free(keys);
        free(values);
        return;
    }
    
    // OBJECTIVE 15: Use angular attention for short sequences with token IDs
    if (token_ids && model) {
        uint32_t num_heads = layer->num_heads;
        uint32_t head_dim = layer->head_dim;
        uint32_t embedding_dim = num_heads * head_dim;
        
        // Allocate buffers for values (still need embeddings for weighted sum)
        float* values = (float*)malloc(seq_len * embedding_dim * sizeof(float));
        if (!values) {
            // Fall back to standard attention if allocation fails
            cllm_attention_forward(layer, input, output, key_cache, value_cache, seq_len);
            return;
        }
        
        // Project input to values
        for (int pos = 0; pos < seq_len; pos++) {
            float* input_vec = &input[pos * embedding_dim];
            
            for (uint32_t h = 0; h < num_heads; h++) {
                for (uint32_t d = 0; d < head_dim; d++) {
                    float sum = 0.0f;
                    for (uint32_t i = 0; i < head_dim; i++) {
                        size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                        sum += layer->value_lattice[weight_idx] * input_vec[h * head_dim + i];
                    }
                    values[pos * embedding_dim + h * head_dim + d] = sum;
                }
            }
        }
        
        // Use cached values if available
        if (value_cache) {
            memcpy(values, value_cache, seq_len * embedding_dim * sizeof(float));
        }
        
        // Apply angular attention for each head
        memset(output, 0, seq_len * embedding_dim * sizeof(float));
        
        for (uint32_t h = 0; h < num_heads; h++) {
            float* head_values = &values[h * head_dim];
            float* head_output = &output[h * head_dim];
            
            // Use angular attention from CLLM wrapper
            cllm_attention_forward_angular(
                model,
                token_ids,      // Query tokens
                token_ids,      // Key tokens (self-attention)
                head_values,    // Value embeddings
                seq_len,
                head_dim,
                h,              // Head index
                head_output
            );
        }
        
        // Update value cache if provided
        if (value_cache) {
            memcpy(value_cache, values, seq_len * embedding_dim * sizeof(float));
        }
        
        free(values);
    } else {
        // Fall back to standard dot product attention
        cllm_attention_forward(layer, input, output, key_cache, value_cache, seq_len);
    }
}