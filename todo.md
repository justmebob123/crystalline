# EMERGENCY FIX - Wiring ALL Unwired Algorithms

## Phase 1: Wire Type-Compatible Algorithms [COMPLETE ✓]

### 1.1 Wire Loss Functions into Training [COMPLETE ✓]
- [x] Add algorithms/include/loss_functions.h to cllm_training.c
- [x] Replace cllm_compute_loss() with loss_cross_entropy()
- [x] Add LossConfig to CLLMTraining structure
- [x] Update loss computation in training loop
- [x] Build successful - no errors
- [x] Committed and pushed to GitHub (1827e72)

### 1.2 Wire Optimizers into Training [COMPLETE ✓]
- [x] Add algorithms/include/optimizers.h to cllm_training.c
- [x] Add OptimizerState to CLLMTraining structure
- [x] Initialize Adam optimizer in training init
- [x] Build successful
- [x] Committed and pushed

### 1.3 Wire Gradient Buffer into Training [COMPLETE ✓]
- [x] Add algorithms/include/backprop.h to cllm_training.c
- [x] Add GradientBuffer to CLLMTraining structure
- [x] Initialize gradient buffer in training init
- [x] Build successful
- [x] Committed and pushed

## Phase 2: Create Double-Precision Versions (Like SIMD Fix) [IN PROGRESS]

### 2.1 NTT Attention Double Version [COMPLETE ✓]
- [x] Create ntt_attention_forward_double() in algorithms/src/ntt_attention.c
- [x] Add ntt_attention_single_head_double() and ntt_attention_multi_head_double()
- [x] Add to algorithms/include/ntt_attention.h
- [x] Build successful - no errors
- [x] Committed and pushed (8c4da1a)
- [ ] Wire into cllm_attention_forward() - NEXT STEP
- [ ] Test and benchmark (expected: 10-100x speedup)

### 2.2 Angular Attention Double Version
- [ ] Create angular_attention_forward_double()
- [ ] Use __m256d for AVX2 double operations
- [ ] Wire into attention computation

### 2.3 Lattice Embeddings Double Version
- [ ] Create lattice_embeddings_init_geometric_double()
- [ ] Wire into model initialization

## Phase 3: Complete Integration
- [ ] Test all wired algorithms
- [ ] Benchmark performance improvements
- [ ] Verify model quality
- [ ] Update documentation

## Phase 4: Continue Depth-23 Audit
- [ ] Check for other unwired functions
- [ ] Verify all algorithm integrations
- [ ] Fix any remaining issues