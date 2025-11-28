#ifndef CLLM_TRAINING_VALIDATION_H
#define CLLM_TRAINING_VALIDATION_H

#include <stddef.h>

/**
 * Calculate number of batches for given parameters
 */
size_t calculate_num_batches(size_t num_tokens, int batch_size, int seq_len);

/**
 * Calculate optimal thread count for dataset size
 * 
 * Returns the maximum number of Level-2 worker threads that can be
 * effectively utilized given the dataset size.
 * 
 * Returns -1 if dataset is too small even for minimal threading
 */
int calculate_optimal_thread_count(size_t num_tokens, int batch_size, int seq_len);

/**
 * Adjust parameters to fit dataset size
 * 
 * Tries to adjust batch_size and seq_len to create enough batches
 * for at least minimal threading (13 threads).
 * 
 * Returns 1 if successful, 0 if dataset is too small even with adjustments
 */
int adjust_parameters_for_dataset(size_t num_tokens, 
                                  int* batch_size, 
                                  int* seq_len,
                                  int* num_threads);

/**
 * Validate training parameters against dataset size
 * 
 * Returns 1 if valid, 0 if invalid
 * Prints detailed error messages if invalid
 */
int validate_training_parameters(size_t num_tokens,
                                 int batch_size,
                                 int seq_len,
                                 int num_threads);

/**
 * Print dataset size recommendations
 */
void print_dataset_requirements(int batch_size, int seq_len);

/**
 * Auto-adjust training parameters for dataset
 * 
 * Main entry point for parameter validation and adjustment.
 * 
 * Returns 1 if parameters are valid or successfully adjusted
 * Returns 0 if dataset is too small even with adjustments
 */
int auto_adjust_training_parameters(size_t num_tokens,
                                    int* batch_size,
                                    int* seq_len,
                                    int* num_threads);

#endif // CLLM_TRAINING_VALIDATION_H