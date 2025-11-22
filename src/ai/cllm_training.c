/*
 * CLLM Training Pipeline
 * Training system for Crystalline Lattice Language Models
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/cllm_format.h"
#include "../include/cllm_training.h"
#include "../include/prime_float_math.h"

#define MAX_BATCH_SIZE 128
#define MAX_SEQUENCE_LENGTH 2048

// Initialize training state
CLLMTraining* cllm_training_init(CLLMModel* model, CLLMTrainingConfig* config) {
    if (!model || !config) return NULL;
    
    CLLMTraining* training = (CLLMTraining*)calloc(1, sizeof(CLLMTraining));
    if (!training) return NULL;
    
    training->model = model;
    training->config = *config;
    training->current_epoch = 0;
    training->current_step = 0;
    training->best_loss = 1e9f;
    
    // Allocate gradient buffers
    size_t total_params = model->header.total_params;
    training->gradients = (float*)calloc(total_params, sizeof(float));
    training->optimizer_state = (float*)calloc(total_params * 2, sizeof(float)); // For Adam: m and v
    
    if (!training->gradients || !training->optimizer_state) {
        cllm_training_cleanup(training);
        return NULL;
    }
    
    // Initialize optimizer state
    memset(training->optimizer_state, 0, total_params * 2 * sizeof(float));
    
    training->start_time = time(NULL);
    
    return training;
}

// Load training data from file
int cllm_load_training_data(CLLMTraining* training, const char* filename) {
    if (!training || !filename) return -1;
    
    FILE* f = fopen(filename, "r");
    if (!f) return -1;
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Read file content
    char* content = (char*)malloc(file_size + 1);
    if (!content) {
        fclose(f);
        return -1;
    }
    
    size_t bytes_read = fread(content, 1, file_size, f);
    content[bytes_read] = '\0';
    fclose(f);
    
    // Tokenize content (simple whitespace tokenization)
    training->num_tokens = 0;
    training->tokens = (uint32_t*)malloc(file_size * sizeof(uint32_t)); // Overallocate
    
    char* token = strtok(content, " \n\t");
    while (token != NULL && training->num_tokens < (size_t)file_size) {
        // Find token in vocabulary
        for (uint32_t i = 0; i < training->model->vocab_size; i++) {
            if (strcmp(training->model->tokens[i].token_str, token) == 0) {
                training->tokens[training->num_tokens++] = i;
                break;
            }
        }
        token = strtok(NULL, " \n\t");
    }
    
    free(content);
    
    // Calculate number of batches
    int tokens_per_batch = training->config.batch_size * training->config.sequence_length;
    training->total_batches = training->num_tokens / tokens_per_batch;
    
    return training->num_tokens;
}

// Get next training batch
int cllm_get_batch(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens) {
    if (!training || !input_tokens || !target_tokens) return -1;
    
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    int tokens_per_batch = batch_size * seq_len;
    
    // Check if we have enough tokens
    if (training->current_batch_offset + (size_t)tokens_per_batch + 1 > training->num_tokens) {
        // Shuffle and restart
        training->current_batch_offset = 0;
        training->current_epoch++;
        return 0; // End of epoch
    }
    
    // Extract batch
    for (int i = 0; i < batch_size; i++) {
        for (int j = 0; j < seq_len; j++) {
            int idx = training->current_batch_offset + i * seq_len + j;
            input_tokens[i * seq_len + j] = training->tokens[idx];
            target_tokens[i * seq_len + j] = training->tokens[idx + 1];
        }
    }
    
    training->current_batch_offset += tokens_per_batch;
    return tokens_per_batch;
}

// Forward pass (compute loss)
float cllm_compute_loss(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens, int num_tokens __attribute__((unused))) {
    if (!training || !input_tokens || !target_tokens) return 0.0f;
    
    // This is a simplified version - full implementation would:
    // 1. Forward pass through all layers
    // 2. Compute cross-entropy loss
    // 3. Store activations for backward pass
    
    float total_loss = 0.0f;
    int count = 0;
    
    // Simplified loss computation
    for (int i = 0; i < num_tokens; i++) {
        uint32_t input = input_tokens[i];
        uint32_t target = target_tokens[i];
        
        if (input < training->model->vocab_size && target < training->model->vocab_size) {
            // Simplified: just use embedding similarity as proxy for loss
            float* input_embed = &training->model->embeddings.embeddings[input * training->model->embeddings.embedding_dim];
            float* target_embed = &training->model->embeddings.embeddings[target * training->model->embeddings.embedding_dim];
            
            float similarity = 0.0f;
            for (uint32_t j = 0; j < training->model->embeddings.embedding_dim; j++) {
                similarity += input_embed[j] * target_embed[j];
            }
            
            // Convert to loss (negative log likelihood approximation)
            total_loss += -prime_logf(prime_fmaxf(similarity, 1e-10f));
            count++;
        }
    }
    
    return count > 0 ? total_loss / count : 0.0f;
}

// Backward pass (compute gradients)
void cllm_backward(CLLMTraining* training, uint32_t* input_tokens, uint32_t* target_tokens, int num_tokens __attribute__((unused))) {
    if (!training || !input_tokens || !target_tokens) return;
    
    // This is a placeholder - full implementation would:
    // 1. Backpropagate through all layers
    // 2. Compute gradients for all parameters
    // 3. Store gradients in training->gradients
    
    // For now, just zero out gradients
    size_t total_params = training->model->header.total_params;
    memset(training->gradients, 0, total_params * sizeof(float));
    
    // TODO: Implement full backpropagation
}

// Adam optimizer step
void cllm_optimizer_step(CLLMTraining* training) {
    if (!training) return;
    
    float lr = training->config.learning_rate;
    float beta1 = 0.9f;
    float beta2 = 0.999f;
    
    size_t total_params = training->model->header.total_params;
    float* m = training->optimizer_state;  // First moment
    float* v = &training->optimizer_state[total_params];  // Second moment
    
    int t = training->current_step + 1;
    float lr_t = lr * prime_sqrtf(1.0f - prime_powf(beta2, t)) / (1.0f - prime_powf(beta1, t));
    (void)lr_t; // Will be used when applying updates to parameters
    
    // Update parameters (simplified - would need to map to actual model parameters)
    for (size_t i = 0; i < total_params; i++) {
        float g = training->gradients[i];
        
        // Update biased first moment estimate
        m[i] = beta1 * m[i] + (1.0f - beta1) * g;
        
        // Update biased second raw moment estimate
        v[i] = beta2 * v[i] + (1.0f - beta2) * g * g;
        
        // Compute update (would apply to actual parameters)
        // float update = lr_t * m[i] / (prime_sqrtf(v[i]) + epsilon);
        
        // TODO: Apply update to actual model parameters
    }
}

// Train for one epoch
float cllm_train_epoch(CLLMTraining* training) {
    if (!training) return 0.0f;
    
    float epoch_loss = 0.0f;
    int num_batches = 0;
    
    uint32_t* input_tokens = (uint32_t*)malloc(training->config.batch_size * 
                                               training->config.sequence_length * 
                                               sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)malloc(training->config.batch_size * 
                                                training->config.sequence_length * 
                                                sizeof(uint32_t));
    
    training->current_batch_offset = 0;
    
    while (1) {
        // Get batch
        int tokens = cllm_get_batch(training, input_tokens, target_tokens);
        if (tokens == 0) break; // End of epoch
        
        // Forward pass
        float loss = cllm_compute_loss(training, input_tokens, target_tokens, tokens);
        epoch_loss += loss;
        num_batches++;
        
        // Backward pass
        cllm_backward(training, input_tokens, target_tokens, tokens);
        
        // Optimizer step
        cllm_optimizer_step(training);
        
        training->current_step++;
        training->current_loss = loss;
        
        // Update best loss
        if (loss < training->best_loss) {
            training->best_loss = loss;
        }
        
        // Print progress
        if (training->current_step % 10 == 0) {
            printf("Step %d: Loss = %.4f (Best: %.4f)\n", 
                   training->current_step, loss, training->best_loss);
        }
    }
    
    free(input_tokens);
    free(target_tokens);
    
    return num_batches > 0 ? epoch_loss / num_batches : 0.0f;
}

// Train the model
int cllm_train(CLLMTraining* training) {
    if (!training) return -1;
    
    printf("Starting training...\n");
    printf("Epochs: %d\n", training->config.num_epochs);
    printf("Batch size: %d\n", training->config.batch_size);
    printf("Sequence length: %d\n", training->config.sequence_length);
    printf("Learning rate: %.6f\n", training->config.learning_rate);
    printf("Total tokens: %zu\n", training->num_tokens);
    printf("Total batches per epoch: %d\n", training->total_batches);
    printf("\n");
    
    for (int epoch = 0; epoch < training->config.num_epochs; epoch++) {
        training->current_epoch = epoch;
        
        printf("Epoch %d/%d\n", epoch + 1, training->config.num_epochs);
        
        float epoch_loss = cllm_train_epoch(training);
        
        printf("Epoch %d complete: Average Loss = %.4f\n\n", epoch + 1, epoch_loss);
        
        // Save checkpoint
        if ((epoch + 1) % training->config.save_every == 0) {
            char checkpoint_path[256];
            snprintf(checkpoint_path, sizeof(checkpoint_path), 
                    "checkpoint_epoch_%d.cllm", epoch + 1);
            cllm_write(checkpoint_path, training->model);
            printf("Checkpoint saved: %s\n", checkpoint_path);
        }
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, training->start_time);
    
    printf("\nTraining complete!\n");
    printf("Total time: %.0f seconds\n", elapsed);
    printf("Final loss: %.4f\n", training->current_loss);
    printf("Best loss: %.4f\n", training->best_loss);
    
    return 0;
}

// Save training checkpoint
int cllm_save_checkpoint(CLLMTraining* training, const char* filename) {
    if (!training || !filename) return -1;
    
    // Save model
    if (cllm_write(filename, training->model) != 0) {
        return -1;
    }
    
    // Save training state
    char state_file[512];
    snprintf(state_file, sizeof(state_file), "%s.state", filename);
    
    FILE* f = fopen(state_file, "wb");
    if (!f) return -1;
    
    fwrite(&training->current_epoch, sizeof(int), 1, f);
    fwrite(&training->current_step, sizeof(int), 1, f);
    fwrite(&training->current_loss, sizeof(float), 1, f);
    fwrite(&training->best_loss, sizeof(float), 1, f);
    
    size_t total_params = training->model->header.total_params;
    fwrite(training->optimizer_state, sizeof(float), total_params * 2, f);
    
    fclose(f);
    
    return 0;
}

// Load training checkpoint
int cllm_load_checkpoint(CLLMTraining* training, const char* filename) {
    if (!training || !filename) return -1;
    
    // Load training state
    char state_file[512];
    snprintf(state_file, sizeof(state_file), "%s.state", filename);
    
    FILE* f = fopen(state_file, "rb");
    if (!f) return -1;
    
    if (fread(&training->current_epoch, sizeof(int), 1, f) != 1) {
        fprintf(stderr, "Error reading current_epoch\n");
        fclose(f);
        return false;
    }
    if (fread(&training->current_step, sizeof(int), 1, f) != 1) {
        fprintf(stderr, "Error reading current_step\n");
        fclose(f);
        return false;
    }
    if (fread(&training->current_loss, sizeof(float), 1, f) != 1) {
        fprintf(stderr, "Error reading current_loss\n");
        fclose(f);
        return false;
    }
    if (fread(&training->best_loss, sizeof(float), 1, f) != 1) {
        fprintf(stderr, "Error reading best_loss\n");
        fclose(f);
        return false;
    }
    
    size_t total_params = training->model->header.total_params;
    if (fread(training->optimizer_state, sizeof(float), total_params * 2, f) != total_params * 2) {
        fprintf(stderr, "Error reading optimizer_state\n");
        fclose(f);
        return false;
    }
    
    fclose(f);
    
    return 0;
}

// Cleanup
void cllm_training_cleanup(CLLMTraining* training) {
    if (!training) return;
    
    free(training->tokens);
    free(training->gradients);
    free(training->optimizer_state);
    free(training);
}

// Alias for compatibility
void cllm_training_free(CLLMTraining* training) {
    cllm_training_cleanup(training);
}