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
   
   ## Current Status: Critical Bug Found and Ready to Fix
   
   ### Critical Issues Identified
   1. **ROOT CAUSE FOUND**: Type mismatch in cllm_training_threaded.c causing buffer overflow
   2. **Work Distribution Broken**: Only 3-4 of 12 threads get work
   3. **41 Type Warnings**: float*/double* mismatches throughout codebase
   
   ### Progress Made
   - ✅ Locks restored and actually being called
   - ✅ epoch_done timing fixed
   - ✅ Build errors resolved
   - ✅ Bounds checking added to batch queue
   - ✅ Valgrind analysis completed
   - ✅ Root cause identified: double*/float* type mismatch
   
   ## Phase 1: Debug with Valgrind [COMPLETED]
   - [x] Run with valgrind to identify memory leaks and invalid access
   - [x] Check for use-after-free errors
   - [x] Check for double-free errors
   - [x] Check for uninitialized memory reads
   - [x] Document all memory issues found
   
   **CRITICAL BUG FOUND:**
   - Line 443 and 497 in cllm_training_threaded.c: logits declared as float* but used as double*
   - Buffer allocated as float* (4 bytes per element)
   - Code writes as double* (8 bytes per element)
   - Results in buffer overflow: writing 1,818,620 bytes into 1,818,624 byte buffer
   - This causes segmentation fault during backward pass
   
   ## Phase 1.5: Fix Critical Type Mismatch Bug [COMPLETED]
   - [x] Fix line 443: Change `double* logits` to `float* logits`
   - [x] Fix line 497: Change `double* logits` to `float* logits`
   - [x] Rebuild the project
   - [x] Restored correct train_model.c from git history
   - [x] Fixed tools/Makefile to link with algorithms library
   - [x] Test without valgrind - program now runs but HANGS during training
   
   **NEW ISSUE DISCOVERED:** Program no longer segfaults but HANGS during epoch processing
   - Threads are created successfully
   - Root control thread starts
   - But no batches are being processed
   - This is the work distribution issue mentioned in the summary
   
   **IMPORTANT NOTE:** The `crystalline` directory is a duplicate/backup. Main codebase is in root directory.
   
   ## Phase 1.6: Fix Thread Hanging Issue [COMPLETED]
   - [x] Debug why threads are not processing batches
   - [x] Added debug output to identify hang location
   - [x] Verified all pointers are valid
   - [x] Confirmed batches are being distributed correctly
   - [x] Confirmed forward pass now works!
   
   **MAJOR PROGRESS:**
   - ✅ Root control thread runs successfully
   - ✅ Batches distributed correctly (2 batches to symmetry group 0)
   - ✅ Sphere 1 receives batch and processes it
   - ✅ Forward pass completes successfully (was crashing before our float fix)
   - ❌ Still crashes during backward pass
   
   ## Phase 1.7: Fix Backward Pass Crash [NEXT]
   - [ ] Identify exact crash location in backward pass
   - [ ] Check for additional float/double mismatches
   - [ ] Check for buffer overflows in gradient computation
   - [ ] Verify prime_expf function is working correctly
   - [ ] Test with fixes applied
   
   ## Phase 2: Simplify for Testing
   - [ ] Create minimal single-threaded test
   - [ ] Verify basic training works without threading
   - [ ] Add threading with 2 workers only
   - [ ] Gradually scale up to 12 workers
   - [ ] Identify at what point it breaks
   
   ## Phase 3: Fix Work Distribution
   - [ ] Debug work queue push/pop logic
   - [ ] Verify all 12 threads are pulling from queue
   - [ ] Check for race conditions in queue access
   - [ ] Ensure batches are distributed evenly
   - [ ] Test with various batch counts
   
   ## Phase 4: Fix Memory Issues
   - [ ] Fix all valgrind-reported issues
   - [ ] Verify gradient buffer sizes match
   - [ ] Check for buffer overflows
   - [ ] Verify proper cleanup on thread exit
   - [ ] Test for memory leaks
   
   ## Phase 5: Verify NaN Gradient Fix
   - [ ] Run training to completion
   - [ ] Check for NaN gradients
   - [ ] Verify gradient values reasonable
   - [ ] Verify loss decreases
   - [ ] Compare with pre-lock-removal results
   
   ## Phase 6: Fix Type Warnings (After Functionality Works)
   - [ ] Decide on float vs double for entire codebase
   - [ ] Systematically update type declarations
   - [ ] Rebuild with zero warnings
   - [ ] Retest to ensure no regressions