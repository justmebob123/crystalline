# TODO - Crystalline CLLM Project

## RULES (PASTE TO TOP WITH EVERY RESPONSE)
- Rule 0: Always paste rules to the top of todo.md with every response
- Rule 1: Always reread MASTER_PLAN.md before any action
- Rule 2: Reference AUDIT.md for architectural state
- Rule 3: Reference SECONDARY_OBJECTIVES.md for detailed tasks
- Rule 4: Do not create new .md files
- Rule 5: Always commit all changes using correct authentication
- Rule 6: MASTER_PLAN.md is read-only - do not edit without explicit approval

## CRITICAL MEMORY LEAK - 395MB ⚠️ IN PROGRESS

### Deep Bidirectional Analysis Complete ✅
**Analysis Document**: DEEP_MEMORY_ANALYSIS.md

### Root Cause Identified ✅
**Problem**: Model manager loads models at startup but cleanup() never calls model_manager_cleanup()
**Evidence**: 
- ASAN reports 395MB leak from cllm_create_model
- model_manager_cleanup() EXISTS but is NEVER called
- continuous_training_cleanup() EXISTS and IS called (when crawler stops)
- Main cleanup() function missing model_manager_cleanup() call

### Cleanup Functions Status
- ✅ model_manager_cleanup() - EXISTS in src/ai/cllm_model_manager.c:134
- ✅ cllm_free_model() - EXISTS in src/ai/cllm_create.c:345
- ✅ continuous_training_cleanup() - EXISTS in src/crawler/continuous_training.c:482
- ✅ crawler_state_cleanup() - CALLS continuous_training_cleanup() correctly
- ❌ cleanup() in app/main.c - MISSING model_manager_cleanup() call
- ❌ cleanup() in app/main.c - MISSING stop_crawler_thread() call

### Fix Plan
**Phase 1: Add Missing Cleanup Calls (CRITICAL)**
- [ ] Add stop_crawler_thread() call to cleanup() in app/main.c
- [ ] Add model_manager_cleanup() call to cleanup() in app/main.c
- [ ] Test with ASAN to verify leak is fixed

**Phase 2: Verify Model Reuse (HIGH PRIORITY)**
- [ ] Verify crawler reuses existing models from model manager
- [ ] Check that only ONE model exists in memory during operation
- [ ] Confirm no duplicate model creation

**Phase 3: Add Memory Monitoring (MEDIUM PRIORITY)**
- [ ] Add memory usage logging at startup
- [ ] Add memory usage logging at shutdown
- [ ] Track model count in model manager

## Previous Fixes (COMPLETE)

### Issue 1: Start Crawler Requires URL Despite Database Having URLs ✅ FIXED
- Modified crawler start logic to check total URLs instead of pending
- Auto-reset crawled URLs to pending when starting with no pending URLs
- Crawler now works with just database URLs

### Issue 2: Cannot Recrawl URLs Marked as "Already Crawled" ✅ FIXED
- Added url_db_reset_all_to_pending() function
- Added "Reset All URLs" button to Crawler tab UI
- Auto-reset happens when starting crawler with no pending URLs

## Next Actions
1. ⏳ Fix memory leak by adding cleanup calls
2. ⏳ Test with ASAN to verify fix
3. ⏳ Verify model reuse logic
4. ⏳ Commit and push changes