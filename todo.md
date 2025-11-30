# TODO - Crystalline CLLM Project

## RULES (PASTE TO TOP WITH EVERY RESPONSE)
- **Rule 0:** Always paste rules to the top of `todo.md` with every response
- **Rule 1:** Always reread `MASTER_PLAN.md` before any action
- **Rule 2:** Reference `AUDIT.md` for architectural state
- **Rule 3:** Reference `SECONDARY_OBJECTIVES.md` for detailed tasks
- **Rule 4:** Do not create new `.md` files
- **Rule 5:** Always commit all changes using correct authentication
- **Rule 6:** `MASTER_PLAN.md` is read-only - do not edit without explicit approval

## Current Status: Phase 3 - Layout and UI Fixes (STARTING)

### MASTER_PLAN Context ✅
- Read MASTER_PLAN.md - OBJECTIVE 15: Comprehensive UI and CLI Analysis
- Phase 1 (Analysis) ✅ COMPLETE
- Phase 2 (Critical Backend Connections) ✅ COMPLETE
- Phase 3 (Layout and UI Fixes) - STARTING NOW

### Phase 2: Critical Backend Connections ✅ COMPLETE
- [x] Fix 1: URL Manager Tab - Backend Integration (2 hours)
- [x] Fix 2: Downloaded Files Tab - Backend Integration (3 hours)
- [x] Fix 3: Model Persistence - Critical Bug Fix (3 hours)

### Phase 3: Layout and UI Fixes (CURRENT - STARTING FIX 4)

#### Fix 4: Crawler Tab Layout Issues ✅ COMPLETE
**File:** `app/ui/tabs/tab_crawler.c` (703 lines)

**Issues Fixed:**
1. ✅ Title position adjusted (y + 30 instead of y + 10) to avoid submenu overlap
2. ✅ Add/Clear buttons now use column width: `(content_width - 10) / 2`
3. ✅ Start/Stop button now uses column width: `content_width`
4. ✅ Save/Load Config buttons now use column width: `content_width`
5. ✅ Added `content_width` calculation in both column 2 and column 3

**Changes Made:**
- Added `int content_width = col->width - (col->padding * 2);` in draw_column2_link_management
- Added `int content_width = col->width - (col->padding * 2);` in draw_column3_status
- Replaced hardcoded button widths (80px, 180px) with responsive column-based sizing
- Adjusted title Y position from `layout->content_area.y + 10` to `layout->content_area.y + 30`
- All buttons now scale properly with column width

**Remaining Issues (Lower Priority):**
- URL pattern checkboxes are visual-only (no click handlers) - DEFERRED
- Save/Load Config buttons not implemented - DEFERRED
- Input labels alignment - WORKING AS DESIGNED (InputManager handles this)

**Build Status:** ✅ Compiles successfully with zero errors

#### Fix 5: Video Tab Unicode and Layout ✅ COMPLETE
**File:** `app/ui/tabs/tab_video.c` (234 lines)

**Issues Fixed:**
1. ✅ Replaced Unicode checkmark (✓) with ASCII [OK]
2. ✅ Replaced Unicode X mark (✗) with ASCII [X]
3. ✅ Replaced Unicode bullets (•) with ASCII dashes (-)
4. ✅ Fixed path truncation bug - proper character-by-character assignment
5. ✅ Adjusted title position (y + 20 instead of y) to avoid submenu overlap

**Changes Made:**
- Line 196: `"✓ FFmpeg: Installed"` → `"[OK] FFmpeg: Installed"`
- Line 198: `"✗ FFmpeg: Not Found"` → `"[X] FFmpeg: Not Found"`
- Lines 206, 208, 210: `"• ..."` → `"- ..."`
- Lines 104-108: Fixed path truncation to use character assignment instead of strcpy
- Line 58: Adjusted title Y position from `y` to `y + 20`

**Build Status:** ✅ Compiles successfully with zero errors

#### Fix 6: Models Tab Implementation (AFTER FIX 5)
**File:** `app/ui/tabs/tab_models.c`

**Issues Identified:**
1. Create dialog has all TODOs (no text rendering)
2. Model details panel has all TODOs (no text rendering)
3. Click handlers not implemented
4. No actual create/load/delete functionality

**Tasks:**
- [ ] Implement create dialog rendering
- [ ] Implement model details rendering
- [ ] Add click handlers
- [ ] Add create/load/delete functionality

## Next Action
Starting Fix 4: Crawler Tab Layout Issues