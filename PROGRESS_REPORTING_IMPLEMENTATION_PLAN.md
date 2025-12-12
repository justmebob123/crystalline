# Progress Reporting Implementation Plan

## Problem Statement
Training output is flooded with debug messages, hiding useful progress information.
User cannot determine:
- Current epoch number
- Batch progress within epoch
- Loss values
- Training speed
- ETA

## Current State Analysis

### Debug Output Issues
1. 28 debug printf statements in cllm_training_threaded.c
2. Debug output on EVERY batch (work queue, gradient accumulation, layer processing)
3. No compile-time flag to disable debug output
4. Progress information buried in debug spam

### Existing Progress Reporting
- Epoch start: "Epoch X - Using Y worker threads"
- Epoch end: "Epoch complete: Total batches: X"
- Per-sphere statistics at epoch end
- NO per-batch progress during epoch

## Implementation Plan

### Phase 1: Wrap Debug Output (30 min)
1. Add CLLM_DEBUG compile flag to Makefile
2. Wrap all [DEBUG] printf statements in #ifdef CLLM_DEBUG
3. Test build with and without flag

### Phase 2: Add Progress Tracking (1 hour)
1. Add progress tracking to KissingSphereSystem:
   - current_batch
   - total_batches
   - batches_processed
   - epoch_start_time
   - last_progress_time

2. Add progress calculation functions:
   - calculate_speed (batches/sec)
   - calculate_eta (time remaining)
   - format_time (seconds to HH:MM:SS)

### Phase 3: Add Periodic Progress Updates (30 min)
1. Add progress update every N batches (configurable, default 10)
2. Format: "Epoch X/Y | Batch A/B (C%) | Loss: D | E batch/s | ETA: F"
3. Update at:
   - Epoch start
   - Every N batches
   - Epoch end

### Phase 4: Update CLI Tool (30 min)
1. Add --verbose flag to enable debug output
2. Default to clean progress output
3. Update help text

## Implementation Details

### Files to Modify
1. src/ai/cllm_training_threaded.c
   - Wrap debug output
   - Add progress tracking
   - Add periodic updates

2. include/cllm_training_threaded.h
   - Add progress fields to KissingSphereSystem

3. Makefile
   - Add CLLM_DEBUG flag option

4. tools/cllm_unified.c
   - Add --verbose flag

## Expected Output

### Before (Current):
```
[DEBUG] work_queue_pop: Worker got batch at index=167
[DEBUG] accumulate_gradients: ENTRY - system=0x...
[DEBUG] accumulate_gradients: Lock acquired
[DEBUG] Processing layer 0
[DEBUG] Processing layer 1
... (hundreds of lines)
```

### After (Clean):
```
Epoch 1/1000 | Batch 167/1000 (16.7%) | Loss: 2.345 | 12.5 batch/s | ETA: 22h 15m
Epoch 1/1000 | Batch 177/1000 (17.7%) | Loss: 2.312 | 12.8 batch/s | ETA: 21h 48m
```

### After (With --verbose):
```
Epoch 1/1000 | Batch 167/1000 (16.7%) | Loss: 2.345 | 12.5 batch/s | ETA: 22h 15m
[DEBUG] work_queue_pop: Worker got batch at index=167
[DEBUG] accumulate_gradients: ENTRY - system=0x...
... (all debug output)
```

## Testing Plan
1. Build with debug disabled (default)
2. Verify clean output
3. Build with CLLM_DEBUG=1
4. Verify debug output appears
5. Test progress updates every 10 batches
6. Verify ETA calculation accuracy

## Success Criteria
- Zero debug output by default
- Clear progress updates every 10 batches
- Accurate speed and ETA calculations
- --verbose flag enables debug output
- Build successful with 0 errors, 0 warnings