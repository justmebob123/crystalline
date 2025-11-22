# Weight Gradient Accumulation Implementation Plan

## Current Status

✅ **COMPLETED:**
- Gradient flow through all layers (embeddings, layer norm, feed-forward, attention)
- Input gradients computed correctly
- Backward pass infrastructure in place

⚠️ **MISSING:**
- Weight gradient accumulation for attention layers
- Weight gradient accumulation for feed-forward layers
- Weight gradient accumulation for layer normalization
- Integration with optimizer for weight updates

## Problem Statement

Currently, the backward pass computes gradients w.r.t. inputs but does NOT accumulate gradients for the weight matrices. This means:
- We can compute how the loss changes w.r.t. layer inputs
- We CANNOT update the weights because we don't have weight gradients
- Training only updates embeddings, not transformer layers

## Architecture Overview

### Current Training State Structure
```c
typedef struct {
    CLLMModel* model;
    float* gradients;           // Currently only used for embeddings
    float* optimizer_state;     // Adam m and v
    // ... other fields
} CLLMTraining;
```

### Model Weight Structure
```c
typedef struct {
    Embeddings embeddings;              // vocab_size x embedding_dim
    AttentionLayer* attention_layers;   // num_layers layers
    FeedForwardLayer* ff_layers;        // num_layers layers
    CLLMLayerNorm* layer_norms;         // num_layers layers
} CLLMModel;
```

### Attention Layer Weights
```c
typedef struct {
    float* query_lattice;   // num_heads x head_dim x head_dim
    float* key_lattice;     // num_heads x head_dim x head_dim
    float* value_lattice;   // num_heads x head_dim x head_dim
} AttentionLayer;
```

### Feed-Forward Layer Weights
```c
typedef struct {
    float* w1_lattice;  // input_dim x hidden_dim
    float* w2_lattice;  // hidden_dim x output_dim
    float* bias1;       // hidden_dim
    float* bias2;       // output_dim
} FeedForwardLayer;
```

### Layer Normalization Weights
```c
typedef struct {
    float* gamma;  // dim
    float* beta;   // dim
} CLLMLayerNorm;
```

## Implementation Strategy

### Phase 1: Extend Training State

Add separate gradient buffers for each layer type:

```c
typedef struct {
    CLLMModel* model;
    
    // Existing
    float* gradients;           // Embedding gradients
    float* optimizer_state;     // Adam state
    
    // NEW: Layer-specific gradient buffers
    struct {
        float* query_lattice;   // Gradients for Q weights
        float* key_lattice;     // Gradients for K weights
        float* value_lattice;   // Gradients for V weights
    }* attention_grads;         // Array of num_layers
    
    struct {
        float* w1_lattice;      // Gradients for W1
        float* w2_lattice;      // Gradients for W2
        float* bias1;           // Gradients for b1
        float* bias2;           // Gradients for b2
    }* ff_grads;                // Array of num_layers
    
    struct {
        float* gamma;           // Gradients for gamma
        float* beta;            // Gradients for beta
    }* ln_grads;                // Array of num_layers
    
} CLLMTraining;
```

### Phase 2: Modify Backward Functions

Update backward pass functions to accept gradient buffers:

```c
// Current signature
void cllm_attention_backward(AttentionLayer* layer, float* input,
                            float* grad_output, float* grad_input, int seq_len);

// NEW signature
void cllm_attention_backward(AttentionLayer* layer, float* input,
                            float* grad_output, float* grad_input,
                            float* grad_query_weights,    // NEW
                            float* grad_key_weights,      // NEW
                            float* grad_value_weights,    // NEW
                            int seq_len);
```

### Phase 3: Implement Weight Gradient Computation

#### Attention Weight Gradients

For each position and head, compute:

```c
// Gradient for query weights: dL/dW_Q = input^T × dL/dQ
for (int pos = 0; pos < seq_len; pos++) {
    for (uint32_t h = 0; h < num_heads; h++) {
        float* input_vec = &input[pos * embedding_dim + h * head_dim];
        float* grad_q = &grad_queries[pos * embedding_dim + h * head_dim];
        
        for (uint32_t d = 0; d < head_dim; d++) {
            for (uint32_t i = 0; i < head_dim; i++) {
                size_t weight_idx = h * head_dim * head_dim + d * head_dim + i;
                grad_query_weights[weight_idx] += input_vec[i] * grad_q[d];
            }
        }
    }
}

// Similar for key and value weights
```

#### Feed-Forward Weight Gradients

```c
// Gradient for W1: dL/dW1 = input^T × dL/dhidden
for (uint32_t i = 0; i < input_dim; i++) {
    for (uint32_t h = 0; h < hidden_dim; h++) {
        grad_w1[i * hidden_dim + h] += input[i] * grad_hidden[h];
    }
}

// Gradient for W2: dL/dW2 = hidden^T × dL/doutput
for (uint32_t h = 0; h < hidden_dim; h++) {
    for (uint32_t o = 0; o < output_dim; o++) {
        grad_w2[h * output_dim + o] += hidden[h] * grad_output[o];
    }
}
```

#### Layer Norm Weight Gradients

Already implemented in `cllm_layer_norm_backward`:

```c
for (uint32_t i = 0; i < dim; i++) {
    float normalized = (input[i] - mean) * inv_std;
    grad_gamma[i] = grad_output[i] * normalized;
    grad_beta[i] = grad_output[i];
}
```

### Phase 4: Integrate with Optimizer

Modify `cllm_adam_step` to update all weights:

```c
void cllm_adam_step(CLLMTraining* training, float learning_rate) {
    // Update embeddings (existing)
    update_embeddings(training, learning_rate);
    
    // NEW: Update attention layers
    for (int layer = 0; layer < num_layers; layer++) {
        update_attention_weights(training, layer, learning_rate);
    }
    
    // NEW: Update feed-forward layers
    for (int layer = 0; layer < num_layers; layer++) {
        update_ff_weights(training, layer, learning_rate);
    }
    
    // NEW: Update layer norm
    for (int layer = 0; layer < num_layers; layer++) {
        update_ln_weights(training, layer, learning_rate);
    }
}
```

## Implementation Steps

### Step 1: Allocate Gradient Buffers
- [ ] Modify `cllm_training_init` to allocate layer-specific gradient buffers
- [ ] Calculate total memory needed for all gradients
- [ ] Initialize all gradient buffers to zero

### Step 2: Update Backward Pass Signatures
- [ ] Add weight gradient parameters to `cllm_attention_backward`
- [ ] Add weight gradient parameters to `cllm_feedforward_backward`
- [ ] Ensure `cllm_layer_norm_backward` uses weight gradients

### Step 3: Implement Weight Gradient Accumulation
- [ ] Implement attention weight gradient computation
- [ ] Implement feed-forward weight gradient computation
- [ ] Verify layer norm weight gradient computation

### Step 4: Connect to Optimizer
- [ ] Extend `cllm_adam_step` to update all weights
- [ ] Implement separate Adam state for each weight type
- [ ] Add gradient zeroing between batches

### Step 5: Testing
- [ ] Test gradient accumulation with simple inputs
- [ ] Verify weight updates change model behavior
- [ ] Check training loss decreases over iterations

## Memory Requirements

For a model with:
- `num_layers = 6`
- `embedding_dim = 512`
- `num_heads = 8`
- `head_dim = 64`
- `ff_hidden_dim = 2048`

**Gradient Memory:**
- Attention: `6 × 3 × (8 × 64 × 64) = 589,824 floats = 2.36 MB`
- Feed-forward: `6 × (512 × 2048 + 2048 × 512 + 2048 + 512) = 12,595,200 floats = 50.38 MB`
- Layer norm: `6 × (512 + 512) = 6,144 floats = 24 KB`
- **Total: ~53 MB**

**Adam State (2x gradients):**
- **Total: ~106 MB**

**Grand Total: ~159 MB** (reasonable for training)

## Performance Considerations

1. **Memory Access Patterns:**
   - Weight gradients accumulate across sequence positions
   - Use cache-friendly access patterns
   - Consider tiling for large matrices

2. **Parallelization:**
   - Each layer's gradients can be computed independently
   - Each head's gradients can be computed independently
   - Consider OpenMP for multi-threading

3. **Numerical Stability:**
   - Use gradient clipping to prevent exploding gradients
   - Check for NaN/Inf after gradient computation
   - Use mixed precision if needed

## Testing Strategy

### Unit Tests
1. Test weight gradient computation with known inputs
2. Verify gradient shapes match weight shapes
3. Check gradient accumulation across batches

### Integration Tests
1. Train on simple sequence (e.g., "A B C" → "B C D")
2. Verify loss decreases
3. Check weights change after updates

### Numerical Gradient Checking
1. Compute numerical gradients: `(f(w + ε) - f(w - ε)) / (2ε)`
2. Compare with analytical gradients
3. Verify relative error < 1e-5

## Next Steps

1. **Implement gradient buffer allocation** in `cllm_training_init`
2. **Update backward pass functions** to accumulate weight gradients
3. **Extend optimizer** to update all weights
4. **Test on simple examples** to verify correctness
5. **Optimize performance** if needed

---

**Status:** Ready for implementation
**Priority:** HIGH - Required for full model training
**Estimated Effort:** 4-6 hours
**Dependencies:** None (backward pass complete)