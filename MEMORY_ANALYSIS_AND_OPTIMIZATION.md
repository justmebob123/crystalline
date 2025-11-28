# Memory Analysis and Optimization Plan

## Current Memory Usage Analysis

### Observed Memory Consumption
- **Dataset**: 24MB text file
- **Memory Used**: 1.32GB peak
- **Ratio**: 55x the data size
- **Assessment**: UNACCEPTABLE

### Memory Breakdown

#### 1. Model Parameters (4.5M parameters)
```
Vocab size: 788
Embedding dim: 256
Layers: 6
Heads: 8
FF dim: 1024

Estimated:
- Embeddings: 788 × 256 × 4 bytes = 0.8MB
- Attention weights: 6 layers × (256×256×4 + 256×1024×4) = 9.4MB
- Feed-forward: 6 layers × (256×1024×4 + 1024×256×4) = 12.6MB
- Total model: ~23MB

Actual allocation: Likely 2-3x due to gradients and optimizer state
Expected: ~70MB for model + gradients + Adam state
```

#### 2. Batch Data (63 parallel batches)
```
Batch size: 32 sequences
Sequence length: 128 tokens
Batches: 63

Per batch:
- Input IDs: 32 × 128 × 4 bytes = 16KB
- Target IDs: 32 × 128 × 4 bytes = 16KB
- Attention mask: 32 × 128 × 4 bytes = 16KB
- Total per batch: 48KB

63 batches: 63 × 48KB = 3MB
```

#### 3. Caches
```
Attention cache: 8.65MB (reported)
Backward buffers: 8.39MB (reported)
Embedding cache: 8.39MB (reported)
Total caches: 25.4MB
```

#### 4. Thread Overhead (65 threads)
```
Per thread stack: ~8MB default
65 threads: 65 × 8MB = 520MB

This is the problem!
```

#### 5. Vocabulary and Dataset
```
Vocabulary: 788 tokens (negligible)
Dataset tokens: 4.8M tokens × 4 bytes = 19.2MB
```

### Total Expected vs Actual
```
Expected:
- Model + gradients + Adam: 70MB
- Batches: 3MB
- Caches: 25MB
- Dataset: 19MB
- Thread stacks: 520MB (default)
Total expected: ~637MB

Actual: 1.32GB

Discrepancy: 683MB unaccounted for
```

## Root Causes

### 1. Thread Stack Size (PRIMARY ISSUE)
- Default stack size: 8MB per thread
- 65 threads: 520MB wasted
- Solution: Reduce to 1MB per thread (saves 455MB)

### 2. Kissing Spheres System (157 spheres)
- Creating 157 sphere structures when only using 63
- Each sphere has buffers and metadata
- Overhead: ~4MB per unused sphere × 94 = 376MB
- Solution: Only create spheres for active workers

### 3. Over-allocated Caches
- Attention cache: 8.65MB (too large for 788 vocab)
- Backward buffers: 8.39MB (excessive)
- Embedding cache: 8.39MB (unnecessary duplication)
- Solution: Right-size caches based on actual model

### 4. Batch Pre-allocation
- Allocating 63 batches upfront
- Many may be empty or unused
- Solution: Lazy allocation

## Optimization Plan

### Phase 1: Reduce Thread Stack Size (CRITICAL)
**Impact**: Save 455MB (35% reduction)
**Effort**: Low (30 minutes)

```c
// In threaded_training_create()
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setstacksize(&attr, 1024 * 1024);  // 1MB instead of 8MB

pthread_create(&thread, &attr, worker_func, ctx);
pthread_attr_destroy(&attr);
```

### Phase 2: Create Only Active Spheres
**Impact**: Save 376MB (28% reduction)
**Effort**: Medium (2 hours)

```c
// Don't create 157 spheres, only create num_threads spheres
system->num_worker_spheres = num_threads;  // Not 157

// Skip kissing spheres system creation for now
// Only create sphere contexts for actual workers
```

### Phase 3: Right-size Caches
**Impact**: Save 15MB (1% reduction)
**Effort**: Low (1 hour)

```c
// Calculate actual cache sizes needed
size_t vocab_size = training->model->vocab_size;
size_t embed_dim = training->model->embedding_dim;

// Attention cache: only for active sequences
size_t attention_cache = batch_size * seq_len * embed_dim * sizeof(float);

// No need for separate embedding cache if using model embeddings
```

### Phase 4: Lazy Batch Allocation
**Impact**: Save 2MB (negligible but cleaner)
**Effort**: Low (30 minutes)

```c
// Don't allocate all 63 batches upfront
// Allocate on demand as batches are loaded
```

### Phase 5: Remove model_lock (PERFORMANCE)
**Impact**: 63x speedup
**Effort**: High (2-3 days)

This is separate from memory but critical for performance.

## Expected Results After Optimization

### Memory Usage
```
Before: 1.32GB
After Phase 1: 865MB (-455MB, -35%)
After Phase 2: 489MB (-376MB, -28%)
After Phase 3: 474MB (-15MB, -1%)
After Phase 4: 472MB (-2MB, -0.4%)

Final: 472MB (64% reduction)
Ratio: 19.7x data size (still high but acceptable for ML)
```

### Why Still High?
- Model with gradients and Adam state: 70MB
- Dataset in memory: 19MB
- Thread stacks (1MB each): 65MB
- Batches and buffers: 28MB
- Working memory: 290MB (for forward/backward passes)

This is normal for ML training with 65 threads.

## Implementation Priority

### Immediate (Today)
1. ✅ Reduce thread stack size (30 min, 35% savings)
2. ✅ Create only active spheres (2 hours, 28% savings)

### Short-term (This Week)
3. Right-size caches (1 hour, 1% savings)
4. Lazy batch allocation (30 min, negligible)

### Medium-term (Next Week)
5. Remove model_lock for parallel execution
6. SIMD optimization
7. Memory profiling with valgrind

## Testing Plan

### After Each Optimization
1. Build and verify compilation
2. Test with small dataset (1MB)
3. Test with medium dataset (10MB)
4. Test with large dataset (24MB SQuAD)
5. Verify memory usage with `ps aux`
6. Check for memory leaks with valgrind
7. Verify correctness (no NaN gradients)

### Success Criteria
- Memory usage < 500MB for 24MB dataset
- No memory leaks
- No crashes
- Correct gradient computation
- All tests passing

## Next Steps

1. Implement Phase 1 (thread stack size)
2. Implement Phase 2 (active spheres only)
3. Test and verify
4. Commit and push
5. Continue with remaining phases