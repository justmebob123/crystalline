# Full Pipeline Test Plan: Training and Inference

## Current System Status
- **Available Cores**: 2 (not 64)
- **Training Data**: data/training/large_corpus.txt (6.3KB, 218 lines)
- **Test Configuration**: 8 layers, 5 epochs
- **Threading**: 2 threads (auto-detected)

## Time Estimation

### With 2 Cores (Current Environment)
Based on the optimizations implemented:

1. **Vocabulary Building** (Parallel with 2 threads):
   - 3 files × ~2KB each = ~6KB total
   - With parallel processing: ~1-2 seconds

2. **Tokenization** (Parallel with 2 threads):
   - 3 files, parallel tokenization
   - Estimated: ~2-3 seconds

3. **Training** (8 layers, 5 epochs, 2 threads):
   - Small dataset (~6KB)
   - Batch size: 32, Sequence length: 128
   - With kissing spheres threading (2 threads)
   - Estimated: **5-10 minutes** (depends on batch count)

4. **Saving Model**: ~1 second

**Total Estimated Time: 6-12 minutes**

### With 8 Cores (Your Saturn Machine)
- Vocabulary Building: <1 second
- Tokenization: <1 second  
- Training: **2-4 minutes**
- Total: **2-5 minutes**

### With 64 Cores (Hypothetical)
- Would be limited by dataset size (too small to benefit)
- Training: **1-2 minutes**
- Total: **1-3 minutes**

## Test Plan

### Phase 1: Setup Test Environment
```bash
# Create test directory
mkdir -p test_pipeline
cd test_pipeline

# Copy training data
cp ../data/training/large_corpus.txt .
cp ../data/training/better_training.txt .
cp ../data/raw/sample1.txt .
```

### Phase 2: Train Model
```bash
../tools/cllm train \
  -d . \
  -v 5000 \
  -e 256 \
  -l 8 \
  -H 8 \
  -E 5 \
  -b 16 \
  -s 64 \
  -r 0.0001 \
  -t 0 \
  -c ./checkpoints
```

**Parameters Explained**:
- `-d .` - Training data directory (current dir with 3 files)
- `-v 5000` - Vocabulary size
- `-e 256` - Embedding dimension
- `-l 8` - **8 layers** (as requested)
- `-H 8` - 8 attention heads
- `-E 5` - **5 epochs** (as requested)
- `-b 16` - Batch size (reduced for small dataset)
- `-s 64` - Sequence length (reduced for small dataset)
- `-r 0.0001` - Learning rate
- `-t 0` - Auto-detect threads (will use 2)
- `-c ./checkpoints` - Checkpoint directory

### Phase 3: Test Inference
```bash
# Test with final model
../tools/cllm infer \
  -m ./checkpoints/final_model.cllm \
  -p "Artificial intelligence" \
  -n 50 \
  -t 0.8

# Test with different prompts
../tools/cllm infer \
  -m ./checkpoints/final_model.cllm \
  -p "Machine learning" \
  -n 50 \
  -t 0.8

../tools/cllm infer \
  -m ./checkpoints/final_model.cllm \
  -p "Neural networks" \
  -n 50 \
  -t 0.8
```

### Phase 4: Verify Results
1. Check model file exists and has reasonable size
2. Check vocabulary was saved correctly
3. Verify inference produces coherent output
4. Test multiple inference runs for consistency
5. Verify progress bar displayed correctly throughout

## Expected Outputs

### Training Output
```
╔══════════════════════════════════════════════════════════════════════════╗
║ CLLM Training Pipeline                            Overall: 0.0% ║
╠══════════════════════════════════════════════════════════════════════════╣
║ Phase 1/5: Loading Data Files                                            ║
║ [>                                                  ] 0/1                 ║
║ 0/1 | Time: 0s | ETA: calculating... | Speed: 0.00/s                     ║
╚══════════════════════════════════════════════════════════════════════════╝

... (progress through all phases) ...

╔══════════════════════════════════════════════════════════════════════════╗
║ CLLM Training Pipeline                            Overall: 100.0% ║
╠══════════════════════════════════════════════════════════════════════════╣
║ Phase 5/5: Complete                                                      ║
║ [==================================================] 100.0%               ║
║ Complete                                                                  ║
╚══════════════════════════════════════════════════════════════════════════╝
```

### Inference Output
```
=== CLLM Inference ===

Loading model from: ./checkpoints/final_model.cllm
✓ Model loaded

✓ Vocabulary loaded from: ./checkpoints/vocab.txt

Prompt: Artificial intelligence
Generating...

Generated (50 tokens):
[Generated text based on training data]
```

## Success Criteria

### Training
- ✅ All 5 phases complete successfully
- ✅ Progress bar displays correctly throughout
- ✅ No segmentation faults or crashes
- ✅ Model file created (~several MB)
- ✅ Vocabulary file created
- ✅ Loss decreases over epochs
- ✅ All 5 epochs complete
- ✅ Checkpoints saved correctly

### Inference
- ✅ Model loads successfully
- ✅ Vocabulary loads correctly
- ✅ Generates text without errors
- ✅ Output is coherent (based on training data)
- ✅ Multiple inference runs work consistently
- ✅ Different prompts produce different outputs

## Known Limitations

### Current Environment (2 Cores)
- Training will take 6-12 minutes (acceptable for testing)
- Limited parallelization benefit
- Small dataset may not fully showcase performance

### Recommendations
1. **For Quick Testing**: Use current 2-core environment (6-12 min)
2. **For Performance Testing**: Use Saturn machine with 8 cores (2-5 min)
3. **For Production**: Use larger dataset with more files

## Risk Assessment

### Low Risk
- Code has been tested and builds with zero warnings
- All optimizations are production-ready
- Thread-safe implementations verified

### Medium Risk
- Small dataset may not reveal all edge cases
- 2 cores may not fully test parallel performance
- Inference quality depends on training data size

### Mitigation
- Test with multiple prompts
- Verify all pipeline stages complete
- Check for memory leaks or crashes
- Monitor progress bar display quality

## Execution Decision

**Recommendation**: Proceed with full pipeline test in current 2-core environment.

**Rationale**:
1. 6-12 minutes is acceptable for comprehensive testing
2. Will verify entire pipeline end-to-end
3. Will validate new progress bar system
4. Will confirm inference works with trained models
5. Small dataset is sufficient for functional testing

**Alternative**: If time is critical, user can run on Saturn machine (2-5 min with 8 cores)