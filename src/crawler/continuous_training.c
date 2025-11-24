/**
 * Continuous Training System
 * 
 * Monitors training queue and trains on new files as they arrive
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>
#include "cllm_training.h"
#include "cllm.h"

#define MAX_TOKENS_PER_FILE 100000

/**
 * Get current timestamp string
 */
static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "[%H:%M:%S]", tm_info);
}

typedef struct {
    char data_dir[1024];
    char model_path[1024];
    CLLMModel* model;
    CLLMTraining* training;
    int running;
    int files_trained;
    int num_threads;
    pthread_mutex_t lock;
} ContinuousTrainingState;

/**
 * Check if file is locked by another thread
 */
static int is_file_locked(const char* filepath) {
    char lockpath[2048];
    snprintf(lockpath, sizeof(lockpath), "%s.lock", filepath);
    return access(lockpath, F_OK) == 0;
}

/**
 * Create lock file
 */
static int create_lock(const char* filepath) {
    char lockpath[2048];
    snprintf(lockpath, sizeof(lockpath), "%s.lock", filepath);
    
    FILE* f = fopen(lockpath, "w");
    if (!f) return -1;
    
    fprintf(f, "%d\n", getpid());
    fclose(f);
    return 0;
}

/**
 * Remove lock file
 */
static void remove_lock(const char* filepath) {
    char lockpath[2048];
    snprintf(lockpath, sizeof(lockpath), "%s.lock", filepath);
    unlink(lockpath);
}

/**
 * Load tokens from file
 */
static int load_tokens_from_file(const char* filepath, uint32_t** tokens, size_t* num_tokens) {
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;
    
    // Skip header lines
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] != '#') break;
    }
    
    // Count tokens first
    long pos = ftell(f);
    int count = 0;
    char token[256];
    while (fscanf(f, "%255s", token) == 1) {
        count++;
    }
    
    if (count == 0) {
        fclose(f);
        return -1;
    }
    
    // Allocate and read tokens
    *tokens = (uint32_t*)malloc(count * sizeof(uint32_t));
    if (!*tokens) {
        fclose(f);
        return -1;
    }
    
    fseek(f, pos, SEEK_SET);
    int i = 0;
    while (fscanf(f, "%255s", token) == 1 && i < count) {
        // Simple hash-based token ID (replace with proper vocab lookup)
        unsigned long hash = 5381;
        for (char* p = token; *p; p++) {
            hash = ((hash << 5) + hash) + *p;
        }
        (*tokens)[i++] = (uint32_t)(hash % 10000);  // Mod by vocab size
    }
    
    *num_tokens = i;
    fclose(f);
    return 0;
}

/**
 * Train on one file
 */
static int train_on_file(ContinuousTrainingState* state, const char* filepath) {
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    printf("\n%s === Training on file ===\n", timestamp);
    printf("%s File: %s\n", timestamp, filepath);
    
    // Load tokens
    uint32_t* tokens = NULL;
    size_t num_tokens = 0;
    
    if (load_tokens_from_file(filepath, &tokens, &num_tokens) != 0) {
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(stderr, "%s Failed to load tokens from: %s\n", timestamp, filepath);
        return -1;
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s Loaded %zu tokens\n", timestamp, num_tokens);
    
    // Update training data
    if (state->training->tokens) {
        free(state->training->tokens);
    }
    state->training->tokens = tokens;
    state->training->num_tokens = num_tokens;
    
    // Train for N epochs
    int epochs = 5;
    float total_loss = 0.0f;
    
    for (int epoch = 0; epoch < epochs; epoch++) {
        extern float cllm_train_epoch_crystalline(CLLMTraining* training);
        float loss = cllm_train_epoch_crystalline(state->training);
        total_loss += loss;
        printf("  Epoch %d/%d: loss = %.4f\n", epoch + 1, epochs, loss);
    }
    
    float avg_loss = total_loss / epochs;
    printf("✓ Training complete: avg loss = %.4f\n", avg_loss);
    
    // Save model
    extern int cllm_write_model(CLLMModel* model, const char* filepath);
    if (cllm_write_model(state->model, state->model_path) == 0) {
        printf("✓ Model saved: %s\n", state->model_path);
    }
    
    return 0;
}

/**
 * Move file to trained directory
 */
static int move_to_trained(const char* data_dir, const char* filename) {
    char src[2048];
    char dst[2048];
    
    snprintf(src, sizeof(src), "%s/training_queue/%s", data_dir, filename);
    snprintf(dst, sizeof(dst), "%s/trained/%s", data_dir, filename);
    
    if (rename(src, dst) != 0) {
        fprintf(stderr, "Failed to move file: %s\n", filename);
        return -1;
    }
    
    printf("✓ Moved to trained: %s\n", filename);
    return 0;
}

/**
 * Training worker thread
 */
static void* training_worker_thread(void* arg) {
    ContinuousTrainingState* state = (ContinuousTrainingState*)arg;
    
    char queue_dir[1024];
    snprintf(queue_dir, sizeof(queue_dir), "%s/training_queue", state->data_dir);
    
    while (state->running) {
        DIR* dir = opendir(queue_dir);
        if (!dir) {
            sleep(5);
            continue;
        }
        
        struct dirent* entry;
        int found_file = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            
            // Only process .tok files (not .tok.lock or .tok.lock.lock)
            size_t len = strlen(entry->d_name);
            if (len < 4 || strcmp(entry->d_name + len - 4, ".tok") != 0) {
                continue;
            }
            
            char filepath[2048];
            snprintf(filepath, sizeof(filepath), "%s/%s", queue_dir, entry->d_name);
            
            // Check if locked
            if (is_file_locked(filepath)) {
                continue;
            }
            
            // Try to lock
            if (create_lock(filepath) != 0) {
                continue;
            }
            
            // Train on file
            if (train_on_file(state, filepath) == 0) {
                // Move to trained
                move_to_trained(state->data_dir, entry->d_name);
                
                pthread_mutex_lock(&state->lock);
                state->files_trained++;
                pthread_mutex_unlock(&state->lock);
            }
            
            // Remove lock
            remove_lock(filepath);
            
            found_file = 1;
            break;  // Process one file at a time per thread
        }
        
        closedir(dir);
        
        if (!found_file) {
            sleep(5);  // Wait for new files
        }
    }
    
    return NULL;
}

/**
 * Initialize continuous training
 */
ContinuousTrainingState* continuous_training_init(const char* data_dir, const char* model_path, 
                                                   CLLMModel* model, int num_threads) {
    ContinuousTrainingState* state = (ContinuousTrainingState*)calloc(1, sizeof(ContinuousTrainingState));
    if (!state) return NULL;
    
    strncpy(state->data_dir, data_dir, sizeof(state->data_dir) - 1);
    strncpy(state->model_path, model_path, sizeof(state->model_path) - 1);
    state->model = model;
    state->running = 1;
    state->files_trained = 0;
    state->num_threads = num_threads;
    pthread_mutex_init(&state->lock, NULL);
    
    // Initialize training state
    CLLMTrainingConfig config = {
        .num_epochs = 5,
        .batch_size = 4,
        .sequence_length = 32,
        .learning_rate = 0.001f,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
        .warmup_steps = 100,
        .save_every = 5,
        .eval_interval = 100,
        .max_steps = 10000
    };
    strcpy(config.optimizer, "adam");
    
    state->training = cllm_training_init(model, &config);
    if (!state->training) {
        free(state);
        return NULL;
    }
    
    return state;
}

/**
 * Start training threads
 */
int continuous_training_start(ContinuousTrainingState* state, pthread_t* threads) {
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    printf("%s === CONTINUOUS TRAINING STARTED ===\n", timestamp);
    printf("%s Threads: %d\n", timestamp, state->num_threads);
    printf("%s Model: %s\n", timestamp, state->model_path);
    
    for (int i = 0; i < state->num_threads; i++) {
        if (pthread_create(&threads[i], NULL, training_worker_thread, state) != 0) {
            fprintf(stderr, "%s Failed to create training thread %d\n", timestamp, i);
            return -1;
        }
    }
    
    return 0;
}

/**
 * Stop training threads
 */
void continuous_training_stop(ContinuousTrainingState* state, pthread_t* threads) {
    state->running = 0;
    
    for (int i = 0; i < state->num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s === CONTINUOUS TRAINING STOPPED ===\n", timestamp);
    printf("%s Total files trained: %d\n", timestamp, state->files_trained);
}

/**
 * Cleanup continuous training
 */
void continuous_training_cleanup(ContinuousTrainingState* state) {
    if (!state) return;
    
    if (state->training) {
        cllm_training_free(state->training);
    }
    
    pthread_mutex_destroy(&state->lock);
    free(state);
}