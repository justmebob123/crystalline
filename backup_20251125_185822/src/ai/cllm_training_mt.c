/**
 * Multi-Threaded Training - PROPER IMPLEMENTATION
 * 
 * Data-parallel training approach:
 * 1. Each thread gets its own CLLMTraining context
 * 2. All threads share the same model weights (read-only during forward)
 * 3. Each thread accumulates gradients independently  
 * 4. After all threads finish, accumulate gradients
 * 5. Single optimizer step with accumulated gradients
 * 
 * This is the standard approach used by PyTorch, TensorFlow, etc.
 */

#include "cllm_training.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Thread work context
typedef struct {
    CLLMTraining* thread_training;  // Independent training context for this thread
    int thread_id;
    
    // Batch assignment
    int start_batch;
    int end_batch;
    
    // Results
    float total_loss;
    int batches_completed;
} ThreadContext;

// Multi-threaded training state
typedef struct {
    CLLMModel* shared_model;        // Shared model (weights are read-only during forward)
    CLLMTraining* main_training;    // Main training context (for final gradient accumulation)
    ThreadContext* thread_contexts;
    int num_threads;
    int total_batches;
} MTTrainingState;

/**
 * Create a training context for a thread
 * Shares the model but has independent gradients
 */
static CLLMTraining* create_thread_training(CLLMModel* model, CLLMTrainingConfig* config) {
    // Create training context
    CLLMTraining* training = cllm_training_init(model, config);
    if (!training) {
        return NULL;
    }
    
    // The training context now has its own gradient buffers
    // but shares the model weights
    return training;
}

/**
 * Worker thread function
 */
static void* worker_thread(void* arg) {
    ThreadContext* ctx = (ThreadContext*)arg;
    CLLMTraining* training = ctx->thread_training;
    
    printf("Thread %d: Processing batches %d to %d\n", 
           ctx->thread_id, ctx->start_batch, ctx->end_batch);
    fflush(stdout);
    
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    int tokens_per_batch = batch_size * seq_len;
    
    // Allocate batch buffers
    uint32_t* input_tokens = (uint32_t*)malloc(tokens_per_batch * sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)malloc(tokens_per_batch * sizeof(uint32_t));
    
    if (!input_tokens || !target_tokens) {
        fprintf(stderr, "Thread %d: Failed to allocate batch buffers\n", ctx->thread_id);
        free(input_tokens);
        free(target_tokens);
        return NULL;
    }
    
    float thread_loss = 0.0f;
    int batches_processed = 0;
    
    // Process assigned batches
    for (int batch_idx = ctx->start_batch; batch_idx < ctx->end_batch; batch_idx++) {
        // Get batch data
        int token_offset = batch_idx * tokens_per_batch;
        if (token_offset + tokens_per_batch > (int)training->num_tokens) {
            break;
        }
        
        // Copy tokens for this batch
        memcpy(input_tokens, &training->tokens[token_offset], tokens_per_batch * sizeof(uint32_t));
        memcpy(target_tokens, &training->tokens[token_offset + 1], (tokens_per_batch - 1) * sizeof(uint32_t));
        target_tokens[tokens_per_batch - 1] = training->tokens[token_offset];
        
        // Forward pass (reads model weights)
        float loss = cllm_forward_training(training, input_tokens);
        
        // Compute loss
        loss += cllm_compute_loss_training(training, target_tokens);
        
        // Backward pass (writes to thread-local gradients)
        cllm_backward_training(training, target_tokens);
        
        thread_loss += loss;
        batches_processed++;
        
        if (batches_processed % 10 == 0) {
            printf("Thread %d: Processed %d/%d batches, avg loss=%.4f\n",
                   ctx->thread_id, batches_processed, ctx->end_batch - ctx->start_batch,
                   thread_loss / batches_processed);
            fflush(stdout);
        }
    }
    
    // Store results
    ctx->total_loss = thread_loss;
    ctx->batches_completed = batches_processed;
    
    printf("Thread %d: Finished - %d batches, avg loss=%.4f\n",
           ctx->thread_id, batches_processed, 
           batches_processed > 0 ? thread_loss / batches_processed : 0.0f);
    fflush(stdout);
    
    free(input_tokens);
    free(target_tokens);
    
    return NULL;
}

/**
 * Accumulate gradients from all threads into main training context
 */
static void accumulate_thread_gradients(MTTrainingState* mt_state) {
    CLLMTraining* main_training = mt_state->main_training;
    CLLMModel* model = mt_state->shared_model;
    
    uint32_t vocab_size = model->vocab_size;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t num_layers = model->num_layers;
    
    printf("Accumulating gradients from %d threads...\n", mt_state->num_threads);
    fflush(stdout);
    
    // Zero out main gradient buffers
    memset(main_training->gradients, 0, vocab_size * embed_dim * sizeof(float));
    
    for (uint32_t layer = 0; layer < num_layers; layer++) {
        uint64_t attn_size = embed_dim * embed_dim;
        memset(main_training->attention_grads[layer].query_lattice, 0, attn_size * sizeof(float));
        memset(main_training->attention_grads[layer].key_lattice, 0, attn_size * sizeof(float));
        memset(main_training->attention_grads[layer].value_lattice, 0, attn_size * sizeof(float));
        
        uint64_t ff_size = embed_dim * embed_dim;
        memset(main_training->ff_grads[layer].w1_lattice, 0, ff_size * sizeof(float));
        memset(main_training->ff_grads[layer].w2_lattice, 0, ff_size * sizeof(float));
        memset(main_training->ff_grads[layer].bias1, 0, embed_dim * sizeof(float));
        memset(main_training->ff_grads[layer].bias2, 0, embed_dim * sizeof(float));
        
        memset(main_training->ln_grads[layer].gamma, 0, embed_dim * sizeof(float));
        memset(main_training->ln_grads[layer].beta, 0, embed_dim * sizeof(float));
    }
    
    // Accumulate from all threads
    for (int t = 0; t < mt_state->num_threads; t++) {
        CLLMTraining* thread_training = mt_state->thread_contexts[t].thread_training;
        
        // Accumulate embedding gradients
        for (size_t i = 0; i < vocab_size * embed_dim; i++) {
            main_training->gradients[i] += thread_training->gradients[i];
        }
        
        // Accumulate layer gradients
        for (uint32_t layer = 0; layer < num_layers; layer++) {
            uint64_t attn_size = embed_dim * embed_dim;
            for (uint64_t i = 0; i < attn_size; i++) {
                main_training->attention_grads[layer].query_lattice[i] += 
                    thread_training->attention_grads[layer].query_lattice[i];
                main_training->attention_grads[layer].key_lattice[i] += 
                    thread_training->attention_grads[layer].key_lattice[i];
                main_training->attention_grads[layer].value_lattice[i] += 
                    thread_training->attention_grads[layer].value_lattice[i];
            }
            
            uint64_t ff_size = embed_dim * embed_dim;
            for (uint64_t i = 0; i < ff_size; i++) {
                main_training->ff_grads[layer].w1_lattice[i] += 
                    thread_training->ff_grads[layer].w1_lattice[i];
                main_training->ff_grads[layer].w2_lattice[i] += 
                    thread_training->ff_grads[layer].w2_lattice[i];
            }
            for (uint32_t i = 0; i < embed_dim; i++) {
                main_training->ff_grads[layer].bias1[i] += 
                    thread_training->ff_grads[layer].bias1[i];
                main_training->ff_grads[layer].bias2[i] += 
                    thread_training->ff_grads[layer].bias2[i];
            }
            
            for (uint32_t i = 0; i < embed_dim; i++) {
                main_training->ln_grads[layer].gamma[i] += 
                    thread_training->ln_grads[layer].gamma[i];
                main_training->ln_grads[layer].beta[i] += 
                    thread_training->ln_grads[layer].beta[i];
            }
        }
    }
    
    // Average gradients
    int total_batches = 0;
    for (int t = 0; t < mt_state->num_threads; t++) {
        total_batches += mt_state->thread_contexts[t].batches_completed;
    }
    
    if (total_batches > 0) {
        float scale = 1.0f / total_batches;
        
        for (size_t i = 0; i < vocab_size * embed_dim; i++) {
            main_training->gradients[i] *= scale;
        }
        
        for (uint32_t layer = 0; layer < num_layers; layer++) {
            uint64_t attn_size = embed_dim * embed_dim;
            for (uint64_t i = 0; i < attn_size; i++) {
                main_training->attention_grads[layer].query_lattice[i] *= scale;
                main_training->attention_grads[layer].key_lattice[i] *= scale;
                main_training->attention_grads[layer].value_lattice[i] *= scale;
            }
            
            uint64_t ff_size = embed_dim * embed_dim;
            for (uint64_t i = 0; i < ff_size; i++) {
                main_training->ff_grads[layer].w1_lattice[i] *= scale;
                main_training->ff_grads[layer].w2_lattice[i] *= scale;
            }
            for (uint32_t i = 0; i < embed_dim; i++) {
                main_training->ff_grads[layer].bias1[i] *= scale;
                main_training->ff_grads[layer].bias2[i] *= scale;
            }
            
            for (uint32_t i = 0; i < embed_dim; i++) {
                main_training->ln_grads[layer].gamma[i] *= scale;
                main_training->ln_grads[layer].beta[i] *= scale;
            }
        }
    }
    
    printf("Gradient accumulation complete\n");
    fflush(stdout);
}

/**
 * Multi-threaded training epoch
 */
float cllm_train_epoch_mt(CLLMTraining* training, int num_threads) {
    if (!training || num_threads <= 0) return 0.0f;
    
    // Calculate total batches
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    int tokens_per_batch = batch_size * seq_len;
    int total_batches = training->num_tokens / tokens_per_batch;
    
    if (total_batches == 0) {
        printf("Not enough tokens for even one batch!\n");
        return 0.0f;
    }
    
    printf("\n=== MULTI-THREADED TRAINING ===\n");
    printf("Threads: %d\n", num_threads);
    printf("Total batches: %d\n", total_batches);
    printf("Batches per thread: ~%d\n", (total_batches + num_threads - 1) / num_threads);
    fflush(stdout);
    
    // Initialize MT state
    MTTrainingState mt_state = {0};
    mt_state.shared_model = training->model;
    mt_state.main_training = training;
    mt_state.num_threads = num_threads;
    mt_state.total_batches = total_batches;
    
    // Allocate thread contexts
    mt_state.thread_contexts = (ThreadContext*)calloc(num_threads, sizeof(ThreadContext));
    if (!mt_state.thread_contexts) {
        fprintf(stderr, "ERROR: Failed to allocate thread contexts\n");
        return 0.0f;
    }
    
    // Create training context for each thread
    int batches_per_thread = (total_batches + num_threads - 1) / num_threads;
    for (int i = 0; i < num_threads; i++) {
        printf("Creating training context for thread %d...\n", i);
        fflush(stdout);
        
        CLLMTraining* thread_training = create_thread_training(training->model, &training->config);
        if (!thread_training) {
            fprintf(stderr, "ERROR: Failed to create training context for thread %d\n", i);
            // Cleanup
            for (int j = 0; j < i; j++) {
                cllm_training_free(mt_state.thread_contexts[j].thread_training);
            }
            free(mt_state.thread_contexts);
            return 0.0f;
        }
        
        // Share the training data
        thread_training->tokens = training->tokens;
        thread_training->num_tokens = training->num_tokens;
        
        mt_state.thread_contexts[i].thread_training = thread_training;
        mt_state.thread_contexts[i].thread_id = i;
        mt_state.thread_contexts[i].start_batch = i * batches_per_thread;
        mt_state.thread_contexts[i].end_batch = (i + 1) * batches_per_thread;
        if (mt_state.thread_contexts[i].end_batch > total_batches) {
            mt_state.thread_contexts[i].end_batch = total_batches;
        }
    }
    
    printf("All thread contexts created. Starting training...\n");
    fflush(stdout);
    
    // Create worker threads
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &mt_state.thread_contexts[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("All threads completed. Accumulating gradients...\n");
    fflush(stdout);
    
    // Accumulate results
    float total_loss = 0.0f;
    int total_batches_processed = 0;
    for (int i = 0; i < num_threads; i++) {
        total_loss += mt_state.thread_contexts[i].total_loss;
        total_batches_processed += mt_state.thread_contexts[i].batches_completed;
    }
    
    // Accumulate gradients from all threads
    accumulate_thread_gradients(&mt_state);
    
    // Single optimizer step with accumulated gradients
    printf("Applying optimizer step...\n");
    fflush(stdout);
    cllm_optimizer_step(training);
    
    // Calculate average loss
    float avg_loss = total_batches_processed > 0 
        ? total_loss / total_batches_processed 
        : 0.0f;
    
    printf("\n=== MT EPOCH COMPLETE ===\n");
    printf("Total batches processed: %d\n", total_batches_processed);
    printf("Average loss: %.4f\n", avg_loss);
    printf("=========================\n\n");
    fflush(stdout);
    
    // Cleanup
    for (int i = 0; i < num_threads; i++) {
        cllm_training_free(mt_state.thread_contexts[i].thread_training);
    }
    free(mt_state.thread_contexts);
    free(threads);
    
    return avg_loss;
}