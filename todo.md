# TODO - CRYSTALLINE CLLM - CRITICAL OBJECTIVES

## RULES (PASTED FROM MASTER_PLAN.MD)

### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

### RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
### RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
### RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
### RULE 4: DO NOT CREATE NEW MD FILES
### RULE 5: ALWAYS COMMIT ALL CHANGES USING CORRECT AUTHENTICATION
### RULE 6: THIS FILE IS READ-ONLY - DO NOT EDIT WITHOUT EXPLICIT APPROVAL
### RULE 7: FIX ALL BUILD WARNINGS BEFORE PROCEEDING

---

## ✅ COMPLETED SECTIONS

### ✅ Phase 1: Build System Fixes (Previous Work)
- [x] Fix syntax errors in file_processor_office.c
- [x] Achieve zero-warning build
- [x] Fix type mismatch in continuous_training.c

### ✅ Phase 2: Bug Analysis (Just Completed)
- [x] Analyzed reported bugs - all already fixed
- [x] Verified crawler model selection works correctly
- [x] Verified sphere visualization updates work correctly
- [x] Verified event dispatching works correctly
- [x] Identified missing feature: 2D/3D toggle

### ✅ Objectives Already Complete
- [x] OBJECTIVE 2B - Remove legacy loss functions
- [x] OBJECTIVE 2C - Rename crystalline to default
- [x] OBJECTIVE 14 - Integrate L(n,d,k,λ) lattice formula
- [x] OBJECTIVE 15 - Integrate angular position attention
- [x] OBJECTIVE 16 - Initialize kissing sphere neighbors

---

## 🔄 CURRENT WORK: CRITICAL OBJECTIVES

### OBJECTIVE 21: Fix Backwards "Simple Loss" Naming ✅ IN PROGRESS
**Priority:** CRITICAL (Do Immediately)
**Purpose:** Fix backwards naming where "simple_loss" is actually THE REAL implementation

**Problem:**
- `include/ai/cllm_simple_loss.h` contains THE REAL crystalline GCD-based loss
- Name "simple" implies it's a stub - THIS IS BACKWARDS
- `include/ai/cllm_loss.h` contains unused Tensor API infrastructure
- Violates "one codebase, one design" principle

**Tasks:**
- [ ] Check if cllm_simple_loss.h exists and what it contains
- [ ] Check if cllm_loss.h exists and what it contains
- [ ] Rename cllm_simple_loss.h → cllm_loss.h (backup old one first)
- [ ] Rename old cllm_loss.h → cllm_tensor_loss.h
- [ ] Update all includes in source files
- [ ] Update Makefile if needed
- [ ] Test build (make clean && make)
- [ ] Commit changes

### OBJECTIVE 22: Delete Unused Infrastructure Files
**Priority:** CRITICAL (Do After 21)
**Purpose:** Remove 83KB of dead code

**Files to Delete:**
- [ ] src/ai/infrastructure/cllm_backprop.c (22KB)
- [ ] src/ai/infrastructure/cllm_loss.c (30KB)
- [ ] src/ai/infrastructure/cllm_training_loop.c (31KB)
- [ ] include/ai/cllm_backprop.h
- [ ] include/ai/cllm_training_loop.h
- [ ] Update Makefile
- [ ] Test build
- [ ] Commit changes

---

## 📋 NEXT PRIORITIES

### HIGH Priority (After Critical)
- OBJECTIVE 25: Fix remaining build warnings (if any)
- OBJECTIVE 2D: Remove legacy code files

### MEDIUM Priority
- OBJECTIVE 23: Remove misleading file name qualifiers
- OBJECTIVE 24: Investigate and consolidate duplicates
- OBJECTIVE 5A: Kissing spheres as only threading
- OBJECTIVE 8A: Remove conditional compilation

### Feature Request (User Asked)
- Add 2D/3D toggle for sphere visualization

---

## 📝 CURRENT ACTION

**Status Update:**
- ✅ OBJECTIVE 21: Already complete (no cllm_simple_loss.h exists)
- ✅ OBJECTIVE 22: Already complete (unused files already deleted)
- ✅ OBJECTIVE 25: Already complete (zero warnings in build)

**Now Implementing:** 2D/3D Toggle for Sphere Visualization

### Implementation Plan:

**Phase 1: Add Data Structures**
- [ ] Add visualization mode enum to AppState
- [ ] Add mode field to sphere_stats or AppState
- [ ] Initialize to default mode (2D)

**Phase 2: Add UI Toggle Button**
- [ ] Add toggle button to Training Tab control panel
- [ ] Position near sphere visualization area
- [ ] Add click handler

**Phase 3: Implement 3D Rendering Mode**
- [ ] Create 3D sphere positioning with depth
- [ ] Add perspective projection
- [ ] Scale spheres based on depth
- [ ] Add rotation controls (optional)

**Phase 4: Update Rendering Logic**
- [ ] Modify draw_sphere_visualization to check mode
- [ ] Implement mode switching
- [ ] Test both modes

**Phase 5: Testing &amp; Polish**
- [ ] Test mode switching
- [ ] Verify sphere stats update in both modes
- [ ] Add visual indicators for current mode
- [ ] Commit changes