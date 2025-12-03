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

## Investigation Complete
- [x] Check sidebar width constant (SIDEBAR_WIDTH) - Correct (200)
- [x] Verify content_x calculation respects sidebar - Correct
- [x] Check if panels are positioned correctly - FOUND ISSUE!
- [x] Examine coordinate system and positioning logic

## Root Cause #3: Wrong Panel Style
Panels used CRYSTALLINE_STYLE_CIRCULAR with width/height values:
- Circular style expects: center point + radius
- Tabs were passing: center point + width + height
- Result: Massive circles (radius ~1000px) overlapping sidebar

Example:
- viewer_width = 1063, passed as radius
- Circle centered at x=531 with radius=1043
- Circle extends from x=-512 to x=1574 (overlaps sidebar at x=0-200!)

## Fix Applied
Changed all panels from CIRCULAR to RECTANGULAR style:
- Research Tab: 3 panels fixed
- URL Manager Tab: 3 panels fixed  
- Downloaded Files Tab: 2 panels fixed

- [ ] Build and test
- [ ] Verify panels now render correctly within bounds