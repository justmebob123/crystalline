# TODO - OBJECTIVE 25: FIX ALL 78 BUILD WARNINGS

## 🔒 RULES (PASTED FROM MASTER_PLAN)

### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

### RULE 7: FIX ALL BUILD WARNINGS BEFORE PROCEEDING
All code must compile with zero warnings before moving to the next objective.

---

## ✅ COMPLETED OBJECTIVES

- ✅ OBJECTIVE 2B: Remove legacy loss functions
- ✅ OBJECTIVE 2C: Rename crystalline to default
- ✅ OBJECTIVE 21: Fix backwards "simple_loss" naming
- ✅ OBJECTIVE 22: Delete 83KB unused infrastructure code
- ✅ Architecture analysis complete
- ✅ WARNING_ANALYSIS.md created (comprehensive categorization)

---

## 🎯 CURRENT: OBJECTIVE 25 - FIX 78 BUILD WARNINGS

**Total:** 78 warnings
**Categories:**
- HIGH: 60 type mismatches (BigFixed** vs float*)
- MEDIUM: 14 sign comparisons (uint32_t vs int)
- MEDIUM: 8 unused variables/parameters
- MEDIUM: 5 unused functions
- LOW: 5 statements with no effect
- LOW: 2 misc warnings

---

## 📋 PHASE 1: FIX HIGH-PRIORITY TYPE MISMATCHES (60 warnings)

### 1.1: Inference Functions (8 warnings in cllm_inference.c)

**Problem:** Helper functions expect float* but receive BigFixed**

- [ ] Create `cllm_apply_temperature_bigfixed()` in cllm_inference.c
- [ ] Create `cllm_softmax_bigfixed()` in cllm_inference.c
- [ ] Create `cllm_sample_top_k_bigfixed()` in cllm_inference.c
- [ ] Create `cllm_sample_top_p_bigfixed()` in cllm_inference.c
- [ ] Update `cllm_generate()` to use BigFixed versions
- [ ] Test build (should reduce warnings by 8)
- [ ] Commit changes

### 1.2: Optimizer Functions (12 warnings in cllm_optimizer.c)

**Problem:** Optimizer expects float* but receives BigFixed**

- [ ] Create `adam_update_params_bigfixed()` static function
- [ ] Update `cllm_adam_step()` to use BigFixed operations
- [ ] Update `cllm_sgd_momentum_step()` to use BigFixed operations
- [ ] Fix printf format (use bigfixed_to_double for printing)
- [ ] Test build (should reduce warnings by 12)
- [ ] Commit changes

### 1.3: Training Type Mismatches (25 warnings in cllm_training.c)

**Problem:** Mixed float* and BigFixed** allocations

- [ ] Fix attention cache allocations (lines 561-566) - use bigfixed_array_create
- [ ] Fix backward pass type handling (lines 2436, 2463, 2491, 2522)
- [ ] Fix attention forward argument types (line 3442)
- [ ] Fix cymatic resonance call (line 1594)
- [ ] Test build (should reduce warnings by 25)
- [ ] Commit changes

### 1.4: LayerNorm Functions (3 warnings in cllm_layernorm.c)

**Problem:** Missing include and wrong allocation

- [ ] Add `#include "cllm_layernorm_bigfixed.h"` at top
- [ ] Fix gamma allocation (line 68) - use bigfixed_array_create
- [ ] Fix beta allocation (line 69) - use bigfixed_array_create
- [ ] Test build (should reduce warnings by 3)
- [ ] Commit changes

### 1.5: Validation Functions (7 warnings in cllm_validate.c)

**Problem:** Validation expects float* but receives BigFixed**

- [ ] Create `cllm_check_numerical_stability_bigfixed()` function
- [ ] Update all 7 call sites to use BigFixed version
- [ ] Test build (should reduce warnings by 7)
- [ ] Commit changes

### 1.6: LLL Embeddings (1 warning in cllm_lll_embeddings.c)

**Problem:** Wrong allocation type

- [ ] Fix line 285 - use bigfixed_array_create instead of calloc
- [ ] Test build (should reduce warnings by 1)
- [ ] Commit changes

### 1.7: Tools Inference (4 warnings in tools/cllm_inference.c)

**Problem:** Same as main inference

- [ ] Update to use BigFixed versions of helper functions
- [ ] Test build (should reduce warnings by 4)
- [ ] Commit changes

**Phase 1 Total:** Should eliminate 60 warnings

---

## 📋 PHASE 2: FIX MEDIUM-PRIORITY WARNINGS (27 warnings)

### 2.1: Sign Comparisons (14 warnings in cllm_training.c)

**Problem:** Comparing uint32_t with int

- [ ] Cast comparisons to same type
- [ ] Or change variable types to match
- [ ] Test build
- [ ] Commit changes

### 2.2: Unused Variables (5 warnings)

- [ ] Fix `gradient_scale` in cllm_optimizer_wrapper.c:35
- [ ] Fix `scale` in cllm_production.c:393
- [ ] Fix `ff_hidden` in cllm_training_threaded.c:388
- [ ] Fix `ln` in cllm_training_threaded.c:409
- [ ] Fix `std` in cllm_training_threaded.c:418
- [ ] Test build
- [ ] Commit changes

### 2.3: Unused Parameters (3 warnings)

- [ ] Fix `precision` in cllm_feedforward_bigfixed.c:28 - add (void)precision
- [ ] Fix `layer` in cllm_training.c:997 - add (void)layer
- [ ] Fix `token_ids` in cllm_training.c:1001 - add (void)token_ids
- [ ] Test build
- [ ] Commit changes

### 2.4: Unused Functions (5 warnings in cllm_utils.c)

- [ ] Mark functions as static inline or remove:
  - cllm_reset_model (line 365)
  - cllm_clone_config (line 399)
  - cllm_compare_configs (line 414)
  - cllm_get_version (line 431)
  - cllm_get_build_info (line 440)
- [ ] Test build
- [ ] Commit changes

**Phase 2 Total:** Should eliminate 27 warnings

---

## 📋 PHASE 3: FIX LOW-PRIORITY WARNINGS (7 warnings)

### 3.1: Statements with No Effect (5 warnings in cllm_production.c)

- [ ] Fix lines 370, 372, 374, 382, 384 in clip_gradients function
- [ ] Either assign to variable or remove statements
- [ ] Test build
- [ ] Commit changes

### 3.2: Control Flow (1 warning in cllm_utils.c)

- [ ] Add return statement at line 442
- [ ] Test build
- [ ] Commit changes

### 3.3: Pointer Mismatch (1 warning in cllm_training.c)

- [ ] Fix line 2436 conditional expression
- [ ] Add proper cast
- [ ] Test build
- [ ] Commit changes

**Phase 3 Total:** Should eliminate 7 warnings

---

## 🎯 EXECUTION STRATEGY

**Start with:** Phase 1.1 (Inference functions) - Most visible to users
**Then:** Work through each category systematically
**Goal:** Zero warnings build

**After each fix:**
1. Test build
2. Verify warning count decreased
3. Commit changes
4. Move to next category

---

## 📊 PROGRESS TRACKING

- [ ] Phase 1.1: Inference (8 warnings)
- [ ] Phase 1.2: Optimizer (12 warnings)
- [ ] Phase 1.3: Training (25 warnings)
- [ ] Phase 1.4: LayerNorm (3 warnings)
- [ ] Phase 1.5: Validation (7 warnings)
- [ ] Phase 1.6: LLL Embeddings (1 warning)
- [ ] Phase 1.7: Tools Inference (4 warnings)
- [ ] Phase 2.1: Sign Comparisons (14 warnings)
- [ ] Phase 2.2: Unused Variables (5 warnings)
- [ ] Phase 2.3: Unused Parameters (3 warnings)
- [ ] Phase 2.4: Unused Functions (5 warnings)
- [ ] Phase 3.1: No Effect Statements (5 warnings)
- [ ] Phase 3.2: Control Flow (1 warning)
- [ ] Phase 3.3: Pointer Mismatch (1 warning)

**Target:** 0 warnings
**Current:** 78 warnings