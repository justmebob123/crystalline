# RULES (PASTED FROM MASTER_PLAN.md)

## ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
At the beginning of EVERY response:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

## RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
Before taking ANY action:
1. Read MASTER_PLAN.md completely
2. Understand the current objectives
3. Verify your action aligns with the master plan
4. Check for any blocking priorities

## RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
The AUDIT.md contains:
- Current architectural violations
- Required fixes with priorities
- Implementation phases
- Testing requirements
- Success criteria

## RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
The SECONDARY_OBJECTIVES.md contains:
- Detailed implementation tasks
- Code examples
- File-by-file changes
- Testing procedures
- Validation steps

---

# COMPREHENSIVE URL MANAGEMENT & UI REORGANIZATION PLAN

## PHASE 1: URL MANAGER ARCHITECTURE DESIGN ⏳

### 1.1: Analyze Current Crawler Implementation ✅ COMPLETE
- [x] Read `src/crawler/crawler_core.c` - understand current URL handling
- [x] Read `src/crawler/link_management.c` - understand queue structure
- [x] Read `app/app_common.h` - understand current tab system
- [x] Document current URL management capabilities
- [x] Document current limitations

**FINDINGS:**
- Current system has `LinkQueue` with basic priority support
- Has `CrawlerLink` struct with: url, priority, added_time, source_url, crawled flag
- NO GET parameter preservation (strips query strings)
- NO blocking patterns (only basic duplicate detection)
- NO file type filtering
- NO domain diversity enforcement
- NO timestamp-based prioritization for uncrawled pages
- NO UI for URL management (only file-based)
- Current tabs: PRIME_SPIRAL, CALCULATOR, SPHERES, PRIME_FOLDING, VIDEO_GENERATOR, LLM, TRAINING, RESEARCH, CRAWLER, MODELS
- Tabs are horizontal across top (need to move to left sidebar)

### 1.2: Design Comprehensive URL Manager ✅ COMPLETE
- [x] Design URL database schema (SQLite-based)
  - URL string
  - Domain
  - Path
  - GET parameters (preserve them!)
  - Timestamp (first seen, last crawled)
  - Status (pending, crawled, failed, blocked)
  - File type
  - Priority score
  - Crawl count
- [ ] Design URL filtering system
  - File type filters (allow/block)
  - Domain whitelist/blacklist
  - URL pattern matching (regex)
  - GET parameter handling rules
- [ ] Design URL prioritization algorithm
  - Prefer uncrawled pages
  - Use prime-based randomization
  - Respect domain diversity
  - Consider page depth
- [ ] Design URL manager API
  - Add URL
  - Remove URL
  - Block URL/domain/pattern
  - Get next URL to crawl
  - Query URL status
  - Export/import URL list

### 1.3: Design Downloaded Content Viewer
- [ ] Design file browser interface
  - List downloaded files
  - Show file metadata (URL, timestamp, size, type)
  - Preview text content
  - Search functionality
- [ ] Design content organization
  - By domain
  - By file type
  - By date
  - By crawl session

---

## PHASE 2: UI REORGANIZATION ARCHITECTURE ⏳

### 2.1: Analyze Current Tab System
- [ ] Read `app/ui.c` - understand current tab rendering
- [ ] Read `app/app_common.h` - understand AppState and tab enums
- [ ] Read all tab files in `app/ui/tabs/` - understand each tab
- [ ] Document current tab structure
- [ ] Document current limitations

### 2.2: Design New Tab System with Submenus
- [ ] Design hierarchical tab structure:
  ```
  Main Tabs (Left Sidebar):
  ├─ 🤖 AI
  │  ├─ LLM Chat
  │  ├─ Training
  │  ├─ Research
  │  └─ Crawler
  ├─ 📊 Models
  │  └─ Model Management
  ├─ ⚙️ System
  │  ├─ Benchmark
  │  └─ Adapters
  └─ 📁 Data
     ├─ URL Manager
     └─ Downloaded Files
  ```
- [ ] Design left sidebar layout
  - Vertical tab buttons
  - Collapsible sections
  - Icons for each tab
  - Active tab highlighting
- [ ] Design submenu system
  - Horizontal submenu bar below main tabs
  - Only show when main tab has submenus
  - Smooth transitions

### 2.3: Design URL Manager Tab
- [ ] Design URL list view
  - Sortable columns (URL, domain, status, timestamp)
  - Filterable by status/domain/type
  - Bulk selection
  - Pagination for large lists
- [ ] Design URL actions panel
  - Add URL button (with input field)
  - Remove selected button
  - Block selected button
  - Export list button
  - Import list button
- [ ] Design filter configuration panel
  - File type checkboxes (HTML, PDF, TXT, etc.)
  - Domain whitelist/blacklist editor
  - URL pattern editor (regex)
  - GET parameter handling options
- [ ] Design block patterns panel
  - List of blocked patterns
  - Add/remove patterns
  - Test pattern against URL

### 2.4: Design Downloaded Files Viewer Tab
- [ ] Design file browser
  - Tree view by domain/date
  - List view with metadata
  - Search bar
  - Sort options
- [ ] Design file preview
  - Text content display
  - Syntax highlighting for code
  - Image preview
  - PDF preview (if possible)
- [ ] Design file actions
  - Open in external viewer
  - Delete file
  - Re-crawl URL
  - Export file

---

## PHASE 3: IMPLEMENTATION - URL MANAGER BACKEND ⏳

### 3.1: Create URL Database ✅ COMPLETE
- [x] Create `src/crawler/url_database.h`
  - Define URLEntry struct
  - Define database API
- [x] Create `src/crawler/url_database.c`
  - Implement SQLite database
  - Implement add_url() with GET parameter preservation
  - Implement remove_url()
  - Implement get_next_url() with priority
  - Implement query_urls() with filters
  - Implement update_url_status()
  - Implement block/unblock operations
  - Implement export/import functionality
  - Implement statistics functions

### 3.2: Implement URL Filtering ✅ COMPLETE
- [x] Create `src/crawler/url_filter.h`
  - Define filter rules struct
  - Define filter API
- [x] Create `src/crawler/url_filter.c`
  - Implement file type filtering (10 types: html, pdf, txt, doc, images, video, audio, archives, code, data)
  - Implement domain whitelist/blacklist with dynamic arrays
  - Implement regex pattern matching with compiled patterns
  - Implement GET parameter preservation settings
  - Implement should_crawl_url() with comprehensive checks
  - Implement add/remove operations for all filter types
  - Implement save/load configuration to file
  - Implement get_config() for UI access

### 3.3: Implement URL Prioritization ✅ COMPLETE
- [x] Create `src/crawler/url_priority.h`
  - Define priority algorithm with configurable factors
  - Define domain statistics tracking
- [x] Create `src/crawler/url_priority.c`
  - Implement uncrawled page priority boost (+100 points)
  - Implement prime-based randomization using large primes (2654435761, 2246822519, 3266489917)
  - Implement domain diversity scoring (underrepresented domains get bonus)
  - Implement time-based decay (older = higher priority)
  - Implement depth penalty (deeper URLs = lower priority)
  - Implement calculate_priority() with all factors combined
  - Implement domain statistics tracking and updates
  - Implement configurable priority factors
  - Uses crystalline math (prime_expf) for decay calculation

### 3.4: Implement Block Patterns
- [ ] Create `src/crawler/url_blocker.h`
  - Define block pattern struct
  - Define blocker API
- [ ] Create `src/crawler/url_blocker.c`
  - Implement add_block_pattern()
  - Implement remove_block_pattern()
  - Implement is_url_blocked()
  - Support regex patterns
  - Support domain blocking
  - Support path blocking

### 3.5: Update Crawler Integration
- [ ] Update `src/crawler/crawler.c`
  - Use url_database for URL storage
  - Use url_filter for filtering
  - Use url_priority for selection
  - Use url_blocker for blocking
  - Preserve GET parameters in URLs
  - Implement domain diversity in crawling

---

## PHASE 4: IMPLEMENTATION - UI REORGANIZATION ⏳

### 4.1: Implement Left Sidebar Tab System
- [ ] Update `app/ui.c`
  - Create render_left_sidebar()
  - Implement vertical tab buttons
  - Implement collapsible sections
  - Add icons for tabs
  - Handle tab switching
- [ ] Update `app/app_common.h`
  - Add MainTab enum (AI, Models, System, Data)
  - Add SubTab enums for each main tab
  - Update AppState with main_tab and sub_tab

### 4.2: Implement Submenu System
- [ ] Create `app/ui/submenu.c`
  - Implement render_submenu()
  - Handle submenu switching
  - Smooth transitions
- [ ] Create `app/ui/submenu.h`
  - Define submenu API

### 4.3: Reorganize Existing Tabs
- [ ] Move LLM tab under AI main tab
- [ ] Move Training tab under AI main tab
- [ ] Move Research tab under AI main tab
- [ ] Move Crawler tab under AI main tab
- [ ] Keep Models tab as main tab
- [ ] Move Benchmark under System main tab
- [ ] Move Adapters under System main tab

---

## PHASE 5: IMPLEMENTATION - URL MANAGER UI ⏳

### 5.1: Create URL Manager Tab
- [ ] Create `app/ui/tabs/tab_url_manager.c`
  - Implement URL list view
  - Implement sortable columns
  - Implement filtering
  - Implement pagination
- [ ] Create `app/ui/tabs/tab_url_manager.h`
  - Define tab API

### 5.2: Implement URL Actions Panel
- [ ] Add URL input field and button
- [ ] Implement remove selected URLs
- [ ] Implement block selected URLs
- [ ] Implement export URL list
- [ ] Implement import URL list

### 5.3: Implement Filter Configuration
- [ ] Create file type filter checkboxes
- [ ] Create domain whitelist/blacklist editor
- [ ] Create URL pattern editor
- [ ] Create GET parameter handling options
- [ ] Save/load filter configuration

### 5.4: Implement Block Patterns Panel
- [ ] Display list of blocked patterns
- [ ] Add pattern input and button
- [ ] Remove pattern button
- [ ] Test pattern functionality

---

## PHASE 6: IMPLEMENTATION - DOWNLOADED FILES VIEWER ⏳

### 6.1: Create Downloaded Files Tab
- [ ] Create `app/ui/tabs/tab_downloaded_files.c`
  - Implement file browser
  - Implement tree view
  - Implement list view
  - Implement search
- [ ] Create `app/ui/tabs/tab_downloaded_files.h`
  - Define tab API

### 6.2: Implement File Preview
- [ ] Implement text content display
- [ ] Implement syntax highlighting
- [ ] Implement image preview (if possible)
- [ ] Implement PDF preview (if possible)

### 6.3: Implement File Actions
- [ ] Open in external viewer
- [ ] Delete file
- [ ] Re-crawl URL
- [ ] Export file

---

## PHASE 7: TESTING & VALIDATION ⏳

### 7.1: Test URL Manager Backend
- [ ] Test URL database operations
- [ ] Test URL filtering
- [ ] Test URL prioritization
- [ ] Test block patterns
- [ ] Test GET parameter preservation
- [ ] Test domain diversity

### 7.2: Test UI Reorganization
- [ ] Test left sidebar navigation
- [ ] Test submenu switching
- [ ] Test all existing tabs still work
- [ ] Test smooth transitions

### 7.3: Test URL Manager UI
- [ ] Test adding/removing URLs
- [ ] Test blocking URLs/domains/patterns
- [ ] Test filtering and sorting
- [ ] Test export/import
- [ ] Test pagination

### 7.4: Test Downloaded Files Viewer
- [ ] Test file browsing
- [ ] Test file preview
- [ ] Test file actions
- [ ] Test search functionality

---

## PHASE 8: DOCUMENTATION ⏳

### 8.1: Update User Documentation
- [ ] Document new tab organization
- [ ] Document URL manager features
- [ ] Document downloaded files viewer
- [ ] Create user guide with screenshots

### 8.2: Update Developer Documentation
- [ ] Document URL database schema
- [ ] Document URL manager API
- [ ] Document filter system
- [ ] Document UI architecture changes

---

## PHASE 9: INTEGRATION & POLISH ⏳

### 9.1: Integrate with Existing Features
- [ ] Update crawler to use new URL manager
- [ ] Update training tab to show crawler status
- [ ] Ensure model manager works with new UI

### 9.2: Polish UI
- [ ] Add icons for all tabs
- [ ] Improve visual consistency
- [ ] Add tooltips
- [ ] Add keyboard shortcuts
- [ ] Optimize rendering performance

### 9.3: Final Testing
- [ ] End-to-end testing
- [ ] Performance testing
- [ ] User acceptance testing

---

## NOTES

**Key Requirements from User:**
1. ✅ URL manager for adding/removing URLs
2. ✅ Preserve GET parameters in URLs
3. ✅ Block specific pages/links/websites
4. ✅ Comprehensive URL manager with all features
5. ✅ File type selection/deselection
6. ✅ Block patterns
7. ✅ Prioritize uncrawled pages (timestamp tracking)
8. ✅ View downloaded pages/files
9. ✅ Reorganize tabs with submenus
10. ✅ Main AI tab with LLM, training, research, crawler
11. ✅ Tabs on left side instead of top

**Architectural Principles:**
- Follow MASTER_PLAN layer architecture
- Use crystalline math only (no math.h)
- Proper error handling
- Clean separation of concerns
- Comprehensive testing

**Current Status:** Planning phase complete, ready to proceed with implementation