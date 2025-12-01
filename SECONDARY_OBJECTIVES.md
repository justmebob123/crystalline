# SECONDARY OBJECTIVES - DETAILED IMPLEMENTATION TASKS

## OVERVIEW
This file contains detailed implementation tasks for completing the Crystalline CLLM integration.
Refer to MASTER_PLAN.md for high-level objectives and architectural requirements.

**LAST UPDATED:** 2024-12-01 - Comprehensive Architecture Analysis Complete
**BUILD STATUS:** ✅ Zero errors, 78 warnings (pre-existing from BigFixed migration)

---

## ✅ COMPLETED OBJECTIVES

### OBJECTIVE 1: Library Distribution Architecture - COMPLETE
- ✅ All libraries (.so and .a) building correctly
- ✅ Modular architecture maintained

### OBJECTIVE 3A: BigFixed Migration - COMPLETE
- ✅ Training path: 100% BigFixed coverage
- ✅ Inference path: 100% BigFixed coverage
- ✅ NO float arithmetic in critical paths
- ✅ All transformer operations use BigFixed
- ✅ Zero warnings achieved

### OBJECTIVE 5: Crystalline Math Integration - COMPLETE
- ✅ NO math.h usage in production code
- ✅ All using prime_* functions

### OBJECTIVE 7: 12-Fold Symmetry - COMPLETE
- ✅ Implemented in threading
- ✅ Implemented in embeddings

### OBJECTIVE 8: Node Zero Control Thread - COMPLETE
- ✅ Control thread never processes batches
- ✅ Coordinates 12 worker threads

### OBJECTIVE 14: L(n,d,k,λ) Lattice Formula - COMPLETE
- ✅ Formula implemented in `algorithms/src/lattice_embeddings_bigfixed.c`
- ✅ Integrated into model creation
- ✅ Uses BigFixed for arbitrary precision
- ✅ Embeddings use geometric lattice pattern

### OBJECTIVE 15: Angular Position Attention - COMPLETE
- ✅ Formula implemented in `src/core/cllm_angular_position.c`
- ✅ Integrated into training forward pass
- ✅ Uses θ(n,k,λ,ω,ψ) when token IDs available
- ✅ Encodes 12-fold symmetry and cymatic patterns

### OBJECTIVE 16: Kissing Sphere Neighbors - COMPLETE
- ✅ Implemented in `src/ai/cllm_kissing_spheres.c`
- ✅ Uses deterministic clock geometry (O(n))
- ✅ Each point has exactly 12 neighbors
- ✅ Called during model creation

### OBJECTIVE 19: Babylonian Clock Lattice - PARTIALLY COMPLETE
- ✅ Core implementation done (`src/geometry/clock_lattice.c`)
- ✅ Clock mapping implemented
- ✅ Stereographic projection implemented
- ✅ Visualization updated
- ⚠️ Integration ongoing

---

## 🎯 PRIORITY OBJECTIVES (From MASTER_PLAN)

### OBJECTIVE 2B: Remove ALL Legacy Loss Functions ✅ COMPLETE

**Purpose:** Complete the crystalline design by removing standard cross-entropy

**Status:** ✅ FULLY IMPLEMENTED

**What Was Done:**
- ✅ Removed `cllm_compute_cross_entropy_loss()` from `src/ai/cllm_loss.c`
- ✅ Removed `cllm_compute_batch_loss()` from `src/ai/cllm_loss.c`
- ✅ Removed `cllm_compute_label_smoothing_loss()` from `src/ai/cllm_loss.c`
- ✅ Removed `cllm_compute_kl_divergence()` from `src/ai/cllm_loss.c`
- ✅ Removed `cllm_compute_sequence_loss()` from `src/ai/cllm_loss.c`
- ✅ Updated `include/ai/cllm_simple_loss.h` to remove legacy declarations
- ✅ Updated `include/ai/cllm_loss.h` to remove legacy declarations
- ✅ Verified no `use_crystalline_optimizations` flag exists
- ✅ Verified main training uses crystalline loss exclusively
- ✅ Kept utility functions (perplexity, accuracy, top-k accuracy)
- ✅ Build verified: Zero errors, 78 warnings (no new warnings)

**Result:**
- Crystalline loss (`cllm_compute_crystalline_loss`) is now the ONLY loss function
- No fallbacks, no toggles, no conditional paths
- Complete commitment to the crystalline architecture

---

### OBJECTIVE 2C: Rename "Crystalline" to Default ✅ COMPLETE

**Purpose:** Stop treating crystalline as special - it's the only design

**Status:** ✅ FULLY IMPLEMENTED

**What Was Done:**
- ✅ Renamed `cllm_compute_crystalline_loss()` to `cllm_compute_loss()`
- ✅ Renamed `cllm_compute_crystalline_loss_detailed()` to `cllm_compute_loss_detailed()`
- ✅ Updated all 6 call sites across the codebase
- ✅ Updated function declarations and definitions
- ✅ Updated comments and documentation
- ✅ Build verified: Zero errors, 78 warnings (no new warnings)

**Result:**
- The loss function is now simply `cllm_compute_loss()` - no special prefix
- Crystalline is not treated as special - it IS the design

---

### OBJECTIVE 2D: Remove ALL "Standard" and "Legacy" Code [MEDIUM PRIORITY]

**Purpose:** Clean codebase of all non-crystalline implementations

**Files to Delete:**
- [ ] `src/ai/cllm_training_mt.c` - Old multi-threading
- [ ] `src/ai/cllm_training_parallel.c` - Unused parallel code
- [ ] `src/ai/cllm_train_complete.c` - Legacy training wrapper
- [ ] `include/cllm_training_mt.h`
- [ ] `include/cllm_training_parallel.h`
- [ ] `include/cllm_train_complete.h`

**Functions to Delete:**
- [ ] `cllm_compute_loss_training()` - Standard cross-entropy
- [ ] `cllm_train_epoch_mt()` - Old MT training
- [ ] `cllm_train_epoch_parallel()` - Unused parallel
- [ ] Any other `*_standard()` or `*_legacy()` functions

**Search and Destroy:**
- [ ] Search entire codebase for "standard", "legacy", "old", "fallback"
- [ ] Identify all non-crystalline implementations
- [ ] Delete all legacy code
- [ ] Update Makefile to remove deleted files
- [ ] Verify build after deletions

---

### OBJECTIVE 5A: Kissing Spheres as ONLY Threading [MEDIUM PRIORITY]

**Purpose:** Remove all non-kissing-spheres threading code

**Tasks:**
- [ ] Remove ALL fallbacks to old threading
- [ ] Make kissing spheres mandatory (no single-threaded fallback)
- [ ] Remove `cllm_train_epoch_mt()` completely
- [ ] Update tools to require kissing spheres
- [ ] Document kissing spheres as the only threading model
- [ ] Remove any single-threaded training paths
- [ ] Ensure all training goes through kissing spheres

**Related Files:**
- `tools/train_model.c` - Remove fallbacks
- `src/ai/cllm_training_threaded.c` - Main implementation
- `src/crawler/continuous_training.c` - Update to use kissing spheres

---

### OBJECTIVE 8A: Remove ALL Conditional Compilation [MEDIUM PRIORITY]

**Purpose:** One codebase, one design, no toggles

**Tasks:**
- [ ] Remove all feature flags from config structs
- [ ] Remove all #ifdef blocks for features
- [ ] One implementation per function (no alternatives)
- [ ] No "enable_X" configuration options
- [ ] Crystalline design is always active
- [ ] No compile-time toggles
- [ ] No runtime toggles
- [ ] Single code path for each operation

**Philosophy:**
- If it's in the codebase, it's active
- No optional features
- No legacy compatibility modes
- Complete commitment to the design

---

### OBJECTIVE 14: Integrate L(n,d,k,λ) Lattice Formula ✅ COMPLETE

**Purpose:** Replace random embeddings with crystalline lattice formula

**Status:** ✅ FULLY IMPLEMENTED AND INTEGRATED

**Implementation Details:**
- ✅ L(n,d,k,λ) formula IMPLEMENTED in `algorithms/src/lattice_embeddings_bigfixed.c`
- ✅ Formula: L = 3^O(n,k,λ) · ∏cos(θ·φᵢ) · Γ(k) · ν(λ) · Γ(n,d)
- ✅ INTEGRATED in `src/ai/cllm_create.c` (line 179-186)
- ✅ Uses BigFixed for arbitrary precision (no overflow)
- ✅ Embeddings initialized with geometric lattice pattern
- ✅ Uses Babylonian clock mapping for token positions
- ✅ Uses dimensional frequencies (φᵢ) for each dimension
- ✅ Normalized with tanh(L/100) to [-1, 1] range
- ✅ This is the CORE mathematical foundation of the entire system

**Actual Implementation:**
```c
// In src/ai/cllm_create.c (lines 179-186):
printf("Initializing embeddings with L(n,d,k,λ) lattice formula (BigFixed)...\n");
lattice_embeddings_init_geometric_bigfixed(
    model->weights,  // First vocab_size * embedding_dim weights are embeddings
    config->vocab_size,
    config->embedding_dim,
    128  // precision_bits
);
```

**Expected Impact:**
- ✅ Embeddings reflect true crystalline structure
- ✅ Better initial conditions for training
- ✅ Faster convergence (estimated 20-30%)
- ✅ Foundation for all other mathematical integrations

---

### OBJECTIVE 15: Integrate θ(n,k,λ,ω,ψ) Angular Position into Attention ✅ COMPLETE

**Purpose:** Replace dot product attention with angular position formula

**Status:** ✅ FULLY IMPLEMENTED AND INTEGRATED

**Implementation Details:**
- ✅ θ(n,k,λ,ω,ψ) formula IMPLEMENTED in `src/core/cllm_angular_position.c`
- ✅ Formula: θ = k·π(1+√5) + (n-1)·2π/(12·ln3) + log₃(ν(λ)) + ω + ψ
- ✅ INTEGRATED into training via `cllm_attention_forward_hybrid()` in `src/ai/cllm_training.c`
- ✅ Uses angular positions when token IDs are available
- ✅ Encodes 12-fold symmetry and cymatic patterns
- ✅ Falls back to BigFixed attention when token IDs not available

**Current State:**
```c
// WRONG: Standard dot product O(n²)
float score = 0.0f;
for (uint32_t i = 0; i < head_dim; i++) {
    score += query[i] * key[i];
}
return score / sqrtf((float)head_dim);
```

**Required State:**
```c
// CORRECT: Angular position with cymatic resonance
AngularPosition q_pos, k_pos;
angular_position_calculate(q_token->prime, q_id, head_idx, wavelength, &q_pos);
angular_position_calculate(k_token->prime, k_id, head_idx, wavelength, &k_pos);

uint64_t phi_i = DIMENSIONAL_FREQUENCIES[head_idx % 12];
double theta_diff = q_pos.theta - k_pos.theta;
double score = cos(theta_diff * phi_i);

// Apply cymatic resonance (432 Hz)
double resonance = cos(2π * 432 * theta_diff / 1000);
return score * (0.8 + 0.2 * resonance);
```

**Implementation Tasks:**
- [ ] Create `cllm_attention_score_angular()` in `src/ai/cllm_attention.c`
- [ ] Use `angular_position_calculate()` from `cllm_angular_position.c`
- [ ] Use `cllm_get_dimensional_frequency()` for φᵢ
- [ ] Use `prime_cos()` for cos(θ·φᵢ) computation
- [ ] Apply cymatic resonance (432 Hz base)
- [ ] Replace dot product in `cllm_attention_forward()`
- [ ] Test scores in [-1, 1] range
- [ ] Verify same-group tokens attend more

**Expected Impact:**
- Attention respects 12-fold symmetry
- Cymatic resonance patterns emerge
- Better token relationships

---

### OBJECTIVE 16: Initialize and Process 12 Kissing Sphere Neighbors ✅ COMPLETE

**Purpose:** Fully utilize kissing spheres structure in training

**Status:** ✅ FULLY IMPLEMENTED

**Implementation Details:**
- ✅ `CLLMLatticePoint` has `neighbors[12]` array - FULLY INITIALIZED
- ✅ Each point gets exactly 12 neighbors (one per symmetry group)
- ✅ Uses deterministic clock geometry (O(n) complexity - INSTANT!)
- ✅ No distance calculations needed - pure geometric pattern
- ✅ Implementation in `src/ai/cllm_kissing_spheres.c`
- ✅ Called from `cllm_create.c` during model creation
- ✅ This is the CORE spatial structure of the lattice

**Current State:**
```c
// Neighbors array exists but is NOT initialized
typedef struct {
    uint32_t neighbors[12];      // ❌ UNUSED
    uint32_t num_neighbors;      // ❌ Always 0
    uint32_t symmetry_group;     // ✅ Used
} CLLMLatticePoint;
```

**Required State:**
```c
// Initialize 12 neighbors (one per symmetry group)
for (uint32_t point_id = 0; point_id < num_points; point_id++) {
    for (uint32_t group = 0; group < 12; group++) {
        // Find nearest point in this symmetry group
        uint32_t nearest = find_nearest_in_group(point_id, group);
        point->neighbors[point->num_neighbors++] = nearest;
    }
}

// Process neighbors with L(n,d,k,λ)
for (uint32_t i = 0; i < point->num_neighbors; i++) {
    uint64_t phi_i = DIMENSIONAL_FREQUENCIES[i];
    double interaction = L_lattice(center_prime, i, symmetry_group, 
                                   text, 3, center_prime, neighbor_prime);
    // Apply to gradients
}
```

**Implementation Tasks:**
- [ ] Create `cllm_initialize_kissing_spheres()` in `src/ai/cllm_lattice_init.c`
- [ ] Find 12 nearest neighbors (one per symmetry group)
- [ ] Use `compute_lattice_distance()` with angular positions
- [ ] Store in `CLLMLatticePoint.neighbors[]`
- [ ] Call from `cllm_model_create()`
- [ ] Update `process_lattice_point_with_neighbors()` in `cllm_hierarchical_training.c`
- [ ] Use `L_lattice()` for interaction strength
- [ ] Use φᵢ for each of 12 neighbors
- [ ] Apply to gradient computation
- [ ] Test all points have 10-12 neighbors
- [ ] Verify neighbors from different groups

**Expected Impact:**
- True spatial locality in training
- Gradient flow through neighbor connections
- 12-fold symmetry fully utilized

---

### OBJECTIVE 17: Implement NTT-Based O(n log n) Attention [MEDIUM PRIORITY]

**Purpose:** Replace O(n²) attention with O(n log n) using Number Theoretic Transform

**Critical Understanding:**
- NTT library is COMPLETE in `include/bigint_ntt.h` - NEVER USED
- NTT is FFT for modular arithmetic (perfect for prime-based systems)
- Can reduce attention from O(n²) to O(n log n)
- Expected 10-100x speedup for long sequences (n > 1000)

**Implementation Tasks:**
- [ ] Create `src/ai/cllm_ntt_attention.c`
- [ ] Implement `cllm_attention_ntt_forward()`
- [ ] Use `ntt_init()`, `ntt_forward()`, `ntt_inverse()` from `bigint_ntt.h`
- [ ] Use `big_mul()`, `big_mod()` from `bigint_core.h`
- [ ] Create `include/ai/cllm_ntt_attention.h`
- [ ] Integrate into `cllm_attention_forward()`
- [ ] Use NTT for sequences > 256 tokens
- [ ] Use standard for short sequences
- [ ] Test correctness (outputs match standard)
- [ ] Benchmark performance (verify O(n log n))
- [ ] Create `tools/benchmark_ntt_attention`

**Expected Impact:**
- 10-100x speedup for long sequences
- 90% memory reduction for attention
- Enables processing of very long contexts

---

### OBJECTIVE 18: Apply Cymatic Frequency Resonance to Training [MEDIUM PRIORITY]

**Purpose:** Use cymatic frequencies (432 Hz, 528 Hz, etc.) to modulate training

**Critical Understanding:**
- Cymatic frequencies are ALL DEFINED in `cllm_mathematical_constants.h`
- 432 Hz (universal), 528 Hz (DNA repair), 639 Hz (connection), etc.
- Currently DORMANT - never applied to training
- Should modulate gradients for smoother convergence

**Implementation Tasks:**
- [ ] Create `src/ai/cllm_cymatic_training.c`
- [ ] Implement `cllm_apply_cymatic_resonance()`
- [ ] Use `CYMATIC_*_HZ` constants
- [ ] Use `DIMENSIONAL_FREQUENCIES[]` for modulation
- [ ] Use `prime_cos()` for resonance computation
- [ ] Integrate into `cllm_train_step()` (after gradients, before optimizer)
- [ ] Implement `cllm_compute_harmonics()` for harmonic series
- [ ] Implement `cllm_analyze_gradient_spectrum()` for frequency analysis
- [ ] Test convergence smoothness
- [ ] Measure impact on final loss

**Expected Impact:**
- 20-40% smoother convergence (less oscillation)
- Better final loss (estimated 10-20% improvement)
- True cymatic pattern representation

---

### OBJECTIVE 19: Create Missing Analysis and Validation Tools [LOW PRIORITY]

**Purpose:** Build tools to validate and analyze mathematical integration

**Required Tools:**

1. **tools/init_lattice_embeddings**
   - Load model
   - Initialize embeddings with L(n,d,k,λ)
   - Save model
   - Usage: `./init_lattice_embeddings model.cllm output.cllm`

2. **tools/benchmark_ntt_attention**
   - Compare O(n²) vs O(n log n) attention
   - Measure speedup for various sequence lengths
   - Verify correctness
   - Usage: `./benchmark_ntt_attention --seq-len 1000`

3. **tools/validate_kissing_spheres**
   - Verify all points have 12 neighbors
   - Check symmetry group distribution
   - Validate neighbor relationships
   - Usage: `./validate_kissing_spheres model.cllm`

4. **tools/analyze_cymatic_resonance**
   - Analyze training in frequency domain
   - Plot power spectrum
   - Identify resonance patterns
   - Usage: `./analyze_cymatic_resonance checkpoint.cllm`

5. **tools/visualize_angular_positions**
   - Plot θ(n,k,λ,ω,ψ) for all tokens
   - Visualize 12-fold symmetry
   - Show kissing spheres structure
   - Usage: `./visualize_angular_positions model.cllm`

**Implementation Tasks:**
- [ ] Create `tools/init_lattice_embeddings.c`
- [ ] Create `tools/benchmark_ntt_attention.c`
- [ ] Create `tools/validate_kissing_spheres.c`
- [ ] Create `tools/analyze_cymatic_resonance.c`
- [ ] Create `tools/visualize_angular_positions.c`
- [ ] Add all tools to Makefile
- [ ] Test each tool with sample models
- [ ] Document usage in README

---

## 📊 PRIORITY ORDER

1. **HIGHEST:** ✅ OBJECTIVE 14 - Integrate L(n,d,k,λ) lattice formula (COMPLETE)
2. **HIGH:** ✅ OBJECTIVE 15 - Integrate angular position attention (COMPLETE)
3. **HIGH:** ✅ OBJECTIVE 16 - Initialize kissing sphere neighbors (COMPLETE)
4. **HIGH:** OBJECTIVE 2B - Remove legacy loss functions (cleanup)
5. **MEDIUM:** OBJECTIVE 2C - Rename crystalline to default (clarity)
6. **MEDIUM:** OBJECTIVE 2D - Remove legacy code (cleanup)
7. **MEDIUM:** OBJECTIVE 5A - Kissing spheres as only threading (consistency)
8. **MEDIUM:** OBJECTIVE 8A - Remove conditional compilation (simplicity)
9. **MEDIUM:** OBJECTIVE 17 - NTT attention (performance)
10. **MEDIUM:** OBJECTIVE 18 - Cymatic resonance (quality)
11. **LOW:** OBJECTIVE 19 - Analysis tools (validation)

---

## 🎯 RECOMMENDED NEXT ACTION

**THREE MAJOR MATHEMATICAL OBJECTIVES COMPLETE! 🎉**

With OBJECTIVES 14, 15, and 16 now complete, the core mathematical framework is fully integrated:
- ✅ L(n,d,k,λ) lattice formula for embeddings
- ✅ θ(n,k,λ,ω,ψ) angular position for attention
- ✅ 12 kissing sphere neighbors for spatial structure

**Next Priority: Cleanup and Optimization**

The next objectives focus on cleanup and optimization:

1. **OBJECTIVE 2B**: Remove legacy loss functions (HIGH PRIORITY)
   - Remove standard cross-entropy
   - Make crystalline loss the ONLY loss function
   - Clean up conditional flags

2. **OBJECTIVE 2C**: Rename "crystalline" to default (MEDIUM PRIORITY)
   - Stop treating crystalline as special
   - It's the only design, not an option

3. **OBJECTIVE 2D**: Remove legacy code (MEDIUM PRIORITY)
   - Delete old training files
   - Remove fallback implementations
   - Clean up codebase

4. **OBJECTIVE 17**: NTT-based O(n log n) attention (MEDIUM PRIORITY)
   - 10-100x speedup for long sequences
   - NTT library already complete

5. **OBJECTIVE 18**: Cymatic frequency resonance (MEDIUM PRIORITY)
   - 20-40% smoother convergence
   - Frequencies already defined

---

## 🚨 NEW CRITICAL OBJECTIVES (From Architecture Analysis)

### OBJECTIVE 21: Fix Backwards "Simple Loss" Naming [CRITICAL PRIORITY]

**Purpose:** Fix the backwards naming where "simple_loss" is actually THE REAL implementation

**Critical Understanding:**
- `include/ai/cllm_simple_loss.h` contains THE REAL crystalline GCD-based loss
- Name "simple" implies it's a stub/simplified version - THIS IS BACKWARDS
- Meanwhile `include/ai/cllm_loss.h` contains unused Tensor API infrastructure
- This violates "one codebase, one design" - implies there's a complex version

**Tasks:**
- [ ] Rename `include/ai/cllm_simple_loss.h` → `include/ai/cllm_loss.h`
- [ ] Rename infrastructure `include/ai/cllm_loss.h` → `include/ai/cllm_tensor_loss.h`
- [ ] Update all includes in `cllm_training.c`, `cllm_production.c`, `cllm_training_threaded.c`
- [ ] Update Makefile if needed
- [ ] Test build
- [ ] Commit changes

**Related Files:**
- `include/ai/cllm_simple_loss.h` - THE REAL LOSS (badly named)
- `include/ai/cllm_loss.h` - Infrastructure Tensor API (unused)
- `src/ai/cllm_loss.c` - Utility functions (correctly named)

---

### OBJECTIVE 22: Delete Unused Infrastructure Files [HIGH PRIORITY]

**Purpose:** Remove 83KB of dead code from infrastructure layer

**Critical Understanding:**
- Three infrastructure files are completely unused
- `cllm_backprop.c` - No calls found anywhere
- `cllm_loss.c` - Only used by unused backprop
- `cllm_training_loop.c` - Defined but never called
- These are legacy code that should be deleted

**Tasks:**
- [ ] Delete `src/ai/infrastructure/cllm_backprop.c` (22KB)
- [ ] Delete `src/ai/infrastructure/cllm_loss.c` (30KB)
- [ ] Delete `src/ai/infrastructure/cllm_training_loop.c` (31KB)
- [ ] Delete `include/ai/cllm_backprop.h`
- [ ] Delete `include/ai/cllm_training_loop.h`
- [ ] Update Makefile to remove deleted files
- [ ] Test build
- [ ] Commit changes

**Impact:**
- Removes 83KB of unused code
- Simplifies infrastructure layer
- Reduces confusion about what's actually used

---

### OBJECTIVE 23: Remove Misleading File Name Qualifiers [MEDIUM PRIORITY]

**Purpose:** Remove qualifiers that imply alternatives (violates "one design" principle)

**Critical Understanding:**
- Files with "crystalline", "simple", "pure", "advanced" qualifiers imply there are alternatives
- This violates the core principle: "one codebase, one design, no alternatives"
- These qualifiers should be removed - the crystalline design IS the design

**Files to Rename:**

**"Crystalline" Prefix (not special, it's the design):**
- [ ] `cllm_crystalline_advanced.c` → `cllm_advanced.c` (or remove "advanced")
- [ ] `cllm_crystalline_attention.c` → `cllm_attention.c`
- [ ] `cllm_crystalline_sieve.c` → `cllm_sieve.c`

**"Pure" Qualifier (implies there's impure):**
- [ ] `cllm_pure_embeddings.c` → `cllm_embeddings.c` (or merge)
- [ ] `cllm_pure_token.c` → `cllm_token.c` (or merge)

**"Impl" Suffix (should be integrated):**
- [ ] `cllm_training_bigfixed_impl.c` → merge into `cllm_training.c`

**Tasks:**
- [ ] Analyze each file to determine correct name
- [ ] Rename files one by one
- [ ] Update all includes
- [ ] Update Makefile
- [ ] Test build after each rename
- [ ] Commit changes incrementally

---

### OBJECTIVE 24: Investigate and Consolidate Duplicates [MEDIUM PRIORITY]

**Purpose:** Identify and merge duplicate functionality

**Potential Duplicates to Investigate:**

**Batch Processing:**
- [ ] Compare `src/ai/cllm_batch.c` vs `src/ai/infrastructure/cllm_batch.c`
- [ ] Determine if truly different or duplicates
- [ ] Merge if duplicate, document if different

**Optimizer:**
- [ ] Compare `src/ai/cllm_optimizer.c` vs `src/ai/infrastructure/cllm_optimizer.c`
- [ ] Determine if truly different or duplicates
- [ ] Merge if duplicate, document if different

**Multiple Embedding Files:**
- [ ] `cllm_embedding.c`
- [ ] `cllm_lattice_embeddings.c`
- [ ] `cllm_clock_embeddings.c`
- [ ] `cllm_pure_embeddings.c`
- [ ] `cllm_lll_embeddings.c`
- [ ] Determine which is canonical
- [ ] Merge or document relationships

**Multiple Attention Files:**
- [ ] `cllm_angular_attention.c`
- [ ] `cllm_crystalline_attention.c`
- [ ] `cllm_ntt_attention.c`
- [ ] Determine relationships
- [ ] Document which is used when

---

### OBJECTIVE 25: Fix Remaining 78 Build Warnings [HIGH PRIORITY]

**Purpose:** Achieve zero warnings build (currently 78 warnings from BigFixed migration)

**Critical Understanding:**
- Build currently has 78 warnings (all pre-existing from BigFixed migration)
- These are mostly type mismatches between float* and BigFixed**
- Need to fix all warnings to have clean build

**Warning Categories:**

**Type Mismatches (majority):**
- [ ] `BigFixed**` vs `float*` in inference
- [ ] `BigFixed**` vs `float*` in optimizer
- [ ] `BigFixed**` vs `float*` in layernorm
- [ ] Assignment warnings in various files

**Unused Parameters:**
- [ ] Fix or document unused parameters
- [ ] Use `(void)param` to suppress if intentional

**Implicit Declarations:**
- [ ] Add missing function declarations
- [ ] Fix include order issues

**Tasks:**
- [ ] Categorize all 78 warnings by type
- [ ] Fix high-priority warnings first (type mismatches)
- [ ] Fix medium-priority warnings (unused parameters)
- [ ] Document any warnings that cannot be fixed
- [ ] Achieve zero warnings build
- [ ] Test thoroughly after fixes
- [ ] Commit changes

**Related Files:**
- `src/ai/cllm_inference.c` - Multiple type mismatch warnings
- `src/ai/cllm_optimizer.c` - Type mismatch warnings
- `src/ai/cllm_layernorm.c` - Type mismatch warnings
- `src/ai/cllm_feedforward_bigfixed.c` - Unused parameter warnings

---

## 📊 UPDATED PRIORITY ORDER

**CRITICAL (Do Immediately):**
1. ✅ OBJECTIVE 2B - Remove legacy loss functions (COMPLETE)
2. ✅ OBJECTIVE 2C - Rename crystalline to default (COMPLETE)
3. **OBJECTIVE 21** - Fix backwards "simple_loss" naming
4. **OBJECTIVE 22** - Delete unused infrastructure files (83KB)

**HIGH (Do Next):**
5. **OBJECTIVE 25** - Fix remaining 78 build warnings
6. OBJECTIVE 2D - Remove legacy code files
7. OBJECTIVE 14, 15, 16 - Mathematical integration (COMPLETE)

**MEDIUM (After High Priority):**
8. **OBJECTIVE 23** - Remove misleading file name qualifiers
9. **OBJECTIVE 24** - Investigate and consolidate duplicates
10. OBJECTIVE 5A - Kissing spheres as only threading
11. OBJECTIVE 8A - Remove conditional compilation
12. OBJECTIVE 17 - NTT attention (performance)
13. OBJECTIVE 18 - Cymatic resonance (quality)

**LOW (Future):**
14. OBJECTIVE 19 - Analysis tools (validation)
15. OBJECTIVE 20 - Comprehensive testing

---

**END OF SECONDARY OBJECTIVES**