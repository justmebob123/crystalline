# EMERGENCY FIX - Wiring ALL Unwired Algorithms

## IMMEDIATE ACTION - Phase 1: Wire Type-Compatible Algorithms (NOW)

### 1.1 Wire Loss Functions into Training [COMPLETE]
- [x] Add algorithms/include/loss_functions.h to cllm_training.c
- [x] Replace cllm_compute_loss() with loss_cross_entropy()
- [x] Add LossConfig to CLLMTraining structure
- [x] Update loss computation in training loop
- [x] Build successful - no errors
- [ ] Test and verify correctness
- [ ] Benchmark speedup

### 1.2 Wire Optimizers into Training
- [ ] Add algorithms/include/optimizers.h to cllm_training.c
- [ ] Replace inline SGD with optimizer_step()
- [ ] Add OptimizerState to CLLMTraining structure
- [ ] Support Adam, RMSprop, Momentum
- [ ] Add learning rate scheduling
- [ ] Test convergence

### 1.3 Wire Gradient Buffer into Training
- [ ] Add algorithms/include/backprop.h to cllm_training.c
- [ ] Replace inline gradient accumulation with GradientBuffer
- [ ] Add gradient clipping and validation
- [ ] Test numerical stability

## Phase 2: Create Double-Precision Versions (Like SIMD Fix)

### 2.1 NTT Attention Double Version
- [ ] Create ntt_attention_forward_double() in algorithms/src/ntt_attention.c
- [ ] Use __m256d for AVX2 double operations
- [ ] Add to algorithms/include/ntt_attention.h
- [ ] Create cllm_attention_ntt_forward_double() wrapper
- [ ] Wire into cllm_attention_forward()

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