/**
 * @file geometric_recovery_orchestrator.h
 * @brief Universal Geometric Recovery Orchestrator
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - NOT crypto-specific!
 * 
 * This orchestrator coordinates geometric recovery algorithms to work with
 * RAW DATA SAMPLES from ANY system:
 * - Cryptographic systems (ECDLP, RSA, DH, etc.)
 * - Non-cryptographic systems (embeddings, symbol maps, etc.)
 * - Any system with geometric structure
 * 
 * Key Principle: Works with SAMPLES, not specific crypto structures.
 * 
 * 7-Phase Pipeline:
 * 1. Triangulation - Geometric positioning
 * 2. Torus Analysis - Oscillation patterns
 * 3. Harmonic Folding - Frequency decomposition
 * 4. Kissing Spheres - Optimal packing
 * 5. Micro-Model Training - Learn from samples
 * 6. Confidence Scoring - Quality metrics
 * 7. Final Recovery - Bounded result
 * 
 * Thesis Reference: Chapter 16 - Geometric Recovery
 */

#ifndef ALGORITHMS_GEOMETRIC_RECOVERY_ORCHESTRATOR_H
#define ALGORITHMS_GEOMETRIC_RECOVERY_ORCHESTRATOR_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// OPAQUE TYPE
// ============================================================================

/**
 * Geometric Recovery Orchestrator (opaque)
 */
typedef struct GeometricRecoveryOrchestrator GeometricRecoveryOrchestrator;

// ============================================================================
// ORCHESTRATOR CREATION & INITIALIZATION
// ============================================================================

/**
 * Create a new geometric recovery orchestrator
 * 
 * UNIVERSAL: Works with ANY system that has geometric structure.
 * 
 * @param name System name (for logging/identification)
 * @param n System size (e.g., curve order, embedding dimension, etc.)
 * @param max_samples Maximum number of training samples
 * @return Initialized orchestrator, or NULL on error
 */
GeometricRecoveryOrchestrator* geometric_recovery_orchestrator_create(
    const char* name,
    uint64_t n,
    uint32_t max_samples
);

/**
 * Free geometric recovery orchestrator
 * 
 * @param orch Orchestrator to free
 */
void geometric_recovery_orchestrator_free(GeometricRecoveryOrchestrator* orch);

// ============================================================================
// SAMPLE MANAGEMENT - UNIVERSAL RAW DATA
// ============================================================================

/**
 * Add a raw data sample
 * 
 * UNIVERSAL: Works with ANY input/output pair:
 * - Crypto: (k, Q) where Q = k·G
 * - Embeddings: (index, embedding_value)
 * - Symbol maps: (symbol_id, mapped_value)
 * - Any system: (input, output)
 * 
 * @param orch Orchestrator
 * @param input Input value (e.g., k, index, symbol_id)
 * @param output Output value (e.g., Q, embedding, mapped_value)
 * @param weight Sample weight (importance, default 1.0)
 * @return 0 on success, -1 on error
 */
int geometric_recovery_orchestrator_add_sample(
    GeometricRecoveryOrchestrator* orch,
    uint64_t input,
    uint64_t output,
    double weight
);

// ============================================================================
// ORCHESTRATOR EXECUTION
// ============================================================================

/**
 * Execute the complete 7-phase geometric recovery pipeline
 * 
 * UNIVERSAL: Recovers bounds for ANY target value in ANY system.
 * 
 * Pipeline:
 * 1. Triangulation - Geometric positioning using tetration attractors
 * 2. Torus Analysis - Detect oscillation patterns in samples
 * 3. Harmonic Folding - Frequency decomposition (12 harmonics)
 * 4. Kissing Spheres - Optimal packing (12 neighbors)
 * 5. Micro-Model Training - Learn input→output mapping
 * 6. Confidence Scoring - Compute overall quality metrics
 * 7. Final Recovery - Predict bounds [min, max] for target
 * 
 * @param orch Orchestrator (must have samples added)
 * @param target Target output value to recover input for
 * @param result_min Output: minimum bound for input
 * @param result_max Output: maximum bound for input
 * @param reduction_factor Output: search space reduction (n / range)
 * @return 0 on success, -1 on error
 */
int geometric_recovery_orchestrator_execute(
    GeometricRecoveryOrchestrator* orch,
    uint64_t target,
    uint64_t* result_min,
    uint64_t* result_max,
    double* reduction_factor
);

// ============================================================================
// STATISTICS & REPORTING
// ============================================================================

/**
 * Print orchestrator summary
 * 
 * Prints comprehensive summary including:
 * - System configuration
 * - Phase completion status
 * - Results and metrics
 * 
 * @param orch Orchestrator
 * @param output Output file (e.g., stdout)
 */
void geometric_recovery_orchestrator_print_summary(
    const GeometricRecoveryOrchestrator* orch,
    FILE* output
);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_GEOMETRIC_RECOVERY_ORCHESTRATOR_H */