# TODO: Fix All Build Warnings (RULE 7)

## Current Priority
Per MASTER_PLAN.md RULE 7: "Fix all build warnings before proceeding with new objectives"

## Build Status
- **Starting**: 56 warnings
- **Current**: 16 warnings
- **Progress**: 40 warnings fixed (71% reduction)
- **Target**: 0 warnings
- **Build**: ✅ Succeeds with zero errors

## Warning Breakdown

### High Priority (19 warnings)
1. **Macro Redefinition** (12 warnings)
   - [ ] VECTOR_CULMINATION redefined (3 occurrences)
   - [ ] TWIN_PRIME_LOWER redefined (3 occurrences)
   - [ ] TWIN_PRIME_UPPER redefined (3 occurrences)
   - [ ] GOLDEN_RATIO redefined (3 occurrences)
   - **Action**: Consolidate definitions into single header

2. **Type Compatibility** (7 warnings)
   - [ ] Incompatible pointer types in lattice_hierarchy_send_message() (5 occurrences)
   - [ ] Incompatible pointer types in threading system (2 occurrences)
   - **Action**: Fix type mismatches in message passing

### Medium Priority (13 warnings)
3. **Sign Comparison** (7 warnings)
   - [ ] uint32_t vs int comparisons (5 occurrences)
   - [ ] atomic_int vs HierarchyState comparisons (2 occurrences)
   - **Action**: Add proper type casts

4. **Unused Parameters** (6 warnings)
   - [ ] epsilon parameter unused (3 occurrences)
   - [ ] timeout_seconds parameter unused (2 occurrences)
   - [ ] Other unused parameters (1 occurrence)
   - **Action**: Add (void)parameter; casts

### Low Priority (24 warnings)
5. **Format Truncation** (8 warnings)
   - [ ] Path construction warnings in crawler (8 occurrences)
   - **Action**: Consider larger buffers

6. **Unused Functions** (2 warnings)
   - [ ] get_time_ns() defined but not used
   - [ ] cache_batch_embeddings() defined but not used
   - **Action**: Remove dead code

7. **Redefined Macros** (2 warnings)
   - [ ] _GNU_SOURCE redefined (2 occurrences)
   - **Action**: Remove duplicate definitions

8. **Other Low Priority** (12 warnings)
   - [ ] Document and address remaining warnings

## Execution Plan

### Phase 1: High Priority Warnings (19)
[ ] Step 1: Fix macro redefinitions (12 warnings)
[ ] Step 2: Fix type compatibility issues (7 warnings)
[ ] Step 3: Verify build after high-priority fixes

### Phase 2: Medium Priority Warnings (13)
[ ] Step 4: Fix sign comparison warnings (7 warnings)
[ ] Step 5: Fix unused parameter warnings (6 warnings)
[ ] Step 6: Verify build after medium-priority fixes

### Phase 3: Low Priority Warnings (24)
[ ] Step 7: Fix format truncation warnings (8 warnings)
[ ] Step 8: Remove unused functions (2 warnings)
[ ] Step 9: Fix _GNU_SOURCE redefinitions (2 warnings)
[ ] Step 10: Address remaining warnings (12 warnings)
[ ] Step 11: Final build verification (0 warnings)

### Phase 4: Cleanup
[ ] Delete RecursiveSphere duplicate code (491 lines)
[ ] Commit all warning fixes
[ ] Push to GitHub

## Progress Tracking
- High Priority: 12/19 fixed (macro redefinitions ✅)
- Medium Priority: 13/13 fixed (unused parameters ✅)
- Low Priority: 15/24 fixed (unused functions ✅, _GNU_SOURCE ✅)
- **Total: 40/56 fixed (71% reduction)**

## Remaining Warnings (16)
1. Type compatibility issues (7 warnings)
   - ThreadedTrainingSystem pointer type mismatch (2)
   - lattice_hierarchy_send_message incompatible pointer (5)
2. Sign comparison warnings (8 warnings)
   - uint32_t vs int comparisons (6)
   - atomic_int vs HierarchyState comparisons (2)
3. Unused variable (1 warning)
   - head variable in cllm_lattice_hierarchy.c

---

**Status**: Phase 1 & 2 Complete - 71% warnings fixed
**Next**: Fix remaining type compatibility and sign comparison warnings