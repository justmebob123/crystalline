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

## 🎯 DECISION POINT: What to Implement Next?

### Option A: LLM Tab Comprehensive Enhancement (RECOMMENDED) ⭐
**Value**: High - Adds major functionality
**Time**: 6 hours total
**Impact**: Complete model management, threading, advanced parameters

### Option B: Complete Collapsible Panels
**Value**: Medium - UI polish
**Time**: 30-45 minutes
**Impact**: Cleaner training tab interface

### Option C: Training Tab Advanced Features
**Value**: Medium - Enhanced visualization
**Time**: 3-4 hours
**Impact**: 3D view, zoom, recursive spheres

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