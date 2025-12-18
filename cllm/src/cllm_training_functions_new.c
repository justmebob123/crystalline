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
// FORWARD PASS - 88D THREAD-CENTRIC IMPLEMENTATION
// ============================================================================

/**
 * Forward pass - THE ONLY implementation
 * 
 * Threading is MANDATORY. This function will abort if pool_88d is NULL.
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
    
    // ========================================================================
    // CRITICAL: Verify 88D thread pool exists
    // ========================================================================
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
        fprintf(stderr, "Aborting...\n");
        abort();  // Hard fail - no recovery possible
    }
    
    int num_tokens = training->config.batch_size * training->config.sequence_length;
    HierarchicalThreadPool* pool = model->pool_88d;
    
    // ========================================================================
    // STEP 1: DISTRIBUTE TOKENS TO THREADS
    // ========================================================================
    
    for (int i = 0; i < num_tokens; i++) {
        uint32_t token_id = input_tokens[i];
        
        // Validate token ID
        if (token_id >= model->vocab_size) {
            fprintf(stderr, "WARNING: Invalid token ID %u (vocab_size=%u)\n", 
                    token_id, model->vocab_size);
            continue;
        }
        
        // Get thread assignment (pre-computed during model creation)
        uint8_t layer = model->token_assignments[token_id].layer;
        uint8_t dimension = model->token_assignments[token_id].dimension;
        HierarchicalThread* thread = model->token_assignments[token_id].thread;
        
        // Verify thread exists
        if (!thread) {
            fprintf(stderr, "FATAL: Thread not found for token %u [%d][%d]\n",
                    token_id, layer, dimension);
            abort();
        }
        
        // For now, we'll use a simplified approach:
        // Mark that this thread has work to do
        // In Phase 4, we'll implement proper work queues
        
        // Increment work counter (atomic)
        __atomic_add_fetch(&thread->batch_count, 1, __ATOMIC_SEQ_CST);
    }
    
    // ========================================================================
    // STEP 2: SIGNAL ALL THREADS TO START
    // ========================================================================
    
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = 
                hierarchical_thread_get_88d(pool, layer, dim);
            
            if (thread) {
                pthread_cond_signal(&thread->control_cond);
            }
        }
    }
    
    // ========================================================================
    // STEP 3: WAIT FOR ALL THREADS TO COMPLETE
    // ========================================================================
    
    if (model->threading.forward_barrier) {
        pthread_barrier_wait(model->threading.forward_barrier);
    }
    
    // ========================================================================
    // STEP 4: COLLECT RESULTS FROM THREADS
    // ========================================================================
    
    double total_loss = 0.0;
    int thread_count = 0;
    
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = 
                hierarchical_thread_get_88d(pool, layer, dim);
            
            if (thread && thread->work_completed > 0) {
                // Get loss from thread's activation buffer
                if (thread->activation_buffer && thread->activation_buffer_size > 0) {
                    total_loss += thread->activation_buffer[0];
                }
                thread_count++;
            }
        }
    }
    
    // Return average loss
    return (thread_count > 0) ? (total_loss / thread_count) : 0.0;
}

// ============================================================================
// BACKWARD PASS - 88D THREAD-CENTRIC IMPLEMENTATION
// ============================================================================

/**
 * Backward pass - THE ONLY implementation
 * 
 * Threading is MANDATORY. Computes gradients in parallel across all threads.
 * Each thread computes gradients for its assigned tokens.
 * Gradients are accumulated in thread-local CrystallineAbacus storage.
 * 
 * @param training Training context
 * @param target_tokens Target token IDs for loss computation
 */
void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens) {
    if (!training || !target_tokens) {
        fprintf(stderr, "ERROR: NULL training or target_tokens\n");
        return;
    }
    
    CLLMModel* model = training->model;
    
    // ========================================================================
    // CRITICAL: Verify 88D thread pool exists
    // ========================================================================
    if (!model->pool_88d) {
        fprintf(stderr, "FATAL ERROR: 88D thread pool not initialized\n");
        fprintf(stderr, "Cannot perform backward pass without threading\n");
        abort();
    }
    
    int num_tokens = training->config.batch_size * training->config.sequence_length;
    HierarchicalThreadPool* pool = model->pool_88d;
    
    // ========================================================================
    // STEP 1: DISTRIBUTE GRADIENT COMPUTATION TO THREADS
    // ========================================================================
    
    for (int i = 0; i < num_tokens; i++) {
        // Get token from training context (stored during forward pass)
        // For now, we'll use a simplified approach
        uint32_t token_id = i % model->vocab_size;  // Simplified - will be fixed in Phase 4
        uint32_t target_id = target_tokens[i];
        
        if (token_id >= model->vocab_size || target_id >= model->vocab_size) {
            continue;
        }
        
        // Get thread for this token
        HierarchicalThread* thread = model->token_assignments[token_id].thread;
        
        if (!thread) continue;
        
        // Store target in thread for gradient computation
        // In Phase 4, we'll use proper work items
        if (thread->gradient_buffer && thread->gradient_buffer_size > 0) {
            double* grad_buf = (double*)thread->gradient_buffer;
            grad_buf[0] = (double)target_id;  // Simplified - store target
        }
        
        // Increment work counter
        __atomic_add_fetch(&thread->batch_count, 1, __ATOMIC_SEQ_CST);
    }
    
    // ========================================================================
    // STEP 2: SIGNAL ALL THREADS TO START
    // ========================================================================
    
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = 
                hierarchical_thread_get_88d(pool, layer, dim);
            
            if (thread) {
                pthread_cond_signal(&thread->control_cond);
            }
        }
    }
    
    // ========================================================================
    // STEP 3: WAIT FOR ALL THREADS TO COMPLETE
    // ========================================================================
    
    if (model->threading.backward_barrier) {
        pthread_barrier_wait(model->threading.backward_barrier);
    }
    
    // Gradients are now accumulated in thread-local CrystallineAbacus
    // No need to collect them here - optimizer will access them directly
}

// ============================================================================
// EMBEDDING PRECOMPUTATION
// ============================================================================

/**
 * Precompute all embeddings
 * 
 * For the 88D architecture, embeddings are already initialized during
 * model creation with clock lattice positions in thread-local storage.
 */
void cllm_precompute_all_embeddings(CLLMModel* model) {
    if (!model) return;
    
    printf("Pre-computing embeddings for %u tokens...\n", model->vocab_size);
    fflush(stdout);
    
    // Embeddings are already initialized in thread-local CrystallineAbacus
    // during model creation. This function is now a no-op.
    
    printf("✓ Embeddings already initialized in thread-local storage\n");
    printf("  Each token's embedding is stored in its assigned thread's CrystallineAbacus\n");
    fflush(stdout);
}

// ============================================================================
// TRAINING INITIALIZATION
// ============================================================================

/**
 * Initialize training state
 */
CLLMTraining* cllm_training_init(CLLMModel* model, CLLMTrainingConfig* config) {
    if (!model || !config) return NULL;
    
    // Verify threading is available
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
    
    // Store initial learning rate
    training->config.initial_learning_rate = config->learning_rate;
    
    // NO global buffers allocated - everything is in threads
    
    // Initialize optimizer buffers (Adam) - these will be in thread-local storage
    if (model->optimizer.type == OPTIMIZER_ADAM) {
        // Optimizer state is stored in thread->temp CrystallineAbacus
        // No global allocation needed
    }
    
    printf("✓ Training initialized (88D thread-centric)\n");
    printf("  All computation will happen in thread-local storage\n");
    
    return training;
}

// ============================================================================
// HELPER FUNCTIONS (kept for compatibility)
// ============================================================================

/**
 * Compute GCD of two numbers (for GCD-based similarity)
 */
static uint32_t gcd(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Compute GCD-based similarity between tokens
 */
static double gcd_similarity(uint32_t token1, uint32_t token2) {
    if (token1 == 0 || token2 == 0) return 0.0;
    
    uint32_t shared = gcd(token1 + 1, token2 + 1);  // +1 to avoid zero
    uint32_t max_val = (token1 > token2) ? token1 : token2;
    
    return (double)shared / (double)(max_val + 1);
}