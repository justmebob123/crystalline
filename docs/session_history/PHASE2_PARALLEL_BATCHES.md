# Phase 2: Parallel Batch Processing Implementation

## Goal
Enable true multi-threaded training by processing multiple batches in parallel with proper gradient accumulation.

## Current State
- Thread pool exists but not used
- Training is single-threaded
- Crystalline optimizations exist but disabled

## Implementation Plan

### 1. Gradient Accumulation System
- Accumulate gradients from multiple batches
- Average gradients before optimizer step
- Thread-safe gradient buffers

### 2. Parallel Batch Processing
- Process N batches in parallel (N = thread count)
- Each thread processes one batch
- Accumulate gradients from all threads
- Single optimizer step after all batches complete

### 3. Expected Speedup
- 4 threads: ~3.5x speedup (accounting for overhead)
- 8 threads: ~6-7x speedup
- 16 threads: ~10-12x speedup

## Implementation

### Files to Modify
1. `src/ai/cllm_training.c` - Add gradient accumulation
2. `src/ai/cllm_training_parallel.c` - Implement parallel batch processing
3. `include/cllm_training.h` - Add parallel training structures

### Key Functions
1. `cllm_accumulate_gradients()` - Thread-safe gradient accumulation
2. `cllm_process_batch_parallel()` - Parallel batch processing
3. `cllm_train_epoch_parallel()` - Parallel epoch training

## Testing
- Verify gradients match single-threaded version
- Measure actual speedup
- Check for race conditions
</file_path>