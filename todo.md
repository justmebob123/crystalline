# TODO - Crystalline CLLM Project

## RULES (PASTE TO TOP WITH EVERY RESPONSE)
- **Rule 0:** Always paste rules to the top of `todo.md` with every response
- **Rule 1:** Always reread `MASTER_PLAN.md` before any action
- **Rule 2:** Reference `AUDIT.md` for architectural state
- **Rule 3:** Reference `SECONDARY_OBJECTIVES.md` for detailed tasks
- **Rule 4:** Do not create new `.md` files
- **Rule 5:** Always commit all changes using correct authentication:
  ```bash
  git add .
  git commit -m "descriptive message"
  git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
  ```
- **Rule 6:** `MASTER_PLAN.md` is read-only - do not edit without explicit approval

## CRITICAL BUG FIXES (USER REPORTED) ✅ COMPLETE

### Issue 1: LLM Send Button Not Working ✅ FIXED
**Problem:** User can type in text box but Send button doesn't work
**Root Cause:** Input box width was 1060px instead of 1040px, causing overlap with send button
**Fix Applied:**
- [x] Fixed input box width in app/input_registration.c from 1060 to 1040
- [x] Updated height from 60 to 80 to match actual rendering
- [x] Send button now at x=1260, input box ends at x=1250 (no overlap)
- [x] Application rebuilt successfully

### Issue 2: URL Manager Database Path Error ✅ FIXED
**Problem:** `Failed to open URL database: data/crawler/crawler.db/urls.db`
**Root Cause:** All three tabs passing wrong path to crawler_url_manager_create()
**Fix Applied:**
- [x] Fixed tab_url_manager.c: "data/crawler/crawler.db" → "data/crawler"
- [x] Fixed tab_downloaded_files.c: "data/crawler/crawler.db" → "data/crawler"
- [x] Fixed tab_crawler.c: "data/crawler/crawler.db" → "data/crawler"
- [x] Database will now be created at correct path: data/crawler/urls.db
- [x] Application rebuilt successfully

## OBJECTIVE 15 Phase 5: CLI Tool Integration (RESUME AFTER FIXES)

### Phase 5.3: Cleanup and Organization (IN PROGRESS)
- [x] Remove legacy/redundant tools
- [x] Move test tools to tests/ directory
- [x] Update tests/Makefile
- [ ] Fix remaining test tool build issues
- [ ] Check main Makefile for legacy tool references