/**
 * Training Thread Manager
 * 
 * Runs training in a separate thread to keep UI responsive
 */

#include "app_common.h"
#include "cllm_integration.h"
#include "../include/cllm_training.h"
#include "../include/cllm_training_parallel.h"
#include <pthread.h>
#include <stdio.h>

static pthread_t training_thread;
static pthread_mutex_t training_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool training_thread_active = false;

/**
 * Training thread function
 */
void* training_thread_func(void* arg) {
    AppState* state = (AppState*)arg;
    
    printf("=== TRAINING THREAD STARTED ===\n");
    
    while (state->training_in_progress && state->training_current_epoch < state->training_epochs) {
        // Train one epoch
        float loss = app_train_epoch(state);
        
        // Update state (thread-safe)
        pthread_mutex_lock(&training_mutex);
        state->training_current_epoch++;
        state->training_loss = loss;
        pthread_mutex_unlock(&training_mutex);
        
        // Check if complete
        if (state->training_current_epoch >= state->training_epochs) {
            pthread_mutex_lock(&training_mutex);
            state->training_in_progress = false;
            pthread_mutex_unlock(&training_mutex);
            
            printf("=== TRAINING COMPLETE ===\n");
            printf("Total epochs: %d\n", state->training_current_epoch);
            printf("Final loss: %.4f\n", loss);
            
            // Save final model
            if (state->cllm_model) {
                printf("\nSaving final model...\n");
                
                // Create models directory
                int mkdir_ret = system("mkdir -p models");
                (void)mkdir_ret;
                
                // Save to default location
                const char* model_path = "models/saved_model.cllm";
                extern int app_save_model(CLLMModel* model, const char* filepath);
                
                if (app_save_model(state->cllm_model, model_path) == 0) {
                    printf("✓ Model saved to: %s\n", model_path);
                    printf("  You can load this model later or continue training from it\n");
                } else {
                    printf("✗ Failed to save model\n");
                }
            }
            
            break;
        }
    }
    
    printf("=== TRAINING THREAD STOPPED ===\n");
    
    pthread_mutex_lock(&training_mutex);
    training_thread_active = false;
    pthread_mutex_unlock(&training_mutex);
    
    return NULL;
}

/**
 * Start training in separate thread
 */
int start_training_thread(AppState* state) {
    if (!state || !state->cllm_training) {
        return -1;
    }
    
    pthread_mutex_lock(&training_mutex);
    
    if (training_thread_active) {
        pthread_mutex_unlock(&training_mutex);
        printf("Training thread already running\n");
        return -1;
    }
    
    training_thread_active = true;
    state->training_in_progress = true;
    state->training_current_epoch = 0;
    
    pthread_mutex_unlock(&training_mutex);
    
    if (pthread_create(&training_thread, NULL, training_thread_func, state) != 0) {
        pthread_mutex_lock(&training_mutex);
        training_thread_active = false;
        state->training_in_progress = false;
        pthread_mutex_unlock(&training_mutex);
        return -1;
    }
    
    // Detach thread so it cleans up automatically
    pthread_detach(training_thread);
    
    printf("✓ Training thread started\n");
    return 0;
}

/**
 * Stop training thread
 */
void stop_training_thread(AppState* state) {
    if (!state) return;
    
    pthread_mutex_lock(&training_mutex);
    state->training_in_progress = false;
    pthread_mutex_unlock(&training_mutex);
    
    printf("Training stop requested\n");
}

/**
 * Check if training thread is active
 */
bool is_training_thread_active(void) {
    bool active;
    pthread_mutex_lock(&training_mutex);
    active = training_thread_active;
    pthread_mutex_unlock(&training_mutex);
    return active;
}

/**
 * Get training state (thread-safe)
 */
void get_training_state(AppState* state, int* epoch, float* loss, bool* in_progress) {
    if (!state) return;
    
    pthread_mutex_lock(&training_mutex);
    if (epoch) *epoch = state->training_current_epoch;
    if (loss) *loss = state->training_loss;
    if (in_progress) *in_progress = state->training_in_progress;
    pthread_mutex_unlock(&training_mutex);
}