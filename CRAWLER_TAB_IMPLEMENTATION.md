# Crawler Tab Complete Implementation Plan

## Current State
- **724 lines** - Basic Crystalline UI implementation
- 2-column layout (URL list + controls)
- Basic functionality: URL management, start/stop/clear

## Target State
- **~950 lines** - Full feature implementation
- 3-column layout (Config + Management + Status)
- All advanced features from original

## Missing Features (8 major areas)

### 1. Prime Configuration Panel
**Location:** Column 1, top section
**Components:**
- Prime frequency input (integer)
- Prime selection method dropdown (FIRST, RANDOM, SEQUENTIAL)
- Delay min input (integer, milliseconds)
- Delay max input (integer, milliseconds)
- Enable/disable toggle checkbox

**Implementation:**
- Use Crystalline input fields
- Add validation (min < max, positive values)
- Store in CrawlerTabState (already defined)

### 2. URL Pattern Selection
**Location:** Column 1, middle section
**Components:**
- 4 checkboxes:
  * href attributes (default: ON)
  * JavaScript onclick (default: ON)
  * Data attributes (default: ON)
  * Meta refresh (default: ON)

**Implementation:**
- Use custom checkbox rendering (Crystalline doesn't have checkboxes)
- Store in CrawlerTabState.pattern_* fields (already defined)

### 3. Content Filtering
**Location:** Column 1, lower section
**Components:**
- 4 radio buttons:
  * Extract All (default)
  * Human Text Only
  * Metadata Only
  * Mixed (Content + Meta)

**Implementation:**
- Use custom radio button rendering
- Store in CrawlerTabState.extraction_mode (already defined)

### 4. Advanced Options Panel
**Location:** Column 1, bottom section
**Components:**
- Collapsible panel with toggle button
- When expanded:
  * GET Parameters input
  * Custom Headers input
  * Timeout seconds input
  * Max Redirects input

**Implementation:**
- Toggle button to show/hide
- Use Crystalline input fields
- Store in CrawlerTabState.advanced_* fields (already defined)

### 5. Activity Log
**Location:** Column 3, bottom section
**Components:**
- 10-line scrolling log
- Timestamped messages
- Auto-scroll to latest

**Implementation:**
- Use CrawlerTabState.activity_log array (already defined)
- Render as text list with scroll
- Add helper function: add_activity_log()

### 6. Model Selector
**Location:** Column 2 or 3, top section
**Components:**
- Dropdown showing available models
- Callback to store selected model

**Implementation:**
- Use existing ModelSelector component
- Store in crawler_selected_model_name (already defined)

### 7. Save/Load Config Buttons
**Location:** Column 1 or 2, bottom
**Components:**
- Save Config button
- Load Config button

**Implementation:**
- Save to JSON file (data/crawler_config.json)
- Load from JSON file
- Persist all settings

### 8. Reset URLs Button
**Location:** Column 2, with other controls
**Components:**
- Reset URLs button

**Implementation:**
- Clear URL manager database
- Reset to empty state
- Add confirmation dialog

## Layout Structure

### 3-Column Layout
```
┌─────────────────┬─────────────────┬─────────────────┐
│   Column 1      │   Column 2      │   Column 3      │
│   (33%)         │   (33%)         │   (33%)         │
├─────────────────┼─────────────────┼─────────────────┤
│ Prime Config    │ URL Input       │ Model Selector  │
│                 │ Add URL Button  │                 │
│ URL Patterns    │                 │ Status Display  │
│                 │ URL List        │                 │
│ Content Filter  │                 │ Crawler Stats   │
│                 │ Start Button    │                 │
│ Advanced Opts   │ Stop Button     │ Activity Log    │
│                 │ Clear Button    │                 │
│                 │ Reset URLs      │                 │
│                 │ Save/Load       │                 │
└─────────────────┴─────────────────┴─────────────────┘
```

## Implementation Steps

### Phase 1: Layout Restructure (30 min)
1. Change from 2-column to 3-column layout
2. Adjust panel sizes and positions
3. Update init_crawler_tab() function

### Phase 2: Column 1 - Configuration (1.5 hours)
1. Add Prime Configuration inputs (30 min)
2. Add URL Pattern checkboxes (20 min)
3. Add Content Filtering radio buttons (20 min)
4. Add Advanced Options panel (20 min)

### Phase 3: Column 2 - Management (30 min)
1. Keep existing URL management
2. Add Reset URLs button
3. Add Save/Load Config buttons

### Phase 4: Column 3 - Status (45 min)
1. Add Model Selector dropdown
2. Enhance status display
3. Add Activity Log

### Phase 5: Wiring & Logic (45 min)
1. Wire all input callbacks
2. Implement save/load config
3. Implement reset URLs
4. Add activity log messages

### Phase 6: Testing (30 min)
1. Test all inputs
2. Test save/load
3. Test crawling with new config
4. Verify all features work

## Total Estimated Time: 4 hours

## Success Criteria
- ✅ All 8 missing features implemented
- ✅ 3-column layout working
- ✅ All inputs functional
- ✅ Save/load config working
- ✅ Activity log displaying messages
- ✅ 0 errors, 0 warnings
- ✅ ~950 lines of code

---

**Created:** 2024-12-07
**Status:** Ready to implement