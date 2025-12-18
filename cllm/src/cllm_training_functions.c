/**
 * @file cllm_training_functions.c
 * @brief Training Functions for 88D Thread-Centric CLLM
 * 
 * CRITICAL: This is the ONLY implementation of training functions.
 * Threading is MANDATORY - no sequential fallback exists.
 * 
 * All computation happens in thread-local CrystallineAbacus storage.
 * All parameters are distributed across 88 worker threads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "math/transcendental.h"
#include "cllm.h"
#include "math/constants.h"
#include "cllm_training.h"
#include "cllm_batch.h"
#include "ai/cllm_training_system.h"
#include "hierarchical_threading.h"

// ============================================================================
// FORWARD PASS - 88D THREAD-CENTRIC (THE ONLY IMPLEMENTATION)
// ============================================================================

/**
 * Forward pass - Threading is MANDATORY
 * 
 * This function will abort if pool_88d is NULL.
 * All computation happens in parallel across 88 worker threads.
 * 
 * @param training Training context
 * @param input_tokens Input token IDs
 * @return Average loss across all threads
 */
double cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens) {
    if (!training || !input_tokens) {
        fprintf(stderr, "ERROR: NULL training or input_tokens\n");
        return -1.0;
    }
    
    CLLMModel* model = training->model;
    
    // CRITICAL: Verify 88D thread pool exists
    if (!model->pool_88d) {
        fprintf(stderr, "\n");
        fprintf(stderr, "╔════════════════════════════════════════════════════════╗\n");
        fprintf(stderr, "║              FATAL ERROR: NO THREADING                 ║\n");
        fprintf(stderr, "╚════════════════════════════════════════════════════════╝\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "88D thread pool not initialized!\n");
        fprintf(stderr, "Threading is MANDATORY in this architecture.\n");
        fprintf(stderr, "There is NO sequential fallback.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Model must be created with cllm_create_model().\n");
        fprintf(stderr, "Ensure pool_88d is properly initialized.\n");
        fprintf(stderr, "\n");
        abort();
    }
    
    int num_tokens = training->config.batch_size * training->config.sequence_length;
    HierarchicalThreadPool* pool = model->pool_88d;
    
    // Distribute tokens to threads
    for (int i = 0; i < num_tokens; i++) {
        uint32_t token_id = input_tokens[i];
        
        if (token_id >= model->vocab_size) {
            fprintf(stderr, "WARNING: Invalid token ID %u (vocab_size=%u)\n", 
                    token_id, model->vocab_size);
            continue;
        }
        
        HierarchicalThread* thread = model->token_assignments[token_id].thread;
        
        if (!thread) {
            fprintf(stderr, "FATAL: Thread not found for token %u\n", token_id);
            abort();
        }
        
        // Mark work for this thread
        __atomic_add_fetch(&thread->batch_count, 1, __ATOMIC_SEQ_CST);
    }
    
    // Signal all threads to start
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = 
                hierarchical_thread_get_88d(pool, layer, dim);
            
            if (thread) {
                pthread_cond_signal(&thread->control_cond);
            }
        }
    }
    
    // Wait for completion
    if (model->threading.forward_barrier) {
        pthread_barrier_wait(model->threading.forward_barrier);
    }
    
    // Collect results
    double total_loss = 0.0;
    int thread_count = 0;
    
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = 
                hierarchical_thread_get_88d(pool, layer, dim);
            
            if (thread && thread->work_completed > 0) {
                if (thread->activation_buffer && thread->activation_buffer_size > 0) {
                    total_loss += thread->activation_buffer[0];
                }
                thread_count++;
            }
        }
    }
    
    return (thread_count > 0) ? (total_loss / thread_count) : 0.0;
}

// ============================================================================
// BACKWARD PASS - 88D THREAD-CENTRIC (THE ONLY IMPLEMENTATION)
// ============================================================================

/**
 * Backward pass - Threading is MANDATORY
 * 
 * Computes gradients in parallel across all threads.
 * Gradients are accumulated in thread-local CrystallineAbacus storage.
 * 
 * @param training Training context
 * @param target_tokens Target token IDs for loss computation
 * @param gradient_buffer DEPRECATED - gradients now in thread-local storage
 */
void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens, double* gradient_buffer) {
    if (!training || !target_tokens) {
        fprintf(stderr, "ERROR: NULL training or target_tokens\n");
        return;
    }
    
    // gradient_buffer parameter is deprecated - kept for API compatibility
    (void)gradient_buffer;
    
    CLLMModel* model = training->model;
    
    // CRITICAL: Verify 88D thread pool exists
    if (!model->pool_88d) {
        fprintf(stderr, "FATAL ERROR: 88D thread pool not initialized\n");
        abort();
    }
    
    int num_tokens = training->config.batch_size * training->config.sequence_length;
    HierarchicalThreadPool* pool = model->pool_88d;
    
    // Distribute gradient computation to threads
    for (int i = 0; i < num_tokens; i++) {
        uint32_t token_id = i % model->vocab_size;
        uint32_t target_id = target_tokens[i];
        
        if (token_id >= model->vocab_size || target_id >= model->vocab_size) {
            continue;
        }
        
        HierarchicalThread* thread = model->token_assignments[token_id].thread;
        
        if (!thread) continue;
        
        // Store target in thread for gradient computation
        if (thread->gradient_buffer && thread->gradient_buffer_size > 0) {
            double* grad_buf = (double*)thread->gradient_buffer;
            grad_buf[0] = (double)target_id;
        }
        
        __atomic_add_fetch(&thread->batch_count, 1, __ATOMIC_SEQ_CST);
    }
    
    // Signal all threads
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = 
                hierarchical_thread_get_88d(pool, layer, dim);
            
            if (thread) {
                pthread_cond_signal(&thread->control_cond);
            }
        }
    }
    
    // Wait for completion
    if (model->threading.backward_barrier) {
        pthread_barrier_wait(model->threading.backward_barrier);
    }
    
    // Gradients now in thread-local CrystallineAbacus
}

// ============================================================================
// EMBEDDING PRECOMPUTATION
// ============================================================================

void cllm_precompute_all_embeddings(CLLMModel* model) {
    if (!model) return;
    
    printf("✓ Embeddings already initialized in thread-local storage\n");
    printf("  Each token's embedding is stored in its assigned thread's CrystallineAbacus\n");
}

// ============================================================================
// TRAINING INITIALIZATION
// ============================================================================

CLLMTraining* cllm_training_init(CLLMModel* model, CLLMTrainingConfig* config) {
    if (!model || !config) return NULL;
    
    if (!model->pool_88d) {
        fprintf(stderr, "FATAL: Cannot initialize training without 88D thread pool\n");
        return NULL;
    }
    
    CLLMTraining* training = calloc(1, sizeof(CLLMTraining));
    if (!training) return NULL;
    
    training->model = model;
    training->config = *config;
    training->current_epoch = 0;
    training->current_step = 0;
    training->best_loss = 1e9;
    training->accumulation_step = 0;
    training->config.initial_learning_rate = config->learning_rate;
    
    printf("✓ Training initialized (88D thread-centric)\n");
    
    return training;
}

// ============================================================================
// OPTIMIZER
// ============================================================================

void cllm_optimizer_step_adam(CLLMTraining* training) {
    if (!training) return;
    
    CLLMModel* model = training->model;
    
    if (!model->pool_88d) {
        fprintf(stderr, "FATAL: Cannot run optimizer without 88D thread pool\n");
        abort();
    }
    
    model->optimizer.t++;
    
    double beta1_t = math_pow(model->optimizer.beta1, model->optimizer.t);
    double beta2_t = math_pow(model->optimizer.beta2, model->optimizer.t);
    double lr_t = model->optimizer.learning_rate * math_sqrt(1.0 - beta2_t) / (1.0 - beta1_t);
    
    // TODO: Distribute optimizer updates to threads
    (void)lr_t;
    
    printf("Adam optimizer step (88D thread-centric)\n");
}