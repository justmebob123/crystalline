# TRAINING FIX PLAN - Immediate Actions

## Priority 1: Add Diagnostics (30 minutes)

### Goal
Understand why loss isn't decreasing by adding diagnostic output.

### Changes

**File**: `src/ai/cllm_training.c`

**Location**: After `cllm_backward_training()` call in `cllm_train_epoch()`

**Add**:
```c
// DIAGNOSTIC: Check gradient magnitudes
if (num_batches == 1 || num_batches % 5 == 0) {
    float max_embed_grad = 0.0f, sum_embed_grad = 0.0f;
    int nonzero_embed = 0;
    size_t embed_size = model->vocab_size * model->embedding_dim;
    
    for (size_t i = 0; i < embed_size && i < 10000; i++) {
        float g = fabsf(training->gradients[i]);
        if (g > 1e-10f) {
            nonzero_embed++;
            sum_embed_grad += g;
            if (g > max_embed_grad) max_embed_grad = g;
        }
    }
    
    float max_attn_grad = 0.0f;
    int nonzero_attn = 0;
    if (training->attention_grads && model->num_layers > 0) {
        size_t size = model->embedding_dim * model->embedding_dim;
        for (size_t i = 0; i < size && i < 10000; i++) {
            float g = fabsf(training->attention_grads[0].query_lattice[i]);
            if (g > 1e-10f) {
                nonzero_attn++;
                if (g > max_attn_grad) max_attn_grad = g;
            }
        }
    }
    
    printf("    Gradients: embed=%d (max=%.2e, avg=%.2e), attn=%d (max=%.2e)\n",
           nonzero_embed, max_embed_grad,
           nonzero_embed > 0 ? sum_embed_grad / nonzero_embed : 0.0f,
           nonzero_attn, max_attn_grad);
}
```

**Also add weight diagnostics at start**:
```c
// DIAGNOSTIC: Check weight initialization (first batch only)
if (training->current_epoch == 0 && num_batches == 0) {
    float sum_embed = 0.0f, sum_attn = 0.0f, sum_ff = 0.0f;
    int count = 100;
    
    for (int i = 0; i < count; i++) {
        sum_embed += fabsf(model->embeddings.embeddings[i]);
    }
    
    if (model->attention_layers && model->attention_layers[0].query_lattice) {
        for (int i = 0; i < count; i++) {
            sum_attn += fabsf(model->attention_layers[0].query_lattice[i]);
        }
    }
    
    if (model->ff_layers && model->ff_layers[0].w1_lattice) {
        for (int i = 0; i < count; i++) {
            sum_ff += fabsf(model->ff_layers[0].w1_lattice[i]);
        }
    }
    
    printf("  Weight magnitudes: embed=%.6f, attn=%.6f, ff=%.6f\n",
           sum_embed/count, sum_attn/count, sum_ff/count);
}
```

## Priority 2: Fix Attention Forward (2 hours)

### Problem
Current training uses simplified attention that just doubles the input:
```c
memcpy(attn_out, input, embed_dim * sizeof(float));
for (uint32_t d = 0; d < embed_dim; d++) attn_out[d] += input[d];
```

This is NOT attention - it's just `output = 2 * input`.

### Solution
Use proper attention from `cllm_attention_forward()`.

**File**: `src/ai/cllm_training.c` in `cllm_forward_training()`

**Replace lines 544-558** (the attention section):

**OLD**:
```c
// Attention (simplified)
memcpy(attn_out, input, embed_dim * sizeof(float));
for (uint32_t d = 0; d < embed_dim; d++) attn_out[d] += input[d];
```

**NEW**:
```c
// Proper multi-head attention
AttentionLayer* attn_layer = &model->attention_layers[layer];

// Process batch through attention
for (int batch_idx = 0; batch_idx < batch_size; batch_idx++) {
    int start_idx = batch_idx * seq_len;
    float* batch_input = &layer_input[start_idx * embed_dim];
    float* batch_output = &training->attention_outputs[layer][start_idx * embed_dim];
    
    cllm_attention_forward(attn_layer, batch_input, batch_output,
                          NULL, NULL, seq_len);
}

// Copy attention output for feedforward
for (int b = 0; b < batch_size; b++) {
    for (int s = 0; s < seq_len; s++) {
        int idx = b * seq_len + s;
        float* attn_out = &training->attention_outputs[layer][idx * embed_dim];
        // attn_out is already set by cllm_attention_forward
    }
}
```

**Note**: Need to include the header:
```c
#include "../include/cllm_inference.h"  // For cllm_attention_forward
```

## Priority 3: Optimize Startup (1 hour)

### Problem 1: Lattice Cache Initialization on Startup

**File**: `app/main.c`

**Find**:
```c
initialize_crystalline_lattice_cache(200000);
```

**Change to**:
```c
// Defer lattice cache initialization - only initialize when needed
// initialize_crystalline_lattice_cache(200000);
```

**Add lazy initialization** in the function that actually uses it.

### Problem 2: Thread Pool Creation on Startup

**File**: `app/main.c` or `app/cllm_integration.c`

**Find**:
```c
cllm_training_parallel_init(3);
```

**Move to**: `app/ui/tabs/tab_training.c` in the START TRAINING button handler.

**Change**:
```c
// In tab_training.c, when START TRAINING clicked:
if (!training_thread_pool_initialized) {
    cllm_training_parallel_init(3);
    training_thread_pool_initialized = true;
}
```

## Testing Procedure

### Test 1: Diagnostics
```bash
cd ~/code/math/crystalline
git pull
make clean && make && make app
app/hyper_prime_spiral
```

1. Go to Training tab
2. Select large_corpus.txt
3. Click START TRAINING
4. Check console output for:
   - Weight magnitudes (should be non-zero, ~0.01-0.1)
   - Gradient magnitudes (should be non-zero, ~1e-4 to 1e-2)
   - Loss values

### Test 2: After Fixing Attention
```bash
# Same as above
# Expected: Loss should decrease from 9.2 to ~7.5 in first epoch
```

### Test 3: After Optimizing Startup
```bash
# Restart app
# Expected: Startup time < 2 seconds
# Expected: No UI freezing
```

## Expected Outcomes

### After Diagnostics
- **If weights are zeros**: Need to fix initialization
- **If gradients are zeros**: Need to fix backward pass
- **If gradients are huge (>1.0)**: Need gradient clipping
- **If gradients are tiny (<1e-6)**: Need to increase learning rate

### After Fixing Attention
- Loss should decrease: 9.2 → 7.5 → 6.0 → 5.0 over 10 epochs
- Generation should produce varied tokens, not repetition
- Model should learn relationships between words

### After Optimizing Startup
- Startup time: 5s → 1s
- No UI unresponsiveness
- Training still works correctly

## Implementation Order

1. ✅ Add diagnostics (30 min)
2. ✅ Test and analyze output (15 min)
3. ✅ Fix attention forward (2 hours)
4. ✅ Test training (30 min)
5. ⏳ Optimize startup (1 hour)
6. ⏳ Final testing (30 min)

**Total time**: ~5 hours

## Next Steps

I will now implement these fixes in order.