# TODO - Crystalline CLLM Integration Project

## RULES (MUST READ FIRST)
1. ALWAYS paste these rules to the top of todo.md
2. ALWAYS reread MASTER_PLAN.md before starting work
3. ALWAYS reference AUDIT.md for current architectural state
4. ALWAYS reference SECONDARY_OBJECTIVES.md for detailed tasks
5. NEVER assume - always verify current state
6. ALWAYS commit and push changes with descriptive messages
7. FOCUS on integrating existing crystalline systems, not creating new ones
8. Use proper naming conventions from existing codebase
9. DO NOT CREATE PARALLEL IMPLEMENTATIONS - FIX THE EXISTING CODE
10. DO NOT SHORTCUT - PROPERLY MERGE ALL CHANGES INTO EXISTING CODEBASE

---

## CURRENT FOCUS: OBJECTIVE 2 - Fix Training Pipeline

**Status:** PARTIALLY ANALYZED - Need to complete analysis and fixes

### Completed Analysis
- [x] Identified OLD vs NEW training functions
- [x] Mapped function call chains
- [x] Identified fallback paths

### Remaining Analysis
- [ ] Analyze `cllm_train_complete.c` for unique features
- [ ] Analyze `cllm_crystalline_training.c` for optimizations
- [ ] Analyze `cllm_training_parallel.c` for unique features
- [ ] Remove fallbacks in `tools/train_model.c`
- [ ] Delete redundant files
- [ ] Update Makefile

---

## OBJECTIVE 2A: Integrate Crystalline GCD Optimizations

**Purpose:** Wire crystalline optimizations into training pipeline

### Tasks
- [ ] Wire `cllm_compute_loss_crystalline()` into actual loss computation
- [ ] Integrate GCD similarity with kissing spheres training
- [ ] Integrate Ulam spiral locality with batch processing
- [ ] Add configuration option to enable/disable crystalline optimizations
- [ ] Benchmark performance improvement (expecting 20-400x speedup)
- [ ] Verify correctness of GCD-based similarity vs standard dot product
- [ ] Test with various model sizes and datasets
- [ ] Document performance characteristics
- [ ] Update training pipeline to use crystalline loss when enabled

---

## OBJECTIVE 2B: Remove ALL Legacy Loss Functions

**Purpose:** Complete the crystalline design by removing standard cross-entropy

### Tasks
- [ ] Remove `cllm_compute_loss_training()` function (standard cross-entropy)
- [ ] Remove the conditional flag check in `cllm_train_epoch()`
- [ ] Make `cllm_compute_loss_crystalline()` the ONLY loss function
- [ ] Rename `cllm_compute_loss_crystalline()` to `cllm_compute_loss()`
- [ ] Remove `use_crystalline_optimizations` flag from config struct
- [ ] Update all callers to use crystalline loss directly
- [ ] Remove any references to "standard" loss in comments/docs

---

## OBJECTIVE 3A: Crystalline Math Everywhere

**Purpose:** Ensure NO standard math library usage anywhere

### Tasks
- [ ] Search ALL files for `#include <math.h>`
- [ ] Search ALL files for standard math functions (sin, cos, exp, log, sqrt, pow, etc.)
- [ ] Replace any found with crystalline equivalents (prime_sinf, prime_cosf, etc.)
- [ ] Verify NO external math dependencies
- [ ] Document crystalline math usage
- [ ] Add verification script to prevent future math.h usage

---

## OBJECTIVE 5A: Kissing Spheres as ONLY Threading

**Purpose:** Remove all non-kissing-spheres threading code

### Tasks
- [ ] Remove ALL fallbacks to old threading
- [ ] Make kissing spheres mandatory (no single-threaded fallback)
- [ ] Remove `cllm_train_epoch_mt()` completely
- [ ] Update tools to require kissing spheres
- [ ] Document kissing spheres as the only threading model
- [ ] Remove any single-threaded training paths
- [ ] Ensure all training goes through kissing spheres

---

## Build Status
- **Errors:** 0 ✅
- **Warnings:** Some (need to fix)
- **Libraries:** All building ✅

---

## Next Actions
1. Complete OBJECTIVE 2 analysis
2. Implement OBJECTIVE 2A (Crystalline GCD integration)
3. Implement OBJECTIVE 2B (Remove legacy loss functions)
4. Verify OBJECTIVE 3A (Crystalline math everywhere)
5. Implement OBJECTIVE 5A (Kissing spheres only)

