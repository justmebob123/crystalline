/**
 * @file cllm_88d_integration.h
 * @brief Integration layer between CLLM and 88D threading system
 * 
 * This file provides the bridge between CLLM's training/inference systems
 * and the algorithms library's 88D threading infrastructure.
 * 
 * Key Features:
 * - Thread pool management (1-96 cores adaptive)
 * - Work distribution across 88 logical threads
 * - Gradient accumulation primitives
 * - Statistics collection
 * 
 * Architecture:
 * - 88 logical threads (8 layers × 11 threads per layer)
 * - N physical threads (N = available cores, 1-96)
 * - Work stealing for load balancing
 * - Barrier synchronization
 */

#ifndef CLLM_88D_INTEGRATION_H
#define CLLM_88D_INTEGRATION_H

#include "../../algorithms/include/hierarchical_threading.h"
#include "../../algorithms/include/adaptive_threading.h"
#include "cllm.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TYPES AND STRUCTURES
// ============================================================================

/**
 * @brief 88D Thread Pool for CLLM
 * 
 * Manages the 88D threading system for CLLM operations.
 * Wraps the algorithms library's HierarchicalThreadPool.
 */
typedef struct {
    HierarchicalThreadPool* pool;      ///< Underlying 88D thread pool
    CLLMModel* model;                  ///< Associated CLLM model
    int num_physical_threads;          ///< Number of physical OS threads
    int num_logical_threads;           ///< Number of logical threads (88)
    bool adaptive_mode;                ///< Adaptive threading enabled
    bool initialized;                  ///< Initialization status
} CLLM88DThreadPool;

/**
 * @brief Operation types for 88D work distribution
 */
typedef enum {
    CLLM_88D_OP_EMBEDDINGS_LOOKUP,     ///< Parallel embeddings lookup
    CLLM_88D_OP_POSITIONAL_ENCODING,   ///< Parallel positional encoding
    CLLM_88D_OP_LAYER_NORM_FORWARD,    ///< Parallel layer norm forward
    CLLM_88D_OP_LAYER_NORM_BACKWARD,   ///< Parallel layer norm backward
    CLLM_88D_OP_ATTENTION_FORWARD,     ///< Parallel attention forward
    CLLM_88D_OP_ATTENTION_BACKWARD,    ///< Parallel attention backward
    CLLM_88D_OP_FEEDFORWARD_FORWARD,   ///< Parallel feedforward forward
    CLLM_88D_OP_FEEDFORWARD_BACKWARD,  ///< Parallel feedforward backward
    CLLM_88D_OP_GRADIENT_ACCUMULATE,   ///< Parallel gradient accumulation
    CLLM_88D_OP_CUSTOM                 ///< Custom operation
} CLLM88DOperationType;

/**
 * @brief Work item for 88D thread distribution
 * 
 * Represents a unit of work to be distributed across 88D threads.
 */
typedef struct {
    CLLM88DOperationType operation_type; ///< Type of operation
    void* input_data;                    ///< Input data pointer
    void* output_data;                   ///< Output data pointer
    void* aux_data;                      ///< Auxiliary data (e.g., weights)
    size_t start_idx;                    ///< Start index for this work item
    size_t end_idx;                      ///< End index for this work item
    size_t total_size;                   ///< Total size of work
    int thread_id;                       ///< Assigned thread ID (0-87)
    void* user_data;                     ///< User-defined data
} CLLM88DWorkItem;

/**
 * @brief Work function signature
 * 
 * User-defined function to execute on each thread.
 * 
 * @param work_item Work item to process
 * @return 0 on success, non-zero on error
 */
typedef int (*CLLM88DWorkFunction)(CLLM88DWorkItem* work_item);

/**
 * @brief Statistics for 88D thread pool
 */
typedef struct {
    uint64_t total_operations;         ///< Total operations executed
    uint64_t total_work_items;         ///< Total work items processed
    double avg_execution_time_ms;      ///< Average execution time (ms)
    double total_execution_time_ms;    ///< Total execution time (ms)
    int active_threads;                ///< Currently active threads
    int idle_threads;                  ///< Currently idle threads
    double load_balance_efficiency;    ///< Load balance efficiency (0-1)
} CLLM88DStatistics;

// ============================================================================
// INITIALIZATION AND CLEANUP
// ============================================================================

/**
 * @brief Initialize 88D threading for CLLM
 * 
 * Creates a 88D thread pool with adaptive threading.
 * 
 * @param model CLLM model to associate with thread pool
 * @param num_threads Number of physical threads (0 = auto-detect)
 * @return Pointer to initialized thread pool, NULL on error
 * 
 * @note If num_threads is 0, the system will auto-detect available cores
 * @note The pool creates 88 logical threads regardless of physical threads
 */
CLLM88DThreadPool* cllm_88d_init(CLLMModel* model, int num_threads);

/**
 * @brief Cleanup 88D threading
 * 
 * Destroys the thread pool and frees all resources.
 * 
 * @param pool Thread pool to cleanup
 */
void cllm_88d_cleanup(CLLM88DThreadPool* pool);

// ============================================================================
// WORK DISTRIBUTION
// ============================================================================

/**
 * @brief Distribute work across 88D threads
 * 
 * Distributes work items across 88 logical threads.
 * Each thread processes its assigned work items in parallel.
 * 
 * @param pool Thread pool
 * @param items Array of work items
 * @param num_items Number of work items
 * @return 0 on success, non-zero on error
 * 
 * @note This function blocks until all work items are completed
 * @note Work items are automatically partitioned across threads
 */
int cllm_88d_distribute_work(CLLM88DThreadPool* pool, 
                             CLLM88DWorkItem* items,
                             int num_items);

/**
 * @brief Execute custom work function across 88D threads
 * 
 * Executes a user-defined function on each of the 88 threads.
 * 
 * @param pool Thread pool
 * @param work_func Work function to execute
 * @param user_data User data passed to work function
 * @param partition_size Size of data partition per thread
 * @return 0 on success, non-zero on error
 * 
 * @note This is a lower-level API for custom operations
 */
int cllm_88d_execute_parallel(CLLM88DThreadPool* pool,
                              CLLM88DWorkFunction work_func,
                              void* user_data,
                              size_t partition_size);

/**
 * @brief Wait for all work to complete
 * 
 * Blocks until all pending work items are completed.
 * 
 * @param pool Thread pool
 * @return 0 on success, non-zero on error
 */
int cllm_88d_wait_completion(CLLM88DThreadPool* pool);

// ============================================================================
// SYNCHRONIZATION
// ============================================================================

/**
 * @brief Barrier synchronization across all threads
 * 
 * Blocks until all 88 threads reach the barrier.
 * 
 * @param pool Thread pool
 * @return 0 on success, non-zero on error
 */
int cllm_88d_barrier(CLLM88DThreadPool* pool);

/**
 * @brief Layer barrier synchronization
 * 
 * Blocks until all threads in a specific layer reach the barrier.
 * 
 * @param pool Thread pool
 * @param layer Layer index (0-7)
 * @return 0 on success, non-zero on error
 */
int cllm_88d_layer_barrier(CLLM88DThreadPool* pool, int layer);

// ============================================================================
// GRADIENT OPERATIONS
// ============================================================================

/**
 * @brief Accumulate gradients across threads
 * 
 * Accumulates gradients from all threads into a single buffer.
 * Uses reduction operation (sum) across threads.
 * 
 * @param pool Thread pool
 * @param local_gradients Array of per-thread gradient buffers
 * @param gradient_size Size of each gradient buffer
 * @param output_gradients Output buffer for accumulated gradients
 * @return 0 on success, non-zero on error
 * 
 * @note This is a collective operation - all threads must participate
 */
int cllm_88d_accumulate_gradients(CLLM88DThreadPool* pool,
                                  double** local_gradients,
                                  size_t gradient_size,
                                  double* output_gradients);

/**
 * @brief Reduce operation across threads
 * 
 * Generic reduction operation (sum, max, min, etc.) across threads.
 * 
 * @param pool Thread pool
 * @param local_values Array of per-thread values
 * @param num_values Number of values per thread
 * @param output_values Output buffer for reduced values
 * @param reduce_op Reduction operation (0=sum, 1=max, 2=min)
 * @return 0 on success, non-zero on error
 */
int cllm_88d_reduce(CLLM88DThreadPool* pool,
                   double** local_values,
                   size_t num_values,
                   double* output_values,
                   int reduce_op);

// ============================================================================
// STATISTICS AND MONITORING
// ============================================================================

/**
 * @brief Get thread pool statistics
 * 
 * Retrieves current statistics for the thread pool.
 * 
 * @param pool Thread pool
 * @param stats Output statistics structure
 * @return 0 on success, non-zero on error
 */
int cllm_88d_get_stats(CLLM88DThreadPool* pool, 
                       CLLM88DStatistics* stats);

/**
 * @brief Reset thread pool statistics
 * 
 * Resets all statistics counters to zero.
 * 
 * @param pool Thread pool
 */
void cllm_88d_reset_stats(CLLM88DThreadPool* pool);

/**
 * @brief Print thread pool statistics
 * 
 * Prints detailed statistics to stdout.
 * 
 * @param pool Thread pool
 */
void cllm_88d_print_stats(CLLM88DThreadPool* pool);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Get number of physical threads
 * 
 * @param pool Thread pool
 * @return Number of physical threads
 */
int cllm_88d_get_num_physical_threads(CLLM88DThreadPool* pool);

/**
 * @brief Get number of logical threads
 * 
 * @param pool Thread pool
 * @return Number of logical threads (always 88)
 */
int cllm_88d_get_num_logical_threads(CLLM88DThreadPool* pool);

/**
 * @brief Check if adaptive threading is enabled
 * 
 * @param pool Thread pool
 * @return true if adaptive threading is enabled
 */
bool cllm_88d_is_adaptive(CLLM88DThreadPool* pool);

/**
 * @brief Get thread ID for current thread
 * 
 * @return Thread ID (0-87), or -1 if not in thread pool
 */
int cllm_88d_get_thread_id(void);

#ifdef __cplusplus
}
#endif

#endif // CLLM_88D_INTEGRATION_H