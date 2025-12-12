# WIRING EXECUTION PLAN
## Step-by-Step Implementation Guide

**Based on:** COMPREHENSIVE_ARCHITECTURE_ANALYSIS.md  
**Date:** 2024-12-05  
**Estimated Time:** 40-60 hours of focused work

---

## PHASE 1: CRITICAL WIRING (Priority: IMMEDIATE)

### Task 1.1: Wire Entropy Allocation into Thread Creation
**Estimated Time:** 2-3 hours  
**Files to Modify:**
- `src/ai/cllm_training_threaded.c`

**Implementation Steps:**

1. **In `sphere_spawn_children()` function:**
```c
// After determining num_children, BEFORE creating children
if (parent->system->entropy_allocation && parent->system->entropy_context) {
    AllocationConfig config;
    allocation_config_init_default(&config);
    config.strategy = ALLOCATION_ADAPTIVE;
    config.enforce_12fold = true;
    
    // Calculate entropy-based allocation
    bool success = calculate_thread_allocation(
        parent->system->entropy_allocation,
        parent->system->entropy_context,
        &config,
        num_children * 12  // Total threads to allocate
    );
    
    if (success) {
        printf("[Sphere %d] Entropy allocation calculated:\n", parent->sphere_id);
        print_allocation_plan(parent->system->entropy_allocation);
        
        // Store allocation plan for children to use
        parent->allocation_plan = parent->system->entropy_allocation;
    } else {
        fprintf(stderr, "[WARNING] Entropy allocation failed, using default\n");
    }
}
```

2. **In child context creation loop:**
```c
for (int i = 0; i < num_children; i++) {
    int child_symmetry_group = i;
    
    // Get dimension-specific allocation if available
    int allocated_threads = 1;  // Default
    if (parent->allocation_plan) {
        const DimensionAllocation* dim_alloc = get_dimension_allocation(
            parent->allocation_plan,
            child_symmetry_group
        );
        if (dim_alloc && dim_alloc->is_active) {
            allocated_threads = dim_alloc->adjusted_threads;
        }
    }
    
    // Create child with allocated thread count
    parent->children[i] = sphere_context_create_with_allocation(
        child_id,
        child_symmetry_group,
        parent->gradient_size,
        parent->shared_gradients,
        parent->system->num_worker_spheres,
        allocated_threads  // NEW PARAMETER
    );
    
    printf("[Sphere %d -> Child %d] Allocated %d threads (dimension %d)\n",
           parent->sphere_id, child_id, allocated_threads, child_symmetry_group);
}
```

3. **Add new function `sphere_context_create_with_allocation()`:**
```c
static SphereTrainingContext* sphere_context_create_with_allocation(
    int sphere_id,
    int symmetry_group,
    size_t gradient_size,
    SharedMemoryRegion* shared_gradients,
    int num_workers,
    int allocated_threads
) {
    SphereTrainingContext* ctx = sphere_context_create(
        sphere_id, symmetry_group, gradient_size, shared_gradients, num_workers
    );
    
    if (ctx) {
        ctx->allocated_threads = allocated_threads;
        ctx->can_spawn_children = (allocated_threads >= 12);
    }
    
    return ctx;
}
```

4. **Update `SphereTrainingContext` structure:**
```c
struct SphereTrainingContext {
    // ... existing fields ...
    
    // PHASE 6: Entropy-based allocation
    int allocated_threads;           // Threads allocated to this sphere
    bool can_spawn_children;         // Can spawn based on allocation
    ThreadAllocationPlan* allocation_plan;  // Allocation plan for children
};
```

**Testing:**
- [ ] Build succeeds
- [ ] Entropy allocation is calculated
- [ ] Children receive dimension-specific thread counts
- [ ] Allocation respects 12-fold symmetry
- [ ] Logging shows allocation decisions

**Verification:**
```bash
# Run training and check logs
./app/hyper_prime_spiral
# Look for: "Entropy allocation calculated"
# Look for: "Allocated X threads (dimension Y)"
```

---

### Task 1.2: Wire Entropy Work Distribution into Batch Processing
**Estimated Time:** 3-4 hours  
**Files to Modify:**
- `src/ai/cllm_training_threaded.c`
- `src/ai/cllm_entropy_work_distribution.c`

**Implementation Steps:**

1. **Add work distribution context to `ThreadedTrainingSystem`:**
```c
struct ThreadedTrainingSystem {
    // ... existing fields ...
    
    // PHASE 6: Entropy work distribution
    EntropyWorkDistribution* work_distribution;  // NEW
};
```

2. **Initialize in `threaded_training_create()`:**
```c
// After entropy_allocation initialization
system->work_distribution = (EntropyWorkDistribution*)calloc(1, sizeof(EntropyWorkDistribution));
if (system->work_distribution && system->entropy_context) {
    EntropyWorkConfig work_config;
    entropy_work_config_init_default(&work_config);
    
    if (entropy_work_distribution_init(
        system->work_distribution,
        system->entropy_context,
        &work_config,
        system->num_worker_spheres
    )) {
        printf("  ✓ Entropy work distribution initialized\n");
    } else {
        free(system->work_distribution);
        system->work_distribution = NULL;
    }
}
```

3. **Wire into batch distribution (in `control_thread_func()` or work queue):**
```c
// BEFORE distributing batches to workers
if (system->work_distribution && system->entropy_context) {
    // Calculate entropy-based weights
    EntropyWeights weights;
    bool success = calculate_entropy_weights(
        &weights,
        system->entropy_context,
        12  // 12 dimensions
    );
    
    if (success) {
        // Distribute work based on entropy weights
        for (int i = 0; i < system->num_worker_spheres; i++) {
            int dimension = i % 12;
            double weight = weights.dimension_weights[dimension];
            
            // Assign more batches to high-entropy dimensions
            int batches_for_worker = (int)(total_batches * weight);
            system->sphere_contexts[i]->assigned_batches = batches_for_worker;
            
            printf("[Worker %d] Assigned %d batches (weight=%.3f)\n",
                   i, batches_for_worker, weight);
        }
    }
}
```

4. **Update batch assignment in worker threads:**
```c
// In sphere_worker_thread_dynamic()
while (atomic_load(&ctx->system->running)) {
    // Check if we have assigned batches
    if (ctx->assigned_batches > 0) {
        // Process assigned batch
        CLLMBatch* batch = get_next_batch_for_worker(ctx);
        if (batch) {
            sphere_process_batch(ctx, ctx->system->training);
            ctx->assigned_batches--;
        }
    }
    
    // ... rest of worker loop ...
}
```

5. **Cleanup in `threaded_training_free()`:**
```c
if (system->work_distribution) {
    entropy_work_distribution_destroy(system->work_distribution);
    free(system->work_distribution);
    printf("  ✓ Entropy work distribution freed\n");
}
```

**Testing:**
- [ ] Build succeeds
- [ ] Entropy weights are calculated
- [ ] Batches distributed according to weights
- [ ] High-entropy dimensions get more work
- [ ] Total batches sum correctly

**Verification:**
```bash
# Check batch distribution in logs
grep "Assigned.*batches" training.log
# Verify high-entropy dimensions get more batches
```

---

## PHASE 2: MATHEMATICAL FORMULA INTEGRATION (Priority: HIGH)

### Task 2.1: Wire L_lattice_complete() into Embeddings
**Estimated Time:** 4-5 hours  
**Files to Modify:**
- `src/ai/cllm_lattice_embeddings.c`
- `src/ai/cllm_pure_embeddings.c`

**Implementation Steps:**

1. **Add formula context to embedding initialization:**
```c
// In cllm_lattice_embeddings_init()
#include "ai/cllm_lattice_formula.h"

// Create formula context
LatticeFormulaContext* formula_ctx = lattice_formula_context_create(true);  // verbose
if (!formula_ctx) {
    fprintf(stderr, "Failed to create formula context\n");
    return -1;
}
```

2. **Replace embedding calculation:**
```c
// BEFORE (old calculation):
for (int n = 0; n < vocab_size; n++) {
    for (int d = 0; d < embedding_dim; d++) {
        embeddings[n * embedding_dim + d] = some_simple_calculation(n, d);
    }
}

// AFTER (using complete formula):
for (int n = 0; n < vocab_size; n++) {
    // Get phonetic value for token if available
    const char* token_phonetic = get_token_phonetic(tokenizer, n);
    
    for (int d = 0; d < embedding_dim; d++) {
        // Use complete L(n,d,k,λ,ω,ψ) formula
        LatticeFormulaParams params = {
            .n = n,
            .d = d,
            .k = 0,  // Base level
            .lambda = token_phonetic ? token_phonetic : "default",
            .omega = CYMATIC_432HZ,  // Use 432 Hz
            .psi = {2, 1}  // Plimpton generators (2,1) -> (3,4,5)
        };
        
        double value = L_lattice_complete(&params, formula_ctx);
        embeddings[n * embedding_dim + d] = (float)value;
        
        // Optional: Log first few for verification
        if (n < 5 && d < 5) {
            printf("  Embedding[%d][%d] = %.6f\n", n, d, value);
        }
    }
}
```

3. **Add phonetic value lookup:**
```c
static const char* get_token_phonetic(CLLMTokenizer* tokenizer, int token_id) {
    // Get token string
    const char* token = cllm_tokenizer_decode_single(tokenizer, token_id);
    if (!token) return NULL;
    
    // Look up phonetic value
    double phonetic_val = get_phonetic_value(token);
    
    // Return phonetic string representation
    // This is a simplified version - you may want a proper mapping
    if (phonetic_val == 3.0) return "dub";
    if (phonetic_val == 5.0) return "knbt";
    // ... etc
    
    return "default";
}
```

4. **Cleanup:**
```c
// After embedding initialization
lattice_formula_context_free(formula_ctx);
```

**Testing:**
- [ ] Build succeeds
- [ ] Embeddings are initialized with formula
- [ ] Values are in reasonable range
- [ ] Phonetic values are applied
- [ ] No NaN or Inf values

**Verification:**
```bash
# Check embedding values
./tools/init_lattice_embeddings --vocab-size 1000 --embedding-dim 128
# Verify output shows formula-based values
```

---

### Task 2.2: Wire angular_position_complete() into Attention
**Estimated Time:** 3-4 hours  
**Files to Modify:**
- `src/ai/cllm_angular_attention.c`
- `src/ai/cllm_crystalline_attention.c`

**Implementation Steps:**

1. **Add formula includes:**
```c
#include "ai/cllm_lattice_formula.h"
#include "ai/cllm_cymatic_frequencies.h"
#include "plimpton_322.h"
```

2. **Replace angular position calculation:**
```c
// In attention score calculation
for (int i = 0; i < seq_len; i++) {
    for (int j = 0; j < seq_len; j++) {
        // BEFORE (simple calculation):
        double theta = angular_position_calculate(i, j, "default");
        
        // AFTER (complete formula):
        double theta = angular_position_complete(
            i,                    // n
            j,                    // k
            "default",            // lambda
            CYMATIC_432HZ,        // omega
            plimpton_generators   // psi (2,1)
        );
        
        // Apply to attention score
        double angular_factor = prime_cos(theta);
        attention_scores[i * seq_len + j] *= angular_factor;
        
        // Optional: Add cymatic modulation
        double cymatic_mod = cymatic_correction(CYMATIC_432HZ, theta);
        attention_scores[i * seq_len + j] *= (1.0 + 0.1 * cymatic_mod);
    }
}
```

3. **Add Plimpton generators:**
```c
// At function start
int plimpton_generators[2] = {2, 1};  // Generates (3,4,5) triple
```

4. **Add logging for verification:**
```c
// Log first few angular positions
if (i < 3 && j < 3) {
    printf("  θ(%d,%d) = %.6f, cos(θ) = %.6f\n", 
           i, j, theta, prime_cos(theta));
}
```

**Testing:**
- [ ] Build succeeds
- [ ] Angular positions calculated correctly
- [ ] Attention scores modified appropriately
- [ ] No NaN or Inf in attention
- [ ] Cymatic modulation applied

**Verification:**
```bash
# Run attention benchmark
./tools/benchmark_ntt_attention
# Check angular position values in output
```

---

## PHASE 3: MEMORY SYSTEM INTEGRATION (Priority: HIGH)

### Task 3.1: Replace local_gradients with Crystalline Memory
**Estimated Time:** 5-6 hours  
**Files to Modify:**
- `src/ai/cllm_training_threaded.c`

**Implementation Steps:**

1. **Remove local_gradients from `SphereTrainingContext`:**
```c
struct SphereTrainingContext {
    // REMOVE:
    // double* local_gradients;  // TODO: Remove after shared memory integration
    // size_t gradient_size;
    
    // KEEP (already exists):
    CrystallineMemoryBlock* crystalline_memory;
};
```

2. **Update `sphere_context_create()` to use crystalline memory:**
```c
// REMOVE:
// ctx->local_gradients = (double*)calloc(gradient_size, sizeof(double));

// REPLACE WITH (already exists but ensure it's used):
ctx->crystalline_memory = crystalline_memory_create(
    gradient_size * sizeof(double),  // total_size
    symmetry_group                    // owner_id
);

if (!ctx->crystalline_memory) {
    fprintf(stderr, "Failed to create crystalline memory for sphere %d\n", sphere_id);
    free(ctx);
    return NULL;
}

printf("[Sphere %d] Created crystalline memory: %zu bytes, 12 segments\n",
       sphere_id, gradient_size * sizeof(double));
```

3. **Update `sphere_process_batch()` to use crystalline memory:**
```c
// BEFORE (using local_gradients):
memset(ctx->local_gradients, 0, ctx->gradient_size * sizeof(double));

// AFTER (using crystalline memory):
double* zeros = (double*)calloc(ctx->gradient_size, sizeof(double));
for (int seg = 0; seg < 12; seg++) {
    crystalline_segment_write(
        ctx->crystalline_memory,
        seg,
        0,  // offset
        zeros,
        ctx->gradient_size * sizeof(double) / 12  // size per segment
    );
}
free(zeros);
```

4. **Update gradient writes:**
```c
// BEFORE:
ctx->local_gradients[i] += gradient_value;

// AFTER:
int segment = i / (ctx->gradient_size / 12);
size_t offset = (i % (ctx->gradient_size / 12)) * sizeof(double);
double gradient_value = /* ... */;

crystalline_segment_write(
    ctx->crystalline_memory,
    segment,
    offset,
    &gradient_value,
    sizeof(double)
);
```

5. **Update gradient reads in accumulation:**
```c
// BEFORE:
for (size_t i = ctx->gradient_segment_start; i < ctx->gradient_segment_end; i++) {
    system->accumulated_gradients[i] = ctx->local_gradients[i];
}

// AFTER:
for (int seg = 0; seg < 12; seg++) {
    double* segment_data = (double*)malloc(segment_size);
    
    crystalline_segment_read(
        ctx->crystalline_memory,
        seg,
        0,
        segment_data,
        segment_size
    );
    
    // Copy to accumulated gradients
    size_t base_idx = seg * (ctx->gradient_size / 12);
    for (size_t i = 0; i < segment_size / sizeof(double); i++) {
        system->accumulated_gradients[base_idx + i] = segment_data[i];
    }
    
    free(segment_data);
}
```

6. **Update `sphere_context_free()`:**
```c
// REMOVE:
// free(ctx->local_gradients);

// KEEP (already exists):
if (ctx->crystalline_memory) {
    crystalline_memory_free(ctx->crystalline_memory);
}
```

**Testing:**
- [ ] Build succeeds
- [ ] No references to local_gradients remain
- [ ] Crystalline memory is created
- [ ] Gradients written to segments
- [ ] Gradients read from segments
- [ ] Accumulation works correctly
- [ ] No memory leaks

**Verification:**
```bash
# Run training and check memory usage
valgrind --leak-check=full ./app/hyper_prime_spiral
# Verify no leaks from local_gradients
```

---

### Task 3.2: Wire Kissing Boundaries for Gradient Sharing
**Estimated Time:** 4-5 hours  
**Files to Modify:**
- `src/ai/cllm_training_threaded.c`

**Implementation Steps:**

1. **Add boundaries array to `SphereTrainingContext`:**
```c
struct SphereTrainingContext {
    // ... existing fields ...
    
    // PHASE 3: Kissing boundaries
    KissingBoundary* boundaries[12];  // Boundaries with siblings
    int num_boundaries;
};
```

2. **Create boundaries in `sphere_spawn_children()`:**
```c
// After creating all children, establish kissing boundaries
printf("[Sphere %d] Establishing kissing boundaries between %d children\n",
       parent->sphere_id, num_children);

for (int i = 0; i < num_children; i++) {
    parent->children[i]->num_boundaries = 0;
    
    for (int j = i + 1; j < num_children; j++) {
        // Calculate boundary size (1/12 of gradient size)
        size_t boundary_size = (parent->gradient_size * sizeof(double)) / 12;
        
        // Create boundary between siblings
        KissingBoundary* boundary = kissing_boundary_create(
            parent->children[i]->crystalline_memory,
            parent->children[j]->crystalline_memory,
            i,  // segment_a
            j,  // segment_b
            boundary_size
        );
        
        if (boundary) {
            // Store boundary in both children
            parent->children[i]->boundaries[parent->children[i]->num_boundaries++] = boundary;
            parent->children[j]->boundaries[parent->children[j]->num_boundaries++] = boundary;
            
            printf("  Created boundary: Child %d <-> Child %d\n",
                   parent->children[i]->sphere_id,
                   parent->children[j]->sphere_id);
        }
    }
}
```

3. **Use boundaries for gradient sharing:**
```c
// In sphere_process_batch(), after computing gradients
if (ctx->num_boundaries > 0) {
    // Share gradients with siblings via kissing boundaries
    for (int b = 0; b < ctx->num_boundaries; b++) {
        KissingBoundary* boundary = ctx->boundaries[b];
        
        // Write our gradients to boundary
        double* our_gradients = /* get from crystalline memory */;
        kissing_boundary_write(
            boundary,
            ctx->symmetry_group,  // our segment
            our_gradients,
            boundary_size
        );
        
        // Read sibling's gradients from boundary
        double* sibling_gradients = (double*)malloc(boundary_size);
        kissing_boundary_read(
            boundary,
            (ctx->symmetry_group + 1) % 12,  // sibling segment
            sibling_gradients,
            boundary_size
        );
        
        // Average with sibling gradients (simple sharing)
        for (size_t i = 0; i < boundary_size / sizeof(double); i++) {
            our_gradients[i] = (our_gradients[i] + sibling_gradients[i]) / 2.0;
        }
        
        free(sibling_gradients);
    }
}
```

4. **Cleanup boundaries in `sphere_context_free()`:**
```c
// Free kissing boundaries
for (int i = 0; i < ctx->num_boundaries; i++) {
    if (ctx->boundaries[i]) {
        kissing_boundary_free(ctx->boundaries[i]);
    }
}
```

**Testing:**
- [ ] Build succeeds
- [ ] Boundaries created between siblings
- [ ] Gradients shared via boundaries
- [ ] Lock-free reads work
- [ ] Spinlock writes work
- [ ] No deadlocks
- [ ] No memory corruption

**Verification:**
```bash
# Run with multiple children and check boundary creation
./app/hyper_prime_spiral
# Look for: "Created boundary: Child X <-> Child Y"
```

---

### Task 3.3: Wire Lock-Free Memory for Gradient Accumulation
**Estimated Time:** 3-4 hours  
**Files to Modify:**
- `src/ai/cllm_training_threaded.c`

**Implementation Steps:**

1. **Replace mutex-based accumulation:**
```c
// In accumulate_gradients()

// BEFORE:
pthread_mutex_lock(&system->gradient_lock);
for (size_t i = 0; i < system->gradient_size; i++) {
    system->accumulated_gradients[i] += worker_gradients[i];
}
pthread_mutex_unlock(&system->gradient_lock);

// AFTER:
lockfree_gradient_accumulate(
    system->accumulated_gradients,
    worker_gradients,
    system->gradient_size
);
```

2. **Add lock-free weight updates:**
```c
// In optimizer application

// BEFORE:
pthread_mutex_lock(&system->model_lock);
for (size_t i = 0; i < model->num_params; i++) {
    model->params[i] -= learning_rate * gradients[i];
}
pthread_mutex_unlock(&system->model_lock);

// AFTER:
lockfree_weight_update(
    model->params,
    gradients,
    learning_rate,
    model->num_params
);
```

3. **Add verification:**
```c
// After lock-free operations, verify correctness
if (epoch % 10 == 0) {
    double gradient_sum = 0.0;
    for (size_t i = 0; i < system->gradient_size; i++) {
        gradient_sum += system->accumulated_gradients[i];
    }
    printf("[Epoch %d] Gradient sum: %.6f (lock-free accumulation)\n",
           epoch, gradient_sum);
}
```

**Testing:**
- [ ] Build succeeds
- [ ] Lock-free accumulation works
- [ ] No race conditions
- [ ] Results match mutex version
- [ ] Performance improved
- [ ] No memory corruption

**Verification:**
```bash
# Compare performance: mutex vs lock-free
# Run with mutex version, measure time
# Run with lock-free version, measure time
# Verify speedup
```

---

## PHASE 4: PLIMPTON WORK DISTRIBUTION (Priority: MEDIUM)

### Task 4.1: Wire Plimpton Ratios into Batch Splitting
**Estimated Time:** 3-4 hours  
**Files to Modify:**
- `src/ai/cllm_training_threaded.c`

**Implementation Steps:**

1. **Add Plimpton relationship to parent-child:**
```c
// In sphere_spawn_children()

// Determine Plimpton generators for this parent-child relationship
int p = parent->hierarchy_level + 2;  // Example: level 0 -> p=2
int q = 1;

// Verify coprime and constraints
if (!plimpton_verify_coprime(p, q)) {
    fprintf(stderr, "[WARNING] Generators (%d,%d) not coprime, using (2,1)\n", p, q);
    p = 2; q = 1;
}

// Calculate Plimpton ratios
PlimptonRatios ratios;
calculate_plimpton_ratios(&ratios, p, q);

printf("[Sphere %d] Plimpton ratios: b/d=%.4f, c/d=%.4f (generators %d,%d)\n",
       parent->sphere_id, ratios.b_over_d, ratios.c_over_d, p, q);
```

2. **Split work according to ratios:**
```c
// Determine work split
double parent_fraction = ratios.b_over_d;
double children_fraction = ratios.c_over_d;

// Assign work fractions
parent->work_fraction = parent_fraction;
for (int i = 0; i < num_children; i++) {
    parent->children[i]->work_fraction = children_fraction / num_children;
}

printf("[Sphere %d] Work split: parent=%.2f%%, children=%.2f%% each\n",
       parent->sphere_id, parent_fraction * 100, 
       (children_fraction / num_children) * 100);
```

3. **Use work fractions in batch distribution:**
```c
// In work queue or batch distribution
int total_batches = get_total_batches();

for (int i = 0; i < system->num_worker_spheres; i++) {
    SphereTrainingContext* ctx = system->sphere_contexts[i];
    
    // Calculate batches based on work fraction
    int batches_for_sphere = (int)(total_batches * ctx->work_fraction);
    ctx->assigned_batches = batches_for_sphere;
    
    printf("[Worker %d] Assigned %d batches (fraction=%.3f)\n",
           i, batches_for_sphere, ctx->work_fraction);
}
```

**Testing:**
- [ ] Build succeeds
- [ ] Plimpton ratios calculated
- [ ] Work split according to ratios
- [ ] Batches distributed correctly
- [ ] Total work sums to 100%

**Verification:**
```bash
# Check work distribution in logs
grep "Work split" training.log
# Verify ratios match Plimpton 322 values
```

---

## PHASE 5: CYMATIC TIMING INTEGRATION (Priority: MEDIUM)

### Task 5.1: Wire Cymatic Barriers into Training Loop
**Estimated Time:** 3-4 hours  
**Files to Modify:**
- `src/ai/cllm_training_threaded.c`

**Implementation Steps:**

1. **Add cymatic barrier to `ThreadedTrainingSystem`:**
```c
struct ThreadedTrainingSystem {
    // ... existing fields ...
    
    // PHASE 5: Cymatic timing
    CymaticBarrier* epoch_barrier;  // Schumann resonance sync
    CymaticBarrier* batch_barrier;  // 432 Hz sync
};
```

2. **Create barriers in `threaded_training_create()`:**
```c
// Create epoch barrier (Schumann resonance 7.83 Hz)
system->epoch_barrier = cymatic_barrier_create(
    system->num_worker_spheres + 1,  // +1 for control thread
    CYMATIC_SCHUMANN_7_83HZ
);

// Create batch barrier (432 Hz)
system->batch_barrier = cymatic_barrier_create(
    system->num_worker_spheres + 1,
    CYMATIC_432HZ
);

if (system->epoch_barrier && system->batch_barrier) {
    printf("  ✓ Cymatic barriers created (7.83 Hz epoch, 432 Hz batch)\n");
}
```

3. **Use in control thread:**
```c
// In control_thread_func()
while (system->control_running) {
    // Wait at epoch barrier
    cymatic_barrier_wait(system->epoch_barrier);
    
    printf("[Node Zero] Epoch barrier synchronized (7.83 Hz)\n");
    
    // Accumulate gradients
    accumulate_gradients(system);
    
    // Apply optimizer
    // ...
    
    // Wait at batch barrier
    cymatic_barrier_wait(system->batch_barrier);
}
```

4. **Use in worker threads:**
```c
// In sphere_worker_thread_dynamic()
while (atomic_load(&ctx->system->running)) {
    // Wait at batch barrier before processing
    cymatic_barrier_wait(ctx->system->batch_barrier);
    
    // Process batch
    sphere_process_batch(ctx, ctx->system->training);
    
    // Wait at epoch barrier after processing
    cymatic_barrier_wait(ctx->system->epoch_barrier);
}
```

5. **Cleanup:**
```c
// In threaded_training_free()
if (system->epoch_barrier) {
    cymatic_barrier_free(system->epoch_barrier);
}
if (system->batch_barrier) {
    cymatic_barrier_free(system->batch_barrier);
}
```

**Testing:**
- [ ] Build succeeds
- [ ] Barriers created
- [ ] Synchronization works
- [ ] Timing measurements logged
- [ ] No deadlocks

**Verification:**
```bash
# Check timing in logs
grep "barrier synchronized" training.log
# Verify timing matches frequencies
```

---

## PHASE 6: UI INTEGRATION (Priority: LOW)

### Task 6.1: Add Entropy Metrics to Training Tab
**Estimated Time:** 2-3 hours  
**Files to Modify:**
- `app/ui/tabs/tab_training.c`

**Implementation Steps:**

1. **Add entropy display section:**
```c
// In render_training_tab()
if (training_system && training_system->entropy_context) {
    ImGui::Separator();
    ImGui::Text("Entropy Metrics");
    
    const ModelEntropyStats* stats = get_model_entropy_stats(
        training_system->entropy_context
    );
    
    ImGui::Text("Model Entropy: %.4f", stats->mean_entropy);
    ImGui::Text("Variance: %.4f", stats->variance);
    ImGui::Text("Min: %.4f, Max: %.4f", stats->min_entropy, stats->max_entropy);
    
    // Per-dimension entropy
    if (ImGui::TreeNode("Dimension Entropy")) {
        for (int d = 0; d < 12; d++) {
            const DimensionEntropyStats* dim_stats = get_dimension_stats(
                training_system->entropy_context,
                d
            );
            
            ImGui::Text("Dim %2d: %.4f (σ²=%.4f)", 
                       d, dim_stats->mean_entropy, dim_stats->variance);
        }
        ImGui::TreePop();
    }
}
```

2. **Add entropy graph:**
```c
// Add entropy history tracking
static float entropy_history[100] = {0};
static int entropy_history_idx = 0;

// Update history
if (stats) {
    entropy_history[entropy_history_idx] = stats->mean_entropy;
    entropy_history_idx = (entropy_history_idx + 1) % 100;
}

// Plot
ImGui::PlotLines("Entropy", entropy_history, 100, 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));
```

**Testing:**
- [ ] Build succeeds
- [ ] Entropy metrics displayed
- [ ] Values update in real-time
- [ ] Graph shows history
- [ ] UI responsive

---

### Task 6.2: Add Adaptive Hierarchy Visualization
**Estimated Time:** 2-3 hours  
**Files to Modify:**
- `app/ui/tabs/tab_training.c`

**Implementation Steps:**

1. **Add hierarchy display:**
```c
if (training_system && training_system->adaptive_hierarchy) {
    ImGui::Separator();
    ImGui::Text("Adaptive Hierarchy");
    
    DepthAdjustmentStats stats;
    get_depth_adjustment_stats(training_system->adaptive_hierarchy, &stats);
    
    ImGui::Text("Current Depth: %d", stats.current_depth);
    ImGui::Text("Recommended Depth: %d", stats.recommended_depth);
    ImGui::Text("Spawn Count: %d", stats.spawn_count);
    ImGui::Text("Despawn Count: %d", stats.despawn_count);
    
    // Progress bar for depth
    float depth_progress = (float)stats.current_depth / 5.0f;
    ImGui::ProgressBar(depth_progress, ImVec2(0, 0), "Depth");
}
```

**Testing:**
- [ ] Build succeeds
- [ ] Hierarchy stats displayed
- [ ] Values update correctly
- [ ] UI clear and readable

---

## VERIFICATION CHECKLIST

After each task:
- [ ] Code compiles without errors
- [ ] Code compiles without warnings
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Manual testing confirms functionality
- [ ] Performance is acceptable
- [ ] Memory leaks checked with valgrind
- [ ] Logging shows expected behavior

---

## ESTIMATED TIMELINE

**Phase 1 (Critical):** 5-7 hours  
**Phase 2 (High):** 7-9 hours  
**Phase 3 (High):** 12-15 hours  
**Phase 4 (Medium):** 3-4 hours  
**Phase 5 (Medium):** 3-4 hours  
**Phase 6 (Low):** 4-6 hours  

**Total:** 34-45 hours of focused implementation work

---

## SUCCESS CRITERIA

- ✅ All critical wiring complete
- ✅ All high-priority wiring complete
- ✅ Build succeeds with zero errors
- ✅ Build succeeds with zero warnings
- ✅ All tests passing
- ✅ Training runs successfully
- ✅ Performance improvements verified
- ✅ UI shows new metrics
- ✅ Documentation updated

---

This execution plan provides step-by-step instructions for completing all wiring identified in the comprehensive architecture analysis.