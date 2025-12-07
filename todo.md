# Reorganization Plan - Correct Library Placement

## User Modifications to Plan
- ✅ Q→k mapping moves to algorithms layer (general algorithm, not AI-specific)
- ✅ Keep TWO_PI constant (fundamental for Möbius calculations)

## Phase 1: Delete Incorrectly Placed Files
- [x] Delete include/ai/cllm_entropy_functions.h
- [x] Delete src/ai/cllm_entropy_functions.c
- [x] Delete include/ai/cllm_cymatics_simulation.h
- [x] Delete src/ai/cllm_cymatics_simulation.c

## Phase 2: Extend algorithms/statistics
- [x] Add 4 entropy functions to algorithms/include/statistics.h
- [x] Implement in algorithms/src/statistics.c
- [x] Build and verify

## Phase 3: Extend algorithms/cymatic_modulation
- [x] Add 5 cymatics functions to algorithms/include/cymatic_modulation.h
- [x] Implement in algorithms/src/cymatic_modulation.c
- [x] Build and verify

## Phase 4: Create algorithms/tensor_ops (NEW)
- [x] Create algorithms/include/tensor_ops.h
- [x] Create algorithms/src/tensor_ops.c
- [x] Add to algorithms Makefile
- [x] Build and verify

## Phase 5: Move Q→k Mapping to Algorithms Layer
- [x] Move include/ai/cllm_qk_mapping.h to algorithms/include/qk_mapping.h
- [x] Move src/ai/cllm_qk_mapping.c to algorithms/src/qk_mapping.c
- [x] Update to use tensor_ops functions
- [x] Rename functions: cllm_qk_* → qk_*
- [x] Update all includes
- [x] Build and verify

## Phase 6: Update Dependencies
- [x] Update any files that include old headers
- [x] Update Makefiles
- [x] Build entire project
- [x] Verify 0 errors, 0 warnings

## Phase 7: Git Operations
- [ ] Commit all changes
- [ ] Push to GitHub
- [ ] Update documentation