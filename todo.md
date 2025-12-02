# TODO - Fix Build Warnings and Complete UI Rewrite

## RULES (PASTED FROM MASTER_PLAN)
- RULE 0: Always paste rules to top of todo.md
- RULE 1: Always reread MASTER_PLAN.md before any action
- RULE 2: Reference AUDIT.md for architectural state
- RULE 3: Reference SECONDARY_OBJECTIVES.md for detailed tasks
- RULE 4: Do not create new MD files
- RULE 5: Always commit all changes using correct authentication
- RULE 6: MASTER_PLAN.md is READ-ONLY
- RULE 7: Fix all build warnings before proceeding

## CRITICAL ISSUES

### Issue 1: Build Warnings (MASTER_PLAN RULE 7)
Two format truncation warnings in tab_training.c:
- Line 894: snprintf may truncate "Model: %s (Not Loaded)" 
- Line 881: snprintf may truncate "Model: %s (Loaded)"

Buffer is 256 bytes, but model name can be 255 bytes, leaving insufficient space for format string.

**Fix**: Increase buffer size or truncate model name before formatting.

### Issue 2: Application OOM Kill
Application is being killed by system (Out of Memory).
Need to investigate what's consuming memory.

**Possible causes**:
- Model loading still happening somewhere
- Memory leak in initialization
- Large allocations during startup

## Fix Plan

### Phase 1: Fix Build Warnings (IMMEDIATE)
- [x] Identify warnings (lines 881, 894)
- [x] Fix buffer size issue (increased to 512 bytes, truncate names to 200 chars)
- [x] Rebuild and verify zero warnings
- [ ] Commit fix

### Phase 2: Investigate OOM Kill
- [x] Found root cause: Multiple tabs loading models in callbacks
- [x] LLM tab: model_manager_acquire_read() in on_model_selected (line 86)
- [x] Research tab: model_manager_acquire_read() in on_model_selected (line 50)
- [x] Models tab: model_manager_acquire_read() on double-click (line 628) - INTENTIONAL
- [x] Crawler tab: model_manager_acquire_write() in callback (line 93)
- [x] Fixed LLM tab to use lazy loading (load on Send)
- [x] Fixed Research tab to use lazy loading (load on analysis)
- [x] Fixed Crawler tab to use lazy loading (load on crawl start)
- [x] Rebuild successful - zero warnings
- [ ] Commit and test

### Phase 3: Complete UI Layout Rewrite (AFTER FIXES)
- [ ] Design new layout system for all tabs
- [ ] Implement responsive layout engine
- [ ] Apply to Training Tab
- [ ] Apply to Models Tab
- [ ] Apply to LLM Tab
- [ ] Apply to Research Tab
- [ ] Apply to Benchmark Tab
- [ ] Apply to Adapters Tab