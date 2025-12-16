#ifndef CLLM_SPHERE_MESSAGE_H
#define CLLM_SPHERE_MESSAGE_H

/**
 * CLLM Sphere Message System - Wrapper for Algorithm Library
 * 
 * This header now uses the algorithm library's message passing implementation.
 * CLLM-specific message types are defined as user-defined types (1000+).
 * 
 * The algorithm library provides:
 * - MessageType enum (standard types 0-999, user-defined 1000+)
 * - MessagePriority enum (CRITICAL=0, HIGH=1, NORMAL=2, LOW=3)
 * - Message structure
 * - All message passing functions
 */

// Use algorithm library's message passing implementation
#include "../../algorithms/include/message_passing.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CLLM-SPECIFIC MESSAGE TYPES (User-Defined: 1000+)
// ============================================================================

/**
 * CLLM Message Types
 * 
 * These are defined as user-defined message types in the algorithm library.
 * They start at MSG_TYPE_USER_DEFINED (1000) to avoid conflicts.
 */
typedef enum {
    // Work Distribution Messages (1000-1099)
    MSG_WORK_REQUEST = MSG_TYPE_USER_DEFINED + 0,      // Request work from sibling
    MSG_WORK_OFFER = MSG_TYPE_USER_DEFINED + 1,        // Offer work to sibling
    MSG_WORK_ACCEPT = MSG_TYPE_USER_DEFINED + 2,       // Accept offered work
    MSG_WORK_REJECT = MSG_TYPE_USER_DEFINED + 3,       // Reject offered work
    
    // Gradient Synchronization Messages (1100-1199)
    MSG_GRADIENT_READY = MSG_TYPE_USER_DEFINED + 100,       // Gradients ready for accumulation
    MSG_GRADIENT_ACCUMULATE = MSG_TYPE_USER_DEFINED + 101,  // Request gradient accumulation
    MSG_GRADIENT_COMPLETE = MSG_TYPE_USER_DEFINED + 102,    // Gradient accumulation complete
    MSG_GRADIENT_UPDATE = MSG_TYPE_USER_DEFINED + 103,      // Gradient update message
    
    // Weight Broadcasting Messages (1200-1299)
    MSG_WEIGHTS_UPDATED = MSG_TYPE_USER_DEFINED + 200,      // Weights have been updated
    MSG_WEIGHTS_REQUEST = MSG_TYPE_USER_DEFINED + 201,      // Request latest weights
    MSG_WEIGHTS_BROADCAST = MSG_TYPE_USER_DEFINED + 202,    // Broadcasting weights to children
    MSG_WEIGHT_UPDATE = MSG_TYPE_USER_DEFINED + 203,        // Weight update message
    
    // Boundary Notifications (1300-1399)
    MSG_BOUNDARY_CROSSING = MSG_TYPE_USER_DEFINED + 300,    // Crossed 144000 boundary
    MSG_TWIN_PRIME_HIT = MSG_TYPE_USER_DEFINED + 301,       // Hit twin prime (143999 or 144001)
    MSG_BOUNDARY_REGION_ENTER = MSG_TYPE_USER_DEFINED + 302,// Entered boundary region
    MSG_BOUNDARY_REGION_EXIT = MSG_TYPE_USER_DEFINED + 303, // Exited boundary region
    
    // Coordination Messages (1400-1499)
    MSG_EPOCH_START = MSG_TYPE_USER_DEFINED + 400,          // Start new epoch
    MSG_EPOCH_COMPLETE = MSG_TYPE_USER_DEFINED + 401,       // Epoch complete
    MSG_BATCH_START = MSG_TYPE_USER_DEFINED + 402,          // Start new batch
    MSG_BATCH_COMPLETE = MSG_TYPE_USER_DEFINED + 403,       // Batch complete
    MSG_SYNC_REQUEST = MSG_TYPE_USER_DEFINED + 404,         // Synchronization request
    MSG_SYNC_RESPONSE = MSG_TYPE_USER_DEFINED + 405,        // Synchronization response
    
    // Hierarchy Management (1500-1599)
    MSG_CHILD_SPAWN = MSG_TYPE_USER_DEFINED + 500,          // Spawn child sphere
    MSG_CHILD_TERMINATE = MSG_TYPE_USER_DEFINED + 501,      // Terminate child sphere
    MSG_PARENT_SYNC = MSG_TYPE_USER_DEFINED + 502,          // Synchronize with parent
    MSG_SIBLING_DISCOVER = MSG_TYPE_USER_DEFINED + 503,     // Discover sibling sphere
    
    // Error Handling (1600-1699)
    MSG_ERROR_REPORT = MSG_TYPE_USER_DEFINED + 600,         // Report error
    MSG_ERROR_RECOVERY = MSG_TYPE_USER_DEFINED + 601,       // Error recovery initiated
    
    // Statistics (1700-1799)
    MSG_STATS_REQUEST = MSG_TYPE_USER_DEFINED + 700,        // Request statistics
    MSG_STATS_REPORT = MSG_TYPE_USER_DEFINED + 701,         // Report statistics
    
    // Shutdown (1800-1899)
    MSG_SHUTDOWN_REQUEST = MSG_TYPE_USER_DEFINED + 800,     // Request shutdown
    MSG_SHUTDOWN_ACK = MSG_TYPE_USER_DEFINED + 801,         // Acknowledge shutdown
    
} CLLMMessageType;

// ============================================================================
// TYPE ALIASES FOR COMPATIBILITY
// ============================================================================

/**
 * SphereMessage is now an alias for Message from the algorithm library
 */
typedef Message SphereMessage;

// ============================================================================
// CLLM-SPECIFIC MESSAGE PAYLOAD STRUCTURES
// ============================================================================

/**
 * Work Request Payload
 */
typedef struct {
    uint64_t requested_items;   // Number of work items requested
    int symmetry_group;         // Requesting symmetry group
    uint64_t current_load;      // Current workload of requester
} WorkRequestPayload;

/**
 * Work Offer Payload
 */
typedef struct {
    uint64_t offered_items;     // Number of work items offered
    int symmetry_group;         // Offering symmetry group
    uint64_t estimated_cost;    // Estimated cost of work
} WorkOfferPayload;

/**
 * Gradient Payload
 */
typedef struct {
    uint64_t gradient_size;     // Size of gradient data
    uint64_t layer_id;          // Layer identifier
    double* gradient_data;      // Pointer to gradient data (in shared memory)
} GradientPayload;

/**
 * Weight Payload
 */
typedef struct {
    uint64_t weight_size;       // Size of weight data
    uint64_t layer_id;          // Layer identifier
    double* weight_data;        // Pointer to weight data (in shared memory)
} WeightPayload;

/**
 * Boundary Notification Payload
 */
typedef struct {
    uint64_t boundary_value;    // The boundary value crossed (prime number)
    int symmetry_group;         // Symmetry group of the sphere
    double confidence;          // Confidence score (0.0-1.0)
    int direction;              // Direction: 1 = up, -1 = down, 0 = at boundary
} BoundaryPayload;

/**
 * Statistics Payload
 */
typedef struct {
    uint64_t messages_sent;
    uint64_t messages_received;
    uint64_t work_completed;
    uint64_t work_stolen;
    uint64_t gradients_computed;
    uint64_t weights_updated;
} StatsPayload;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// sphere_message_create is now implemented in cllm_sphere_message.c
// See function declarations below

/**
 * Create sphere message (compatibility wrapper)
 * 
 * Creates a message with no payload data.
 * For messages with payloads, allocate the payload separately and set it.
 * 
 * @param type CLLM message type
 * @param priority Message priority
 * @param sender_id Sender sphere ID
 * @param receiver_id Receiver sphere ID
 * @return Created message, or NULL on error
 */
SphereMessage* sphere_message_create(
    CLLMMessageType type,
    MessagePriority priority,
    int sender_id,
    int receiver_id
);

/**
 * Free sphere message
 * 
 * @param message Message to free
 */
void sphere_message_free(SphereMessage* message);

/**
 * Clone sphere message
 * 
 * @param message Message to clone
 * @return Cloned message, or NULL on error
 */
SphereMessage* sphere_message_clone(const SphereMessage* message);

/**
 * Set work request payload
 */
void sphere_message_set_work_request(
    SphereMessage* message,
    uint64_t requested_items,
    int symmetry_group,
    uint64_t current_load
);

/**
 * Set work offer payload
 */
void sphere_message_set_work_offer(
    SphereMessage* message,
    uint64_t offered_items,
    int symmetry_group,
    uint64_t estimated_cost
);

/**
 * Set gradient payload
 */
void sphere_message_set_gradient(
    SphereMessage* message,
    uint64_t gradient_size,
    uint64_t layer_id,
    double* gradient_data
);

/**
 * Set weight payload
 */
void sphere_message_set_weight(
    SphereMessage* message,
    uint64_t weight_size,
    uint64_t layer_id,
    double* weight_data
);

/**
 * Set boundary payload
 */
void sphere_message_set_boundary(
    SphereMessage* message,
    uint64_t boundary_value,
    int symmetry_group,
    double confidence,
    int direction
);

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
);

/**
 * Get message type name
 * 
 * @param type Message type
 * @return String name of message type
 */
const char* sphere_message_type_name(CLLMMessageType type);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_SPHERE_MESSAGE_H */