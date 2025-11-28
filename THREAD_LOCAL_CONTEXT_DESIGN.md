# Thread-Local Training Context Design

## Problem Statement
Currently, all 63 worker threads are serialized by `model_lock` because they share a single `CLLMTraining` structure. This causes 63x slowdown instead of 63x speedup.

## Root Cause Analysis
- `cllm_forward_training()` writes to `training->input_embeddings`, `training->logits`, etc.
- `cllm_backward_training()` writes to `training->gradients`, `training->attention_grads`, etc.
- Multiple threads writing to the same buffers = race conditions
- `model_lock` prevents race conditions but serializes execution

## Solution: Thread-Local Activation Buffers

### Key Insight
- **Model weights are READ-ONLY** during forward/backward passes
- Only **activation buffers** need to be thread-local
- Gradient accumulation is already lock-free (Phase 4)

### Design: ThreadLocalTrainingContext

```c
typedef struct {
    // Forward pass activation storage (thread-local)
    float* input_embeddings;         // [batch * seq * embed]
    float** layer_inputs;            // [num_layers][batch * seq * embed]
    float** attention_outputs;       // [num_layers][batch * seq * embed]
    float** ff_outputs;              // [num_layers][batch * seq * embed]
    float** layer_outputs;           // [num_layers][batch * seq * embed]
    float** ff_hidden;               // [num_layers][batch * seq * ff_hidden]
    float* final_hidden;             // [batch * seq * embed]
    float* logits;                   // [batch * seq * vocab]
    
    // Attention cache (thread-local)
    struct {
        float* attention_weights;    // [num_heads * seq * seq]
        float* queries;              // [seq * embed]
        float* keys;                 // [seq * embed]
        float* values;               // [seq * embed]
        float* scores;               // [num_heads * seq * seq]
    }* attention_cache;              // [num_layers]
    
    // Backward pass temporary buffers (thread-local)
    float* grad_logits;              // [batch * seq * vocab]
    float* grad_hidden;              // [batch * seq * embed]
    float* grad_layer;               // [batch * seq * embed]
    
    // Configuration (copied from main training)
    int batch_size;
    int seq_len;
    int num_layers;
    int embed_dim;
    int vocab_size;
    int ff_hidden_dim;
    int num_heads;
} ThreadLocalTrainingContext;
```

### Memory Requirements
- Per thread: ~6.1 MB
- 63 threads: 386 MB (acceptable)
- 8 threads: 49 MB (very reasonable)

### Integration Points

#### 1. Initialization (in `threaded_training_create()`)
```c
// For each worker thread
ctx->thread_local_training = thread_local_training_create(
    training->config.batch_size,
    training->config.sequence_length,
    model->num_layers,
    model->embedding_dim,
    model->vocab_size,
    ff_hidden_dim,
    num_heads
);
```

#### 2. Forward Pass (in `sphere_process_batch()`)
```c
// BEFORE (with lock):
pthread_mutex_lock(&ctx->system->model_lock);
float seq_loss = cllm_forward_training(training, &batch->input_ids[offset]);
pthread_mutex_unlock(&ctx->system->model_lock);

// AFTER (no lock):
float seq_loss = cllm_forward_training_threaded(
    training,
    ctx->thread_local_training,
    &batch->input_ids[offset]
);
```

#### 3. Backward Pass (in `sphere_process_batch()`)
```c
// BEFORE (with lock):
pthread_mutex_lock(&ctx->system->model_lock);
cllm_backward_training(training, &batch->target_ids[offset], ctx->local_gradients);
pthread_mutex_unlock(&ctx->system->model_lock);

// AFTER (no lock):
cllm_backward_training_threaded(
    training,
    ctx->thread_local_training,
    &batch->target_ids[offset],
    ctx->local_gradients
);
```

#### 4. Cleanup (in `threaded_training_destroy()`)
```c
// For each worker thread
thread_local_training_free(ctx->thread_local_training);
```

### Implementation Strategy

#### Phase 1: Create Thread-Local Context Structure
1. Add `ThreadLocalTrainingContext` to `cllm_training_threaded.h`
2. Implement `thread_local_training_create()`
3. Implement `thread_local_training_free()`
4. Add to `SphereThreadContext`

#### Phase 2: Create Threaded Forward/Backward Functions
1. Create `cllm_forward_training_threaded()` - takes thread-local context
2. Create `cllm_backward_training_threaded()` - takes thread-local context
3. These functions are identical to originals but use thread-local buffers

#### Phase 3: Update sphere_process_batch()
1. Replace `cllm_forward_training()` with `cllm_forward_training_threaded()`
2. Replace `cllm_backward_training()` with `cllm_backward_training_threaded()`
3. Remove `pthread_mutex_lock(&ctx->system->model_lock)`
4. Remove `pthread_mutex_unlock(&ctx->system->model_lock)`

#### Phase 4: Remove model_lock
1. Remove `pthread_mutex_t model_lock` from `ThreadedTrainingSystem`
2. Remove `pthread_mutex_init(&system->model_lock, NULL)`
3. Remove `pthread_mutex_destroy(&system->model_lock)`

#### Phase 5: Test Parallel Execution
1. Test with 1 thread (baseline)
2. Test with 2 threads (verify parallelism)
3. Test with 4, 8, 16, 32, 63 threads
4. Verify correctness (no NaN, proper convergence)
5. Measure actual speedup

### Expected Results
- **Before**: 63 threads = 63x slower (serialized)
- **After**: 63 threads = 40-50x faster (true parallelism)
- **Memory overhead**: 386 MB for 63 threads (acceptable)

### Risk Mitigation
1. Keep original functions intact (backward compatibility)
2. Test incrementally (1, 2, 4, 8 threads)
3. Verify correctness at each step
4. Monitor memory usage
5. Can roll back if issues arise

## Next Steps
1. Implement `ThreadLocalTrainingContext` structure
2. Implement allocation/deallocation functions
3. Create threaded versions of forward/backward
4. Update `sphere_process_batch()`
5. Remove `model_lock`
6. Test thoroughly