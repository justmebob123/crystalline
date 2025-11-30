/**
 * Model Metadata Management
 * 
 * Functions for managing model names, epochs, and configuration
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cllm.h"
#include "cllm_model_manager.h"

/**
 * Get epochs trained for a model
 */
int cllm_model_get_epochs_trained(CLLMModel* model) {
    if (!model) return -1;
    return model->epochs_trained;
}

/**
 * Set target epochs for a model
 */
int cllm_model_set_target_epochs(CLLMModel* model, int target) {
    if (!model || target < 0) return -1;
    
    // Cannot set target less than already trained
    if (target < model->epochs_trained) {
        fprintf(stderr, "Error: Cannot set target_epochs (%d) less than epochs_trained (%d)\n",
                target, model->epochs_trained);
        return -1;
    }
    
    model->target_epochs = target;
    return 0;
}

/**
 * Validate epoch configuration
 */
int cllm_model_validate_epochs(CLLMModel* model, int target_epochs) {
    if (!model) return -1;
    
    if (target_epochs < model->epochs_trained) {
        fprintf(stderr, "Error: target_epochs (%d) cannot be less than epochs_trained (%d)\n",
                target_epochs, model->epochs_trained);
        return -1;
    }
    
    return 0;
}

/**
 * Get model name
 */
const char* cllm_model_get_name(CLLMModel* model) {
    if (!model) return NULL;
    return model->model_name;
}

/**
 * Set model name
 */
int cllm_model_set_name(CLLMModel* model, const char* name) {
    if (!model || !name) return -1;
    
    // Validate name length
    if (strlen(name) >= 256) {
        fprintf(stderr, "Error: Model name too long (max 255 characters)\n");
        return -1;
    }
    
    // Validate name characters (alphanumeric, underscore, hyphen only)
    for (const char* p = name; *p; p++) {
        if (!isalnum(*p) && *p != '_' && *p != '-') {
            fprintf(stderr, "Error: Invalid character '%c' in model name\n", *p);
            return -1;
        }
    }
    
    strncpy(model->model_name, name, 255);
    model->model_name[255] = '\0';
    
    // Update queue directory
    snprintf(model->queue_directory, 512, "models/%s_queue", name);
    
    return 0;
}

/**
 * Generate default model name
 */
char* cllm_model_generate_default_name(void) {
    static int counter = 1;
    char* name = malloc(64);
    if (!name) return NULL;
    
    snprintf(name, 64, "model_%d", counter++);
    return name;
}

/**
 * Validate model name is unique
 */
int cllm_model_validate_name_unique(const char* name) {
    if (!name) return -1;
    
    // Check if model file exists
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "models/%s.cllm", name);
    
    FILE* f = fopen(filepath, "r");
    if (f) {
        fclose(f);
        return -1; // Name already exists
    }
    
    return 0; // Name is unique
}

/**
 * Initialize model metadata with defaults
 */
int cllm_model_init_metadata(CLLMModel* model, const char* name) {
    if (!model) return -1;
    
    // Set name
    if (name) {
        if (cllm_model_set_name(model, name) != 0) {
            return -1;
        }
    } else {
        // Generate default name
        char* default_name = cllm_model_generate_default_name();
        if (!default_name) return -1;
        
        cllm_model_set_name(model, default_name);
        free(default_name);
    }
    
    // Initialize epochs
    model->epochs_trained = 0;
    model->target_epochs = 10; // Default target
    
    // Initialize training config with defaults
    model->training_config.learning_rate = 0.001f;
    model->training_config.batch_size = 4;
    model->training_config.sequence_length = 64;
    model->training_config.num_epochs = 10;
    model->training_config.weight_decay = 0.01f;
    model->training_config.gradient_clip = 1.0f;
    
    // Initialize history
    model->loss_history = NULL;
    model->history_size = 0;
    model->history_capacity = 0;
    
    return 0;
}

/**
 * Add loss to history
 */
int cllm_model_add_loss_to_history(CLLMModel* model, float loss) {
    if (!model) return -1;
    
    // Expand capacity if needed
    if (model->history_size >= model->history_capacity) {
        int new_capacity = model->history_capacity == 0 ? 100 : model->history_capacity * 2;
        float* new_history = realloc(model->loss_history, new_capacity * sizeof(float));
        if (!new_history) {
            fprintf(stderr, "Error: Failed to expand loss history\n");
            return -1;
        }
        model->loss_history = new_history;
        model->history_capacity = new_capacity;
    }
    
    // Add loss
    model->loss_history[model->history_size++] = loss;
    return 0;
}

/**
 * Get loss history
 */
const float* cllm_model_get_loss_history(CLLMModel* model, int* size) {
    if (!model || !size) return NULL;
    *size = model->history_size;
    return model->loss_history;
}

/**
 * Clear loss history
 */
void cllm_model_clear_loss_history(CLLMModel* model) {
    if (!model) return;
    
    if (model->loss_history) {
        free(model->loss_history);
        model->loss_history = NULL;
    }
    model->history_size = 0;
    model->history_capacity = 0;
}
