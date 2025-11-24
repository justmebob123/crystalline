/**
 * Multi-Threaded Training with Gradient Accumulation
 * 
 * Enables true parallel batch processing by:
 * 1. Allocating per-thread gradient buffers
 * 2. Processing batches in parallel
 * 3. Accumulating gradients across threads
 * 4. Single optimizer step with accumulated gradients
 */

#include "cllm_training.h"
#include "cllm_crystalline_training.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Thread-local gradient storage
typedef struct {
    // Embedding gradients
    float* embedding_grads;
    
    // Per-layer gradients
    struct {
        float* query_lattice;
        float* key_lattice;
        float* value_lattice;
    }* attention_grads;
    
    struct {
        float* w1_lattice;
        float* w2_lattice;
        float* bias1;
        float* bias2;
    }* ff_grads;
    
    struct {
        float* gamma;
        float* beta;
    }* ln_grads;
    
    // Thread state
    int thread_id;
    float thread_loss;
    int batches_processed;
} ThreadGradients;

// Multi-threaded training state
typedef struct {
    CLLMTraining* training;
    ThreadGradients* thread_grads;
    int num_threads;
    
    // Synchronization
    pthread_mutex_t mutex;
    pthread_barrier_t barrier;
    
    // Batch queue
    int next_batch_idx;
    int total_batches;
    int batches_per_thread;
    
    // Accumulated results
    float total_loss;
    int total_batches_processed;
} MTTrainingState;

/**
 * Allocate thread-local gradient buffers
 */
static ThreadGradients* allocate_thread_gradients(CLLMModel* model) {
    ThreadGradients* tg = (ThreadGradients*)calloc(1, sizeof(ThreadGradients));
    if (!tg) return NULL;
    
    uint32_t vocab_size = model->vocab_size;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t num_layers = model->num_layers;
    
    // Allocate embedding gradients
    tg->embedding_grads = (float*)calloc(vocab_size * embed_dim, sizeof(float));
    
    // Allocate attention gradients
    tg->attention_grads = (typeof(tg->attention_grads))calloc(num_layers, sizeof(*tg->attention_grads));
    for (uint32_t i = 0; i < num_layers; i++) {
        uint64_t attn_size = embed_dim * embed_dim;
        tg->attention_grads[i].query_lattice = (float*)calloc(attn_size, sizeof(float));
        tg->attention_grads[i].key_lattice = (float*)calloc(attn_size, sizeof(float));
        tg->attention_grads[i].value_lattice = (float*)calloc(attn_size, sizeof(float));
    }
    
    // Allocate feedforward gradients
    tg->ff_grads = (typeof(tg->ff_grads))calloc(num_layers, sizeof(*tg->ff_grads));
    for (uint32_t i = 0; i < num_layers; i++) {
        uint64_t ff_size = embed_dim * embed_dim;
        tg->ff_grads[i].w1_lattice = (float*)calloc(ff_size, sizeof(float));
        tg->ff_grads[i].w2_lattice = (float*)calloc(ff_size, sizeof(float));
        tg->ff_grads[i].bias1 = (float*)calloc(embed_dim, sizeof(float));
        tg->ff_grads[i].bias2 = (float*)calloc(embed_dim, sizeof(float));
    }
    
    // Allocate layer norm gradients
    tg->ln_grads = (typeof(tg->ln_grads))calloc(num_layers, sizeof(*tg->ln_grads));
    for (uint32_t i = 0; i < num_layers; i++) {
        tg->ln_grads[i].gamma = (float*)calloc(embed_dim, sizeof(float));
        tg->ln_grads[i].beta = (float*)calloc(embed_dim, sizeof(float));
    }
    
    return tg;
}

/**
 * Free thread-local gradient buffers
 */
static void free_thread_gradients(ThreadGradients* tg, uint32_t num_layers) {
    if (!tg) return;
    
    free(tg->embedding_grads);
    
    if (tg->attention_grads) {
        for (uint32_t i = 0; i < num_layers; i++) {
            free(tg->attention_grads[i].query_lattice);
            free(tg->attention_grads[i].key_lattice);
            free(tg->attention_grads[i].value_lattice);
        }
        free(tg->attention_grads);
    }
    
    if (tg->ff_grads) {
        for (uint32_t i = 0; i < num_layers; i++) {
            free(tg->ff_grads[i].w1_lattice);
            free(tg->ff_grads[i].w2_lattice);
            free(tg->ff_grads[i].bias1);
            free(tg->ff_grads[i].bias2);
        }
        free(tg->ff_grads);
    }
    
    if (tg->ln_grads) {
        for (uint32_t i = 0; i < num_layers; i++) {
            free(tg->ln_grads[i].gamma);
            free(tg->ln_grads[i].beta);
        }
        free(tg->ln_grads);
    }
    
    free(tg);
}

/**
 * Zero out thread-local gradients
 */
static void zero_thread_gradients(ThreadGradients* tg, CLLMModel* model) {
    uint32_t vocab_size = model->vocab_size;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t num_layers = model->num_layers;
    
    memset(tg->embedding_grads, 0, vocab_size * embed_dim * sizeof(float));
    
    for (uint32_t i = 0; i < num_layers; i++) {
        uint64_t attn_size = embed_dim * embed_dim;
        memset(tg->attention_grads[i].query_lattice, 0, attn_size * sizeof(float));
        memset(tg->attention_grads[i].key_lattice, 0, attn_size * sizeof(float));
        memset(tg->attention_grads[i].value_lattice, 0, attn_size * sizeof(float));
        
        uint64_t ff_size = embed_dim * embed_dim;
        memset(tg->ff_grads[i].w1_lattice, 0, ff_size * sizeof(float));
        memset(tg->ff_grads[i].w2_lattice, 0, ff_size * sizeof(float));
        memset(tg->ff_grads[i].bias1, 0, embed_dim * sizeof(float));
        memset(tg->ff_grads[i].bias2, 0, embed_dim * sizeof(float));
        
        memset(tg->ln_grads[i].gamma, 0, embed_dim * sizeof(float));
        memset(tg->ln_grads[i].beta, 0, embed_dim * sizeof(float));
    }
    
    tg->thread_loss = 0.0f;
    tg->batches_processed = 0;
}

/**
 * Process single batch (thread-local computation)
 */
static float process_batch_thread_local(
    CLLMTraining* training,
    ThreadGradients* tg,
    uint32_t* input_tokens,
    uint32_t* target_tokens
) {
    // Forward pass (uses shared model - read-only)
    float loss = cllm_forward_training(training, input_tokens);
    loss += cllm_compute_loss_training(training, target_tokens);
    
    // Backward pass (writes to thread-local gradients)
    // TODO: Modify backward pass to write to thread-local buffers
    cllm_backward_training(training, target_tokens);
    
    // Copy gradients to thread-local storage
    CLLMModel* model = training->model;
    uint32_t vocab_size = model->vocab_size;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t num_layers = model->num_layers;
    
    // Copy embedding gradients
    if (training->gradients) {
        for (size_t i = 0; i < vocab_size * embed_dim; i++) {
            tg->embedding_grads[i] += training->gradients[i];
        }
    }
    
    // Copy layer gradients
    for (uint32_t layer = 0; layer < num_layers; layer++) {
        uint64_t attn_size = embed_dim * embed_dim;
        
        if (training->attention_grads && training->attention_grads[layer].query_lattice) {
            for (uint64_t i = 0; i < attn_size; i++) {
                tg->attention_grads[layer].query_lattice[i] += training->attention_grads[layer].query_lattice[i];
                tg->attention_grads[layer].key_lattice[i] += training->attention_grads[layer].key_lattice[i];
                tg->attention_grads[layer].value_lattice[i] += training->attention_grads[layer].value_lattice[i];
            }
        }
        
        if (training->ff_grads) {
            uint64_t ff_size = embed_dim * embed_dim;
            for (uint64_t i = 0; i < ff_size; i++) {
                tg->ff_grads[layer].w1_lattice[i] += training->ff_grads[layer].w1_lattice[i];
                tg->ff_grads[layer].w2_lattice[i] += training->ff_grads[layer].w2_lattice[i];
            }
            for (uint32_t i = 0; i < embed_dim; i++) {
                tg->ff_grads[layer].bias1[i] += training->ff_grads[layer].bias1[i];
                tg->ff_grads[layer].bias2[i] += training->ff_grads[layer].bias2[i];
            }
        }
        
        if (training->ln_grads) {
            for (uint32_t i = 0; i < embed_dim; i++) {
                tg->ln_grads[layer].gamma[i] += training->ln_grads[layer].gamma[i];
                tg->ln_grads[layer].beta[i] += training->ln_grads[layer].beta[i];
            }
        }
    }
    
    return loss;
}

/**
 * Worker thread function
 */
static void* worker_thread(void* arg) {
    MTTrainingState* mt_state = (MTTrainingState*)arg;
    CLLMTraining* training = mt_state->training;
    ThreadGradients* tg = &mt_state->thread_grads[pthread_self() % mt_state->num_threads];
    
    // Allocate batch buffers
    int batch_size = training->config.batch_size;
    int seq_len = training->config.sequence_length;
    uint32_t* input_tokens = (uint32_t*)malloc(batch_size * seq_len * sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)malloc(batch_size * seq_len * sizeof(uint32_t));
    
    while (1) {
        // Get next batch index (thread-safe)
        pthread_mutex_lock(&mt_state->mutex);
        int batch_idx = mt_state->next_batch_idx++;
        pthread_mutex_unlock(&mt_state->mutex);
        
        // Check if done
        if (batch_idx >= mt_state->total_batches) {
            break;
        }
        
        // Get batch data
        int tokens = cllm_get_batch(training, input_tokens, target_tokens);
        if (tokens == 0) break;
        
        // Process batch (thread-local computation)
        float loss = process_batch_thread_local(training, tg, input_tokens, target_tokens);
        
        // Update thread-local stats
        tg->thread_loss += loss;
        tg->batches_processed++;
    }
    
    free(input_tokens);
    free(target_tokens);
    
    return NULL;
}

/**
 * Accumulate gradients from all threads
 */
static void accumulate_gradients(MTTrainingState* mt_state) {
    CLLMTraining* training = mt_state->training;
    CLLMModel* model = training->model;
    uint32_t vocab_size = model->vocab_size;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t num_layers = model->num_layers;
    
    // Zero out main gradient buffers
    memset(training->gradients, 0, vocab_size * embed_dim * sizeof(float));
    
    for (uint32_t layer = 0; layer < num_layers; layer++) {
        uint64_t attn_size = embed_dim * embed_dim;
        memset(training->attention_grads[layer].query_lattice, 0, attn_size * sizeof(float));
        memset(training->attention_grads[layer].key_lattice, 0, attn_size * sizeof(float));
        memset(training->attention_grads[layer].value_lattice, 0, attn_size * sizeof(float));
        
        uint64_t ff_size = embed_dim * embed_dim;
        memset(training->ff_grads[layer].w1_lattice, 0, ff_size * sizeof(float));
        memset(training->ff_grads[layer].w2_lattice, 0, ff_size * sizeof(float));
        memset(training->ff_grads[layer].bias1, 0, embed_dim * sizeof(float));
        memset(training->ff_grads[layer].bias2, 0, embed_dim * sizeof(float));
        
        memset(training->ln_grads[layer].gamma, 0, embed_dim * sizeof(float));
        memset(training->ln_grads[layer].beta, 0, embed_dim * sizeof(float));
    }
    
    // Accumulate from all threads
    for (int t = 0; t < mt_state->num_threads; t++) {
        ThreadGradients* tg = &mt_state->thread_grads[t];
        
        // Accumulate embedding gradients
        for (size_t i = 0; i < vocab_size * embed_dim; i++) {
            training->gradients[i] += tg->embedding_grads[i];
        }
        
        // Accumulate layer gradients
        for (uint32_t layer = 0; layer < num_layers; layer++) {
            uint64_t attn_size = embed_dim * embed_dim;
            for (uint64_t i = 0; i < attn_size; i++) {
                training->attention_grads[layer].query_lattice[i] += tg->attention_grads[layer].query_lattice[i];
                training->attention_grads[layer].key_lattice[i] += tg->attention_grads[layer].key_lattice[i];
                training->attention_grads[layer].value_lattice[i] += tg->attention_grads[layer].value_lattice[i];
            }
            
            uint64_t ff_size = embed_dim * embed_dim;
            for (uint64_t i = 0; i < ff_size; i++) {
                training->ff_grads[layer].w1_lattice[i] += tg->ff_grads[layer].w1_lattice[i];
                training->ff_grads[layer].w2_lattice[i] += tg->ff_grads[layer].w2_lattice[i];
            }
            for (uint32_t i = 0; i < embed_dim; i++) {
                training->ff_grads[layer].bias1[i] += tg->ff_grads[layer].bias1[i];
                training->ff_grads[layer].bias2[i] += tg->ff_grads[layer].bias2[i];
            }
            
            for (uint32_t i = 0; i < embed_dim; i++) {
                training->ln_grads[layer].gamma[i] += tg->ln_grads[layer].gamma[i];
                training->ln_grads[layer].beta[i] += tg->ln_grads[layer].beta[i];
            }
        }
        
        // Accumulate loss
        mt_state->total_loss += tg->thread_loss;
        mt_state->total_batches_processed += tg->batches_processed;
    }
    
    // Average gradients
    int total_batches = mt_state->total_batches_processed;
    if (total_batches > 0) {
        float scale = 1.0f / total_batches;
        
        for (size_t i = 0; i < vocab_size * embed_dim; i++) {
            training->gradients[i] *= scale;
        }
        
        for (uint32_t layer = 0; layer < num_layers; layer++) {
            uint64_t attn_size = embed_dim * embed_dim;
            for (uint64_t i = 0; i < attn_size; i++) {
                training->attention_grads[layer].query_lattice[i] *= scale;
                training->attention_grads[layer].key_lattice[i] *= scale;
                training->attention_grads[layer].value_lattice[i] *= scale;
            }
            
            uint64_t ff_size = embed_dim * embed_dim;
            for (uint64_t i = 0; i < ff_size; i++) {
                training->ff_grads[layer].w1_lattice[i] *= scale;
                training->ff_grads[layer].w2_lattice[i] *= scale;
            }
            for (uint32_t i = 0; i < embed_dim; i++) {
                training->ff_grads[layer].bias1[i] *= scale;
                training->ff_grads[layer].bias2[i] *= scale;
            }
            
            for (uint32_t i = 0; i < embed_dim; i++) {
                training->ln_grads[layer].gamma[i] *= scale;
                training->ln_grads[layer].beta[i] *= scale;
            }
        }
    }
}

/**
 * Multi-threaded training epoch
 */
float cllm_train_epoch_mt(CLLMTraining* training, int num_threads) {
    if (!training || num_threads <= 0) return 0.0f;
    
    printf("=== MULTI-THREADED CRYSTALLINE TRAINING ===\n");
    printf("Threads: %d\n", num_threads);
    printf("Training data: %zu tokens\n", training->num_tokens);
    
    // Initialize MT state
    MTTrainingState mt_state = {0};
    mt_state.training = training;
    mt_state.num_threads = num_threads;
    mt_state.next_batch_idx = 0;
    mt_state.total_batches = training->total_batches;
    mt_state.batches_per_thread = (training->total_batches + num_threads - 1) / num_threads;
    
    pthread_mutex_init(&mt_state.mutex, NULL);
    pthread_barrier_init(&mt_state.barrier, NULL, num_threads);
    
    // Allocate thread-local gradient buffers
    mt_state.thread_grads = (ThreadGradients*)calloc(num_threads, sizeof(ThreadGradients));
    for (int i = 0; i < num_threads; i++) {
        ThreadGradients* tg = allocate_thread_gradients(training->model);
        if (!tg) {
            fprintf(stderr, "ERROR: Failed to allocate thread gradients\n");
            return 0.0f;
        }
        mt_state.thread_grads[i] = *tg;
        mt_state.thread_grads[i].thread_id = i;
        zero_thread_gradients(&mt_state.thread_grads[i], training->model);
    }
    
    // Create worker threads
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &mt_state);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Accumulate gradients from all threads
    accumulate_gradients(&mt_state);
    
    // Single optimizer step with accumulated gradients
    cllm_optimizer_step(training);
    
    // Calculate average loss
    float avg_loss = mt_state.total_batches_processed > 0 
        ? mt_state.total_loss / mt_state.total_batches_processed 
        : 0.0f;
    
    printf("MT epoch complete: %d batches, avg loss = %.4f\n", 
           mt_state.total_batches_processed, avg_loss);
    
    // Cleanup
    for (int i = 0; i < num_threads; i++) {
        free_thread_gradients(&mt_state.thread_grads[i], training->model->num_layers);
    }
    free(mt_state.thread_grads);
    free(threads);
    pthread_mutex_destroy(&mt_state.mutex);
    pthread_barrier_destroy(&mt_state.barrier);
    
    return avg_loss;
}