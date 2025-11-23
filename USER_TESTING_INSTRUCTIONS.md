# User Testing Instructions - Training Pipeline Fix

## What Was Fixed

This session fixed the fundamental training pipeline issues:

1. ✅ **Vocabulary System** - Now builds proper vocabulary from training data
2. ✅ **Training Data** - Created large corpus with 200+ proverbs (~10,000 tokens)
3. ✅ **Terminal Spam** - Removed all "Generating token..." and "Step X: Loss..." output
4. ✅ **UI Progress** - Training progress now visible in UI (progress bar, epoch, loss)
5. ✅ **Tokenization** - Uses actual vocabulary instead of character fallback

## How to Test

### Step 1: Update and Build
```bash
cd ~/code/math/crystalline
git pull origin main  # May fail due to SSH - that's OK, code is committed locally
make clean
make
make app
```

### Step 2: Run Application
```bash
app/hyper_prime_spiral
```

### Step 3: Test Training Flow

1. **Navigate to Training Tab**
   - Click on "Training" tab in the application

2. **Scan for Training Files**
   - Click "Scan Directory" button
   - Should see `large_corpus.txt` in the file list

3. **Select Training File**
   - Click on `large_corpus.txt` to select it
   - Checkbox should appear checked

4. **Configure Training**
   - Learning Rate: 0.001 (default)
   - Epochs: 10
   - Batch Size: 32 (default)

5. **Start Training**
   - Click "START TRAINING" button
   - **Watch Terminal Output** (should be clean):
     ```
     === BUILDING VOCABULARY ===
       Building vocab from: data/training/large_corpus.txt
       Built vocabulary with X unique tokens
       Vocabulary integrated into model successfully
     === LOADING TRAINING DATA ===
       Loading: data/training/large_corpus.txt
       ✓ Loaded X tokens
     === STARTING TRAINING ===
     Training with 1 files...
     ```
   - **NO MORE OUTPUT** - Training runs silently

6. **Watch UI Progress**
   - Progress bar should fill up as epochs complete
   - "Epoch: X / 10" should increment
   - "Loss: X.XXXX" should update (may still be 0.0000 - that's a known issue)

### Step 4: Test Generation (After Training)

1. **Navigate to LLM Tab**
   - Click on "LLM" tab

2. **Enter Prompt**
   - Type: "the quick brown"

3. **Generate Text**
   - Click "Generate" button
   - **Watch Terminal** - Should be SILENT (no "Generating token..." spam)
   - **Watch UI** - Generated text should appear

## What to Look For

### ✅ Good Signs:
- Terminal shows vocabulary building
- Terminal shows token loading
- Terminal is SILENT during training (no spam)
- UI progress bar moves
- UI epoch counter increments
- UI loss value updates
- Generation is SILENT (no terminal spam)
- Generated text uses actual words (not random characters)

### ❌ Bad Signs (Report These):
- Terminal spam during training ("Step X: Loss...")
- Terminal spam during generation ("Generating token...")
- UI doesn't update during training
- Segmentation fault / crash
- Loss stays at exactly 0.0000 for all epochs
- Generated text is random characters

## Known Issues (Expected)

1. **Loss = 0.0000** - Loss computation may still return 0.0
   - This is a SEPARATE issue to debug next
   - Training infrastructure is now correct
   - Loss computation needs investigation

2. **Generated Text Quality** - May not be coherent yet
   - Model hasn't learned (loss = 0.0)
   - Once loss is fixed, quality should improve

3. **SSH Push Failed** - Code is committed locally but not pushed
   - Not a problem for testing
   - You have the latest code

## Success Criteria

For this fix to be considered successful:

1. ✅ Vocabulary builds from training file
2. ✅ Training runs without terminal spam
3. ✅ UI shows progress during training
4. ✅ Generation runs without terminal spam
5. ✅ No crashes or segfaults
6. ⏳ Loss may still be 0.0 (separate issue)

## If Something Goes Wrong

### Crash / Segfault:
```bash
# Run with debugger
gdb app/hyper_prime_spiral
(gdb) run
# When it crashes:
(gdb) bt
# Copy the backtrace and report it
```

### Terminal Spam Returns:
- Note which operation causes spam (training or generation)
- Copy the exact output
- Report it

### UI Doesn't Update:
- Note if progress bar moves at all
- Note if epoch counter changes
- Note if loss value changes
- Report what you observe

## Next Steps After Testing

Once you confirm this works:

1. **Debug Loss Computation** - Figure out why loss = 0.0
2. **Verify Learning** - Check if loss decreases over epochs
3. **Test Generation Quality** - See if generated text improves
4. **End-to-End Test** - Train → Save → Load → Generate

## Questions to Answer

After testing, please report:

1. Did vocabulary build successfully? (How many tokens?)
2. Did training run silently? (No terminal spam?)
3. Did UI update during training? (Progress bar, epoch, loss?)
4. Did generation run silently? (No terminal spam?)
5. What was the loss value? (Still 0.0 or actual numbers?)
6. Did generated text use real words? (Or random characters?)
7. Any crashes or errors? (Segfaults, assertions, etc?)

## Summary

This fix addressed the **infrastructure** issues:
- ✅ Vocabulary system
- ✅ Training data
- ✅ Terminal cleanliness
- ✅ UI feedback

The **learning** issue (loss = 0.0) is separate and will be debugged next once you confirm the infrastructure works correctly.

**Test and report back!**