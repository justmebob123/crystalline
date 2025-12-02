# TODO - Complete UI Layout Rewrite

## RULES (PASTED FROM MASTER_PLAN)
- RULE 0: Always paste rules to top of todo.md
- RULE 1: Always reread MASTER_PLAN.md before any action
- RULE 2: Reference AUDIT.md for architectural state
- RULE 3: Reference SECONDARY_OBJECTIVES.md for detailed tasks
- RULE 4: Do not create new MD files
- RULE 5: Always commit all changes using correct authentication
- RULE 6: MASTER_PLAN.md is READ-ONLY
- RULE 7: Fix all build warnings before proceeding

## COMPLETED FIXES

### ✅ Build Warnings Fixed
- Fixed format truncation warnings in tab_training.c
- Increased buffer to 512 bytes, truncate names to 200 chars
- Build: Zero errors, zero warnings

### ✅ OOM Kill Issue Resolved
- REVERTED lazy loading from model_manager.c (was causing OOM)
- Restored original behavior: acquire_read/write returns NULL if not loaded
- Tab-level fixes remain: tabs store model names, don't auto-load
- Models must be explicitly loaded before use

## CURRENT STATE
- Application should no longer OOM kill
- All tabs store selected model names without loading
- Models load only when explicitly requested
- Build clean with zero warnings

## NEXT: Complete UI Layout Rewrite

### Phase 1: Design New Layout System
- [ ] Analyze current layout issues across all tabs
- [ ] Design responsive layout engine
- [ ] Create reusable layout components
- [ ] Document layout patterns

### Phase 2: Implement Layout Engine
- [ ] Create layout manager system
- [ ] Implement auto-sizing and positioning
- [ ] Add responsive breakpoints
- [ ] Test with different window sizes

### Phase 3: Apply to All Tabs
- [ ] Training Tab
- [ ] Models Tab
- [ ] LLM Tab
- [ ] Research Tab
- [ ] Benchmark Tab
- [ ] Adapters Tab
- [ ] Crawler Tab

### Phase 4: Testing and Refinement
- [ ] Test all tabs with new layout
- [ ] Fix any layout issues
- [ ] Optimize performance
- [ ] Document usage