/**
 * cllm_model_manager.c - Global Model Manager Implementation
 * 
 * Provides global model management with concurrent access control.
 */

#include "cllm_model_manager.h"
#include "cllm_utils.h"
#include "cllm_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Forward declarations for save/load (will implement later)
static bool cllm_save_model_internal(CLLMModel* model, const char* path);
static CLLMModel* cllm_load_model_internal(const char* path);

// Global model manager instance
static ModelManager g_model_manager = {0};
static bool g_manager_initialized = false;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static ManagedModel* find_model_by_name(const char* name) {
    for (uint32_t i = 0; i < g_model_manager.num_models; i++) {
        if (strcmp(g_model_manager.models[i]->name, name) == 0) {
            return g_model_manager.models[i];
        }
    }
    return NULL;
}

static bool ensure_models_directory(const char* dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, 0755) == -1) {
            fprintf(stderr, "Failed to create models directory: %s\n", strerror(errno));
            return false;
        }
    }
    return true;
}

// ============================================================================
// INITIALIZATION AND CLEANUP
// ============================================================================

bool model_manager_init(const char* models_dir) {
    if (g_manager_initialized) {
        return true;  // Already initialized
    }
    
    // Set models directory
    if (models_dir == NULL) {
        snprintf(g_model_manager.models_dir, MODEL_PATH_MAX, "./models");
    } else {
        snprintf(g_model_manager.models_dir, MODEL_PATH_MAX, "%s", models_dir);
    }
    
    // Create models directory if it doesn't exist
    if (!ensure_models_directory(g_model_manager.models_dir)) {
        return false;
    }
    
    // Initialize manager
    g_model_manager.models = NULL;
    g_model_manager.num_models = 0;
    g_model_manager.capacity = 0;
    
    // Initialize manager lock
    if (pthread_mutex_init(&g_model_manager.manager_lock, NULL) != 0) {
        fprintf(stderr, "Failed to initialize manager lock\n");
        return false;
    }
    
    g_manager_initialized = true;
    printf("Model manager initialized: %s\n", g_model_manager.models_dir);
    return true;
}

void model_manager_cleanup(void) {
    if (!g_manager_initialized) {
        return;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Free all models
    for (uint32_t i = 0; i < g_model_manager.num_models; i++) {
        ManagedModel* managed = g_model_manager.models[i];
        if (managed) {
            // Destroy lock
            pthread_rwlock_destroy(&managed->lock);
            
            // Free model if loaded
            if (managed->model) {
                cllm_free_model(managed->model);
            }
            
            free(managed);
        }
    }
    
    free(g_model_manager.models);
    g_model_manager.models = NULL;
    g_model_manager.num_models = 0;
    g_model_manager.capacity = 0;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    pthread_mutex_destroy(&g_model_manager.manager_lock);
    
    g_manager_initialized = false;
    printf("Model manager cleaned up\n");
}

// ============================================================================
// MODEL CREATION AND LOADING
// ============================================================================

ManagedModel* model_manager_create(const char* name, const CLLMConfig* config) {
    if (!g_manager_initialized || !name || !config) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Check if model already exists
    if (find_model_by_name(name) != NULL) {
        fprintf(stderr, "Model '%s' already exists\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Create managed model
    ManagedModel* managed = (ManagedModel*)calloc(1, sizeof(ManagedModel));
    if (!managed) {
        fprintf(stderr, "Failed to allocate managed model\n");
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Set name and path
    snprintf(managed->name, MODEL_NAME_MAX, "%s", name);
    int path_len = snprintf(managed->path, MODEL_PATH_MAX, "%s/%s.cllm", 
                           g_model_manager.models_dir, name);
    if (path_len >= MODEL_PATH_MAX) {
        fprintf(stderr, "Model path too long\n");
        free(managed);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Create the model
    managed->model = cllm_create_model(config);
    if (!managed->model) {
        fprintf(stderr, "Failed to create model\n");
        free(managed);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Initialize lock
    if (pthread_rwlock_init(&managed->lock, NULL) != 0) {
        fprintf(stderr, "Failed to initialize model lock\n");
        cllm_free_model(managed->model);
        free(managed);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Set metadata
    managed->is_loaded = true;
    managed->is_training = false;
    managed->read_count = 0;
    managed->vocab_size = config->vocab_size;
    managed->embedding_dim = config->embedding_dim;
    managed->num_layers = config->num_layers;
    managed->num_heads = config->num_heads;
    managed->created_time = (uint64_t)time(NULL);
    managed->modified_time = managed->created_time;
    
    // Add to manager
    if (g_model_manager.num_models >= g_model_manager.capacity) {
        uint32_t new_capacity = g_model_manager.capacity == 0 ? 4 : g_model_manager.capacity * 2;
        ManagedModel** new_models = (ManagedModel**)realloc(
            g_model_manager.models, 
            new_capacity * sizeof(ManagedModel*)
        );
        if (!new_models) {
            fprintf(stderr, "Failed to expand models array\n");
            pthread_rwlock_destroy(&managed->lock);
            cllm_free_model(managed->model);
            free(managed);
            pthread_mutex_unlock(&g_model_manager.manager_lock);
            return NULL;
        }
        g_model_manager.models = new_models;
        g_model_manager.capacity = new_capacity;
    }
    
    g_model_manager.models[g_model_manager.num_models++] = managed;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("Created model '%s' (%u vocab, %u dim, %u layers)\n", 
           name, config->vocab_size, config->embedding_dim, config->num_layers);
    
    return managed;
}

ManagedModel* model_manager_load(const char* name, const char* path) {
    if (!g_manager_initialized || !name || !path) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Check if model already exists
    if (find_model_by_name(name) != NULL) {
        fprintf(stderr, "Model '%s' already exists\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Load the model
    CLLMModel* model = cllm_load_model_internal(path);
    if (!model) {
        fprintf(stderr, "Failed to load model from '%s'\n", path);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Create managed model
    ManagedModel* managed = (ManagedModel*)calloc(1, sizeof(ManagedModel));
    if (!managed) {
        fprintf(stderr, "Failed to allocate managed model\n");
        cllm_free_model(model);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Set name and path
    snprintf(managed->name, MODEL_NAME_MAX, "%s", name);
    snprintf(managed->path, MODEL_PATH_MAX, "%s", path);
    
    managed->model = model;
    
    // Initialize lock
    if (pthread_rwlock_init(&managed->lock, NULL) != 0) {
        fprintf(stderr, "Failed to initialize model lock\n");
        cllm_free_model(model);
        free(managed);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    // Set metadata from model
    managed->is_loaded = true;
    managed->is_training = false;
    managed->read_count = 0;
    managed->vocab_size = (uint32_t)model->vocab_size;
    managed->embedding_dim = (uint32_t)model->embedding_dim;
    managed->num_layers = model->num_layers;
    managed->num_heads = model->header.num_heads;
    managed->created_time = (uint64_t)time(NULL);
    managed->modified_time = managed->created_time;
    
    // Add to manager
    if (g_model_manager.num_models >= g_model_manager.capacity) {
        uint32_t new_capacity = g_model_manager.capacity == 0 ? 4 : g_model_manager.capacity * 2;
        ManagedModel** new_models = (ManagedModel**)realloc(
            g_model_manager.models, 
            new_capacity * sizeof(ManagedModel*)
        );
        if (!new_models) {
            fprintf(stderr, "Failed to expand models array\n");
            pthread_rwlock_destroy(&managed->lock);
            cllm_free_model(model);
            free(managed);
            pthread_mutex_unlock(&g_model_manager.manager_lock);
            return NULL;
        }
        g_model_manager.models = new_models;
        g_model_manager.capacity = new_capacity;
    }
    
    g_model_manager.models[g_model_manager.num_models++] = managed;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("Loaded model '%s' from '%s'\n", name, path);
    
    return managed;
}

bool model_manager_save(const char* name) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    if (!managed->is_loaded || !managed->model) {
        fprintf(stderr, "Model '%s' is not loaded\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    // Save the model
    bool success = cllm_save_model_internal(managed->model, managed->path);
    if (success) {
        managed->modified_time = (uint64_t)time(NULL);
        printf("Saved model '%s' to '%s'\n", name, managed->path);
    } else {
        fprintf(stderr, "Failed to save model '%s'\n", name);
    }
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return success;
}

// Continued in next message due to length...

bool model_manager_unload(const char* name) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    if (!managed->is_loaded) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return true;  // Already unloaded
    }
    
    // Free the model
    if (managed->model) {
        cllm_free_model(managed->model);
        managed->model = NULL;
    }
    
    managed->is_loaded = false;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("Unloaded model '%s'\n", name);
    return true;
}

bool model_manager_reload(const char* name) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    if (managed->is_loaded) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return true;  // Already loaded
    }
    
    // Load the model
    managed->model = cllm_load_model_internal(managed->path);
    if (!managed->model) {
        fprintf(stderr, "Failed to reload model '%s'\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    managed->is_loaded = true;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("Reloaded model '%s'\n", name);
    return true;
}

bool model_manager_delete(const char* name, bool delete_file) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    // Find model index
    int index = -1;
    for (uint32_t i = 0; i < g_model_manager.num_models; i++) {
        if (strcmp(g_model_manager.models[i]->name, name) == 0) {
            index = (int)i;
            break;
        }
    }
    
    if (index == -1) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    ManagedModel* managed = g_model_manager.models[index];
    
    // Delete file if requested
    if (delete_file) {
        if (remove(managed->path) != 0) {
            fprintf(stderr, "Failed to delete model file '%s'\n", managed->path);
        }
    }
    
    // Free model
    if (managed->model) {
        cllm_free_model(managed->model);
    }
    
    // Destroy lock
    pthread_rwlock_destroy(&managed->lock);
    
    // Free managed model
    free(managed);
    
    // Remove from array
    for (uint32_t i = (uint32_t)index; i < g_model_manager.num_models - 1; i++) {
        g_model_manager.models[i] = g_model_manager.models[i + 1];
    }
    g_model_manager.num_models--;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    printf("Deleted model '%s'\n", name);
    return true;
}

// ============================================================================
// MODEL ACCESS CONTROL
// ============================================================================

CLLMModel* model_manager_acquire_read(const char* name) {
    if (!g_manager_initialized || !name) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    if (!managed->is_loaded || !managed->model) {
        fprintf(stderr, "Model '%s' is not loaded\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    // Acquire read lock
    pthread_rwlock_rdlock(&managed->lock);
    
    // Increment read count
    pthread_mutex_lock(&g_model_manager.manager_lock);
    managed->read_count++;
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return managed->model;
}

CLLMModel* model_manager_acquire_write(const char* name) {
    if (!g_manager_initialized || !name) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        fprintf(stderr, "Model '%s' not found\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    if (!managed->is_loaded || !managed->model) {
        fprintf(stderr, "Model '%s' is not loaded\n", name);
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return NULL;
    }
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    // Acquire write lock
    pthread_rwlock_wrlock(&managed->lock);
    
    // Set training flag
    pthread_mutex_lock(&g_model_manager.manager_lock);
    managed->is_training = true;
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return managed->model;
}

void model_manager_release_read(const char* name) {
    if (!g_manager_initialized || !name) {
        return;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return;
    }
    
    // Decrement read count
    if (managed->read_count > 0) {
        managed->read_count--;
    }
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    // Release read lock
    pthread_rwlock_unlock(&managed->lock);
}

void model_manager_release_write(const char* name) {
    if (!g_manager_initialized || !name) {
        return;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return;
    }
    
    // Clear training flag
    managed->is_training = false;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    // Release write lock
    pthread_rwlock_unlock(&managed->lock);
}

// ============================================================================
// MODEL QUERY AND DISCOVERY
// ============================================================================

ManagedModel* model_manager_get(const char* name) {
    if (!g_manager_initialized || !name) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    ManagedModel* managed = find_model_by_name(name);
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return managed;
}

ManagedModel** model_manager_list(uint32_t* count) {
    if (!g_manager_initialized || !count) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    *count = g_model_manager.num_models;
    ManagedModel** models = g_model_manager.models;
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return models;
}

bool model_manager_exists(const char* name) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    bool exists = (find_model_by_name(name) != NULL);
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return exists;
}

bool model_manager_get_status(const char* name, bool* is_loaded, 
                              bool* is_training, uint32_t* read_count) {
    if (!g_manager_initialized || !name) {
        return false;
    }
    
    pthread_mutex_lock(&g_model_manager.manager_lock);
    
    ManagedModel* managed = find_model_by_name(name);
    if (!managed) {
        pthread_mutex_unlock(&g_model_manager.manager_lock);
        return false;
    }
    
    if (is_loaded) *is_loaded = managed->is_loaded;
    if (is_training) *is_training = managed->is_training;
    if (read_count) *read_count = managed->read_count;
    
    pthread_mutex_unlock(&g_model_manager.manager_lock);
    
    return true;
}

// ============================================================================
// INTERNAL SAVE/LOAD FUNCTIONS (Stubs for now)
// ============================================================================

static bool cllm_save_model_internal(CLLMModel* model, const char* path) {
    if (!model || !path) {
        return false;
    }
    
    // TODO: Implement proper model serialization
    // For now, use cllm_format functions if available
    FILE* f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "Failed to open file for writing: %s\n", path);
        return false;
    }
    
    // Write header
    fwrite(&model->header, sizeof(CLLMHeader), 1, f);
    
    // Write embeddings
    size_t embedding_size = model->vocab_size * model->embedding_dim;
    fwrite(model->embeddings.embeddings, sizeof(float), embedding_size, f);
    
    fclose(f);
    return true;
}

static CLLMModel* cllm_load_model_internal(const char* path) {
    if (!path) {
        return NULL;
    }
    
    // TODO: Implement proper model deserialization
    // For now, just create a new model with default config
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open file for reading: %s\n", path);
        return NULL;
    }
    
    // Read header
    CLLMHeader header;
    if (fread(&header, sizeof(CLLMHeader), 1, f) != 1) {
        fprintf(stderr, "Failed to read model header\n");
        fclose(f);
        return NULL;
    }
    
    // Create config from header
    CLLMConfig config = {
        .vocab_size = (uint32_t)header.vocab_size,
        .embedding_dim = (uint32_t)header.embedding_dim,
        .num_layers = (uint32_t)header.num_layers,
        .num_heads = header.num_heads,
        .ff_dim = (uint32_t)header.embedding_dim * 4,  // Standard 4x
        .max_seq_len = header.context_length,
        .dropout = 0.1f
    };
    
    // Create model
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        fclose(f);
        return NULL;
    }
    
    // Read embeddings
    size_t embedding_size = header.vocab_size * header.embedding_dim;
    if (fread(model->embeddings.embeddings, sizeof(float), embedding_size, f) != embedding_size) {
        fprintf(stderr, "Failed to read embeddings\n");
        cllm_free_model(model);
        fclose(f);
        return NULL;
    }
    
    fclose(f);
    return model;
}
