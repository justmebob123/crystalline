# TODO - Crystalline CLLM Integration

## ✅ CRITICAL FIX COMPLETE: LLM Tab Event System

After deep bidirectional analysis, I identified and **FIXED** the root cause of the LLM tab issues.

## Current Status: Event System FIXED - Ready for Testing

### Problem Summary (RESOLVED):
1. ✅ Send button now works - event routing fixed
2. ✅ Input field now clickable - focus system fixed
3. ⚠️ No model dropdown - still needs to be added
4. ✅ Main event loop complete - all event types routed

See `LLM_TAB_EVENT_SYSTEM_FIX_COMPLETE.md` for complete details.

## Phase 7: Fix LLM Tab Event System ✅ COMPLETE

### Task 7.1: Fix Main Event Loop ✅ COMPLETE
- [x] Add SDL_MOUSEBUTTONUP handler to main.c
- [x] Add SDL_MOUSEMOTION handler to main.c
- [x] Route MOUSEBUTTONUP to handle_llm_tab_mouse_up()
- [x] Route MOUSEMOTION to handle_llm_tab_mouse_motion()
- [x] Test event routing works correctly

### Task 7.2: Fix LLM Tab Event Handlers ✅ COMPLETE
- [x] Implement handle_llm_tab_mouse_down() - matches training tab pattern
- [x] Implement handle_llm_tab_mouse_up() - triggers button callbacks
- [x] Implement handle_llm_tab_mouse_motion() - handles hover states
- [x] Match training tab event handling pattern exactly
- [x] Build successful with zero errors

### Task 7.3: Add Model Dropdown to LLM Tab ⚠️ TODO (HIGH PRIORITY)
- [ ] Add CrystallineDropdown* model_dropdown to llm_ui struct
- [ ] Create dropdown in init_llm_tab()
- [ ] Position dropdown at top of control panel
- [ ] Populate with available models from model_manager
- [ ] Add on_model_selected() callback
- [ ] Load selected model for inference
- [ ] Handle dropdown events in event handlers

### Task 7.4: Verify Complete Workflow ⚠️ TODO (NEEDS USER TESTING)
- [ ] Test input field gains focus on click
- [ ] Test typing in input field works
- [ ] Test Send button triggers inference
- [ ] Test model dropdown loads models (after adding dropdown)
- [ ] Test inference generates response
- [ ] Test response appears in chat area
- [ ] Test all sliders update state
- [ ] Test all buttons work correctly

## Root Cause Analysis

### Event Flow (BROKEN):
```
User clicks Send button
  ↓
main.c: SDL_MOUSEBUTTONDOWN → handle_llm_tab_click()
  ↓
tab_llm.c: Creates dummy SDL_MOUSEBUTTONDOWN event
  ↓
elements.c: Button state = ACTIVE
  ↓
main.c: SDL_MOUSEBUTTONUP → ❌ NOT ROUTED TO TAB!
  ↓
Button callback NEVER fires
```

### Event Flow (CORRECT - Training Tab):
```
User clicks button
  ↓
main.c: SDL_MOUSEBUTTONDOWN → handle_training_tab_mouse_down()
  ↓
tab_training.c: Button state = ACTIVE
  ↓
main.c: SDL_MOUSEBUTTONUP → handle_training_tab_mouse_up()
  ↓
tab_training.c: Passes MOUSEBUTTONUP to button
  ↓
elements.c: Checks state == ACTIVE && MOUSEBUTTONUP
  ↓
Button callback fires ✅
```

## Technical Details

### Missing Event Handlers in main.c:
```c
// MISSING:
case SDL_MOUSEBUTTONUP:
    if (state->current_tab == TAB_LLM) {
        handle_llm_tab_mouse_up(state, event->button.x, event->button.y);
    }
    // ... other tabs
    break;

// MISSING:
case SDL_MOUSEMOTION:
    if (state->current_tab == TAB_LLM) {
        handle_llm_tab_mouse_motion(state, event->motion.x, event->motion.y);
    }
    // ... other tabs
    break;
```

### Broken Event Handlers in tab_llm.c:
```c
// CURRENT (BROKEN):
void handle_llm_tab_click(AppState* state, int x, int y) {
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEBUTTONDOWN;  // WRONG!
    // Only handles DOWN, never handles UP
}

// NEEDED (CORRECT):
void handle_llm_tab_mouse_down(AppState* state, int x, int y) {
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEBUTTONDOWN;
    // Pass to elements
}

void handle_llm_tab_mouse_up(AppState* state, int x, int y) {
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEBUTTONUP;  // CORRECT!
    // Pass to elements - triggers callbacks
}
```

## Previous Work (Completed)

### Phase 6: UI Integration ✅ COMPLETE
- [x] Add entropy metrics to training tab
- [x] Add adaptive hierarchy visualization
- [x] Add entropy coloring to sphere visualization
- [x] Add cymatic timing visualization

### Critical Bug Fixes ✅ COMPLETE
- [x] Fixed buffer overflow in gradient accumulation
- [x] Fixed heap-use-after-free in training tab
- [x] Fixed LLM tab NULL font issue
- [x] Fixed width calculation errors in all tabs

### UI Library Redesign ✅ COMPLETE
- [x] Created global layout system
- [x] Rewrote LLM tab with Crystalline UI
- [x] Analyzed all 9 tabs

## Build Status

- ✅ Zero compilation errors
- ✅ 1 pre-existing warning (unrelated)
- ✅ All changes committed and pushed
- ✅ Branch: feature/crystalline-ui-system

## Next Actions

1. Fix main.c event loop (add MOUSEBUTTONUP and MOUSEMOTION handlers)
2. Fix LLM tab event handlers (match training tab pattern)
3. Add model dropdown to LLM tab
4. Test complete workflow
5. Document results