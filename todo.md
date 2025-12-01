# TODO - Crystalline CLLM Integration

## 🔒 RULES (PASTED FROM MASTER_PLAN)

### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
**HIGHEST PRIORITY RULE - MUST BE FOLLOWED WITH EVERY SINGLE RESPONSE**

At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

This creates a permanent loop ensuring rules are never forgotten.

### RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
**SECOND HIGHEST PRIORITY RULE**

Before taking ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Understand the current objectives
3. Verify your action aligns with the master plan
4. Check for any blocking priorities

This ensures all work follows the architectural design.

### RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
**CRITICAL REFERENCE DOCUMENT**

The AUDIT.md contains:
- Current architectural violations
- Required fixes with priorities
- Implementation phases
- Testing requirements
- Success criteria

Always consult AUDIT.md before starting work to understand:
- What is broken
- What needs fixing
- What is blocking other work
- What the correct architecture should be

### RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
**DETAILED IMPLEMENTATION GUIDE**

The SECONDARY_OBJECTIVES.md contains:
- Detailed implementation tasks
- Code examples
- File-by-file changes
- Testing procedures
- Validation steps

Use this for step-by-step implementation guidance.

---

## 📋 CURRENT STATUS

**Build Status:** ✅ Zero errors, 78 warnings (pre-existing from BigFixed migration)
**Last Major Completion:** OBJECTIVES 14, 15, 16 (Lattice Formula, Angular Attention, Kissing Spheres)

**Core Mathematical Framework:** ✅ COMPLETE
- ✅ L(n,d,k,λ) lattice embeddings integrated
- ✅ θ(n,k,λ,ω,ψ) angular position attention integrated
- ✅ 12 kissing sphere neighbors initialized

---

## 🎯 NEXT PRIORITY OBJECTIVES (From MASTER_PLAN)

Based on the MASTER_PLAN, the next logical objectives to tackle are the **cleanup and optimization** objectives that will solidify the crystalline architecture:

### PHASE 1: CLEANUP LEGACY CODE (OBJECTIVES 2B, 2C, 2D)

#### OBJECTIVE 2B: Remove ALL Legacy Loss Functions ✅ COMPLETE
**Purpose:** Make crystalline loss the ONLY loss function (no fallbacks)

- [x] Analyze current loss function implementations
- [x] Identify all standard cross-entropy code
- [x] Verify no `use_crystalline_optimizations` flag exists
- [x] Verify main training uses crystalline loss only
- [x] Remove `cllm_compute_cross_entropy_loss()` from `src/ai/cllm_loss.c`
- [x] Remove `cllm_compute_batch_loss()` from `src/ai/cllm_loss.c`
- [x] Remove `cllm_compute_label_smoothing_loss()` from `src/ai/cllm_loss.c`
- [x] Remove `cllm_compute_kl_divergence()` from `src/ai/cllm_loss.c`
- [x] Remove `cllm_compute_sequence_loss()` from `src/ai/cllm_loss.c`
- [x] Update `include/ai/cllm_simple_loss.h` to remove legacy declarations
- [x] Update `include/ai/cllm_loss.h` to remove legacy declarations
- [x] Verify infrastructure layer cross-entropy is NOT used
- [x] Keep utility functions (perplexity, accuracy, top-k accuracy)
- [x] Remove "LEGACY" comments from headers
- [x] Update documentation to reflect crystalline as only loss
- [x] Test build (✅ Zero errors, 78 warnings - no new warnings)
- [x] Commit changes

#### OBJECTIVE 2C: Rename "Crystalline" to Default ✅ COMPLETE
**Purpose:** Stop treating crystalline as special - it's the only design

**What Was Done:**
- [x] Renamed `cllm_compute_crystalline_loss()` to `cllm_compute_loss()`
- [x] Renamed `cllm_compute_crystalline_loss_detailed()` to `cllm_compute_loss_detailed()`
- [x] Updated function declaration in `include/ai/cllm_simple_loss.h`
- [x] Updated function definition in `src/ai/cllm_training.c`
- [x] Updated all 6 call sites:
  - `src/ai/cllm_training.c` (2 places)
  - `src/ai/cllm_production.c`
  - `src/ai/cllm_training_threaded.c`
  - `src/ai/cllm_loss.c` (comment)
- [x] Updated comments and documentation
- [x] Kept internal helper names (crystalline_gcd_similarity, etc.) - they're descriptive
- [x] Test build (✅ Zero errors, 78 warnings - no new warnings)
- [x] Commit changes

**Result:**
- The loss function is now simply `cllm_compute_loss()` - no special prefix
- Crystalline is not treated as special - it IS the design

#### OBJECTIVE 2D: Remove ALL "Standard" and "Legacy" Code
**Purpose:** Clean codebase of all non-crystalline implementations

- [ ] Search for "standard", "legacy", "old", "fallback" in codebase
- [ ] Identify files to delete:
  - [ ] `src/ai/cllm_training_mt.c`
  - [ ] `src/ai/cllm_training_parallel.c`
  - [ ] `src/ai/cllm_train_complete.c`
  - [ ] `include/cllm_training_mt.h`
  - [ ] `include/cllm_training_parallel.h`
  - [ ] `include/cllm_train_complete.h`
- [ ] Identify functions to delete
- [ ] Update Makefile
- [ ] Delete legacy files
- [ ] Verify build
- [ ] Commit changes

### PHASE 2: ADVANCED OPTIMIZATIONS (OBJECTIVES 17, 18)

#### OBJECTIVE 17: NTT-Based O(n log n) Attention
**Purpose:** Replace O(n²) attention with O(n log n) using Number Theoretic Transform

- [ ] Analyze `include/bigint_ntt.h` implementation
- [ ] Create `src/ai/cllm_ntt_attention.c`
- [ ] Implement `cllm_attention_ntt_forward()`
- [ ] Use NTT for sequences > 256 tokens
- [ ] Test correctness (outputs match standard)
- [ ] Benchmark performance
- [ ] Create `tools/benchmark_ntt_attention`
- [ ] Integrate into main attention
- [ ] Commit changes

#### OBJECTIVE 18: Cymatic Frequency Resonance
**Purpose:** Use cymatic frequencies to modulate training

- [ ] Analyze cymatic constants in `cllm_mathematical_constants.h`
- [ ] Create `src/ai/cllm_cymatic_training.c`
- [ ] Implement `cllm_apply_cymatic_resonance()`
- [ ] Use CYMATIC_*_HZ constants
- [ ] Integrate into training step
- [ ] Test convergence smoothness
- [ ] Measure impact on final loss
- [ ] Commit changes

### PHASE 3: ARCHITECTURE CLEANUP (OBJECTIVES 5A, 8A)

#### OBJECTIVE 5A: Kissing Spheres as ONLY Threading
**Purpose:** Remove all non-kissing-spheres threading code

- [ ] Remove fallbacks to old threading
- [ ] Make kissing spheres mandatory
- [ ] Remove `cllm_train_epoch_mt()` completely
- [ ] Update tools to require kissing spheres
- [ ] Remove single-threaded training paths
- [ ] Verify build
- [ ] Commit changes

#### OBJECTIVE 8A: Remove ALL Conditional Compilation
**Purpose:** One codebase, one design, no toggles

- [ ] Remove all feature flags from config structs
- [ ] Remove all #ifdef blocks for features
- [ ] Remove "enable_X" configuration options
- [ ] Ensure single code path for each operation
- [ ] Verify build
- [ ] Commit changes

---

## 🚀 EXECUTION PLAN

**Starting with:** OBJECTIVE 2B (Remove Legacy Loss Functions)

### Current State Analysis:
✅ **Good News:**
- No `use_crystalline_optimizations` flags found in codebase
- Main training code uses `cllm_compute_crystalline_loss()` exclusively
- No calls to standard cross-entropy in training pipeline

⚠️ **Legacy Code Still Present:**
- `cllm_compute_cross_entropy_loss()` still exists in `src/ai/cllm_loss.c`
- Marked as "LEGACY" in `include/ai/cllm_simple_loss.h`
- Infrastructure layer has `loss_cross_entropy_forward()` in `src/ai/infrastructure/cllm_loss.c`
- These are NOT used in training but should be removed

**Next Steps:**
1. Read MASTER_PLAN.md completely ✅
2. Read AUDIT.md for architectural state ✅
3. Read SECONDARY_OBJECTIVES.md for detailed tasks
4. Begin OBJECTIVE 2B implementation <-- STARTING NOW

---

## 📝 NOTES

- All three major mathematical objectives (14, 15, 16) are COMPLETE
- Build is clean (zero errors, pre-existing warnings only)
- Focus now shifts to cleanup and optimization
- Each objective should be completed fully before moving to next
- Always commit changes after completing each objective