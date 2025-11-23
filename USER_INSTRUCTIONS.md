# USER INSTRUCTIONS - Testing Training Fixes

## Quick Start

```bash
cd ~/code/math/crystalline
git status  # Verify commit 978eefa is present
make clean && make && make app
app/hyper_prime_spiral
```

## What Was Fixed

1. **Attention Mechanism** - Replaced broken simplified attention with proper multi-head attention
2. **Diagnostic Output** - Added weight and gradient statistics
3. **Startup Performance** - Deferred lattice cache initialization (5s → 1s)

## Testing Steps

### 1. Start Application
- Should start in ~1 second (much faster than before)
- No lattice cache initialization on startup

### 2. Go to Training Tab
- Click "Scan Directory"
- Select "large_corpus.txt"
- Set epochs to 10-20

### 3. Start Training
- Click "START TRAINING"
- Watch console output

### 4. Check Console Output

**You should see**:
```
=== TRAINING MODE ===
Training data: 1241 tokens
  Weight magnitudes: embed=0.012345, attn=0.008765, ff=0.015432
  Batch 1: loss = 9.2103
    Gradients: embed=5234 (max=1.23e-02, avg=3.45e-04), attn=2048 (max=8.76e-03)
  Batch 5: loss = 8.7543
    Gradients: embed=5189 (max=1.15e-02, avg=3.21e-04), attn=2031 (max=8.12e-03)
```

**Key things to verify**:
- ✅ Weights are non-zero (all > 0.001)
- ✅ Gradients are non-zero (thousands of them)
- ✅ **Loss DECREASES** over batches (9.2 → 8.7 → 8.3 → ...)

### 5. After Training
- Go to LLM tab
- Try generating text
- Should be more varied than before

## What to Report

### If It Works ✅
- "Loss decreased from X to Y over Z epochs"
- "Generation quality improved"
- "Startup time is faster"

### If It Doesn't Work ❌
Please provide:
1. **Console output** (copy/paste the diagnostic lines)
2. **Loss values** (did they change at all?)
3. **Weight magnitudes** (from first batch)
4. **Gradient magnitudes** (from first batch)
5. **Any error messages**

## Expected Behavior

### Loss Should Decrease
- Epoch 1: 9.2 → 8.5
- Epoch 5: 7.8 → 7.0
- Epoch 10: 6.5 → 6.0

### Generation Should Improve
- Before: "is the blue is the blue is the blue"
- After: "the sky is blue because of light scattering"

## Troubleshooting

### If loss stays constant:
- Check if weights are zeros (broken initialization)
- Check if gradients are zeros (broken backward pass)
- Provide console output for analysis

### If loss increases:
- Learning rate might be too high
- Gradient explosion
- Provide gradient magnitudes

### If app crashes:
- Provide error message
- Check if model file is corrupted
- Try deleting models/saved_model.cllm and retraining

## Files to Check

- `models/saved_model.cllm` - Trained model
- Console output - Diagnostic information
- `todo.md` - Updated status

## Next Steps After Testing

If training works:
1. Train for more epochs (50-100)
2. Try larger training corpus
3. Test generation quality
4. Consider integrating crystalline algorithms

If training doesn't work:
1. Report findings
2. We'll debug based on diagnostic output
3. May need additional fixes

## Summary

The main fix was replacing the broken attention mechanism. The model should now be able to learn relationships between tokens and the loss should decrease over epochs. Test it and let me know the results!
