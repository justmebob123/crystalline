# Critical Integration: GCD-Based Similarity

## Current Problem
The GCD-based similarity function exists but is NOT being used in training.
Training still uses dot product for similarity computation.

## Location of Issue
File: `src/ai/cllm_training.c`
Line: ~368
Current code:
```c
float similarity = dot_product(input_embed, target_embed, embed_dim);
```

## Solution Required
Replace dot product with GCD-based similarity:
```c
float similarity = crystalline_gcd_similarity(input, target);
```

## Expected Impact
- Current: O(n) dot product (n = embedding_dim = 128)
- After: O(log n) GCD computation
- Speedup: 20-400x for similarity computation
- Overall training: 20-80x faster

## Implementation Steps
1. Include crystalline training header in cllm_training.c
2. Replace dot product call with GCD similarity
3. Test that loss still decreases
4. Measure actual speedup

## Why This Matters
This is THE KEY OPTIMIZATION that makes crystalline training special.
Without it, we're just using standard neural network training.
