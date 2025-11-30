# TODO - Crystalline CLLM Project

## RULES (ALWAYS FOLLOW)

### RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE
**HIGHEST PRIORITY RULE - MUST BE FOLLOWED WITH EVERY SINGLE RESPONSE**

At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

### RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
Before taking ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Understand the current objectives
3. Verify your action aligns with the master plan
4. Check for any blocking priorities

### RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
The AUDIT.md contains:
- Current architectural violations
- Required fixes with priorities
- Implementation phases
- Testing requirements
- Success criteria

### RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
The SECONDARY_OBJECTIVES.md contains:
- Detailed implementation tasks
- Code examples
- File-by-file changes
- Testing procedures
- Validation steps

### RULE 4: DO NOT CREATE NEW MD FILES
All documentation goes in existing files or this master plan only.

### RULE 5: ALWAYS COMMIT ALL CHANGES USING CORRECT AUTHENTICATION
```bash
git add .
git commit -m "descriptive message"
git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
```

### RULE 6: THIS FILE IS READ-ONLY - DO NOT EDIT WITHOUT EXPLICIT APPROVAL
**CRITICAL**: MASTER_PLAN.md contains OBJECTIVES ONLY - NO status updates, NO ephemeral information.

---

## CRITICAL ISSUES (USER REPORTED - ROUND 3)

### Issue 1: URL Input Field Not Clickable (CRITICAL)
- [ ] URL field does not highlight when clicked
- [ ] Clicks not detected - hotkeys execute instead
- [ ] Input manager not receiving click events
- [ ] Need to debug input bounds and click detection

### Issue 2: Models Tab Completely Broken
- [ ] Only shows "blocky areas"
- [ ] No actual content rendering
- [ ] Need to investigate draw_model_list and draw_model_details
- [ ] May have incomplete implementation

---

## INVESTIGATION PLAN

### Phase 1: Debug URL Input Field
1. [ ] Add logging to show registered input bounds
2. [ ] Add logging to show click coordinates
3. [ ] Check if crawler tab is setting current_tab correctly
4. [ ] Verify input bounds are in correct coordinate space
5. [ ] Test if bounds overlap with sidebar/submenu

### Phase 2: Fix Models Tab
1. [ ] Check draw_model_list implementation
2. [ ] Check draw_model_details implementation
3. [ ] Verify model manager is returning data
4. [ ] Complete any TODO sections in rendering

---

## FIXES APPLIED (ROUND 3)

### Issue 1: URL Input Field - DEBUGGING ADDED
- [x] Added extensive logging to input_manager.c:
  - Logs all input registrations with bounds
  - Logs all click events with coordinates and current_tab
  - Logs which inputs are being checked
  - Logs tab mismatches
  - Logs successful focus operations
- [x] This will help identify why URL field not clickable

### Issue 2: Models Tab - FIXED
- [x] Completed all TODO sections in draw_model_list()
- [x] Completed all TODO sections in draw_model_details()
- [x] Completed all TODO sections in draw_action_buttons()
- [x] Completed all TODO sections in draw_status_message()
- [x] Fixed in_use field (should be read_count > 0)
- [x] Tab now shows:
  - Model list with names, status, and size info
  - Model details panel with full configuration
  - Action buttons (Create, Load, Delete)
  - Status messages

### Build Status:
Zero errors, zero warnings

---

## STATUS: ROUND 3 FIXES COMPLETE - READY FOR TESTING WITH LOGGING