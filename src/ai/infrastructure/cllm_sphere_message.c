/**
 * @file cllm_sphere_message.c
 * @brief CLLM Sphere Message Implementation - Wrapper for Algorithm Library
 * 
 * This file now uses the algorithm library's message passing implementation.
 * Functions are wrappers that delegate to the algorithm library.
 */

#include "ai/cllm_sphere_message.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// MESSAGE TYPE NAME MAPPING
// ============================================================================

const char* sphere_message_type_name(CLLMMessageType type) {
    switch (type) {
        // Work Distribution
        case MSG_WORK_REQUEST: return "WORK_REQUEST";
        case MSG_WORK_OFFER: return "WORK_OFFER";
        case MSG_WORK_ACCEPT: return "WORK_ACCEPT";
        case MSG_WORK_REJECT: return "WORK_REJECT";
        
        // Gradient Synchronization
        case MSG_GRADIENT_READY: return "GRADIENT_READY";
        case MSG_GRADIENT_ACCUMULATE: return "GRADIENT_ACCUMULATE";
        case MSG_GRADIENT_COMPLETE: return "GRADIENT_COMPLETE";
        case MSG_GRADIENT_UPDATE: return "GRADIENT_UPDATE";
        
        // Weight Broadcasting
        case MSG_WEIGHTS_UPDATED: return "WEIGHTS_UPDATED";
        case MSG_WEIGHTS_REQUEST: return "WEIGHTS_REQUEST";
        case MSG_WEIGHTS_BROADCAST: return "WEIGHTS_BROADCAST";
        case MSG_WEIGHT_UPDATE: return "WEIGHT_UPDATE";
        
        // Boundary Notifications
        case MSG_BOUNDARY_CROSSING: return "BOUNDARY_CROSSING";
        case MSG_TWIN_PRIME_HIT: return "TWIN_PRIME_HIT";
        case MSG_BOUNDARY_REGION_ENTER: return "BOUNDARY_REGION_ENTER";
        case MSG_BOUNDARY_REGION_EXIT: return "BOUNDARY_REGION_EXIT";
        
        // Coordination
        case MSG_EPOCH_START: return "EPOCH_START";
        case MSG_EPOCH_COMPLETE: return "EPOCH_COMPLETE";
        case MSG_BATCH_START: return "BATCH_START";
        case MSG_BATCH_COMPLETE: return "BATCH_COMPLETE";
        case MSG_SYNC_REQUEST: return "SYNC_REQUEST";
        case MSG_SYNC_RESPONSE: return "SYNC_RESPONSE";
        
        // Hierarchy Management
        case MSG_CHILD_SPAWN: return "CHILD_SPAWN";
        case MSG_CHILD_TERMINATE: return "CHILD_TERMINATE";
        case MSG_PARENT_SYNC: return "PARENT_SYNC";
        case MSG_SIBLING_DISCOVER: return "SIBLING_DISCOVER";
        
        // Error Handling
        case MSG_ERROR_REPORT: return "ERROR_REPORT";
        case MSG_ERROR_RECOVERY: return "ERROR_RECOVERY";
        
        // Statistics
        case MSG_STATS_REQUEST: return "STATS_REQUEST";
        case MSG_STATS_REPORT: return "STATS_REPORT";
        
        // Shutdown
        case MSG_SHUTDOWN_REQUEST: return "SHUTDOWN_REQUEST";
        case MSG_SHUTDOWN_ACK: return "SHUTDOWN_ACK";
        
        default: return "UNKNOWN";
    }
}

// ============================================================================
// COMPATIBILITY WRAPPERS
// ============================================================================

/**
 * Create sphere message (compatibility wrapper)
 * 
 * Note: This is a simplified wrapper for backward compatibility.
 * In the new architecture, messages should be created through the MessageSystem
 * using message_create() from the algorithm library.
 * 
 * This function creates a message with NULL data and 0 size.
 * For messages with payloads, use message_create() directly.
 */
SphereMessage* sphere_message_create(
    CLLMMessageType type,
    MessagePriority priority,
    int sender_id,
    int receiver_id
) {
    // Create a message with no payload
    // Note: In the new architecture, this should go through MessageSystem
    // For now, we create a standalone message
    SphereMessage* msg = calloc(1, sizeof(SphereMessage));
    if (!msg) {
        return NULL;
    }
    
    msg->type = (MessageType)type;
    msg->priority = priority;
    msg->sender_id = sender_id;
    msg->receiver_id = receiver_id;
    msg->data = NULL;
    msg->data_size = 0;
    msg->timestamp = 0; // Will be set when sent
    msg->next = NULL;
    
    return msg;
}

/**
 * Free sphere message (compatibility wrapper)
 */
void sphere_message_free(SphereMessage* message) {
    if (!message) {
        return;
    }
    
    // Note: We don't free data here as it may be managed elsewhere
    // The caller is responsible for data lifecycle
    free(message);
}

/**
 * Clone sphere message (compatibility wrapper)
 */
SphereMessage* sphere_message_clone(const SphereMessage* message) {
    if (!message) {
        return NULL;
    }
    
    SphereMessage* clone = calloc(1, sizeof(SphereMessage));
    if (!clone) {
        return NULL;
    }
    
    // Copy message fields
    clone->type = message->type;
    clone->priority = message->priority;
    clone->sender_id = message->sender_id;
    clone->receiver_id = message->receiver_id;
    clone->data = message->data;  // Shallow copy - caller manages data
    clone->data_size = message->data_size;
    clone->timestamp = message->timestamp;
    clone->next = NULL;
    
    return clone;
}

// ============================================================================
// PAYLOAD SETTERS (Deprecated - use message data field directly)
// ============================================================================

/**
 * Set work request payload
 * 
 * Note: In the new architecture, payloads should be set directly in the
 * message data field when creating the message.
 */
void sphere_message_set_work_request(
    SphereMessage* message,
    uint64_t requested_items,
    int symmetry_group,
    uint64_t current_load
) {
    if (!message) {
        return;
    }
    
    // Allocate payload
    WorkRequestPayload* payload = malloc(sizeof(WorkRequestPayload));
    if (!payload) {
        return;
    }
    
    payload->requested_items = requested_items;
    payload->symmetry_group = symmetry_group;
    payload->current_load = current_load;
    
    message->data = payload;
    message->data_size = sizeof(WorkRequestPayload);
}

/**
 * Set work offer payload
 */
void sphere_message_set_work_offer(
    SphereMessage* message,
    uint64_t offered_items,
    int symmetry_group,
    uint64_t estimated_cost
) {
    if (!message) {
        return;
    }
    
    WorkOfferPayload* payload = malloc(sizeof(WorkOfferPayload));
    if (!payload) {
        return;
    }
    
    payload->offered_items = offered_items;
    payload->symmetry_group = symmetry_group;
    payload->estimated_cost = estimated_cost;
    
    message->data = payload;
    message->data_size = sizeof(WorkOfferPayload);
}

/**
 * Set gradient payload
 */
void sphere_message_set_gradient(
    SphereMessage* message,
    uint64_t gradient_size,
    uint64_t layer_id,
    double* gradient_data
) {
    if (!message) {
        return;
    }
    
    GradientPayload* payload = malloc(sizeof(GradientPayload));
    if (!payload) {
        return;
    }
    
    payload->gradient_size = gradient_size;
    payload->layer_id = layer_id;
    payload->gradient_data = gradient_data;
    
    message->data = payload;
    message->data_size = sizeof(GradientPayload);
}

/**
 * Set weight payload
 */
void sphere_message_set_weight(
    SphereMessage* message,
    uint64_t weight_size,
    uint64_t layer_id,
    double* weight_data
) {
    if (!message) {
        return;
    }
    
    WeightPayload* payload = malloc(sizeof(WeightPayload));
    if (!payload) {
        return;
    }
    
    payload->weight_size = weight_size;
    payload->layer_id = layer_id;
    payload->weight_data = weight_data;
    
    message->data = payload;
    message->data_size = sizeof(WeightPayload);
}

/**
 * Set boundary payload
 */
void sphere_message_set_boundary(
    SphereMessage* message,
    uint64_t boundary_value,
    int symmetry_group,
    double confidence,
    int direction
) {
    if (!message) {
        return;
    }
    
    BoundaryPayload* payload = malloc(sizeof(BoundaryPayload));
    if (!payload) {
        return;
    }
    
    payload->boundary_value = boundary_value;
    payload->symmetry_group = symmetry_group;
    payload->confidence = confidence;
    payload->direction = direction;
    
    message->data = payload;
    message->data_size = sizeof(BoundaryPayload);
}

/**
 * Set statistics payload
 */
void sphere_message_set_stats(
    SphereMessage* message,
    uint64_t messages_sent,
    uint64_t messages_received,
    uint64_t work_completed,
    uint64_t work_stolen,
    uint64_t gradients_computed,
    uint64_t weights_updated
) {
    if (!message) {
        return;
    }
    
    StatsPayload* payload = malloc(sizeof(StatsPayload));
    if (!payload) {
        return;
    }
    
    payload->messages_sent = messages_sent;
    payload->messages_received = messages_received;
    payload->work_completed = work_completed;
    payload->work_stolen = work_stolen;
    payload->gradients_computed = gradients_computed;
    payload->weights_updated = weights_updated;
    
    message->data = payload;
    message->data_size = sizeof(StatsPayload);
}