/**
 * @file physical_worker.c
 * @brief Physical worker thread implementation for adaptive threading
 * 
 * This implements the physical worker threads that process work items
 * from logical threads. This allows N physical threads to handle 96
 * logical threads, enabling the system to run on 2-16 core systems.
 */

#include "hierarchical_threading.h"
#include "work_queue.h"
#include "worker_functions_geometric.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Forward declarations
int worker_process_forward(HierarchicalThread* thread, AdaptiveWorkItem* work);
int worker_process_backward(HierarchicalThread* thread, AdaptiveWorkItem* work);

// ============================================================================
// PHYSICAL WORKER THREAD FUNCTION
// ============================================================================

/**
 * Physical worker thread main loop
 * 
 * This is the main loop for physical worker threads. Each physical thread:
 * 1. Pulls work items from the shared work queue
 * 2. Processes the work item using the logical thread's data
 * 3. Repeats until shutdown signal
 * 
 * @param arg PhysicalWorker pointer
 * @return NULL
 */
void* physical_worker_thread(void* arg) {
    PhysicalWorker* worker = (PhysicalWorker*)arg;
    
    if (!worker) {
        fprintf(stderr, "ERROR: Physical worker is NULL\n");
        return NULL;
    }
    
    printf("Physical worker %u started\n", worker->worker_id);
    
    uint64_t start_time = 0; // TODO: get_time_ns();
    worker->running = true;
    
    // Main worker loop
    while (!worker->should_stop) {
        // Pull work item from queue (blocking)
        AdaptiveWorkItem* work = adaptive_work_queue_pop(worker->work_queue);
        
        // If NULL, queue is shutdown
        if (!work) {
            break;
        }
        
        // Check for shutdown signal
        if (work->type == ADAPTIVE_WORK_TYPE_SHUTDOWN) {
            adaptive_work_item_free(work);
            break;
        }
        
        // Process work item
        HierarchicalThread* logical_thread = work->logical_thread;
        if (!logical_thread) {
            fprintf(stderr, "ERROR: Work item has NULL logical thread\n");
            adaptive_work_item_free(work);
            continue;
        }
        
        int result = 0;
        
        switch (work->type) {
            case ADAPTIVE_WORK_TYPE_FORWARD:
                result = worker_process_forward(logical_thread, work);
                break;
                
            case ADAPTIVE_WORK_TYPE_BACKWARD:
                result = worker_process_backward(logical_thread, work);
                break;
                
            case ADAPTIVE_WORK_TYPE_INFERENCE:
                result = worker_process_forward(logical_thread, work);
                break;
                
            default:
                fprintf(stderr, "WARNING: Unknown work type %d\n", work->type);
                result = -1;
                break;
        }
        
        // Update statistics
        if (result == 0) {
            __atomic_add_fetch(&worker->work_items_processed, 1, __ATOMIC_SEQ_CST);
        }
        
        // Free work item
        adaptive_work_item_free(work);
    }
    
    worker->running = false;
    worker->total_runtime = 0; // TODO: get_time_ns() - start_time;
    
    printf("Physical worker %u stopped (processed %lu items)\n", 
           worker->worker_id, worker->work_items_processed);
    
    return NULL;
}

// ============================================================================
// PHYSICAL WORKER MANAGEMENT
// ============================================================================

/**
 * Create physical worker
 * 
 * @param worker_id Worker ID
 * @param pool Parent thread pool
 * @param work_queue Shared work queue
 * @return Allocated physical worker, or NULL on error
 */
PhysicalWorker* physical_worker_create(
    uint32_t worker_id,
    HierarchicalThreadPool* pool,
    AdaptiveWorkQueue* work_queue
) {
    PhysicalWorker* worker = (PhysicalWorker*)calloc(1, sizeof(PhysicalWorker));
    if (!worker) {
        return NULL;
    }
    
    worker->worker_id = worker_id;
    worker->pool = pool;
    worker->work_queue = work_queue;
    worker->running = false;
    worker->should_stop = false;
    worker->work_items_processed = 0;
    worker->total_runtime = 0;
    
    return worker;
}

/**
 * Start physical worker
 * 
 * @param worker Worker to start
 * @return 0 on success, -1 on error
 */
int physical_worker_start(PhysicalWorker* worker) {
    if (!worker) {
        return -1;
    }
    
    int result = pthread_create(&worker->pthread, NULL, physical_worker_thread, worker);
    if (result != 0) {
        fprintf(stderr, "ERROR: Failed to create physical worker thread %u\n", 
                worker->worker_id);
        return -1;
    }
    
    return 0;
}

/**
 * Stop physical worker
 * 
 * @param worker Worker to stop
 * @return 0 on success, -1 on error
 */
int physical_worker_stop(PhysicalWorker* worker) {
    if (!worker) {
        return -1;
    }
    
    worker->should_stop = true;
    
    return 0;
}

/**
 * Wait for physical worker to finish
 * 
 * @param worker Worker to wait for
 * @return 0 on success, -1 on error
 */
int physical_worker_join(PhysicalWorker* worker) {
    if (!worker) {
        return -1;
    }
    
    int result = pthread_join(worker->pthread, NULL);
    if (result != 0) {
        fprintf(stderr, "ERROR: Failed to join physical worker thread %u\n", 
                worker->worker_id);
        return -1;
    }
    
    return 0;
}

/**
 * Free physical worker
 * 
 * @param worker Worker to free
 */
void physical_worker_free(PhysicalWorker* worker) {
    if (worker) {
        free(worker);
    }
}

// ============================================================================
// WORK PROCESSING FUNCTIONS
// ============================================================================

/**
 * Process forward pass work item
 * 
 * @param thread Logical thread to process
 * @param work Work item
 * @return 0 on success, -1 on error
 */
int worker_process_forward(HierarchicalThread* thread, AdaptiveWorkItem* work) {
    if (!thread || !work) {
        return -1;
    }
    
    // TODO: Implement forward pass using worker functions
    // This should call worker_get_embedding_double(), worker_compute_attention_double(), etc.
    
    // For now, just mark as processed
    return 0;
}

/**
 * Process backward pass work item
 * 
 * @param thread Logical thread to process
 * @param work Work item
 * @return 0 on success, -1 on error
 */
int worker_process_backward(HierarchicalThread* thread, AdaptiveWorkItem* work) {
    if (!thread || !work) {
        return -1;
    }
    
    // TODO: Implement backward pass using worker functions
    // This should call worker_compute_gradients_double(), etc.
    
    // For now, just mark as processed
    return 0;
}