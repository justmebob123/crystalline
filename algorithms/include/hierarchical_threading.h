/**
 * @file hierarchical_threading.h
 * @brief Unified Hierarchical Threading System
 * 
 * This is the complete integration of all Week 1-4 components:
 * - Week 1: Hierarchical Memory + Abacus Integration
 * - Week 2: Shared Memory + Rainbow Table
 * - Week 3: Message Passing System
 * - Week 4: State Management + Work Distribution
 * 
 * Design Philosophy:
 * - Sphere threading with neighbor operations
 * - Geometric memory organization (12-fold symmetry)
 * - Lock-free communication
 * - Hierarchical state management
 * - Work stealing for load balancing
 * - Zero external dependencies
 * 
 * Key Features:
 * - N-fold geometric organization (configurable)
 * - Parent-child-sibling relationships
 * - Kissing boundaries between neighbors
 * - 3-tier shared memory (READ_ONLY, COW, LOCKED_WRITE)
 * - Lock-free message passing
 * - State machines for thread lifecycle
 * - Work distribution with stealing
 * - Thread pool management
 * - NUMA-aware allocation
 */

#ifndef HIERARCHICAL_THREADING_H
#define HIERARCHICAL_THREADING_H

#include "hierarchical_memory.h"
#include "shared_memory.h"
#include "shared_memory_enhanced.h"
#include "message_passing.h"
#include "state_management.h"
#include "work_distribution.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

#define HIERARCHICAL_THREAD_MAX_NEIGHBORS 12
#define HIERARCHICAL_THREAD_MAX_CHILDREN 12
#define HIERARCHICAL_THREAD_BROADCAST_ID 0xFFFFFFFF

// ============================================================================
// THREAD TYPES
// ============================================================================

/**
 * Thread role in hierarchy
 */
typedef enum {
    THREAD_ROLE_CONTROL,    // Control thread (root of hierarchy)
    THREAD_ROLE_WORKER,     // Worker thread (leaf node)
    THREAD_ROLE_MANAGER,    // Manager thread (intermediate node)
    THREAD_ROLE_HELPER      // Helper thread (temporary)
} ThreadRole;

/**
 * Thread relationship type
 */
typedef enum {
    THREAD_REL_PARENT,      // Parent thread
    THREAD_REL_CHILD,       // Child thread
    THREAD_REL_SIBLING,     // Sibling thread (same parent)
    THREAD_REL_NEIGHBOR,    // Neighbor thread (kissing sphere)
    THREAD_REL_NONE         // No relationship
} ThreadRelationType;

// ============================================================================
// THREAD NEIGHBOR STRUCTURE
// ============================================================================

/**
 * Thread neighbor information
 */
typedef struct {
    uint32_t thread_id;                 // Neighbor thread ID
    ThreadRelationType relationship;    // Relationship type
    double distance;                    // Geometric distance
    SharedMemoryEnhanced* boundary;     // Shared boundary memory
} ThreadNeighbor;

// ============================================================================
// HIERARCHICAL THREAD STRUCTURE
// ============================================================================

/**
 * Hierarchical Thread
 * 
 * Represents one thread in the hierarchical threading system.
 * Integrates all Week 1-4 components.
 */
typedef struct HierarchicalThread {
    // Identity
    uint32_t thread_id;                 // Unique thread ID
    pthread_t pthread;                  // POSIX thread handle
    ThreadRole role;                    // Thread role
    
    // Hierarchy
    struct HierarchicalThread* parent;  // Parent thread
    struct HierarchicalThread** children; // Child threads
    uint32_t num_children;              // Number of children
    uint32_t max_children;              // Maximum children
    
    // Neighbors (kissing spheres)
    ThreadNeighbor neighbors[HIERARCHICAL_THREAD_MAX_NEIGHBORS];
    uint32_t num_neighbors;             // Number of neighbors
    
    // Week 1: Hierarchical Memory
    HierarchicalMemoryBlock* memory;    // Thread's hierarchical memory
    uint32_t memory_segment_id;         // Primary memory segment
    
    // Week 2: Shared Memory
    SharedMemoryEnhanced* local_shared; // Local shared memory
    SharedMemoryEnhanced* parent_shared; // Shared with parent
    SharedMemoryEnhanced** child_shared; // Shared with children
    
    // Week 3: Message Passing
    MessageQueue* inbox;                // Incoming messages
    MessageQueue* outbox;               // Outgoing messages
    uint64_t messages_sent;             // Total messages sent
    uint64_t messages_received;         // Total messages received
    
    // Week 4: State Management
    StateMachine* state_machine;        // Thread state machine
    StateType current_state;            // Current state (cached)
    
    // Week 4: Work Distribution
    WorkPool* work_pool;                // Thread's work pool
    uint64_t work_completed;            // Total work completed
    uint64_t work_stolen;               // Work stolen from others
    
    // Geometric Position (from abacus)
    double* position;                   // Position in N-dimensional space
    uint32_t num_dimensions;            // Number of dimensions
    uint32_t symmetry_group;            // Symmetry group (0 to N-1)
    
    // Thread Control
    volatile bool running;              // Thread is running
    volatile bool should_stop;          // Thread should stop
    pthread_mutex_t control_mutex;      // Control mutex
    pthread_cond_t control_cond;        // Control condition variable
    
    // Statistics
    uint64_t start_time;                // Thread start time
    uint64_t total_runtime;             // Total runtime (nanoseconds)
    uint64_t context_switches;          // Number of context switches
    
    // NUMA
    int numa_node;                      // NUMA node this thread is on
    
} HierarchicalThread;

// ============================================================================
// THREAD POOL STRUCTURE
// ============================================================================

/**
 * Hierarchical Thread Pool
 * 
 * Manages a pool of hierarchical threads organized geometrically.
 */
typedef struct {
    // Threads
    HierarchicalThread** threads;       // Array of threads
    uint32_t num_threads;               // Number of threads
    uint32_t max_threads;               // Maximum threads
    
    // Hierarchy
    HierarchicalThread* root;           // Root control thread
    uint32_t num_levels;                // Number of hierarchy levels
    uint32_t threads_per_level;         // Threads per level
    
    // Shared Components
    HierarchicalMemoryBlock* global_memory;  // Global hierarchical memory
    WorkDistributor* work_distributor;       // Global work distributor
    StateManager* state_manager;             // Global state manager
    
    // Configuration
    uint32_t symmetry_fold;             // N-fold symmetry (default 12)
    uint32_t num_dimensions;            // Number of dimensions
    bool numa_aware;                    // Use NUMA-aware allocation
    
    // Statistics
    uint64_t total_messages;            // Total messages passed
    uint64_t total_work_items;          // Total work items processed
    uint64_t total_state_changes;       // Total state changes
    
    // Control
    pthread_mutex_t pool_mutex;         // Pool-wide mutex
    volatile bool initialized;          // Pool is initialized
    volatile bool running;              // Pool is running
    
} HierarchicalThreadPool;

// ============================================================================
// THREAD POOL OPERATIONS
// ============================================================================

/**
 * Create hierarchical thread pool
 * 
 * @param num_threads Number of threads to create
 * @param symmetry_fold N-fold symmetry (e.g., 12 for dodecahedron)
 * @param num_dimensions Number of dimensions for geometric organization
 * @param numa_aware Use NUMA-aware allocation
 * @return Allocated thread pool, or NULL on error
 */
HierarchicalThreadPool* hierarchical_thread_pool_create(
    uint32_t num_threads,
    uint32_t symmetry_fold,
    uint32_t num_dimensions,
    bool numa_aware
);

/**
 * Free hierarchical thread pool
 * 
 * @param pool Pool to free
 */
void hierarchical_thread_pool_free(HierarchicalThreadPool* pool);

/**
 * Start thread pool
 * 
 * Starts all threads in the pool.
 * 
 * @param pool Pool to start
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_start(HierarchicalThreadPool* pool);

/**
 * Stop thread pool
 * 
 * Stops all threads in the pool gracefully.
 * 
 * @param pool Pool to stop
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_stop(HierarchicalThreadPool* pool);

/**
 * Wait for thread pool to finish
 * 
 * Blocks until all threads have stopped.
 * 
 * @param pool Pool to wait for
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_wait(HierarchicalThreadPool* pool);

// ============================================================================
// THREAD OPERATIONS
// ============================================================================

/**
 * Create hierarchical thread
 * 
 * @param thread_id Unique thread ID
 * @param role Thread role
 * @param parent Parent thread (NULL for root)
 * @param pool Thread pool this thread belongs to
 * @return Allocated thread, or NULL on error
 */
HierarchicalThread* hierarchical_thread_create(
    uint32_t thread_id,
    ThreadRole role,
    HierarchicalThread* parent,
    HierarchicalThreadPool* pool
);

/**
 * Free hierarchical thread
 * 
 * @param thread Thread to free
 */
void hierarchical_thread_free(HierarchicalThread* thread);

/**
 * Start thread
 * 
 * @param thread Thread to start
 * @param work_fn Work function to execute
 * @param work_data Data to pass to work function
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_start(
    HierarchicalThread* thread,
    void* (*work_fn)(void*),
    void* work_data
);

/**
 * Stop thread
 * 
 * @param thread Thread to stop
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_stop(HierarchicalThread* thread);

/**
 * Join thread
 * 
 * @param thread Thread to join
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_join(HierarchicalThread* thread);

// ============================================================================
// NEIGHBOR OPERATIONS
// ============================================================================

/**
 * Add neighbor to thread
 * 
 * Establishes a neighbor relationship between two threads.
 * Creates a shared boundary memory region.
 * 
 * @param thread Thread to add neighbor to
 * @param neighbor_id Neighbor thread ID
 * @param relationship Relationship type
 * @param distance Geometric distance
 * @param pool Thread pool (for looking up neighbor)
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_add_neighbor(
    HierarchicalThread* thread,
    uint32_t neighbor_id,
    ThreadRelationType relationship,
    double distance,
    HierarchicalThreadPool* pool
);

/**
 * Get neighbor by ID
 * 
 * @param thread Thread to query
 * @param neighbor_id Neighbor thread ID
 * @return Neighbor info, or NULL if not found
 */
ThreadNeighbor* hierarchical_thread_get_neighbor(
    HierarchicalThread* thread,
    uint32_t neighbor_id
);

/**
 * Get all neighbors of a specific type
 * 
 * @param thread Thread to query
 * @param relationship Relationship type to filter by
 * @param out_neighbors Output buffer for neighbors
 * @param max_neighbors Maximum number of neighbors to return
 * @return Number of neighbors returned
 */
uint32_t hierarchical_thread_get_neighbors_by_type(
    HierarchicalThread* thread,
    ThreadRelationType relationship,
    ThreadNeighbor* out_neighbors,
    uint32_t max_neighbors
);

/**
 * Access neighbor boundary
 * 
 * Gets the shared boundary memory with a neighbor.
 * 
 * @param thread Thread to query
 * @param neighbor_id Neighbor thread ID
 * @return Shared boundary memory, or NULL if not found
 */
SharedMemoryEnhanced* hierarchical_thread_get_boundary(
    HierarchicalThread* thread,
    uint32_t neighbor_id
);

// ============================================================================
// MESSAGE OPERATIONS
// ============================================================================

/**
 * Send message to thread
 * 
 * @param sender Sender thread
 * @param receiver_id Receiver thread ID (or BROADCAST)
 * @param type Message type
 * @param priority Message priority
 * @param data Message data
 * @param data_size Data size in bytes
 * @param pool Thread pool (for routing)
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_send_message(
    HierarchicalThread* sender,
    uint32_t receiver_id,
    MessageType type,
    MessagePriority priority,
    void* data,
    size_t data_size,
    HierarchicalThreadPool* pool
);

/**
 * Receive message
 * 
 * @param thread Thread to receive message
 * @param timeout_ms Timeout in milliseconds (0 = no wait, -1 = infinite)
 * @return Received message, or NULL if none available
 */
Message* hierarchical_thread_receive_message(
    HierarchicalThread* thread,
    int timeout_ms
);

/**
 * Broadcast message to all threads
 * 
 * @param sender Sender thread
 * @param type Message type
 * @param priority Message priority
 * @param data Message data
 * @param data_size Data size in bytes
 * @param pool Thread pool
 * @return Number of messages sent, or -1 on error
 */
int hierarchical_thread_broadcast_message(
    HierarchicalThread* sender,
    MessageType type,
    MessagePriority priority,
    void* data,
    size_t data_size,
    HierarchicalThreadPool* pool
);

// ============================================================================
// STATE OPERATIONS
// ============================================================================

/**
 * Change thread state
 * 
 * @param thread Thread to change state
 * @param new_state New state
 * @return Transition result
 */
TransitionResult hierarchical_thread_change_state(
    HierarchicalThread* thread,
    StateType new_state
);

/**
 * Get thread state
 * 
 * @param thread Thread to query
 * @return Current state
 */
StateType hierarchical_thread_get_state(HierarchicalThread* thread);

/**
 * Register state change callback
 * 
 * @param thread Thread to register callback for
 * @param callback Callback function
 * @param user_data User data to pass to callback
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_register_state_callback(
    HierarchicalThread* thread,
    StateChangeCallback callback,
    void* user_data
);

// ============================================================================
// WORK OPERATIONS
// ============================================================================

/**
 * Submit work to thread
 * 
 * @param thread Thread to submit work to
 * @param work_fn Work function
 * @param data Work data
 * @param data_size Data size in bytes
 * @param priority Work priority
 * @return Work item ID, or 0 on error
 */
uint64_t hierarchical_thread_submit_work(
    HierarchicalThread* thread,
    void (*work_fn)(void*),
    void* data,
    size_t data_size,
    WorkPriority priority
);

/**
 * Get work from thread pool
 * 
 * Tries to get work from local queue first, then steals from neighbors.
 * 
 * @param thread Thread requesting work
 * @return Work item, or NULL if none available
 */
WorkItem* hierarchical_thread_get_work(HierarchicalThread* thread);

/**
 * Complete work item
 * 
 * @param thread Thread completing work
 * @param item Work item to complete
 * @param success Whether work completed successfully
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_complete_work(
    HierarchicalThread* thread,
    WorkItem* item,
    bool success
);

// ============================================================================
// MEMORY OPERATIONS
// ============================================================================

/**
 * Allocate thread-local memory
 * 
 * @param thread Thread to allocate for
 * @param size Size in bytes
 * @return Allocated memory, or NULL on error
 */
void* hierarchical_thread_alloc_local(HierarchicalThread* thread, size_t size);

/**
 * Allocate shared memory with parent
 * 
 * @param thread Thread to allocate for
 * @param size Size in bytes
 * @param mode Access mode
 * @return Allocated shared memory, or NULL on error
 */
SharedMemoryEnhanced* hierarchical_thread_alloc_parent_shared(
    HierarchicalThread* thread,
    size_t size,
    SharedMemoryAccessMode mode
);

/**
 * Allocate shared memory with child
 * 
 * @param thread Thread to allocate for
 * @param child_id Child thread ID
 * @param size Size in bytes
 * @param mode Access mode
 * @return Allocated shared memory, or NULL on error
 */
SharedMemoryEnhanced* hierarchical_thread_alloc_child_shared(
    HierarchicalThread* thread,
    uint32_t child_id,
    size_t size,
    SharedMemoryAccessMode mode
);

/**
 * Access neighbor boundary memory
 * 
 * @param thread Thread accessing boundary
 * @param neighbor_id Neighbor thread ID
 * @param mode Access mode
 * @return Boundary memory, or NULL on error
 */
void* hierarchical_thread_access_boundary(
    HierarchicalThread* thread,
    uint32_t neighbor_id,
    SharedMemoryAccessMode mode
);

// ============================================================================
// STATISTICS & MONITORING
// ============================================================================

/**
 * Thread statistics
 */
typedef struct {
    uint32_t thread_id;
    ThreadRole role;
    StateType current_state;
    
    // Hierarchy
    uint32_t num_children;
    uint32_t num_neighbors;
    
    // Messages
    uint64_t messages_sent;
    uint64_t messages_received;
    uint64_t messages_pending;
    
    // Work
    uint64_t work_completed;
    uint64_t work_stolen;
    uint64_t work_pending;
    
    // Memory
    size_t local_memory_used;
    size_t shared_memory_used;
    size_t boundary_memory_used;
    
    // Timing
    uint64_t total_runtime;
    uint64_t state_durations[10];  // Duration in each standard state
    
    // Performance
    double cpu_utilization;
    double work_throughput;
    double message_latency;
    
} HierarchicalThreadStats;

/**
 * Get thread statistics
 * 
 * @param thread Thread to query
 * @param stats Output statistics
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_get_stats(
    HierarchicalThread* thread,
    HierarchicalThreadStats* stats
);

/**
 * Pool statistics
 */
typedef struct {
    uint32_t num_threads;
    uint32_t num_levels;
    uint32_t symmetry_fold;
    
    // Aggregate statistics
    uint64_t total_messages;
    uint64_t total_work_items;
    uint64_t total_state_changes;
    
    // Memory usage
    size_t total_memory_used;
    size_t total_shared_memory;
    size_t total_boundary_memory;
    
    // Performance
    double avg_cpu_utilization;
    double avg_work_throughput;
    double avg_message_latency;
    double load_balance_factor;  // 1.0 = perfect balance
    
} HierarchicalThreadPoolStats;

/**
 * Get pool statistics
 * 
 * @param pool Pool to query
 * @param stats Output statistics
 * @return 0 on success, -1 on error
 */
int hierarchical_thread_pool_get_stats(
    HierarchicalThreadPool* pool,
    HierarchicalThreadPoolStats* stats
);

/**
 * Print thread statistics
 * 
 * @param thread Thread to print stats for
 */
void hierarchical_thread_print_stats(HierarchicalThread* thread);

/**
 * Print pool statistics
 * 
 * @param pool Pool to print stats for
 */
void hierarchical_thread_pool_print_stats(HierarchicalThreadPool* pool);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get thread by ID
 * 
 * @param pool Thread pool
 * @param thread_id Thread ID to find
 * @return Thread, or NULL if not found
 */
HierarchicalThread* hierarchical_thread_pool_get_thread(
    HierarchicalThreadPool* pool,
    uint32_t thread_id
);

/**
 * Calculate geometric distance between threads
 * 
 * @param thread1 First thread
 * @param thread2 Second thread
 * @return Euclidean distance in N-dimensional space
 */
double hierarchical_thread_distance(
    HierarchicalThread* thread1,
    HierarchicalThread* thread2
);

/**
 * Find nearest neighbors
 * 
 * Finds the K nearest neighbors to a thread based on geometric distance.
 * 
 * @param thread Thread to find neighbors for
 * @param pool Thread pool
 * @param k Number of neighbors to find
 * @param out_neighbors Output buffer for neighbor IDs
 * @return Number of neighbors found
 */
uint32_t hierarchical_thread_find_nearest_neighbors(
    HierarchicalThread* thread,
    HierarchicalThreadPool* pool,
    uint32_t k,
    uint32_t* out_neighbors
);

#ifdef __cplusplus
}
#endif

#endif // HIERARCHICAL_THREADING_H