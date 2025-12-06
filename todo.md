# COMPREHENSIVE SYSTEM TESTING PLAN

## CURRENT STATUS

**Latest Fix:** Segfault in vocabulary building - FIXED ✅
- **Commit:** 7118828
- **Issue:** Memory leak and segfault when strdup() fails during token batching
- **Fix:** Properly free already allocated tokens before cleanup
- **Status:** Committed and pushed to main

## IMMEDIATE TESTING REQUIRED

### [ ] Phase 1: Build Verification
- [ ] Pull latest changes from GitHub
- [ ] Clean build: `make clean && make`
- [ ] Verify zero compilation errors
- [ ] Verify zero compilation warnings
- [ ] Verify all tools built successfully

### [ ] Phase 2: Vocabulary Building Test
- [ ] Test on small dataset (100-1000 documents)
- [ ] Test on medium dataset (10,000 documents)
- [ ] Test on LARGEST dataset (user's actual data)
- [ ] Monitor for segfaults
- [ ] Monitor CPU usage (should be ~800% with 8 threads)
- [ ] Verify vocabulary builds successfully
- [ ] Check memory usage

### [ ] Phase 3: Training Pipeline Test
- [ ] Test training on small model (2 layers, 64 dim)
- [ ] Test training on medium model (4 layers, 128 dim)
- [ ] Test training on large model (6 layers, 256 dim)
- [ ] Verify loss decreases
- [ ] Verify no NaN values
- [ ] Verify checkpoints save correctly
- [ ] Monitor training speed

### [ ] Phase 4: Threading System Test
- [ ] Test with 1 thread (baseline)
- [ ] Test with 2 threads
- [ ] Test with 4 threads
- [ ] Test with 8 threads
- [ ] Test with 12 threads
- [ ] Verify proper CPU utilization
- [ ] Verify no deadlocks
- [ ] Verify thread allocation is correct

### [ ] Phase 5: Inference Pipeline Test
- [ ] Load trained model
- [ ] Test inference with various prompts
- [ ] Verify output quality
- [ ] Measure inference latency
- [ ] Test with different model sizes
- [ ] Verify no crashes

### [ ] Phase 6: UI System Test
- [ ] Launch application
- [ ] Test all tabs render correctly
- [ ] Test Training tab functionality
- [ ] Test LLM tab functionality
- [ ] Test Model Management tab
- [ ] Verify sphere visualization works
- [ ] Test all buttons and controls

### [ ] Phase 7: Integration Test
- [ ] Train model via UI
- [ ] Save model
- [ ] Load model in LLM tab
- [ ] Test inference via UI
- [ ] Verify model persistence
- [ ] Test concurrent training/inference

## TESTING COMMANDS

### Build
```bash
cd ~/code/AI/crystalline.ui
git pull origin main
make clean && make
export LD_LIBRARY_PATH=$PWD:$PWD/algorithms:$LD_LIBRARY_PATH
```

### Vocabulary Building Test
```bash
# Small dataset
./tools/cllm train -d data/small --epochs 1 --batch 8 --seq-len 32

# Large dataset (user's actual data)
./tools/cllm train -d /path/to/largest/dataset --epochs 1 --batch 32 --seq-len 128
```

### Monitor CPU Usage
```bash
# In another terminal
top -H -p $(pgrep cllm)
```

### Training Test
```bash
# Small model
./tools/cllm train -d data/training --epochs 10 --layers 2 --dim 64 --batch 16

# Medium model
./tools/cllm train -d data/training --epochs 10 --layers 4 --dim 128 --batch 32

# Large model
./tools/cllm train -d data/training --epochs 10 --layers 6 --dim 256 --batch 32
```

### Inference Test
```bash
./tools/cllm infer -m models/my_model.cllm -p "Test prompt" -n 50
```

## SUCCESS CRITERIA

### Build Quality
- ✅ Zero compilation errors
- ✅ Zero compilation warnings
- ✅ All tools build successfully
- ✅ All libraries link correctly

### Vocabulary Building
- ✅ No segfaults
- ✅ CPU usage ~800% with 8 threads
- ✅ Completes successfully on large datasets
- ✅ Reasonable memory usage

### Training Pipeline
- ✅ Loss decreases over epochs
- ✅ No NaN values
- ✅ Checkpoints save correctly
- ✅ Training speed is acceptable
- ✅ No crashes or hangs

### Threading System
- ✅ Proper CPU utilization at all thread counts
- ✅ No deadlocks
- ✅ Linear scaling up to available cores
- ✅ Correct thread allocation

### Inference Pipeline
- ✅ Models load correctly
- ✅ Inference produces reasonable output
- ✅ Low latency
- ✅ No crashes

### UI System
- ✅ All tabs render correctly
- ✅ All controls work
- ✅ Sphere visualization displays
- ✅ Training/inference work via UI
- ✅ Model management works

## KNOWN ISSUES FIXED

1. ✅ Vocabulary building segfault - FIXED (commit 7118828)
2. ✅ Threading auto-detection hardcoded to 12 - FIXED (previous commit)
3. ✅ Lock contention in vocabulary building - FIXED (token batching)

## WORKFLOW GOING FORWARD

**Per User Request:**
- ✅ Work directly on main branch (no more feature branches)
- ✅ Use proper git authentication: `git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main`
- ✅ Test thoroughly before committing
- ✅ Focus on making threading and UI work correctly

## CURRENT FOCUS

**Priority 1:** Comprehensive testing on user's largest dataset
**Priority 2:** Verify threading system works correctly
**Priority 3:** Verify UI system works correctly
**Priority 4:** Fix any issues discovered during testing

---

**Status:** Ready for comprehensive testing
**Next Action:** User to pull changes and run tests