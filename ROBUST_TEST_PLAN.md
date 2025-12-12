# Robust Pipeline Test Plan

## Objectives

Based on Master Plan requirements and user feedback, create a substantially more robust test that:

1. **Investigates vocabulary save warning**
2. **Adds more training data** (substantial dataset)
3. **Tunes hyperparameters for better convergence**
4. **Adds validation metrics**
5. **Tests at speed** (leveraging 2-core performance)

## Test Configuration

### Model Architecture
- **Layers**: 8 (as before)
- **Embedding Dimension**: 256
- **Attention Heads**: 8
- **Vocabulary Size**: 10,000 (increased from 5,000)

### Training Configuration
- **Epochs**: 10 (increased from 5)
- **Batch Size**: 8 (reduced from 16 for more batches)
- **Sequence Length**: 32 (reduced from 64 for more batches)
- **Learning Rate**: 0.0001 (same)
- **Threads**: 2 (auto-detected)

### Expected Improvements
- More batches per epoch = better gradient updates
- More epochs = better convergence
- Larger vocabulary = better token coverage
- Smaller batch/sequence = more frequent updates

## Phase 1: Create Substantial Training Dataset

### Strategy
Combine multiple existing datasets to create a robust corpus:

1. **Existing Data Files**:
   - data/training/large_corpus.txt (6.3KB)
   - data/training/better_training.txt (2.2KB)
   - data/raw/sample1.txt (1.4KB)
   - data/raw/sample2.txt (1.5KB)
   - data/raw/sample3.txt (1.4KB)

2. **Generate Additional Content**:
   - Create synthetic training data based on AI/ML topics
   - Ensure diverse vocabulary
   - Target: 50-100KB total dataset

### Implementation
```bash
# Create robust test directory
mkdir -p robust_test/training_data

# Copy existing files
cp data/training/*.txt robust_test/training_data/
cp data/raw/sample*.txt robust_test/training_data/

# Generate additional training data
# (Will create 5-10 additional files with AI/ML content)
```

## Phase 2: Investigate Vocabulary Save Warning

### Root Cause Analysis
The warning "Failed to save vocabulary" appears but the file is created. Investigate:

1. Check `cllm_save_vocab()` function return value
2. Verify file write operations
3. Check error handling logic
4. Ensure proper file closure

### Fix Strategy
1. Locate the function in source code
2. Add debug output to identify exact failure point
3. Fix the issue
4. Verify fix with test

## Phase 3: Tune Hyperparameters

### Current Issues
- Loss stuck at 4.2079 (not decreasing)
- Only 1 batch per epoch (insufficient gradient updates)

### Tuning Strategy

#### Batch Configuration
```
Old: batch=16, seq_len=64 → 1 batch
New: batch=8, seq_len=32 → ~7 batches (estimated)
```

#### Learning Rate Schedule
- Start: 0.0001
- Consider: Warmup + decay schedule
- Monitor: Loss per epoch

#### Validation Split
- Training: 80% of data
- Validation: 20% of data
- Track both training and validation loss

## Phase 4: Add Validation Metrics

### Metrics to Track

1. **Training Metrics**:
   - Loss per epoch
   - Loss per batch
   - Gradient norms
   - Learning rate

2. **Validation Metrics**:
   - Validation loss
   - Perplexity
   - Token accuracy

3. **Performance Metrics**:
   - Tokens per second
   - Batches per second
   - Time per epoch

### Implementation
Create validation dataset and compute metrics after each epoch.

## Phase 5: Comprehensive Testing

### Test Sequence

1. **Build and Verify**:
   ```bash
   make clean && make
   # Verify zero warnings
   ```

2. **Train with Robust Configuration**:
   ```bash
   ./tools/cllm train \
     -d robust_test/training_data \
     -v 10000 \
     -e 256 \
     -l 8 \
     -H 8 \
     -E 10 \
     -b 8 \
     -s 32 \
     -r 0.0001 \
     -t 0 \
     -c robust_test/checkpoints
   ```

3. **Test Inference Thoroughly**:
   - Test 10+ different prompts
   - Measure generation quality
   - Check for consistency
   - Verify no crashes

4. **Validate Metrics**:
   - Verify loss decreases over epochs
   - Check validation metrics
   - Ensure no overfitting

## Expected Results

### Training
- **Time**: ~2-5 minutes (with 2 cores)
- **Batches per Epoch**: 5-10 (vs 1 before)
- **Loss Behavior**: Should decrease over epochs
- **Final Loss**: <3.0 (vs 4.2079 before)

### Inference
- **Quality**: Significantly better than before
- **Coherence**: Some structure visible
- **Consistency**: Stable across prompts

### Validation
- **Validation Loss**: Should track training loss
- **No Overfitting**: Val loss shouldn't diverge
- **Metrics**: All tracked and reported

## Success Criteria

1. ✅ Vocabulary save warning fixed
2. ✅ Substantial training dataset created (50-100KB)
3. ✅ Loss decreases over epochs
4. ✅ More batches per epoch (>5)
5. ✅ Validation metrics implemented
6. ✅ Inference quality improved
7. ✅ No crashes or errors
8. ✅ Progress bar working throughout

## Timeline

With 2 cores and optimized pipeline:
- Dataset creation: 2 minutes
- Fix vocabulary warning: 5 minutes
- Training (10 epochs): 3-5 minutes
- Inference testing: 2 minutes
- **Total**: ~15-20 minutes

## Next Steps

1. Create substantial training dataset
2. Fix vocabulary save warning
3. Implement validation metrics
4. Run robust training test
5. Validate results
6. Document findings