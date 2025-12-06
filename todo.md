# TODO: Model Management Redesign - Refined Approach

## Overview
Redesign model management to enable concurrent operations while keeping Models Tab as a unified management/monitoring UI.

## User Requirements (CRITICAL)
- ✅ KEEP Models Tab for unified model management UI
- ✅ KEEP model statistics, properties, metadata display
- ✅ KEEP model management features (create, rename, delete)
- ✅ KEEP training state tracking and display
- ✅ KEEP unified view of all models
- ❌ REMOVE global coordination layer preventing concurrent ops
- ❌ REMOVE locks blocking training + inference
- ✅ ENABLE concurrent training and inference

## Phase 1: Architecture Design ✅ COMPLETE
- [x] Analyze CLI tool architecture
- [x] Analyze current UI model manager
- [x] Identify problems with current approach
- [x] Design refined solution
- [x] Document analysis

## Phase 2: Refined Design ✅ COMPLETE
- [x] Design Models Tab as monitoring UI (not coordination)
- [x] Design per-tab model ownership
- [x] Design training state tracking
- [x] Design model metadata system
- [x] Create implementation plan
- [x] Get user approval

## Phase 3: Implementation 🔄 IN PROGRESS

### Phase 3.1: Model Registry (2-3 hours) ✅ COMPLETE
- [x] Create include/cllm_model_registry.h
- [x] Create src/ai/cllm_model_registry.c
- [x] Implement registry_init()
- [x] Implement registry_scan()
- [x] Implement registry_get()
- [x] Implement registry_rename()
- [x] Implement registry_delete()
- [x] Update Makefile (automatic via wildcard)
- [x] Build successful (zero errors)
- [x] Test registry operations (all tests passed!)

**Registry Test Results:**
- ✅ Scanned models directory
- ✅ Found and registered final_model.cllm
- ✅ Read metadata correctly (500 vocab, 128 dim, 4 layers)
- ✅ Get by name works
- ✅ Get by index works
- ✅ Exists check works
- ✅ Path generation works
- ✅ All operations successful

### Phase 3.2: Training Tab Refactor (3-4 hours) 🔄 IN PROGRESS
- [x] Part A: Add TrainingTabState structure (30 min) ✅ COMPLETE
  - Added TrainingTabState typedef with model ownership fields
  - Added training state fields (training, thread, flags)
  - Added statistics structure with sphere stats
  - Added tab_state to g_training_ui
  - Build successful: 0 errors, 0 warnings
- [ ] Part B: Implement model management functions (1 hour) - NEXT
- [ ] Part C: Implement training functions (1-2 hours)
- [ ] Part D: Remove model_manager calls (30 min)
- [ ] Part E: Update statistics display (30 min)
- [ ] Part F: Test training independently (1 hour)

### Phase 3.3: LLM Tab Refactor (2-3 hours)
- [ ] Add per-tab model state
- [ ] Remove model_manager calls
- [ ] Implement load/unload
- [ ] Add inference stats tracking
- [ ] Test inference independently

### Phase 3.4: Models Tab Refactor (4-5 hours)
- [ ] Use registry instead of manager
- [ ] Add training stats display
- [ ] Add status indicators
- [ ] Implement management operations
- [ ] Test Models Tab display

### Phase 3.5: Crawler Tab Refactor (2-3 hours)
- [ ] Add per-tab model state
- [ ] Remove model_manager calls
- [ ] Add crawler stats tracking
- [ ] Test crawler independently

### Phase 3.6: Cleanup (1-2 hours)
- [ ] Remove coordination code
- [ ] Update model_manager to registry
- [ ] Clean up unused code
- [ ] Update documentation

## Phase 4: Testing & Verification
- [ ] Test concurrent training + inference
- [ ] Test model statistics display
- [ ] Test model management features
- [ ] Verify memory usage
- [ ] Test all tabs independently

## Status
**Current Focus:** Phase 2 - Refined Design
**Blocker:** None
**Next Action:** Design refined architecture with Models Tab as monitoring UI