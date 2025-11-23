/**
 * PROPER Training Implementation
 * 
 * This implements a complete training loop with:
 * 1. Forward pass through all layers (with activation storage)
 * 2. Cross-entropy loss from logits
 * 3. Backward pass from loss gradients (proper cross-entropy gradients)
 * 4. Proper parameter updates
 */

#include "cllm_training.h"
#include "cllm_activations.h"
#include "cllm_forward_backward_proper.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Old forward/loss functions removed - now using proper implementation
// with activation storage in cllm_forward_with_storage.c

/**
 * Train one epoch with proper forward/backward pass
 * 
 * @param training Training state
 * @return Average loss for epoch
 */
float cllm_train_epoch_proper(CLLMTraining* training) {
    if (!training) return 0.0f;
    
    // Validate training state
    if (!training->model) {
        fprintf(stderr, "ERROR: training->model is NULL\n");
        return 0.0f;
    }
    if (!training->tokens || training->num_tokens == 0) {
        fprintf(stderr, "ERROR: No training data loaded\n");
        return 0.0f;
    }
    
    printf("=== PROPER TRAINING MODE (WITH CORRECT GRADIENTS) ===\n");
    printf("Using complete forward pass + cross-entropy backward pass\n");
    printf("Training data: %zu tokens\n", training->num_tokens);
    
    float epoch_loss = 0.0f;
    int num_batches = 0;
    
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    uint32_t embed_dim = training->model->embedding_dim;
    uint32_t vocab_size = training->model->vocab_size;
    uint32_t num_layers = training->model->num_layers;
    
    // Allocate activation storage (reused across batches)
    ForwardActivations* activations = cllm_allocate_activations(
        batch_size, seq_len, embed_dim, vocab_size, num_layers
    );
    
    if (!activations) {
        fprintf(stderr, "ERROR: Failed to allocate activations\n");
        return 0.0f;
    }
    
    // Allocate token buffers
    uint32_t* input_tokens = (uint32_t*)malloc(batch_size * seq_len * sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)malloc(batch_size * seq_len * sizeof(uint32_t));
    
    if (!input_tokens || !target_tokens) {
        free(input_tokens);
        free(target_tokens);
        cllm_free_activations(activations);
        return 0.0f;
    }
    
    training->current_batch_offset = 0;
    
    while (1) {
        // Get batch
        int tokens = cllm_get_batch(training, input_tokens, target_tokens);
        if (tokens == 0) break;  // End of epoch
        
        // Forward pass with activation storage
        cllm_forward_with_activations(training, input_tokens, activations);
        
        // Compute loss from stored logits
        float loss = cllm_compute_loss_from_activations(activations, target_tokens);
        epoch_loss += loss;
        num_batches++;
        
        // Backward pass with proper cross-entropy gradients
        cllm_backward_cross_entropy(training, activations, target_tokens);
        
        // Optimizer step
        cllm_optimizer_step(training);
        
        training->current_step++;
        training->current_loss = loss;
        
        // Update best loss
        if (loss < training->best_loss) {
            training->best_loss = loss;
        }
        
        if (num_batches % 5 == 0) {
            printf("  Batch %d: loss = %.4f\n", num_batches, loss);
        }
    }
    
    free(input_tokens);
    free(target_tokens);
    cllm_free_activations(activations);
    
    printf("Proper training epoch complete: %d batches, avg loss = %.4f\n",
           num_batches, num_batches > 0 ? epoch_loss / num_batches : 0.0f);
    
    return num_batches > 0 ? epoch_loss / num_batches : 0.0f;
}