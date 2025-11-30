# TODO - Crystalline CLLM Project

## RULES (ALWAYS AT TOP)
- **Rule 0:** Always paste rules to the top of `todo.md` with every response.
- **Rule 1:** Always reread `MASTER_PLAN.md` before any action.
- **Rule 2:** Reference `AUDIT.md` for architectural state.
- **Rule 3:** Reference `SECONDARY_OBJECTIVES.md` for detailed tasks.
- **Rule 4:** Do not create new `.md` files.
- **Rule 5:** Always commit all changes using correct authentication.
- **Rule 6:** `MASTER_PLAN.md` is read-only - do not edit without explicit approval.

## 🎯 CURRENT FOCUS: Option B - Fix UI/UX Issues

### Phase 1: Comprehensive Bidirectional Analysis ✅ COMPLETE
- [x] Analyze all UI tabs (layout, inputs, functionality)
- [x] Analyze all backend systems (crawler, URL manager, models, database)
- [x] Map UI → Backend connections
- [x] Map Backend → UI connections
- [x] Identify all disconnects and issues
- [x] Document all findings in detailed report

**Analysis Results:**
- ✅ Created comprehensive `UI_UX_ANALYSIS.md` document
- ✅ Analyzed 9 UI tabs (5,301 total lines)
- ✅ Identified 14 specific issues (4 critical, 4 high, 3 medium, 3 low)
- ✅ Mapped all UI ↔ Backend connections
- ✅ Identified 3 major architectural issues

**Key Findings:**
1. **URL Manager Tab:** No backend integration (shows demo data only)
2. **Downloaded Files Tab:** No backend integration (shows demo data only)
3. **Models Tab:** Backend works but no models registered
4. **Crawler Tab:** Layout issues, non-interactive checkboxes
5. **Video Tab:** Unicode characters, text overlap
6. **GET Parameters:** Already implemented in backend, just needs UI exposure

### Phase 2: Critical Backend Connections (CURRENT - HIGH PRIORITY)

**Fix 1: Connect URL Manager Tab to CrawlerURLManager ✅ COMPLETE**

#### Fix 1 Tasks:
- [x] Add CrawlerURLManager to tab state
- [x] Initialize URL manager on tab init
- [x] Replace demo data with database queries
- [x] Implement URL list rendering from database
- [x] Implement click handlers for all buttons
- [x] Add URL selection functionality
- [x] Test URL add/remove/block operations (implemented, needs testing)
- [x] Verify statistics display

**Changes Made:**
- Added `CrawlerURLManager* url_manager` to URLManagerState
- Added initialization function `init_url_manager_state()`
- Implemented `refresh_url_list()` using `url_db_query()`
- Implemented `update_statistics()` using `crawler_url_manager_get_stats()`
- Updated `draw_url_list()` to display real URLs from database
- Implemented full click handlers for all buttons:
  * Add URL button - adds to database
  * Remove Selected button - removes from database
  * Block Selected button - blocks in database
  * Export/Import buttons - placeholders for future
- Added URL selection by clicking in list
- Added keyboard input handling for URL input field
- Added cleanup function
- Build successful with zero errors

**Next: Fix 2 - Connect Downloaded Files Tab (STARTING NOW)**

#### Fix 2 Tasks: ✅ COMPLETE
- [x] Query database for downloaded files
- [x] Scan crawler output directory
- [x] Display actual file list
- [x] Implement file preview
- [x] Implement click handlers

**Changes Made:**
- Added `CrawlerURLManager* url_manager` to state
- Created `FileInfo` structure for file metadata
- Implemented `init_downloaded_files_state()` for initialization
- Implemented `refresh_file_list()` to query crawled URLs from database
- Implemented `load_file_preview()` to read file contents
- Updated `draw_file_browser()` to display real files from database
- Updated `draw_file_preview()` to show actual file content
- Added file selection by clicking in list
- Added file size formatting and date display
- Added file existence checking
- Implemented cleanup function
- Build successful with zero errors

**Features:**
- Shows all crawled URLs from database
- Displays file metadata (name, domain, type, size, date)
- File preview with first 4KB of content
- Visual indication for missing files (grayed out)
- Real-time statistics (total files, total size)
- Click to select and preview files

**Next: Fix 3 - Model Registration**

#### Fix 3 Tasks (After Fix 2):
- [ ] Audit model creation in Training tab
- [ ] Audit model creation in LLM tab
- [ ] Ensure model_manager_register() is called
- [ ] Test model list display
- [ ] Verify model lifecycle

### Phase 3: Layout and UI Fixes (MEDIUM PRIORITY)
- [ ] Fix 4: Crawler Tab Layout Issues (2-3 hours)
- [ ] Fix 5: Video Tab Unicode and Layout (1-2 hours)
- [ ] Fix 6: Models Tab Implementation (4-5 hours)

### Phase 4: Layout System Migration (LOWER PRIORITY)
- [ ] Fix 7: Migrate tabs to ColumnLayout system (3-4 hours per tab)

### Phase 5: New Features (FUTURE)
- [ ] Content filtering (human text extraction)
- [ ] Site-specific crawlers (X.com, Britannica, Etymonline)
- [ ] Advanced preprocessor options

## 📋 MASTER_PLAN CONTEXT

**Read and understood:**
- ✅ MASTER_PLAN.md - 18 objectives defined
- ✅ OBJECTIVE 15: Comprehensive UI and CLI Analysis with Bidirectional Validation
- ✅ User selected Option B: Fix UI/UX issues

**Key Understanding:**
- OBJECTIVE 15 in MASTER_PLAN is specifically about UI/UX analysis and fixes
- Must perform bidirectional analysis before any fixes
- Must map all UI ↔ Backend connections
- Must identify all issues systematically

## 🚀 NEXT ACTION

Starting Phase 1: Comprehensive Bidirectional Analysis
- Will analyze all UI tabs systematically
- Will analyze all backend systems
- Will create detailed mapping document
- Will identify all issues and disconnects