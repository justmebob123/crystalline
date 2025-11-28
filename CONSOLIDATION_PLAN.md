# Training Code Consolidation Plan

## Problem Statement

We have redundant, poorly named files that create confusion:
1. `cllm_training.c` - "standard" single-threaded training
2. `cllm_crystalline_training.c` - crystalline loss functions (should be merged)
3. `cllm_training_threaded.c` - parallel training (should be THE default)

**The Reality**: 
- Crystalline IS the only implementation (not optional)
- Threaded/parallel IS the only implementation (not optional)
- We don't need multiple training systems

## Current Usage

### tools/train_model.c
- Uses `ThreadedTrainingSystem` 
- Calls `threaded_train_epoch()`
- This is correct - it's using the parallel system

### app/training_thread.c
- Uses `ThreadedTrainingSystem`
- Calls threaded functions
- This is correct

### src/crawler/continuous_training.c
- Uses `CLLMTraining`
- Calls `cllm_train_epoch()` (single-threaded)
- **This is WRONG** - should use parallel system

## Consolidation Strategy

### Phase 1: Merge Crystalline Loss into Main Training
**Goal**: Remove `cllm_crystalline_training.c` redundancy

1. Move `cllm_compute_loss()` from `cllm_crystalline_training.c` to `cllm_training.c`
2. Move helper functions (`crystalline_gcd_similarity`, `crystalline_sort_by_locality`)
3. Delete `cllm_crystalline_training.c`
4. Delete `include/cllm_crystalline_training.h`
5. Update includes

**Result**: One training file with crystalline loss built-in

### Phase 2: Make Threaded Training THE Default
**Goal**: Remove "threaded" naming - it's just "training"

**Rename files**:
- `cllm_training_threaded.c` → Keep as implementation
- `cllm_training.c` → Becomes helper functions only

**Rename functions** (remove "threaded" prefix):
- `threaded_train_epoch()` → `cllm_train_epoch()` (overwrite old one)
- `threaded_training_create()` → `cllm_training_create_parallel()`
- `threaded_training_free()` → `cllm_training_free_parallel()`

**Or better**: Just make the threaded system the ONLY system:
- Keep `ThreadedTrainingSystem` as the main training system
- Remove old `CLLMTraining` single-threaded code
- Update all callers

### Phase 3: Simplify API
**Goal**: One clear training API

**Single entry point**:
```c
// Create training system (always parallel)
TrainingSystem* training_create(CLLMModel* model, TrainingConfig* config, int num_threads);

// Train one epoch (always parallel, always crystalline)
float training_run_epoch(TrainingSystem* system);

// Cleanup
void training_free(TrainingSystem* system);
```

### Phase 4: Update All Callers
1. `tools/train_model.c` - Already correct
2. `app/training_thread.c` - Already correct
3. `src/crawler/continuous_training.c` - Update to use parallel system

## Recommended Approach

### Option A: Minimal Changes (Safest)
1. Merge crystalline loss into `cllm_training.c`
2. Keep both training systems for now
3. Update crawler to use threaded system
4. Document that threaded is preferred

### Option B: Full Consolidation (Cleanest)
1. Merge crystalline loss into `cllm_training.c`
2. Make `ThreadedTrainingSystem` the ONLY training system
3. Remove all single-threaded code
4. Rename to remove "threaded" suffix
5. Update all callers

### Option C: Pragmatic (Recommended)
1. **Keep current structure** but clarify naming:
   - `cllm_training.c` → Core training functions (forward/backward/loss)
   - `cllm_training_threaded.c` → Parallel orchestration (the main API)
   
2. **Merge crystalline** into `cllm_training.c`:
   - Move `cllm_compute_loss()` 
   - Delete `cllm_crystalline_training.c`
   
3. **Update documentation** to clarify:
   - `cllm_training.c` = Core training operations
   - `cllm_training_threaded.c` = Main training API (parallel)
   
4. **Update crawler** to use parallel system

5. **Add deprecation warnings** to single-threaded functions

## Implementation Order

1. ✅ Merge crystalline loss (move functions to cllm_training.c)
2. ✅ Delete cllm_crystalline_training.c and header
3. ✅ Update includes
4. ✅ Build and test
5. ✅ Update crawler to use parallel system
6. ✅ Add comments clarifying file purposes
7. ✅ Update documentation

## Files to Modify

### Delete:
- `src/ai/cllm_crystalline_training.c`
- `include/cllm_crystalline_training.h`

### Modify:
- `src/ai/cllm_training.c` (add crystalline loss)
- `src/crawler/continuous_training.c` (use parallel system)
- `include/cllm_training.h` (update declarations)

### Keep:
- `src/ai/cllm_training_threaded.c` (main training API)
- `include/cllm_training_threaded.h`
- `tools/train_model.c` (already correct)
- `app/training_thread.c` (already correct)

## Success Criteria

1. ✅ Only one loss implementation (crystalline)
2. ✅ Only one training system (parallel)
3. ✅ Clear file purposes
4. ✅ All tools use parallel system
5. ✅ Build successful
6. ✅ Tests pass