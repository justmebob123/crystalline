#include "cllm_training.h"
#include "cllm.h"
#include "cllm_training.h"
#include "bigfixed_core.h"
#include "loss_functions_bigfixed.h"
#include "optimizers_bigfixed.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Compute loss using BigFixed arbitrary precision
 */
void cllm_compute_loss_bigfixed(
    BigFixed** logits,
    uint32_t* targets,
    BigFixed* loss,
    int batch_size,
    int num_classes,
    int precision
) {
    cross_entropy_loss_bigfixed(
        logits,
        targets,
        loss,
        batch_size,
        num_classes,
        precision
    );
}

/**
 * Optimizer step using BigFixed arbitrary precision
 */
void cllm_optimizer_step_bigfixed(
    CLLMModel* model,
    CLLMTraining* training,
    float learning_rate,
    int precision
) {
    if (!model || !training) return;
    
    // Use Adam optimizer
    adam_step_bigfixed(
        model->weights,
        training->gradients,
        training->optimizer_state,
        &training->optimizer_state[model->num_weights],
        model->num_weights,
        learning_rate,
        0.9f,  // beta1
        0.999f, // beta2
        1e-8f,  // epsilon
        precision
    );
}

/**
 * Complete forward pass using BigFixed
 */
void cllm_forward_bigfixed(
    CLLMModel* model,
    BigFixed** input,
    BigFixed** output,
    int seq_len,
    int precision
) {
    if (!model || !input || !output || seq_len <= 0) return;
    
    uint32_t embedding_dim = model->embedding_dim;
    uint32_t num_layers = model->num_layers;
    
    // Allocate layer buffers
    BigFixed** layer_input = input;
    BigFixed** layer_output = (BigFixed**)calloc(seq_len * embedding_dim, sizeof(BigFixed*));
    BigFixed** attn_output = (BigFixed**)calloc(seq_len * embedding_dim, sizeof(BigFixed*));
    BigFixed** ff_output = (BigFixed**)calloc(seq_len * embedding_dim, sizeof(BigFixed*));
    
    if (!layer_output || !attn_output || !ff_output) {
        free(layer_output);
        free(attn_output);
        free(ff_output);
        return;
    }
    
    for (int i = 0; i < seq_len * embedding_dim; i++) {
        layer_output[i] = big_fixed_create(precision);
        attn_output[i] = big_fixed_create(precision);
        ff_output[i] = big_fixed_create(precision);
    }
    
    // Process through layers
    for (uint32_t l = 0; l < num_layers; l++) {
        // Attention
        cllm_attention_forward_bigfixed(
            &model->attention_layers[l],
            layer_input,
            attn_output,
            seq_len,
            precision
        );
        
        // Add residual connection
        for (int i = 0; i < seq_len * embedding_dim; i++) {
            big_fixed_add(attn_output[i], attn_output[i], layer_input[i]);
        }
        
        // Layer norm after attention
        cllm_layernorm_forward_bigfixed(
            &model->layer_norms[l * 2],
            attn_output,
            layer_output,
            seq_len,
            precision
        );
        
        // Feedforward
        cllm_feedforward_forward_bigfixed(
            &model->ff_layers[l],
            layer_output,
            ff_output,
            seq_len,
            precision
        );
        
        // Add residual connection
        for (int i = 0; i < seq_len * embedding_dim; i++) {
            big_fixed_add(ff_output[i], ff_output[i], layer_output[i]);
        }
        
        // Layer norm after feedforward
        cllm_layernorm_forward_bigfixed(
            &model->layer_norms[l * 2 + 1],
            ff_output,
            layer_output,
            seq_len,
            precision
        );
        
        layer_input = layer_output;
    }
    
    // Copy final output
    for (int i = 0; i < seq_len * embedding_dim; i++) {
        big_fixed_assign(output[i], layer_output[i]);
    }
    
    // Cleanup
    for (int i = 0; i < seq_len * embedding_dim; i++) {
        big_fixed_free(layer_output[i]);
        big_fixed_free(attn_output[i]);
        big_fixed_free(ff_output[i]);
    }
    
    free(layer_output);
    free(attn_output);
    free(ff_output);
}

/**
 * Training step using BigFixed
 */
float cllm_train_step_bigfixed(
    CLLMModel* model,
    CLLMTraining* training,
    uint32_t* input_tokens,
    uint32_t* target_tokens,
    int seq_len,
    float learning_rate
) {
    if (!model || !training || !input_tokens || !target_tokens) return -1.0f;
    
    int precision = model->precision_bits;
    uint32_t embedding_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Allocate BigFixed buffers
    BigFixed** input = (BigFixed**)calloc(seq_len * embedding_dim, sizeof(BigFixed*));
    BigFixed** output = (BigFixed**)calloc(seq_len * embedding_dim, sizeof(BigFixed*));
    BigFixed** logits = (BigFixed**)calloc(seq_len * vocab_size, sizeof(BigFixed*));
    
    if (!input || !output || !logits) {
        free(input);
        free(output);
        free(logits);
        return -1.0f;
    }
    
    for (int i = 0; i < seq_len * embedding_dim; i++) {
        input[i] = big_fixed_create(precision);
        output[i] = big_fixed_create(precision);
    }
    
    for (int i = 0; i < seq_len * vocab_size; i++) {
        logits[i] = big_fixed_create(precision);
    }
    
    // Get embeddings from model
    for (int i = 0; i < seq_len; i++) {
        uint32_t token_id = input_tokens[i];
        if (token_id >= vocab_size) continue;
        
        // Copy embedding
        for (uint32_t d = 0; d < embedding_dim; d++) {
            big_fixed_assign(
                input[i * embedding_dim + d],
                model->weights[token_id * embedding_dim + d]
            );
        }
    }
    
    // Forward pass
    cllm_forward_bigfixed(model, input, output, seq_len, precision);
    
    // Compute logits (simplified - just use output as logits)
    for (int i = 0; i < seq_len; i++) {
        for (uint32_t v = 0; v < vocab_size; v++) {
            // Dot product with output embedding
            BigFixed* sum = big_fixed_create(precision);
            big_fixed_from_int(sum, 0);
            
            for (uint32_t d = 0; d < embedding_dim; d++) {
                BigFixed* prod = big_fixed_create(precision);
                big_fixed_mul(
                    prod,
                    output[i * embedding_dim + d],
                    model->weights[v * embedding_dim + d]
                );
                big_fixed_add(sum, sum, prod);
                big_fixed_free(prod);
            }
            
            big_fixed_assign(logits[i * vocab_size + v], sum);
            big_fixed_free(sum);
        }
    }
    
    // Compute loss
    BigFixed* loss = big_fixed_create(precision);
    cllm_compute_loss_bigfixed(
        logits,
        target_tokens,
        loss,
        seq_len,
        vocab_size,
        precision
    );
    
    float loss_value = big_fixed_to_double(loss);
    
    // TODO: Backward pass (compute gradients)
    // For now, just zero gradients
    for (uint64_t i = 0; i < model->num_weights; i++) {
        big_fixed_from_int(training->gradients[i], 0);
    }
    
    // Optimizer step
    cllm_optimizer_step_bigfixed(model, training, learning_rate, precision);
    
    // Cleanup
    for (int i = 0; i < seq_len * embedding_dim; i++) {
        big_fixed_free(input[i]);
        big_fixed_free(output[i]);
    }
    
    for (int i = 0; i < seq_len * vocab_size; i++) {
        big_fixed_free(logits[i]);
    }
    
    big_fixed_free(loss);
    free(input);
    free(output);
    free(logits);
    
    return loss_value;
}
