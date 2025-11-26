# Day 4: Integration Testing with Real Repository Data

**Date:** November 26, 2024  
**Status:** 🔄 IN PROGRESS

## Overview

This guide documents the process of training the CLLM model on the entire repository codebase using the kissing spheres threading architecture.

## Training Data Preparation

### Repository Statistics
- **C Source Files:** 530 files
- **C Header Files:** 250 files  
- **Total Lines:** 225,940 lines
- **Total Size:** 7.2 MB
- **Training Files:** 780 files

### Data Collection

Training data has been prepared and consolidated into:
```
training_data/repo_code.txt
```

This file contains all C source and header files from the repository, with file markers for context.

## Training Configuration

### Model Architecture
```
Vocabulary Size: 50,000 tokens
Embedding Dimension: 256
Number of Layers: 6
Attention Heads: 8
Feed-Forward Dimension: 1024
Max Sequence Length: 512
Dropout: 0.1
```

### Training Parameters
```
Epochs: 10
Batch Size: 32
Sequence Length: 128
Learning Rate: 0.001
Weight Decay: 0.01
Gradient Clipping: 1.0
Warmup Steps: 100
```

### Performance Optimizations
```
Gradient Accumulation: 4 steps (effective batch size: 128)
Mixed Precision: Enabled (FP16/FP32)
Loss Scaling: 1024.0 (dynamic)
Threading: 12 worker spheres (kissing spheres architecture)
```

## Training Methods

### Method 1: Using the UI Application (Recommended)

1. **Build and Run the Application:**
   ```bash
   cd app
   make clean && make
   LD_LIBRARY_PATH=.. ./hyper_prime_spiral
   ```

2. **Navigate to Training Tab:**
   - Click on the "Training" tab in the UI

3. **Load Training Data:**
   - Click "Scan Dir" to scan the training_data directory
   - Select `repo_code.txt`
   - Or manually select multiple .c and .h files

4. **Configure Training:**
   - Learning Rate: 0.001
   - Epochs: 10
   - Batch Size: 32
   - Threads: 0 (auto-detect = 12 spheres)

5. **Start Training:**
   - Click "START TRAINING"
   - Monitor the sphere visualization in real-time
   - Watch the loss graph decrease
   - Observe per-sphere statistics

6. **Monitor Progress:**
   - **Sphere Visualization:** Shows 12 spheres with activity colors
   - **Statistics Table:** Per-sphere batch counts and losses
   - **Loss Graph:** Training loss over time
   - **Progress Bar:** Current epoch progress

7. **Save Model:**
   - Click "Save" to create a checkpoint
   - Model saved to: `checkpoints/checkpoint_epoch_N.cllm`
   - Final model: `models/trained_model_kissing_spheres.cllm`

### Method 2: Using Command-Line Training (Alternative)

For automated/batch training, you can use the training thread directly:

```bash
# Create a simple training script
./train_on_repository.sh
```

This would:
1. Load the training data
2. Initialize the model
3. Run training with 12 spheres
4. Save checkpoints
5. Generate final model

## Expected Training Timeline

### Per-Epoch Estimates
```
Data Size: 7.2 MB
Batch Size: 32
Sequence Length: 128
Effective Batch Size: 128 (with accumulation)

Estimated batches per epoch: ~1,500
Estimated time per batch: 50-100ms (with 12 threads)
Estimated time per epoch: 75-150 seconds

Total training time (10 epochs): 12-25 minutes
```

### Performance Expectations

**Single-Threaded Baseline:**
- Time per batch: ~500ms
- Time per epoch: ~750 seconds
- Total time: ~125 minutes

**12-Sphere Threading:**
- Time per batch: ~50-100ms
- Time per epoch: ~75-150 seconds  
- Total time: ~12-25 minutes
- **Expected Speedup: 5-10x**

## Monitoring Training

### Real-Time Metrics

1. **Sphere Activity:**
   - Each sphere shows its processing activity
   - Color indicates relative workload
   - Batch counts displayed per sphere

2. **Loss Tracking:**
   - Current loss value
   - Best loss achieved
   - Loss history graph

3. **Gradient Monitoring:**
   - Total gradient norm
   - Per-sphere gradient contributions
   - Gradient clipping events

4. **System Stats:**
   - Active spheres (X/12)
   - Total batches processed
   - Current epoch/step

### Checkpointing

Checkpoints are saved:
- Every 2 epochs (configurable)
- When best loss is achieved
- On manual save request

Checkpoint format:
```
checkpoints/checkpoint_epoch_N_loss_X.XXXX.cllm
```

## Testing Code Generation

After training, test the model's code generation capabilities:

### Test Prompts

1. **Function Definition:**
   ```c
   int main() {
   ```
   Expected: Complete main function with proper structure

2. **Prime Calculation:**
   ```c
   // Calculate the nth prime number
   ```
   Expected: Prime calculation algorithm

3. **CLLM Function:**
   ```c
   void cllm_
   ```
   Expected: CLLM library function definition

4. **Lattice Operation:**
   ```c
   // Convert to lattice coordinates
   ```
   Expected: Lattice conversion code

### Validation Criteria

**Success Indicators:**
- ✅ Syntactically correct C code
- ✅ Proper function signatures
- ✅ Consistent naming conventions
- ✅ Appropriate comments
- ✅ Logical code structure

**Quality Metrics:**
- Compilation success rate
- Semantic correctness
- Style consistency
- Documentation quality

## Benchmark Data Collection

### Training Metrics

Record the following during training:

1. **Performance:**
   - Time per epoch
   - Time per batch
   - Throughput (tokens/second)
   - GPU/CPU utilization

2. **Loss Metrics:**
   - Initial loss
   - Final loss
   - Best loss
   - Loss reduction rate

3. **Threading Efficiency:**
   - Per-sphere utilization
   - Load balancing
   - Synchronization overhead
   - Speedup vs single-threaded

4. **Memory Usage:**
   - Peak memory
   - Average memory
   - Per-sphere memory
   - Gradient buffer size

### Code Generation Metrics

After training, evaluate:

1. **Syntax Accuracy:**
   - Percentage of compilable code
   - Parse error rate
   - Syntax error types

2. **Semantic Quality:**
   - Logical correctness
   - API usage accuracy
   - Variable naming
   - Code structure

3. **Style Consistency:**
   - Indentation
   - Naming conventions
   - Comment style
   - Code organization

## Expected Results

### Training Convergence

**Initial Loss:** ~8-10 (random initialization)
**Target Loss:** <2.0 (good convergence)
**Best Case:** <1.0 (excellent convergence)

### Loss Trajectory
```
Epoch 1: 8.5 → 6.2
Epoch 2: 6.2 → 4.8
Epoch 3: 4.8 → 3.9
Epoch 4: 3.9 → 3.2
Epoch 5: 3.2 → 2.7
Epoch 6: 2.7 → 2.3
Epoch 7: 2.3 → 2.0
Epoch 8: 2.0 → 1.8
Epoch 9: 1.8 → 1.6
Epoch 10: 1.6 → 1.5
```

### Code Generation Quality

**After 5 Epochs:**
- Basic syntax correct
- Simple functions work
- Some semantic errors

**After 10 Epochs:**
- Good syntax accuracy
- Reasonable semantics
- Consistent style
- Usable code snippets

## Troubleshooting

### Common Issues

1. **Out of Memory:**
   - Reduce batch size
   - Reduce sequence length
   - Disable mixed precision

2. **Slow Training:**
   - Check thread count (should be 12)
   - Verify CPU utilization
   - Check disk I/O

3. **Poor Convergence:**
   - Adjust learning rate
   - Increase warmup steps
   - Check data quality

4. **Sphere Imbalance:**
   - Check batch distribution
   - Verify load balancing
   - Monitor per-sphere stats

## Next Steps

After successful training:

1. **Save Final Model:**
   ```
   models/repo_trained_model.cllm
   ```

2. **Test Generation:**
   - Run code generation tests
   - Validate output quality
   - Measure accuracy

3. **Benchmark Results:**
   - Document training time
   - Record loss metrics
   - Analyze sphere efficiency

4. **Commit to Repository:**
   ```bash
   git add models/repo_trained_model.cllm
   git commit -m "feat: Add trained model on repository codebase"
   git push
   ```

## Success Criteria

- ✅ Training completes all 10 epochs
- ✅ Loss decreases consistently
- ✅ All 12 spheres active
- ✅ No crashes or errors
- ✅ Model generates compilable code
- ✅ Speedup of 5-10x achieved
- ✅ Model saved successfully

## Documentation

All training results will be documented in:
- `DAY4_TRAINING_RESULTS.md` - Detailed results
- `TRAINING_BENCHMARKS.md` - Performance data
- `MODEL_EVALUATION.md` - Code generation quality

---

**Status:** Ready for Training  
**Next:** Execute training and collect benchmarks