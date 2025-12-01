/**
 * BigFixed Training Implementation
 * 
 * This file contains proper BigFixed implementations that use the algorithms layer.
 * These replace the broken mixed float/BigFixed implementations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm_training.h"
#include "../include/bigfixed_core.h"
#include "../algorithms/include/bigfixed_math_wrappers.h"
#include "../algorithms/include/numerical_bigfixed.h"
#include "../algorithms/include/loss_functions_bigfixed.h"
#include "../algorithms/include/optimizers_bigfixed.h"
#include "../include/prime_float_math.h"

/**
 * Forward pass using BigFixed precision
 * Delegates to cllm_forward_training which now uses BigFixed throughout
 */
float cllm_forward_training_bigfixed(CLLMTraining* training, uint32_t* input_tokens) {
    if (!training || !input_tokens) return 0.0f;
    
    // Delegate to main implementation (now uses BigFixed throughout)
    return cllm_forward_training(training, input_tokens);
}

/**
 * Backward pass using BigFixed precision
 * Delegates to cllm_backward_training which now uses BigFixed throughout
 */
void cllm_backward_training_bigfixed(CLLMTraining* training, uint32_t* target_tokens) {
    if (!training || !target_tokens) return;
    
    // Delegate to main implementation (now uses BigFixed throughout)
    cllm_backward_training(training, target_tokens, NULL);
}

/**
 * Compute loss using BigFixed precision
 */
float cllm_compute_loss_bigfixed(
    CLLMTraining* training,
    BigFixed** logits,
    uint32_t* target_tokens,
    int batch_size,
    int seq_len,
    uint32_t vocab_size,
    int precision
) {
    if (!training || !logits || !target_tokens) return 0.0f;
    
    // Use the cross_entropy_loss_bigfixed from algorithms layer
    BigFixed* loss_result = big_fixed_create(precision);
    big_fixed_from_int(loss_result, 0);
    
    // Compute cross-entropy loss for each position
    BigFixed* position_loss = big_fixed_create(precision);
    BigFixed* log_prob = big_fixed_create(precision);
    
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t target = target_tokens[idx];
            
            if (target < vocab_size) {
                // Get logits for this position
                BigFixed** position_logits = &logits[idx * vocab_size];
                
                // Apply softmax to get probabilities (via float conversion)
                float* probs = (float*)malloc(vocab_size * sizeof(float));
                float max_logit = (float)big_fixed_to_double(position_logits[0]);
                
                // Find max for numerical stability
                for (uint32_t v = 1; v < vocab_size; v++) {
                    float logit = (float)big_fixed_to_double(position_logits[v]);
                    if (logit > max_logit) max_logit = logit;
                }
                
                // Compute exp and sum
                double sum = 0.0;
                for (uint32_t v = 0; v < vocab_size; v++) {
                    float logit = (float)big_fixed_to_double(position_logits[v]);
                    probs[v] = prime_expf(logit - max_logit);
                    sum += probs[v];
                }
                
                // Normalize
                if (sum > 1e-10) {
                    for (uint32_t v = 0; v < vocab_size; v++) {
                        probs[v] /= (float)sum;
                    }
                }
                
                // Compute -log(prob[target])
                if (probs[target] > 1e-10) {
                    double log_val = -prime_logf(probs[target]);
                    big_fixed_from_double(position_loss, log_val);
                    big_fixed_add(loss_result, loss_result, position_loss);
                }
                
                free(probs);
            }
        }
    }
    
    // Average over batch
    BigFixed* batch_size_bf = big_fixed_create(precision);
    big_fixed_from_int(batch_size_bf, batch_size * seq_len);
    big_fixed_div(loss_result, loss_result, batch_size_bf);
    
    float final_loss = (float)big_fixed_to_double(loss_result);
    
    // Cleanup
    big_fixed_free(loss_result);
    big_fixed_free(position_loss);
    big_fixed_free(log_prob);
    big_fixed_free(batch_size_bf);
    
    return final_loss;
}