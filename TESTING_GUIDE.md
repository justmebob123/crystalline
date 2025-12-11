# Crawler Training Testing Guide

**Date:** 2024-12-02
**Build Status:** ✅ ZERO errors, ZERO warnings

---

## PRE-TESTING CHECKLIST

**Build Verification:**
- ✅ All libraries compiled successfully
- ✅ All tools compiled successfully  
- ✅ Zero compilation errors
- ✅ Zero compilation warnings
- ✅ Application built successfully

**Files Modified:**
- ✅ src/crawler/crawler_api.c - Model preparation added
- ✅ src/ai/cllm_model_manager.c - get_first_name() added
- ✅ app/ui/tabs/tab_training.c - Function signature fixed

---

## TESTING PROCEDURE

### Step 1: Build and Run Application

```bash
# From repository root
make clean && make

# Run application
cd app
LD_LIBRARY_PATH=.. ./hyper_prime_spiral
```

### Step 2: Navigate to Training Tab

1. Launch the application
2. Click on the "Training" tab
3. Observe the initial state

### Step 3: Start Crawler

1. Enter a URL in the crawler URL field (or leave empty to use database)
2. Click "START CRAWLER" button
3. **WATCH THE CONSOLE OUTPUT CAREFULLY**

### Step 4: Verify Model Preparation (CRITICAL)

**Expected Console Output:**
```
=== Starting Crawler ===
Using URLs from database (no start URL)
Data directory: crawler_data
⚠ No model specified, will use first available model

Preparing first available model: model
✓ Model prepared successfully
✓ Using first available model from model manager

Starting crawler with 7 threads per stage
[Crawler threads starting...]
```

**Key Messages to Look For:**
- ✅ "Preparing first available model: [name]"
- ✅ "✓ Model prepared successfully"
- ✅ "✓ Using first available model"

**If You See These Instead (OLD BEHAVIOR - BAD):**
- ❌ "Model 'model' is not accessible"
- ❌ "Failed to acquire model"
- ❌ No training threads starting

### Step 5: Monitor Training Activity

**Console Output to Watch:**
```
[CRAWLER DOWNLOADED] Downloaded X new page(s) (Total: X)
[CRAWLER PREPROCESSED] Preprocessed X new page(s) (Total: X)
[CRAWLER TOKENIZED] Tokenized X new page(s) (Total: X)
[CRAWLER TRAINED] Trained X new page(s) (Total: X)  ← THIS IS NEW!
```

**CRITICAL:** You should now see **[CRAWLER TRAINED]** messages!

### Step 6: Check CPU Usage

**Before Fix:**
- CPU usage: ~50% (1 core downloading only)
- Only download/preprocess/tokenize threads active
- NO training happening

**After Fix (Expected):**
```bash
# In another terminal, monitor CPU usage
top -p $(pgrep hyper_prime_spiral)

# Or use htop for better visualization
htop -p $(pgrep hyper_prime_spiral)
```

**Expected CPU Usage:**
- CPU usage: 100%+ (multiple cores)
- Download thread: 1 core
- Preprocessor threads: 7 cores
- Tokenizer threads: 7 cores
- **Training threads: 7 cores** ← THIS IS NEW!

### Step 7: Verify Training Progress in UI

**Training Tab Should Show:**
- Sphere visualization updating
- Training metrics (if available)
- Batch progress
- Loss values (if training is active)

---

## EXPECTED RESULTS

### ✅ SUCCESS INDICATORS

1. **Model Preparation:**
   - Console shows "Preparing model..."
   - Console shows "✓ Model prepared successfully"
   - No "not accessible" errors

2. **Training Threads:**
   - Console shows training thread initialization
   - [CRAWLER TRAINED] messages appear
   - Training counter increases

3. **CPU Usage:**
   - Significantly higher than before (100%+)
   - Multiple cores active
   - Training threads consuming CPU

4. **UI Updates:**
   - Sphere visualization active
   - Training metrics updating
   - Progress indicators working

### ❌ FAILURE INDICATORS

1. **Model Not Accessible:**
   - "Model 'X' is not accessible" errors
   - "Failed to acquire model" errors
   - No training threads starting

2. **No Training:**
   - No [CRAWLER TRAINED] messages
   - Training counter stays at 0
   - CPU usage stays low (~50%)

3. **Crashes:**
   - Application crashes on crawler start
   - Segmentation faults
   - Memory errors

---

## TROUBLESHOOTING

### Issue: "Model not accessible"

**Cause:** Model preparation failed
**Solution:**
1. Check if model file exists in ./models/
2. Check model file permissions
3. Check console for abacus expansion messages
4. Verify model file is not corrupted

### Issue: No training threads starting

**Cause:** Training initialization failed
**Solution:**
1. Check console for error messages
2. Verify model was prepared successfully
3. Check if training data directory exists
4. Verify sufficient disk space

### Issue: Low CPU usage

**Cause:** Training not actually running
**Solution:**
1. Verify [CRAWLER TRAINED] messages appear
2. Check if training threads initialized
3. Monitor training queue directory
4. Check for error messages in console

### Issue: Application crashes

**Cause:** Memory or threading issue
**Solution:**
1. Run with valgrind to check for memory leaks
2. Check console for stack traces
3. Verify all libraries are up to date
4. Check for null pointer dereferences

---

## PERFORMANCE BENCHMARKS

### Before Fix

**CPU Usage:**
- Total: ~50% (1 core)
- Download: 1 core active
- Preprocess: 7 cores idle
- Tokenize: 7 cores idle
- Training: 0 cores (NOT RUNNING)

**Throughput:**
- Pages downloaded: ~1-2 per minute
- Pages preprocessed: ~1-2 per minute
- Pages tokenized: ~1-2 per minute
- Pages trained: **0** (BROKEN)

### After Fix (Expected)

**CPU Usage:**
- Total: 100%+ (multiple cores)
- Download: 1 core active
- Preprocess: 7 cores active
- Tokenize: 7 cores active
- Training: **7 cores active** (FIXED!)

**Throughput:**
- Pages downloaded: ~1-2 per minute
- Pages preprocessed: ~5-10 per minute
- Pages tokenized: ~5-10 per minute
- Pages trained: **~5-10 per minute** (NOW WORKING!)

---

## TESTING CHECKLIST

**Pre-Test:**
- [ ] Clean build completed
- [ ] Zero errors, zero warnings
- [ ] Application launches successfully
- [ ] Can navigate to Training tab

**During Test:**
- [ ] Crawler starts without errors
- [ ] Model preparation messages appear
- [ ] Model prepared successfully
- [ ] Training threads initialize
- [ ] [CRAWLER TRAINED] messages appear
- [ ] CPU usage increases significantly
- [ ] Multiple cores active
- [ ] UI updates with training progress

**Post-Test:**
- [ ] Crawler can be stopped cleanly
- [ ] No memory leaks detected
- [ ] No crashes or errors
- [ ] Training data saved correctly
- [ ] Model state preserved

---

## REPORTING RESULTS

**If Testing Succeeds:**
Report:
- ✅ Model preparation working
- ✅ Training threads starting
- ✅ CPU usage increased to X%
- ✅ Training progress visible
- ✅ No errors or crashes

**If Testing Fails:**
Report:
- ❌ Specific error messages from console
- ❌ CPU usage percentage
- ❌ Which step failed
- ❌ Any crash logs or stack traces
- ❌ Screenshots of UI state

---

## NEXT STEPS AFTER TESTING

**If Test Passes:**
1. Proceed with UI improvements (OBJECTIVE 26)
2. Add model selector to training tab
3. Add custom model name input
4. Add "Load Selected Model" button

**If Test Fails:**
1. Analyze error messages
2. Debug model preparation
3. Check training thread initialization
4. Fix identified issues
5. Retest

---

## ADDITIONAL NOTES

**Model Files:**
- Location: `./models/`
- Format: `.cllm` files
- Current models: Check with `ls -lh models/`

**Training Data:**
- Location: `crawler_data/`
- Subdirectories:
  - `raw_pages/` - Downloaded HTML
  - `preprocessed/` - Cleaned text
  - `training_queue/` - Tokenized data
  - `trained/` - Completed training files

**Logs:**
- Console output shows all activity
- Can redirect to file: `./hyper_prime_spiral 2>&1 | tee test.log`

---

**END OF TESTING GUIDE**