#include "cllm_recursive_spheres.h"
#include "cllm_simd_gradient_ops.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

// Internal helper functions
static RecursiveSphere* sphere_create(
    uint32_t sphere_id,
    uint32_t level,
    uint32_t index_at_level,
    RecursiveSphere *parent,
    SphereType type
);

static void sphere_destroy(RecursiveSphere *sphere);
static void* sphere_worker_thread(void *arg);
static int sphere_process_batches(RecursiveSphere *sphere, SphereHierarchy *hierarchy);

// Calculate total spheres at depth: (12^(depth+1) - 1) / 11
uint32_t sphere_hierarchy_count_at_depth(uint32_t depth) {
    if (depth == 0) return 1;
    
    // Calculate 12^(depth+1)
    uint64_t power = 1;
    for (uint32_t i = 0; i <= depth; i++) {
        power *= 12;
    }
    
    return (uint32_t)((power - 1) / 11);
}

// Get optimal depth for batch count
uint32_t sphere_hierarchy_optimal_depth(uint32_t total_batches) {
    if (total_batches <= 1) return 0;
    if (total_batches <= 13) return 1;
    if (total_batches <= 157) return 2;
    if (total_batches <= 1885) return 3;
    if (total_batches <= 22621) return 4;
    
    // For larger batch counts, calculate dynamically
    uint32_t depth = 0;
    while (depth < MAX_RECURSION_DEPTH) {
        uint32_t spheres = sphere_hierarchy_count_at_depth(depth);
        if (spheres >= total_batches) break;
        depth++;
    }
    
    return depth;
}

// Create individual sphere
static RecursiveSphere* sphere_create(
    uint32_t sphere_id,
    uint32_t level,
    uint32_t index_at_level,
    RecursiveSphere *parent,
    SphereType type
) {
    RecursiveSphere *sphere = calloc(1, sizeof(RecursiveSphere));
    if (!sphere) return NULL;
    
    sphere->sphere_id = sphere_id;
    sphere->level = level;
    sphere->index_at_level = index_at_level;
    sphere->parent = parent;
    sphere->type = type;
    sphere->state = SPHERE_STATE_IDLE;
    sphere->num_children = 0;
    sphere->thread_active = false;
    
    pthread_mutex_init(&sphere->state_mutex, NULL);
    pthread_cond_init(&sphere->work_ready, NULL);
    pthread_cond_init(&sphere->work_complete, NULL);
    
    return sphere;
}

// Destroy sphere
static void sphere_destroy(RecursiveSphere *sphere) {
    if (!sphere) return;
    
    // Destroy children recursively
    for (uint32_t i = 0; i < sphere->num_children; i++) {
        sphere_destroy(sphere->children[i]);
    }
    
    // Free gradient segment if allocated
    if (sphere->gradient_segment) {
        free(sphere->gradient_segment);
    }
    
    pthread_mutex_destroy(&sphere->state_mutex);
    pthread_cond_destroy(&sphere->work_ready);
    pthread_cond_destroy(&sphere->work_complete);
    
    free(sphere);
}

// Create sphere hierarchy
SphereHierarchy* sphere_hierarchy_create(
    CLLMTraining *training_state,
    CLLMBatchIterator *batch_iterator,
    uint32_t max_depth,
    uint32_t total_batches
) {
    if (!training_state || !batch_iterator) return NULL;
    if (max_depth > MAX_RECURSION_DEPTH) max_depth = MAX_RECURSION_DEPTH;
    
    SphereHierarchy *hierarchy = calloc(1, sizeof(SphereHierarchy));
    if (!hierarchy) return NULL;
    
    hierarchy->max_depth = max_depth;
    hierarchy->total_batches = total_batches;
    hierarchy->shared_training_state = training_state;
    hierarchy->batch_iterator = batch_iterator;
    hierarchy->training_complete = false;
    
    // Calculate total spheres needed
    hierarchy->total_spheres = sphere_hierarchy_count_at_depth(max_depth);
    
    // Allocate sphere array for fast lookup
    hierarchy->sphere_array = calloc(hierarchy->total_spheres, sizeof(RecursiveSphere*));
    if (!hierarchy->sphere_array) {
        free(hierarchy);
        return NULL;
    }
    hierarchy->sphere_array_size = hierarchy->total_spheres;
    
    // Allocate global gradient buffer
    hierarchy->gradient_size = training_state->model->total_params;
    hierarchy->global_gradients = calloc(hierarchy->gradient_size, sizeof(float));
    if (!hierarchy->global_gradients) {
        free(hierarchy->sphere_array);
        free(hierarchy);
        return NULL;
    }
    
    // Create root sphere
    hierarchy->root = sphere_create(0, 0, 0, NULL, SPHERE_TYPE_ROOT);
    if (!hierarchy->root) {
        free(hierarchy->global_gradients);
        free(hierarchy->sphere_array);
        free(hierarchy);
        return NULL;
    }
    hierarchy->sphere_array[0] = hierarchy->root;
    
    pthread_mutex_init(&hierarchy->hierarchy_mutex, NULL);
    
    printf("[Sphere Hierarchy] Created with max_depth=%u, total_spheres=%u, total_batches=%u\n",
           max_depth, hierarchy->total_spheres, total_batches);
    
    return hierarchy;
}

// Destroy hierarchy
void sphere_hierarchy_destroy(SphereHierarchy *hierarchy) {
    if (!hierarchy) return;
    
    sphere_destroy(hierarchy->root);
    free(hierarchy->global_gradients);
    free(hierarchy->sphere_array);
    pthread_mutex_destroy(&hierarchy->hierarchy_mutex);
    free(hierarchy);
}

// Recursively spawn children
int sphere_spawn_children(
    RecursiveSphere *parent,
    SphereHierarchy *hierarchy,
    uint32_t remaining_depth
) {
    if (!parent || !hierarchy || remaining_depth == 0) return 0;
    
    // Calculate how many children to spawn (up to 12)
    uint32_t children_to_spawn = 12;
    
    // Don't spawn more spheres than we have batches
    uint32_t spheres_so_far = 0;
    for (uint32_t i = 0; i < hierarchy->sphere_array_size; i++) {
        if (hierarchy->sphere_array[i]) spheres_so_far++;
    }
    
    if (spheres_so_far >= hierarchy->total_batches) {
        return 0; // Already have enough spheres
    }
    
    // Spawn children
    for (uint32_t i = 0; i < children_to_spawn; i++) {
        // Find next available sphere ID
        uint32_t child_id = 0;
        for (uint32_t j = 0; j < hierarchy->sphere_array_size; j++) {
            if (!hierarchy->sphere_array[j]) {
                child_id = j;
                break;
            }
        }
        
        if (child_id >= hierarchy->sphere_array_size) break;
        
        // Determine child type
        SphereType child_type = (remaining_depth > 1) ? SPHERE_TYPE_CONTROL : SPHERE_TYPE_WORKER;
        
        // Create child sphere
        RecursiveSphere *child = sphere_create(
            child_id,
            parent->level + 1,
            i,
            parent,
            child_type
        );
        
        if (!child) {
            fprintf(stderr, "[Sphere Hierarchy] Failed to create child sphere\n");
            return -1;
        }
        
        parent->children[i] = child;
        parent->num_children++;
        hierarchy->sphere_array[child_id] = child;
        
        // Recursively spawn grandchildren if this is a control sphere
        if (child_type == SPHERE_TYPE_CONTROL) {
            sphere_spawn_children(child, hierarchy, remaining_depth - 1);
        }
        
        // Stop if we have enough spheres
        spheres_so_far++;
        if (spheres_so_far >= hierarchy->total_batches) break;
    }
    
    return 0;
}

// Distribute batches across spheres
static void distribute_batches(SphereHierarchy *hierarchy) {
    // Collect all leaf (worker) spheres
    RecursiveSphere **workers = malloc(hierarchy->total_spheres * sizeof(RecursiveSphere*));
    uint32_t num_workers = 0;
    
    for (uint32_t i = 0; i < hierarchy->sphere_array_size; i++) {
        RecursiveSphere *sphere = hierarchy->sphere_array[i];
        if (sphere && sphere->type == SPHERE_TYPE_WORKER) {
            workers[num_workers++] = sphere;
        }
    }
    
    if (num_workers == 0) {
        free(workers);
        return;
    }
    
    // Distribute batches evenly
    uint32_t batches_per_worker = hierarchy->total_batches / num_workers;
    uint32_t remainder = hierarchy->total_batches % num_workers;
    
    uint32_t current_batch = 0;
    for (uint32_t i = 0; i < num_workers; i++) {
        workers[i]->start_batch = current_batch;
        workers[i]->end_batch = current_batch + batches_per_worker;
        
        // Distribute remainder
        if (i < remainder) {
            workers[i]->end_batch++;
        }
        
        current_batch = workers[i]->end_batch;
        
        // Allocate gradient segment
        uint32_t segment_size = hierarchy->gradient_size / num_workers;
        workers[i]->segment_size = segment_size;
        workers[i]->segment_offset = i * segment_size;
        workers[i]->gradient_segment = calloc(segment_size, sizeof(float));
        
        printf("[Sphere %u] Level %u, Batches [%u, %u), Gradient segment [%u, %u)\n",
               workers[i]->sphere_id, workers[i]->level,
               workers[i]->start_batch, workers[i]->end_batch,
               workers[i]->segment_offset, workers[i]->segment_offset + segment_size);
    }
    
    free(workers);
}

// Worker thread function
static void* sphere_worker_thread(void *arg) {
    RecursiveSphere *sphere = (RecursiveSphere*)arg;
    SphereHierarchy *hierarchy = NULL;
    
    // Find hierarchy (stored in root's parent pointer temporarily)
    for (RecursiveSphere *s = sphere; s; s = s->parent) {
        if (s->level == 0) {
            hierarchy = (SphereHierarchy*)s->parent; // Hack: stored here temporarily
            break;
        }
    }
    
    if (!hierarchy) {
        fprintf(stderr, "[Sphere %u] ERROR: Cannot find hierarchy\n", sphere->sphere_id);
        return NULL;
    }
    
    // Process batches
    sphere->state = SPHERE_STATE_WORKING;
    int result = sphere_process_batches(sphere, hierarchy);
    
    if (result == 0) {
        sphere->state = SPHERE_STATE_COMPLETE;
    } else {
        sphere->state = SPHERE_STATE_ERROR;
    }
    
    return NULL;
}

// Process batches for a sphere
static int sphere_process_batches(RecursiveSphere *sphere, SphereHierarchy *hierarchy) {
    if (!sphere || !hierarchy) return -1;
    
    CLLMTraining *training = hierarchy->shared_training_state;
    CLLMBatchIterator *iterator = hierarchy->batch_iterator;
    
    // Process assigned batches
    for (uint32_t batch_idx = sphere->start_batch; batch_idx < sphere->end_batch; batch_idx++) {
        // Get next batch
        CLLMBatch *batch = cllm_batch_iterator_next(iterator);
        if (!batch) {
            fprintf(stderr, "[Sphere %u] Failed to get batch %u\n", sphere->sphere_id, batch_idx);
            return -1;
        }
        
        // Forward pass (read-only, lock-free)
        cllm_forward_training(training, batch->input_ids, batch->batch_size);
        
        // Compute loss and backward pass
        cllm_backward(training, batch->input_ids, batch->target_ids, batch->batch_size);
        
        // Copy gradients to local segment (simple memcpy for now)
        if (sphere->gradient_segment && training->model) {
            uint32_t offset = sphere->segment_offset;
            uint32_t size = sphere->segment_size;
            
            // Accumulate gradients into local segment
            for (uint32_t i = 0; i < size && (offset + i) < training->model->total_params; i++) {
                sphere->gradient_segment[i] += training->model->gradients[offset + i];
            }
        }
        
        sphere->batches_processed++;
        sphere->operations_completed += batch->batch_size * batch->seq_len;
    }
    
    return 0;
}

// Accumulate gradients from children
int sphere_accumulate_gradients(RecursiveSphere *parent) {
    if (!parent || parent->num_children == 0) return 0;
    
    // Accumulate from all children
    for (uint32_t i = 0; i < parent->num_children; i++) {
        RecursiveSphere *child = parent->children[i];
        if (!child) continue;
        
        // If child is a control sphere, recursively accumulate its children first
        if (child->type == SPHERE_TYPE_CONTROL) {
            sphere_accumulate_gradients(child);
        }
        
        // Accumulate child's gradient segment into parent's segment
        if (child->gradient_segment && parent->gradient_segment) {
            uint32_t size = child->segment_size < parent->segment_size ? 
                           child->segment_size : parent->segment_size;
            for (uint32_t j = 0; j < size; j++) {
                parent->gradient_segment[j] += child->gradient_segment[j];
            }
        }
    }
    
    return 0;
}

// Train across hierarchy
int sphere_hierarchy_train(SphereHierarchy *hierarchy) {
    if (!hierarchy || !hierarchy->root) return -1;
    
    printf("[Sphere Hierarchy] Starting training with %u spheres at depth %u\n",
           hierarchy->total_spheres, hierarchy->max_depth);
    
    // Spawn child spheres recursively
    if (sphere_spawn_children(hierarchy->root, hierarchy, hierarchy->max_depth) != 0) {
        fprintf(stderr, "[Sphere Hierarchy] Failed to spawn children\n");
        return -1;
    }
    
    // Distribute batches across worker spheres
    distribute_batches(hierarchy);
    
    // Hack: Store hierarchy pointer in root's parent for worker threads to find
    hierarchy->root->parent = (RecursiveSphere*)hierarchy;
    
    // Launch worker threads for all leaf spheres
    uint32_t threads_launched = 0;
    for (uint32_t i = 0; i < hierarchy->sphere_array_size; i++) {
        RecursiveSphere *sphere = hierarchy->sphere_array[i];
        if (sphere && sphere->type == SPHERE_TYPE_WORKER) {
            if (pthread_create(&sphere->thread, NULL, sphere_worker_thread, sphere) == 0) {
                sphere->thread_active = true;
                threads_launched++;
            } else {
                fprintf(stderr, "[Sphere %u] Failed to launch thread\n", sphere->sphere_id);
            }
        }
    }
    
    printf("[Sphere Hierarchy] Launched %u worker threads\n", threads_launched);
    
    // Wait for all workers to complete
    for (uint32_t i = 0; i < hierarchy->sphere_array_size; i++) {
        RecursiveSphere *sphere = hierarchy->sphere_array[i];
        if (sphere && sphere->thread_active) {
            pthread_join(sphere->thread, NULL);
            sphere->thread_active = false;
        }
    }
    
    printf("[Sphere Hierarchy] All workers complete, accumulating gradients...\n");
    
    // Accumulate gradients up the tree
    sphere_accumulate_gradients(hierarchy->root);
    
    // Copy final gradients to global buffer
    if (hierarchy->root->gradient_segment) {
        memcpy(hierarchy->global_gradients,
               hierarchy->root->gradient_segment,
               hierarchy->root->segment_size * sizeof(float));
    }
    
    hierarchy->training_complete = true;
    
    printf("[Sphere Hierarchy] Training complete!\n");
    
    return 0;
}

// Print statistics
void sphere_hierarchy_print_stats(SphereHierarchy *hierarchy) {
    if (!hierarchy) return;
    
    printf("\n=== Sphere Hierarchy Statistics ===\n");
    printf("Max Depth: %u\n", hierarchy->max_depth);
    printf("Total Spheres: %u\n", hierarchy->total_spheres);
    printf("Total Batches: %u\n", hierarchy->total_batches);
    printf("Gradient Size: %u floats\n", hierarchy->gradient_size);
    
    // Count spheres by type
    uint32_t root_count = 0, control_count = 0, worker_count = 0;
    for (uint32_t i = 0; i < hierarchy->sphere_array_size; i++) {
        RecursiveSphere *sphere = hierarchy->sphere_array[i];
        if (!sphere) continue;
        
        switch (sphere->type) {
            case SPHERE_TYPE_ROOT: root_count++; break;
            case SPHERE_TYPE_CONTROL: control_count++; break;
            case SPHERE_TYPE_WORKER: worker_count++; break;
        }
    }
    
    printf("Root Spheres: %u\n", root_count);
    printf("Control Spheres: %u\n", control_count);
    printf("Worker Spheres: %u\n", worker_count);
    
    // Print per-level statistics
    printf("\nPer-Level Breakdown:\n");
    for (uint32_t level = 0; level <= hierarchy->max_depth; level++) {
        uint32_t count = 0;
        uint64_t total_ops = 0;
        
        for (uint32_t i = 0; i < hierarchy->sphere_array_size; i++) {
            RecursiveSphere *sphere = hierarchy->sphere_array[i];
            if (sphere && sphere->level == level) {
                count++;
                total_ops += sphere->operations_completed;
            }
        }
        
        printf("  Level %u: %u spheres, %lu operations\n", level, count, total_ops);
    }
    
    printf("===================================\n\n");
}