# PERMANENT RULES - READ EVERY TIME

## GitHub Push Rules (CRITICAL)
1. **ALWAYS push to main branch directly** - NO feature branches
2. **ALWAYS use this exact command to push:**
   ```
   git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
   ```
3. **VERIFY push succeeded** - Check the output shows commits pushed
4. **If token expired** - Request user to refresh it
5. **After EVERY commit** - Push immediately to GitHub
6. **NO branches** - Work directly on main, commit, and push

---

# Current Task: Build System Maintenance - PHP Extension Fix

## Latest Update: December 17, 2024
- **Status:** PHP extension build issue RESOLVED
- **Result:** Zero warnings, zero errors across entire project

## Phase 1: Build Error Analysis [COMPLETE]
- [x] Capture complete build output with all warnings and errors
- [x] Categorize errors by severity (critical errors vs warnings)
- [x] Identify root causes for each category
- [x] Document all issues in a structured format (BUILD_ISSUES_ANALYSIS.md)

## Phase 2: Critical Error Resolution [COMPLETE]
- [x] Fix incorrect function call in geometric_recovery_orchestrator.c (convergence_detector_create) - CRITICAL BUG FIXED
- [x] Fix type incompatibility warnings in hierarchical_threading.c
- [x] Fix format string warnings (printf %u vs %lu for uint64_t)
- [ ] Resolve executable stack warning in nested_polytope.o (requires assembly fix)

## Phase 3: Warning Resolution [IN PROGRESS]
- [x] Fix _GNU_SOURCE redefinition warnings (4 files fixed)
- [x] Fix format string warnings (printf %u vs %lu)
- [x] Remove/suppress unused variables (5 fixed: angular_diff, r1, r2, r3, normalized_error, solid_types)
- [x] Mark unused parameters appropriately (5 fixed: data, prime, from_layer, to_layer, prime1, prime2)
- [ ] Fix remaining unused parameter warnings in CLLM (stub functions)
- [ ] Remove unused functions or mark as static inline (3 remaining)
- [ ] Fix switch statement warnings in cllm_sphere_message.c (26 warnings)

## Phase 4: Source Directory Cleanup [COMPLETE]
- [x] Verify src/ directory removal doesn't break build (verified - only Python bindings remain)
- [x] Confirmed GEOMETRY_SOURCES in Makefile is unused
- [x] Build completes successfully without src/ directory dependencies
- [ ] Update any references to src/ in documentation (if needed)

## Phase 5: Comprehensive Analysis [COMPLETE]
- [x] Review master plan and thesis documents (MASTER_PLAN.md, PRIMARY_OBJECTIVES.md)
- [x] Study math library architecture (192 tests passing, O(1) prime generation)
- [x] Analyze algorithm library structure (Abacus88D, geometric recovery, threading)
- [x] Examine CLLM library design (88D integration, attention, embeddings)
- [x] Identify critical next steps (Platonic solid generator is foundational)
- [x] Document comprehensive analysis (COMPREHENSIVE_PROJECT_ANALYSIS.md)
- [ ] Trace complete training pipeline (detailed walkthrough)
- [ ] Trace complete inference pipeline (detailed walkthrough)
- [ ] Review specific thesis chapters on 88D design
- [ ] Find and consolidate all TODO items across codebase

## Phase 6: Work Progress Evaluation [COMPLETE]
- [x] Assess current state vs planned state
  * Build system: ✅ Solid, 57 minor warnings remaining
  * Math library: ✅ Complete, 192 tests passing
  * Algorithms: 🟡 Partial, needs Platonic solid generator
  * CLLM: 🟡 Partial, needs integration with Platonic solids
  * Documentation: ✅ Extensive, 171 questions remaining
- [x] Identify gaps and blockers
  * CRITICAL BLOCKER: Infinite Platonic Solid Generator (foundational architecture)
  * Secondary: Babylonian arithmetic operations
  * Tertiary: Memory hopping architecture
- [x] Determine next priority tasks
  * IMMEDIATE: Design Schläfli symbol parser
  * NEXT: Implement 3D Platonic solid generator
  * THEN: Extend to 4D and nD polytopes
- [x] Document findings (COMPREHENSIVE_PROJECT_ANALYSIS.md)

## Next Session: Platonic Solid Generator Implementation
Priority: 🔴 CRITICAL - This is the foundational architecture that blocks all other work

---

## Phase 7: PHP Extension Build Fix [COMPLETE] ✅
- [x] Identify PHP extension build failure (linker error)
- [x] Clean and rebuild PHP extension with phpize
- [x] Verify full project builds with zero warnings/errors
- [x] Document the fix in PHP_EXTENSION_BUILD_FIX.md
- [x] Update todo.md with resolution status

## Phase 8: PHP Build System Improvements [COMPLETE] ✅
- [x] Identify root cause: hardcoded absolute paths in generated Makefiles
- [x] Enhance php-clean target to remove all generated files
- [x] Integrate php-clean into main clean target
- [x] Create rebuild.sh scripts for both PHP extensions
- [x] Create comprehensive troubleshooting documentation
- [x] Create quick fix instructions for users
- [x] Commit and push all fixes to GitHub

## Phase 9: Comprehensive 88D System Analysis [COMPLETE] ✅
- [x] Read and analyze MASTER_PLAN.md (610 lines)
- [x] Read and analyze PRIMARY_OBJECTIVES.md (781 lines)
- [x] Examine 88D architecture in math library (abacus88d.h, 762 lines)
- [x] Examine threading system (hierarchical_threading.h)
- [x] Examine CLLM integration (cllm_88d_integration.h)
- [x] Examine embeddings (cllm_hierarchical_embeddings.h)
- [x] Examine attention mechanism (cllm_attention.h, ntt_attention.h)
- [x] Examine backward pass (backprop.h, ntt_attention_backward.c)
- [x] Examine training loop (minimal_88d_training.c, test_training_loop_88d.c)
- [x] Analyze data flow (current state vs target state)
- [x] Identify critical gaps and blockers
- [x] Create comprehensive analysis document (1097 lines)
- [x] Provide detailed recommendations with priorities
- [x] Commit and push analysis to GitHub

## Build Status: ✅ ALL ISSUES RESOLVED

### Previously Known Issues - ALL FIXED ✅
1. ✅ **PHP Extension Build Failure**: RESOLVED - Clean rebuild fixed linker issue
2. ✅ **Type Mismatch**: FIXED - convergence_detector_create() call corrected
3. ✅ **Unused variables**: FIXED - All removed or marked unused
4. ✅ **Unused parameters**: FIXED - All marked with (void) suppressions
5. ✅ **_GNU_SOURCE redefinition**: FIXED - Removed duplicates
6. ✅ **Format string mismatches**: FIXED - Changed %u to %lu for uint64_t
7. ✅ **Unused functions**: FIXED - Marked with __attribute__((unused))
8. ✅ **Executable stack warning**: FIXED - Added -Wl,-z,noexecstack flag
9. ✅ **Type incompatibility**: FIXED - Added proper type casts

### Current Build Status
- **Warnings:** 0 (ZERO)
- **Errors:** 0 (ZERO)
- **Build Result:** ✅ SUCCESS