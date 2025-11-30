# TODO - Crystalline CLLM Project

## RULES (PASTE TO TOP WITH EVERY RESPONSE)
- Rule 0: Always paste rules to the top of todo.md with every response
- Rule 1: Always reread MASTER_PLAN.md before any action
- Rule 2: Reference AUDIT.md for architectural state
- Rule 3: Reference SECONDARY_OBJECTIVES.md for detailed tasks
- Rule 4: Do not create new .md files
- Rule 5: Always commit all changes using correct authentication
- Rule 6: MASTER_PLAN.md is read-only - do not edit without explicit approval

## CRITICAL ISSUES - USER FEEDBACK

### Issue 1: Start Crawler Requires URL Despite Database Having URLs ✅ FIXED
**Problem:** User clicks "Start Crawler" but it won't start because URL field is empty, even though database has URLs
**Solution Implemented:**
- [x] Modified crawler start logic to check total URLs instead of pending
- [x] Auto-reset crawled URLs to pending when starting with no pending URLs
- [x] Removed requirement for URL input field to be filled
- [x] Crawler now works with just database URLs - no manual URL entry needed!

**Changes Made:**
- Modified app/ui/tabs/tab_crawler.c - Changed check from pending == 0 to total == 0
- Added auto-reset logic: if pending == 0 &amp;&amp; crawled > 0, automatically reset all to pending
- Removed unnecessary URL entry retrieval code

### Issue 2: Cannot Recrawl URLs Marked as "Already Crawled" ✅ FIXED
**Problem:** All URLs in database show status "crawled", but user wants to recrawl them
**Solution Implemented:**
- [x] Added url_db_reset_all_to_pending() function to database layer
- [x] Added crawler_url_manager_reset_all() wrapper function
- [x] Added "Reset All URLs" button to Crawler tab UI
- [x] Implemented click handler with stats display
- [x] Auto-reset also happens when starting crawler with no pending URLs

**Changes Made:**
- src/crawler/url_database.h - Added function declaration
- src/crawler/url_database.c - Implemented reset function (SQL UPDATE)
- src/crawler/crawler_url_manager.h - Added wrapper function
- src/crawler/crawler_url_manager.c - Implemented wrapper
- app/ui/tabs/tab_crawler.c - Added button, click handler, and auto-reset logic

## Build Status
✅ All libraries compile successfully
✅ Application builds successfully
✅ Zero errors, 1 minor warning (unused function)

## Summary of Fixes
Both issues are now resolved:
1. **Start Crawler** - Works without URL input, automatically resets crawled URLs if needed
2. **Reset Button** - Manual "Reset All URLs" button available for explicit resets
3. **User Experience** - Just click "Start Crawler" and it works!

## Next Steps
- User should test the fixes with real database URLs
- Verify auto-reset works when starting with crawled URLs
- Verify manual reset button works correctly