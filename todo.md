# TODO: URGENT - Multiple Critical Issues

## CRITICAL ISSUE 1: UI Shows Nothing During Preprocessing
- [ ] Add preprocessing phase detection
- [ ] Add data loading status display
- [ ] Add tokenization progress display
- [ ] Add batch creation progress display
- [ ] Show "Loading data..." / "Tokenizing..." / "Creating batches..." messages
- [ ] Update UI BEFORE training starts

## CRITICAL ISSUE 2: Thread Status Not Updating
- [ ] Debug why sphere visualization not updating
- [ ] Verify metrics callback is being invoked
- [ ] Check if training_metrics pointer is valid
- [ ] Add debug logging to callback
- [ ] Fix real-time thread status display

## CRITICAL ISSUE 3: Complete Other Tabs
- [ ] Video Tab - Complete implementation (currently stub)
- [ ] LLM Tab - Add model creation/saving
- [ ] Benchmark Tab - Verify functionality
- [ ] Research Tab - Verify functionality

## Priority Order
1. Fix thread status updates (HIGHEST - user can't see anything)
2. Add preprocessing status (HIGH - user doesn't know what's happening)
3. Complete Video tab (HIGH - currently just "Coming Soon")
4. Complete LLM tab (MEDIUM - needs model management)