# TODO: Fix Crystalline UI Display Issues - Lists and Panels Not Rendering

## Status: CRITICAL ISSUE - Affects ALL 5 Converted Tabs

## User Report
- Buttons now detect clicks ✅
- Research Tab (and all converted tabs) have rendering issues:
  * File lists NOT VISIBLE
  * Panels/boxes NOT VISIBLE  
  * Text input has no label
  * Only see: buttons, unlabeled input, model dropdown, random "file viewer" and "files" text

## Completed Tasks
- [x] Read MASTER_PLAN.md
- [x] Read AUDIT.md
- [x] Confirmed issue affects all 5 converted tabs globally

## Investigation Tasks
- [x] Check if crystalline_list_render() is being called - YES
- [x] Check if crystalline_panel_render() is being called - YES
- [x] Verify lists and panels are being created properly - FOUND ISSUE!
- [x] Check if lists/panels have visible flag set - YES
- [x] Examine draw functions for all 5 tabs
- [x] Identify root cause of rendering failure - FOUND!

## Root Cause
Lists created with NULL font in 3 tabs:
- Research Tab: list_files created with NULL font
- URL Manager Tab: list_urls created with NULL font
- Downloaded Files Tab: list_files created with NULL font

crystalline_list_render() returns immediately if font is NULL!

## Fix Applied
- [x] Changed NULL to get_global_font() in all 3 tabs
- [ ] Build and test
- [ ] Verify lists now render correctly