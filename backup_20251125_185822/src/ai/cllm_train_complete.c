/**
 * Complete Training Loop
 * 
 * Integrates all optimizations and production features:
 * 1. Multi-threaded training with gradient accumulation
 * 2. LLL lattice reduction for dimensionality reduction
 * 3. Advanced crystalline features (CVP, SVP, cached GCD)
 * 4. Learning rate scheduling with warmup and decay
 * 5. Gradient clipping
 * 6. Validation and early stopping
 * 7. Checkpoint saving
 * 8. Metrics logging
 */

#include "cllm_training.h"
#include "cllm_training_mt.h"
#include "cllm_lll_embeddings.h"
#include "cllm_crystalline_advanced.h"
#include "cllm_production.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../include/prime_float_math.h"

// Complete training configuration
typedef struct {
    // Basic training
    int num_epochs;
    int batch_size;
    int sequence_length;
    float learning_rate;
    
    // Multi-threading
    int num_threads;  // 0 = auto-detect
    
    // LLL reduction
    int use_lll;
    int lll_target_dim;  // e.g., 64 or 32
    
    // Advanced crystalline
    int use_advanced_crystalline;
    
    // Learning rate scheduling
    int use_lr_schedule;
    int warmup_epochs;
    
    // Gradient clipping
    int use_grad_clip;
    float grad_clip_norm;
    
    // Validation
    int use_validation;
    const char* val_data_path;
    int val_interval;  // Validate every N epochs
    
    // Early stopping
    int use_early_stopping;
    int early_stop_patience;
    
    // Checkpointing
    int use_checkpoints;
    const char* checkpoint_dir;
    int checkpoint_interval;  // Save every N epochs
    
    // Metrics
    int use_metrics;
    const char* metrics_path;
} CompleteTrainingConfig;

/**
 * Auto-detect number of CPU cores
 */
static int get_cpu_count() {
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1) return 4;  // Default fallback
    return (int)nprocs;
}

/**
 * Create default complete training config
 */
CompleteTrainingConfig create_default_complete_config() {
    CompleteTrainingConfig config = {0};
    
    // Basic training
    config.num_epochs = 100;
    config.batch_size = 4;
    config.sequence_length = 32;
    config.learning_rate = 0.001f;
    
    // Multi-threading (use all cores minus 1)
    config.num_threads = get_cpu_count() - 1;
    if (config.num_threads < 1) config.num_threads = 1;
    
    // LLL reduction (reduce to 64 dimensions)
    config.use_lll = 1;
    config.lll_target_dim = 64;
    
    // Advanced crystalline features
    config.use_advanced_crystalline = 1;
    
    // Learning rate scheduling
    config.use_lr_schedule = 1;
    config.warmup_epochs = 5;
    
    // Gradient clipping
    config.use_grad_clip = 1;
    config.grad_clip_norm = 1.0f;
    
    // Validation
    config.use_validation = 1;
    config.val_data_path = "data/training/validation.txt";
    config.val_interval = 5;
    
    // Early stopping
    config.use_early_stopping = 1;
    config.early_stop_patience = 10;
    
    // Checkpointing
    config.use_checkpoints = 1;
    config.checkpoint_dir = "checkpoints";
    config.checkpoint_interval = 10;
    
    // Metrics
    config.use_metrics = 1;
    config.metrics_path = "training_metrics.csv";
    
    return config;
}

/**
 * Complete training loop with all optimizations
 */
int train_complete(CLLMTraining* training, CompleteTrainingConfig* config) {
    if (!training || !config) return -1;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     CRYSTALLINE CLLM - COMPLETE TRAINING SYSTEM           ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    // Print configuration
    printf("Configuration:\n");
    printf("  Epochs: %d\n", config->num_epochs);
    printf("  Batch size: %d\n", config->batch_size);
    printf("  Sequence length: %d\n", config->sequence_length);
    printf("  Learning rate: %.6f\n", config->learning_rate);
    printf("  Threads: %d\n", config->num_threads);
    printf("  LLL reduction: %s", config->use_lll ? "enabled" : "disabled");
    if (config->use_lll) printf(" (%lu → %d dims)", (unsigned long)training->model->embedding_dim, config->lll_target_dim);
    printf("\n");
    printf("  Advanced crystalline: %s\n", config->use_advanced_crystalline ? "enabled" : "disabled");
    printf("  LR scheduling: %s\n", config->use_lr_schedule ? "enabled" : "disabled");
    printf("  Gradient clipping: %s", config->use_grad_clip ? "enabled" : "disabled");
    if (config->use_grad_clip) printf(" (max norm: %.2f)", config->grad_clip_norm);
    printf("\n");
    printf("  Validation: %s\n", config->use_validation ? "enabled" : "disabled");
    printf("  Early stopping: %s", config->use_early_stopping ? "enabled" : "disabled");
    if (config->use_early_stopping) printf(" (patience: %d)", config->early_stop_patience);
    printf("\n");
    printf("  Checkpointing: %s\n", config->use_checkpoints ? "enabled" : "disabled");
    printf("\n");
    
    // Initialize components
    ValidationSet* val_set = NULL;
    EarlyStoppingState* early_stop = NULL;
    LRScheduler* lr_scheduler = NULL;
    TrainingMetrics* metrics = NULL;
    CrystallineAdvancedState* advanced_state = NULL;
    
    // Apply LLL reduction
    if (config->use_lll && config->lll_target_dim > 0 && 
        config->lll_target_dim < (int)training->model->embedding_dim) {
        printf("Applying LLL lattice reduction...\n");
        lll_integrate_training(training, config->lll_target_dim);
        printf("✓ LLL reduction complete\n\n");
    }
    
    // Initialize advanced crystalline features
    if (config->use_advanced_crystalline) {
        printf("Initializing advanced crystalline features...\n");
        advanced_state = crystalline_advanced_create(training->model);
        printf("✓ Advanced features initialized\n\n");
    }
    
    // Load validation set
    if (config->use_validation && config->val_data_path) {
        printf("Loading validation set...\n");
        val_set = create_validation_set(config->val_data_path, 
                                       config->batch_size, 
                                       config->sequence_length);
        if (val_set) {
            printf("✓ Validation set loaded\n\n");
        } else {
            printf("⚠ Validation set not found, continuing without validation\n\n");
            config->use_validation = 0;
        }
    }
    
    // Initialize early stopping
    if (config->use_early_stopping && config->use_validation) {
        early_stop = create_early_stopping(INFINITY, config->early_stop_patience);
    }
    
    // Initialize learning rate scheduler
    if (config->use_lr_schedule) {
        int warmup_steps = config->warmup_epochs * training->total_batches;
        int total_steps = config->num_epochs * training->total_batches;
        lr_scheduler = create_lr_scheduler(config->learning_rate, warmup_steps, total_steps);
    }
    
    // Initialize metrics tracking
    if (config->use_metrics) {
        metrics = create_training_metrics(config->num_epochs);
    }
    
    // Create checkpoint directory
    if (config->use_checkpoints) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "mkdir -p %s", config->checkpoint_dir);
        system(cmd);
    }
    
    printf("Starting training...\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    time_t start_time = time(NULL);
    
    // Training loop
    for (int epoch = 0; epoch < config->num_epochs; epoch++) {
        training->current_epoch = epoch;
        
        printf("Epoch %d/%d:\n", epoch + 1, config->num_epochs);
        
        // Train one epoch (multi-threaded if enabled)
        float train_loss;
        if (config->num_threads > 1) {
            train_loss = cllm_train_epoch_mt(training, config->num_threads);
        } else {
            train_loss = cllm_train_epoch(training);
        }
        
        // Clip gradients if enabled
        if (config->use_grad_clip) {
            clip_gradients(training, config->grad_clip_norm);
        }
        
        // Update learning rate
        if (config->use_lr_schedule && lr_scheduler) {
            step_lr_scheduler(lr_scheduler, training);
            float current_lr = get_learning_rate(lr_scheduler);
            printf("  Learning rate: %.6f\n", current_lr);
        }
        
        printf("  Train loss: %.4f\n", train_loss);
        
        // Validation
        float val_loss = INFINITY;
        if (config->use_validation && val_set && 
            (epoch + 1) % config->val_interval == 0) {
            printf("  Validating...\n");
            val_loss = evaluate_validation(training, val_set);
            printf("  Val loss: %.4f\n", val_loss);
            
            // Check early stopping
            if (config->use_early_stopping && early_stop) {
                if (check_early_stopping(early_stop, val_loss)) {
                    printf("\n✓ Early stopping triggered\n");
                    break;
                }
            }
        }
        
        // Record metrics
        if (config->use_metrics && metrics) {
            float current_lr = config->use_lr_schedule && lr_scheduler 
                ? get_learning_rate(lr_scheduler) 
                : config->learning_rate;
            record_metrics(metrics, epoch + 1, train_loss, val_loss, current_lr);
        }
        
        // Save checkpoint
        if (config->use_checkpoints && 
            (epoch + 1) % config->checkpoint_interval == 0) {
            char checkpoint_path[512];
            snprintf(checkpoint_path, sizeof(checkpoint_path), 
                    "%s/checkpoint_epoch_%d.ckpt", config->checkpoint_dir, epoch + 1);
            save_checkpoint(training, checkpoint_path);
        }
        
        printf("\n");
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    printf("════════════════════════════════════════════════════════════\n");
    printf("Training complete!\n");
    printf("  Total time: %.0f seconds (%.2f minutes)\n", elapsed, elapsed / 60.0);
    printf("  Epochs completed: %d\n", training->current_epoch + 1);
    printf("  Final loss: %.4f\n", training->current_loss);
    if (training->best_loss < INFINITY) {
        printf("  Best loss: %.4f\n", training->best_loss);
    }
    printf("\n");
    
    // Save final checkpoint
    if (config->use_checkpoints) {
        char final_path[512];
        snprintf(final_path, sizeof(final_path), "%s/final_checkpoint.ckpt", 
                config->checkpoint_dir);
        save_checkpoint(training, final_path);
        printf("✓ Final checkpoint saved\n");
    }
    
    // Save metrics
    if (config->use_metrics && metrics && config->metrics_path) {
        save_metrics_csv(metrics, config->metrics_path);
        printf("✓ Metrics saved\n");
    }
    
    // Cleanup
    if (val_set) free_validation_set(val_set);
    if (early_stop) free(early_stop);
    if (lr_scheduler) free(lr_scheduler);
    if (metrics) free_training_metrics(metrics);
    if (advanced_state) crystalline_advanced_free(advanced_state);
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║              TRAINING COMPLETE - SUCCESS!                 ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}

/**
 * Quick training function with sensible defaults
 */
int train_quick(CLLMTraining* training, int num_epochs) {
    CompleteTrainingConfig config = create_default_complete_config();
    config.num_epochs = num_epochs;
    return train_complete(training, &config);
}