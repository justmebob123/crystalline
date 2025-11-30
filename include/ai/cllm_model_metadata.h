#ifndef CLLM_MODEL_METADATA_H
#define CLLM_MODEL_METADATA_H

#include "cllm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get epochs trained for a model
 */
int cllm_model_get_epochs_trained(CLLMModel* model);

/**
 * Set target epochs for a model
 * Returns -1 if target < epochs_trained
 */
int cllm_model_set_target_epochs(CLLMModel* model, int target);

/**
 * Validate epoch configuration
 * Returns -1 if target_epochs < epochs_trained
 */
int cllm_model_validate_epochs(CLLMModel* model, int target_epochs);

/**
 * Get model name
 */
const char* cllm_model_get_name(CLLMModel* model);

/**
 * Set model name
 * Validates name (alphanumeric, underscore, hyphen only)
 * Updates queue_directory automatically
 */
int cllm_model_set_name(CLLMModel* model, const char* name);

/**
 * Generate default model name (model_1, model_2, etc.)
 * Caller must free returned string
 */
char* cllm_model_generate_default_name(void);

/**
 * Validate model name is unique
 * Returns 0 if unique, -1 if already exists
 */
int cllm_model_validate_name_unique(const char* name);

/**
 * Initialize model metadata with defaults
 * If name is NULL, generates default name
 */
int cllm_model_init_metadata(CLLMModel* model, const char* name);

/**
 * Add loss to history
 */
int cllm_model_add_loss_to_history(CLLMModel* model, float loss);

/**
 * Get loss history
 * Returns pointer to history array and sets size
 */
const float* cllm_model_get_loss_history(CLLMModel* model, int* size);

/**
 * Clear loss history
 */
void cllm_model_clear_loss_history(CLLMModel* model);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_MODEL_METADATA_H */