#include "ai/cllm_training_loop.h"
#include "prime_math_custom.h"
#include "prime_float_math.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static double get_current_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static bool is_valid_double(double value) {
    // Check for NaN: NaN != NaN
    bool is_nan = (value != value);
    // Check for infinity
    bool is_inf = (value == INFINITY || value == -INFINITY);
    return !is_nan && !is_inf;
}

static double compute_norm(const double* values, size_t count) {
    double sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        sum += values[i] * values[i];
    }
    return prime_sqrt(sum);
}

static void create_directory(const char* path) {
    mkdir(path, 0755);
}

// ============================================================================
// LIFECYCLE FUNCTIONS
// ============================================================================

TrainingLoop* training_loop_create(const TrainingConfiguration* config,
                                   ControlProcess* control_process) {
    if (!config || !control_process) {
        fprintf(stderr, "Error: NULL configuration or control process\n");
        return NULL;
    }
    
    TrainingLoop* loop = (TrainingLoop*)calloc(1, sizeof(TrainingLoop));
    if (!loop) {
        fprintf(stderr, "Error: Failed to allocate training loop\n");
        return NULL;
    }
    
    // Initialize state
    loop->state = TRAINING_STATE_IDLE;
    pthread_mutex_init(&loop->state_mutex, NULL);
    
    // Copy configuration
    memcpy(&loop->config, config, sizeof(TrainingConfiguration));
    
    // Set control process
    loop->control_process = control_process;
    
    // Initialize gradient buffer
    loop->gradient_buffer.gradients = NULL;
    loop->gradient_buffer.gradient_count = 0;
    loop->gradient_buffer.version = 0;
    loop->gradient_buffer.accumulation_count = 0;
    loop->gradient_buffer.ready_for_sync = false;
    pthread_mutex_init(&loop->gradient_buffer.mutex, NULL);
    
    // Initialize weight buffer
    loop->weight_buffer.weights = NULL;
    loop->weight_buffer.weight_count = 0;
    loop->weight_buffer.version = 0;
    pthread_mutex_init(&loop->weight_buffer.mutex, NULL);
    
    // Initialize metrics
    memset(&loop->metrics, 0, sizeof(TrainingMetrics));
    loop->metrics.best_loss = INFINITY;
    loop->metrics.current_learning_rate = config->learning_rate;
    pthread_mutex_init(&loop->metrics_mutex, NULL);
    
    // Initialize callbacks
    loop->callbacks = NULL;
    pthread_mutex_init(&loop->callback_mutex, NULL);
    
    // Initialize checkpoint
    loop->checkpoint_version = 0;
    memset(loop->last_checkpoint_path, 0, sizeof(loop->last_checkpoint_path));
    
    // Create checkpoint directory
    create_directory(config->checkpoint_dir);
    
    // Initialize performance profiling
    if (config->profile_performance) {
        loop->batch_times_capacity = 1000;
        loop->batch_times = (double*)calloc(loop->batch_times_capacity, sizeof(double));
        loop->batch_times_count = 0;
    } else {
        loop->batch_times = NULL;
        loop->batch_times_count = 0;
        loop->batch_times_capacity = 0;
    }
    
    return loop;
}

void training_loop_free(TrainingLoop* loop) {
    if (!loop) return;
    
    // Free gradient buffer
    pthread_mutex_lock(&loop->gradient_buffer.mutex);
    if (loop->gradient_buffer.gradients) {
        free(loop->gradient_buffer.gradients);
    }
    pthread_mutex_unlock(&loop->gradient_buffer.mutex);
    pthread_mutex_destroy(&loop->gradient_buffer.mutex);
    
    // Free weight buffer
    pthread_mutex_lock(&loop->weight_buffer.mutex);
    if (loop->weight_buffer.weights) {
        free(loop->weight_buffer.weights);
    }
    pthread_mutex_unlock(&loop->weight_buffer.mutex);
    pthread_mutex_destroy(&loop->weight_buffer.mutex);
    
    // Free callbacks
    pthread_mutex_lock(&loop->callback_mutex);
    CallbackRegistration* cb = loop->callbacks;
    while (cb) {
        CallbackRegistration* next = cb->next;
        free(cb);
        cb = next;
    }
    pthread_mutex_unlock(&loop->callback_mutex);
    pthread_mutex_destroy(&loop->callback_mutex);
    
    // Free performance profiling
    if (loop->batch_times) {
        free(loop->batch_times);
    }
    
    // Destroy mutexes
    pthread_mutex_destroy(&loop->state_mutex);
    pthread_mutex_destroy(&loop->metrics_mutex);
    
    free(loop);
}

bool training_loop_run(TrainingLoop* loop, uint32_t num_epochs) {
    if (!loop) return false;
    
    pthread_mutex_lock(&loop->state_mutex);
    
    if (loop->state != TRAINING_STATE_IDLE) {
        pthread_mutex_unlock(&loop->state_mutex);
        fprintf(stderr, "Error: Training loop not in IDLE state\n");
        return false;
    }
    
    loop->state = TRAINING_STATE_INITIALIZING;
    pthread_mutex_unlock(&loop->state_mutex);
    
    // Use config epochs if not specified
    if (num_epochs == 0) {
        num_epochs = loop->config.num_epochs;
    }
    
    // Start control process if not running
    if (control_process_get_state(loop->control_process) != CONTROL_STATE_RUNNING) {
        if (!control_process_start(loop->control_process)) {
            pthread_mutex_lock(&loop->state_mutex);
            loop->state = TRAINING_STATE_ERROR;
            pthread_mutex_unlock(&loop->state_mutex);
            return false;
        }
    }
    
    // Update state to running
    pthread_mutex_lock(&loop->state_mutex);
    loop->state = TRAINING_STATE_RUNNING;
    pthread_mutex_unlock(&loop->state_mutex);
    
    // Main training loop
    for (uint32_t epoch = 0; epoch < num_epochs; epoch++) {
        // Check if paused or stopped
        pthread_mutex_lock(&loop->state_mutex);
        if (loop->state == TRAINING_STATE_PAUSED) {
            pthread_mutex_unlock(&loop->state_mutex);
            // Wait for resume
            while (loop->state == TRAINING_STATE_PAUSED) {
                usleep(100000); // 100ms
            }
            pthread_mutex_lock(&loop->state_mutex);
        }
        if (loop->state != TRAINING_STATE_RUNNING) {
            pthread_mutex_unlock(&loop->state_mutex);
            break;
        }
        pthread_mutex_unlock(&loop->state_mutex);
        
        // Start epoch
        double epoch_start_time = get_current_time();
        
        pthread_mutex_lock(&loop->metrics_mutex);
        loop->metrics.current_epoch = epoch;
        loop->metrics.epoch_loss = 0.0;
        loop->metrics.epoch_accuracy = 0.0;
        pthread_mutex_unlock(&loop->metrics_mutex);
        
        // Trigger epoch start callbacks
        training_loop_trigger_callbacks(loop, CALLBACK_EPOCH_START, &epoch);
        
        // Start epoch in control process
        control_process_start_epoch(loop->control_process, loop->config.batch_size);
        
        // TODO: Actual batch processing would go here
        // For now, this is a placeholder
        
        // End epoch
        control_process_end_epoch(loop->control_process);
        
        // Update epoch metrics
        double epoch_end_time = get_current_time();
        pthread_mutex_lock(&loop->metrics_mutex);
        loop->metrics.epoch_time = epoch_end_time - epoch_start_time;
        pthread_mutex_unlock(&loop->metrics_mutex);
        
        // Trigger epoch end callbacks
        training_loop_trigger_callbacks(loop, CALLBACK_EPOCH_END, &epoch);
        
        // Checkpoint if needed
        if (loop->config.auto_checkpoint && 
            (epoch + 1) % loop->config.checkpoint_frequency == 0) {
            training_loop_checkpoint(loop, NULL);
        }
        
        // Print progress
        if ((epoch + 1) % loop->config.log_frequency == 0) {
            training_loop_print_progress(loop);
        }
    }
    
    // Update state to completed
    pthread_mutex_lock(&loop->state_mutex);
    loop->state = TRAINING_STATE_COMPLETED;
    pthread_mutex_unlock(&loop->state_mutex);
    
    return true;
}

bool training_loop_step(TrainingLoop* loop, const BatchInfo* batch) {
    if (!loop || !batch) return false;
    
    double step_start_time = get_current_time();
    
    // Trigger batch start callbacks
    training_loop_trigger_callbacks(loop, CALLBACK_BATCH_START, (void*)batch);
    
    // TODO: Actual forward/backward pass would go here
    // For now, this is a placeholder
    
    // Update metrics
    pthread_mutex_lock(&loop->metrics_mutex);
    loop->metrics.current_batch++;
    loop->metrics.total_batches++;
    loop->metrics.current_loss = batch->loss;
    loop->metrics.current_accuracy = batch->accuracy;
    pthread_mutex_unlock(&loop->metrics_mutex);
    
    // Synchronize gradients if needed
    if (loop->metrics.current_batch % loop->config.sync_frequency == 0) {
        training_loop_sync_gradients(loop);
    }
    
    // Update weights
    training_loop_update_weights(loop);
    
    // Broadcast weights if needed
    if (loop->metrics.current_batch % loop->config.sync_frequency == 0) {
        training_loop_broadcast_weights(loop);
    }
    
    // Record batch time
    double step_end_time = get_current_time();
    double batch_time = step_end_time - step_start_time;
    
    pthread_mutex_lock(&loop->metrics_mutex);
    loop->metrics.batch_time = batch_time;
    pthread_mutex_unlock(&loop->metrics_mutex);
    
    // Record for profiling
    if (loop->batch_times && loop->batch_times_count < loop->batch_times_capacity) {
        loop->batch_times[loop->batch_times_count++] = batch_time;
    }
    
    // Trigger batch end callbacks
    training_loop_trigger_callbacks(loop, CALLBACK_BATCH_END, (void*)batch);
    
    return true;
}

bool training_loop_pause(TrainingLoop* loop) {
    if (!loop) return false;
    
    pthread_mutex_lock(&loop->state_mutex);
    
    if (loop->state != TRAINING_STATE_RUNNING) {
        pthread_mutex_unlock(&loop->state_mutex);
        return false;
    }
    
    loop->state = TRAINING_STATE_PAUSED;
    pthread_mutex_unlock(&loop->state_mutex);
    
    // Pause control process
    control_process_pause(loop->control_process);
    
    return true;
}

bool training_loop_resume(TrainingLoop* loop) {
    if (!loop) return false;
    
    pthread_mutex_lock(&loop->state_mutex);
    
    if (loop->state != TRAINING_STATE_PAUSED) {
        pthread_mutex_unlock(&loop->state_mutex);
        return false;
    }
    
    loop->state = TRAINING_STATE_RUNNING;
    pthread_mutex_unlock(&loop->state_mutex);
    
    // Resume control process
    control_process_resume(loop->control_process);
    
    return true;
}

bool training_loop_stop(TrainingLoop* loop) {
    if (!loop) return false;
    
    pthread_mutex_lock(&loop->state_mutex);
    loop->state = TRAINING_STATE_IDLE;
    pthread_mutex_unlock(&loop->state_mutex);
    
    return true;
}

// ============================================================================
// GRADIENT SYNCHRONIZATION
// ============================================================================

bool training_loop_sync_gradients(TrainingLoop* loop) {
    if (!loop) return false;
    
    double sync_start_time = get_current_time();
    
    pthread_mutex_lock(&loop->gradient_buffer.mutex);
    
    // Collect gradients from control process
    if (loop->gradient_buffer.gradients && loop->gradient_buffer.gradient_count > 0) {
        control_process_collect_gradients(loop->control_process,
                                         loop->gradient_buffer.gradients,
                                         loop->gradient_buffer.gradient_count);
    }
    
    // Average gradients
    training_loop_average_gradients(loop);
    
    // Clip gradients
    if (loop->config.gradient_clip_value > 0.0) {
        training_loop_clip_gradients(loop);
    }
    
    // Validate gradients
    bool valid = training_loop_validate_gradients(loop);
    
    pthread_mutex_unlock(&loop->gradient_buffer.mutex);
    
    // Update sync time
    double sync_end_time = get_current_time();
    pthread_mutex_lock(&loop->metrics_mutex);
    loop->metrics.sync_time = sync_end_time - sync_start_time;
    pthread_mutex_unlock(&loop->metrics_mutex);
    
    // Trigger gradient sync callbacks
    training_loop_trigger_callbacks(loop, CALLBACK_GRADIENT_SYNC, NULL);
    
    return valid;
}

bool training_loop_accumulate_gradients(TrainingLoop* loop, 
                                        const double* gradients,
                                        size_t count) {
    if (!loop || !gradients) return false;
    
    pthread_mutex_lock(&loop->gradient_buffer.mutex);
    
    // Allocate gradient buffer if needed
    if (!loop->gradient_buffer.gradients) {
        loop->gradient_buffer.gradients = (double*)calloc(count, sizeof(double));
        loop->gradient_buffer.gradient_count = count;
    }
    
    // Accumulate gradients
    for (size_t i = 0; i < count && i < loop->gradient_buffer.gradient_count; i++) {
        loop->gradient_buffer.gradients[i] += gradients[i];
    }
    
    loop->gradient_buffer.accumulation_count++;
    
    // Check if ready for sync
    if (loop->gradient_buffer.accumulation_count >= loop->config.accumulation_steps) {
        loop->gradient_buffer.ready_for_sync = true;
    }
    
    pthread_mutex_unlock(&loop->gradient_buffer.mutex);
    
    return true;
}

bool training_loop_average_gradients(TrainingLoop* loop) {
    if (!loop) return false;
    
    // Already locked by caller
    
    if (loop->gradient_buffer.accumulation_count > 0) {
        double scale = 1.0 / loop->gradient_buffer.accumulation_count;
        for (size_t i = 0; i < loop->gradient_buffer.gradient_count; i++) {
            loop->gradient_buffer.gradients[i] *= scale;
        }
        loop->gradient_buffer.accumulation_count = 0;
    }
    
    return true;
}

bool training_loop_clip_gradients(TrainingLoop* loop) {
    if (!loop) return false;
    
    // Already locked by caller
    
    double clip_value = loop->config.gradient_clip_value;
    
    for (size_t i = 0; i < loop->gradient_buffer.gradient_count; i++) {
        if (loop->gradient_buffer.gradients[i] > clip_value) {
            loop->gradient_buffer.gradients[i] = clip_value;
        } else if (loop->gradient_buffer.gradients[i] < -clip_value) {
            loop->gradient_buffer.gradients[i] = -clip_value;
        }
    }
    
    return true;
}

bool training_loop_validate_gradients(TrainingLoop* loop) {
    if (!loop) return false;
    
    // Already locked by caller
    
    for (size_t i = 0; i < loop->gradient_buffer.gradient_count; i++) {
        if (!is_valid_double(loop->gradient_buffer.gradients[i])) {
            fprintf(stderr, "Error: Invalid gradient at index %zu\n", i);
            return false;
        }
    }
    
    // Compute gradient statistics
    loop->gradient_buffer.gradient_norm = compute_norm(loop->gradient_buffer.gradients,
                                                       loop->gradient_buffer.gradient_count);
    
    loop->gradient_buffer.gradient_max = loop->gradient_buffer.gradients[0];
    loop->gradient_buffer.gradient_min = loop->gradient_buffer.gradients[0];
    
    for (size_t i = 1; i < loop->gradient_buffer.gradient_count; i++) {
        if (loop->gradient_buffer.gradients[i] > loop->gradient_buffer.gradient_max) {
            loop->gradient_buffer.gradient_max = loop->gradient_buffer.gradients[i];
        }
        if (loop->gradient_buffer.gradients[i] < loop->gradient_buffer.gradient_min) {
            loop->gradient_buffer.gradient_min = loop->gradient_buffer.gradients[i];
        }
    }
    
    return true;
}

// ============================================================================
// WEIGHT BROADCASTING
// ============================================================================

bool training_loop_broadcast_weights(TrainingLoop* loop) {
    if (!loop) return false;
    
    pthread_mutex_lock(&loop->weight_buffer.mutex);
    
    if (loop->weight_buffer.weights && loop->weight_buffer.weight_count > 0) {
        control_process_broadcast_weights(loop->control_process,
                                         loop->weight_buffer.weights,
                                         loop->weight_buffer.weight_count);
    }
    
    loop->weight_buffer.version++;
    
    pthread_mutex_unlock(&loop->weight_buffer.mutex);
    
    return true;
}

bool training_loop_update_weights(TrainingLoop* loop) {
    if (!loop) return false;
    
    pthread_mutex_lock(&loop->weight_buffer.mutex);
    pthread_mutex_lock(&loop->gradient_buffer.mutex);
    
    // Allocate weight buffer if needed
    if (!loop->weight_buffer.weights && loop->gradient_buffer.gradient_count > 0) {
        loop->weight_buffer.weights = (double*)calloc(loop->gradient_buffer.gradient_count,
                                                      sizeof(double));
        loop->weight_buffer.weight_count = loop->gradient_buffer.gradient_count;
    }
    
    // Update weights with gradients (simple SGD)
    double learning_rate = loop->metrics.current_learning_rate;
    
    for (size_t i = 0; i < loop->weight_buffer.weight_count && 
                        i < loop->gradient_buffer.gradient_count; i++) {
        loop->weight_buffer.weights[i] -= learning_rate * loop->gradient_buffer.gradients[i];
    }
    
    pthread_mutex_unlock(&loop->gradient_buffer.mutex);
    pthread_mutex_unlock(&loop->weight_buffer.mutex);
    
    // Trigger weight update callbacks
    training_loop_trigger_callbacks(loop, CALLBACK_WEIGHT_UPDATE, NULL);
    
    return true;
}

bool training_loop_validate_weights(TrainingLoop* loop) {
    if (!loop) return false;
    
    pthread_mutex_lock(&loop->weight_buffer.mutex);
    
    for (size_t i = 0; i < loop->weight_buffer.weight_count; i++) {
        if (!is_valid_double(loop->weight_buffer.weights[i])) {
            pthread_mutex_unlock(&loop->weight_buffer.mutex);
            fprintf(stderr, "Error: Invalid weight at index %zu\n", i);
            return false;
        }
    }
    
    // Compute weight statistics
    loop->weight_buffer.weight_norm = compute_norm(loop->weight_buffer.weights,
                                                   loop->weight_buffer.weight_count);
    
    loop->weight_buffer.weight_max = loop->weight_buffer.weights[0];
    loop->weight_buffer.weight_min = loop->weight_buffer.weights[0];
    
    for (size_t i = 1; i < loop->weight_buffer.weight_count; i++) {
        if (loop->weight_buffer.weights[i] > loop->weight_buffer.weight_max) {
            loop->weight_buffer.weight_max = loop->weight_buffer.weights[i];
        }
        if (loop->weight_buffer.weights[i] < loop->weight_buffer.weight_min) {
            loop->weight_buffer.weight_min = loop->weight_buffer.weights[i];
        }
    }
    
    pthread_mutex_unlock(&loop->weight_buffer.mutex);
    
    return true;
}

uint32_t training_loop_get_weight_version(const TrainingLoop* loop) {
    if (!loop) return 0;
    return loop->weight_buffer.version;
}

// ============================================================================
// CHECKPOINT/RESTORE
// ============================================================================

bool training_loop_checkpoint(TrainingLoop* loop, const char* checkpoint_name) {
    if (!loop) return false;
    
    pthread_mutex_lock(&loop->state_mutex);
    TrainingState prev_state = loop->state;
    loop->state = TRAINING_STATE_CHECKPOINTING;
    pthread_mutex_unlock(&loop->state_mutex);
    
    // Generate checkpoint name if not provided
    char checkpoint_path[512];
    if (checkpoint_name) {
        snprintf(checkpoint_path, sizeof(checkpoint_path), "%s/%s",
                loop->config.checkpoint_dir, checkpoint_name);
    } else {
        snprintf(checkpoint_path, sizeof(checkpoint_path), "%s/checkpoint_%u.ckpt",
                loop->config.checkpoint_dir, loop->checkpoint_version);
    }
    
    // Save checkpoint
    FILE* fp = fopen(checkpoint_path, "wb");
    if (!fp) {
        pthread_mutex_lock(&loop->state_mutex);
        loop->state = prev_state;
        pthread_mutex_unlock(&loop->state_mutex);
        fprintf(stderr, "Error: Failed to create checkpoint file\n");
        return false;
    }
    
    // Write checkpoint header
    fprintf(fp, "CLLM_CHECKPOINT_V1\n");
    fprintf(fp, "version=%u\n", loop->checkpoint_version);
    fprintf(fp, "epoch=%u\n", loop->metrics.current_epoch);
    fprintf(fp, "batch=%u\n", loop->metrics.current_batch);
    fprintf(fp, "learning_rate=%f\n", loop->metrics.current_learning_rate);
    
    // TODO: Write actual model state
    
    fclose(fp);
    
    // Update checkpoint info
    strncpy(loop->last_checkpoint_path, checkpoint_path, 
           sizeof(loop->last_checkpoint_path) - 1);
    loop->last_checkpoint_path[sizeof(loop->last_checkpoint_path) - 1] = '\0';  // Ensure null termination
    loop->checkpoint_version++;
    
    // Trigger checkpoint callbacks
    training_loop_trigger_callbacks(loop, CALLBACK_CHECKPOINT, checkpoint_path);
    
    // Restore previous state
    pthread_mutex_lock(&loop->state_mutex);
    loop->state = prev_state;
    pthread_mutex_unlock(&loop->state_mutex);
    
    // Cleanup old checkpoints
    if (loop->config.max_checkpoints > 0) {
        training_loop_cleanup_checkpoints(loop, loop->config.max_checkpoints);
    }
    
    return true;
}

bool training_loop_restore(TrainingLoop* loop, const char* checkpoint_path) {
    if (!loop || !checkpoint_path) return false;
    
    pthread_mutex_lock(&loop->state_mutex);
    loop->state = TRAINING_STATE_RESTORING;
    pthread_mutex_unlock(&loop->state_mutex);
    
    FILE* fp = fopen(checkpoint_path, "rb");
    if (!fp) {
        pthread_mutex_lock(&loop->state_mutex);
        loop->state = TRAINING_STATE_ERROR;
        pthread_mutex_unlock(&loop->state_mutex);
        fprintf(stderr, "Error: Failed to open checkpoint file\n");
        return false;
    }
    
    // Read checkpoint header
    char header[256];
    if (!fgets(header, sizeof(header), fp) || 
        strncmp(header, "CLLM_CHECKPOINT_V1", 18) != 0) {
        fclose(fp);
        pthread_mutex_lock(&loop->state_mutex);
        loop->state = TRAINING_STATE_ERROR;
        pthread_mutex_unlock(&loop->state_mutex);
        fprintf(stderr, "Error: Invalid checkpoint format\n");
        return false;
    }
    
    // Read checkpoint data
    uint32_t version, epoch, batch;
    double learning_rate;
    fscanf(fp, "version=%u\n", &version);
    fscanf(fp, "epoch=%u\n", &epoch);
    fscanf(fp, "batch=%u\n", &batch);
    fscanf(fp, "learning_rate=%lf\n", &learning_rate);
    
    // TODO: Read actual model state
    
    fclose(fp);
    
    // Update training loop state
    pthread_mutex_lock(&loop->metrics_mutex);
    loop->metrics.current_epoch = epoch;
    loop->metrics.current_batch = batch;
    loop->metrics.current_learning_rate = learning_rate;
    pthread_mutex_unlock(&loop->metrics_mutex);
    
    loop->checkpoint_version = version + 1;
    
    pthread_mutex_lock(&loop->state_mutex);
    loop->state = TRAINING_STATE_IDLE;
    pthread_mutex_unlock(&loop->state_mutex);
    
    return true;
}

uint32_t training_loop_list_checkpoints(const TrainingLoop* loop,
                                        char** checkpoints,
                                        uint32_t max_checkpoints) {
    if (!loop || !checkpoints) return 0;
    
    DIR* dir = opendir(loop->config.checkpoint_dir);
    if (!dir) return 0;
    
    uint32_t count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL && count < max_checkpoints) {
        if (strstr(entry->d_name, ".ckpt")) {
            checkpoints[count] = strdup(entry->d_name);
            count++;
        }
    }
    
    closedir(dir);
    return count;
}

uint32_t training_loop_cleanup_checkpoints(TrainingLoop* loop, uint32_t keep_count) {
    (void)keep_count; // Reserved for future checkpoint cleanup implementation
    if (!loop) return 0;
    
    // TODO: Implement checkpoint cleanup
    // This would involve:
    // 1. List all checkpoints
    // 2. Sort by timestamp
    // 3. Delete oldest checkpoints beyond keep_count
    
    return 0;
}

// ============================================================================
// CALLBACK SYSTEM
// ============================================================================

bool training_loop_register_callback(TrainingLoop* loop,
                                     CallbackType type,
                                     TrainingCallback callback,
                                     void* user_data) {
    if (!loop || !callback) return false;
    
    CallbackRegistration* reg = (CallbackRegistration*)malloc(sizeof(CallbackRegistration));
    if (!reg) return false;
    
    reg->type = type;
    reg->callback = callback;
    reg->user_data = user_data;
    
    pthread_mutex_lock(&loop->callback_mutex);
    reg->next = loop->callbacks;
    loop->callbacks = reg;
    pthread_mutex_unlock(&loop->callback_mutex);
    
    return true;
}

bool training_loop_unregister_callback(TrainingLoop* loop,
                                       CallbackType type,
                                       TrainingCallback callback) {
    if (!loop || !callback) return false;
    
    pthread_mutex_lock(&loop->callback_mutex);
    
    CallbackRegistration** prev = &loop->callbacks;
    CallbackRegistration* curr = loop->callbacks;
    
    while (curr) {
        if (curr->type == type && curr->callback == callback) {
            *prev = curr->next;
            free(curr);
            pthread_mutex_unlock(&loop->callback_mutex);
            return true;
        }
        prev = &curr->next;
        curr = curr->next;
    }
    
    pthread_mutex_unlock(&loop->callback_mutex);
    return false;
}

void training_loop_trigger_callbacks(TrainingLoop* loop,
                                     CallbackType type,
                                     void* callback_data) {
    if (!loop) return;
    
    pthread_mutex_lock(&loop->callback_mutex);
    
    CallbackRegistration* cb = loop->callbacks;
    while (cb) {
        if (cb->type == type) {
            cb->callback(type, cb->user_data, callback_data);
        }
        cb = cb->next;
    }
    
    pthread_mutex_unlock(&loop->callback_mutex);
}

// ============================================================================
// METRICS & MONITORING
// ============================================================================

bool training_loop_get_metrics(const TrainingLoop* loop, TrainingMetrics* metrics) {
    if (!loop || !metrics) return false;
    
    pthread_mutex_lock((pthread_mutex_t*)&loop->metrics_mutex);
    memcpy(metrics, &loop->metrics, sizeof(TrainingMetrics));
    pthread_mutex_unlock((pthread_mutex_t*)&loop->metrics_mutex);
    
    return true;
}

void training_loop_set_learning_rate(TrainingLoop* loop, double learning_rate) {
    if (!loop) return;
    
    pthread_mutex_lock(&loop->metrics_mutex);
    loop->metrics.current_learning_rate = learning_rate;
    pthread_mutex_unlock(&loop->metrics_mutex);
}

double training_loop_get_learning_rate(const TrainingLoop* loop) {
    if (!loop) return 0.0;
    return loop->metrics.current_learning_rate;
}

void training_loop_print_progress(const TrainingLoop* loop) {
    if (!loop) return;
    
    pthread_mutex_lock((pthread_mutex_t*)&loop->metrics_mutex);
    
    printf("Epoch %u | Batch %u | Loss: %.6f | Accuracy: %.4f | LR: %.6f\n",
           loop->metrics.current_epoch,
           loop->metrics.current_batch,
           loop->metrics.current_loss,
           loop->metrics.current_accuracy,
           loop->metrics.current_learning_rate);
    
    pthread_mutex_unlock((pthread_mutex_t*)&loop->metrics_mutex);
}

void training_loop_print_stats(const TrainingLoop* loop) {
    if (!loop) return;
    
    printf("\n=== Training Statistics ===\n");
    
    pthread_mutex_lock((pthread_mutex_t*)&loop->metrics_mutex);
    
    printf("\nProgress:\n");
    printf("  Current Epoch: %u\n", loop->metrics.current_epoch);
    printf("  Current Batch: %u\n", loop->metrics.current_batch);
    printf("  Total Batches: %u\n", loop->metrics.total_batches);
    
    printf("\nLoss:\n");
    printf("  Current Loss: %.6f\n", loop->metrics.current_loss);
    printf("  Epoch Loss: %.6f\n", loop->metrics.epoch_loss);
    printf("  Best Loss: %.6f\n", loop->metrics.best_loss);
    
    printf("\nAccuracy:\n");
    printf("  Current Accuracy: %.4f\n", loop->metrics.current_accuracy);
    printf("  Epoch Accuracy: %.4f\n", loop->metrics.epoch_accuracy);
    printf("  Best Accuracy: %.4f\n", loop->metrics.best_accuracy);
    
    printf("\nTiming:\n");
    printf("  Epoch Time: %.2f seconds\n", loop->metrics.epoch_time);
    printf("  Batch Time: %.4f seconds\n", loop->metrics.batch_time);
    printf("  Sync Time: %.4f seconds\n", loop->metrics.sync_time);
    
    printf("\nThroughput:\n");
    printf("  Batches/Second: %.2f\n", loop->metrics.batches_per_second);
    printf("  Samples/Second: %.2f\n", loop->metrics.samples_per_second);
    
    pthread_mutex_unlock((pthread_mutex_t*)&loop->metrics_mutex);
    
    printf("\n");
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

TrainingState training_loop_get_state(const TrainingLoop* loop) {
    if (!loop) return TRAINING_STATE_ERROR;
    return loop->state;
}

const char* training_loop_state_to_string(TrainingState state) {
    switch (state) {
        case TRAINING_STATE_IDLE: return "IDLE";
        case TRAINING_STATE_INITIALIZING: return "INITIALIZING";
        case TRAINING_STATE_RUNNING: return "RUNNING";
        case TRAINING_STATE_PAUSED: return "PAUSED";
        case TRAINING_STATE_CHECKPOINTING: return "CHECKPOINTING";
        case TRAINING_STATE_RESTORING: return "RESTORING";
        case TRAINING_STATE_COMPLETED: return "COMPLETED";
        case TRAINING_STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

bool training_loop_validate(const TrainingLoop* loop) {
    if (!loop) return false;
    
    // Check control process
    if (!loop->control_process) return false;
    
    // Basic validation without locking
    // (weights and gradients validation requires locking, skip for const)
    
    return true;
}