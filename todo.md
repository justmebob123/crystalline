# TODO: CRYSTALLINE CLLM - Complete Implementation Plan

## ✅ ARCHITECTURE UNDERSTANDING COMPLETE

### The Profound Design (Now Understood):

**The Model IS a Fractal:**
- Entire model file is a fractal structure interpreted through the crystalline lattice abacus
- Primes define the fundamental geometry, self-similar at every scale
- Clock face geometry: Outer ring (∞/0) = smallest primes, inner rings = larger primes, center (1) = unity
- 12-fold symmetry from clock structure, kissing spheres recursive at every level

**Model File Structure:**
- Metadata: vocab_size, embedding_dim, num_layers, etc.
- Trained weights and embeddings
- **NOT the primes** (except maybe first 30 for bootstrap in future)
- The fractal pattern is implicit in the structure

**The Abacus Role:**
- Pre-generated prime table (10,030 primes currently)
- On-demand prime generation when needed
- Geometric decoder - understands where primes exist in fractal
- Shared resource across all threads

**Token Architecture:**
- Root words → Prime positions in lattice
- Composite words → Coprime or composite positions  
- Geometry = meaning (position in fractal = semantic relationship)

**Model Loading Process:**
1. Read metadata to understand model size/structure
2. Abacus uses metadata to determine required prime count
3. Generate additional primes on-demand if needed
4. Abacus maps the fractal geometry
5. Model accessible for inference/training (from disk!)

---

## 🎯 CRITICAL PRIORITY OBJECTIVES

### OBJECTIVE 26: Fix Model Manager Architecture [CRITICAL - DO FIRST]
**Purpose:** Remove "loading into memory" concept, models are accessible from disk

**Tasks:**
- [ ] Add `model_manager_read_metadata()` - read just header without loading weights
- [ ] Add `model_manager_estimate_primes()` - calculate required primes from metadata
- [ ] Add `model_manager_check_abacus()` - verify abacus has enough primes
- [ ] Add `model_manager_prepare()` - expand abacus if needed
- [ ] Remove `is_loaded` flag - replace with `is_accessible`
- [ ] Update UI to show "Model Accessible" not "Model Loaded"
- [ ] Show required vs available prime count
- [ ] Add "Prepare Model" button in UI
- [ ] Remove memory usage concerns from UI
- [ ] Test inference from disk
- [ ] Test training from disk

**Expected Impact:**
- Models work from disk without OOM
- Abacus expands on-demand
- User sees clear status of model readiness

---

### OBJECTIVE 25: Fix Remaining Build Warnings [CRITICAL]
**Purpose:** Achieve zero warnings build (currently 78 warnings)

**Status:** ⚠️ IN PROGRESS - 78 warnings from BigFixed migration

**Tasks:**
- [ ] Categorize all 78 warnings by type
- [ ] Fix type mismatches (BigFixed** vs float*)
- [ ] Fix unused parameters
- [ ] Fix implicit declarations
- [ ] Document any warnings that cannot be fixed
- [ ] Achieve zero warnings build

---

### OBJECTIVE 21: Fix Backwards "Simple Loss" Naming [HIGH PRIORITY]
**Purpose:** Fix backwards naming where "simple_loss" is actually THE REAL implementation

**Tasks:**
- [ ] Rename `include/ai/cllm_simple_loss.h` → `include/ai/cllm_loss.h`
- [ ] Rename infrastructure `include/ai/cllm_loss.h` → `include/ai/cllm_tensor_loss.h`
- [ ] Update all includes
- [ ] Test build

---

### OBJECTIVE 22: Delete Unused Infrastructure Files [HIGH PRIORITY]
**Purpose:** Remove 83KB of dead code

**Tasks:**
- [ ] Delete `src/ai/infrastructure/cllm_backprop.c` (22KB)
- [ ] Delete `src/ai/infrastructure/cllm_loss.c` (30KB)
- [ ] Delete `src/ai/infrastructure/cllm_training_loop.c` (31KB)
- [ ] Delete corresponding headers
- [ ] Update Makefile
- [ ] Test build

---

## 🔧 HIGH PRIORITY OBJECTIVES

### OBJECTIVE 2D: Remove ALL "Standard" and "Legacy" Code
**Purpose:** Clean codebase of all non-crystalline implementations

**Files to Delete:**
- [ ] `src/ai/cllm_training_mt.c` - Old multi-threading
- [ ] `src/ai/cllm_training_parallel.c` - Unused parallel code
- [ ] `src/ai/cllm_train_complete.c` - Legacy training wrapper
- [ ] Corresponding headers
- [ ] Update Makefile

---

### OBJECTIVE 23: Remove Misleading File Name Qualifiers
**Purpose:** Remove qualifiers that imply alternatives

**Files to Rename:**
- [ ] `cllm_crystalline_advanced.c` → `cllm_advanced.c`
- [ ] `cllm_crystalline_attention.c` → `cllm_attention.c`
- [ ] `cllm_crystalline_sieve.c` → `cllm_sieve.c`
- [ ] `cllm_pure_embeddings.c` → `cllm_embeddings.c`
- [ ] `cllm_pure_token.c` → `cllm_token.c`

---

## 📊 MEDIUM PRIORITY OBJECTIVES

### OBJECTIVE 24: Investigate and Consolidate Duplicates
**Purpose:** Identify and merge duplicate functionality

**Potential Duplicates:**
- [ ] Compare batch processing files
- [ ] Compare optimizer files
- [ ] Compare embedding files (5+ files)
- [ ] Compare attention files (3+ files)
- [ ] Merge or document relationships

---

### OBJECTIVE 5A: Kissing Spheres as ONLY Threading
**Purpose:** Remove all non-kissing-spheres threading code

**Tasks:**
- [ ] Remove ALL fallbacks to old threading
- [ ] Make kissing spheres mandatory
- [ ] Remove `cllm_train_epoch_mt()` completely
- [ ] Update tools to require kissing spheres

---

### OBJECTIVE 8A: Remove ALL Conditional Compilation
**Purpose:** One codebase, one design, no toggles

**Tasks:**
- [ ] Remove all feature flags from config structs
- [ ] Remove all #ifdef blocks for features
- [ ] One implementation per function
- [ ] No "enable_X" configuration options

---

### OBJECTIVE 17: Implement NTT-Based O(n log n) Attention
**Purpose:** Replace O(n²) attention with O(n log n)

**Tasks:**
- [ ] Create `src/ai/cllm_ntt_attention.c`
- [ ] Implement `cllm_attention_ntt_forward()`
- [ ] Use NTT library from `bigint_ntt.h`
- [ ] Integrate into attention forward pass
- [ ] Benchmark performance

**Expected Impact:** 10-100x speedup for long sequences

---

### OBJECTIVE 18: Apply Cymatic Frequency Resonance
**Purpose:** Use cymatic frequencies to modulate training

**Tasks:**
- [ ] Create `src/ai/cllm_cymatic_training.c`
- [ ] Implement `cllm_apply_cymatic_resonance()`
- [ ] Use CYMATIC_*_HZ constants
- [ ] Integrate into training step
- [ ] Test convergence smoothness

**Expected Impact:** 20-40% smoother convergence

---

## ✅ COMPLETED OBJECTIVES

### OBJECTIVE 1: Library Distribution Architecture - COMPLETE
- ✅ All libraries (.so and .a) building correctly

### OBJECTIVE 2B: Remove Legacy Loss Functions - COMPLETE
- ✅ Removed all standard cross-entropy functions
- ✅ Crystalline loss is now the ONLY loss function

### OBJECTIVE 2C: Rename "Crystalline" to Default - COMPLETE
- ✅ Renamed `cllm_compute_crystalline_loss()` to `cllm_compute_loss()`

### OBJECTIVE 3A: BigFixed Migration - COMPLETE
- ✅ 100% BigFixed coverage in training and inference

### OBJECTIVE 5: Crystalline Math Integration - COMPLETE
- ✅ NO math.h usage in production code

### OBJECTIVE 7: 12-Fold Symmetry - COMPLETE
- ✅ Implemented in threading and embeddings

### OBJECTIVE 8: Node Zero Control Thread - COMPLETE
- ✅ Control thread coordinates 12 workers

### OBJECTIVE 14: L(n,d,k,λ) Lattice Formula - COMPLETE
- ✅ Formula implemented and integrated

### OBJECTIVE 15: Angular Position Attention - COMPLETE
- ✅ θ(n,k,λ,ω,ψ) formula integrated

### OBJECTIVE 16: Kissing Sphere Neighbors - COMPLETE
- ✅ Each point has exactly 12 neighbors

### OBJECTIVE 19: Babylonian Clock Lattice - COMPLETE
- ✅ Clock mapping and stereographic projection implemented

---

## 📋 PRIORITY ORDER

**DO IMMEDIATELY:**
1. **OBJECTIVE 26** - Fix model manager architecture (disk-based access)
2. **OBJECTIVE 25** - Fix 78 build warnings
3. **OBJECTIVE 21** - Fix "simple_loss" naming
4. **OBJECTIVE 22** - Delete unused infrastructure (83KB)

**DO NEXT:**
5. **OBJECTIVE 2D** - Remove legacy code files
6. **OBJECTIVE 23** - Remove misleading qualifiers

**THEN:**
7. **OBJECTIVE 24** - Consolidate duplicates
8. **OBJECTIVE 5A** - Kissing spheres only threading
9. **OBJECTIVE 8A** - Remove conditional compilation
10. **OBJECTIVE 17** - NTT attention
11. **OBJECTIVE 18** - Cymatic resonance

---

## 🎯 CURRENT FOCUS

Starting with **OBJECTIVE 26: Fix Model Manager Architecture** to properly implement disk-based model access with abacus-driven prime generation.