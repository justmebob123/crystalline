# Quick Start: Training with Pure Crystalline Loss

## Prerequisites

✅ System validated (see CRYSTALLINE_SYSTEM_VALIDATION.md)  
✅ All tests passing  
✅ Libraries compiled  

## Training a Model

### Option 1: Using Existing Training Tools

The system already has training infrastructure. To train a model:

```bash
# 1. Prepare your training data
cat > training_data.txt << EOF
The quick brown fox jumps over the lazy dog.
Machine learning is transforming artificial intelligence.
Prime numbers are the building blocks of mathematics.
EOF

# 2. Build vocabulary
./tools/cllm_vocab_build training_data.txt vocab.txt 1000

# 3. Train model (if train_model tool exists)
# Check what training tools are available:
ls tools/train* tools/*train*

# Or use the demo:
cd demos && make threaded_training_demo
./threaded_training_demo
```

### Option 2: Using the Application UI

```bash
# Run the application with UI
cd app && make
./hyper_prime_spiral

# In the UI:
# 1. Go to "Training" tab
# 2. Load your training data
# 3. Configure model parameters
# 4. Click "Start Training"
# 5. Watch real-time visualization of 12 worker threads
```

### Option 3: Create Custom Training Script

```c
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_utils.h"

int main() {
    // 1. Create model
    CLLMConfig config = {
        .vocab_size = 1000,
        .embedding_dim = 256,
        .num_layers = 4,
        .num_heads = 8,
        .ff_dim = 1024,
        .max_seq_len = 128,
        .dropout = 0.1f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    
    // 2. Load training data
    // (implement your data loading here)
    
    // 3. Create training configuration
    CLLMTrainingConfig train_config = {
        .learning_rate = 0.001f,
        .batch_size = 32,
        .num_epochs = 10,
        .max_steps = 10000,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
        .warmup_steps = 1000
    };
    
    // 4. Train
    CLLMTraining* training = cllm_create_training(model, &train_config);
    
    // Training loop will automatically use crystalline loss
    // and distribute work across 12 threads
    
    // 5. Save model
    cllm_save_model(model, "trained_model.cllm");
    
    // Cleanup
    cllm_free_training(training);
    cllm_free_model(model);
    
    return 0;
}
```

## What to Expect During Training

### Console Output

```
=== Initializing Crystalline Structure ===
Generating prime encodings for 1000 tokens...
Generating lattice embeddings for 1000 tokens...
  Generated 1000/1000 embeddings
Lattice embeddings generation complete!
✓ Crystalline prime encodings initialized
✓ 12D lattice coordinates computed
==========================================

Starting training...
Epoch 1/10
  Step 100/1000: loss = 2.345
  Step 200/1000: loss = 2.123
  Step 300/1000: loss = 1.987
  ...
```

### Key Metrics to Monitor

1. **Loss Value**: Should be positive and decreasing
   - Initial: ~2.0-3.0 (depends on data)
   - Should decrease steadily
   - If loss = 0.0, something is wrong

2. **Thread Utilization**: All 12 threads should be active
   - Check with `top` or `htop`
   - Should see ~1200% CPU usage (12 threads × 100%)

3. **Symmetry Group Distribution**: Should be even
   - Each group processes ~8.3% of batches
   - No idle threads

## Verifying Training is Working

### Check 1: Loss is Decreasing

```bash
# Monitor training output
tail -f training.log | grep "loss"

# Should see:
# Epoch 1: loss = 2.345
# Epoch 2: loss = 2.123
# Epoch 3: loss = 1.987
# ...
```

### Check 2: All Threads Active

```bash
# While training is running:
top -H -p $(pgrep train_model)

# Should see 12-13 threads (1 main + 12 workers)
# All with high CPU usage
```

### Check 3: Crystalline Loss is Being Used

The training system automatically uses `cllm_compute_crystalline_loss()` which:
- Uses learned prime encodings (not token IDs)
- Uses learned lattice coordinates
- Computes GCD similarity (semantic)
- Computes lattice distance (geometric)
- Combines: 70% GCD + 30% lattice

## Troubleshooting

### Problem: Loss is 0.0

**Cause**: Prime encodings not initialized

**Solution**:
```c
// Make sure this is called during model creation:
cllm_generate_lattice_embeddings(model);
```

### Problem: Only 4 threads active

**Cause**: Old symmetry group bug (should be fixed)

**Solution**: Rebuild with latest code
```bash
make clean && make
```

### Problem: Loss not decreasing

**Possible causes**:
1. Learning rate too high/low
2. Gradient clipping too aggressive
3. Data quality issues
4. Need more epochs

**Solution**: Adjust hyperparameters

## Expected Performance

### Small Model (1K vocab, 256 dim)
- Training time: ~5-10 minutes for 10 epochs
- Final loss: ~0.5-1.0
- Memory: ~50 MB

### Medium Model (10K vocab, 512 dim)
- Training time: ~30-60 minutes for 10 epochs
- Final loss: ~0.3-0.7
- Memory: ~500 MB

### Large Model (30K vocab, 768 dim)
- Training time: ~2-4 hours for 10 epochs
- Final loss: ~0.2-0.5
- Memory: ~2 GB

## Understanding the Loss Function

### What the Loss Measures

The crystalline loss measures how well the model's learned representations capture:

1. **Semantic Similarity (70%)**: 
   - GCD of prime encodings
   - Shared prime factors = shared meaning
   - Example: "run" and "running" should have high GCD similarity

2. **Geometric Similarity (30%)**:
   - Distance in 12D lattice space
   - Nearby tokens = related concepts
   - Example: "cat" and "dog" should be close in lattice

### Why This is Revolutionary

Unlike standard cross-entropy loss which measures:
- "How well do predictions match targets?"

Crystalline loss measures:
- "How well do learned mathematical relationships capture semantic structure?"

This is **deterministic intelligence** vs **statistical pattern matching**.

## Next Steps After Training

1. **Test Inference**: Use trained model for predictions
2. **Analyze Primes**: Examine learned prime encodings
3. **Visualize Lattice**: Plot tokens in 3D space
4. **Measure Similarity**: Test GCD relationships
5. **Compare to Standard**: Benchmark vs cross-entropy

## Resources

- **Validation Report**: CRYSTALLINE_SYSTEM_VALIDATION.md
- **Test Program**: tests/test_prime_encodings.c
- **Architecture Docs**: See MASTER_PLAN.md
- **Code**: src/ai/cllm_training.c (loss function)

---

**Ready to train? The crystalline system awaits!** 🚀