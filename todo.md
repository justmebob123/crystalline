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

## Current Status: Systematic Debugging of Threading System

### Critical Issues Identified
1. **Segmentation Faults**: Program crashes during batch processing
2. **Work Distribution Broken**: Only 3-4 of 12 threads get work
3. **Memory Corruption**: Invalid pointer access in threading system
4. **41 Type Warnings**: float*/double* mismatches

### Progress Made
- ✅ Locks restored and actually being called
- ✅ epoch_done timing fixed
- ✅ Build errors resolved
- ✅ Bounds checking added to batch queue

## Phase 1: Debug with Valgrind [NEXT]
- [ ] Run with valgrind to identify memory leaks and invalid access
- [ ] Check for use-after-free errors
- [ ] Check for double-free errors
- [ ] Check for uninitialized memory reads
- [ ] Document all memory issues found

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