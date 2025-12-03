# Critical Bug Fix: Crystalline UI Button Interactions

## Problem
Buttons in converted tabs don't respond to hover or click events.

## Root Cause
Crystalline buttons require THREE event types to function:
1. `SDL_MOUSEMOTION` - for hover states (visual feedback)
2. `SDL_MOUSEBUTTONDOWN` - for press states (visual feedback)
3. `SDL_MOUSEBUTTONUP` - for click callbacks (actual action)

Current implementation only sends `SDL_MOUSEBUTTONDOWN` to tabs, so:
- Buttons never show hover state
- Buttons never trigger click callbacks
- Users see no response to interaction

## Solution
Add three handlers to each converted tab:
1. `handle_*_tab_click()` - handles MOUSEBUTTONDOWN (already exists)
2. `handle_*_tab_mouse_motion()` - handles MOUSEMOTION (NEW)
3. `handle_*_tab_mouse_up()` - handles MOUSEBUTTONUP (NEW)

Update `main.c` to dispatch all three event types to converted tabs.

## Implementation Status

### Completed Tabs
- [x] Video Tab - handlers added
- [ ] Benchmark Tab - handlers added (not wired to main.c yet)
- [ ] Research Tab - needs handlers
- [ ] URL Manager Tab - needs handlers
- [ ] Downloaded Files Tab - needs handlers

### Main.c Updates
- [x] Added MOUSEMOTION dispatch for converted tabs
- [x] Added MOUSEBUTTONUP dispatch for converted tabs
- [x] Added MOUSEBUTTONDOWN dispatch for URL Manager and Downloaded Files

## Files Modified
- `app/main.c` - event dispatching
- `app/ui/tabs/tab_video.c` - added motion and up handlers
- `app/ui/tabs/tab_video.h` - added function declarations
- `app/ui/tabs/tab_benchmark.c` - added motion and up handlers

## Next Steps
1. Add handlers to remaining tabs (research, url_manager, downloaded_files)
2. Test all button interactions
3. Verify hover states work
4. Verify click callbacks trigger
5. Document the pattern for future tabs