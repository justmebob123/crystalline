#ifndef CLLM_RECURSIVE_SPHERES_H
#define CLLM_RECURSIVE_SPHERES_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "cllm_training_threaded.h"

/**
 * Recursive Kissing Spheres Architecture
 * 
 * This implements an infinitely recursive, self-similar sphere hierarchy:
 * - Each sphere can spawn 12 child spheres
 * - Creates a fractal tree: 1 -> 13 -> 169 -> 2,197 -> 28,561 -> ...
 * - Scales to arbitrary depth limited only by system resources
 * - Designed for distributed computing and GPU acceleration
 * 
 * Mathematical Properties:
 * - Level 0: 1 control sphere
 * - Level 1: 1 + 12 = 13 spheres
 * - Level 2: 1 + 12 + 144 = 157 spheres
 * - Level n: sum(12^i) for i=0 to n = (12^(n+1) - 1) / 11
 * 
 * Memory Hierarchy:
 * - Each level shares read-only model weights
 * - Each sphere owns exclusive gradient segment
 * - Gradients accumulate up the tree
 * - Lock-free within each level
 */

// Maximum recursion depth (configurable at runtime)
#define MAX_RECURSION_DEPTH 10

// Sphere types
typedef enum {
    SPHERE_TYPE_ROOT,       // Top-level control sphere
    SPHERE_TYPE_CONTROL,    // Mid-level control sphere (has children)
    SPHERE_TYPE_WORKER,     // Leaf worker sphere (no children)
} SphereType;

// Sphere state
typedef enum {
    SPHERE_STATE_IDLE,
    SPHERE_STATE_INITIALIZING,
    SPHERE_STATE_WORKING,
    SPHERE_STATE_ACCUMULATING,
    SPHERE_STATE_COMPLETE,
    SPHERE_STATE_ERROR,
} SphereState;

// Forward declarations
typedef struct RecursiveSphere RecursiveSphere;
typedef struct SphereHierarchy SphereHierarchy;

// Individual sphere in the hierarchy
struct RecursiveSphere {
    // Identity
    uint32_t sphere_id;           // Unique ID across entire hierarchy
    uint32_t level;               // Depth in tree (0 = root)
    uint32_t index_at_level;      // Position among siblings
    SphereType type;
    SphereState state;
    
    // Hierarchy relationships
    RecursiveSphere *parent;      // Parent sphere (NULL for root)
    RecursiveSphere *children[12]; // Child spheres (NULL if leaf)
    uint32_t num_children;        // Actual number of children spawned
    
    // Work assignment
    uint32_t start_batch;         // First batch this sphere processes
    uint32_t end_batch;           // Last batch (exclusive)
    uint32_t batches_processed;   // Progress counter
    
    // Gradient segment ownership
    float *gradient_segment;      // Exclusive gradient memory
    uint32_t segment_size;        // Size in floats
    uint32_t segment_offset;      // Offset in global gradient array
    
    // Threading
    pthread_t thread;             // Thread handle (if active)
    bool thread_active;
    
    // Performance metrics
    uint64_t operations_completed;
    uint64_t cache_hits;
    uint64_t cache_misses;
    double time_spent_seconds;
    
    // Synchronization (minimal, lock-free where possible)
    pthread_mutex_t state_mutex;
    pthread_cond_t work_ready;
    pthread_cond_t work_complete;
};

// Complete sphere hierarchy
struct SphereHierarchy {
    // Configuration
    uint32_t max_depth;           // Maximum recursion depth
    uint32_t total_spheres;       // Total spheres in hierarchy
    uint32_t total_batches;       // Total batches to process
    
    // Root sphere
    RecursiveSphere *root;
    
    // Flat array for fast lookup (indexed by sphere_id)
    RecursiveSphere **sphere_array;
    uint32_t sphere_array_size;
    
    // Shared model (read-only across all spheres)
    CLLMTraining *shared_training_state;
    
    // Global gradient accumulation buffer
    float *global_gradients;
    uint32_t gradient_size;
    
    // Batch iterator (shared, thread-safe)
    CLLMBatchIterator *batch_iterator;
    
    // Performance tracking
    uint64_t total_operations;
    double total_time_seconds;
    
    // Synchronization
    pthread_mutex_t hierarchy_mutex;
    bool training_complete;
};

// API Functions

/**
 * Create a new sphere hierarchy
 * 
 * @param training_state Shared training state (read-only)
 * @param batch_iterator Batch iterator for work distribution
 * @param max_depth Maximum recursion depth (0 = single sphere, 1 = 13 spheres, etc.)
 * @param total_batches Total number of batches to process
 * @return Initialized hierarchy or NULL on error
 */
SphereHierarchy* sphere_hierarchy_create(
    CLLMTraining *training_state,
    CLLMBatchIterator *batch_iterator,
    uint32_t max_depth,
    uint32_t total_batches
);

/**
 * Destroy sphere hierarchy and free all resources
 */
void sphere_hierarchy_destroy(SphereHierarchy *hierarchy);

/**
 * Start training across the entire hierarchy
 * 
 * This spawns threads recursively and begins processing batches.
 * Returns when all work is complete.
 * 
 * @param hierarchy The sphere hierarchy
 * @return 0 on success, -1 on error
 */
int sphere_hierarchy_train(SphereHierarchy *hierarchy);

/**
 * Get total number of spheres at a given depth
 * 
 * @param depth Recursion depth (0 = root only)
 * @return Number of spheres: (12^(depth+1) - 1) / 11
 */
uint32_t sphere_hierarchy_count_at_depth(uint32_t depth);

/**
 * Get optimal depth for a given number of batches
 * 
 * This calculates the ideal recursion depth to maximize parallelism
 * without creating more spheres than batches.
 * 
 * @param total_batches Number of batches to process
 * @return Recommended depth
 */
uint32_t sphere_hierarchy_optimal_depth(uint32_t total_batches);

/**
 * Print hierarchy statistics
 */
void sphere_hierarchy_print_stats(SphereHierarchy *hierarchy);

/**
 * Recursively spawn child spheres for a given sphere
 * 
 * @param parent Parent sphere
 * @param hierarchy Hierarchy context
 * @param remaining_depth How many more levels to spawn
 * @return 0 on success, -1 on error
 */
int sphere_spawn_children(
    RecursiveSphere *parent,
    SphereHierarchy *hierarchy,
    uint32_t remaining_depth
);

/**
 * Accumulate gradients from children up to parent
 * 
 * This is called after all children complete their work.
 * Uses SIMD operations for efficiency.
 * 
 * @param parent Parent sphere
 * @return 0 on success, -1 on error
 */
int sphere_accumulate_gradients(RecursiveSphere *parent);

#endif // CLLM_RECURSIVE_SPHERES_H