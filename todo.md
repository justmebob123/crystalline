# TODO: URGENT - Multiple Critical Issues

## CRITICAL ISSUE 1: UI Shows Nothing During Preprocessing ✅ FIXED
- [x] Add preprocessing phase detection
- [x] Add data loading status display
- [x] Add "Building vocabulary..." message
- [x] Add "Loading training data..." message
- [x] Add "Initializing 12 training threads..." message
- [x] Add progress bar (0% → 10% → 30% → 80% → 100%)
- [x] Add SDL_PumpEvents() to allow UI updates
- [ ] TEST: Verify status messages appear during training

## CRITICAL ISSUE 2: Thread Status Not Updating ⚠️ DEBUG ADDED
- [x] Add debug logging to metrics callback
- [x] Callback prints every 10 invocations
- [ ] TEST: Check console for "[METRICS CALLBACK #10]" messages
- [ ] TEST: Verify sphere visualization updates
- [ ] If callback not printing: investigate why callback not invoked
- [ ] If callback printing but UI not updating: investigate UI redraw

## CRITICAL ISSUE 3: Complete Other Tabs ✅ COMPLETE
- [x] Video Tab - Complete implementation ✅
  - [x] Create tab_video.c with full UI
  - [x] Add recording status display
  - [x] Add frame counter
  - [x] Add start/stop button
  - [x] Add requirements check (ffmpeg)
  - [x] Integrate with main.c
  - [x] Build and test compilation
- [x] LLM Tab - Fixed memory leak ✅
  - [x] Model creation already implemented
  - [x] Model saving already implemented
  - [x] Fixed memory leak (cllm_free_model)
- [x] Benchmark Tab - Already functional ✅
- [x] Research Tab - Already functional ✅

## IMMEDIATE NEXT STEPS
1. **USER TESTING** - Start training and verify:
   - Status messages appear during preprocessing
   - Progress bar updates
   - Console shows callback messages
   - Thread status updates during training
2. **If issues persist** - Check console output for debug messages
3. **Complete other tabs** - After UI feedback is confirmed working