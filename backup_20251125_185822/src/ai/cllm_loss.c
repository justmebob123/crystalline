/*
 * CLLM Loss Computation
 * Implements loss functions and gradient computation for training
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/prime_float_math.h"

/**
 * Compute softmax in-place
 * 
 * @param logits Input/output logits [size]
 * @param size Array size
 */
static void softmax_inplace(float* logits, int size) {
    if (!logits || size <= 0) return;
    
    // Find max for numerical stability
    float max_logit = logits[0];
    for (int i = 1; i < size; i++) {
        if (logits[i] > max_logit) {
            max_logit = logits[i];
        }
    }
    
    // Compute exp and sum
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        logits[i] = prime_exp(logits[i] - max_logit);
        sum += logits[i];
    }
    
    // Normalize
    if (sum > 1e-8f) {
        for (int i = 0; i < size; i++) {
            logits[i] /= sum;
        }
    }
}

/**
 * Compute cross-entropy loss
 * 
 * Loss = -log(P(target))
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target Target token ID
 * @param vocab_size Vocabulary size
 * @return Cross-entropy loss value
 */
float cllm_compute_cross_entropy_loss(float* logits, uint32_t target, int vocab_size) {
    if (!logits || target >= (uint32_t)vocab_size || vocab_size <= 0) {
        return 0.0f;
    }
    
    // Create copy for softmax
    float* probs = (float*)malloc(vocab_size * sizeof(float));
    if (!probs) return 0.0f;
    
    memcpy(probs, logits, vocab_size * sizeof(float));
    
    // Apply softmax
    softmax_inplace(probs, vocab_size);
    
    // Compute loss: -log(P(target))
    float loss = -prime_log(probs[target] + 1e-8f);
    
    free(probs);
    return loss;
}

/**
 * Compute cross-entropy loss gradient
 * 
 * Gradient = P(predicted) - 1[target]
 * where 1[target] is one-hot vector
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target Target token ID
 * @param grad_output Output gradient [vocab_size]
 * @param vocab_size Vocabulary size
 */
void cllm_compute_loss_gradient(float* logits, uint32_t target, 
                                float* grad_output, int vocab_size) {
    if (!logits || !grad_output || target >= (uint32_t)vocab_size || vocab_size <= 0) {
        return;
    }
    
    // Compute softmax probabilities
    memcpy(grad_output, logits, vocab_size * sizeof(float));
    softmax_inplace(grad_output, vocab_size);
    
    // Subtract 1 from target position: grad = P - 1[target]
    grad_output[target] -= 1.0f;
}

/**
 * Compute batch cross-entropy loss
 * 
 * @param logits Predicted logits [batch_size x vocab_size]
 * @param targets Target token IDs [batch_size]
 * @param batch_size Batch size
 * @param vocab_size Vocabulary size
 * @return Average loss over batch
 */
float cllm_compute_batch_loss(float* logits, uint32_t* targets, 
                              int batch_size, int vocab_size) {
    if (!logits || !targets || batch_size <= 0 || vocab_size <= 0) {
        return 0.0f;
    }
    
    float total_loss = 0.0f;
    
    for (int i = 0; i < batch_size; i++) {
        float* batch_logits = &logits[i * vocab_size];
        float loss = cllm_compute_cross_entropy_loss(batch_logits, targets[i], vocab_size);
        total_loss += loss;
    }
    
    return total_loss / (float)batch_size;
}

/**
 * Compute perplexity from loss
 * 
 * Perplexity = exp(loss)
 * 
 * @param loss Cross-entropy loss
 * @return Perplexity value
 */
float cllm_compute_perplexity(float loss) {
    return prime_exp(loss);
}

/**
 * Compute label smoothing loss
 * Smooths the target distribution to prevent overconfidence
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target Target token ID
 * @param vocab_size Vocabulary size
 * @param smoothing Smoothing factor (typically 0.1)
 * @return Smoothed cross-entropy loss
 */
float cllm_compute_label_smoothing_loss(float* logits, uint32_t target, 
                                       int vocab_size, float smoothing) {
    if (!logits || target >= (uint32_t)vocab_size || vocab_size <= 0) {
        return 0.0f;
    }
    
    // Create copy for softmax
    float* probs = (float*)malloc(vocab_size * sizeof(float));
    if (!probs) return 0.0f;
    
    memcpy(probs, logits, vocab_size * sizeof(float));
    softmax_inplace(probs, vocab_size);
    
    // Compute smoothed target distribution
    float confidence = 1.0f - smoothing;
    float smooth_prob = smoothing / (float)vocab_size;
    
    // Loss = -sum(q * log(p))
    // where q is smoothed target distribution
    float loss = 0.0f;
    for (int i = 0; i < vocab_size; i++) {
        float q = (i == (int)target) ? confidence + smooth_prob : smooth_prob;
        loss -= q * prime_log(probs[i] + 1e-8f);
    }
    
    free(probs);
    return loss;
}

/**
 * Compute KL divergence loss
 * Measures divergence between predicted and target distributions
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target_dist Target distribution [vocab_size]
 * @param vocab_size Vocabulary size
 * @return KL divergence
 */
float cllm_compute_kl_divergence(float* logits, float* target_dist, int vocab_size) {
    if (!logits || !target_dist || vocab_size <= 0) {
        return 0.0f;
    }
    
    // Compute predicted probabilities
    float* probs = (float*)malloc(vocab_size * sizeof(float));
    if (!probs) return 0.0f;
    
    memcpy(probs, logits, vocab_size * sizeof(float));
    softmax_inplace(probs, vocab_size);
    
    // KL(P||Q) = sum(P * log(P/Q))
    float kl = 0.0f;
    for (int i = 0; i < vocab_size; i++) {
        if (target_dist[i] > 1e-8f) {
            kl += target_dist[i] * prime_log((target_dist[i] + 1e-8f) / (probs[i] + 1e-8f));
        }
    }
    
    free(probs);
    return kl;
}

/**
 * Compute sequence loss (for full sequence prediction)
 * 
 * @param logits Predicted logits [seq_len x vocab_size]
 * @param targets Target token IDs [seq_len]
 * @param seq_len Sequence length
 * @param vocab_size Vocabulary size
 * @return Average loss over sequence
 */
float cllm_compute_sequence_loss(float* logits, uint32_t* targets,
                                 int seq_len, int vocab_size) {
    if (!logits || !targets || seq_len <= 0 || vocab_size <= 0) {
        return 0.0f;
    }
    
    float total_loss = 0.0f;
    
    for (int t = 0; t < seq_len; t++) {
        float* step_logits = &logits[t * vocab_size];
        float loss = cllm_compute_cross_entropy_loss(step_logits, targets[t], vocab_size);
        total_loss += loss;
    }
    
    return total_loss / (float)seq_len;
}

/**
 * Compute accuracy
 * 
 * @param logits Predicted logits [batch_size x vocab_size]
 * @param targets Target token IDs [batch_size]
 * @param batch_size Batch size
 * @param vocab_size Vocabulary size
 * @return Accuracy (0 to 1)
 */
float cllm_compute_accuracy(float* logits, uint32_t* targets,
                           int batch_size, int vocab_size) {
    if (!logits || !targets || batch_size <= 0 || vocab_size <= 0) {
        return 0.0f;
    }
    
    int correct = 0;
    
    for (int i = 0; i < batch_size; i++) {
        float* batch_logits = &logits[i * vocab_size];
        
        // Find argmax
        int pred = 0;
        float max_logit = batch_logits[0];
        for (int j = 1; j < vocab_size; j++) {
            if (batch_logits[j] > max_logit) {
                max_logit = batch_logits[j];
                pred = j;
            }
        }
        
        if (pred == (int)targets[i]) {
            correct++;
        }
    }
    
    return (float)correct / (float)batch_size;
}

/**
 * Compute top-k accuracy
 * 
 * @param logits Predicted logits [batch_size x vocab_size]
 * @param targets Target token IDs [batch_size]
 * @param batch_size Batch size
 * @param vocab_size Vocabulary size
 * @param k Top-k value
 * @return Top-k accuracy (0 to 1)
 */
float cllm_compute_top_k_accuracy(float* logits, uint32_t* targets,
                                  int batch_size, int vocab_size, int k) {
    if (!logits || !targets || batch_size <= 0 || vocab_size <= 0 || k <= 0) {
        return 0.0f;
    }
    
    int correct = 0;
    
    for (int i = 0; i < batch_size; i++) {
        float* batch_logits = &logits[i * vocab_size];
        uint32_t target = targets[i];
        
        // Find top-k predictions
        int* top_k_indices = (int*)malloc(k * sizeof(int));
        if (!top_k_indices) continue;
        
        // Simple selection of top-k
        for (int j = 0; j < k && j < vocab_size; j++) {
            int max_idx = 0;
            float max_val = -1e9f;
            
            for (int m = 0; m < vocab_size; m++) {
                // Check if already selected
                int already_selected = 0;
                for (int n = 0; n < j; n++) {
                    if (top_k_indices[n] == m) {
                        already_selected = 1;
                        break;
                    }
                }
                
                if (!already_selected && batch_logits[m] > max_val) {
                    max_val = batch_logits[m];
                    max_idx = m;
                }
            }
            
            top_k_indices[j] = max_idx;
        }
        
        // Check if target is in top-k
        for (int j = 0; j < k; j++) {
            if (top_k_indices[j] == (int)target) {
                correct++;
                break;
            }
        }
        
        free(top_k_indices);
    }
    
    return (float)correct / (float)batch_size;
}