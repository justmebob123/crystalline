# TODO - Crystalline CLLM Project

**CRITICAL RULES (from MASTER_PLAN.md) - MUST FOLLOW:**
- **Rule 0**: ALWAYS paste rules to top of todo.md with EVERY response, read MASTER_PLAN.md, AUDIT.md, SECONDARY_OBJECTIVES.md
- **Rule 1**: ALWAYS reread MASTER_PLAN.md before ANY action
- **Rule 2**: Reference AUDIT.md for architectural state
- **Rule 3**: Reference SECONDARY_OBJECTIVES.md for detailed tasks
- **Rule 4**: Do NOT create new .md files
- **Rule 5**: ALWAYS commit using: `git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main`
- **Rule 6**: MASTER_PLAN.md is READ-ONLY - do not edit without explicit approval
- **Rule 7**: FIX ALL BUILD WARNINGS before proceeding
- **Rule 8**: NO math.h usage - ONLY crystalline math functions (prime_*)

## 🔍 ROOT CAUSE IDENTIFIED: O(n²) Tokenization Hang

### The Actual Problem

**Location**: `src/ai/cllm_training.c:457-463`
**Issue**: O(n²) vocabulary lookup during data loading
**Impact**: With 73MB file + 50k vocab, this takes FOREVER and appears to hang

**Code**:
```c
// For EACH token in file (could be millions)
for (uint32_t i = 0; i < training->model->vocab_size; i++) {
    // Linear search through ENTIRE vocabulary (50k tokens)
    if (strcmp(training->model->tokens[i].token_str, token) == 0) {
        // ...
    }
}
```

**Complexity**: O(num_tokens_in_file × vocab_size)
- 73MB file ≈ 10M tokens
- vocab_size = 50,000
- Total comparisons: 10M × 50k = 500 BILLION string comparisons!

**This is why it hangs!**

### Solution: Use Tokenizer's Hash Map

The tokenizer already has O(1) hash map lookup! We just need to use it.

1. [ ] Store tokenizer reference in CLLMTraining structure
2. [ ] Use tokenizer's cllm_tokenize() function instead of manual loop
3. [ ] Reduce complexity from O(n²) to O(n)

### Implementation Plan

1. [ ] Add `CLLMTokenizer* tokenizer` field to CLLMTraining structure
2. [ ] Pass tokenizer to cllm_training_init()
3. [ ] Use `cllm_tokenize()` in cllm_load_training_data()
4. [ ] Remove O(n²) linear search loop
5. [ ] Test with 73MB file
6. [ ] Verify no hang
7. [ ] Verify threads start
8. [ ] Verify training runs

### Current Status

**Build**: ✅ Compiles
**Type Fixes**: ✅ Applied
**Application Hang**: 🔍 ROOT CAUSE IDENTIFIED - O(n²) tokenization
**Issue**: NOT threading, NOT batch size - it's tokenization performance
**Solution**: Use existing tokenizer hash map

### Previous Errors Acknowledged

- ✅ Acknowledged incomplete testing
- ✅ Acknowledged false claims  
- ✅ Identified actual root cause
- ✅ Focused on real user issue

---

**Status**: 🔍 ROOT CAUSE IDENTIFIED
**Priority**: HIGHEST - Fix O(n²) tokenization
**Next Action**: Implement tokenizer-based loading