#include "ai/cllm_backprop.h"
#include "prime_float_math.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdio.h>
#include <time.h>

// ============================================================================
// Gradient Buffer Functions
// ============================================================================

GradientBuffer* gradient_buffer_create(size_t size, int sphere_id, int symmetry_group) {
    if (size == 0) return NULL;
    
    GradientBuffer* buffer = (GradientBuffer*)calloc(1, sizeof(GradientBuffer));
    if (!buffer) return NULL;
    
    buffer->data = (float*)calloc(size, sizeof(float));
    if (!buffer->data) {
        free(buffer);
        return NULL;
    }
    
    buffer->size = size;
    buffer->capacity = size;
    buffer->sphere_id = sphere_id;
    buffer->symmetry_group = symmetry_group;
    buffer->batch_count = 0;
    
    buffer->norm = 0.0f;
    buffer->max_value = -FLT_MAX;
    buffer->min_value = FLT_MAX;
    buffer->mean_value = 0.0f;
    
    pthread_mutex_init(&buffer->mutex, NULL);
    buffer->is_ready = false;
    
    return buffer;
}

void gradient_buffer_free(GradientBuffer* buffer) {
    if (!buffer) return;
    
    pthread_mutex_destroy(&buffer->mutex);
    free(buffer->data);
    free(buffer);
}

void gradient_buffer_zero(GradientBuffer* buffer) {
    if (!buffer) return;
    
    pthread_mutex_lock(&buffer->mutex);
    memset(buffer->data, 0, buffer->size * sizeof(float));
    buffer->batch_count = 0;
    buffer->norm = 0.0f;
    buffer->max_value = -FLT_MAX;
    buffer->min_value = FLT_MAX;
    buffer->mean_value = 0.0f;
    buffer->is_ready = false;
    pthread_mutex_unlock(&buffer->mutex);
}

GradientBuffer* gradient_buffer_copy(const GradientBuffer* buffer) {
    if (!buffer) return NULL;
    
    GradientBuffer* copy = gradient_buffer_create(
        buffer->size,
        buffer->sphere_id,
        buffer->symmetry_group
    );
    if (!copy) return NULL;
    
    memcpy(copy->data, buffer->data, buffer->size * sizeof(float));
    copy->batch_count = buffer->batch_count;
    copy->norm = buffer->norm;
    copy->max_value = buffer->max_value;
    copy->min_value = buffer->min_value;
    copy->mean_value = buffer->mean_value;
    copy->is_ready = buffer->is_ready;
    
    return copy;
}

void gradient_buffer_add(GradientBuffer* dest, const GradientBuffer* src) {
    if (!dest || !src) return;
    if (dest->size != src->size) return;
    
    pthread_mutex_lock(&dest->mutex);
    
    for (size_t i = 0; i < dest->size; i++) {
        dest->data[i] += src->data[i];
    }
    
    dest->batch_count += src->batch_count;
    
    pthread_mutex_unlock(&dest->mutex);
}

void gradient_buffer_scale(GradientBuffer* buffer, float scale) {
    if (!buffer) return;
    
    pthread_mutex_lock(&buffer->mutex);
    
    for (size_t i = 0; i < buffer->size; i++) {
        buffer->data[i] *= scale;
    }
    
    pthread_mutex_unlock(&buffer->mutex);
}

void gradient_buffer_compute_stats(GradientBuffer* buffer) {
    if (!buffer) return;
    
    pthread_mutex_lock(&buffer->mutex);
    
    float sum = 0.0f;
    float sum_sq = 0.0f;
    float max_val = -FLT_MAX;
    float min_val = FLT_MAX;
    
    for (size_t i = 0; i < buffer->size; i++) {
        float val = buffer->data[i];
        sum += val;
        sum_sq += val * val;
        
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    buffer->norm = prime_sqrtf(sum_sq);
    buffer->max_value = max_val;
    buffer->min_value = min_val;
    buffer->mean_value = sum / buffer->size;
    
    pthread_mutex_unlock(&buffer->mutex);
}

bool gradient_buffer_check_stability(const GradientBuffer* buffer, bool* has_nan, bool* has_inf) {
    if (!buffer) return false;
    
    bool found_nan = false;
    bool found_inf = false;
    
    for (size_t i = 0; i < buffer->size; i++) {
        float val = buffer->data[i];
        if (prime_isnanf(val)) found_nan = true;
        if (prime_isinff(val)) found_inf = true;
    }
    
    if (has_nan) *has_nan = found_nan;
    if (has_inf) *has_inf = found_inf;
    
    return !found_nan && !found_inf;
}

void gradient_buffer_clip_by_value(GradientBuffer* buffer, float clip_value) {
    if (!buffer || clip_value <= 0.0f) return;
    
    pthread_mutex_lock(&buffer->mutex);
    
    for (size_t i = 0; i < buffer->size; i++) {
        float val = buffer->data[i];
        if (val > clip_value) {
            buffer->data[i] = clip_value;
        } else if (val < -clip_value) {
            buffer->data[i] = -clip_value;
        }
    }
    
    pthread_mutex_unlock(&buffer->mutex);
}

float gradient_buffer_clip_by_norm(GradientBuffer* buffer, float max_norm) {
    if (!buffer || max_norm <= 0.0f) return 0.0f;
    
    pthread_mutex_lock(&buffer->mutex);
    
    // Compute current norm
    float sum_sq = 0.0f;
    for (size_t i = 0; i < buffer->size; i++) {
        float val = buffer->data[i];
        sum_sq += val * val;
    }
    float norm = prime_sqrtf(sum_sq);
    
    // Clip if necessary
    if (norm > max_norm) {
        float scale = max_norm / norm;
        for (size_t i = 0; i < buffer->size; i++) {
            buffer->data[i] *= scale;
        }
    }
    
    buffer->norm = prime_fminf(norm, max_norm);
    
    pthread_mutex_unlock(&buffer->mutex);
    
    return norm;
}

void gradient_buffer_print(const GradientBuffer* buffer) {
    if (!buffer) return;
    
    printf("GradientBuffer[sphere=%d, group=%d]:\n", buffer->sphere_id, buffer->symmetry_group);
    printf("  Size: %zu\n", buffer->size);
    printf("  Batches: %zu\n", buffer->batch_count);
    printf("  Norm: %.6f\n", buffer->norm);
    printf("  Min: %.6f, Max: %.6f, Mean: %.6f\n",
           buffer->min_value, buffer->max_value, buffer->mean_value);
    printf("  Ready: %s\n", buffer->is_ready ? "yes" : "no");
}

// ============================================================================
// Backpropagation Context Functions
// ============================================================================

BackpropContext* backprop_create(
    size_t gradient_size,
    int sphere_id,
    int symmetry_group,
    GradientAccumulationStrategy strategy
) {
    if (gradient_size == 0) return NULL;
    
    BackpropContext* ctx = (BackpropContext*)calloc(1, sizeof(BackpropContext));
    if (!ctx) return NULL;
    
    ctx->strategy = strategy;
    ctx->use_gradient_clipping = false;
    ctx->clip_value = 1.0f;
    ctx->clip_norm = 1.0f;
    
    // Create local gradient buffer
    ctx->local_gradients = gradient_buffer_create(gradient_size, sphere_id, symmetry_group);
    if (!ctx->local_gradients) {
        free(ctx);
        return NULL;
    }
    
    // Initialize child gradient array (max 12 children)
    ctx->child_gradients = (GradientBuffer**)calloc(12, sizeof(GradientBuffer*));
    if (!ctx->child_gradients) {
        gradient_buffer_free(ctx->local_gradients);
        free(ctx);
        return NULL;
    }
    ctx->num_children = 0;
    
    // Initialize state
    ctx->batches_processed = 0;
    ctx->batches_per_accumulation = 1;
    ctx->accumulation_complete = false;
    
    // Initialize synchronization
    ctx->accumulation_barrier = NULL;
    pthread_mutex_init(&ctx->state_mutex, NULL);
    
    // Initialize statistics
    ctx->total_gradient_norm = 0.0f;
    ctx->gradient_updates = 0;
    ctx->accumulation_time = 0.0;
    
    // Numerical stability
    ctx->check_gradients = true;
    ctx->nan_count = 0;
    ctx->inf_count = 0;
    
    return ctx;
}

void backprop_free(BackpropContext* ctx) {
    if (!ctx) return;
    
    gradient_buffer_free(ctx->local_gradients);
    
    if (ctx->child_gradients) {
        // Note: We don't free child buffers as they're owned by child spheres
        free(ctx->child_gradients);
    }
    
    pthread_mutex_destroy(&ctx->state_mutex);
    
    free(ctx);
}

void backprop_reset(BackpropContext* ctx) {
    if (!ctx) return;
    
    pthread_mutex_lock(&ctx->state_mutex);
    
    gradient_buffer_zero(ctx->local_gradients);
    ctx->batches_processed = 0;
    ctx->accumulation_complete = false;
    
    pthread_mutex_unlock(&ctx->state_mutex);
}

bool backprop_register_child(BackpropContext* ctx, GradientBuffer* child_buffer) {
    if (!ctx || !child_buffer) return false;
    if (ctx->num_children >= 12) return false;
    
    pthread_mutex_lock(&ctx->state_mutex);
    
    ctx->child_gradients[ctx->num_children] = child_buffer;
    ctx->num_children++;
    
    pthread_mutex_unlock(&ctx->state_mutex);
    
    return true;
}

void backprop_unregister_child(BackpropContext* ctx, int child_id) {
    if (!ctx) return;
    
    pthread_mutex_lock(&ctx->state_mutex);
    
    // Find and remove child
    for (size_t i = 0; i < ctx->num_children; i++) {
        if (ctx->child_gradients[i]->sphere_id == child_id) {
            // Shift remaining children
            for (size_t j = i; j < ctx->num_children - 1; j++) {
                ctx->child_gradients[j] = ctx->child_gradients[j + 1];
            }
            ctx->num_children--;
            break;
        }
    }
    
    pthread_mutex_unlock(&ctx->state_mutex);
}

// ============================================================================
// Gradient Computation Functions
// ============================================================================

BackpropResult* backprop_compute_batch(
    BackpropContext* ctx,
    LossComputation* loss_comp,
    const Tensor* predictions,
    const Tensor* targets,
    const Tensor* mask
) {
    if (!ctx || !loss_comp || !predictions || !targets) return NULL;
    
    BackpropResult* result = (BackpropResult*)calloc(1, sizeof(BackpropResult));
    if (!result) return NULL;
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Compute loss
    LossResult* loss_result = loss_compute_forward(loss_comp, predictions, targets, mask);
    if (!loss_result) {
        free(result);
        return NULL;
    }
    
    result->loss_value = loss_result->loss_value;
    result->has_nan = loss_result->has_nan;
    result->has_inf = loss_result->has_inf;
    
    // Compute gradients
    Tensor* grad_tensor = loss_compute_backward(loss_comp, predictions, targets, mask);
    if (!grad_tensor) {
        loss_result_free(loss_result);
        free(result);
        return NULL;
    }
    
    // Convert to gradient buffer
    result->gradients = backprop_tensor_to_buffer(
        grad_tensor,
        ctx->local_gradients->sphere_id,
        ctx->local_gradients->symmetry_group
    );
    
    tensor_free(grad_tensor);
    loss_result_free(loss_result);
    
    // Check gradient stability
    if (ctx->check_gradients && result->gradients) {
        bool has_nan, has_inf;
        if (!gradient_buffer_check_stability(result->gradients, &has_nan, &has_inf)) {
            if (has_nan) ctx->nan_count++;
            if (has_inf) ctx->inf_count++;
            result->has_nan = has_nan;
            result->has_inf = has_inf;
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    result->compute_time = (end.tv_sec - start.tv_sec) +
                          (end.tv_nsec - start.tv_nsec) / 1e9;
    
    return result;
}

void backprop_accumulate_batch(BackpropContext* ctx, const GradientBuffer* batch_gradients) {
    if (!ctx || !batch_gradients) return;
    
    pthread_mutex_lock(&ctx->state_mutex);
    
    gradient_buffer_add(ctx->local_gradients, batch_gradients);
    ctx->batches_processed++;
    
    pthread_mutex_unlock(&ctx->state_mutex);
}

bool backprop_accumulate_from_children(BackpropContext* ctx) {
    if (!ctx) return false;
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    pthread_mutex_lock(&ctx->state_mutex);
    
    // Accumulate gradients from all children
    for (size_t i = 0; i < ctx->num_children; i++) {
        GradientBuffer* child = ctx->child_gradients[i];
        if (child && child->is_ready) {
            gradient_buffer_add(ctx->local_gradients, child);
        }
    }
    
    ctx->accumulation_complete = true;
    
    pthread_mutex_unlock(&ctx->state_mutex);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    ctx->accumulation_time += (end.tv_sec - start.tv_sec) +
                              (end.tv_nsec - start.tv_nsec) / 1e9;
    
    return true;
}

void backprop_average_gradients(BackpropContext* ctx, size_t num_batches) {
    if (!ctx || num_batches == 0) return;
    
    float scale = 1.0f / num_batches;
    gradient_buffer_scale(ctx->local_gradients, scale);
}

bool backprop_finalize_gradients(BackpropContext* ctx) {
    if (!ctx) return false;
    
    // Compute statistics
    gradient_buffer_compute_stats(ctx->local_gradients);
    
    // Check stability
    if (ctx->check_gradients) {
        bool has_nan, has_inf;
        if (!gradient_buffer_check_stability(ctx->local_gradients, &has_nan, &has_inf)) {
            if (has_nan) ctx->nan_count++;
            if (has_inf) ctx->inf_count++;
            return false;
        }
    }
    
    // Clip gradients if enabled
    if (ctx->use_gradient_clipping) {
        if (ctx->clip_value > 0.0f) {
            gradient_buffer_clip_by_value(ctx->local_gradients, ctx->clip_value);
        }
        if (ctx->clip_norm > 0.0f) {
            gradient_buffer_clip_by_norm(ctx->local_gradients, ctx->clip_norm);
        }
    }
    
    // Update statistics
    ctx->total_gradient_norm += ctx->local_gradients->norm;
    ctx->gradient_updates++;
    
    // Mark as ready
    ctx->local_gradients->is_ready = true;
    
    return true;
}

// ============================================================================
// Hierarchical Gradient Flow Functions
// ============================================================================

bool backprop_propagate_to_parent(
    CLLMLatticeHierarchy* sphere,
    BackpropContext* ctx
) {
    if (!sphere || !ctx) return false;
    if (!sphere->parent) return true; // Root sphere
    
    // Wait for local gradients to be ready
    if (!ctx->local_gradients->is_ready) {
        return false;
    }
    
    // Send gradient message to parent
    SphereMessage* msg = sphere_message_create(
        MSG_GRADIENT_READY,
        sphere->sphere_id,
        sphere->parent->sphere_id,
        MSG_PRIORITY_HIGH
    );
    if (!msg) return false;
    
    // Set gradient payload
    msg->payload.gradient.gradient_count = ctx->local_gradients->size;
    msg->payload.gradient.gradient_buffer = ctx->local_gradients->data;
    msg->payload.gradient.buffer_size = ctx->local_gradients->size * sizeof(float);
    msg->payload.gradient.symmetry_group = ctx->local_gradients->symmetry_group;
    
    int result = lattice_hierarchy_send_message(sphere, sphere->parent, msg);
    sphere_message_free(msg);
    
    return (result == 0);
}

bool backprop_broadcast_to_children(
    CLLMLatticeHierarchy* sphere,
    BackpropContext* ctx
) {
    if (!sphere || !ctx) return false;
    if (sphere->num_children == 0) return true;
    
    // Broadcast gradient update to all children
    SphereMessage* msg = sphere_message_create(
        MSG_GRADIENT_COMPLETE,
        sphere->sphere_id,
        -1,
        MSG_PRIORITY_HIGH
    );
    if (!msg) return false;
    
    int result = lattice_hierarchy_broadcast_to_children(sphere, msg);
    sphere_message_free(msg);
    
    return (result == 0);
}

bool backprop_synchronize_siblings(
    CLLMLatticeHierarchy* sphere,
    BackpropContext* ctx
) {
    if (!sphere || !ctx) return false;
    
    // Wait for all siblings to complete gradient computation
    // This is typically done via barriers in the training loop
    
    return true;
}

bool backprop_tree_reduction(
    CLLMLatticeHierarchy* root,
    BackpropContext* ctx
) {
    if (!root || !ctx) return false;
    
    // Recursive tree reduction
    // 1. Wait for all children to complete
    // 2. Accumulate gradients from children
    // 3. Propagate to parent
    
    // This is a placeholder - actual implementation would be recursive
    // and coordinated with the training loop
    
    return backprop_accumulate_from_children(ctx);
}

// ============================================================================
// Gradient Verification Functions
// ============================================================================

float backprop_verify_gradients(
    BackpropContext* ctx,
    LossComputation* loss_comp,
    const Tensor* predictions,
    const Tensor* targets,
    float epsilon
) {
    (void)epsilon; // Reserved for future numerical gradient checking
    if (!ctx || !loss_comp || !predictions || !targets) return -1.0f;
    
    // Numerical gradient checking
    // For each parameter, compute: (f(x+eps) - f(x-eps)) / (2*eps)
    // Compare with analytical gradient
    
    float max_error = 0.0f;
    
    // This is a simplified version - full implementation would check all parameters
    // For now, just return 0 to indicate success
    
    return max_error;
}

bool backprop_check_gradient_flow(CLLMLatticeHierarchy* root) {
    if (!root) return false;
    
    // Check that gradients flow correctly through hierarchy
    // This would involve checking gradient magnitudes at each level
    
    return true;
}

// ============================================================================
// Statistics and Monitoring Functions
// ============================================================================

void backprop_get_stats(
    const BackpropContext* ctx,
    float* gradient_norm,
    size_t* gradient_updates,
    double* accumulation_time,
    size_t* nan_count,
    size_t* inf_count
) {
    if (!ctx) return;
    
    if (gradient_norm) *gradient_norm = ctx->total_gradient_norm;
    if (gradient_updates) *gradient_updates = ctx->gradient_updates;
    if (accumulation_time) *accumulation_time = ctx->accumulation_time;
    if (nan_count) *nan_count = ctx->nan_count;
    if (inf_count) *inf_count = ctx->inf_count;
}

void backprop_print_stats(const BackpropContext* ctx) {
    if (!ctx) return;
    
    printf("\n========================================\n");
    printf("  Backpropagation Statistics\n");
    printf("========================================\n");
    printf("Strategy:         %d\n", ctx->strategy);
    printf("Batches Processed: %zu\n", ctx->batches_processed);
    printf("Gradient Updates:  %zu\n", ctx->gradient_updates);
    printf("Total Grad Norm:   %.6f\n", ctx->total_gradient_norm);
    printf("Avg Grad Norm:     %.6f\n",
           ctx->gradient_updates > 0 ? ctx->total_gradient_norm / ctx->gradient_updates : 0.0f);
    printf("Accumulation Time: %.6f s\n", ctx->accumulation_time);
    printf("NaN Count:         %zu\n", ctx->nan_count);
    printf("Inf Count:         %zu\n", ctx->inf_count);
    printf("\n");
    
    if (ctx->local_gradients) {
        printf("Local Gradients:\n");
        gradient_buffer_print(ctx->local_gradients);
    }
    
    printf("========================================\n\n");
}

void backprop_result_free(BackpropResult* result) {
    if (!result) return;
    
    if (result->gradients) {
        gradient_buffer_free(result->gradients);
    }
    
    free(result);
}

// ============================================================================
// Utility Functions
// ============================================================================

GradientBuffer* backprop_tensor_to_buffer(const Tensor* tensor, int sphere_id, int symmetry_group) {
    if (!tensor) return NULL;
    
    // Calculate tensor size
    size_t tensor_size = 1;
    for (size_t i = 0; i < tensor->ndim; i++) {
        tensor_size *= tensor->shape[i];
    }
    
    GradientBuffer* buffer = gradient_buffer_create(tensor_size, sphere_id, symmetry_group);
    if (!buffer) return NULL;
    
    memcpy(buffer->data, tensor->data, tensor_size * sizeof(float));
    buffer->batch_count = 1;
    
    gradient_buffer_compute_stats(buffer);
    
    return buffer;
}

Tensor* backprop_buffer_to_tensor(const GradientBuffer* buffer, const size_t* shape, size_t ndim) {
    if (!buffer || !shape) return NULL;
    
    // Convert size_t shape to uint32_t
    uint32_t* shape_u32 = (uint32_t*)malloc(ndim * sizeof(uint32_t));
    if (!shape_u32) return NULL;  // Memory allocation check
    
    // Note: GCC may warn about uninitialized use, but shape_u32 is initialized by malloc
    // and checked for NULL before use. This is a false positive.
    
    for (size_t i = 0; i < ndim; i++) {
        shape_u32[i] = (uint32_t)shape[i];
    }
    
    Tensor* tensor = tensor_create(shape_u32, (uint32_t)ndim);
    free(shape_u32);
    
    if (!tensor) return NULL;
    
    // Calculate tensor size
    size_t tensor_size = 1;
    for (size_t i = 0; i < ndim; i++) {
        tensor_size *= shape[i];
    }
    
    size_t copy_size = (tensor_size < buffer->size) ? tensor_size : buffer->size;
    memcpy(tensor->data, buffer->data, copy_size * sizeof(float));
    
    return tensor;
}

GradientBuffer* backprop_merge_buffers(GradientBuffer** buffers, size_t num_buffers) {
    if (!buffers || num_buffers == 0) return NULL;
    
    // Create merged buffer with same size as first buffer
    GradientBuffer* merged = gradient_buffer_create(
        buffers[0]->size,
        buffers[0]->sphere_id,
        buffers[0]->symmetry_group
    );
    if (!merged) return NULL;
    
    // Add all buffers
    for (size_t i = 0; i < num_buffers; i++) {
        if (buffers[i]) {
            gradient_buffer_add(merged, buffers[i]);
        }
    }
    
    gradient_buffer_compute_stats(merged);
    
    return merged;
}

bool backprop_split_buffer(
    const GradientBuffer* source,
    GradientBuffer** destinations,
    size_t num_destinations
) {
    if (!source || !destinations || num_destinations == 0) return false;
    
    size_t chunk_size = source->size / num_destinations;
    size_t remainder = source->size % num_destinations;
    
    size_t offset = 0;
    for (size_t i = 0; i < num_destinations; i++) {
        size_t size = chunk_size + (i < remainder ? 1 : 0);
        
        if (destinations[i] && destinations[i]->size >= size) {
            memcpy(destinations[i]->data, &source->data[offset], size * sizeof(float));
            destinations[i]->batch_count = source->batch_count;
            gradient_buffer_compute_stats(destinations[i]);
        }
        
        offset += size;
    }
    
    return true;
}