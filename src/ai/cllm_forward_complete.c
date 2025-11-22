/*
 * CLLM Complete Forward Pass
 * Integrates all components for end-to-end inference
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/prime_float_math.h"

/**
 * Complete forward pass through transformer layer
 * Integrates: LayerNorm -> Attention -> Residual -> LayerNorm -> FFN -> Residual
 * 
 * @param inf Inference engine state
 * @param layer_idx Layer index
 * @param input Input sequence [seq_len x embedding_dim]
 * @param output Output sequence [seq_len x embedding_dim]
 * @param seq_len Sequence length
 */
void cllm_transformer_layer_forward(CLLMInference* inf, int layer_idx,
                                   float* input, float* output, int seq_len) {
    if (!inf || !input || !output || layer_idx < 0 || seq_len <= 0) return;
    
    CLLMModel* model = inf->model;
    if (layer_idx >= (int)model->num_layers) return;
    
    uint32_t embedding_dim = model->embedding_dim;
    size_t seq_size = seq_len * embedding_dim;
    
    // Allocate temporary buffers
    float* attn_input = (float*)malloc(seq_size * sizeof(float));
    float* attn_output = (float*)malloc(seq_size * sizeof(float));
    float* ffn_input = (float*)malloc(seq_size * sizeof(float));
    float* ffn_output = (float*)malloc(seq_size * sizeof(float));
    
    if (!attn_input || !attn_output || !ffn_input || !ffn_output) {
        if (attn_input) free(attn_input);
        if (attn_output) free(attn_output);
        if (ffn_input) free(ffn_input);
        if (ffn_output) free(ffn_output);
        return;
    }
    
    // Get layer components
    CLLMLayerNorm* ln1 = &model->layer_norms[layer_idx * 2];  // Pre-attention norm
    CLLMLayerNorm* ln2 = &model->layer_norms[layer_idx * 2 + 1];  // Pre-FFN norm
    
    // 1. Layer Normalization (pre-attention)
    for (int i = 0; i < seq_len; i++) {
        cllm_layer_norm(ln1, &input[i * embedding_dim], &attn_input[i * embedding_dim]);
    }
    
    // 2. Multi-Head Attention
    cllm_multi_head_attention(inf, layer_idx, attn_input, attn_output, seq_len);
    
    // 3. Residual connection (attention)
    for (size_t i = 0; i < seq_size; i++) {
        ffn_input[i] = input[i] + attn_output[i];
    }
    
    // 4. Layer Normalization (pre-FFN)
    for (int i = 0; i < seq_len; i++) {
        float* ln_input = &ffn_input[i * embedding_dim];
        float* ln_output = &attn_input[i * embedding_dim];  // Reuse buffer
        cllm_layer_norm(ln2, ln_input, ln_output);
    }
    
    // 5. Feed-Forward Network
    FeedForwardLayer* ff = &model->ff_layers[layer_idx];
    for (int i = 0; i < seq_len; i++) {
        cllm_feedforward(ff, &attn_input[i * embedding_dim], &ffn_output[i * embedding_dim]);
    }
    
    // 6. Residual connection (FFN)
    for (size_t i = 0; i < seq_size; i++) {
        output[i] = ffn_input[i] + ffn_output[i];
    }
    
    free(attn_input);
    free(attn_output);
    free(ffn_input);
    free(ffn_output);
}

/**
 * Complete forward pass through entire model
 * 
 * @param inf Inference engine state
 * @param tokens Input token IDs [seq_len]
 * @param seq_len Sequence length
 * @param output_logits Output logits [vocab_size]
 */
void cllm_forward_complete(CLLMInference* inf, uint32_t* tokens, int seq_len, float* output_logits) {
    if (!inf || !tokens || !output_logits || seq_len <= 0) return;
    
    CLLMModel* model = inf->model;
    uint32_t embedding_dim = model->embedding_dim;
    size_t seq_size = seq_len * embedding_dim;
    
    // Allocate buffers for sequence processing
    float* embeddings = (float*)malloc(seq_size * sizeof(float));
    float* layer_input = (float*)malloc(seq_size * sizeof(float));
    float* layer_output = (float*)malloc(seq_size * sizeof(float));
    
    if (!embeddings || !layer_input || !layer_output) {
        if (embeddings) free(embeddings);
        if (layer_input) free(layer_input);
        if (layer_output) free(layer_output);
        return;
    }
    
    // 1. Embedding layer
    printf("Forward pass: Embedding tokens...\n");
    for (int i = 0; i < seq_len; i++) {
        cllm_get_embedding_transformed(inf, tokens[i], &embeddings[i * embedding_dim]);
    }
    
    // 2. Add positional encoding
    printf("Forward pass: Adding positional encoding...\n");
    for (int i = 0; i < seq_len; i++) {
        cllm_apply_positional_encoding_complete(inf, &embeddings[i * embedding_dim], i);
    }
    
    // Copy to layer input
    memcpy(layer_input, embeddings, seq_size * sizeof(float));
    
    // 3. Process through transformer layers
    printf("Forward pass: Processing through %u transformer layers...\n", model->num_layers);
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        cllm_transformer_layer_forward(inf, layer, layer_input, layer_output, seq_len);
        
        // Swap buffers for next layer
        float* temp = layer_input;
        layer_input = layer_output;
        layer_output = temp;
        
        if ((layer + 1) % 4 == 0) {
            printf("  Processed layer %u/%u\n", layer + 1, model->num_layers);
        }
    }
    
    // 4. Final layer normalization
    printf("Forward pass: Final layer normalization...\n");
    CLLMLayerNorm* final_ln = &model->layer_norms[model->num_layers * 2 - 1];
    float* final_hidden = (float*)malloc(embedding_dim * sizeof(float));
    if (final_hidden) {
        // Use last token's hidden state for prediction
        cllm_layer_norm(final_ln, &layer_input[(seq_len - 1) * embedding_dim], final_hidden);
        
        // 5. Project to vocabulary
        printf("Forward pass: Projecting to vocabulary...\n");
        cllm_project_to_vocab(inf, final_hidden, output_logits);
        
        free(final_hidden);
    }
    
    free(embeddings);
    free(layer_input);
    free(layer_output);
    
    printf("Forward pass complete!\n");
}

/**
 * Generate next token using complete forward pass
 * 
 * @param inf Inference engine state
 * @param tokens Input token sequence [seq_len]
 * @param seq_len Sequence length
 * @return Next token ID
 */
uint32_t cllm_generate_next_token(CLLMInference* inf, uint32_t* tokens, int seq_len) {
    if (!inf || !tokens || seq_len <= 0) return 0;
    
    uint32_t vocab_size = inf->model->vocab_size;
    
    // Allocate logits buffer
    float* logits = (float*)malloc(vocab_size * sizeof(float));
    if (!logits) return 0;
    
    // Forward pass
    cllm_forward_complete(inf, tokens, seq_len, logits);
    
    // Apply temperature
    if (inf->temperature != 1.0f) {
        for (uint32_t i = 0; i < vocab_size; i++) {
            logits[i] /= inf->temperature;
        }
    }
    
    // Sample token
    uint32_t next_token = cllm_sample_token(inf, logits);
    
    free(logits);
    return next_token;
}

/**
 * Generate sequence autoregressively
 * 
 * @param inf Inference engine state
 * @param prompt Prompt tokens [prompt_len]
 * @param prompt_len Prompt length
 * @param output Output tokens [max_length]
 * @param max_length Maximum generation length
 * @return Number of tokens generated
 */
int cllm_generate_sequence(CLLMInference* inf, uint32_t* prompt, int prompt_len,
                          uint32_t* output, int max_length) {
    if (!inf || !prompt || !output || prompt_len <= 0 || max_length <= 0) return 0;
    
    // Copy prompt to output
    memcpy(output, prompt, prompt_len * sizeof(uint32_t));
    
    int current_len = prompt_len;
    
    printf("\nGenerating sequence (max %d tokens)...\n", max_length);
    
    // Generate tokens one by one
    for (int i = 0; i < max_length - prompt_len; i++) {
        uint32_t next_token = cllm_generate_next_token(inf, output, current_len);
        
        output[current_len] = next_token;
        current_len++;
        
        // Check for end-of-sequence token (assuming 0 or specific EOS token)
        if (next_token == 0) {
            printf("Generated %d tokens (EOS reached)\n", current_len - prompt_len);
            break;
        }
        
        if ((i + 1) % 10 == 0) {
            printf("  Generated %d/%d tokens\n", i + 1, max_length - prompt_len);
        }
    }
    
    return current_len;
}

/**
 * Initialize model for inference
 * Sets up all necessary components
 * 
 * @param model CLLM model
 * @return Initialized inference engine
 */
CLLMInference* cllm_initialize_inference(CLLMModel* model) {
    if (!model) return NULL;
    
    printf("\n=== Initializing CLLM Inference ===\n");
    
    // Create inference engine
    CLLMInference* inf = cllm_inference_init(model);
    if (!inf) {
        printf("ERROR: Failed to create inference engine\n");
        return NULL;
    }
    
    printf("✓ Inference engine created\n");
    printf("  Vocabulary size: %lu\n", (unsigned long)model->vocab_size);
    printf("  Embedding dim: %lu\n", (unsigned long)model->embedding_dim);
    printf("  Num layers: %u\n", model->num_layers);
    printf("  Context length: %u\n", model->header.context_length);
    
    // Generate lattice embeddings if not already present
    if (model->embeddings.embeddings) {
        printf("✓ Embeddings present\n");
    } else {
        printf("⚠ Generating lattice embeddings...\n");
        cllm_generate_lattice_embeddings(model);
    }
    
    // Generate positional encodings if not already present
    if (model->pos_encoding.spiral_positions) {
        printf("✓ Positional encodings present\n");
    } else {
        printf("⚠ Generating positional encodings...\n");
        cllm_generate_all_positional_encodings(model);
    }
    
    printf("=== Inference initialization complete! ===\n\n");
    
    return inf;
}