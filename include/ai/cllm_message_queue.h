#ifndef CLLM_MESSAGE_QUEUE_H
#define CLLM_MESSAGE_QUEUE_H

/**
 * CLLM Message Queue - Wrapper for Algorithm Library
 * 
 * This header now uses the algorithm library's MessageSystem implementation.
 * The MessageSystem provides lock-free message queues with priority support.
 * 
 * The algorithm library provides:
 * - MessageSystem structure (manages all message channels)
 * - MessageChannel structure (per-thread message queue)
 * - MessageQueue structure (lock-free priority queue)
 * - All message queue functions
 */

// Use algorithm library's message passing implementation
#include "../../algorithms/include/message_passing.h"
#include "ai/cllm_sphere_message.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TYPE ALIASES FOR COMPATIBILITY
// ============================================================================

/**
 * LockFreeMessageQueue is now an alias for MessageQueue from the algorithm library
 */
typedef MessageQueue LockFreeMessageQueue;

/**
 * MessageChannel is provided by the algorithm library
 * Each sphere has its own MessageChannel for receiving messages
 */
// typedef MessageChannel MessageChannel; // Already defined in message_passing.h

// ============================================================================
// COMPATIBILITY FUNCTIONS
// ============================================================================

/**
 * Create message queue (compatibility wrapper)
 * 
 * Note: In the new architecture, message queues are created by the MessageSystem.
 * This function is provided for backward compatibility.
 * 
 * @param capacity Maximum queue capacity
 * @return Created queue, or NULL on error
 */
static inline LockFreeMessageQueue* message_queue_create(uint32_t capacity) {
    // In the new architecture, queues are managed by MessageSystem
    // This is a placeholder for compatibility
    (void)capacity;
    return NULL;
}

/**
 * Free message queue (compatibility wrapper)
 * 
 * Note: In the new architecture, message queues are freed by the MessageSystem.
 * 
 * @param queue Queue to free
 */
static inline void message_queue_free(LockFreeMessageQueue* queue) {
    // In the new architecture, queues are managed by MessageSystem
    (void)queue;
}

/**
 * Enqueue message (compatibility wrapper)
 * 
 * @param queue Queue to enqueue to
 * @param msg Message to enqueue
 * @return true on success, false on failure
 */
static inline bool message_queue_enqueue(LockFreeMessageQueue* queue, SphereMessage* msg) {
    if (!queue || !msg) {
        return false;
    }
    return message_queue_push(queue, msg);
}

/**
 * Dequeue message (compatibility wrapper)
 * 
 * @param queue Queue to dequeue from
 * @return Dequeued message, or NULL if queue is empty
 */
static inline SphereMessage* message_queue_dequeue(LockFreeMessageQueue* queue) {
    if (!queue) {
        return NULL;
    }
    return message_queue_pop(queue);
}

/**
 * Check if queue is empty (compatibility wrapper)
 * 
 * @param queue Queue to check
 * @return true if empty, false otherwise
 */
static inline bool message_queue_is_empty(const LockFreeMessageQueue* queue) {
    if (!queue) {
        return true;
    }
    return message_queue_size((LockFreeMessageQueue*)queue) == 0;
}

/**
 * Get queue size (compatibility wrapper)
 * 
 * @param queue Queue to query
 * @return Number of messages in queue
 */
static inline uint64_t message_queue_size(const LockFreeMessageQueue* queue) {
    if (!queue) {
        return 0;
    }
    // The algorithm library's message_queue_size is not const-correct
    return message_queue_size((LockFreeMessageQueue*)queue);
}

#ifdef __cplusplus
}
#endif

#endif /* CLLM_MESSAGE_QUEUE_H */