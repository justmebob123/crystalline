# Remaining Issues and Solutions

## Current Status

### ✅ What's Working
1. UI stays responsive during training (separate thread)
2. Terminal output is clean (no spam)
3. Training completes successfully
4. Basic generation works

### ❌ What Needs Fixing

## 1. Training Not Using All CPU Cores

**Problem**: Thread pool is initialized but training is still single-threaded

**Root Cause**: `cllm_train_epoch_parallel()` creates thread pool but doesn't actually parallelize batch processing. The current implementation still processes batches sequentially.

**Solution Needed**:
```c
// Current (single-threaded):
for each batch:
    forward_pass(batch)
    backward_pass(batch)
    optimizer_step()

// Needed (multi-threaded):
for each batch:
    submit_to_thread_pool(batch)
wait_for_all_threads()
accumulate_gradients()
optimizer_step()
```

**Files to Modify**:
- `src/ai/cllm_training_parallel.c` - Implement actual parallel batch processing
- Need gradient accumulation across threads
- Need thread-safe gradient updates

## 2. Model Weights Not Saving

**Problem**: "Warning: Model has no weights to save (weights=(nil))"

**Root Cause**: Model structure has `weights` field but it's never allocated. Actual weights are in individual layer structures (attention_layers, ff_layers, embeddings).

**Two Possible Solutions**:

### Option A: Flatten weights into single array
- Allocate `model->weights` array
- Copy all layer weights into flat array before saving
- Restore from flat array when loading
- **Pros**: Works with current save/load format
- **Cons**: Memory overhead, complex copying

### Option B: Save layer-by-layer
- Modify save format to save each layer separately
- Save embeddings, attention layers, ff layers individually
- **Pros**: No memory overhead, cleaner
- **Cons**: Requires rewriting save/load functions

**Recommended**: Option B (cleaner architecture)

**Files to Modify**:
- `src/ai/cllm_format.c` - Rewrite save/load to handle layer structures
- `app/cllm_integration.c` - Ensure proper initialization

## 3. Model Not Loading on Startup

**Problem**: Model saved to `models/saved_model.cllm` but startup looks in wrong paths

**Status**: ✅ FIXED in latest commit
- Changed default paths to include `models/saved_model.cllm`
- Removed incorrect `~/.cllm_models/` prefix

**Testing Needed**: Verify model loads on next startup

## 4. No File Picker for Loading Models

**Problem**: Can't browse filesystem to select model file

**Solution Needed**:
- Add file picker dialog (use native file dialog or implement simple one)
- Add "Browse..." button next to Load button
- Store selected path and load from there

**Options**:
1. Use native file dialog (requires additional library)
2. Implement simple file browser in SDL
3. Add text input for file path

**Recommended**: Option 3 (simplest, no dependencies)

**Files to Create/Modify**:
- Add text input field for model path in LLM tab
- Add "Browse" or "Load from path" functionality

## 5. No Prompt Before Training

**Problem**: Training starts immediately, potentially overwriting existing model

**Solution Needed**:
- Show dialog before training starts
- Options: "Create New Model", "Overwrite Existing", "Cancel"
- If "Create New", prompt for model name
- If "Overwrite", confirm action

**Status**: Dialog system created but not integrated
- `app/simple_dialog.c/h` - Dialog system ready
- Need to integrate with training tab
- Need to handle dialog results

**Files to Modify**:
- `app/ui/tabs/tab_training.c` - Add dialog before training
- `app/app_common.h` - Add dialog state
- `app/main.c` - Handle dialog rendering and events

## 6. Generation Quality Poor

**Problem**: Generated text is nonsensical ("is the efficient")

**Root Causes**:
1. Very small training data (only 357 tokens total)
2. Model not actually learning (loss stays constant)
3. Vocabulary too small (34 tokens)

**Solutions**:
1. **Immediate**: Use larger training corpus
   - Current: sample_training.txt (33 tokens) + better_training.txt (324 tokens)
   - Needed: large_corpus.txt (10,000+ tokens) - already created!
   - User should select large_corpus.txt instead

2. **Short-term**: Fix loss computation
   - Loss is 23.0259 and doesn't decrease
   - Need to verify loss calculation is correct
   - Need to verify gradients are being computed

3. **Long-term**: Improve training
   - More epochs
   - Better learning rate
   - Proper validation set

## Priority Order

### High Priority (Critical)
1. ✅ Model loading on startup - FIXED
2. ❌ Model weights saving/loading - CRITICAL
3. ❌ Loss computation verification - CRITICAL

### Medium Priority (Important)
4. ❌ Actual parallel training - PERFORMANCE
5. ❌ Training prompt dialog - USER EXPERIENCE
6. ❌ Use larger training data - QUALITY

### Low Priority (Nice to Have)
7. ❌ File picker for models - CONVENIENCE
8. ❌ Better error messages - UX
9. ❌ Training progress details - UX

## Recommended Next Steps

1. **Fix model save/load** (Option B - layer-by-layer)
   - Rewrite `cllm_write()` to save all layer weights
   - Rewrite `cllm_read_model()` to load all layer weights
   - Test save/load cycle

2. **Verify loss computation**
   - Add debug output to loss function
   - Check if gradients are non-zero
   - Verify backward pass is working

3. **Use larger training data**
   - Select `large_corpus.txt` instead of small files
   - Should have 10,000+ tokens
   - Better vocabulary coverage

4. **Integrate training dialog**
   - Add dialog to training tab
   - Show before training starts
   - Handle user choice

5. **Implement actual parallel training**
   - Parallelize batch processing
   - Add gradient accumulation
   - Test performance improvement

## Testing Checklist

After fixes:
- [ ] Model saves without warnings
- [ ] Model loads on startup
- [ ] Training uses all CPU cores
- [ ] Loss decreases over epochs
- [ ] Generated text is coherent
- [ ] Dialog appears before training
- [ ] Can select model file to load
- [ ] UI stays responsive
- [ ] No crashes or memory leaks

## Notes

- The architecture is sound (threading works, UI responsive)
- Main issues are in model persistence and training effectiveness
- Generation quality will improve once training actually works
- Small training data is a temporary issue (large corpus exists)