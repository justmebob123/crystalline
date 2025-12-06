# COMPLETE PIPELINE TESTING AND FIXING - NO SHORTCUTS

## CRITICAL: Read Master Plan First
- [x] Read MASTER_PLAN.md
- [x] Understand objectives and architecture
- [x] No stubs, no simplifications, no shortcuts

## CRITICAL BUG FOUND: Loss is NaN, Gradients are Zero

Training is running but:
- Loss = -nan
- All gradients = 0.00e+00
- Model is not learning

This is caused by the wired loss function from algorithms layer!

## Phase 1: Fix NaN Loss and Zero Gradients [URGENT]

### 1.1 Analyze Application UI Integration
- [ ] Check app/ui/tabs/tab_training.c integration with training pipeline
- [ ] Check app/ui/tabs/tab_llm.c integration with inference
- [ ] Verify all UI callbacks are wired correctly
- [ ] Check if training status updates work
- [ ] Check if inference results display correctly

### 1.2 Analyze Training Thread Integration
- [ ] Check app/training_thread.c integration
- [ ] Verify training thread uses correct training functions
- [ ] Check if progress updates work
- [ ] Verify checkpoint saving works
- [ ] Check error handling

### 1.3 Analyze CLI Tool Integration
- [ ] Check tools/cllm.c train command implementation
- [ ] Check tools/cllm.c infer command implementation
- [ ] Verify all CLI options work
- [ ] Check if output is correct
- [ ] Test error handling

## Phase 2: Wire ALL Remaining Double-Precision Functions

### 2.1 Complete NTT Attention Wiring
- [ ] Modify cllm_attention_forward() to use ntt_attention_forward_double()
- [ ] Add adaptive selection based on sequence length
- [ ] Test with different sequence lengths
- [ ] Verify correctness
- [ ] Benchmark speedup

### 2.2 Create and Wire Angular Attention Double
- [ ] Create angular_attention_forward_double() in algorithms
- [ ] Add to algorithms/include/angular_attention.h
- [ ] Wire into attention computation
- [ ] Test correctness
- [ ] Benchmark

### 2.3 Create and Wire Lattice Embeddings Double
- [ ] Create lattice_embeddings_init_geometric_double() in algorithms
- [ ] Add to algorithms/include/lattice_embeddings.h
- [ ] Wire into model initialization
- [ ] Test correctness
- [ ] Verify geometric structure

### 2.4 Complete Optimizer Integration
- [ ] Wire optimizer_step() into cllm_optimizer_step()
- [ ] Replace ALL inline SGD code
- [ ] Test Adam, RMSprop, Momentum
- [ ] Verify convergence
- [ ] Benchmark

### 2.5 Complete Gradient Buffer Integration
- [ ] Wire gradient_buffer_accumulate() into training loop
- [ ] Replace ALL inline gradient accumulation
- [ ] Add gradient clipping
- [ ] Add gradient validation
- [ ] Test numerical stability

## Phase 3: Complete End-to-End Training Test

### 3.1 Prepare Training Data
- [ ] List all training data files in data/
- [ ] Concatenate into single training file
- [ ] Verify data format
- [ ] Check data size

### 3.2 Train Complete Model
- [ ] Create model with reasonable size (vocab=1000, embed=128, layers=4)
- [ ] Train for 10 epochs on ALL training data
- [ ] Monitor loss convergence
- [ ] Save checkpoints
- [ ] Verify model saves correctly

### 3.3 Test Inference
- [ ] Load trained model
- [ ] Test with simple prompts
- [ ] Test with complex prompts
- [ ] Verify outputs are meaningful
- [ ] Check if responses relate to training data

## Phase 4: Valgrind Analysis

### 4.1 Memory Leak Detection
- [ ] Run training under valgrind
- [ ] Check for memory leaks
- [ ] Fix all leaks found
- [ ] Re-run until clean

### 4.2 Invalid Memory Access
- [ ] Check for invalid reads
- [ ] Check for invalid writes
- [ ] Fix all issues
- [ ] Re-run until clean

## Phase 5: GDB Analysis

### 5.1 Crash Detection
- [ ] Run training under gdb
- [ ] Catch any segfaults
- [ ] Analyze stack traces
- [ ] Fix all crashes

### 5.2 Logic Verification
- [ ] Set breakpoints in critical functions
- [ ] Verify gradient computation
- [ ] Verify weight updates
- [ ] Verify loss computation

## Phase 6: Strace Analysis

### 6.1 System Call Analysis
- [ ] Run training under strace
- [ ] Check file operations
- [ ] Check memory operations
- [ ] Identify bottlenecks

## Phase 7: Depth-17 Bidirectional Analysis (If Needed)

### 7.1 Forward Pass Analysis
- [ ] Trace data flow through forward pass
- [ ] Verify all computations
- [ ] Check for numerical issues
- [ ] Verify activations

### 7.2 Backward Pass Analysis
- [ ] Trace gradient flow through backward pass
- [ ] Verify all gradient computations
- [ ] Check for zero gradients
- [ ] Verify gradient accumulation

### 7.3 Optimizer Analysis
- [ ] Verify weight updates
- [ ] Check learning rate
- [ ] Verify momentum/Adam state
- [ ] Check for NaN/Inf

## Phase 8: Final Validation

### 8.1 Quality Tests
- [ ] Train model on "sky is blue" data
- [ ] Test inference with "is the sky blue?"
- [ ] Verify meaningful response
- [ ] Test multiple prompts

### 8.2 Stress Tests
- [ ] Train with maximum data
- [ ] Train with maximum epochs
- [ ] Test with long sequences
- [ ] Test with large vocabulary

### 8.3 Performance Tests
- [ ] Benchmark training speed
- [ ] Benchmark inference speed
- [ ] Compare with baseline
- [ ] Verify speedup claims

## Success Criteria

- [ ] Training completes without crashes
- [ ] Loss decreases consistently
- [ ] Model saves and loads correctly
- [ ] Inference produces meaningful outputs
- [ ] No memory leaks
- [ ] No invalid memory access
- [ ] All algorithms properly wired
- [ ] Performance improvements verified