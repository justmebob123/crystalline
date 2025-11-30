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

## NEW CRITICAL ISSUES (USER REPORTED)

### Issue 1: AI Tab - Display Area Overlaps Sidebar
- [ ] Investigate LLM tab rendering
- [ ] Check if using old coordinate system
- [ ] Fix overlap with sidebar

### Issue 2: Models Tab - Rendering Issues
- [ ] Models tab shows only "couple large blocky areas"
- [ ] Investigate tab_models.c rendering
- [ ] Fix rendering to show proper model list

### Issue 3: URL Field Segfault (CRITICAL)
- [ ] URL field not clickable
- [ ] Double-click causes segfault
- [ ] Investigate input_manager click detection
- [ ] Fix bounds checking in URL field

---

## PREVIOUS FIXES (COMPLETED)
- [x] Phase 1: Fixed unicode characters (emoji → ASCII)
- [x] Phase 2: Updated RENDER_WIDTH to account for sidebar
- [x] Phase 2: Fixed visualization coordinate system
- [x] Phase 2: Fixed control panel positioning
- [x] Phase 2: Fixed training tab visualization area
- [x] Phase 3: Added input bounds validation
- [x] Phase 3: Added layout validation in crawler tab

---

## FIXES APPLIED (ROUND 2)

### Issue 1: AI Tab - FIXED
- [x] Fixed LLM tab control panel position (RENDER_OFFSET_X + RENDER_WIDTH)
- [x] Fixed LLM tab control panel Y (RENDER_OFFSET_Y)
- [x] Fixed chat area X position (RENDER_OFFSET_X + 10)
- [x] Fixed input rect X position (RENDER_OFFSET_X + 10)

### Issue 2: Models Tab - FIXED
- [x] Fixed window_width calculation (subtract SIDEBAR_WIDTH)
- [x] Fixed window_height calculation (subtract SUBMENU_HEIGHT)
- [x] Fixed X position (RENDER_OFFSET_X + 20)
- [x] Fixed Y position (RENDER_OFFSET_Y + 20)

### Issue 3: URL Field Segfault - FIXED
- [x] Added NULL pointer validation in click handler
- [x] Added bounds validation before checking clicks
- [x] Added bounds validation in focus function
- [x] Added extensive logging for debugging
- [x] Added error messages for invalid operations

### Files Modified:
1. app/ui/tabs/tab_llm.c - Fixed all coordinate calculations
2. app/ui/tabs/tab_models.c - Fixed all coordinate calculations
3. app/input_manager.c - Added comprehensive validation and logging

### Build Status:
Zero errors, zero warnings

---

## STATUS: ROUND 2 FIXES COMPLETE - READY FOR TESTING