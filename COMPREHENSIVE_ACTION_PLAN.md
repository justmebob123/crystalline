# Comprehensive Action Plan - UI Fixes

## Current State Analysis

### Issue 1: Dropdown used to work, now does nothing
**Symptom:** Dropdown used to expand, now clicking does nothing
**Hypothesis:** Event handling regression - possibly conflicting handlers

**Code Analysis Needed:**
1. Check `handle_training_tab_mouse_down()` - line ~825
2. Check `handle_training_tab_mouse_up()` - line ~795
3. Check `handle_training_tab_mouse_motion()` - line ~876
4. Verify dropdown receives ALL three event types
5. Check if multiple handlers are fighting over the same event

**Exact Code Locations:**
```
app/ui/tabs/tab_training.c:
- Line 813: dropdown in mouse_up handler
- Line 837: dropdown in mouse_down handler  
- Line 899: dropdown in mouse_motion handler (ADDED RECENTLY)
```

**Potential Problem:**
- Mouse motion handler might be consuming events before down/up handlers
- Need to check event propagation order in main.c

### Issue 2: Checkbox is "just a fucking dot"
**Symptom:** Can't see checkbox state, appears as tiny dot
**Current State:** 18px circle (from library default)

**Code Analysis:**
```
app/ui/crystalline/elements.c:
- Line 1068: checkbox_size = CHECKBOX_SIZE_MEDIUM (18px)
- Line 1191: crystalline_draw_circle() renders checkbox
```

**Problem:** 18px is still too small
**Solution:** Need 24-28px minimum, with better visual styling

### Issue 3: Checkbox toggle sets 1 then 0
**Symptom:** Checkbox toggles on then immediately off
**Hypothesis:** Multiple event handlers or callback being called twice

**Code Analysis:**
```
app/ui/crystalline/elements.c:
- Line 1273: Toggle checkbox
- Line 1279: Call on_check callback
- Line 1282: return true (should stop propagation)
- Line 1286: Regular item selection (should NOT execute after checkbox)
```

**Potential Problem:**
- Event might be processed twice (BUTTONDOWN and BUTTONUP)
- Need to verify only ONE event type toggles checkbox

### Issue 4: SELECT button wired to learning rate
**Symptom:** SELECT button affects learning rate slider
**Code Analysis:**
```
app/ui/tabs/tab_training.c:
- Line 471: btn_select created
- Line 479: callback = on_select_all_clicked (CORRECT)
- Line 526: slider_lr created  
- Line 534: callback = on_learning_rate_changed (CORRECT)
```

**Hypothesis:** Event coordinates might be overlapping
**Need to verify:** Button and slider positions don't overlap

### Issue 5: START/PAUSE/SAVE overlap sphere
**Current State:**
```
Line 419: btn_x = RENDER_OFFSET_X + viz_width / 2
Line 420: btn_y = WINDOW_HEIGHT - 100 (= 800)
```
**Problem:** Buttons are in viz panel at y=800
**Solution:** Move to control panel bottom (y=820)

### Issue 6-8: LLM Tab Issues
**Need full analysis of LLM tab layout**

## Action Plan

### Phase 1: Diagnostic Build (DO THIS FIRST)
**Goal:** Add comprehensive logging to understand what's happening

**Changes:**
1. Add event tracing to dropdown handler
2. Add checkbox toggle tracing
3. Add button click tracing
4. Log all mouse coordinates and element bounds

**Files to modify:**
- `app/ui/crystalline/elements.c` - Add debug to dropdown/list handlers
- `app/ui/tabs/tab_training.c` - Add debug to all callbacks

### Phase 2: Fix Dropdown (CRITICAL)
**Root Cause:** Need to identify why dropdown stopped expanding

**Investigation Steps:**
1. Check if dropdown.expanded is being set
2. Check if options are being rendered
3. Check if click is hitting dropdown bounds
4. Verify event order: MOTION → DOWN → UP

**Potential Fixes:**
- Remove dropdown from motion handler if it's interfering
- Ensure DOWN event toggles expanded state
- Verify bounds calculation is correct

### Phase 3: Fix Checkbox Visibility
**Changes:**
1. Increase checkbox size to 24px minimum
2. Add filled circle with border
3. Add checkmark icon when checked
4. Ensure proper color contrast

**Code Changes:**
```c
// In elements.c line 1068:
list->checkbox_size = 24.0f;  // Increase from 18px

// In checkbox rendering (line 1191):
// Draw filled circle
// Draw thick border (2-3px)
// If checked, draw checkmark or X inside
```

### Phase 4: Fix Checkbox Toggle
**Investigation:**
1. Add logging to see if callback is called multiple times
2. Check if both BUTTONDOWN and BUTTONUP toggle
3. Verify event propagation stops after toggle

**Potential Fix:**
- Only toggle on BUTTONDOWN, not BUTTONUP
- Add flag to prevent double-toggle

### Phase 5: Fix Button Positions
**Changes:**
```c
// Line 419-420 in tab_training.c:
int btn_x = RENDER_OFFSET_X + viz_width + control_width / 2;
int btn_y = WINDOW_HEIGHT - 80;  // Bottom of control panel
```

### Phase 6: Fix LLM Tab
**Need to:**
1. Audit all elements in LLM tab
2. Remove legacy input box
3. Add labels to buttons
4. Fix off-screen elements
5. Ensure all use Crystalline UI

## Implementation Order

### Step 1: Create Diagnostic Build
- Add logging
- Build and test
- Analyze output
- Identify exact problems

### Step 2: Fix One Issue at a Time
- Fix dropdown
- Build and test
- Fix checkbox size
- Build and test
- Fix checkbox toggle
- Build and test
- Fix button positions
- Build and test

### Step 3: Fix LLM Tab
- Complete audit
- Remove legacy code
- Fix positioning
- Build and test

## Code References

### Training Tab Event Handlers
```
handle_training_tab_mouse_up()     - Line 795
handle_training_tab_mouse_down()   - Line 825  
handle_training_tab_mouse_motion() - Line 876
```

### Dropdown Event Handler
```
crystalline_dropdown_handle_mouse() - Line 1802 in elements.c
```

### Checkbox Rendering
```
crystalline_list_render() - Line 1178 in elements.c (checkbox drawing)
```

### Checkbox Click Detection
```
crystalline_list_handle_mouse() - Line 1247 in elements.c
```

## Next Steps

1. Create diagnostic build with extensive logging
2. Run and capture output
3. Analyze to find exact root causes
4. Fix issues one at a time
5. Test each fix before moving to next