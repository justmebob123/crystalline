# SECONDARY OBJECTIVES - DETAILED IMPLEMENTATION TASKS
   
   ## OVERVIEW
   This file contains detailed implementation tasks for completing the Crystalline CLLM integration.
   Refer to MASTER_PLAN.md for high-level objectives and architectural requirements.
   
   ---
   
   ## OBJECTIVE 3A: COMPLETE BIGFIXED MIGRATION
   
   ### CRITICAL REQUIREMENT
   **NO float arithmetic ANYWHERE in the codebase** (MASTER_PLAN OBJECTIVE 3A)
   
   ### CURRENT STATE (from conversation summary)
   - ✅ Training path: COMPLETE with BigFixed
   - ❌ Inference path: Uses float arithmetic (VIOLATION)
   - ✅ BigFixed implementations created: feedforward, layernorm, attention
   - ⚠️ 79 type mismatch warnings
   
   ### PHASE 1: Convert Inference to BigFixed [HIGH PRIORITY]
   
   **Problem:** Inference uses float-based functions, violating OBJECTIVE 3A
   
   **Files to Update:**
   1. `src/ai/cllm_inference.c` - Line 343 calls float-based attention
   2. `include/cllm_inference.h` - Update CLLMInference structure
   
   **Tasks:**
   - [ ] Update `CLLMInference` structure to use `BigFixed**` instead of `float*`
   - [ ] Update inference forward pass to call `cllm_attention_forward_bigfixed()`
   - [ ] Update inference to call `cllm_layer_norm_bigfixed()`
   - [ ] Update inference to call `cllm_feedforward_bigfixed()`
   - [ ] Convert embeddings and logits to BigFixed
   - [ ] Test inference with BigFixed
   
   **Implementation Details:**
   ```c
   // OLD (float-based):
   cllm_attention_forward(attn_layer, inference->hidden_states, ...)
   cllm_layer_norm_old(...)
   cllm_feed_forward(...)
   
   // NEW (BigFixed):
   cllm_attention_forward_bigfixed(attn_layer, inference->hidden_states, ...)
   cllm_layer_norm_bigfixed(...)
   cllm_feedforward_bigfixed(...)
   ```
   
   ### PHASE 2: Remove/Deprecate Float Versions [CLEANUP]
   
   **Problem:** Float-based functions still exist and could be called accidentally
   
   **Functions to Mark as Deprecated:**
   - [ ] `cllm_layer_norm_old()` in `src/ai/cllm_layernorm.c`
   - [ ] `cllm_attention_forward()` in `src/ai/cllm_training.c`
   - [ ] `cllm_feed_forward()` in `src/ai/cllm_feedforward.c`
   
   **Tasks:**
   - [ ] Add deprecation warnings to float-based functions
   - [ ] Update all callers to use BigFixed versions
   - [ ] Consider removing float versions entirely
   - [ ] Update documentation to reflect BigFixed as standard
   
   ### PHASE 3: Fix Type Mismatch Warnings [CODE QUALITY]
   
   **Problem:** 79 warnings about incompatible pointer types (BigFixed** vs float*)
   
   **Files with Warnings:**
   - `src/ai/cllm_feedforward_bigfixed.c` - New file warnings
   - `src/ai/cllm_layernorm_bigfixed.c` - New file warnings
   - `src/ai/cllm_training.c` - Type mismatches
   - `src/ai/cllm_optimizer.c` - Type mismatches
   - `src/ai/cllm_validate.c` - Type mismatches
   
   **Tasks:**
   - [ ] Fix function signatures to match BigFixed** types
   - [ ] Update memory allocations to use BigFixed
   - [ ] Fix pointer casts and conversions
   - [ ] Achieve zero warnings
   
   ### PHASE 4: Verify Complete Coverage [VALIDATION]
   
   **Tasks:**
   - [ ] Search for all `float*` usage in training/inference
   - [ ] Verify all math operations use crystalline equivalents
   - [ ] Check that all allocations use BigFixed
   - [ ] Run functional tests to verify correctness
   - [ ] Benchmark performance
   
   ---
   
   ## OBJECTIVE 2: TRAINING PIPELINE CLEANUP
   
   ### Remove Legacy Loss Functions
   
   **Problem:** Standard cross-entropy still exists as fallback
   
   **Tasks:**
   - [ ] Remove `cllm_compute_loss_training()` function
   - [ ] Remove conditional flag check in `cllm_train_epoch()`
   - [ ] Make `cllm_compute_loss_crystalline()` the ONLY loss function
   - [ ] Rename to `cllm_compute_loss()`
   - [ ] Remove `use_crystalline_optimizations` flag
   
   ### Remove Legacy Training Files
   
   **Files to Delete:**
   - [ ] `src/ai/cllm_training_mt.c` - Old multi-threading
   - [ ] `src/ai/cllm_training_parallel.c` - Unused parallel code
   - [ ] `src/ai/cllm_train_complete.c` - Legacy training wrapper
   - [ ] `include/cllm_training_mt.h`
   - [ ] `include/cllm_training_parallel.h`
   - [ ] `include/cllm_train_complete.h`
   
   ### Update Makefile
   - [ ] Remove deleted files from build
   - [ ] Verify build after deletions
   
   ---
   
   ## OBJECTIVE 5A: KISSING SPHERES AS ONLY THREADING
   
   ### Remove Non-Kissing-Spheres Threading
   
   **Problem:** Multiple threading implementations exist
   
   **Tasks:**
   - [ ] Remove ALL fallbacks to old threading in `tools/train_model.c`
   - [ ] Make kissing spheres mandatory (no single-threaded fallback)
   - [ ] Remove `cllm_train_epoch_mt()` completely
   - [ ] Update tools to require kissing spheres
   - [ ] Document kissing spheres as the only threading model
   
   ---
   
   ## OBJECTIVE 8A: REMOVE ALL CONDITIONAL COMPILATION
   
   ### Remove Feature Flags
   
   **Problem:** Feature flags and conditional compilation everywhere
   
   **Tasks:**
   - [ ] Remove all feature flags from config structs
   - [ ] Remove all #ifdef blocks for features
   - [ ] One implementation per function (no alternatives)
   - [ ] No "enable_X" configuration options
   - [ ] Crystalline design is always active
   - [ ] No compile-time toggles
   - [ ] No runtime toggles
   - [ ] Single code path for each operation
   
   ---
   
   ## OBJECTIVE 14: INTEGRATE L(n,d,k,λ) LATTICE FORMULA
   
   ### Replace Random Embeddings with Crystalline Lattice
   
   **Problem:** Embeddings use random initialization instead of L(n,d,k,λ) formula
   
   **Tasks:**
   - [ ] Create `cllm_embeddings_init_lattice()` in `src/ai/cllm_embeddings.c`
   - [ ] Use `L_lattice()` from `src/geometry/prime_lattice_core.c`
   - [ ] Use `DIMENSIONAL_FREQUENCIES[]` from `cllm_mathematical_constants.h`
   - [ ] Use `prime_tanh()` for normalization
   - [ ] Replace call in `cllm_model_create()`
   - [ ] Test embeddings are in [-1, 1] range
   - [ ] Verify symmetry group similarity
   - [ ] Compare convergence to random baseline
   
   **Implementation:**
   ```c
   // Use L(n,d,k,λ) formula for embeddings
   for (uint32_t token_id = 0; token_id < vocab_size; token_id++) {
       for (uint32_t dim = 0; dim < embedding_dim; dim++) {
           uint64_t phi_i = DIMENSIONAL_FREQUENCIES[dim % 12];
           double L_value = L_lattice(
               token->prime_encoding,    // n
               dim,                      // d
               token->symmetry_group,    // k
               token->text,              // λ
               3,                        // ω
               token->prime_encoding,    // p
               phi_i                     // q
           );
           embeddings[token_id * embedding_dim + dim] = (float)tanh(L_value / 100.0);
       }
   }
   ```
   
   ---
   
   ## STATUS TRACKING
   
   ### Completed:
   - [x] Created `cllm_feedforward_bigfixed.c` (COMPLETE - NO STUBS)
   - [x] Created `cllm_layernorm_bigfixed.c` (COMPLETE - NO STUBS)
   - [x] Created `cllm_attention_forward_bigfixed()` (COMPLETE)
   - [x] Fixed 5 critical warnings (sqrt, expf, memory bugs)
   - [x] Training path uses BigFixed throughout
   
   ### In Progress:
   - [ ] Convert inference to BigFixed (PHASE 1)
   - [ ] Fix 79 type mismatch warnings (PHASE 3)
   
   ### Not Started:
   - [ ] Remove legacy loss functions
   - [ ] Remove legacy training files
   - [ ] Remove non-kissing-spheres threading
   - [ ] Remove conditional compilation
   - [ ] Integrate L(n,d,k,λ) lattice formula
   
   ---
   
   ## PRIORITY ORDER
   
   1. **HIGHEST:** Convert inference to BigFixed (violates OBJECTIVE 3A)
   2. **HIGH:** Fix type mismatch warnings (code quality)
   3. **MEDIUM:** Remove legacy code (cleanup)
   4. **MEDIUM:** Integrate lattice formula (mathematical correctness)
   5. **LOW:** Documentation and testing