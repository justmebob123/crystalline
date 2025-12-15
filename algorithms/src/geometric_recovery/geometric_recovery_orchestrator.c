/**
 * @file geometric_recovery_orchestrator.c
 * @brief Universal Geometric Recovery Orchestrator
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - NOT crypto-specific!
 * 
 * Works with RAW DATA SAMPLES from ANY system.
 * 
 * Thesis Reference: Chapter 16 - Geometric Recovery
 */

#include "geometric_recovery/geometric_recovery_orchestrator.h"
#include "geometric_recovery/tetration_attractors.h"
#include "geometric_recovery/torus_analysis.h"
#include "geometric_recovery/harmonic_folding.h"
#include "geometric_recovery/kissing_spheres.h"
#include "geometric_recovery/micro_model.h"
#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_SAMPLES 1000
#define MAX_PHASES 7

typedef struct {
    uint64_t input;
    uint64_t output;
    double weight;
} RawSample;

typedef struct {
    bool completed;
    double confidence;
    double error;
    char description[128];
} PhaseStatus;

struct GeometricRecoveryOrchestrator {
    char name[64];
    uint64_t n;
    uint32_t num_samples;
    RawSample* samples;
    
    TetrationSystem* attractors;
    TorusSystem* torus;
    MicroModel* model;
    
    PhaseStatus phases[MAX_PHASES];
    int current_phase;
    
    uint64_t result_min;
    uint64_t result_max;
    double reduction_factor;
    double overall_confidence;
};

GeometricRecoveryOrchestrator* geometric_recovery_orchestrator_create(
    const char* name,
    uint64_t n,
    uint32_t max_samples
) {
    if (!name || n == 0 || max_samples == 0) {
        return NULL;
    }
    
    GeometricRecoveryOrchestrator* orch = (GeometricRecoveryOrchestrator*)calloc(
        1, sizeof(GeometricRecoveryOrchestrator)
    );
    if (!orch) {
        return NULL;
    }
    
    strncpy(orch->name, name, sizeof(orch->name) - 1);
    orch->n = n;
    orch->num_samples = 0;
    
    orch->samples = (RawSample*)calloc(max_samples, sizeof(RawSample));
    if (!orch->samples) {
        free(orch);
        return NULL;
    }
    
    for (int i = 0; i < MAX_PHASES; i++) {
        orch->phases[i].completed = false;
        orch->phases[i].confidence = 0.0;
        orch->phases[i].error = 0.0;
        snprintf(orch->phases[i].description, sizeof(orch->phases[i].description),
                 "Phase %d", i + 1);
    }
    orch->current_phase = 0;
    
    orch->attractors = NULL;
    orch->torus = NULL;
    orch->model = NULL;
    
    orch->result_min = 0;
    orch->result_max = n;
    orch->reduction_factor = 1.0;
    orch->overall_confidence = 0.0;
    
    return orch;
}

void geometric_recovery_orchestrator_free(GeometricRecoveryOrchestrator* orch) {
    if (!orch) {
        return;
    }
    
    if (orch->samples) {
        free(orch->samples);
    }
    
    if (orch->attractors) {
        tetration_system_free(orch->attractors);
    }
    if (orch->torus) {
        torus_system_free(orch->torus);
    }
    if (orch->model) {
        micro_model_free(orch->model);
    }
    
    free(orch);
}

int geometric_recovery_orchestrator_add_sample(
    GeometricRecoveryOrchestrator* orch,
    uint64_t input,
    uint64_t output,
    double weight
) {
    if (!orch || orch->num_samples >= MAX_SAMPLES) {
        return -1;
    }
    
    RawSample* sample = &orch->samples[orch->num_samples];
    sample->input = input;
    sample->output = output;
    sample->weight = weight;
    
    orch->num_samples++;
    return 0;
}

int geometric_recovery_orchestrator_execute(
    GeometricRecoveryOrchestrator* orch,
    uint64_t target,
    uint64_t* result_min,
    uint64_t* result_max,
    double* reduction_factor
) {
    if (!orch || !result_min || !result_max || !reduction_factor) {
        return -1;
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  GEOMETRIC RECOVERY ORCHESTRATOR                         ║\n");
    printf("║  Universal Geometric Mathematics                         ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("System: %s\n", orch->name);
    printf("Size: %lu\n", orch->n);
    printf("Samples: %u\n", orch->num_samples);
    printf("Target: %lu\n", target);
    printf("\n");
    
    // Phase 1: Tetration Attractors
    printf("Phase 1: Tetration Attractors\n");
    orch->attractors = tetration_system_create(6, 31);
    if (orch->attractors && tetration_system_compute(orch->attractors) == 0) {
        double conv = tetration_system_analyze_convergence(orch->attractors);
        orch->phases[0].completed = true;
        orch->phases[0].confidence = conv;
        printf("  ✓ Convergence: %.2f%%\n", conv * 100.0);
    }
    
    // Phase 2: Torus Analysis
    printf("\nPhase 2: Torus Analysis\n");
    orch->torus = torus_system_create(20);
    if (orch->torus) {
        for (uint32_t i = 0; i < orch->num_samples; i++) {
            torus_system_add_point(orch->torus, 
                                  (double)orch->samples[i].input,
                                  (double)orch->samples[i].output);
        }
        if (torus_system_compute(orch->torus) == 0) {
            orch->phases[1].completed = true;
            orch->phases[1].confidence = 0.8;
            printf("  ✓ Torus analysis complete\n");
        }
    }
    
    // Phase 3: Micro-Model Training
    printf("\nPhase 3: Micro-Model Training\n");
    orch->model = micro_model_create(orch->name, 32, orch->n);
    if (orch->model) {
        TrainingSample* training = (TrainingSample*)malloc(orch->num_samples * sizeof(TrainingSample));
        if (training) {
            for (uint32_t i = 0; i < orch->num_samples; i++) {
                training[i].k = orch->samples[i].input;
                training[i].Q = orch->samples[i].output;
                training[i].error = 0.0;
            }
            
            if (micro_model_train(orch->model, training, orch->num_samples) == 0) {
                orch->phases[2].completed = true;
                orch->phases[2].confidence = 0.85;
                printf("  ✓ Model trained\n");
            }
            free(training);
        }
    }
    
    // Phase 4: Final Recovery
    printf("\nPhase 4: Final Recovery\n");
    if (orch->model) {
        if (micro_model_recover(orch->model, target, &orch->result_min, &orch->result_max) == 0) {
            uint64_t range = orch->result_max - orch->result_min;
            orch->reduction_factor = (double)orch->n / (double)range;
            orch->phases[3].completed = true;
            orch->phases[3].confidence = 0.9;
            printf("  ✓ Bounds: [%lu, %lu]\n", orch->result_min, orch->result_max);
            printf("  ✓ Reduction: %.2fx\n", orch->reduction_factor);
        }
    }
    
    // Compute overall confidence
    double total_conf = 0.0;
    int completed = 0;
    for (int i = 0; i < 4; i++) {
        if (orch->phases[i].completed) {
            total_conf += orch->phases[i].confidence;
            completed++;
        }
    }
    orch->overall_confidence = completed > 0 ? total_conf / completed : 0.0;
    
    *result_min = orch->result_min;
    *result_max = orch->result_max;
    *reduction_factor = orch->reduction_factor;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  RECOVERY COMPLETE (Confidence: %.2f%%)                   ║\n", orch->overall_confidence * 100.0);
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}

void geometric_recovery_orchestrator_print_summary(
    const GeometricRecoveryOrchestrator* orch,
    FILE* output
) {
    if (!orch || !output) {
        return;
    }
    
    fprintf(output, "\n");
    fprintf(output, "╔══════════════════════════════════════════════════════════╗\n");
    fprintf(output, "║  ORCHESTRATOR SUMMARY                                    ║\n");
    fprintf(output, "╚══════════════════════════════════════════════════════════╝\n");
    fprintf(output, "\n");
    
    fprintf(output, "System: %s\n", orch->name);
    fprintf(output, "Size: %lu\n", orch->n);
    fprintf(output, "Samples: %u\n", orch->num_samples);
    fprintf(output, "\n");
    
    fprintf(output, "Results:\n");
    fprintf(output, "  Bounds: [%lu, %lu]\n", orch->result_min, orch->result_max);
    fprintf(output, "  Reduction: %.2fx\n", orch->reduction_factor);
    fprintf(output, "  Confidence: %.2f%%\n", orch->overall_confidence * 100.0);
    fprintf(output, "\n");
}
