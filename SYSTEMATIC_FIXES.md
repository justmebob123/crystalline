# Systematic Fixes for Reported Issues

## Issues to Fix

### 1. File checkbox toggle - sets 1 then immediately sets 0
**Status:** Need to investigate - code looks correct
**Action:** Add debug output to trace the issue

### 2. Dropdown not working
**Status:** Need to verify event handling and rendering
**Action:** Add more debug output, verify options are visible

### 3. SELECT button wired to learning rate
**Status:** Code shows correct wiring - need to verify at runtime
**Action:** Add debug output to confirm which callback is called

### 4. START, PAUSE, SAVE buttons overlap sphere
**Current:** Buttons at y=800 in viz panel
**Fix:** Move to control panel below file list (y=750)

### 5. Checkbox visual representation too small
**Current:** 18px circles
**Fix:** Increase to 24px, add better visual styling

### 6. LLM tab has legacy input box
**Status:** Need to remove old input, keep only Crystalline UI version
**Action:** Find and remove legacy code

### 7. LLM tab has unlabeled buttons
**Status:** Need to add labels or remove buttons
**Action:** Identify buttons and add labels

### 8. LLM tab has box off-screen bottom-left
**Status:** Need to fix positioning
**Action:** Check all element positions in LLM tab

## Fix Order

1. Add comprehensive debug output
2. Fix button positioning (Training Tab)
3. Increase checkbox size
4. Fix LLM tab layout completely
5. Test and verify each fix