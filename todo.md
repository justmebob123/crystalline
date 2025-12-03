# TODO: Fix Critical Research Tab UX Issues

## Current Status
User reported 8 critical usability issues with the Research Tab. All issues documented in `RESEARCH_TAB_UX_ISSUES.md`.

---

## Phase 1: Critical Fixes (IMMEDIATE)

### 1. Fix Model Dropdown Mouse Handling
- [x] Analyze current event handling in `app/ui/model_selector.c`
- [x] Separate BUTTONDOWN (hover) from BUTTONUP (click) logic
- [x] Track click origin to prevent immediate closure
- [ ] Test dropdown stays open until item selected
- [ ] Verify dropdown closes only on outside BUTTONUP

### 2. Change Search Input to Rectangular
- [x] Change `input_search` from CIRCULAR to RECTANGULAR style
- [x] Recalculate position for center-based coordinates
- [ ] Test input looks like standard text box
- [ ] Verify placeholder text displays correctly
- [ ] Test typing and text display

### 3. Fix File List Positioning
- [x] Increase padding between panel border and list (40px -> 50px)
- [ ] Verify first item doesn't overlap panel border
- [ ] Test with different numbers of files
- [ ] Check scroll behavior

### 4. Display Current Directory Path
- [x] Add directory path display above file list
- [x] Show format: "Directory: docs/research"
- [x] Update on directory navigation
- [x] Make it prominent and clear

---

## Phase 2: High Priority Fixes

### 5. Reduce Button Sizes and Fix Overlapping
- [x] Reduce button radius from 40-45px to 32px (control) and 28px (sort)
- [x] Recalculate spacing to prevent overlap (increased vertical spacing)
- [x] Changed "UP" to "UP DIR" for clarity
- [ ] Verify button labels fit within bounds
- [ ] Test all 7 buttons (SCAN, REFRESH, UP DIR, NAME, SIZE, DATE, TYPE)
- [ ] Consider vertical stacking if needed

### 6. Create Visual Hierarchy for Nested Panels
- [x] Outer panels (viewer, controls): Thicker border (3px), lighter color (100,120,140)
- [x] Inner panel (files): Medium border (2px), medium color (80,100,120)
- [ ] Test visual distinction is clear
- [ ] Verify nested panels are clearly distinguishable

### 7. Improve Panel Labels
- [x] Changed "FILE VIEWER" to "CONTENT"
- [ ] Consider showing actual filename when file selected
- [ ] Show "No file selected" when nothing selected
- [ ] Test label positioning and clarity

### 8. Remove Confusing Decorative Geometry
- [x] No decorative geometry found in Research tab
- [x] Input changed from dodecagon to rectangle
- [ ] Verify no other confusing geometry elements
- [ ] Test interface is cleaner and clearer

---

## Phase 3: Testing and Verification

### Comprehensive Testing
- [ ] Test all button interactions
- [ ] Test dropdown behavior
- [ ] Test file selection and viewing
- [ ] Test directory navigation
- [ ] Test search functionality
- [ ] Test with empty directories
- [ ] Test with many files
- [ ] Verify no overlapping elements
- [ ] Verify clear visual hierarchy
- [ ] Get user feedback

---

## Phase 4: Documentation

### Update Documentation
- [ ] Document all changes made
- [ ] Update usability standards if needed
- [ ] Create before/after comparison
- [ ] Document lessons learned
- [ ] Update master plan if principles changed

---

## Success Criteria

✅ **All issues resolved:**
1. Input field looks like standard text box (no dodecagon)
2. No confusing decorative geometry
3. UP button shows current directory clearly
4. Files don't "disappear" - navigation is clear
5. Panel labels are helpful and contextual
6. File list doesn't overlap panel border
7. Buttons don't overlap each other or text
8. Nested panels have clear visual distinction
9. Dropdown stays open until item selected
10. All interactions feel natural and expected

✅ **User confirms:**
- Interface is clear and understandable
- All elements are obviously clickable
- Navigation makes sense
- No confusion about purpose of elements

---

## Notes

**Core Principle:**
> "Usability First, Sacred Geometry Second"
> Sacred geometry should ENHANCE usability, not HARM it.
> When in doubt, choose clarity over aesthetics.

**Master Plan Alignment:**
While the Master Plan emphasizes crystalline mathematics and sacred geometry, it should NOT come at the expense of usability. The UI must be functional, clear, and easy to use FIRST, then we can add aesthetic enhancements that don't interfere.