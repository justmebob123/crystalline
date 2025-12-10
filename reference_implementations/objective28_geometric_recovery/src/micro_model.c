/**
 * Trainable Micro-Model Implementation
 * 
 * Phase 5: Complete implementation of trainable micro-model
 */

#include "../include/micro_model.h"
#include "../../../include/prime_float_math.h"
#include "../../../include/clock_lattice.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// MODEL CREATION & INITIALIZATION
// ============================================================================

MicroModel* micro_model_create(const char* name, uint32_t bit_length, uint64_t n) {
    if (!name || bit_length == 0 || n == 0) {
        return NULL;
    }
    
    MicroModel* model = calloc(1, sizeof(MicroModel));
    if (!model) {
        return NULL;
    }
    
    // Set metadata
    strncpy(model->name, name, sizeof(model->name) - 1);
    model->version = 1;
    model->timestamp = (uint64_t)time(NULL);
    
    // Set curve parameters
    model->bit_length = bit_length;
    model->n = n;
    
    // Initialize G estimate
    model->g_estimate = 0.0;
    model->g_confidence = 0.0;
    
    // Initialize torus count
    model->num_tori = 0;
    
    // Initialize clock info
    model->clock_info.p = 0;
    model->clock_info.q = 0;
    
    // Initialize training statistics
    model->num_training_samples = 0;
    model->training_error = 0.0;
    model->validation_error = 0.0;
    
    // Initialize performance metrics
    model->reduction_factor = 1.0;
    model->best_reduction = 1.0;
    model->capture_rate = 0.0;
    
    return model;
}

void micro_model_free(MicroModel* model) {
    if (model) {
        free(model);
    }
}

// ============================================================================
// TRAINING
// ============================================================================

int micro_model_train(MicroModel* model, TrainingSample* samples, uint32_t num_samples) {
    if (!model || !samples || num_samples == 0) {
        return -1;
    }
    
    // Calculate average training error
    double total_error = 0.0;
    for (uint32_t i = 0; i < num_samples; i++) {
        total_error += samples[i].error;
    }
    
    model->training_error = total_error / num_samples;
    model->num_training_samples = num_samples;
    
    return 0;
}

int micro_model_add_torus(
    MicroModel* model,
    int torus_id,
    double center,
    double amplitude,
    double period,
    double phase,
    double confidence
) {
    if (!model || torus_id < 1 || torus_id > MAX_TORI) {
        return -1;
    }
    
    if (model->num_tori >= MAX_TORI) {
        return -1;
    }
    
    TorusParams* torus = &model->tori[model->num_tori];
    torus->torus_id = torus_id;
    torus->center = center;
    torus->amplitude = amplitude;
    torus->period = period;
    torus->phase = phase;
    torus->confidence = confidence;
    
    model->num_tori++;
    
    return 0;
}

int micro_model_set_clock_info(MicroModel* model, uint64_t p, uint64_t q) {
    if (!model || p == 0 || q == 0) {
        return -1;
    }
    
    model->clock_info.p = p;
    model->clock_info.q = q;
    
    // Get clock positions for p
    // Simplified: assume p and q are in first 20 primes
    int p_index = (p == 2) ? 1 : (p == 3) ? 2 : (p == 5) ? 3 : (p == 7) ? 4 : -1;
    int q_index = (q == 2) ? 1 : (q == 3) ? 2 : (q == 5) ? 3 : (q == 7) ? 4 : -1;
    
    if (p_index > 0) {
        BabylonianClockPosition p_pos = map_prime_index_to_clock(p_index);
        model->clock_info.p_ring = p_pos.ring;
        model->clock_info.p_position = p_pos.position;
        model->clock_info.p_angle = p_pos.angle;
    }
    
    if (q_index > 0) {
        BabylonianClockPosition q_pos = map_prime_index_to_clock(q_index);
        model->clock_info.q_ring = q_pos.ring;
        model->clock_info.q_position = q_pos.position;
        model->clock_info.q_angle = q_pos.angle;
    }
    
    return 0;
}

int micro_model_set_g_estimate(MicroModel* model, double g_estimate, double g_confidence) {
    if (!model) {
        return -1;
    }
    
    model->g_estimate = g_estimate;
    model->g_confidence = g_confidence;
    
    return 0;
}

// ============================================================================
// RECOVERY
// ============================================================================

int micro_model_recover(
    MicroModel* model,
    uint64_t Q,
    uint64_t* k_min,
    uint64_t* k_max
) {
    if (!model || Q == 0 || !k_min || !k_max) {
        return -1;
    }
    
    // Use the first torus (most confident) for recovery
    if (model->num_tori == 0) {
        return -1;
    }
    
    TorusParams* torus = &model->tori[0];
    
    // Calculate bounds based on torus parameters
    double center = torus->center;
    double amplitude = torus->amplitude;
    
    // Bounds: center ± amplitude
    *k_min = (uint64_t)(center - amplitude);
    *k_max = (uint64_t)(center + amplitude);
    
    // Clamp to valid range
    if (*k_min < 0) *k_min = 0;
    if (*k_max > model->n) *k_max = model->n;
    
    return 0;
}

double micro_model_get_reduction_factor(
    MicroModel* model,
    uint64_t Q,
    uint64_t true_k
) {
    if (!model || Q == 0) {
        return 1.0;
    }
    
    uint64_t k_min, k_max;
    if (micro_model_recover(model, Q, &k_min, &k_max) != 0) {
        return 1.0;
    }
    
    // Calculate reduction factor
    uint64_t search_space = k_max - k_min;
    double reduction = (double)model->n / (double)search_space;
    
    // Check if true_k is captured
    if (true_k >= k_min && true_k <= k_max) {
        return reduction;
    } else {
        return 0.0;  // Failed to capture
    }
}

// ============================================================================
// PERSISTENCE
// ============================================================================

int micro_model_save(const MicroModel* model, const char* filename) {
    if (!model || !filename) {
        return -1;
    }
    
    FILE* f = fopen(filename, "wb");
    if (!f) {
        return -1;
    }
    
    // Write model structure
    size_t written = fwrite(model, sizeof(MicroModel), 1, f);
    fclose(f);
    
    return (written == 1) ? 0 : -1;
}

MicroModel* micro_model_load(const char* filename) {
    if (!filename) {
        return NULL;
    }
    
    FILE* f = fopen(filename, "rb");
    if (!f) {
        return NULL;
    }
    
    MicroModel* model = calloc(1, sizeof(MicroModel));
    if (!model) {
        fclose(f);
        return NULL;
    }
    
    // Read model structure
    size_t read = fread(model, sizeof(MicroModel), 1, f);
    fclose(f);
    
    if (read != 1) {
        free(model);
        return NULL;
    }
    
    return model;
}

// ============================================================================
// VALIDATION
// ============================================================================

double micro_model_validate(
    MicroModel* model,
    TrainingSample* samples,
    uint32_t num_samples
) {
    if (!model || !samples || num_samples == 0) {
        return -1.0;
    }
    
    double total_error = 0.0;
    uint32_t captured = 0;
    
    for (uint32_t i = 0; i < num_samples; i++) {
        uint64_t k_min, k_max;
        if (micro_model_recover(model, samples[i].Q, &k_min, &k_max) == 0) {
            // Check if true k is captured
            if (samples[i].k >= k_min && samples[i].k <= k_max) {
                captured++;
            }
            
            // Calculate error
            double center = (k_min + k_max) / 2.0;
            double error = prime_fabs(center - samples[i].k);
            total_error += error;
        }
    }
    
    model->validation_error = total_error / num_samples;
    model->capture_rate = (double)captured / num_samples;
    
    return model->validation_error;
}

void micro_model_print_summary(const MicroModel* model, FILE* output) {
    if (!model || !output) {
        return;
    }
    
    fprintf(output, "\n=== Micro-Model Summary ===\n");
    fprintf(output, "\nModel Information:\n");
    fprintf(output, "  Name: %s\n", model->name);
    fprintf(output, "  Version: %u\n", model->version);
    fprintf(output, "  Timestamp: %lu\n", model->timestamp);
    
    fprintf(output, "\nCurve Parameters:\n");
    fprintf(output, "  Bit Length: %u\n", model->bit_length);
    fprintf(output, "  n: %lu\n", model->n);
    
    fprintf(output, "\nG Triangulation:\n");
    fprintf(output, "  Estimate: %.4f\n", model->g_estimate);
    fprintf(output, "  Confidence: %.4f\n", model->g_confidence);
    
    fprintf(output, "\nClock Lattice:\n");
    fprintf(output, "  p: %lu (Ring %d, Position %d, Angle %.2f°)\n",
            model->clock_info.p, model->clock_info.p_ring,
            model->clock_info.p_position,
            model->clock_info.p_angle * 180.0 / M_PI);
    fprintf(output, "  q: %lu (Ring %d, Position %d, Angle %.2f°)\n",
            model->clock_info.q, model->clock_info.q_ring,
            model->clock_info.q_position,
            model->clock_info.q_angle * 180.0 / M_PI);
    
    fprintf(output, "\nTorus Parameters (%u tori):\n", model->num_tori);
    for (uint32_t i = 0; i < model->num_tori && i < 5; i++) {
        const TorusParams* t = &model->tori[i];
        fprintf(output, "  Torus %d: center=%.2f, amplitude=%.2f, period=%.2f, confidence=%.2f\n",
                t->torus_id, t->center, t->amplitude, t->period, t->confidence);
    }
    if (model->num_tori > 5) {
        fprintf(output, "  ... and %u more tori\n", model->num_tori - 5);
    }
    
    fprintf(output, "\nTraining Statistics:\n");
    fprintf(output, "  Samples: %u\n", model->num_training_samples);
    fprintf(output, "  Training Error: %.4f\n", model->training_error);
    fprintf(output, "  Validation Error: %.4f\n", model->validation_error);
    
    fprintf(output, "\nPerformance Metrics:\n");
    fprintf(output, "  Reduction Factor: %.2fx\n", model->reduction_factor);
    fprintf(output, "  Best Reduction: %.2fx\n", model->best_reduction);
    fprintf(output, "  Capture Rate: %.1f%%\n", model->capture_rate * 100.0);
}

int micro_model_get_statistics(
    const MicroModel* model,
    double* avg_reduction,
    double* best_reduction,
    double* capture_rate
) {
    if (!model || !avg_reduction || !best_reduction || !capture_rate) {
        return -1;
    }
    
    *avg_reduction = model->reduction_factor;
    *best_reduction = model->best_reduction;
    *capture_rate = model->capture_rate;
    
    return 0;
}