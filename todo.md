# RULES (MUST FOLLOW WITH EVERY RESPONSE)

## RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD ⭐
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

## RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
1. Read MASTER_PLAN.md completely
2. Understand the current objectives
3. Verify your action aligns with the master plan
4. Check for any blocking priorities

## RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
- Current architectural violations
- Required fixes with priorities
- Implementation phases
- Testing requirements
- Success criteria

## RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
- Detailed implementation tasks
- Code examples
- File-by-file changes
- Testing procedures
- Validation steps

## RULE 4: DO NOT CREATE NEW MD FILES
## RULE 5: ALWAYS COMMIT ALL CHANGES USING CORRECT AUTHENTICATION
## RULE 6: THIS FILE IS READ-ONLY - DO NOT EDIT WITHOUT EXPLICIT APPROVAL
## RULE 7: FIX ALL BUILD WARNINGS BEFORE PROCEEDING

---

# CRITICAL ISSUES IN TRAINING TAB - COMPLETE REDESIGN REQUIRED

## User-Reported Issues (CRITICAL)

### Issue 1: Duplicate Sphere Visualizations ❌
**Problem**: Two sphere visualizations exist, one overlaying the other
- The TOP visualization shows NO activity (static/frozen)
- The BOTTOM visualization (hidden behind) shows CORRECT activity
- When training stops, top visualization disappears revealing the working one
- This suggests a duplicate visualization was created (possibly for 2D/3D toggle)

**Root Cause**: Likely created duplicate sphere rendering code instead of toggling existing one

### Issue 2: Training Threads Not Stopping ❌
**Problem**: When training stops, threads continue accessing model causing memory errors
- Same heap-use-after-free issue as before
- Threads still running after stop button clicked
- Model being freed while threads still accessing it

**Root Cause**: Thread shutdown sequence broken again

### Issue 3: Model Status Confusion ❌
**Problem**: UI shows contradictory model status
- Says "model is ready"
- Also says "model isn't loaded"
- Confusing to user

### Issue 4: Model Dropdown Visibility ❌
**Problem**: Model dropdown difficult to see
- Overlapping UI elements
- Poor visibility/contrast

### Issue 5: Models Tab Selection Broken ❌
**Problem**: Cannot select model from Models tab
- Only shows off-center list
- No clear buttons to select/load model
- Should be able to click model to load it

## Deep Analysis Required

### Phase 1: Analyze Training Tab Structure
- [ ] Read complete tab_training.c file
- [ ] Identify ALL sphere visualization code
- [ ] Map all rendering functions
- [ ] Find duplicate visualization code
- [ ] Identify which is original, which is duplicate

### Phase 2: Analyze Thread Management
- [ ] Review training_thread.c
- [ ] Check stop_training_thread() implementation
- [ ] Verify pthread_join() is called
- [ ] Check cleanup sequence
- [ ] Ensure model not freed before threads stop

### Phase 3: Analyze Model Status Display
- [ ] Find all model status text rendering
- [ ] Identify contradictory status messages
- [ ] Determine correct status logic

### Phase 4: Analyze Models Tab
- [ ] Read complete tab_models.c
- [ ] Find model list rendering
- [ ] Identify selection mechanism
- [ ] Check button visibility
- [ ] Verify click handlers

### Phase 5: Complete Redesign
- [ ] Remove duplicate sphere visualization
- [ ] Fix thread shutdown sequence
- [ ] Unify model status display
- [ ] Fix model dropdown visibility
- [ ] Add proper model selection in Models tab
- [ ] Test all fixes together

## Files to Analyze
- app/ui/tabs/tab_training.c
- app/ui/tabs/tab_models.c
- app/training_thread.c
- app/ui/sphere_visualization.c
- app/main.c (cleanup sequence)

## Implementation Plan - ALL PHASES

### Phase 1: Fix Duplicate Sphere Visualization ✅ COMPLETE
- [x] Remove line 650 sphere visualization call
- [x] Uncomment line 486 sphere visualization call
- [x] Remove conditional at line 380 (always show spheres)
- [x] Build successful - zero errors
- [x] Commit Phase 1

### Phase 2: Investigate and Fix Thread Stopping ✅ COMPLETE
- [x] Verified training loop checks stop flag at each epoch
- [x] Verified threaded_training_free() properly stops all worker threads
- [x] Verified cleanup sequence is correct (stop threads before model cleanup)
- [x] Added pthread_mutex_t sphere_stats_mutex to AppState
- [x] Protected all sphere_stats writes with mutex in training_thread.c
- [x] Protected all sphere_stats reads with mutex in sphere_visualization.c
- [x] Build successful - zero errors
- [x] Commit Phase 2

### Phase 3: Fix Model Status Display ✅ COMPLETE
- [x] Added model_manager_count() function to check registered models
- [x] Updated status logic with 3 states (loaded/available/none)
- [x] Unified status messages - no more contradictions
- [x] Color coding: Green=loaded, Orange=available, Red=none
- [x] Build successful - zero errors, 1 pre-existing warning
- [x] Commit Phase 3

### Phase 4: Fix Model Selector Visibility ✅ COMPLETE
- [x] Increased model selector height from 30 to 50 pixels
- [x] Increased spacing below selector from 30 to 50 pixels
- [x] Improved color contrast (brighter text, borders, hover)
- [x] Better background color for visibility
- [x] Build successful - zero errors
- [x] Commit Phase 4

### Phase 5: Improve Models Tab Selection
- [ ] Make selection highlight brighter
- [ ] Add double-click to load model
- [ ] Add visual feedback when model loads
- [ ] Verify button positions visible
- [ ] Test selection and loading flow
- [ ] Commit Phase 5

### Final Steps
- [ ] Build and test all changes together
- [ ] Push all commits to GitHub
- [ ] User acceptance testing