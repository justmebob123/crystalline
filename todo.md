# TODO: Fix Crystalline UI - Lists Not Displaying Items

## Status: PROGRESS - Panels Fixed, Lists Still Empty

## User Report (Research Tab)
Progress:
- ✅ Two panels now properly positioned (don't overlap menu)
- ✅ One panel has a box inside (for files)
- ❌ No files actually displayed in the list
- ❌ Buttons don't seem to populate the list
- ❌ Debug says "Found 2 documents" but nothing visible
- ❌ Strange outline of a box visible
- ❌ Outline of multi-sided circular shape visible

## Completed Fixes
- [x] Fix #1: Button interactions (event handling)
- [x] Fix #2: List fonts (NULL → get_global_font)
- [x] Fix #3: Panel style (CIRCULAR → RECTANGULAR)
- [x] Fix #4: Panel positioning (center point calculation)

## Investigation Complete
- [x] Check if list items are being added - YES (in draw function)
- [x] Verify list bounds and positioning - FOUND ISSUE!
- [x] Lists also use center positioning, not top-left

## Root Cause #5: List Positioning
Lists also use crystalline_rect_create() which treats x,y as CENTER!
- Same issue as panels
- Lists positioned as if x,y were top-left
- Result: Lists off-screen or in wrong location

## Fix Applied
Calculate center point for all lists:
- Research Tab: list_files repositioned
- URL Manager Tab: list_urls repositioned
- Downloaded Files Tab: list_files repositioned

Formula: center = position + (display_height / 2)
Note: display_height = item_height * visible_items = 30 * 10 = 300px

- [ ] Build and test
- [ ] Verify lists now display items