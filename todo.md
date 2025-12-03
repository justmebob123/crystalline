# TODO: Fix Crystalline UI Layout Issues

## Status: NEW CRITICAL ISSUES - Layout and Positioning Problems

## User Report (Research Tab - affects all converted tabs)
1. ✅ Buttons detect clicks (fixed)
2. ✅ Lists have fonts (fixed)
3. ❌ UI draws OVER the left sidebar menu
4. ❌ Still don't see actual file viewer or list
5. ❌ Debug says "Found 2 documents" but nothing visible
6. ❌ Layout doesn't make sense

## Completed Tasks
- [x] Read MASTER_PLAN.md
- [x] Fixed button interactions
- [x] Fixed list fonts

## Previous Fixes
- [x] Fix #1: Button interactions (NULL font)
- [x] Fix #2: List rendering (NULL font)
- [x] Fix #3: Panel style (CIRCULAR to RECTANGULAR)

## Current Issue: Panels Off-Screen
User reports:
- Three panels go off TOP of screen
- One panel covers menu and goes off LEFT side
- Panels not centered properly
- Random shapes visible (box outline, multi-sided shape)

## Investigation Complete
- [x] Check SUBMENU_HEIGHT value - 40px
- [x] Verify panel Y coordinates - FOUND ISSUE!
- [x] Examine panel positioning calculations

## Root Cause #4: Center vs Top-Left Positioning
crystalline_rect_create() treats x,y as CENTER, not top-left!

Example bug:
- Panel created at (210, 50) with size (1043, 840)
- Rectangle centered at (210, 50)
- Top edge: 50 - 420 = -370 (OFF SCREEN!)
- Left edge: 210 - 521 = -311 (OFF SCREEN!)

## Fix Applied
Calculate center point correctly for all panels:
- Research Tab: 3 panels fixed
- URL Manager Tab: 3 panels fixed
- Downloaded Files Tab: 2 panels fixed

Formula: center = position + (size / 2)

- [ ] Build and test
- [ ] Verify panels now render on-screen