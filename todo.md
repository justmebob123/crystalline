# TODO - Crystalline CLLM Project

**CRITICAL RULES (from MASTER_PLAN.md) - MUST FOLLOW:**
- **Rule 0**: ALWAYS paste rules to top of todo.md with EVERY response, read MASTER_PLAN.md, AUDIT.md, SECONDARY_OBJECTIVES.md
- **Rule 1**: ALWAYS reread MASTER_PLAN.md before ANY action
- **Rule 2**: Reference AUDIT.md for architectural state
- **Rule 3**: Reference SECONDARY_OBJECTIVES.md for detailed tasks
- **Rule 4**: Do NOT create new .md files
- **Rule 5**: ALWAYS commit using: `git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main`
- **Rule 6**: MASTER_PLAN.md is READ-ONLY - do not edit without explicit approval
- **Rule 7**: FIX ALL BUILD WARNINGS before proceeding

## CRITICAL ISSUES IDENTIFIED

### Issue 1: Locks NOT Being Used
**Problem**: Code calls `accumulate_gradients_lockfree()` instead of `accumulate_gradients()`
- The locked version was marked `__attribute__((unused))`
- Lines 1586 and 2095 call the lock-free version
- This means the restored locks are NEVER used
- **Status**: FIXED - Now calling `accumulate_gradients()` with locks

### Issue 2: Segmentation Fault
**Problem**: Segfault during training in `threaded_train_epoch_lockfree`
- Occurs during batch processing
- Related to work queue or batch management
- Need to debug memory access patterns
- **Status**: INVESTIGATING

### Issue 3: Single Core Usage
**Problem**: Only using 1 core instead of 12-fold symmetry
- Workers report "processed 0 batches" or "processed 1 batch"
- Not distributing work properly across 12 threads
- Violates 12-fold symmetry architecture
- **Status**: NOT YET ADDRESSED

### Issue 4: 41 Type Warnings
**Problem**: float* vs double* pointer type mismatches
- Architectural inconsistency in type system
- 41 warnings across multiple files
- Safe but not ideal
- **Status**: DOCUMENTED, will fix after functionality restored

## Current Phase: Fix Critical Threading Issues

### Phase 1: Restore Proper Lock Usage [IN PROGRESS]
- [x] Remove `__attribute__((unused))` from `accumulate_gradients()`
- [x] Replace `accumulate_gradients_lockfree()` calls with `accumulate_gradients()`
- [x] Verify locks are actually being used
- [ ] Rebuild and test
- [ ] Verify no segfaults with proper locking

### Phase 2: Fix Segmentation Fault
- [ ] Add debug logging to identify exact crash location
- [ ] Check batch lifecycle and memory management
- [ ] Verify work queue integrity
- [ ] Check for race conditions in batch access
- [ ] Test with single thread first, then scale up

### Phase 3: Restore 12-Fold Symmetry
- [ ] Verify work distribution across all 12 threads
- [ ] Check why workers process 0 or 1 batches
- [ ] Ensure proper load balancing
- [ ] Verify kissing spheres geometry is maintained
- [ ] Test with multiple batches per thread

### Phase 4: Verify NaN Gradient Fix
- [ ] Run training to completion
- [ ] Check for NaN gradients
- [ ] Verify gradient values are reasonable
- [ ] Verify loss decreases over epochs
- [ ] Compare with previous results

### Phase 5: Type System Cleanup (If Needed)
- [ ] Decide on float vs double for entire codebase
- [ ] Systematically update all type declarations
- [ ] Rebuild with zero warnings
- [ ] Retest to ensure no regressions

### Phase 6: Final Validation
- [ ] Run full training session
- [ ] Verify all 12 threads active
- [ ] Monitor symmetry group distribution
- [ ] Test convergence
- [ ] Document results

## Notes

**User Feedback**: 
- Build is NOT clean (41 warnings)
- Testing showed segfaults (plural)
- Only using single core (lost 12-fold symmetry)
- Need to fix these critical issues before claiming success

**Root Causes Identified**:
1. Locks restored but NOT being called (lock-free version still in use)
2. Memory management issues causing segfaults
3. Work distribution broken (not using all 12 threads)
4. Type system inconsistency (float vs double)

**Priority**: Fix threading and segfaults FIRST, then address type warnings