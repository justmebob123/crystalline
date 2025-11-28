# TODO: Continue UI Enhancements

## Completed ✅
- [x] Fix progress bar off-by-one error
- [x] Fix sphere visualization clearing prematurely
- [x] Build and test fixes

## In Progress: Collapsible Panels Infrastructure ⚙️

**Status**: Infrastructure complete, full implementation pending

**Completed**:
- [x] Added `draw_collapsible_header()` helper function
- [x] Added panel state tracking to AppState
- [x] Initialized panel states in main.c

**Remaining** (requires careful layout refactoring):
- [ ] Replace panel titles with collapsible headers
- [ ] Track header bounds for click detection
- [ ] Add conditional rendering based on expanded state
- [ ] Adjust Y positions dynamically
- [ ] Add click handling in handle_training_tab_click()
- [ ] Test with various states

**Note**: Full implementation requires extensive refactoring of training tab layout code. Infrastructure is ready for future completion.

---

## 🎯 IMPLEMENTATION PLAN: All Three Enhancements

**User Decision**: Implement ALL enhancements
**Total Time**: ~10 hours
**Order**: LLM Tab → Collapsible Panels → Advanced Training Features

---

## Phase 1: LLM Tab Comprehensive Enhancement (6 hours) 🔄 IN PROGRESS

### 1.1 Model Management (2 hours) 🎯 CURRENT
- [ ] Create ModelBrowser structure
- [ ] Implement directory scanning
- [ ] Add model file list UI
- [ ] Add model selection logic
- [ ] Implement model export dialog
- [ ] Add detailed model statistics
- [ ] Add overwrite confirmation

### 1.2 Thread Management (1.5 hours)
- [ ] Create ConversationThread structure
- [ ] Implement thread list UI
- [ ] Add thread switching
- [ ] Add thread persistence
- [ ] Add thread export

### 1.3 Enhanced Parameters (1 hour)
- [ ] Extend token range slider (1-4096)
- [ ] Add Top-K slider
- [ ] Add Top-P slider
- [ ] Add penalty sliders
- [ ] Add stop sequences input
- [ ] Add random seed input

### 1.4 Loading Indicator (30 min)
- [ ] Add "Thinking" animation
- [ ] Add progress display
- [ ] Add cancel button

### 1.5 UI Polish (1 hour)
- [ ] Add collapsible sections
- [ ] Add keyboard shortcuts
- [ ] Add context menus
- [ ] Add drag and drop

---

## Phase 2: Complete Collapsible Panels (45 min)
- [ ] Integrate headers into training tab
- [ ] Add click detection
- [ ] Implement conditional rendering
- [ ] Test all states

---

## Phase 3: Training Tab Advanced Features (3-4 hours)
- [ ] Recursive sub-spheres visualization
- [ ] Zoom and pan controls
- [ ] Mouse-over tooltips
- [ ] 3D visualization mode
- [ ] 2D/3D toggle

---

## Future Enhancements

### Option A: LLM Tab Comprehensive Enhancement (6 hours)
Based on LLM_TAB_ENHANCEMENT_PLAN.md:

#### Phase 1: Model Management (2 hours)
- [ ] Model directory browser
- [ ] Model selection UI
- [ ] Model export functionality
- [ ] Detailed model statistics
- [ ] Overwrite confirmation dialogs

#### Phase 2: Thread Management (1.5 hours)
- [ ] Multiple conversation threads
- [ ] Thread list UI
- [ ] Thread switching
- [ ] Thread persistence
- [ ] Thread export

#### Phase 3: Enhanced Parameters (1 hour)
- [ ] Extended token range (1-4096)
- [ ] Top-K sampling
- [ ] Top-P (nucleus) sampling
- [ ] Repetition penalty
- [ ] Frequency penalty
- [ ] Presence penalty
- [ ] Stop sequences
- [ ] Random seed

#### Phase 4: Loading Indicator (30 min)
- [ ] "Thinking" animation
- [ ] Token generation progress
- [ ] Cancel button

#### Phase 5: UI Polish (1 hour)
- [ ] Collapsible sections
- [ ] Keyboard shortcuts
- [ ] Context menus
- [ ] Drag and drop

### Option B: Training Tab Advanced Features (3-4 hours)
- [ ] Recursive sub-spheres visualization
- [ ] Zoom and mouse-over tooltips
- [ ] 3D visualization mode
- [ ] 2D/3D toggle
- [ ] Pan and rotate controls

---

## Current Focus

**IMMEDIATE**: Implement collapsible panels in Training tab (30 min)
**NEXT**: User decision on LLM Tab vs Advanced Training features