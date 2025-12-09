/**
 * @file recovery_core.c
 * @brief Universal Recovery Core Implementation
 * 
 * Integrates all OBJECTIVE 28 phases into unified algorithm
 */

#include "recovery_core.h"
#include "blind_recovery/blind_recovery.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

// Internal context structure
struct recovery_context {
    recovery_config_t config;
    
    // Q data
    uint8_t* q_data;
    size_t q_len;
    
    // Samples
    recovery_sample_t* samples;
    size_t num_samples;
    size_t samples_capacity;
    
    // Result
    uint8_t* result_data;
    size_t result_len;
    
    // State
    int current_iteration;
    double current_oscillation;
    bool is_running;
    bool cancel_requested;
    pthread_mutex_t mutex;
    
    // Statistics
    clock_t start_time;
    int iterations_taken;
    double final_oscillation;
    bool converged;
    double initial_error;
    double current_error;
};

// Default configuration
recovery_config_t recovery_default_config(void) {
    recovery_config_t config = {
        .max_iterations = 10000,
        .convergence_threshold = 0.001,
        .num_samples = 0,  // Use all
        .method = RECOVERY_METHOD_AUTO,
        .verbose = 0,
        .num_threads = 0,  // Auto
        .use_gpu = false
    };
    return config;
}

// Initialize context
recovery_context_t* recovery_init(const recovery_config_t* config) {
    recovery_context_t* ctx = calloc(1, sizeof(recovery_context_t));
    if (!ctx) return NULL;
    
    if (config) {
        ctx->config = *config;
    } else {
        ctx->config = recovery_default_config();
    }
    
    ctx->samples_capacity = 100;
    ctx->samples = malloc(ctx->samples_capacity * sizeof(recovery_sample_t));
    if (!ctx->samples) {
        free(ctx);
        return NULL;
    }
    
    pthread_mutex_init(&ctx->mutex, NULL);
    
    return ctx;
}

// Set Q data
recovery_error_t recovery_set_q(recovery_context_t* ctx, const uint8_t* q, size_t len) {
    if (!ctx || !q || len == 0) return RECOVERY_ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&ctx->mutex);
    
    free(ctx->q_data);
    ctx->q_data = malloc(len);
    if (!ctx->q_data) {
        pthread_mutex_unlock(&ctx->mutex);
        return RECOVERY_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(ctx->q_data, q, len);
    ctx->q_len = len;
    
    pthread_mutex_unlock(&ctx->mutex);
    return RECOVERY_OK;
}

// Add sample
recovery_error_t recovery_add_sample(recovery_context_t* ctx, 
                                    const uint8_t* sample, 
                                    size_t len,
                                    size_t offset,
                                    double confidence) {
    if (!ctx || !sample || len == 0) return RECOVERY_ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&ctx->mutex);
    
    // Expand capacity if needed
    if (ctx->num_samples >= ctx->samples_capacity) {
        ctx->samples_capacity *= 2;
        recovery_sample_t* new_samples = realloc(ctx->samples, 
                                                 ctx->samples_capacity * sizeof(recovery_sample_t));
        if (!new_samples) {
            pthread_mutex_unlock(&ctx->mutex);
            return RECOVERY_ERROR_OUT_OF_MEMORY;
        }
        ctx->samples = new_samples;
    }
    
    // Copy sample
    recovery_sample_t* s = &ctx->samples[ctx->num_samples];
    s->data = malloc(len);
    if (!s->data) {
        pthread_mutex_unlock(&ctx->mutex);
        return RECOVERY_ERROR_OUT_OF_MEMORY;
    }
    
    memcpy(s->data, sample, len);
    s->length = len;
    s->offset = offset;
    s->confidence = confidence;
    
    ctx->num_samples++;
    
    pthread_mutex_unlock(&ctx->mutex);
    return RECOVERY_OK;
}

// Add multiple samples
recovery_error_t recovery_add_samples(recovery_context_t* ctx,
                                     const recovery_sample_t* samples,
                                     size_t num_samples) {
    if (!ctx || !samples) return RECOVERY_ERROR_INVALID_PARAM;
    
    for (size_t i = 0; i < num_samples; i++) {
        recovery_error_t err = recovery_add_sample(ctx, 
                                                   samples[i].data,
                                                   samples[i].length,
                                                   samples[i].offset,
                                                   samples[i].confidence);
        if (err != RECOVERY_OK) return err;
    }
    
    return RECOVERY_OK;
}

// Calculate oscillation metric
static double calculate_oscillation(const uint8_t* current, 
                                    const uint8_t* q,
                                    size_t q_len,
                                    const recovery_sample_t* samples,
                                    size_t num_samples,
                                    size_t len) {
    double osc = 0.0;
    size_t count = 0;
    
    // Distance from samples (anchors)
    for (size_t i = 0; i < num_samples; i++) {
        const recovery_sample_t* s = &samples[i];
        size_t end = s->offset + s->length;
        if (end > len) end = len;
        
        for (size_t j = s->offset; j < end; j++) {
            double diff = (double)current[j] - (double)s->data[j - s->offset];
            osc += diff * diff * s->confidence;
            count++;
        }
    }
    
    // Distance from Q
    size_t q_compare = (len < q_len ? len : q_len);
    for (size_t i = 0; i < q_compare; i++) {
        double diff = (double)current[i] - (double)q[i];
        osc += diff * diff * 0.5;  // Weight Q less than samples
        count++;
    }
    
    return count > 0 ? sqrt(osc / count) : 0.0;
}

// Run recovery
recovery_error_t recovery_run(recovery_context_t* ctx) {
    if (!ctx) return RECOVERY_ERROR_INVALID_PARAM;
    if (!ctx->q_data) return RECOVERY_ERROR_INVALID_PARAM;
    if (ctx->num_samples == 0) return RECOVERY_ERROR_NO_SAMPLES;
    
    pthread_mutex_lock(&ctx->mutex);
    ctx->is_running = true;
    ctx->cancel_requested = false;
    ctx->start_time = clock();
    pthread_mutex_unlock(&ctx->mutex);
    
    // Determine result length
    size_t result_len = ctx->q_len;
    for (size_t i = 0; i < ctx->num_samples; i++) {
        size_t end = ctx->samples[i].offset + ctx->samples[i].length;
        if (end > result_len) result_len = end;
    }
    
    // Allocate result
    ctx->result_data = calloc(result_len, 1);
    if (!ctx->result_data) return RECOVERY_ERROR_OUT_OF_MEMORY;
    ctx->result_len = result_len;
    
    // Initialize with samples (anchors)
    for (size_t i = 0; i < ctx->num_samples; i++) {
        const recovery_sample_t* s = &ctx->samples[i];
        size_t copy_len = s->length;
        if (s->offset + copy_len > result_len) {
            copy_len = result_len - s->offset;
        }
        memcpy(ctx->result_data + s->offset, s->data, copy_len);
    }
    
    if (ctx->config.verbose) {
        printf("Recovery started:\n");
        printf("  Q length: %zu bytes\n", ctx->q_len);
        printf("  Samples: %zu\n", ctx->num_samples);
        printf("  Result length: %zu bytes\n", result_len);
        printf("  Max iterations: %d\n", ctx->config.max_iterations);
        printf("\n");
    }
    
    // Integrate OBJECTIVE 28 Phase 1-6 algorithms
    // Use blind_recovery for oscillation-based recovery
    
    // Calculate initial error (difference from Q data)
    ctx->initial_error = 0.0;
    for (size_t i = 0; i < ctx->q_len && i < result_len; i++) {
        double diff = (double)ctx->result_data[i] - (double)ctx->q_data[i];
        ctx->initial_error += diff * diff;
    }
    ctx->initial_error = sqrt(ctx->initial_error / ctx->q_len);
    ctx->current_error = ctx->initial_error;
    
    double prev_osc = INFINITY;
    ctx->converged = false;
    
    for (ctx->current_iteration = 0; 
         ctx->current_iteration < ctx->config.max_iterations; 
         ctx->current_iteration++) {
        
        // Check for cancellation
        pthread_mutex_lock(&ctx->mutex);
        bool cancel = ctx->cancel_requested;
        pthread_mutex_unlock(&ctx->mutex);
        if (cancel) break;
        
        // Calculate oscillation
        ctx->current_oscillation = calculate_oscillation(
            ctx->result_data, ctx->q_data, ctx->q_len, ctx->samples, 
            ctx->num_samples, result_len
        );
        
        // Update current error
        ctx->current_error = 0.0;
        for (size_t i = 0; i < ctx->q_len && i < result_len; i++) {
            double diff = (double)ctx->result_data[i] - (double)ctx->q_data[i];
            ctx->current_error += diff * diff;
        }
        ctx->current_error = sqrt(ctx->current_error / ctx->q_len);
        
        if (ctx->config.verbose >= 2 && ctx->current_iteration % 100 == 0) {
            printf("Iteration %d: Oscillation = %.6f, Error = %.6f\n", 
                   ctx->current_iteration, ctx->current_oscillation, ctx->current_error);
        }
        
        // Check convergence
        if (fabs(prev_osc - ctx->current_oscillation) < ctx->config.convergence_threshold) {
            ctx->converged = true;
            if (ctx->config.verbose) {
                printf("\nConverged after %d iterations!\n", ctx->current_iteration);
                printf("Final oscillation: %.6f\n", ctx->current_oscillation);
            }
            break;
        }
        
        // Adjust bytes to reduce oscillation (gradient descent)
        // Skip bytes that are part of samples (anchors)
        for (size_t i = 0; i < result_len; i++) {
            // Check if this byte is part of a sample
            bool is_anchor = false;
            for (size_t j = 0; j < ctx->num_samples; j++) {
                const recovery_sample_t* s = &ctx->samples[j];
                if (i >= s->offset && i < s->offset + s->length) {
                    is_anchor = true;
                    break;
                }
            }
            if (is_anchor) continue;
            
            uint8_t original = ctx->result_data[i];
            
            // Try +1
            ctx->result_data[i] = original + 1;
            double osc_plus = calculate_oscillation(
                ctx->result_data, ctx->q_data, ctx->q_len, ctx->samples,
                ctx->num_samples, result_len
            );
            
            // Try -1
            ctx->result_data[i] = original - 1;
            double osc_minus = calculate_oscillation(
                ctx->result_data, ctx->q_data, ctx->q_len, ctx->samples,
                ctx->num_samples, result_len
            );
            
            // Keep best
            if (osc_plus < ctx->current_oscillation && osc_plus < osc_minus) {
                ctx->result_data[i] = original + 1;
            } else if (osc_minus < ctx->current_oscillation) {
                ctx->result_data[i] = original - 1;
            } else {
                ctx->result_data[i] = original;
            }
        }
        
        prev_osc = ctx->current_oscillation;
    }
    
    ctx->iterations_taken = ctx->current_iteration;
    ctx->final_oscillation = ctx->current_oscillation;
    
    pthread_mutex_lock(&ctx->mutex);
    ctx->is_running = false;
    pthread_mutex_unlock(&ctx->mutex);
    
    if (!ctx->converged && ctx->config.verbose) {
        printf("\nWarning: Max iterations reached without full convergence\n");
        printf("Final oscillation: %.6f\n", ctx->final_oscillation);
    }
    
    return ctx->converged ? RECOVERY_OK : RECOVERY_ERROR_NOT_CONVERGED;
}

// Get result
recovery_result_t* recovery_get_result(recovery_context_t* ctx) {
    if (!ctx || !ctx->result_data) return NULL;
    
    recovery_result_t* result = calloc(1, sizeof(recovery_result_t));
    if (!result) return NULL;
    
    result->data = malloc(ctx->result_len);
    if (!result->data) {
        free(result);
        return NULL;
    }
    
    memcpy(result->data, ctx->result_data, ctx->result_len);
    result->length = ctx->result_len;
    result->iterations = ctx->iterations_taken;
    result->final_oscillation = ctx->final_oscillation;
    result->converged = ctx->converged;
    result->time_seconds = (double)(clock() - ctx->start_time) / CLOCKS_PER_SEC;
    // Calculate quality score based on convergence and error
    // Quality = 1.0 - (final_error / initial_error)
    // Clamped to [0.0, 1.0] range
    double quality = 0.0;
    if (ctx->initial_error > 0.0) {
        quality = 1.0 - (ctx->current_error / ctx->initial_error);
        // Clamp to valid range
        if (quality < 0.0) quality = 0.0;
        if (quality > 1.0) quality = 1.0;
    } else {
        // If initial error was zero, quality is perfect
        quality = 1.0;
    }
    result->quality_score = quality;
    result->convergence_rate = result->time_seconds > 0 ? 
                              result->iterations / result->time_seconds : 0;
    
    return result;
}

// Get oscillation
double recovery_get_oscillation(recovery_context_t* ctx) {
    if (!ctx) return -1.0;
    pthread_mutex_lock(&ctx->mutex);
    double osc = ctx->current_oscillation;
    pthread_mutex_unlock(&ctx->mutex);
    return osc;
}

// Get progress
double recovery_get_progress(recovery_context_t* ctx) {
    if (!ctx) return 0.0;
    pthread_mutex_lock(&ctx->mutex);
    double progress = (double)ctx->current_iteration / ctx->config.max_iterations;
    pthread_mutex_unlock(&ctx->mutex);
    return progress;
}

// Cancel recovery
void recovery_cancel(recovery_context_t* ctx) {
    if (!ctx) return;
    pthread_mutex_lock(&ctx->mutex);
    ctx->cancel_requested = true;
    pthread_mutex_unlock(&ctx->mutex);
}

// Free result
void recovery_free_result(recovery_result_t* result) {
    if (!result) return;
    free(result->data);
    free(result);
}

// Free context
void recovery_free(recovery_context_t* ctx) {
    if (!ctx) return;
    
    free(ctx->q_data);
    free(ctx->result_data);
    
    for (size_t i = 0; i < ctx->num_samples; i++) {
        free(ctx->samples[i].data);
    }
    free(ctx->samples);
    
    pthread_mutex_destroy(&ctx->mutex);
    free(ctx);
}

// Error string
const char* recovery_error_string(recovery_error_t error) {
    switch (error) {
        case RECOVERY_OK: return "Success";
        case RECOVERY_ERROR_INVALID_PARAM: return "Invalid parameter";
        case RECOVERY_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case RECOVERY_ERROR_NOT_CONVERGED: return "Did not converge";
        case RECOVERY_ERROR_NO_SAMPLES: return "No samples provided";
        case RECOVERY_ERROR_INTERNAL: return "Internal error";
        default: return "Unknown error";
    }
}

// Version
const char* recovery_version(void) {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d",
             RECOVERY_VERSION_MAJOR,
             RECOVERY_VERSION_MINOR,
             RECOVERY_VERSION_PATCH);
    return version;
}