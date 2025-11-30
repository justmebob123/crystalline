# TODO - Crystalline CLLM Project

## RULES (PASTE TO TOP WITH EVERY RESPONSE)
- **Rule 0:** Always paste rules to the top of `todo.md` with every response
- **Rule 1:** Always reread `MASTER_PLAN.md` before any action
- **Rule 2:** Reference `AUDIT.md` for architectural state
- **Rule 3:** Reference `SECONDARY_OBJECTIVES.md` for detailed tasks
- **Rule 4:** Do not create new `.md` files
- **Rule 5:** Always commit all changes using correct authentication
- **Rule 6:** `MASTER_PLAN.md` is read-only - do not edit without explicit approval

## Current Status: OBJECTIVE 15 Phase 4 Feature 2 COMPLETE ✅

### MASTER_PLAN Context ✅
- Read MASTER_PLAN.md
- OBJECTIVE 15: Comprehensive UI and CLI Analysis
  * Phase 1: Analysis ✅
  * Phase 2: Critical Backend Connections ✅
  * Phase 3: Layout and UI Fixes ✅
  * Phase 4: Advanced Crawler Features
    - Feature 1: Content Filtering ✅ COMPLETE
    - Feature 2: Site-Specific Crawlers ✅ COMPLETE
    - Feature 3: Advanced Preprocessor Options - PENDING
  * Phase 5: CLI Tool Integration - DEFERRED (Future)

## OBJECTIVE 15 - Phase 4 Feature 2: Site-Specific Crawlers ✅ COMPLETE

### Implementation Summary
**Purpose:** Create specialized crawlers for specific websites with custom extraction logic

**Architecture: Plugin-Based Site Handler System**
- Site handler registry for managing multiple handlers
- URL pattern matching for automatic handler selection
- Fallback to generic crawler for unknown sites
- Clean separation between handler framework and specific implementations

**Files Created:**
1. ✅ `src/crawler/site_handlers.h` - Handler API and registry (150 lines)
2. ✅ `src/crawler/site_handlers.c` - Handler registration and dispatch (200 lines)
3. ✅ `src/crawler/handlers/handlers.h` - Handler collection API
4. ✅ `src/crawler/handlers/handlers.c` - Handler registration function
5. ✅ `src/crawler/handlers/twitter_handler.c` - Twitter/X.com handler (150 lines)
6. ✅ `src/crawler/handlers/britannica_handler.c` - Britannica handler (130 lines)
7. ✅ `src/crawler/handlers/etymonline_handler.c` - Etymonline handler (120 lines)

**Files Modified:**
8. ✅ `src/crawler/preprocessor.c` - Integrated site handlers
9. ✅ `Makefile` - Added all handler files to build

**Framework Features:**
- ✅ SiteHandler structure with callbacks (can_handle, extract, cleanup)
- ✅ Handler registry (up to 32 handlers)
- ✅ URL pattern matching (case-insensitive substring)
- ✅ Handler selection by URL or name
- ✅ Automatic handler initialization
- ✅ Domain extraction utility
- ✅ Fallback to generic HTML processing

**Handler Implementations:**

**1. Twitter/X.com Handler:**
- Extracts tweet text from multiple HTML patterns
- Extracts author username from URL or meta tags
- Extracts timestamps
- Handles both twitter.com and x.com domains
- Notes about JavaScript-rendered content

**2. Britannica.com Handler:**
- Extracts article titles
- Extracts article body content
- Strips HTML tags for clean text
- Handles long articles with truncation
- Preserves article structure

**3. Etymonline.com Handler:**
- Extracts word being defined
- Extracts etymology text
- Strips HTML formatting
- Handles etymology sections
- Clean text output

**Integration with Preprocessor:**
- ✅ Handlers initialized on first preprocessor init
- ✅ Handler selection happens before generic HTML processing
- ✅ Automatic fallback if no handler matches
- ✅ Handler output logged for debugging
- ✅ Seamless integration with existing extraction modes

**Data Flow:**
```
URL arrives at preprocessor
  ↓
site_handlers_find(url) - Check for matching handler
  ↓
If handler found:
  handler->extract(html, url, &output, &len)
  ↓
  Return handler output (specialized extraction)
Else:
  Fall back to generic HTML processing
  ↓
  Apply content_filter based on extraction mode
```

**Build Status:** ✅ Zero errors, clean build

**Testing Needed:**
- [ ] Test Twitter handler with real tweet URLs
- [ ] Test Britannica handler with encyclopedia articles
- [ ] Test Etymonline handler with word pages
- [ ] Verify handler selection logic
- [ ] Verify fallback to generic crawler
- [ ] Test with mixed URL types

### Feature 3: Advanced Preprocessor Options (NEXT)
**Purpose:** Expose advanced crawler options in UI
- [ ] GET parameter handling UI controls
- [ ] Content type classification options
- [ ] Custom header configuration
- [ ] Cookie management
- [ ] JavaScript execution toggle

## Previous Work Completed

### Feature 1: Content Filtering ✅ COMPLETE
- ✅ 4 extraction modes implemented
- ✅ UI radio buttons with click handlers
- ✅ Complete data flow wiring
- ✅ Build successful

## Next Steps - User Decision Required

**Options:**
1. **Continue OBJECTIVE 15 Phase 4** - Implement Feature 3 (advanced preprocessor options)
2. **Test current implementation** - Verify site handlers work with real websites
3. **Move to OBJECTIVE 16 Phase 3** - Medium priority technical debt cleanup
4. **Move to different OBJECTIVE** - Choose from MASTER_PLAN (2-14, 17-20)

**Current State:**
- ✅ Build successful (zero errors)
- ✅ All UI tabs functional
- ✅ Backend connections working
- ✅ Content filtering fully implemented
- ✅ Site-specific handlers fully implemented
- ✅ 3 specialized crawlers ready (Twitter, Britannica, Etymonline)
- ✅ Plugin architecture for easy handler addition
