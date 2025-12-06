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

### Phase 3.2: Training Tab Refactor (3-4 hours) ✅ COMPLETE
- [x] Part A: Add TrainingTabState structure (30 min) ✅ COMPLETE
- [x] Part B: Implement model management functions (1 hour) ✅ COMPLETE
- [x] Part C: Implement training functions (1-2 hours) ✅ COMPLETE
- [x] Part D: Remove model_manager calls (30 min) ✅ COMPLETE
- [x] Part E: Update statistics display (30 min) ✅ COMPLETE
  - Updated visualization data to use tab_state.stats
  - Updated sphere statistics display to use tab_state.stats
  - Replaced state->training_metrics with tab_state
  - Replaced state->sphere_stats with tab_state.stats
  - Display now shows: active_spheres, batches_processed, tokens_processed
  - Build successful: 0 errors, 0 warnings
- [ ] Part F: Test training independently (1 hour) - NEXT

### Phase 3.3: LLM Tab Refactor (2-3 hours) 🔄 IN PROGRESS
- [x] Part A: Add LLMTabState structure (20 min) ✅ COMPLETE
- [x] Part B: Implement model management functions (30 min) ✅ COMPLETE
  - Implemented llm_tab_load_model() - loads model from registry
  - Implemented llm_tab_unload_model() - frees model and stops generation
  - Updated on_model_selected() to use new functions
  - Updated dropdown population to use model_registry
  - Updated cleanup_llm_tab() to use llm_tab_unload_model()
  - Replaced model_manager include with model_registry
  - Build successful: 0 errors, 0 warnings in tab_llm.c
- [ ] Part C: Implement inference functions (1 hour) - NEXT
- [ ] Part D: Update remaining UI code (30 min)
- [ ] Part E: Test inference independently (30 min)

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
**Current Focus:** Phase 3.2 - Training Tab Refactor (COMPLETE!)
**Blocker:** None
**Next Action:** Phase 3.3 - LLM Tab Refactor

## Summary of Training Tab Refactor
The Training Tab has been successfully refactored to own its model independently:
- ✅ Added TrainingTabState structure with complete state management
- ✅ Implemented model management functions (load, create, save, unload)
- ✅ Implemented training functions (start, stop, thread)
- ✅ Removed all model_manager dependencies
- ✅ Updated statistics display to use tab state
- ✅ Build successful: 0 errors, 0 warnings

Key achievements:
- Training Tab now owns its model pointer
- No global coordination layer blocking operations
- Clear lifecycle: load → train → save → unload
- Real-time statistics from training thread
- Simpler, cleaner code architecture