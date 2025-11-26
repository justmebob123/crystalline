/**
 * CLLM Multi-Threaded Training Header
 * 
 * Multi-threaded training with kissing spheres architecture
 */

#ifndef CLLM_TRAINING_THREADED_H
#define CLLM_TRAINING_THREADED_H

#include "cllm_training.h"
#include "cllm_batch.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Threaded Training System (opaque)
 */
typedef struct ThreadedTrainingSystem ThreadedTrainingSystem;

/**
 * Create threaded training system
 * 
 * @param training Training context
 * @param batch_iterator Batch iterator for data
 * @return Threaded training system or NULL on failure
 */
ThreadedTrainingSystem* threaded_training_create(CLLMTraining* training, 
                                                  CLLMBatchIterator* batch_iterator);

/**
 * Free threaded training system
 */
void threaded_training_free(ThreadedTrainingSystem* system);

/**
 * Train one epoch with multi-threading
 * 
 * Uses 12 kissing spheres to process batches in parallel
 * 
 * @param system Threaded training system
 * @return Average epoch loss
 */
float threaded_train_epoch(ThreadedTrainingSystem* system);

/**
 * Print threading statistics
 */
void threaded_training_print_stats(ThreadedTrainingSystem* system);

/**
 * Get per-sphere statistics
 * 
 * @param system Threaded training system
 * @param sphere_id Sphere ID (0-11)
 * @param batches_processed Output: number of batches processed by this sphere
 * @param avg_loss Output: average loss for this sphere
 * @return 0 on success, -1 on failure
 */
int threaded_training_get_sphere_stats(ThreadedTrainingSystem* system,
                                       int sphere_id,
                                       int* batches_processed,
                                       float* avg_loss);

/**
 * Get total gradient norm
 * 
 * @param system Threaded training system
 * @return Total gradient norm
 */
float threaded_training_get_gradient_norm(ThreadedTrainingSystem* system);

#ifdef __cplusplus
}
#endif

#endif // CLLM_TRAINING_THREADED_H