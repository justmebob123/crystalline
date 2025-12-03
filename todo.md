# TODO: Global Crystalline UI Fixes & Training Tab Conversion

## Current Status
⚠️ **CRITICAL REALIZATION**: Research tab fixes were EXAMPLE ONLY
- Must apply ALL fixes GLOBALLY to entire Crystalline UI system
- Must fix ALL 5 converted tabs (Video, Benchmark, Downloaded Files, URL Manager, Research)
- Must establish proper standards BEFORE continuing to Training tab
- User will be "far more critical" of Training tab - must get it right

---

## Phase 1: GLOBAL Fixes to All Converted Tabs (CRITICAL)

### 1.1 Fix ALL Input Fields (CRITICAL)
- [ ] URL Manager: Change input from CIRCULAR to RECTANGULAR
- [ ] Downloaded Files: Check for any inputs
- [ ] Video Tab: Check for any inputs
- [ ] Benchmark Tab: Check for any inputs
- [ ] Research Tab: ✅ Already fixed
- [ ] **RULE**: ALL text inputs MUST be RECTANGULAR, never dodecagon

### 1.2 Fix ALL Button Sizes (CRITICAL)
- [ ] Video Tab: Review button sizes (currently has START button)
- [ ] Benchmark Tab: Review button sizes (RUN/CLEAR buttons)
- [ ] Downloaded Files: Review button sizes (OPEN/DELETE/RECRAWL)
- [ ] URL Manager: Review button sizes (5 action buttons)
- [ ] Research Tab: ✅ Already fixed (32px/28px)
- [ ] **STANDARD**: Primary 60-80px, Secondary 50px, Tertiary 40-45px, NEVER below 40px

### 1.3 Add Context Displays (HIGH)
- [ ] Video Tab: Add current video status/info
- [ ] Benchmark Tab: Add current benchmark info
- [ ] Downloaded Files: Add current directory/file count
- [ ] URL Manager: Add current URL count/status
- [ ] Research Tab: ✅ Already has directory display
- [ ] **RULE**: Always show user where they are and what they're doing

### 1.4 Fix ALL Visual Hierarchy (HIGH)
- [ ] Video Tab: Check panel nesting and colors
- [ ] Benchmark Tab: Check panel nesting and colors
- [ ] Downloaded Files: Check panel nesting and colors
- [ ] URL Manager: Check panel nesting and colors
- [ ] Research Tab: ✅ Already fixed (3px/2px borders, different colors)
- [ ] **STANDARD**: Outer 3px/lighter, Inner 2px/darker

### 1.5 Fix ALL Model Dropdowns (CRITICAL)
- [ ] Check which tabs have model selectors
- [ ] Update ALL to use button_down/button_up pattern
- [ ] Research Tab: ✅ Already fixed
- [ ] **RULE**: Separate BUTTONDOWN from BUTTONUP for all dropdowns

### 1.6 Review ALL Panel Labels (MEDIUM)
- [ ] Video Tab: Check panel labels are clear
- [ ] Benchmark Tab: Check panel labels are clear
- [ ] Downloaded Files: Check panel labels are clear
- [ ] URL Manager: Check panel labels are clear
- [ ] Research Tab: ✅ Changed to "CONTENT"
- [ ] **RULE**: Labels must be contextual and helpful, not generic

---

## Phase 2: Update Crystalline UI System Standards

### 2.1 Update Core Elements
- [ ] Update crystalline_input_create() documentation
- [ ] Add warnings about CIRCULAR style for inputs
- [ ] Update button creation guidelines
- [ ] Add size validation/warnings
- [ ] Update panel creation guidelines

### 2.2 Update CRYSTALLINE_UI_STANDARDS.md
- [ ] Add "NEVER use CIRCULAR for text inputs" rule
- [ ] Add "ALWAYS show context" rule
- [ ] Add "ALWAYS separate BUTTONDOWN/BUTTONUP" rule
- [ ] Add visual hierarchy requirements
- [ ] Add examples of good vs bad designs

### 2.3 Create Design Checklist
- [ ] Create pre-conversion checklist
- [ ] Create post-conversion verification checklist
- [ ] Add to standards document
- [ ] Use for Training tab and remaining tabs

---

## Phase 3: Training Tab Conversion (NEXT MAJOR TASK)

### 3.1 Pre-Conversion Analysis
- [ ] Read entire tab_training.c (~1,806 lines)
- [ ] Identify all UI elements (buttons, inputs, panels, lists)
- [ ] Map current layout and interactions
- [ ] Identify complexity areas
- [ ] Plan Crystalline UI design

### 3.2 Design Planning
- [ ] Apply ALL lessons learned from previous tabs
- [ ] Design with usability FIRST
- [ ] Plan visual hierarchy
- [ ] Plan context displays
- [ ] Plan button sizes and spacing
- [ ] Get user approval BEFORE coding

### 3.3 Implementation
- [ ] Convert to Crystalline UI
- [ ] Apply ALL global standards
- [ ] Test thoroughly
- [ ] Document conversion
- [ ] Get user approval

---

## Phase 4: Update SECONDARY_OBJECTIVES.md

### 4.1 Add Crystalline UI Conversion Objective
- [ ] Document Phase 3B progress (5/9 tabs complete)
- [ ] Document global fixes required
- [ ] Document standards established
- [ ] Document remaining work (4 tabs)
- [ ] Add to SECONDARY_OBJECTIVES.md

### 4.2 Update Status
- [ ] Mark completed objectives
- [ ] Add new sub-objectives for UI work
- [ ] Update priority levels
- [ ] Document lessons learned

---

## Success Criteria

### Global Fixes Complete:
- [ ] ALL text inputs are RECTANGULAR (no dodecagons)
- [ ] ALL buttons meet size standards (40px minimum)
- [ ] ALL tabs show context (where user is, what they're doing)
- [ ] ALL nested panels have clear visual hierarchy
- [ ] ALL dropdowns use proper event handling
- [ ] ALL panel labels are clear and contextual
- [ ] Build successful with zero errors
- [ ] User approves all fixes

### Training Tab Ready:
- [ ] All global fixes applied and tested
- [ ] Standards document updated
- [ ] Design checklist created
- [ ] Pre-conversion analysis complete
- [ ] Design plan approved by user

### SECONDARY_OBJECTIVES.md Updated:
- [ ] Phase 3B documented
- [ ] Progress tracked
- [ ] Standards documented
- [ ] Remaining work clear

---

## Notes

**Critical Understanding:**
- Research tab was EXAMPLE, not complete solution
- Must fix ALL tabs, not just one
- User will be "far more critical" of Training tab
- Must establish proper standards FIRST
- Must apply lessons learned GLOBALLY

**Core Principles:**
1. Usability First, Sacred Geometry Second
2. ALL text inputs MUST be rectangular
3. ALWAYS show context to user
4. ALWAYS meet minimum size standards
5. ALWAYS create clear visual hierarchy
6. ALWAYS separate BUTTONDOWN from BUTTONUP

**Workflow:**
1. Fix all existing tabs FIRST
2. Update standards SECOND
3. Plan Training tab THIRD
4. Get user approval FOURTH
5. Implement Training tab FIFTH