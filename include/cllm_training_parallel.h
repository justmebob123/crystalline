#ifndef CLLM_TRAINING_PARALLEL_H
#define CLLM_TRAINING_PARALLEL_H

#include "cllm_training.h"

/**
 * Get optimal thread count (CPU count - 1, minimum 1)
 */
int cllm_get_optimal_thread_count(void);

/**
 * Initialize thread pool for parallel training
 * If thread_count <= 0, uses optimal count
 * Returns actual thread count or -1 on error
 */
int cllm_init_thread_pool(int thread_count);

/**
 * Cleanup thread pool
 */
void cllm_cleanup_thread_pool(void);

/**
 * Train epoch with parallel batch processing
 * Uses thread pool for efficient CPU utilization
 */
float cllm_train_epoch_parallel(CLLMTraining* training);

/**
 * Get current thread count
 */
int cllm_get_thread_count(void);

/**
 * Set thread count (reinitializes pool)
 * If count <= 0, uses optimal count
 * Returns actual thread count or -1 on error
 */
int cllm_set_thread_count(int count);

#endif // CLLM_TRAINING_PARALLEL_H