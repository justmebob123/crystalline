/**
 * CLLM Integration Header
 * 
 * Function declarations for CLLM integration with the application
 */

#ifndef CLLM_INTEGRATION_H
#define CLLM_INTEGRATION_H

#include "app_common.h"
#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_training.h"
#include "../include/cllm_format.h"
#include "../include/prime_math.h"

// Model creation and initialization
CLLMModel* app_create_cllm_model_default(void);
CLLMModel* app_create_cllm_model_custom(uint32_t vocab_size,
                                         uint32_t embedding_dim,
                                         uint32_t num_layers,
                                         uint32_t num_heads,
                                         uint32_t ff_dim);

// Inference and training initialization
CLLMInference* app_init_inference(CLLMModel* model);
CLLMTraining* app_init_training(CLLMModel* model, const char* data_path);

// Text generation
int app_generate_text(AppState* state, const char* prompt, char* output, size_t output_size);

// Training
float app_train_epoch(AppState* state);

// Model I/O
int app_save_model(CLLMModel* model, const char* filepath);
CLLMModel* app_load_model(const char* filepath);

// Model management
void app_free_cllm_model(CLLMModel* model);
void app_get_model_info(CLLMModel* model, char* buffer, size_t buffer_size);

#endif /* CLLM_INTEGRATION_H */