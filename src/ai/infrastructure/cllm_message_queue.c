/**
 * @file cllm_message_queue.c
 * @brief CLLM Message Queue Implementation - Wrapper for Algorithm Library
 * 
 * This file now uses the algorithm library's MessageSystem implementation.
 * Most functionality is delegated to the algorithm library.
 * 
 * Note: The algorithm library provides all the lock-free queue operations.
 * This file only provides compatibility wrappers if needed.
 */

#include "ai/cllm_message_queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// COMPATIBILITY WRAPPERS
// ============================================================================

/**
 * All message queue functions are now provided by the algorithm library.
 * See algorithms/include/message_passing.h for the full API.
 * 
 * Available functions:
 * - message_queue_enqueue(queue, msg) - Add message to queue
 * - message_queue_dequeue(queue) - Remove message from queue
 * - message_queue_is_empty(queue) - Check if queue is empty
 * - message_queue_size(queue) - Get number of messages in queue
 * 
 * For creating message queues, use MessageSystem:
 * - message_system_create(max_channels, pool_size)
 * - message_system_get_channel(system, channel_id)
 * - message_channel_send(channel, msg)
 * - message_channel_receive(channel, timeout_ms)
 */

// This file is now mostly empty as all functionality is in the algorithm library
// Any CLLM-specific extensions can be added here if needed