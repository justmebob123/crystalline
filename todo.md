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

# CRITICAL UNDERSTANDING: FIX THE CRYSTALLINE UI LIBRARY

## The Real Problem
The Crystalline UI library (app/ui/crystalline/) is INCOMPLETE and needs features added.
**DO NOT bypass it with manual rendering - FIX THE LIBRARY ITSELF.**

## Crystalline UI Library Issues to Fix

### 1. CrystallineList Needs Checkboxes
**Current:** CrystallineList only shows text items
**Needed:** Support for checkboxes/toggles per item
**Fix:** Add checkbox support to elements.c/elements.h

### 2. CrystallineButton Needs Hover Animations
**Current:** Buttons have no hover state
**Needed:** Smooth hover animations with color transitions
**Fix:** Implement hover state in button rendering

### 3. CrystallineSlider Needs Better Sizing
**Current:** Sliders may be too large
**Needed:** Configurable handle size and track height
**Fix:** Add size parameters to slider creation

### 4. CrystallinePanel Positioning Issues
**Current:** Panels may go off screen
**Needed:** Proper bounds checking and centering
**Fix:** Add layout validation in panel creation

### 5. Missing Features in Elements
**Current:** Limited element types
**Needed:** 
- List with checkboxes
- Better dropdown rendering
- Progress bars with text overlay
- Hover states for all elements

## Action Plan - Fix Crystalline UI Library

### Phase 1: Add Checkbox Support to CrystallineList
- [x] Add `bool* item_checked` array to CrystallineList struct
- [x] Add checkbox rendering in crystalline_list_render()
- [x] Add checkbox click detection in crystalline_list_handle_mouse()
- [x] Add API: crystalline_list_set_item_checked()
- [x] Add API: crystalline_list_get_item_checked()
- [x] Add callback for checkbox changes

### Phase 2: Implement Hover Animations
- [ ] Add hover state detection to all elements
- [ ] Implement color interpolation for smooth transitions
- [ ] Add hover callbacks
- [ ] Test hover on buttons, sliders, list items

### Phase 3: Fix Button Sizing
- [ ] Verify button size parameters work correctly
- [ ] Test with global size constants
- [ ] Ensure consistent sizing across all button types

### Phase 4: Fix Panel Layout
- [ ] Add bounds checking to prevent off-screen rendering
- [ ] Implement auto-centering option
- [ ] Add padding/margin support
- [ ] Test with different screen sizes

### Phase 5: Enhance Slider Rendering
- [ ] Make handle size configurable
- [ ] Make track height configurable
- [ ] Improve label positioning
- [ ] Add value display options

### Phase 6: Apply Fixed Library to Training Tab
- [ ] Use CrystallineList with checkboxes for file list
- [ ] Use proper button hover states
- [ ] Use enhanced sliders
- [ ] Verify all elements work correctly

## Current Status
- [x] Identified that manual rendering is WRONG approach
- [x] Understood need to fix Crystalline UI library itself
- [x] Fix CrystallineList to support checkboxes
- [ ] Fix hover animations (Phase 2 - Next)
- [ ] Fix sizing issues (Phase 2 - Next)
- [x] Apply to Training Tab using ONLY Crystalline UI elements

## Key Principle
**FIX THE UI LIBRARY - DON'T BYPASS IT**
All UI elements must use the Crystalline UI library.
If the library is missing features, ADD THEM TO THE LIBRARY.