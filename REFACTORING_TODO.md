# Architecture Refactoring TODO

## Overview
Restructure the codebase to move generic threading and visualization from CLLM layer to algorithms layer, remove naming redundancy, and create a unified visualization framework.

---

## Phase 1: Move Threading to Algorithms Layer ⏳

### 1.1: Create Generic Sphere Threading API
- [ ] Create `algorithms/include/sphere_threading.h`
- [ ] Define `SphereThreadingModel` structure
- [ ] Define generic threading API functions
- [ ] Document 12-fold symmetry principles
- [ ] Document cache locality optimization

### 1.2: Implement Generic Sphere Threading
- [ ] Create `algorithms/src/sphere_threading.c`
- [ ] Move initialization logic from `cllm_kissing_spheres.c`
- [ ] Move neighbor setup logic (clock geometry)
- [ ] Move cache optimization logic
- [ ] Implement generic work distribution
- [ ] Add validation functions

### 1.3: Update Algorithms Makefile
- [ ] Add `sphere_threading.o` to build
- [ ] Update dependencies
- [ ] Verify clean build

### 1.4: Create CLLM Threading Wrapper
- [ ] Create `include/cllm_threading.h` (simplified)
- [ ] Create `src/ai/cllm_threading.c` (wrapper)
- [ ] Use algorithms layer `sphere_threading.h`
- [ ] Keep only CLLM-specific logic
- [ ] Update all CLLM code to use new API

### 1.5: Update Tests
- [ ] Create `algorithms/tests/test_sphere_threading.c`
- [ ] Move tests from `tests/test_kissing_spheres_integration.c`
- [ ] Update CLLM tests to use new API
- [ ] Verify all tests pass

---

## Phase 2: Create Generic Visualization Framework ⏳

### 2.1: Design Visualization API
- [ ] Create `algorithms/include/visualization.h`
- [ ] Define `VisualizationData` structure
- [ ] Define `VisualizationMode` enum (2D, 3D, Crystalline)
- [ ] Define projection functions
- [ ] Define statistics functions
- [ ] Define export functions

### 2.2: Implement Core Visualization
- [ ] Create `algorithms/src/visualization.c`
- [ ] Implement data structure management
- [ ] Implement statistics calculation
- [ ] Implement JSON export
- [ ] Move logic from `cllm_kissing_spheres_visualization.c`

### 2.3: Implement 2D Projection
- [ ] Create `algorithms/src/visualization_2d.c`
- [ ] Implement 2D projection algorithms
- [ ] Implement 2D layout algorithms
- [ ] Implement 2D connection rendering data

### 2.4: Implement 3D Projection
- [ ] Create `algorithms/src/visualization_3d.c`
- [ ] Implement 3D projection algorithms
- [ ] Implement 3D sphere positioning
- [ ] Implement 3D connection geometry
- [ ] Use existing `sphere_packing.h` utilities

### 2.5: Implement Crystalline Projection
- [ ] Create `algorithms/src/visualization_crystalline.c`
- [ ] Implement crystalline structure projection
- [ ] Implement 12-fold symmetry visualization
- [ ] Implement clock lattice visualization
- [ ] Integrate with `clock_lattice.h`

### 2.6: Update Algorithms Makefile
- [ ] Add visualization object files
- [ ] Update dependencies
- [ ] Verify clean build

### 2.7: Create CLLM Visualization Wrapper
- [ ] Create `include/cllm_visualization.h` (simplified)
- [ ] Create `src/ai/cllm_visualization.c` (wrapper)
- [ ] Use algorithms layer `visualization.h`
- [ ] Add CLLM-specific visualization (attention, embeddings)
- [ ] Update all CLLM code to use new API

### 2.8: Update UI Layer
- [ ] Update `app/ui/sphere_visualization.c` to use algorithms API
- [ ] Update `app/ui/crystalline_visualization.c` to use algorithms API
- [ ] Keep only SDL rendering logic in UI
- [ ] Remove duplicate visualization code
- [ ] Verify UI still works

### 2.9: Update Tests
- [ ] Create `algorithms/tests/test_visualization.c`
- [ ] Move tests from `tests/test_kissing_spheres_visualization.c`
- [ ] Test 2D projection
- [ ] Test 3D projection
- [ ] Test Crystalline projection
- [ ] Test statistics
- [ ] Test JSON export
- [ ] Update CLLM visualization tests
- [ ] Verify all tests pass

---

## Phase 3: Implement Memory Management ⏳

### 3.1: Design Memory Management API
- [ ] Create `algorithms/include/memory_management.h`
- [ ] Define `SphereMemoryManager` structure
- [ ] Define allocation functions
- [ ] Define cache optimization functions
- [ ] Define compression functions

### 3.2: Implement Memory Management
- [ ] Create `algorithms/src/memory_management.c`
- [ ] Implement sphere-based allocation
- [ ] Implement cache-aware allocation
- [ ] Implement memory pooling
- [ ] Implement compression algorithms

### 3.3: Integrate with Threading
- [ ] Update `sphere_threading.c` to use memory management
- [ ] Optimize memory layout for cache locality
- [ ] Add memory statistics

### 3.4: Update Tests
- [ ] Create `algorithms/tests/test_memory_management.c`
- [ ] Test allocation
- [ ] Test cache optimization
- [ ] Test compression
- [ ] Verify performance improvements

---

## Phase 4: Remove Naming Redundancy ⏳

### 4.1: Rename Files (Algorithms Layer)
- [ ] (Already correct - no "kissing_spheres" in algorithms)

### 4.2: Rename Files (CLLM Layer)
- [ ] `cllm_kissing_spheres.h` → `cllm_threading.h`
- [ ] `cllm_kissing_spheres.c` → `cllm_threading.c`
- [ ] `cllm_kissing_spheres_visualization.h` → `cllm_visualization.h`
- [ ] `cllm_kissing_spheres_visualization.c` → `cllm_visualization.c`
- [ ] `cllm_kissing_spheres_attention.h` → (merge into `cllm_attention.h`)
- [ ] `cllm_kissing_spheres_attention.c` → (merge into `cllm_attention.c`)
- [ ] `cllm_kissing_spheres_embeddings.h` → (merge into `cllm_embeddings.h`)
- [ ] `cllm_kissing_spheres_embeddings.c` → (merge into `cllm_embeddings.c`)

### 4.3: Rename Files (Tests)
- [ ] `test_kissing_spheres_visualization.c` → `test_visualization.c` (in algorithms/tests)
- [ ] `test_kissing_spheres_integration.c` → `test_sphere_threading.c` (in algorithms/tests)
- [ ] Create `test_cllm_threading.c` (CLLM-specific tests)

### 4.4: Update All Includes
- [ ] Update all `#include` statements
- [ ] Update Makefiles
- [ ] Update documentation
- [ ] Verify clean build

### 4.5: Update Function Names
- [ ] Remove "kissing_spheres" from function names
- [ ] Use simple, clear names
- [ ] Update all call sites
- [ ] Verify all tests pass

---

## Phase 5: Comprehensive Testing ⏳

### 5.1: Algorithms Layer Tests
- [ ] Test sphere threading initialization
- [ ] Test work distribution
- [ ] Test neighbor relationships
- [ ] Test 12-fold symmetry
- [ ] Test cache optimization
- [ ] Test 2D visualization
- [ ] Test 3D visualization
- [ ] Test Crystalline visualization
- [ ] Test statistics calculation
- [ ] Test JSON export
- [ ] Test memory management
- [ ] Verify 100% pass rate

### 5.2: CLLM Layer Tests
- [ ] Test CLLM threading integration
- [ ] Test model training with sphere threading
- [ ] Test attention visualization
- [ ] Test embedding visualization
- [ ] Test end-to-end training pipeline
- [ ] Verify 100% pass rate

### 5.3: Integration Tests
- [ ] Test cross-layer communication
- [ ] Test performance (should match or exceed current)
- [ ] Test memory efficiency
- [ ] Test cache hit rates
- [ ] Test load balancing
- [ ] Verify no regressions

### 5.4: UI Tests
- [ ] Test 2D visualization rendering
- [ ] Test 3D visualization rendering
- [ ] Test Crystalline visualization rendering
- [ ] Test real-time updates
- [ ] Test user interactions
- [ ] Verify UI functionality

---

## Phase 6: Test Unified CLLM Tool ⏳

### 6.1: Training Tests
- [ ] Test small model training (vocab=1000, dim=64)
- [ ] Test medium model training (vocab=10000, dim=128)
- [ ] Test large model training (vocab=50000, dim=256)
- [ ] Test with different batch sizes
- [ ] Test with different learning rates
- [ ] Test checkpoint saving/loading
- [ ] Test resume from checkpoint
- [ ] Verify convergence
- [ ] Verify loss decreases
- [ ] Verify model quality

### 6.2: Inference Tests
- [ ] Test inference with trained model
- [ ] Test text generation
- [ ] Test perplexity calculation
- [ ] Test with different temperatures
- [ ] Test with different top-k values
- [ ] Test with different sequence lengths
- [ ] Verify output quality
- [ ] Verify inference speed

### 6.3: Performance Tests
- [ ] Benchmark training speed
- [ ] Benchmark inference speed
- [ ] Benchmark memory usage
- [ ] Benchmark cache efficiency
- [ ] Compare with baseline
- [ ] Verify improvements maintained

### 6.4: Stress Tests
- [ ] Test with very large models
- [ ] Test with very long sequences
- [ ] Test with many epochs
- [ ] Test with limited memory
- [ ] Test with limited CPU
- [ ] Verify stability
- [ ] Verify no memory leaks

---

## Phase 7: Documentation ⏳

### 7.1: API Documentation
- [ ] Document algorithms/sphere_threading.h
- [ ] Document algorithms/visualization.h
- [ ] Document algorithms/memory_management.h
- [ ] Document cllm_threading.h
- [ ] Document cllm_visualization.h
- [ ] Add usage examples

### 7.2: Architecture Documentation
- [ ] Update MASTER_PLAN.md
- [ ] Update AUDIT.md
- [ ] Create ARCHITECTURE.md
- [ ] Document layer responsibilities
- [ ] Document data flow
- [ ] Document threading model

### 7.3: User Documentation
- [ ] Update README.md
- [ ] Create visualization guide
- [ ] Create threading guide
- [ ] Create performance tuning guide
- [ ] Add troubleshooting section

---

## Phase 8: Final Verification ⏳

### 8.1: Build Verification
- [ ] Clean build from scratch
- [ ] Verify 0 errors
- [ ] Verify 0 warnings
- [ ] Verify all libraries build
- [ ] Verify all tools build
- [ ] Verify all tests build

### 8.2: Test Verification
- [ ] Run all algorithms tests
- [ ] Run all CLLM tests
- [ ] Run all integration tests
- [ ] Run all UI tests
- [ ] Verify 100% pass rate
- [ ] Verify no regressions

### 8.3: Performance Verification
- [ ] Run performance benchmarks
- [ ] Compare with baseline
- [ ] Verify improvements maintained
- [ ] Verify no performance regressions
- [ ] Document performance metrics

### 8.4: Code Quality Verification
- [ ] Verify no code duplication
- [ ] Verify clear separation of concerns
- [ ] Verify consistent naming
- [ ] Verify proper documentation
- [ ] Verify clean architecture

---

## Success Criteria

### Build Quality
- ✅ 0 compilation errors
- ✅ 0 warnings
- ✅ All libraries build successfully
- ✅ All tools build successfully
- ✅ All tests build successfully

### Test Quality
- ✅ 100% test pass rate
- ✅ All algorithms tests pass
- ✅ All CLLM tests pass
- ✅ All integration tests pass
- ✅ All UI tests pass

### Architecture Quality
- ✅ Clear layer separation
- ✅ No naming redundancy
- ✅ Generic algorithms in algorithms layer
- ✅ Specific implementations in CLLM layer
- ✅ Reusable components

### Performance Quality
- ✅ No performance regressions
- ✅ Improvements maintained
- ✅ Memory efficiency maintained
- ✅ Cache efficiency maintained

### Documentation Quality
- ✅ Complete API documentation
- ✅ Clear architecture documentation
- ✅ Comprehensive user guides
- ✅ Usage examples provided

---

## Estimated Timeline

- **Phase 1:** 4-6 hours (Threading to algorithms)
- **Phase 2:** 6-8 hours (Visualization framework)
- **Phase 3:** 3-4 hours (Memory management)
- **Phase 4:** 2-3 hours (Naming cleanup)
- **Phase 5:** 4-6 hours (Comprehensive testing)
- **Phase 6:** 4-6 hours (CLLM tool testing)
- **Phase 7:** 2-3 hours (Documentation)
- **Phase 8:** 2-3 hours (Final verification)

**Total:** 27-39 hours

---

## Current Status

**Phase 1:** ⏳ Not Started  
**Phase 2:** ⏳ Not Started  
**Phase 3:** ⏳ Not Started  
**Phase 4:** ⏳ Not Started  
**Phase 5:** ⏳ Not Started  
**Phase 6:** ⏳ Not Started  
**Phase 7:** ⏳ Not Started  
**Phase 8:** ⏳ Not Started  

**Overall Progress:** 0%

---

**Next Action:** Begin Phase 1.1 - Create Generic Sphere Threading API