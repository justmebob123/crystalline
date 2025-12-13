# Critical Bugs Identified - Unified CLLM Tool

## Bug #1: Gradient Buffer Size Mismatch ⚠️ CRITICAL

**Location:** `src/ai/cllm_training_threaded.c:1427`

**Problem:**
```c
// WRONG: Uses vocab_size instead of max_tokens
system->gradient_size = training->model->vocab_size * training->model->embedding_dim;
```

**Actual allocation in training object:**
```c
// In cllm_training_functions.c:101
size_t max_tokens = config->batch_size * config->sequence_length;
training->gradients = calloc(max_tokens * model->embedding_dim, sizeof(double));
```

**Impact:**
- When copying gradients, we write beyond the allocated buffer
- Causes heap corruption
- Results in "free(): invalid pointer" crash
- Valgrind shows: "Invalid write of size 8... 0 bytes after a block of size 262,144"

**Fix:**
```c
// CORRECT: Use max_tokens like the training object does
size_t max_tokens = training->config.batch_size * training->config.sequence_length;
system->gradient_size = max_tokens * training->model->embedding_dim;
```

---

## Bug #2: Batch Buffer Overflow ⚠️ CRITICAL

**Location:** `src/ai/cllm_training_threaded.c:428, 594, 574`

**Problem:**
Reading beyond allocated batch buffer when accessing `input_tokens[idx]`

**Root Cause:**
The batch is allocated with size `batch_size * seq_len`, but the code might be accessing beyond this when:
1. Sequence length varies
2. Batch padding is incorrect
3. Index calculation is wrong

**Valgrind Output:**
```
Invalid read of size 4
  at cllm_forward_training_threaded (cllm_training_threaded.c:428)
  Address 0x931b0d0 is 0 bytes after a block of size 2,048 alloc'd
```

**Analysis:**
- Block size: 2,048 bytes = 512 uint32_t values
- With batch_size=4, seq_len=128: 4 * 128 = 512 tokens ✓
- The issue is reading at index 512 (one past the end)

**Likely Cause:**
Loop condition is `<=` instead of `<`, or off-by-one error in index calculation.

---

## Bug #3: Sequence Length Mismatch

**Problem:**
The training context uses `local_ctx->seq_len` which might not match `batch->seq_len`

**Impact:**
- Accessing wrong indices in batch data
- Reading/writing beyond allocated memory

---

## Fixes Required

### Fix #1: Correct Gradient Size Calculation
```c
// In threaded_training_create_system()
size_t max_tokens = training->config.batch_size * training->config.sequence_length;
system->gradient_size = max_tokens * training->model->embedding_dim;
```

### Fix #2: Add Bounds Checking
```c
// In cllm_forward_training_threaded()
for (int b = 0; b < batch_size; b++) {
    for (int s = 0; s < seq_len; s++) {
        int idx = b * seq_len + s;
        
        // ADD BOUNDS CHECK
        if (idx >= batch_size * seq_len) {
            fprintf(stderr, "ERROR: Index %d exceeds batch size %d\n", 
                    idx, batch_size * seq_len);
            continue;
        }
        
        uint32_t token_id = input_tokens[idx];
        // ...
    }
}
```

### Fix #3: Verify Sequence Lengths Match
```c
// Ensure local_ctx->seq_len matches batch->seq_len
if (local_ctx->seq_len != batch->seq_len) {
    fprintf(stderr, "ERROR: Sequence length mismatch: ctx=%d, batch=%d\n",
            local_ctx->seq_len, batch->seq_len);
    return -1;
}
```

---

## Priority

1. **HIGHEST:** Fix gradient size calculation (causes heap corruption)
2. **HIGH:** Add bounds checking in batch processing
3. **MEDIUM:** Verify sequence length consistency