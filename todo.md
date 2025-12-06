# CLLM Training & Inference - COMPREHENSIVE TESTING PLAN

## Current Status: 🔬 DEPTH-17 BIDIRECTIONAL ANALYSIS IN PROGRESS
- System rebuilt: ✅ CLEAN BUILD COMPLETED
- Code pushed to GitHub: ✅ ALL FIXES COMMITTED
- Float→Double conversion: ✅ VERIFIED (entire pipeline uses double)
- Optimizer gradient direction: ✅ VERIFIED (all use -= correctly)
- Next: FULL PIPELINE TESTING with valgrind/gdb/strace

## Phase 1: System Verification ✅
- [x] Read master plan and status reports
- [x] Verify code is pushed to GitHub
- [x] Clean rebuild entire system
- [x] Verify optimizer uses correct gradient direction (-)
- [x] Check for float usage in critical paths

## Phase 2: Deep Code Analysis 🔄
- [ ] Grep ALL float usage and document locations
- [ ] Analyze SIMD code for double precision support
- [ ] Check forward pass for type consistency
- [ ] Check backward pass for type consistency
- [ ] Verify loss computation uses double
- [ ] Verify all layer operations use double

## Phase 3: Training Pipeline Testing 🔄
- [ ] Train tiny model (100 vocab, 32 dim, 1 layer, 2 epochs)
- [ ] Monitor loss - MUST decrease
- [ ] Check for NaN errors
- [ ] Verify gradient magnitudes
- [ ] Train small model (200 vocab, 64 dim, 2 layers, 5 epochs)
- [ ] Train medium model (500 vocab, 128 dim, 4 layers, 10 epochs)
- [ ] Train FULL model on all_training/full_corpus.txt (617 lines)

## Phase 4: Valgrind Analysis 🔄
- [ ] Run valgrind on tiny model training
- [ ] Check for memory leaks
- [ ] Check for invalid reads/writes
- [ ] Check for uninitialized values
- [ ] Run valgrind on full model training
- [ ] Document all memory issues found

## Phase 5: GDB Analysis 🔄
- [ ] Set breakpoints in forward pass
- [ ] Verify hidden_states values
- [ ] Check attention computation
- [ ] Check feedforward computation
- [ ] Set breakpoints in backward pass
- [ ] Verify gradient computation
- [ ] Check optimizer weight updates

## Phase 6: Strace Analysis 🔄
- [ ] Run strace on training
- [ ] Check file I/O patterns
- [ ] Check memory allocation patterns
- [ ] Identify performance bottlenecks
- [ ] Document system call usage

## Phase 7: Inference Testing 🔄
- [ ] Test inference with trained model
- [ ] Input: "The sky is blue"
- [ ] Verify output is meaningful
- [ ] Test with various prompts
- [ ] Check token generation quality
- [ ] Verify no NaN errors
- [ ] Test temperature settings
- [ ] Test top-k/top-p sampling

## Phase 8: End-to-End Validation 🔄
- [ ] Train model on full dataset (617 lines)
- [ ] Save model checkpoint
- [ ] Load model and test inference
- [ ] Verify model can answer: "Is the sky blue?"
- [ ] Test with 10 different prompts
- [ ] Document output quality
- [ ] Measure perplexity
- [ ] Calculate accuracy metrics

## Critical Issues to Investigate

### Issue 1: Loss Increases Despite Correct Optimizer
**Status**: UNRESOLVED
**Evidence**: Loss increases even with -= in optimizer
**Hypothesis**: 
1. Loss computation might be wrong
2. Forward pass might have bugs
3. Gradient signs might be inverted somewhere
4. Learning rate might be too high
**Action**: Deep analysis of loss computation and gradient flow

### Issue 2: Model Generates Mostly token_0
**Status**: UNRESOLVED
**Evidence**: Inference outputs are not meaningful
**Hypothesis**:
1. Model not trained properly (loss increases)
2. Embeddings not initialized correctly
3. Softmax temperature too low
4. Sampling strategy broken
**Action**: Test with properly trained model

### Issue 3: Float Usage in SIMD Code
**Status**: IDENTIFIED
**Evidence**: 623 float usages found in codebase
**Locations**: cllm_simd_utils.c, cllm_lattice_embed.c
**Impact**: May cause precision issues in some operations
**Action**: Audit and convert to double where needed

## Testing Strategy

### Minimal Test (2 minutes)
```bash
./tools/cllm train \
  --data data/tiny.txt \
  --vocab-size 100 \
  --embed-dim 32 \
  --num-layers 1 \
  --num-heads 2 \
  --epochs 2 \
  --learning-rate 0.0001 \
  --output checkpoints/tiny_model.cllm
```

### Small Test (5 minutes)
```bash
./tools/cllm train \
  --data data/test.txt \
  --vocab-size 200 \
  --embed-dim 64 \
  --num-layers 2 \
  --num-heads 4 \
  --epochs 5 \
  --learning-rate 0.0001 \
  --output checkpoints/small_model.cllm
```

### Full Test (10-15 minutes)
```bash
./tools/cllm train \
  --data data/all_training/full_corpus.txt \
  --vocab-size 500 \
  --embed-dim 128 \
  --num-layers 4 \
  --num-heads 8 \
  --epochs 20 \
  --learning-rate 0.0001 \
  --output checkpoints/full_model.cllm
```

### Valgrind Test
```bash
valgrind --leak-check=full --show-leak-kinds=all \
  --track-origins=yes --verbose \
  ./tools/cllm train \
  --data data/tiny.txt \
  --vocab-size 100 \
  --embed-dim 32 \
  --num-layers 1 \
  --num-heads 2 \
  --epochs 2 \
  --learning-rate 0.0001 \
  --output checkpoints/valgrind_test.cllm
```

### GDB Test
```bash
gdb --args ./tools/cllm train \
  --data data/tiny.txt \
  --vocab-size 100 \
  --embed-dim 32 \
  --num-layers 1 \
  --num-heads 2 \
  --epochs 2 \
  --learning-rate 0.0001 \
  --output checkpoints/gdb_test.cllm
```

## Success Criteria

### Training Success
- ✅ Loss DECREASES over epochs
- ✅ No NaN errors
- ✅ Gradients remain stable (< 1.0)
- ✅ Model saves successfully
- ✅ No memory leaks (valgrind clean)

### Inference Success
- ✅ Model loads without errors
- ✅ Generates tokens (not all token_0)
- ✅ Output is somewhat coherent
- ✅ Can respond to simple prompts
- ✅ No NaN in outputs

### Quality Metrics
- Perplexity < 100 (for tiny dataset)
- At least 50% of outputs are non-token_0
- Can generate 2-3 word responses
- Responses relate to training data

## Files to Monitor
- src/ai/cllm_training.c - Training loop
- src/ai/cllm_inference.c - Inference pipeline
- src/ai/cllm_optimizer_wrapper.c - Optimizer
- src/ai/cllm_create.c - Model initialization
- src/ai/cllm_format.c - Model save/load

## Next Actions (Priority Order)
1. ⚠️ CRITICAL: Run minimal test and verify loss decreases
2. ⚠️ CRITICAL: If loss still increases, deep dive into loss computation
3. Run valgrind on minimal test
4. Run gdb on minimal test
5. Fix any issues found
6. Scale up to small test
7. Scale up to full test
8. Test inference thoroughly
9. Document all findings
10. Commit and push final fixes