#ifndef CLLM_ACTIVATIONS_H
#define CLLM_ACTIVATIONS_H

#include <stdint.h>
#include <stdlib.h>

/**
 * Forward Pass Activation Storage
 * 
 * Stores all intermediate activations needed for backward pass.
 * This enables proper gradient computation through all layers.
 */
typedef struct {
    // Dimensions
    int batch_size;
    int seq_len;
    uint32_t embed_dim;
    uint32_t vocab_size;
    uint32_t num_layers;
    
    // Input embeddings [batch_size * seq_len * embed_dim]
    float* input_embeddings;
    
    // Per-layer activations [num_layers][batch_size * seq_len * embed_dim]
    float** layer_inputs;        // Input to each transformer layer
    float** attention_outputs;   // Output of attention (before residual)
    float** ff_outputs;          // Output of feedforward (before residual)
    float** layer_outputs;       // Final output of layer (after residual + norm)
    
    // Final outputs
    float* final_hidden;         // Final hidden state [batch_size * seq_len * embed_dim]
    float* logits;               // Output logits [batch_size * seq_len * vocab_size]
    
    // Intermediate values for backward pass
    float** attention_scores;    // Attention scores for each layer
    float** ff_hidden;           // FeedForward hidden activations
    
} ForwardActivations;

/**
 * Allocate activation storage
 */
ForwardActivations* cllm_allocate_activations(
    int batch_size,
    int seq_len,
    uint32_t embed_dim,
    uint32_t vocab_size,
    uint32_t num_layers
);

/**
 * Free activation storage
 */
void cllm_free_activations(ForwardActivations* activations);

/**
 * Zero all activations (for reuse)
 */
void cllm_zero_activations(ForwardActivations* activations);

#endif // CLLM_ACTIVATIONS_H