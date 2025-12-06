# DEPTH-17 BIDIRECTIONAL ANALYSIS - FIXING NaN LOSS

## CRITICAL ISSUE
- Loss starts at ~7.19 (working)
- Loss becomes NaN during training
- All gradients become zero
- This is a NUMERICAL STABILITY bug in forward/backward pass

## Phase 1: Identify Where NaN Originates [STARTING NOW]

### 1.1 Check Forward Pass for NaN
- [ ] Add NaN checks after each operation in forward pass
- [ ] Check embeddings for NaN
- [ ] Check attention outputs for NaN
- [ ] Check feedforward outputs for NaN
- [ ] Check logits for NaN

### 1.2 Check Backward Pass for NaN
- [ ] Add NaN checks in gradient computation
- [ ] Check if gradients become NaN
- [ ] Check if weights become NaN
- [ ] Identify exact operation causing NaN

### 1.3 Check Loss Computation for NaN
- [ ] Verify softmax doesn't produce NaN
- [ ] Check for log(0) or log(negative)
- [ ] Check for division by zero
- [ ] Verify numerical stability

## Phase 2: Run Under Valgrind

### 2.1 Memory Leak Detection
- [ ] Run training under valgrind --leak-check=full
- [ ] Fix all memory leaks
- [ ] Re-run until clean

### 2.2 Invalid Memory Access
- [ ] Check for buffer overflows
- [ ] Check for use-after-free
- [ ] Fix all invalid accesses

## Phase 3: Run Under GDB

### 3.1 Set Breakpoints
- [ ] Break on NaN detection
- [ ] Break in loss computation
- [ ] Break in gradient computation
- [ ] Examine variables when NaN occurs

### 3.2 Trace Execution
- [ ] Step through forward pass
- [ ] Step through backward pass
- [ ] Identify exact line causing NaN

## Phase 4: Fix Numerical Stability

### 4.1 Add Gradient Clipping
- [ ] Clip gradients to prevent explosion
- [ ] Add checks for inf/nan
- [ ] Normalize gradients if needed

### 4.2 Fix Softmax Stability
- [ ] Ensure max subtraction works
- [ ] Add epsilon to prevent log(0)
- [ ] Clamp values to safe range

### 4.3 Fix Weight Initialization
- [ ] Check if weights are initialized correctly
- [ ] Verify no NaN in initial weights
- [ ] Check embedding initialization

## Phase 5: Test With Simpler Model

### 5.1 Minimal Test
- [ ] Create tiny model (vocab=10, embed=8, layers=1)
- [ ] Train on tiny data (10 tokens)
- [ ] Verify loss decreases
- [ ] Verify no NaN

### 5.2 Gradual Increase
- [ ] Increase model size gradually
- [ ] Test at each step
- [ ] Find where NaN starts occurring

## Phase 6: Complete Pipeline Test

### 6.1 Train Working Model
- [ ] Use configuration that doesn't produce NaN
- [ ] Train for 10 epochs
- [ ] Verify loss decreases consistently
- [ ] Save model

### 6.2 Test Inference
- [ ] Load trained model
- [ ] Test with "sky is blue" prompts
- [ ] Verify meaningful outputs
- [ ] Test multiple prompts

## Success Criteria
- [ ] Loss decreases consistently (no NaN)
- [ ] Gradients are non-zero
- [ ] Model trains successfully
- [ ] Inference produces meaningful outputs
- [ ] No memory leaks
- [ ] No crashes