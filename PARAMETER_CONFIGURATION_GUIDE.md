# CLLM Parameter Configuration Guide

## Overview
This guide provides recommendations for configuring CLLM training parameters to achieve optimal performance and avoid common pitfalls.

## The Configuration Problem

### Issue Description
When training on small datasets with large batch sizes and sequence lengths, the system creates very few batches, resulting in:
- Poor thread utilization (many threads idle)
- Very slow training progress
- Appearance of "hanging" or "deadlock"

### Root Cause
The number of batches created is determined by:
```
num_batches = total_tokens / (batch_size × sequence_length)
```

For example:
- Dataset: 17,408 tokens
- Batch size: 32
- Sequence length: 128
- Result: 17,408 / (32 × 128) = **4.25 batches** (only 4 complete batches)

With 12 worker threads, this means 8 threads will be idle throughout training!

## Parameter Guidelines

### 1. Batch Size (`--batch` or `-b`)

**Purpose**: Number of sequences processed together in one training step.

**Recommendations**:
- **Small datasets (< 50K tokens)**: 4-8
- **Medium datasets (50K-500K tokens)**: 16-32
- **Large datasets (> 500K tokens)**: 32-64

**Trade-offs**:
- Larger batch sizes: More stable gradients, better GPU utilization, but slower per-batch
- Smaller batch sizes: Faster iterations, more noisy gradients, better for small datasets

### 2. Sequence Length (`--seq-len` or `-s`)

**Purpose**: Maximum length of input sequences (in tokens).

**Recommendations**:
- **Small datasets**: 8-32
- **Medium datasets**: 32-128
- **Large datasets**: 128-512

**Trade-offs**:
- Longer sequences: Better context, more memory usage, slower attention computation
- Shorter sequences: Faster training, less memory, limited context

### 3. Number of Threads (`--threads` or `-t`)

**Purpose**: Number of worker threads for parallel training.

**Recommendations**:
- **Auto mode (0)**: System automatically detects CPU cores (recommended)
- **Manual**: Set to number of CPU cores, capped at 12 for optimal kissing sphere architecture
- **Rule of thumb**: Ensure `num_batches >= num_threads` for good utilization

**Trade-offs**:
- More threads: Faster training IF enough batches exist
- Fewer threads: Better for small datasets with few batches

### 4. Vocabulary Size (`--vocab` or `-v`)

**Purpose**: Maximum number of unique tokens in the vocabulary.

**Recommendations**:
- **Small datasets**: 1,000-5,000
- **Medium datasets**: 5,000-20,000
- **Large datasets**: 20,000-50,000

**Trade-offs**:
- Larger vocabulary: Better coverage, more memory, slower embedding lookups
- Smaller vocabulary: Faster, less memory, may miss rare words

### 5. Embedding Dimension (`--embed` or `-e`)

**Purpose**: Dimensionality of token embeddings and model hidden states.

**Recommendations**:
- **Small models**: 128-256
- **Medium models**: 256-768
- **Large models**: 768-2048

**Trade-offs**:
- Higher dimensions: More expressive, better quality, more memory and computation
- Lower dimensions: Faster, less memory, may limit model capacity

### 6. Number of Layers (`--layers` or `-l`)

**Purpose**: Number of transformer layers in the model.

**Recommendations**:
- **Small models**: 4-6
- **Medium models**: 6-12
- **Large models**: 12-24

**Trade-offs**:
- More layers: Better representation learning, slower training
- Fewer layers: Faster training, may limit model capacity

### 7. Number of Attention Heads (`--heads` or `-H`)

**Purpose**: Number of parallel attention mechanisms per layer.

**Recommendations**:
- **Must be divisible by 12** (for 12-fold symmetry)
- **Small models**: 8 or 12
- **Medium models**: 12
- **Large models**: 12 or 24

**Constraints**:
- Must evenly divide embedding dimension
- Must be divisible by 12 for crystalline architecture

### 8. Learning Rate (`--lr` or `-r`)

**Purpose**: Step size for gradient descent optimization.

**Recommendations**:
- **Default**: 0.0001 (1e-4)
- **Small models**: 0.0001-0.001
- **Large models**: 0.00001-0.0001

**Trade-offs**:
- Higher learning rate: Faster convergence, risk of instability
- Lower learning rate: More stable, slower convergence

### 9. Number of Epochs (`--epochs` or `-E`)

**Purpose**: Number of complete passes through the training data.

**Recommendations**:
- **Small datasets**: 50-200 epochs
- **Medium datasets**: 20-100 epochs
- **Large datasets**: 10-50 epochs

**Trade-offs**:
- More epochs: Better convergence, risk of overfitting
- Fewer epochs: Faster training, may underfit

## Configuration Examples

### Example 1: Small Dataset (< 50K tokens)
```bash
cllm train -d ./data \
  --vocab 2000 \
  --embed 128 \
  --layers 4 \
  --heads 8 \
  --batch 4 \
  --seq-len 16 \
  --epochs 100 \
  --threads 0
```

**Why these parameters?**
- Small batch (4) and seq-len (16) create many batches for good thread utilization
- Small vocab (2000) and embed (128) appropriate for limited data
- 100 epochs to ensure convergence on small dataset

### Example 2: Medium Dataset (100K-500K tokens)
```bash
cllm train -d ./data \
  --vocab 10000 \
  --embed 512 \
  --layers 8 \
  --heads 12 \
  --batch 16 \
  --seq-len 64 \
  --epochs 50 \
  --threads 0
```

**Why these parameters?**
- Moderate batch (16) and seq-len (64) balance speed and batch count
- Larger vocab (10000) and embed (512) for better representation
- 50 epochs sufficient for medium dataset

### Example 3: Large Dataset (> 1M tokens)
```bash
cllm train -d ./data \
  --vocab 30000 \
  --embed 1024 \
  --layers 12 \
  --heads 12 \
  --batch 32 \
  --seq-len 128 \
  --epochs 20 \
  --threads 0
```

**Why these parameters?**
- Large batch (32) and seq-len (128) for efficient processing
- Large vocab (30000) and embed (1024) for complex patterns
- 20 epochs sufficient for large dataset

## Automatic Warnings

The CLLM training system automatically detects suboptimal configurations and provides warnings:

### Warning 1: Too Few Batches
```
⚠️  WARNING: Configuration creates only 4 batch(es) for 12 workers
   This will result in very slow training with poor thread utilization.
   Recommendation: Use smaller batch/seq-len parameters:
   Try: --batch 4 --seq-len 16 (creates ~68 batches)
```

**Action**: Reduce batch size and/or sequence length to create more batches.

### Warning 2: Very Small Dataset
```
⚠️  WARNING: Very small dataset (856 tokens)
   Consider using a larger dataset for better model quality.
```

**Action**: Add more training data or adjust expectations for model quality.

## Calculating Optimal Parameters

### Step 1: Determine Dataset Size
```bash
# Count tokens in your dataset
wc -w data/*.txt
```

### Step 2: Calculate Batch Count
```
num_batches = total_tokens / (batch_size × sequence_length)
```

### Step 3: Ensure Sufficient Batches
**Rule of thumb**: `num_batches >= 2 × num_threads`

For 12 threads, aim for at least 24 batches.

### Step 4: Adjust Parameters
If `num_batches < 24`:
1. Reduce batch size (try 4, 8, or 16)
2. Reduce sequence length (try 8, 16, or 32)
3. Recalculate until `num_batches >= 24`

## Performance Optimization

### Memory Usage
- Embedding dimension and vocabulary size are the primary memory consumers
- Reduce these if you encounter out-of-memory errors

### Training Speed
- Batch size and sequence length affect speed most
- More threads help only if you have enough batches
- Larger models train slower but produce better results

### Model Quality
- More layers and higher embedding dimensions improve quality
- Larger vocabulary captures more nuances
- More epochs improve convergence but risk overfitting

## Troubleshooting

### Problem: Training appears to hang
**Cause**: Too few batches for the number of threads
**Solution**: Reduce batch size and sequence length

### Problem: Out of memory
**Cause**: Model too large or batch size too high
**Solution**: Reduce embedding dimension, vocabulary size, or batch size

### Problem: Loss not decreasing
**Cause**: Learning rate too high/low, or insufficient epochs
**Solution**: Adjust learning rate or increase epochs

### Problem: Poor model quality
**Cause**: Model too small or insufficient training
**Solution**: Increase model size (layers, embedding dim) or train longer

## Best Practices

1. **Start small**: Begin with small models and gradually increase size
2. **Monitor warnings**: Pay attention to automatic configuration warnings
3. **Balance parameters**: Ensure batch count is sufficient for thread count
4. **Use auto-threading**: Let the system detect optimal thread count
5. **Save checkpoints**: Use `--checkpoint` to save progress regularly
6. **Experiment**: Try different configurations to find optimal settings
7. **Validate results**: Test model quality after training

## Advanced Topics

### 12-Fold Symmetry Constraint
The CLLM architecture uses a crystalline 12-fold symmetry structure:
- Number of attention heads must be divisible by 12
- Optimal thread count is 12 (one per dimension)
- Work distribution follows icosahedral geometry

### Kissing Spheres Architecture
- 12 worker threads operate as "kissing spheres"
- Each sphere processes batches independently
- Gradients are accumulated at sphere boundaries
- Lock-free work queue enables efficient coordination

### Entropy-Based Work Distribution
- Work is distributed based on dimensional entropy
- Higher entropy dimensions receive more work
- Ensures balanced load across all 12 dimensions
- Maintains 12-fold symmetry throughout training

## Conclusion

Proper parameter configuration is crucial for efficient CLLM training. Follow these guidelines to:
- Avoid common pitfalls (too few batches, poor thread utilization)
- Achieve optimal training speed and model quality
- Make the most of the crystalline architecture

For more information, see:
- `MASTER_PLAN.md` - Overall project architecture
- `THREADING_SYSTEM_VALIDATED.md` - Threading system details
- `PARALLEL_VOCAB_BUILDING.md` - Vocabulary building optimization