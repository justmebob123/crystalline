# Phase 4: Unified API & Naming Cleanup - Refactoring Plan

## Principle: Kissing Spheres is THE Solution, Not AN Option

Since kissing spheres is the ONLY threading model and the fundamental architecture, we should:
1. Remove redundant "kissing_spheres" naming
2. Integrate into core APIs as default behavior
3. Eliminate the illusion of choice (no "with_neighbors" suffixes)

---

## REFACTORING STRATEGY

### Option A: Enhance Existing Functions (RECOMMENDED)
**Approach:** Add neighbor processing to existing attention/embedding functions
**Pros:** 
- Clean API (no redundant names)
- Single source of truth
- Natural integration
**Cons:**
- Need to modify existing code carefully
- More complex initial refactoring

### Option B: Keep Separate But Rename
**Approach:** Keep separate files but remove redundant prefixes
**Pros:**
- Less risky initially
- Easier to test
**Cons:**
- Still maintains artificial separation
- Doesn't reflect "ONLY solution" principle

**DECISION: Use Option A** - Enhance existing functions

---

## DETAILED REFACTORING PLAN

### 1. ATTENTION INTEGRATION

**Current State:**
- `cllm_attention_forward()` in `cllm_inference.c` - standard attention
- `cllm_attention_kissing_spheres()` in `cllm_kissing_spheres_attention.c` - neighbor attention

**Target State:**
- `cllm_attention_forward()` - enhanced with neighbor processing
- Remove separate kissing_spheres_attention files

**Implementation:**
```c
// Enhanced cllm_attention_forward() in cllm_inference.c
void cllm_attention_forward(AttentionLayer* layer, double* input, double* output,
                           CLLMModel* model, uint32_t token_id, int seq_len) {
    // Standard attention computation
    // ... existing code ...
    
    // Add neighbor influence (if model has lattice points)
    if (model && model->lattice_points && token_id < model->num_lattice_points) {
        cllm_add_neighbor_attention(model, token_id, output, layer->embed_dim);
    }
}

// Internal helper function
static void cllm_add_neighbor_attention(CLLMModel* model, uint32_t token_id,
                                       double* output, uint32_t embed_dim) {
    // Compute neighbor contributions using L(n,d,k,λ)
    // Add weighted neighbor influence to output
}
```

### 2. EMBEDDING INTEGRATION

**Current State:**
- `cllm_embeddings_init()` - standard initialization
- `cllm_embedding_refine_with_neighbors()` - neighbor refinement

**Target State:**
- `cllm_embeddings_init()` - includes neighbor influence by default
- `cllm_embedding_refine()` - simplified name (neighbors are implicit)

**Implementation:**
```c
// Enhanced initialization
int cllm_embeddings_init(CLLMModel* model, float neighbor_influence) {
    // Standard initialization
    // ... existing code ...
    
    // Add neighbor influence (always, if lattice exists)
    if (model->lattice_points) {
        cllm_apply_neighbor_influence(model, neighbor_influence);
    }
}

// Simplified refinement
int cllm_embedding_refine(CLLMModel* model, float influence_strength) {
    // Refine using 12 neighbors (implicit)
    return cllm_apply_neighbor_influence(model, influence_strength);
}
```

### 3. BATCH DISTRIBUTION INTEGRATION

**Current State:**
- `cllm_batch_assign_by_symmetry()` - symmetry-based assignment

**Target State:**
- `cllm_batch_assign()` - simplified name (symmetry is implicit)

**Implementation:**
```c
// Simplified batch assignment
int cllm_batch_assign(CLLMBatch* batch, struct CLLMModel* model,
                     int* assignments, int num_workers) {
    // Assign by symmetry (the only way)
    return cllm_batch_assign_by_symmetry_impl(batch, model, assignments, num_workers);
}
```

---

## FILE ORGANIZATION

### Files to Modify:
1. `src/ai/cllm_inference.c` - Enhance attention
2. `src/ai/cllm_embeddings.c` - Enhance embeddings
3. `src/ai/cllm_batch.c` - Simplify naming
4. `include/cllm_inference.h` - Update signatures
5. `include/cllm.h` - Update signatures

### Files to Deprecate/Remove:
1. `include/cllm_kissing_spheres_attention.h` - Merge into cllm_inference.h
2. `src/ai/cllm_kissing_spheres_attention.c` - Merge into cllm_inference.c
3. `include/cllm_kissing_spheres_embeddings.h` - Merge into cllm.h
4. `src/ai/cllm_kissing_spheres_embeddings.c` - Merge into cllm_embeddings.c

### New Internal Files (if needed):
1. `src/ai/cllm_neighbor_ops.c` - Internal neighbor operations
2. `include/cllm_neighbor_ops.h` - Internal header (not public API)

---

## MIGRATION STRATEGY

### Phase 4A: Create Internal Helpers
1. Extract neighbor logic into internal helper functions
2. Keep existing APIs working
3. Test thoroughly

### Phase 4B: Enhance Core Functions
1. Add neighbor processing to core functions
2. Make it default behavior
3. Update documentation

### Phase 4C: Deprecate Old APIs
1. Mark old functions as deprecated
2. Update all callers
3. Remove deprecated code

### Phase 4D: Simplify Naming
1. Remove redundant prefixes/suffixes
2. Update all references
3. Final cleanup

---

## BACKWARD COMPATIBILITY

### Approach:
- Keep old function names as wrappers initially
- Add deprecation warnings
- Remove in next major version

### Example:
```c
// Deprecated wrapper
__attribute__((deprecated("Use cllm_attention_forward instead")))
int cllm_attention_kissing_spheres(CLLMModel* model, uint32_t token_id,
                                   const float* query, float* output,
                                   float* attention_weights) {
    // Call new unified function
    return cllm_attention_forward_with_token(model, token_id, query, output);
}
```

---

## TESTING STRATEGY

### Unit Tests:
1. Test neighbor computation in isolation
2. Test attention with/without neighbors
3. Test embedding refinement
4. Test batch assignment

### Integration Tests:
1. Full training loop with new API
2. Inference with new API
3. Performance benchmarks

### Regression Tests:
1. Ensure results match previous implementation
2. Verify no performance degradation
3. Check memory usage

---

## SUCCESS CRITERIA

1. ✅ Zero redundant "kissing_spheres" naming
2. ✅ Neighbor processing is default (not optional)
3. ✅ Single, unified API
4. ✅ 0 errors, 0 warnings
5. ✅ All tests pass
6. ✅ Documentation updated
7. ✅ Clean, maintainable code

---

## TIMELINE

- **Phase 4A:** 2-3 hours (internal helpers)
- **Phase 4B:** 2-3 hours (enhance core functions)
- **Phase 4C:** 1-2 hours (deprecate old APIs)
- **Phase 4D:** 1 hour (final cleanup)
- **Total:** 6-9 hours

---

## DECISION: Proceed with Option A

We will enhance existing functions rather than maintain separate APIs. This reflects the reality that kissing spheres is THE solution, not an optional feature.

**Next Steps:**
1. Create internal helper functions
2. Enhance cllm_attention_forward()
3. Enhance cllm_embeddings_init()
4. Simplify batch assignment naming
5. Test and verify
6. Remove deprecated code