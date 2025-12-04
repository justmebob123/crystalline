# 🔒 MASTER PLAN RULES (READ FIRST - MANDATORY)

## ⭐ RULE 0: ALWAYS READ THESE RULES FIRST ⭐
**MANDATORY WITH EVERY RESPONSE**

Before ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Read AUDIT.md for current architectural state
3. Read SECONDARY_OBJECTIVES.md for detailed tasks
4. Update todo.md with current progress

## RULE 1: CRITICAL REFERENCE FILES
**Before making ANY code changes, ALWAYS check these files:**

1. **app/ui/sphere_visualization.c** - 3D kissing spheres visualization
   - PURE crystalline mathematics ONLY (NO math.h)
   - Uses ONLY prime_* functions from crystalline library
   - Implements icosahedral geometry with golden ratio
   - 13 spheres: 1 control + 12 workers

2. **src/geometry/clock_lattice.c** - Babylonian clock structure
   - Ring structure: 12, 60, 60, 100
   - Prime position mapping

3. **src/ai/cllm_kissing_spheres.c** - Kissing spheres implementation
   - 12 neighbors per point
   - Shared memory structure

**CRITICAL**: Never use math.h or standard library functions. Only use prime_* functions.

## RULE 2: GIT OPERATIONS
**ALWAYS use correct authentication:**
```bash
git add .
git commit -m "descriptive message"
git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git feature/crystalline-ui-system
```

## RULE 3: THIS FILE IS READ-ONLY
**⚠️ DO NOT EDIT MASTER_PLAN.md WITHOUT EXPLICIT APPROVAL ⚠️**

## RULE 4: BUILD VERIFICATION
**MANDATORY: Test every build after making changes**

Process (NEVER SKIP):
1. Make code changes
2. Run: `make clean && make 2>&1 | tee build.log`
3. Count warnings: `grep -c "warning:" build.log`
4. **VERIFY BUILD SUCCESS**
5. Fix all warnings
6. Rebuild and verify
7. **ONLY THEN** commit changes

## RULE 5: FIX HTML ENTITIES IMMEDIATELY
When creating files, HTML entities may be introduced.

**Solution:**
```bash
python3 tools/fix_html_entities.py <file>
```

---

# Training Tab - Critical UI Fixes Required

## Current Issues (From Screenshot Analysis)

### CRITICAL LAYOUT ISSUES
- [ ] Panels going off screen (top-left especially)
- [ ] Nothing is centered correctly
- [ ] Sliders overlapping their labels
- [ ] Sliders are too large
- [ ] Missing file list display
- [ ] Missing file selection checkboxes/toggles
- [ ] Information layout is poor
- [ ] Some information appears missing or off-screen

### BUTTON ISSUES
- [ ] Start/Pause/Save buttons are too large
- [ ] Button sizes inconsistent across tabs
- [ ] No hover animations on buttons
- [ ] 2D/3D toggle button doesn't work
- [ ] Need global button size constants

### MISSING FUNCTIONALITY
- [ ] File list not visible
- [ ] File selection toggles not visible
- [ ] Some metrics may be missing
- [ ] Need to verify all wiring is correct

## Action Plan

### Phase 1: Analyze Old Code for Missing Information
- [x] Compare old code with new implementation
- [x] List ALL information that was displayed
- [x] List ALL interactive elements
- [x] Document exact layout and positioning
- [x] Create TRAINING_TAB_ISSUES_ANALYSIS.md

### Phase 2: Create Global Button Size Constants
- [x] Define standard button sizes (small, medium, large)
- [x] Create constants file for UI sizing (app/ui/button_sizes.h)
- [x] Apply to Training Tab
- [ ] Apply to other tabs (Research, Models, etc.)
- [ ] Ensure hover animations work

### Phase 3: Fix Layout and Positioning
- [x] Fix slider positioning and sizing
- [x] Ensure proper spacing between elements (using constants)
- [x] Fix slider label overlap
- [ ] Fix panels going off screen (needs runtime testing)
- [ ] Center all elements correctly (needs runtime testing)
- [ ] Make sure all content is visible (needs runtime testing)

### Phase 4: Restore Missing Elements
- [x] Add file list display
- [x] Add file selection checkboxes
- [x] Make checkboxes clickable
- [x] Show file count and selected count
- [ ] Restore any missing metrics (needs verification)
- [ ] Verify all information is present (needs runtime testing)

### Phase 5: Fix Button Functionality
- [x] Wire 2D/3D toggle correctly
- [x] Reduce button sizes appropriately
- [x] Update 2D/3D button label dynamically
- [ ] Add hover animations to all buttons
- [ ] Test all button callbacks (needs runtime testing)

### Phase 6: Verify Wiring
- [ ] Test all buttons work
- [ ] Test all sliders update values
- [ ] Test file selection works
- [ ] Test training actually starts
- [ ] Verify all callbacks are connected

## Button Size Standards (To Be Created)
```c
// Global button size constants
#define BUTTON_SIZE_SMALL 30    // Tertiary actions
#define BUTTON_SIZE_MEDIUM 40   // Secondary actions
#define BUTTON_SIZE_LARGE 50    // Primary actions
#define BUTTON_SIZE_TINY 20     // Utility buttons (2D/3D toggle)
```

## Key Principles
1. ALL information from old code must be present
2. ALL interactive elements must work
3. Button sizes must be consistent across tabs
4. Layout must be properly centered and visible
5. Hover animations must work on all buttons
6. File list and selection must be visible and functional