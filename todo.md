# RULES (READ FIRST - ALWAYS AT TOP)

## ⭐ RULE 0: ALWAYS READ THESE RULES FIRST ⭐
**MANDATORY WITH EVERY RESPONSE**

Before ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Read AUDIT.md for current architectural state
3. Read SECONDARY_OBJECTIVES.md for detailed tasks
4. Update todo.md with current progress

## RULE 1: CRITICAL REFERENCE FILES
**Before making ANY code changes, ALWAYS check these files:**

1. **app/ui/sphere_visualization.c** - 3D kissing spheres visualization
   - PURE crystalline mathematics ONLY (NO math.h)
   - Uses ONLY prime_* functions from crystalline library
   - Implements icosahedral geometry with golden ratio
   - 13 spheres: 1 control + 12 workers

2. **src/geometry/clock_lattice.c** - Babylonian clock structure
   - Ring structure: 12, 60, 60, 100
   - Prime position mapping

3. **src/ai/cllm_kissing_spheres.c** - Kissing spheres implementation
   - 12 neighbors per point
   - Shared memory structure

**CRITICAL**: Never use math.h or standard library functions. Only use prime_* functions.

## RULE 2: NO EXTERNAL MATH LIBRARIES
- NO math.h
- NO complex.h
- NO standard library math functions
- ONLY use NEW math library (math/) with Crystalline Abacus
- If missing operations, add to math library as O(1) functions

## RULE 3: NO DUPLICATE CONSTANTS
- Check for multiple definitions of constants (infinity, pi, phi, etc.)
- Consolidate all constants in ONE location
- Remove duplicates across codebase

## RULE 4: PROPER NAMING CONVENTIONS
- NO "bigfixed" in names
- NO "complete" in names
- NO "new" in names
- NO "old" in names
- Use clear, descriptive names that reflect purpose
- Follow consistent naming across files and functions

---

# COMPREHENSIVE REASSESSMENT - PHASE 1

## 1. Read All Master Plan and Related Documents
- [x] Read MASTER_PLAN.md completely
- [x] Read AUDIT.md
- [x] Read SECONDARY_OBJECTIVES.md
- [x] Read TERTIARY_OBJECTIVES.md
- [x] Created COMPREHENSIVE_REASSESSMENT_ANALYSIS.md with all findings

## 2. PRIORITY 1: Fix Duplicate Constants ✅ COMPLETE
- [x] Remove duplicate MATH_TWO_PI from math/include/math/types.h (line 213)
- [x] Remove M_PI alias from include/prime_types.h (line 430)
- [x] Replace all M_PI references with MATH_PI
- [x] Verify math library compiles successfully
- [x] Commit and push changes

## 3. PRIORITY 2: Fix Bad Naming Conventions ✅ COMPLETE
- [x] Rename cross_entropy_loss_bigfixed → cross_entropy_loss
- [x] Rename softmax_bigfixed → softmax
- [x] Update algorithms/include/loss_functions.h
- [x] Update algorithms/src/loss_functions.c
- [x] Remove "bigfixed" from file comments
- [x] Fix duplicate function declarations (math_sign, math_is_nan, math_is_inf, math_is_finite)
- [x] Consolidate implementations in validation.h as static inline
- [x] Verify math library compiles
- [x] Verify algorithms library compiles
- [x] Ready to commit

## 4. PRIORITY 3: Audit Complex Operations ✅ COMPLETE
- [x] Review math/src/core/complex.c completely
- [x] Verify all operations are O(1) - CONFIRMED
- [x] Check for missing operations - ALL PRESENT (exp, log, pow, sin, cos)
- [x] No missing functions - implementation is COMPLETE
- [x] Ensure no math.h dependencies - CONFIRMED (uses math/transcendental.h)
- [x] Document findings - PRIORITY_3_COMPLEX_AUDIT_COMPLETE.md created

## 5. PRIORITY 4: Deep CLLM Analysis
- [ ] Analyze src/ai/cllm_token.c in detail
- [ ] Analyze src/ai/cllm_embeddings.c in detail
- [ ] Analyze src/ai/bigfixed_array_utils.c in detail
- [ ] Analyze src/ai/cllm_lattice_conversion.c in detail
- [ ] Analyze src/ai/cllm_optimizer.c in detail
- [ ] Analyze src/ai/cllm_training_threaded.c in detail
- [ ] Analyze src/ai/cllm_production.c in detail
- [ ] Map dependencies between files
- [ ] Identify Abacus integration points
- [ ] Document current BigFixed usage
- [ ] Create detailed migration specification

## 6. Create Detailed Action Plan
- [ ] Document migration order
- [ ] Estimate time for each file
- [ ] Identify potential challenges
- [ ] Create step-by-step guide
- [ ] Set milestones