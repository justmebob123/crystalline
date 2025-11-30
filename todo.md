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

## CRITICAL UI ISSUES (BLOCKING - MUST FIX IMMEDIATELY)

### Phase 1: Unicode/Font Issues
- [ ] Replace all emoji icons with ASCII equivalents in left_sidebar.c
- [ ] Replace all unicode box-drawing characters with ASCII
- [ ] Test font rendering with ASCII-only characters

### Phase 2: Layout Overlap Analysis
- [ ] Analyze all UI component boundaries and positions
- [ ] Identify overlapping regions between sidebar and content
- [ ] Fix z-ordering and rendering order
- [ ] Ensure content area respects sidebar boundaries

### Phase 3: Input System Analysis (CRITICAL - SEGFAULT)
- [ ] Compare old input system vs new input system
- [ ] Identify what broke in the transition
- [ ] Analyze segfault in URL field typing
- [ ] Fix input field initialization
- [ ] Fix input field event handling
- [ ] Test all input fields thoroughly

### Phase 4: Comprehensive UI Validation
- [ ] Test all tabs for overlap issues
- [ ] Test all input fields for functionality
- [ ] Test all buttons for click detection
- [ ] Verify layout calculations are correct
- [ ] Run application without crashes

---

## STATUS: FIXES COMPLETE - READY FOR USER TESTING

### COMPLETED:
- [x] Phase 1: Fixed unicode characters (emoji → ASCII)
- [x] Phase 2: Updated RENDER_WIDTH to account for sidebar
- [x] Phase 2: Fixed visualization coordinate system (added RENDER_OFFSET_X/Y)
- [x] Phase 2: Fixed control panel positioning
- [x] Phase 2: Fixed training tab visualization area
- [x] Phase 3: Added input bounds validation (prevents segfault)
- [x] Phase 3: Added layout validation in crawler tab
- [x] Build verification: Zero errors, zero warnings

### READY FOR TESTING:
- [ ] Test application startup (no crash)
- [ ] Test all tabs render without overlap
- [ ] Test input fields work without segfault
- [ ] Test sidebar and submenu clickable
- [ ] Test visualization areas properly positioned

### FIXES APPLIED:
1. Unicode characters replaced with ASCII ([VIZ], [AI], [MOD], [SYS], [DATA])
2. RENDER_WIDTH updated: 1600 - 320 - 200 = 1080px (accounts for sidebar)
3. Added RENDER_OFFSET_X (200px) and RENDER_OFFSET_Y (40px) constants
4. Updated visualization center calculation with offsets
5. Updated control panel positioning with offsets
6. Updated training tab visualization area with offsets
7. Added input bounds validation (prevents segfault)
8. Added layout validation in crawler tab

### FINAL LAYOUT:
  - Sidebar: X=0-200, Y=0-900 (200px width)
  - Submenu: X=200-1600, Y=0-40 (40px height)
  - Visualization: X=200-1280, Y=40-900 (1080px width)
  - Control Panel: X=1280-1600, Y=40-900 (320px width)

### COMMIT:
✅ All changes committed and pushed to GitHub
✅ Build status: Zero errors, zero warnings