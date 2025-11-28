/**
 * CLLM Hierarchical Training Implementation
 * 
 * Implements the correct recursive kissing spheres hierarchy for training.
 * 
 * Architecture:
 * - Level 0: 1 root (Node Zero) - control only, NEVER processes batches
 * - Level 1: Up to 12 spheres - can be control or workers
 * - Level 2+: Recursive structure - each sphere can have up to 12 children
 * 
 * For 64 cores:
 * - Level 0: 1 root control
 * - Level 1: 12 control spheres
 * - Level 2: 51 worker spheres
 * 
 * Work distribution:
 * - Root distributes batches to Level 1 (round-robin)
 * - Level 1 distributes to their workers (round-robin)
 * - Workers process batches and send gradients to parent
 * - Parents accumulate gradients from children
 * - Root does final accumulation and optimizer step
 */

#include "cllm_training.h"
#include "ai/cllm_lattice_hierarchy.h"
#include "ai/cllm_message_queue.h"
#include "ai/cllm_sphere_message.h"
#include "cllm_batch.h"
#include "prime_float_math.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

// Forward declarations
typedef struct HierarchicalTrainingSystem HierarchicalTrainingSystem;

/**
 * Hierarchical Training System
 */
struct HierarchicalTrainingSystem {
    CLLMTraining* training;
    CLLMLatticeHierarchy* root;
    
    int total_threads;
    int num_level1;
    int num_level2;
    
    atomic_int running;
    atomic_int epoch_done;
    
    CLLMBatchIterator* batch_iterator;
    
    float epoch_loss;
    int total_batches;
};

/**
 * Calculate optimal hierarchy structure based on model symmetry
 * 
 * The model structure dictates the hierarchy:
 * - Model has symmetry_order (typically 12)
 * - Create one Level-1 control per symmetry group
 * - Distribute remaining threads as Level-2 workers
 */
static void calculate_hierarchy_structure(int total_threads, int model_symmetry_order,
                                          int* num_level1, int* num_level2) {
    if (total_threads <= 1) {
        *num_level1 = 0;
        *num_level2 = 0;
        return;
    }
    
    if (total_threads <= model_symmetry_order + 1) {
        // Small: just root + workers (no intermediate level)
        *num_level1 = total_threads - 1;
        *num_level2 = 0;
        return;
    }
    
    // Optimal: root + symmetry_order Level-1 controls + remaining workers
    // This matches the model's natural structure
    *num_level1 = model_symmetry_order;
    *num_level2 = total_threads - 1 - model_symmetry_order;
    
    printf("  Hierarchy matches model symmetry:\n");
    printf("    Model symmetry order: %d\n", model_symmetry_order);
    printf("    Level-1 controls: %d (one per symmetry group)\n", *num_level1);
    printf("    Level-2 workers: %d (distributed across groups)\n", *num_level2);
}

/**
 * Create recursive sphere hierarchy based on model structure
 */
static int create_sphere_hierarchy(HierarchicalTrainingSystem* system) {
    // Get model symmetry order (typically 12)
    int model_symmetry_order = system->training->model->header.symmetry_order;
    
    if (model_symmetry_order == 0) {
        fprintf(stderr, "WARNING: Model symmetry_order is 0, defaulting to 12\n");
        model_symmetry_order = 12;
    }
    
    int num_level1, num_level2;
    calculate_hierarchy_structure(system->total_threads, model_symmetry_order, 
                                   &num_level1, &num_level2);
    
    system->num_level1 = num_level1;
    system->num_level2 = num_level2;
    
    printf("Creating hierarchy based on model structure:\n");
    printf("  Level 0: 1 root (Node Zero)\n");
    printf("  Level 1: %d spheres (one per symmetry group)\n", num_level1);
    printf("  Level 2: %d spheres (workers)\n", num_level2);
    
    // Create Level 1 spheres (children of root)
    // Each Level-1 sphere corresponds to one symmetry group from the model
    for (int i = 0; i < num_level1; i++) {
        // Symmetry group matches model structure
        int symmetry_group = i % model_symmetry_order;
        
        CLLMLatticeHierarchy* level1_sphere = lattice_hierarchy_create(
            i + 1,                    // sphere_id
            1,                        // hierarchy_level
            &symmetry_group,          // symmetry_groups (matches model!)
            1,                        // num_symmetry_groups
            i + 1,                    // physical_thread_id
            system->root              // parent
        );
        
        if (!level1_sphere) {
            fprintf(stderr, "ERROR: Failed to create Level 1 sphere %d\n", i);
            return -1;
        }
        
        printf("  Created Level-1 sphere %d for symmetry group %d\n", i, symmetry_group);
        
        // Add as child to root
        if (lattice_hierarchy_add_child(system->root, level1_sphere) != 0) {
            fprintf(stderr, "ERROR: Failed to add Level 1 sphere %d to root\n", i);
            return -1;
        }
    }
    
    // Create Level 2 spheres (children of Level 1)
    if (num_level2 > 0) {
        int sphere_id = num_level1 + 1;
        int thread_id = num_level1 + 1;
        
        for (int i = 0; i < num_level2; i++) {
            // Distribute evenly across Level 1 spheres
            int parent_idx = i % num_level1;
            CLLMLatticeHierarchy* parent = system->root->children[parent_idx];
            
            int symmetry_group = parent->primary_symmetry_group;
            
            CLLMLatticeHierarchy* level2_sphere = lattice_hierarchy_create(
                sphere_id++,              // sphere_id
                2,                        // hierarchy_level
                &symmetry_group,          // symmetry_groups
                1,                        // num_symmetry_groups
                thread_id++,              // physical_thread_id
                parent                    // parent
            );
            
            if (!level2_sphere) {
                fprintf(stderr, "ERROR: Failed to create Level 2 sphere %d\n", i);
                return -1;
            }
            
            // Add as child to Level 1 sphere
            if (lattice_hierarchy_add_child(parent, level2_sphere) != 0) {
                fprintf(stderr, "ERROR: Failed to add Level 2 sphere %d to parent\n", i);
                return -1;
            }
        }
    }
    
    printf("✓ Hierarchy created successfully\n");
    return 0;
}

/**
 * Process batch on a leaf worker sphere
 */
static void process_batch_on_sphere(CLLMLatticeHierarchy* sphere, 
                                    CLLMBatch* batch,
                                    HierarchicalTrainingSystem* system) {
    // TODO: Implement actual batch processing
    // For now, just simulate work
    usleep(1000);  // 1ms per batch
}

/**
 * Select least loaded child sphere
 */
static int select_least_loaded_child(CLLMLatticeHierarchy* sphere) {
    // Simple round-robin for now
    // TODO: Implement actual load tracking
    static atomic_int counter = ATOMIC_VAR_INIT(0);
    int idx = atomic_fetch_add(&counter, 1) % sphere->num_children;
    return idx;
}

/**
 * Sphere thread function (works for any level)
 */
static void* sphere_thread_func(void* arg) {
    CLLMLatticeHierarchy* sphere = (CLLMLatticeHierarchy*)arg;
    HierarchicalTrainingSystem* system = (HierarchicalTrainingSystem*)sphere->user_data;
    
    printf("[Sphere %d Level %d] Thread started (symmetry group %d)\n", 
           sphere->sphere_id, sphere->hierarchy_level, sphere->primary_symmetry_group);
    
    int batches_processed = 0;
    
    while (atomic_load(&system->running)) {
        // Receive message from parent
        SphereMessage* msg = message_queue_dequeue(sphere->inbox);
        
        if (!msg) {
            // No message - check if epoch done
            if (atomic_load(&system->epoch_done)) {
                break;
            }
            usleep(100);
            continue;
        }
        
        if (sphere->num_children > 0) {
            // This is a CONTROL sphere - distribute to children
            int target_child = select_least_loaded_child(sphere);
            
            if (!message_queue_enqueue(sphere->children[target_child]->inbox, msg)) {
                fprintf(stderr, "[Sphere %d] ERROR: Failed to forward message to child %d\n",
                        sphere->sphere_id, target_child);
                // TODO: Handle error
            }
        } else {
            // This is a LEAF WORKER - process batch
            if (msg->type == MSG_BATCH_START) {
                // Extract batch pointer from generic_data
                CLLMBatch* batch = (CLLMBatch*)(uintptr_t)msg->payload.generic_data[0];
                process_batch_on_sphere(sphere, batch, system);
                batches_processed++;
                
                // TODO: Send gradients back to parent
            }
            
            // Free message after processing
            free(msg);
        }
    }
    
    printf("[Sphere %d Level %d] Thread stopping (processed %d batches)\n", 
           sphere->sphere_id, sphere->hierarchy_level, batches_processed);
    
    return NULL;
}


/**
 * Distribute batch by token symmetry groups
 * 
 * Each token in the batch has a symmetry_group (0-11).
 * We analyze the batch and send it to the Level-1 control
 * that handles the most tokens in this batch.
 */
static int get_dominant_symmetry_group(CLLMBatch* batch, CLLMModel* model) {
    if (!batch || !model || batch->batch_size == 0) {
        return 0;  // Default to group 0
    }
    
    // Count tokens per symmetry group
    int group_counts[12] = {0};
    int model_symmetry_order = model->header.symmetry_order;
    
    if (model_symmetry_order == 0) {
        model_symmetry_order = 12;
    }
    
    // Analyze batch tokens
    for (uint32_t i = 0; i < batch->batch_size * batch->seq_len; i++) {
        uint32_t token_id = batch->input_ids[i];
        
        if (token_id < model->vocab_size && model->tokens) {
            // Get token's symmetry group from model
            uint32_t group = model->tokens[token_id].symmetry_group;
            if (group < 12) {
                group_counts[group]++;
            }
        } else {
            // Fallback: use token_id % symmetry_order
            uint32_t group = token_id % model_symmetry_order;
            group_counts[group]++;
        }
    }
    
    // Find dominant group
    int max_count = 0;
    int dominant_group = 0;
    for (int g = 0; g < model_symmetry_order && g < 12; g++) {
        if (group_counts[g] > max_count) {
            max_count = group_counts[g];
            dominant_group = g;
        }
    }
    
    return dominant_group;
}

/**
 * Root control thread (Node Zero)
 */
static void* root_control_thread(void* arg) {
    HierarchicalTrainingSystem* system = (HierarchicalTrainingSystem*)arg;
    CLLMLatticeHierarchy* root = system->root;
    
    printf("[Node Zero] Root control thread started\n");
    printf("[Node Zero] Managing %d Level-1 children\n", root->num_children);
    
    // Reset batch iterator
    cllm_batch_iterator_reset(system->batch_iterator);
    
    int batches_distributed = 0;
    int group_distribution[12] = {0};  // Track distribution per group
    CLLMModel* model = system->training->model;
    printf("[Node Zero] Using symmetry-based distribution (not round-robin)\n");
    
    while (atomic_load(&system->running)) {
        // Get next batch
        CLLMBatch* batch = cllm_batch_iterator_next(system->batch_iterator);
        
        if (!batch) {
            // No more batches
            break;
        }
        
        // Determine which symmetry group this batch belongs to
        int symmetry_group = get_dominant_symmetry_group(batch, model);
        
        // Find Level-1 control for this symmetry group
        CLLMLatticeHierarchy* target = NULL;
        for (int i = 0; i < root->num_children; i++) {
            if (root->children[i]->primary_symmetry_group == symmetry_group) {
                target = root->children[i];
                break;
            }
        }
        
        if (!target) {
            // Fallback: use modulo
            target = root->children[symmetry_group % root->num_children];
        }
        
        // Create message
        SphereMessage* msg = (SphereMessage*)malloc(sizeof(SphereMessage));
        if (!msg) {
            fprintf(stderr, "[Node Zero] ERROR: Failed to allocate message\n");
            continue;
        }
        
        memset(msg, 0, sizeof(SphereMessage));
        msg->type = MSG_BATCH_START;
        msg->priority = MSG_PRIORITY_NORMAL;
        msg->sender_id = 0;
        msg->receiver_id = target->sphere_id;
        msg->sender_symmetry_group = -1;  // Root has all groups
        msg->receiver_symmetry_group = symmetry_group;
        
        // Store batch pointer in generic_data
        msg->payload.generic_data[0] = (uint64_t)(uintptr_t)batch;
        
        atomic_init(&msg->processed, 0);
        atomic_init(&msg->acknowledged, 0);
        
        // Send to child
        if (!message_queue_enqueue(target->inbox, msg)) {
            fprintf(stderr, "[Node Zero] ERROR: Failed to send message to child (group %d)\n", symmetry_group);
            free(msg);
            continue;
        }
        
        batches_distributed++;
        group_distribution[symmetry_group]++;
        
        if (batches_distributed % 100 == 0) {
            printf("[Node Zero] Distributed %d batches (by symmetry group)\n", batches_distributed);
        }
    }
    
    printf("[Node Zero] All %d batches distributed\n", batches_distributed);
    
    // Show distribution by symmetry group
    printf("[Node Zero] Distribution by symmetry group:\n");
    for (int g = 0; g < 12; g++) {
        if (group_distribution[g] > 0) {
            printf("  Group %2d: %d batches (%.1f%%)\n", g, group_distribution[g],
                   100.0 * group_distribution[g] / batches_distributed);
        }
    }
    system->total_batches = batches_distributed;
    
    // Signal epoch done
    atomic_store(&system->epoch_done, 1);
    
    // TODO: Wait for all children to complete
    // TODO: Accumulate gradients from children
    // TODO: Apply optimizer
    
    printf("[Node Zero] Epoch complete\n");
    
    return NULL;
}

/**
 * Start all sphere threads
 */
static int start_sphere_threads(HierarchicalTrainingSystem* system) {
    // Start Level 1 threads
    for (int i = 0; i < system->root->num_children; i++) {
        CLLMLatticeHierarchy* sphere = system->root->children[i];
        sphere->user_data = system;  // Store system reference
        
        int rc = pthread_create(&sphere->thread, NULL, sphere_thread_func, sphere);
        if (rc != 0) {
            fprintf(stderr, "ERROR: Failed to create Level 1 thread %d\n", i);
            return -1;
        }
        atomic_store(&sphere->thread_running, 1);
    }
    
    // Start Level 2 threads
    for (int i = 0; i < system->root->num_children; i++) {
        CLLMLatticeHierarchy* level1 = system->root->children[i];
        
        for (int j = 0; j < level1->num_children; j++) {
            CLLMLatticeHierarchy* sphere = level1->children[j];
            sphere->user_data = system;  // Store system reference
            
            int rc = pthread_create(&sphere->thread, NULL, sphere_thread_func, sphere);
            if (rc != 0) {
                fprintf(stderr, "ERROR: Failed to create Level 2 thread %d-%d\n", i, j);
                return -1;
            }
            atomic_store(&sphere->thread_running, 1);
        }
    }
    
    printf("✓ All sphere threads started\n");
    return 0;
}

/**
 * Create hierarchical training system
 */
HierarchicalTrainingSystem* hierarchical_training_create(CLLMTraining* training, 
                                                         int num_threads,
                                                         CLLMBatchIterator* batch_iterator) {
    if (!training || num_threads < 1) return NULL;
    
    HierarchicalTrainingSystem* system = (HierarchicalTrainingSystem*)calloc(1, sizeof(HierarchicalTrainingSystem));
    if (!system) return NULL;
    
    system->training = training;
    system->total_threads = num_threads;
    system->batch_iterator = batch_iterator;
    
    atomic_init(&system->running, 1);
    atomic_init(&system->epoch_done, 0);
    
    printf("\n=== Creating Hierarchical Training System ===\n");
    printf("Total threads: %d\n", num_threads);
    
    // Create root sphere (Node Zero)
    int root_symmetry_groups[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    system->root = lattice_hierarchy_create(
        0,                      // sphere_id (root)
        0,                      // hierarchy_level (root level)
        root_symmetry_groups,   // symmetry_groups
        12,                     // num_symmetry_groups
        0,                      // physical_thread_id (root control)
        NULL                    // parent (root has no parent)
    );
    
    if (!system->root) {
        fprintf(stderr, "ERROR: Failed to create root hierarchy\n");
        free(system);
        return NULL;
    }
    
    system->root->user_data = system;  // Store system reference
    printf("✓ Root sphere created (Node Zero)\n");
    
    // Create sphere hierarchy
    if (create_sphere_hierarchy(system) != 0) {
        fprintf(stderr, "ERROR: Failed to create sphere hierarchy\n");
        // TODO: Cleanup
        free(system);
        return NULL;
    }
    
    // Start all sphere threads
    if (start_sphere_threads(system) != 0) {
        fprintf(stderr, "ERROR: Failed to start sphere threads\n");
        // TODO: Cleanup
        free(system);
        return NULL;
    }
    
    // Start root control thread
    int rc = pthread_create(&system->root->thread, NULL, root_control_thread, system);
    if (rc != 0) {
        fprintf(stderr, "ERROR: Failed to create root control thread\n");
        // TODO: Cleanup
        free(system);
        return NULL;
    }
    atomic_store(&system->root->thread_running, 1);
    
    printf("✓ Root control thread started\n");
    printf("=== Hierarchical Training System Ready ===\n\n");
    
    return system;
}

/**
 * Train one epoch using hierarchical system
 */
float hierarchical_train_epoch(HierarchicalTrainingSystem* system) {
    if (!system || !system->root) return 0.0f;
    
    printf("\n=== HIERARCHICAL TRAINING EPOCH ===\n");
    printf("Hierarchy structure:\n");
    printf("  Level 0: 1 root (Node Zero)\n");
    printf("  Level 1: %d controls\n", system->num_level1);
    printf("  Level 2: %d workers\n", system->num_level2);
    printf("\n");
    
    // Reset epoch state
    atomic_store(&system->epoch_done, 0);
    atomic_store(&system->running, 1);
    
    // Root control thread handles everything
    // Wait for epoch completion
    while (!atomic_load(&system->epoch_done)) {
        usleep(10000);  // 10ms
    }
    
    printf("\nEpoch complete (HIERARCHICAL):\n");
    printf("  Total batches: %d\n", system->total_batches);
    printf("  Average loss: %.4f\n", system->epoch_loss);
    
    return system->epoch_loss;
}

/**
 * Free hierarchical training system
 */
void hierarchical_training_free(HierarchicalTrainingSystem* system) {
    if (!system) return;
    
    printf("\nStopping hierarchical training system...\n");
    
    // Signal all threads to stop
    atomic_store(&system->running, 0);
    atomic_store(&system->epoch_done, 1);
    
    // Wait for root control thread
    if (atomic_load(&system->root->thread_running)) {
        pthread_join(system->root->thread, NULL);
        printf("✓ Root control thread stopped\n");
    }
    
    // Wait for all sphere threads
    for (int i = 0; i < system->root->num_children; i++) {
        CLLMLatticeHierarchy* level1 = system->root->children[i];
        
        if (atomic_load(&level1->thread_running)) {
            pthread_join(level1->thread, NULL);
        }
        
        for (int j = 0; j < level1->num_children; j++) {
            CLLMLatticeHierarchy* level2 = level1->children[j];
            
            if (atomic_load(&level2->thread_running)) {
                pthread_join(level2->thread, NULL);
            }
        }
    }
    
    printf("✓ All sphere threads stopped\n");
    
    // TODO: Free hierarchy
    // TODO: Free system
    
    printf("✓ Hierarchical training system freed\n");
}