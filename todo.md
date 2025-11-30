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

## CRITICAL BUG FIXES (USER REPORTED) ✅ FIXED

### Issue 1: LLM Send Button Not Working ✅ FIXED
**Problem:** User can type in text box but Send button doesn't work
**Root Cause:** Input box width was 1060px, overlapping with send button at x=1260

**Fix Applied:**
- [x] Fixed input box width from 1060 to 1040 in app/input_registration.c
- [x] Fixed input box height from 60 to 80 to match rendering
- [x] Input box now: (210, 820, 1040, 80) - ends at x=1250
- [x] Send button at: (1260, 820, 90, 80) - starts at x=1260
- [x] 10px gap between input and button - no overlap
- [x] Application rebuilt successfully

**Result:** Send button clicks will no longer be captured by InputManager

### Issue 2: URL Manager Database Path Error ✅ FIXED
**Problem:** `Failed to open URL database: data/crawler/urls.db`
**Root Cause:** Directory `data/crawler/` didn't exist

**Fix Applied:**
- [x] Fixed database path in all three tabs (tab_url_manager, tab_downloaded_files, tab_crawler)
- [x] Added directory creation in crawler_url_manager_create()
- [x] Added sys/stat.h and sys/types.h includes
- [x] Directory now created with mkdir() if it doesn't exist
- [x] Application rebuilt successfully

**Result:** URL manager will create directory and database automatically

## OBJECTIVE 15 Phase 5: CLI Tool Integration (RESUME AFTER FIXES)

### Phase 5.3: Cleanup and Organization (IN PROGRESS)
- [x] Remove legacy/redundant tools
- [x] Move test tools to tests/ directory
- [x] Update tests/Makefile
- [ ] Fix remaining test tool build issues
- [ ] Check main Makefile for legacy tool references