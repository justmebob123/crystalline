# TODO: Fix Duplicate Functions and Architecture Issues

## Context
After rereading MASTER_PLAN and analyzing the codebase, discovered:
1. ✅ Build error in training_thread.c (FIXED)
2. ✅ Duplicate training functions (REMOVED)
3. Architecture doesn't match MASTER_PLAN OBJECTIVE 6A (flat vs recursive)

See: DUPLICATE_FUNCTIONS_ANALYSIS.md for full details

---

## Phase 1: Remove Duplicate Legacy Function ✅ COMPLETE

### 1.1 Delete Legacy Function ✅
- [x] Remove `threaded_train_epoch()` from `src/ai/cllm_training_threaded.c` (lines 2076-2243)
- [x] Remove declaration from `include/cllm_training_threaded.h`

### 1.2 Update Callers ✅
- [x] Update `src/crawler/continuous_training.c` to use `threaded_train_epoch_lockfree()`
- [x] Update `demos/threaded_training_demo.c` to use `threaded_train_epoch_lockfree()`
- [x] Update `tests/test_repo_training.c` to use `threaded_train_epoch_lockfree()`
- [x] Update `test_race_fix.c` to use `threaded_train_epoch_lockfree()`
- [x] Update `test_phase4.c` to use `threaded_train_epoch_lockfree()`
- [x] Update `train_cllm_repo.c` to use `threaded_train_epoch_lockfree()`
- [x] Update `repo_training_data/training_thread.c` to use `threaded_train_epoch_lockfree()`
- [x] Update `repo_training_data/train_model.c` to use `threaded_train_epoch_lockfree()`
- [x] Update comment in `src/ai/cllm_training_threaded.c`

### 1.3 Verify Build ✅
- [x] Run `make clean && make` - SUCCESS (0 errors, 0 warnings)
- [x] Run `make app` - SUCCESS (3 pre-existing warnings in tab_video.c)
- [x] Run `make tools` - SUCCESS
- [x] Verify zero errors, zero warnings (except pre-existing)

### 1.4 Commit Changes ✅
- [x] Git add all changes
- [x] Commit with message: "Remove legacy threaded_train_epoch() function"
- [x] Push to main branch (commit 929681b)

---

## Phase 2: Document Architecture Gap (NEXT)

### 2.1 Analysis
- [ ] Document current flat architecture (ThreadedTrainingSystem)
- [ ] Document required recursive architecture (OBJECTIVE 6A)
- [ ] Identify what needs to be implemented
- [ ] Estimate effort for recursive hierarchy implementation

### 2.2 Create Implementation Plan
- [ ] Design recursive training system API
- [ ] Design thread spawning mechanism
- [ ] Design parent-child coordination
- [ ] Design dynamic depth management
- [ ] Design load balancing strategy

---

## Phase 3: Implement Recursive Hierarchy (FUTURE - MAJOR WORK)

This is deferred to a future session as it requires 2-3 hours of focused work.

See MASTER_PLAN OBJECTIVE 6A for requirements.

---

## Summary

**Phase 1 Status**: ✅ COMPLETE
- Legacy function removed (168 lines deleted)
- All 9 callers updated
- Build successful (0 errors)
- Ready to commit

**Next Steps**: Commit changes, then document architecture gap