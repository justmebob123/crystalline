# TODO - Crystalline CLLM Project

## RULES (PASTE TO TOP WITH EVERY RESPONSE)
- **Rule 0:** Always paste rules to the top of `todo.md` with every response
- **Rule 1:** Always reread `MASTER_PLAN.md` before any action
- **Rule 2:** Reference `AUDIT.md` for architectural state
- **Rule 3:** Reference `SECONDARY_OBJECTIVES.md` for detailed tasks
- **Rule 4:** Do not create new `.md` files
- **Rule 5:** Always commit all changes using correct authentication
- **Rule 6:** `MASTER_PLAN.md` is read-only - do not edit without explicit approval

## Current Status: OBJECTIVE 15 Phase 4 Feature 2B COMPLETE ✅ - Moving to Feature 3

### MASTER_PLAN Context ✅
- Read MASTER_PLAN.md
- OBJECTIVE 15: Comprehensive UI and CLI Analysis
  * Phase 1: Analysis ✅
  * Phase 2: Critical Backend Connections ✅
  * Phase 3: Layout and UI Fixes ✅
  * Phase 4: Advanced Crawler Features
    - Feature 1: Content Filtering ✅ COMPLETE
    - Feature 2: Site-Specific Crawlers ✅ COMPLETE (7 handlers total)
    - Feature 3: Advanced Preprocessor Options 🔄 NEXT
  * Phase 5: CLI Tool Integration - DEFERRED (Future)

## OBJECTIVE 15 - Phase 4 Feature 2B: Additional Site Handlers ✅ COMPLETE

### Summary
Successfully added 4 new site handlers, bringing total to 7 specialized crawlers

**New Handlers Created:**
1. ✅ Wikipedia Handler - Encyclopedia articles
2. ✅ Reddit Handler - Posts and discussions
3. ✅ Stack Overflow Handler - Q&A content
4. ✅ News Handler - News articles (CNN, BBC, Reuters, etc.)

**All 7 Handlers:**
1. Twitter/X.com - Social media posts
2. Britannica - Encyclopedia entries
3. Etymonline - Word etymologies
4. Wikipedia - Encyclopedia articles
5. Reddit - Discussion posts
6. Stack Overflow - Programming Q&A
7. News Sites - News articles

**Files Created (4):**
- src/crawler/handlers/wikipedia_handler.c (150 lines)
- src/crawler/handlers/reddit_handler.c (120 lines)
- src/crawler/handlers/stackoverflow_handler.c (110 lines)
- src/crawler/handlers/news_handler.c (140 lines)

**Files Modified (3):**
- src/crawler/handlers/handlers.h - Added 4 new handler declarations
- src/crawler/handlers/handlers.c - Register all 7 handlers
- Makefile - Added 4 new handler files to build

**Build Status:** ✅ Zero errors, clean build

## OBJECTIVE 15 - Phase 4 Feature 3: Advanced Preprocessor Options (NEXT)

### Purpose
Expose advanced crawler configuration options in the UI

### Implementation Plan

#### Step 1: UI Design
- [ ] Design UI layout for advanced options
- [ ] Add collapsible "Advanced Options" section in Crawler tab
- [ ] Plan controls placement

#### Step 2: GET Parameter Handling
- [ ] Add text input for GET parameters
- [ ] Format: key1=value1&key2=value2
- [ ] Pass parameters to crawler URL
- [ ] Display in URL preview

#### Step 3: Custom Headers
- [ ] Add text area for custom HTTP headers
- [ ] Format: Header-Name: Header-Value (one per line)
- [ ] User-Agent configuration
- [ ] Accept-Language configuration
- [ ] Pass headers to crawler

#### Step 4: Request Options
- [ ] Add timeout configuration (slider)
- [ ] Add max redirects configuration
- [ ] Add retry attempts configuration
- [ ] Pass options to crawler

#### Step 5: Integration
- [ ] Wire UI controls to crawler_thread
- [ ] Pass options through crawler_api
- [ ] Update preprocessor to use options
- [ ] Test with various configurations

## Previous Work Completed

### Feature 1: Content Filtering ✅ COMPLETE
- ✅ 4 extraction modes implemented
- ✅ UI radio buttons with click handlers
- ✅ Complete data flow wiring

### Feature 2: Site-Specific Crawlers ✅ COMPLETE
- ✅ Plugin-based handler framework
- ✅ 7 handlers: Twitter, Britannica, Etymonline, Wikipedia, Reddit, Stack Overflow, News
- ✅ Automatic handler selection
- ✅ Fallback to generic processing

## Next Steps

**CURRENT**: Implement Feature 3 (Advanced preprocessor options)
- Add UI controls for advanced options
- Wire options through the system
- Test with real URLs

**THEN**: Test all handlers with real URLs
- Verify extraction quality
- Test fallback behavior
- Measure performance

**FINALLY**: Move to Phase 5 or different OBJECTIVE

**Current State:**
- ✅ Build successful (zero errors)
- ✅ Handler framework complete
- ✅ 7 handlers working
- ✅ All handlers registered
- 🔄 Ready for Feature 3
