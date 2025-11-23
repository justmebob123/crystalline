/**
 * CLLM Integration Module
 * 
 * Integrates the CLLM library with the application.
 * Provides high-level functions for:
 * - Model creation and initialization
 * - Inference pipeline
 * - Training integration
 * - Model I/O
 */

#include "app_common.h"
#include "../include/cllm.h"
#include "../include/cllm_utils.h"
#include "../include/cllm_format.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_training.h"
#include "../include/cllm_training_parallel.h"
#include "../include/cllm_crystalline_training.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// External declarations for initialization functions
// External declarations for CLLM library functions
extern void cllm_init_model(CLLMModel* model);
// Use cllm_lattice_aware_init from cllm_lattice_init.c
extern void cllm_lattice_aware_init(CLLMModel* model, float scale);
extern CLLMConfig* cllm_create_config(uint32_t vocab_size, uint32_t embedding_dim, 
                                      uint32_t num_layers, uint32_t num_heads, uint32_t ff_dim);
extern void cllm_free_config(CLLMConfig* config);
extern int cllm_validate_config(CLLMConfig* config);
extern void cllm_print_config(CLLMConfig* config);
extern uint64_t cllm_get_parameter_count(CLLMModel* model);
extern uint64_t cllm_get_memory_usage(CLLMModel* model);
extern void cllm_print_model_stats(CLLMModel* model);
extern int cllm_validate_model(void* model);
extern int cllm_check_model_health(CLLMModel* model);

// Note: cllm_training_free is actually cllm_training_cleanup
#define cllm_training_free cllm_training_cleanup

/**
 * Create a new CLLM model with default configuration
 */
CLLMModel* app_create_cllm_model_default(void) {
    printf("Creating default CLLM model...\n");
    
    // Create configuration
    CLLMConfig config = {
        .vocab_size = 10000,
        .embedding_dim = 512,
        .num_layers = 6,
        .num_heads = 8,
        .ff_dim = 2048,
        .max_seq_len = 512,
        .dropout = 0.1f
    };
    
    // Use the proper model creation function with random initialization
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return NULL;
    }
    
    printf("✓ Model created with random weight initialization\n");
    
    // Verify weights are non-zero
    if (model->embeddings.embeddings) {
        float sum = 0.0f;
        for (int i = 0; i < 100; i++) {
            sum += fabsf(model->embeddings.embeddings[i]);
        }
        printf("  Sample weight magnitude: %.6f (should be ~0.01-0.1)\n", sum / 100);
        
        if (sum < 0.0001f) {
            fprintf(stderr, "WARNING: Weights appear to be zero!\n");
        }
    } else {
        fprintf(stderr, "ERROR: Model embeddings are NULL!\n");
    }
    
    return model;
}

// OLD BROKEN VERSION - DO NOT USE
CLLMModel* app_create_cllm_model_default_OLD_BROKEN(void) {
    printf("Creating default CLLM model...\n");
    
    // Create configuration
    CLLMConfig* config = cllm_create_config(
        10000,  // vocab_size
        512,    // embedding_dim
        6,      // num_layers
        8,      // num_heads
        2048    // ff_dim
    );
    
    if (!config) {
        fprintf(stderr, "Failed to create configuration\n");
        return NULL;
    }
    
    // Validate configuration
    if (!cllm_validate_config(config)) {
        fprintf(stderr, "Invalid configuration\n");
        cllm_free_config(config);
        return NULL;
    }
    
    // Print configuration
    cllm_print_config(config);
    
    // Allocate model structure
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) {
        fprintf(stderr, "Failed to allocate model\n");
        cllm_free_config(config);
        return NULL;
    }
    
    // Set basic parameters
    model->vocab_size = config->vocab_size;
    model->embedding_dim = config->embedding_dim;
    model->num_layers = config->num_layers;
    
    // Initialize header (CRITICAL for inference)
    memcpy(model->header.magic, "CLLM", 4);
    model->header.version = 1;
    model->header.vocab_size = config->vocab_size;
    model->header.embedding_dim = config->embedding_dim;
    model->header.num_layers = config->num_layers;
    model->header.num_heads = config->num_heads;
    model->header.context_length = config->max_seq_len;
    
    // Allocate embeddings
    model->embeddings.vocab_size = config->vocab_size;
    model->embeddings.embedding_dim = config->embedding_dim;
    model->embeddings.embeddings = (float*)calloc(
        config->vocab_size * config->embedding_dim, sizeof(float));
    
    if (!model->embeddings.embeddings) {
        fprintf(stderr, "Failed to allocate embeddings\n");
        free(model);
        cllm_free_config(config);
        return NULL;
    }
    
    // Allocate lattice transformations
    model->embeddings.lattice_transform = (float*)calloc(
        config->embedding_dim * config->embedding_dim, sizeof(float));
    model->embeddings.inverse_transform = (float*)calloc(
        config->embedding_dim * config->embedding_dim, sizeof(float));
    
    // Allocate positional encoding
    model->pos_encoding.max_length = config->max_seq_len;
    model->pos_encoding.embedding_dim = config->embedding_dim;
    model->pos_encoding.spiral_positions = (float*)calloc(
        config->max_seq_len * config->embedding_dim, sizeof(float));
    model->pos_encoding.clock_positions = (float*)calloc(
        config->max_seq_len * config->embedding_dim, sizeof(float));
    model->pos_encoding.prime_positions = (float*)calloc(
        config->max_seq_len * config->embedding_dim, sizeof(float));
    model->pos_encoding.learned_positions = (float*)calloc(
        config->max_seq_len * config->embedding_dim, sizeof(float));
    
    // Allocate transformer layers
    model->attention_layers = (AttentionLayer*)calloc(config->num_layers, sizeof(AttentionLayer));
    model->ff_layers = (FeedForwardLayer*)calloc(config->num_layers, sizeof(FeedForwardLayer));
    model->layer_norms = (CLLMLayerNorm*)calloc(config->num_layers * 2, sizeof(CLLMLayerNorm));
    
    if (!model->attention_layers || !model->ff_layers || !model->layer_norms) {
        fprintf(stderr, "Failed to allocate layers\n");
        // TODO: Add proper cleanup
        cllm_free_config(config);
        return NULL;
    }
    
    // Initialize each layer
    for (uint32_t i = 0; i < config->num_layers; i++) {
        // Attention layer
        AttentionLayer* attn = &model->attention_layers[i];
        attn->layer_id = i;
        attn->num_heads = config->num_heads;
        attn->head_dim = config->embedding_dim / config->num_heads;
        
        uint32_t d_model = attn->num_heads * attn->head_dim;
        attn->query_lattice = (float*)calloc(d_model * d_model, sizeof(float));
        attn->key_lattice = (float*)calloc(d_model * d_model, sizeof(float));
        attn->value_lattice = (float*)calloc(d_model * d_model, sizeof(float));
        
        // Feed-forward layer
        FeedForwardLayer* ffn = &model->ff_layers[i];
        ffn->layer_id = i;
        ffn->input_dim = config->embedding_dim;
        ffn->hidden_dim = config->ff_dim;
        ffn->output_dim = config->embedding_dim;
        
        ffn->w1_lattice = (float*)calloc(config->embedding_dim * config->ff_dim, sizeof(float));
        ffn->w2_lattice = (float*)calloc(config->ff_dim * config->embedding_dim, sizeof(float));
        ffn->bias1 = (float*)calloc(config->ff_dim, sizeof(float));
        ffn->bias2 = (float*)calloc(config->embedding_dim, sizeof(float));
        
        // Layer norms
        CLLMLayerNorm* ln1 = &model->layer_norms[i * 2];
        CLLMLayerNorm* ln2 = &model->layer_norms[i * 2 + 1];
        
        ln1->layer_id = i * 2;
        ln1->dim = config->embedding_dim;
        ln1->epsilon = 1e-5f;
        ln1->gamma = (float*)calloc(config->embedding_dim, sizeof(float));
        ln1->beta = (float*)calloc(config->embedding_dim, sizeof(float));
        
        ln2->layer_id = i * 2 + 1;
        ln2->dim = config->embedding_dim;
        ln2->epsilon = 1e-5f;
        ln2->gamma = (float*)calloc(config->embedding_dim, sizeof(float));
        ln2->beta = (float*)calloc(config->embedding_dim, sizeof(float));
    }
    
    printf("Model structure allocated\n");
    
    // Initialize weights with lattice-aware initialization
    printf("Initializing weights with lattice structure...\n");
    cllm_lattice_aware_init(model, 1.0f);
    
    printf("Model created successfully!\n");
    cllm_print_model_stats(model);
    
    cllm_free_config(config);
    return model;
}

/**
 * Create CLLM model with custom configuration
 */
CLLMModel* app_create_cllm_model_custom(uint32_t vocab_size,
                                         uint32_t embedding_dim,
                                         uint32_t num_layers,
                                         uint32_t num_heads,
                                         uint32_t ff_dim) {
    printf("Creating custom CLLM model...\n");
    printf("  Vocab: %u, Dim: %u, Layers: %u, Heads: %u, FF: %u\n",
           vocab_size, embedding_dim, num_layers, num_heads, ff_dim);
    
    // Create configuration
    CLLMConfig* config = cllm_create_config(vocab_size, embedding_dim, 
                                            num_layers, num_heads, ff_dim);
    
    if (!config || !cllm_validate_config(config)) {
        fprintf(stderr, "Invalid configuration\n");
        if (config) cllm_free_config(config);
        return NULL;
    }
    
    // Use the default creation with this config
    // (For now, we'll use the default function and just validate the config)
    cllm_free_config(config);
    
    return app_create_cllm_model_default();
}

/**
 * Initialize inference context for model
 */
CLLMInference* app_init_inference(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "Cannot initialize inference: model is NULL\n");
        return NULL;
    }
    
    printf("Initializing inference context...\n");
    
    // Validate model first
    if (!cllm_validate_model(model)) {
        fprintf(stderr, "Model validation failed\n");
        return NULL;
    }
    
    // Check model health
    if (!cllm_check_model_health(model)) {
        fprintf(stderr, "Model health check failed (NaN/Inf detected)\n");
        return NULL;
    }
    
    // Initialize inference
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Failed to initialize inference\n");
        return NULL;
    }
    
    // Set default parameters
    cllm_set_temperature(inference, 0.8f);
    cllm_set_max_tokens(inference, 100);
    cllm_set_top_p(inference, 0.9f);
    
    printf("Inference context initialized successfully\n");
    return inference;
}

/**
 * Initialize training context for model
 */
CLLMTraining* app_init_training(CLLMModel* model, const char* data_path) {
    if (!model) {
        fprintf(stderr, "Cannot initialize training: model is NULL\n");
        return NULL;
    }
    
    printf("Initializing training context...\n");
    printf("  Data path: %s\n", data_path ? data_path : "none");
    
    // Initialize training with default config
    CLLMTrainingConfig config = {
        .learning_rate = 0.0001f,
        .batch_size = 32,
        .num_epochs = 100,
        .max_steps = 10000,
        .warmup_steps = 1000,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
        .save_every = 5,
        .save_interval = 5,
        .eval_interval = 100,
        .sequence_length = 512
    };
    strncpy(config.optimizer, "adam", sizeof(config.optimizer));
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "Failed to initialize training\n");
        return NULL;
    }
    
    // Load training data if path provided
    if (data_path && strlen(data_path) > 0) {
        printf("Loading training data from: %s\n", data_path);
        int result = cllm_load_training_data(training, data_path);
        if (result != 0) {
            fprintf(stderr, "Failed to load training data\n");
            cllm_training_free(training);
            return NULL;
        }
        printf("Training data loaded successfully\n");
    }
    
    // Training parameters are set in config above
    
    printf("Training context initialized successfully\n");
    return training;
}

/**
 * Generate text using the model
 */
int app_generate_text(AppState* state, const char* prompt, char* output, size_t output_size) {
    if (!state || !state->cllm_inference || !prompt || !output) {
        return -1;
    }
    
    printf("Generating text for prompt: %s\n", prompt);
    
    // Use the inference generate function
    int result = cllm_generate(state->cllm_inference, prompt, output, (int)output_size);
    
    if (result > 0) {
        printf("Generated %d tokens\n", result);
    } else {
        fprintf(stderr, "Generation failed\n");
    }
    
    return result;
}

/**
 * Train model for one epoch
 * 
 * Training modes:
 * 1. Crystalline (fastest) - Uses prime-based similarity and Ulam spiral
 * 2. Parallel (fast) - Uses thread pool with optimizations
 * 3. Standard (baseline) - Single-threaded standard training
 */
float app_train_epoch(AppState* state) {
    if (!state || !state->cllm_training) {
        return -1.0f;
    }
    
    float loss;
    
    // PRIORITY 1: Try crystalline training (20-400x speedup potential)
    // This uses prime-based similarity and Ulam spiral locality
    extern float cllm_train_epoch_crystalline(CLLMTraining* training);
    loss = cllm_train_epoch_crystalline(state->cllm_training);
    
    // Update UI state
    state->training_loss = loss;
    
    return loss;
}

/**
 * Save model to file
 */
int app_save_model(CLLMModel* model, const char* filepath) {
    if (!model || !filepath) {
        return -1;
    }
    
    printf("Saving model to: %s\n", filepath);
    
    // Validate model before saving
    if (!cllm_validate_model(model)) {
        fprintf(stderr, "Cannot save invalid model\n");
        return -1;
    }
    
    // Use CLLM write function (note: signature is (filename, model))
    int result = cllm_write(filepath, model);
    
    if (result == 0) {
        printf("Model saved successfully\n");
    } else {
        fprintf(stderr, "Failed to save model\n");
    }
    
    return result;
}

/**
 * Load model from file
 */
CLLMModel* app_load_model(const char* filepath) {
    if (!filepath) {
        return NULL;
    }
    
    printf("Loading model from: %s\n", filepath);
    
    // Use CLLM read function
    CLLMModel* model = cllm_read_model(filepath);
    
    if (model) {
        printf("Model loaded successfully\n");
        cllm_print_model_stats(model);
        
        // Validate loaded model
        if (!cllm_validate_model(model)) {
            fprintf(stderr, "Warning: Loaded model failed validation\n");
        }
        
        // Check health
        if (!cllm_check_model_health(model)) {
            fprintf(stderr, "Warning: Loaded model has NaN/Inf values\n");
        }
    } else {
        fprintf(stderr, "Failed to load model\n");
    }
    
    return model;
}

/**
 * Free CLLM model and associated resources
 */
void app_free_cllm_model(CLLMModel* model) {
    if (!model) return;
    
    printf("Freeing CLLM model...\n");
    
    // Free embeddings
    if (model->embeddings.embeddings) free(model->embeddings.embeddings);
    if (model->embeddings.lattice_transform) free(model->embeddings.lattice_transform);
    if (model->embeddings.inverse_transform) free(model->embeddings.inverse_transform);
    
    // Free positional encoding
    if (model->pos_encoding.spiral_positions) free(model->pos_encoding.spiral_positions);
    if (model->pos_encoding.clock_positions) free(model->pos_encoding.clock_positions);
    if (model->pos_encoding.prime_positions) free(model->pos_encoding.prime_positions);
    if (model->pos_encoding.learned_positions) free(model->pos_encoding.learned_positions);
    
    // Free layers
    if (model->attention_layers) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            AttentionLayer* attn = &model->attention_layers[i];
            if (attn->query_lattice) free(attn->query_lattice);
            if (attn->key_lattice) free(attn->key_lattice);
            if (attn->value_lattice) free(attn->value_lattice);
        }
        free(model->attention_layers);
    }
    
    if (model->ff_layers) {
        for (uint32_t i = 0; i < model->num_layers; i++) {
            FeedForwardLayer* ffn = &model->ff_layers[i];
            if (ffn->w1_lattice) free(ffn->w1_lattice);
            if (ffn->w2_lattice) free(ffn->w2_lattice);
            if (ffn->bias1) free(ffn->bias1);
            if (ffn->bias2) free(ffn->bias2);
        }
        free(model->ff_layers);
    }
    
    if (model->layer_norms) {
        for (uint32_t i = 0; i < model->num_layers * 2; i++) {
            CLLMLayerNorm* ln = &model->layer_norms[i];
            if (ln->gamma) free(ln->gamma);
            if (ln->beta) free(ln->beta);
        }
        free(model->layer_norms);
    }
    
    // Free lattice points if present
    if (model->lattice_points) free(model->lattice_points);
    if (model->tokens) free(model->tokens);
    if (model->weights) free(model->weights);
    
    free(model);
    printf("Model freed\n");
}

/**
 * Get model info string for UI display
 */
void app_get_model_info(CLLMModel* model, char* buffer, size_t buffer_size) {
    if (!model || !buffer) return;
    
    uint64_t params = cllm_get_parameter_count(model);
    uint64_t memory = cllm_get_memory_usage(model);
    
    snprintf(buffer, buffer_size,
             "Vocab: %u | Dim: %u | Layers: %u | Params: %.1fM | Memory: %.1fMB",
             model->embeddings.vocab_size,
             model->embeddings.embedding_dim,
             model->num_layers,
             params / 1e6,
             memory / (1024.0 * 1024.0));
}