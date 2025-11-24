#ifndef CLLM_TRAINING_MT_H
#define CLLM_TRAINING_MT_H

#include "cllm_training.h"

/**
 * Multi-threaded training with gradient accumulation
 * 
 * Enables parallel batch processing with proper gradient accumulation.
 * Each thread processes batches independently, accumulates gradients,
 * then a single optimizer step updates weights.
 * 
 * @param training Training state
 * @param num_threads Number of worker threads (0 = auto-detect CPU count)
 * @return Average loss for the epoch
 */
float cllm_train_epoch_mt(CLLMTraining* training, int num_threads);

#endif // CLLM_TRAINING_MT_H