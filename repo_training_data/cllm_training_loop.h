#ifndef CLLM_TRAINING_LOOP_H
#define CLLM_TRAINING_LOOP_H

#include "ai/cllm_control_process.h"
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TRAINING STATES
// ============================================================================

typedef enum {
    TRAINING_STATE_IDLE,           // Not training
    TRAINING_STATE_INITIALIZING,   // Initializing training
    TRAINING_STATE_RUNNING,        // Training in progress
    TRAINING_STATE_PAUSED,         // Training paused
    TRAINING_STATE_CHECKPOINTING,  // Saving checkpoint
    TRAINING_STATE_RESTORING,      // Restoring from checkpoint
    TRAINING_STATE_COMPLETED,      // Training completed
    TRAINING_STATE_ERROR           // Error occurred
} TrainingState;

// ============================================================================
// CALLBACK TYPES
// ============================================================================

typedef enum {
    CALLBACK_EPOCH_START,          // Called at epoch start
    CALLBACK_EPOCH_END,            // Called at epoch end
    CALLBACK_BATCH_START,          // Called at batch start
    CALLBACK_BATCH_END,            // Called at batch end
    CALLBACK_GRADIENT_SYNC,        // Called after gradient sync
    CALLBACK_WEIGHT_UPDATE,        // Called after weight update
    CALLBACK_CHECKPOINT,           // Called during checkpoint
    CALLBACK_ERROR                 // Called on error
} CallbackType;

// ============================================================================
// TRAINING CONFIGURATION
// ============================================================================

typedef struct {
    // Training parameters
    uint32_t num_epochs;           // Total number of epochs
    uint32_t batch_size;           // Batch size
    double learning_rate;          // Initial learning rate
    double learning_rate_decay;    // Learning rate decay factor
    uint32_t warmup_steps;         // Warmup steps
    
    // Gradient settings
    double gradient_clip_value;    // Gradient clipping threshold
    bool gradient_accumulation;    // Enable gradient accumulation
    uint32_t accumulation_steps;   // Steps to accumulate gradients
    
    // Synchronization settings
    uint32_t sync_frequency;       // Batches between synchronizations
    bool async_gradient_sync;      // Asynchronous gradient sync
    
    // Checkpoint settings
    uint32_t checkpoint_frequency; // Epochs between checkpoints
    char checkpoint_dir[256];      // Checkpoint directory
    bool auto_checkpoint;          // Automatic checkpointing
    uint32_t max_checkpoints;      // Maximum checkpoints to keep
    
    // Validation settings
    uint32_t validation_frequency; // Batches between validation
    double validation_split;       // Validation data split
    
    // Performance settings
    bool profile_performance;      // Enable performance profiling
    uint32_t log_frequency;        // Batches between logging
    
} TrainingConfiguration;

// ============================================================================
// BATCH INFORMATION
// ============================================================================

typedef struct {
    uint32_t batch_id;             // Batch ID
    uint32_t epoch_id;             // Epoch ID
    uint32_t batch_size;           // Actual batch size
    
    // Data pointers
    void* input_data;              // Input data
    void* target_data;             // Target data
    
    // Batch statistics
    double loss;                   // Batch loss
    double accuracy;               // Batch accuracy
    double processing_time;        // Processing time (seconds)
    
} BatchInfo;

// ============================================================================
// GRADIENT BUFFER
// ============================================================================

typedef struct {
    double* gradients;             // Gradient values
    size_t gradient_count;         // Number of gradients
    uint32_t version;              // Gradient version
    
    // Accumulation
    uint32_t accumulation_count;   // Number of accumulated gradients
    bool ready_for_sync;           // Ready for synchronization
    
    // Statistics
    double gradient_norm;          // Gradient norm
    double gradient_max;           // Maximum gradient value
    double gradient_min;           // Minimum gradient value
    
    pthread_mutex_t mutex;         // Mutex for thread safety
    
} GradientBuffer;

// ============================================================================
// WEIGHT BUFFER
// ============================================================================

typedef struct {
    double* weights;               // Weight values
    size_t weight_count;           // Number of weights
    uint32_t version;              // Weight version
    
    // Statistics
    double weight_norm;            // Weight norm
    double weight_max;             // Maximum weight value
    double weight_min;             // Minimum weight value
    
    pthread_mutex_t mutex;         // Mutex for thread safety
    
} WeightBuffer;

// ============================================================================
// TRAINING METRICS
// ============================================================================

typedef struct {
    // Loss metrics
    double current_loss;           // Current batch loss
    double epoch_loss;             // Epoch average loss
    double best_loss;              // Best loss achieved
    
    // Accuracy metrics
    double current_accuracy;       // Current batch accuracy
    double epoch_accuracy;         // Epoch average accuracy
    double best_accuracy;          // Best accuracy achieved
    
    // Learning rate
    double current_learning_rate;  // Current learning rate
    
    // Throughput
    double batches_per_second;     // Training throughput
    double samples_per_second;     // Samples processed per second
    
    // Timing
    double epoch_time;             // Time per epoch (seconds)
    double batch_time;             // Time per batch (seconds)
    double sync_time;              // Time for synchronization (seconds)
    
    // Gradient statistics
    double gradient_norm;          // Average gradient norm
    double gradient_variance;      // Gradient variance
    
    // Progress
    uint32_t current_epoch;        // Current epoch
    uint32_t current_batch;        // Current batch
    uint32_t total_batches;        // Total batches processed
    
} TrainingMetrics;

// ============================================================================
// CALLBACK FUNCTION TYPE
// ============================================================================

typedef void (*TrainingCallback)(CallbackType type, void* user_data, void* callback_data);

// ============================================================================
// CALLBACK REGISTRATION
// ============================================================================

typedef struct CallbackRegistration {
    CallbackType type;             // Callback type
    TrainingCallback callback;     // Callback function
    void* user_data;               // User data
    struct CallbackRegistration* next; // Next callback
} CallbackRegistration;

// ============================================================================
// TRAINING LOOP STRUCTURE
// ============================================================================

typedef struct TrainingLoop {
    // State
    TrainingState state;           // Current training state
    pthread_mutex_t state_mutex;   // Mutex for state changes
    
    // Configuration
    TrainingConfiguration config;  // Training configuration
    
    // Control process
    ControlProcess* control_process; // Control process reference
    
    // Buffers
    GradientBuffer gradient_buffer; // Gradient buffer
    WeightBuffer weight_buffer;     // Weight buffer
    
    // Metrics
    TrainingMetrics metrics;       // Training metrics
    pthread_mutex_t metrics_mutex; // Mutex for metrics
    
    // Callbacks
    CallbackRegistration* callbacks; // Callback list
    pthread_mutex_t callback_mutex;  // Mutex for callbacks
    
    // Checkpoint
    char last_checkpoint_path[512]; // Last checkpoint path
    uint32_t checkpoint_version;    // Checkpoint version
    
    // Performance profiling
    double* batch_times;            // Batch processing times
    uint32_t batch_times_count;     // Number of recorded times
    uint32_t batch_times_capacity;  // Capacity of times array
    
} TrainingLoop;

// ============================================================================
// LIFECYCLE FUNCTIONS
// ============================================================================

/**
 * Create and initialize a training loop
 * 
 * @param config Training configuration
 * @param control_process Control process reference
 * @return Pointer to created training loop, or NULL on failure
 */
TrainingLoop* training_loop_create(const TrainingConfiguration* config,
                                   ControlProcess* control_process);

/**
 * Free a training loop and all associated resources
 * 
 * @param loop Training loop to free
 */
void training_loop_free(TrainingLoop* loop);

/**
 * Run the training loop for specified number of epochs
 * 
 * @param loop Training loop
 * @param num_epochs Number of epochs to train (0 = use config)
 * @return true on success, false on failure
 */
bool training_loop_run(TrainingLoop* loop, uint32_t num_epochs);

/**
 * Execute a single training step (one batch)
 * 
 * @param loop Training loop
 * @param batch Batch information
 * @return true on success, false on failure
 */
bool training_loop_step(TrainingLoop* loop, const BatchInfo* batch);

/**
 * Pause the training loop
 * 
 * @param loop Training loop
 * @return true on success, false on failure
 */
bool training_loop_pause(TrainingLoop* loop);

/**
 * Resume the training loop
 * 
 * @param loop Training loop
 * @return true on success, false on failure
 */
bool training_loop_resume(TrainingLoop* loop);

/**
 * Stop the training loop
 * 
 * @param loop Training loop
 * @return true on success, false on failure
 */
bool training_loop_stop(TrainingLoop* loop);

// ============================================================================
// GRADIENT SYNCHRONIZATION
// ============================================================================

/**
 * Synchronize gradients across all spheres
 * 
 * @param loop Training loop
 * @return true on success, false on failure
 */
bool training_loop_sync_gradients(TrainingLoop* loop);

/**
 * Accumulate gradients into buffer
 * 
 * @param loop Training loop
 * @param gradients Gradient values to accumulate
 * @param count Number of gradients
 * @return true on success, false on failure
 */
bool training_loop_accumulate_gradients(TrainingLoop* loop, 
                                        const double* gradients,
                                        size_t count);

/**
 * Average accumulated gradients
 * 
 * @param loop Training loop
 * @return true on success, false on failure
 */
bool training_loop_average_gradients(TrainingLoop* loop);

/**
 * Clip gradients to prevent exploding gradients
 * 
 * @param loop Training loop
 * @return true on success, false on failure
 */
bool training_loop_clip_gradients(TrainingLoop* loop);

/**
 * Validate gradients for NaN or Inf values
 * 
 * @param loop Training loop
 * @return true if valid, false if invalid
 */
bool training_loop_validate_gradients(TrainingLoop* loop);

// ============================================================================
// WEIGHT BROADCASTING
// ============================================================================

/**
 * Broadcast weights to all spheres
 * 
 * @param loop Training loop
 * @return true on success, false on failure
 */
bool training_loop_broadcast_weights(TrainingLoop* loop);

/**
 * Update weights with gradients
 * 
 * @param loop Training loop
 * @return true on success, false on failure
 */
bool training_loop_update_weights(TrainingLoop* loop);

/**
 * Validate weights for NaN or Inf values
 * 
 * @param loop Training loop
 * @return true if valid, false if invalid
 */
bool training_loop_validate_weights(TrainingLoop* loop);

/**
 * Get current weight version
 * 
 * @param loop Training loop
 * @return Current weight version
 */
uint32_t training_loop_get_weight_version(const TrainingLoop* loop);

// ============================================================================
// CHECKPOINT/RESTORE
// ============================================================================

/**
 * Save training state to checkpoint
 * 
 * @param loop Training loop
 * @param checkpoint_name Checkpoint name (NULL for auto-generated)
 * @return true on success, false on failure
 */
bool training_loop_checkpoint(TrainingLoop* loop, const char* checkpoint_name);

/**
 * Restore training state from checkpoint
 * 
 * @param loop Training loop
 * @param checkpoint_path Path to checkpoint
 * @return true on success, false on failure
 */
bool training_loop_restore(TrainingLoop* loop, const char* checkpoint_path);

/**
 * List available checkpoints
 * 
 * @param loop Training loop
 * @param checkpoints Output array of checkpoint paths
 * @param max_checkpoints Maximum number of checkpoints to return
 * @return Number of checkpoints found
 */
uint32_t training_loop_list_checkpoints(const TrainingLoop* loop,
                                        char** checkpoints,
                                        uint32_t max_checkpoints);

/**
 * Delete old checkpoints, keeping only the most recent
 * 
 * @param loop Training loop
 * @param keep_count Number of checkpoints to keep
 * @return Number of checkpoints deleted
 */
uint32_t training_loop_cleanup_checkpoints(TrainingLoop* loop, uint32_t keep_count);

// ============================================================================
// CALLBACK SYSTEM
// ============================================================================

/**
 * Register a training callback
 * 
 * @param loop Training loop
 * @param type Callback type
 * @param callback Callback function
 * @param user_data User data to pass to callback
 * @return true on success, false on failure
 */
bool training_loop_register_callback(TrainingLoop* loop,
                                     CallbackType type,
                                     TrainingCallback callback,
                                     void* user_data);

/**
 * Unregister a training callback
 * 
 * @param loop Training loop
 * @param type Callback type
 * @param callback Callback function
 * @return true on success, false on failure
 */
bool training_loop_unregister_callback(TrainingLoop* loop,
                                       CallbackType type,
                                       TrainingCallback callback);

/**
 * Trigger callbacks of specified type
 * 
 * @param loop Training loop
 * @param type Callback type
 * @param callback_data Data to pass to callbacks
 */
void training_loop_trigger_callbacks(TrainingLoop* loop,
                                     CallbackType type,
                                     void* callback_data);

// ============================================================================
// METRICS & MONITORING
// ============================================================================

/**
 * Get current training metrics
 * 
 * @param loop Training loop
 * @param metrics Output buffer for metrics
 * @return true on success, false on failure
 */
bool training_loop_get_metrics(const TrainingLoop* loop, TrainingMetrics* metrics);

/**
 * Update learning rate
 * 
 * @param loop Training loop
 * @param learning_rate New learning rate
 */
void training_loop_set_learning_rate(TrainingLoop* loop, double learning_rate);

/**
 * Get current learning rate
 * 
 * @param loop Training loop
 * @return Current learning rate
 */
double training_loop_get_learning_rate(const TrainingLoop* loop);

/**
 * Print training progress
 * 
 * @param loop Training loop
 */
void training_loop_print_progress(const TrainingLoop* loop);

/**
 * Print training statistics
 * 
 * @param loop Training loop
 */
void training_loop_print_stats(const TrainingLoop* loop);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get current training state
 * 
 * @param loop Training loop
 * @return Current state
 */
TrainingState training_loop_get_state(const TrainingLoop* loop);

/**
 * Get string representation of training state
 * 
 * @param state State to convert
 * @return String representation
 */
const char* training_loop_state_to_string(TrainingState state);

/**
 * Validate training loop integrity
 * 
 * @param loop Training loop
 * @return true if valid, false if corrupted
 */
bool training_loop_validate(const TrainingLoop* loop);

#ifdef __cplusplus
}
#endif

#endif // CLLM_TRAINING_LOOP_H