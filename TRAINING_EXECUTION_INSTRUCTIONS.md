# 🚀 Training Execution Instructions

## Quick Start Guide for Training CLLM on Repository Code

**Estimated Time:** 12-25 minutes  
**Difficulty:** Easy (UI-based)  
**Prerequisites:** Application built and ready

---

## Step-by-Step Instructions

### 1. Start the Application

```bash
cd app
LD_LIBRARY_PATH=.. ./hyper_prime_spiral
```

The application window will open with multiple tabs.

### 2. Navigate to Training Tab

Click on the **"Training"** tab in the top navigation bar.

### 3. Load Training Data

**Option A: Load Prepared Data (Recommended)**
1. Click the **"Scan Dir"** button
2. The system will scan the `training_data/` directory
3. You should see `repo_code.txt` in the file list
4. Click the checkbox next to `repo_code.txt` to select it

**Option B: Load Multiple Files**
1. Click **"Scan Dir"** to scan any directory
2. Select multiple `.c` and `.h` files
3. Click **"Select All"** to select all files at once

### 4. Configure Training Parameters

Set the following parameters in the control panel:

| Parameter | Value | Description |
|-----------|-------|-------------|
| **Learning Rate** | 0.001 | Controls how fast the model learns |
| **Epochs** | 10 | Number of complete passes through data |
| **Batch Size** | 32 | Number of samples per batch |
| **Threads** | 0 | Auto-detect (will use 12 spheres) |

### 5. Start Training

1. Click the **"START TRAINING"** button
2. The button will change to **"STOP TRAINING"**
3. Training will begin immediately

### 6. Monitor Training Progress

You'll see three main visualization areas:

#### A. Sphere Visualization (Left Side)
- **12 colored spheres** arranged in a circle
- **Color coding:**
  - Dark Gray = Inactive
  - Blue = Low activity
  - Cyan = Medium activity
  - Yellow = High activity
  - Orange/Red = Very high activity
- **Numbers** on each sphere show batch counts

#### B. Statistics Table (Right Side)
- Per-sphere batch counts
- Average loss per sphere
- Active/Idle status for each sphere

#### C. Loss Graph (Bottom)
- Real-time loss curve
- Shows training progress
- Loss should decrease over time

#### D. Progress Bar
- Shows current epoch progress
- Displays current and best loss values

### 7. What to Expect

**Initial Phase (Epochs 1-2):**
- Loss starts high (~8-10)
- All 12 spheres activate
- Colors change as spheres process batches
- Loss begins to decrease

**Middle Phase (Epochs 3-7):**
- Loss continues decreasing (~3-5)
- Sphere activity stabilizes
- Training speed becomes consistent
- Loss graph shows clear downward trend

**Final Phase (Epochs 8-10):**
- Loss reaches target range (~1.5-2.5)
- Sphere utilization optimized
- Training completes successfully
- Final model saved automatically

### 8. Save the Model

**Automatic Saves:**
- Checkpoints saved every 2 epochs
- Best model saved automatically
- Location: `checkpoints/checkpoint_epoch_N.cllm`

**Manual Save:**
- Click the **"Save"** button anytime
- Creates immediate checkpoint
- Useful for preserving good states

**Final Model:**
- Automatically saved at completion
- Location: `models/trained_model_kissing_spheres.cllm`
- Ready for code generation testing

### 9. Test Code Generation

After training completes:

1. Navigate to the **"LLM"** tab
2. Load the trained model
3. Try these test prompts:
   - `int main() {`
   - `// Calculate the nth prime number`
   - `void cllm_`
   - `// Convert to lattice coordinates`
4. Observe the generated code quality

### 10. Collect Benchmarks

Record the following metrics:

**Training Metrics:**
- Total training time
- Time per epoch
- Final loss value
- Best loss achieved

**Sphere Metrics:**
- Number of active spheres (should be 12/12)
- Per-sphere batch counts
- Load balance (variance between spheres)
- Gradient norm values

**Performance Metrics:**
- Throughput (tokens/second)
- Speedup vs single-threaded (if known)
- Memory usage
- CPU utilization

---

## Troubleshooting

### Issue: Training Won't Start

**Symptoms:** Click "START TRAINING" but nothing happens

**Solutions:**
1. Verify at least one file is selected
2. Check that model is loaded (status shows "Model: Loaded")
3. Try creating a new model (restart application)
4. Check console output for error messages

### Issue: Slow Training

**Symptoms:** Training takes much longer than expected

**Solutions:**
1. Verify threads = 0 (auto-detect)
2. Check CPU usage (should be high)
3. Reduce batch size to 16 or 8
4. Reduce sequence length
5. Check disk I/O (training data access)

### Issue: High Loss / Not Converging

**Symptoms:** Loss stays high or increases

**Solutions:**
1. Reduce learning rate to 0.0005
2. Increase warmup steps
3. Check training data quality
4. Verify vocabulary was built correctly
5. Try training for more epochs

### Issue: Sphere Imbalance

**Symptoms:** Some spheres process many more batches than others

**Solutions:**
1. This is normal for small datasets
2. Larger datasets will balance better
3. Check that all 12 spheres are active
4. Monitor over multiple epochs

### Issue: Out of Memory

**Symptoms:** Application crashes or freezes

**Solutions:**
1. Reduce batch size to 16 or 8
2. Reduce sequence length to 64
3. Disable mixed precision
4. Close other applications
5. Check available RAM

---

## Expected Results

### Training Timeline

```
Epoch 1:  ~2 minutes  | Loss: 8.5 → 6.2
Epoch 2:  ~2 minutes  | Loss: 6.2 → 4.8
Epoch 3:  ~2 minutes  | Loss: 4.8 → 3.9
Epoch 4:  ~2 minutes  | Loss: 3.9 → 3.2
Epoch 5:  ~2 minutes  | Loss: 3.2 → 2.7
Epoch 6:  ~2 minutes  | Loss: 2.7 → 2.3
Epoch 7:  ~2 minutes  | Loss: 2.3 → 2.0
Epoch 8:  ~2 minutes  | Loss: 2.0 → 1.8
Epoch 9:  ~2 minutes  | Loss: 1.8 → 1.6
Epoch 10: ~2 minutes  | Loss: 1.6 → 1.5

Total: ~20 minutes
```

### Success Indicators

✅ **Training Successful If:**
- All 10 epochs complete
- Loss decreases consistently
- Final loss < 2.5
- All 12 spheres active
- No crashes or errors
- Model saved successfully

⚠️ **Warning Signs:**
- Loss increases
- Spheres become inactive
- Training crashes
- Very slow progress
- Memory errors

### Code Generation Quality

**After 5 Epochs:**
- Basic syntax mostly correct
- Simple functions work
- Some semantic errors
- Inconsistent style

**After 10 Epochs:**
- Good syntax accuracy (>80%)
- Reasonable semantics
- Consistent style
- Usable code snippets
- Proper function signatures

---

## Benchmark Data to Collect

### Performance Metrics

```
Training Time:
  - Total: _____ minutes
  - Per Epoch: _____ seconds
  - Per Batch: _____ ms

Loss Metrics:
  - Initial Loss: _____
  - Final Loss: _____
  - Best Loss: _____
  - Reduction: _____% 

Sphere Utilization:
  - Active Spheres: _____ / 12
  - Avg Batches/Sphere: _____
  - Load Variance: _____%
  - Gradient Norm: _____

System Resources:
  - Peak Memory: _____ MB
  - Avg CPU: _____%
  - Disk I/O: _____ MB/s
```

### Code Generation Tests

Test each prompt and rate the output:

1. **`int main() {`**
   - Syntax: ☐ Correct ☐ Errors
   - Semantics: ☐ Good ☐ Fair ☐ Poor
   - Style: ☐ Consistent ☐ Inconsistent

2. **`// Calculate prime`**
   - Syntax: ☐ Correct ☐ Errors
   - Semantics: ☐ Good ☐ Fair ☐ Poor
   - Logic: ☐ Correct ☐ Flawed

3. **`void cllm_`**
   - Syntax: ☐ Correct ☐ Errors
   - API Usage: ☐ Accurate ☐ Inaccurate
   - Naming: ☐ Consistent ☐ Inconsistent

4. **`// Lattice`**
   - Syntax: ☐ Correct ☐ Errors
   - Domain Knowledge: ☐ Good ☐ Fair ☐ Poor
   - Completeness: ☐ Complete ☐ Partial

---

## After Training

### 1. Save Results

Create a file `MY_TRAINING_RESULTS.md` with:
- All benchmark data collected
- Screenshots of sphere visualization
- Code generation examples
- Any issues encountered
- Observations and notes

### 2. Validate Model

```bash
# Check model file exists
ls -lh models/trained_model_kissing_spheres.cllm

# Check checkpoints
ls -lh checkpoints/

# Verify file sizes are reasonable
```

### 3. Test Code Generation

Use the LLM tab to test various prompts and evaluate quality.

### 4. Share Results

If desired, share your results:
- Training time achieved
- Loss values
- Code generation quality
- Any interesting observations

---

## Quick Reference

### Key Files

```
training_data/repo_code.txt          - Training data (7.2 MB)
models/trained_model_kissing_spheres.cllm  - Final model
checkpoints/checkpoint_epoch_N.cllm  - Periodic checkpoints
```

### Key Metrics

```
Target Loss: < 2.5
Training Time: 12-25 minutes
Active Spheres: 12/12
Speedup: 5-10x vs single-threaded
```

### Support

If you encounter issues:
1. Check the troubleshooting section above
2. Review console output for errors
3. Try reducing batch size or epochs
4. Restart the application if needed

---

## Summary

**What You'll Do:**
1. Start application
2. Load training data
3. Configure parameters
4. Click "START TRAINING"
5. Watch sphere visualization
6. Wait 12-25 minutes
7. Model saved automatically
8. Test code generation

**What You'll Get:**
- Trained CLLM model
- Performance benchmarks
- Code generation capability
- Sphere utilization data
- Training experience

**Time Required:**
- Setup: 2 minutes
- Training: 12-25 minutes
- Testing: 5 minutes
- Total: ~20-30 minutes

---

**Ready to train? Start the application and follow the steps above!**

Good luck! 🚀