# Comprehensive Tab Audit - All Tabs Analysis

## Purpose
Systematic analysis of ALL tabs to identify missing functionality after Crystalline UI rewrites.

## Tabs to Audit

### 1. Training Tab ✅ ANALYZED
**Status:** Refactored for Phase 3 - appears complete
**File:** `app/ui/tabs/tab_training.c`
**Features:**
- Model selection dropdown
- Create new model
- Start/Stop training
- Training parameters (epochs, batch size, etc.)
- Real-time statistics display
- Sphere visualization
- Per-tab model ownership

**Action:** ✅ Complete - Phase 3.2 refactor successful

### 2. LLM Tab ✅ ANALYZED
**Status:** Refactored for Phase 3 - appears complete
**File:** `app/ui/tabs/tab_llm.c`
**Features:**
- Model selection dropdown
- Text input for prompts
- Generate button
- Output display
- Per-tab model ownership
- Independent inference

**Action:** ✅ Complete - Phase 3.3 refactor successful

### 3. Models Tab ✅ ANALYZED
**Status:** Refactored for Phase 3 - appears complete
**File:** `app/ui/tabs/tab_models.c`
**Features:**
- Model list display
- Model selection
- Model metadata display
- Delete model button
- Refresh button
- View-only monitoring UI

**Action:** ✅ Complete - Phase 3.4 refactor successful

### 4. Crawler Tab ⚠️ CRITICAL ISSUES FOUND
**Status:** INCOMPLETE - Major functionality missing
**File:** `app/ui/tabs/tab_crawler.c`
**Current Size:** 644 lines
**Original Size:** 699 lines (commit b5921a9)

**Missing Features:**
1. ❌ Prime Configuration Panel
   - Frequency Prime input
   - Selection Prime input
   - Delay Min/Max inputs
   - Enable/Disable toggle
   - Real-time prime validation

2. ❌ SQLite URL Manager Integration
   - Persistent URL storage (`data/crawler/crawler.db`)
   - URL statistics (total, pending, crawled, blocked)
   - URL deduplication
   - URL status tracking

3. ❌ URL Pattern Selection
   - Extract from href attributes
   - Extract from onclick handlers
   - Extract from data-* attributes
   - Extract from meta refresh tags

4. ❌ Activity Log
   - 10-line scrolling log
   - Real-time status updates
   - Action confirmations

5. ❌ Configuration Persistence
   - Save Config button
   - Load Config button

6. ❌ Advanced UI
   - 3-column layout (current: 2-column)
   - Input validation indicators
   - Confirmation messages with timers

**Current Features (Simplified):**
- ✅ URL input field
- ✅ Add URL button
- ✅ Clear URLs button
- ✅ Start/Stop buttons
- ✅ URL list display
- ✅ Basic stats display
- ✅ Sliders (max_depth, max_urls, rate_limit)

**Action Required:** 🔴 HIGH PRIORITY - Restore missing functionality

### 5. Research Tab 🔍 NEEDS ANALYSIS
**Status:** Needs detailed comparison
**File:** `app/ui/tabs/tab_research.c`
**Current Size:** 735 lines
**Previous Size:** 728 lines (commit 90ee326)

**Action:** Analyze for missing functionality

### 6. URL Manager Tab 🔍 NEEDS ANALYSIS
**Status:** Needs detailed comparison
**File:** `app/ui/tabs/tab_url_manager.c`

**Action:** Analyze for missing functionality

### 7. Downloaded Files Tab 🔍 NEEDS ANALYSIS
**Status:** Needs detailed comparison
**File:** `app/ui/tabs/tab_downloaded_files.c`

**Action:** Analyze for missing functionality

### 8. Video Tab 🔍 NEEDS ANALYSIS
**Status:** Needs detailed comparison
**File:** `app/ui/tabs/tab_video.c`

**Action:** Analyze for missing functionality

### 9. Benchmark Tab 🔍 NEEDS ANALYSIS
**Status:** Needs detailed comparison
**File:** `app/ui/tabs/tab_benchmark.c`

**Action:** Analyze for missing functionality

## Analysis Methodology

For each tab:
1. Find the commit before Crystalline UI rewrite
2. Compare line counts
3. Compare feature lists
4. Identify missing functionality
5. Assess impact (LOW/MEDIUM/HIGH/CRITICAL)
6. Document restoration requirements

## Priority Order

### Immediate (Critical):
1. 🔴 Crawler Tab - Major features missing

### High Priority:
2. URL Manager Tab - May have database integration issues
3. Downloaded Files Tab - File management features

### Medium Priority:
4. Video Tab - Media handling features
5. Benchmark Tab - Testing features

### Low Priority (Already Complete):
- Training Tab ✅
- LLM Tab ✅
- Models Tab ✅
- Research Tab (appears complete)

## Next Steps

1. Complete analysis of remaining tabs
2. Prioritize restoration work
3. Create restoration plan for each tab
4. Implement restorations systematically
5. Test each tab thoroughly

## Status
**Tabs Analyzed:** 4/9
**Critical Issues Found:** 1 (Crawler Tab)
**Next:** Analyze URL Manager, Downloaded Files, Video, Benchmark tabs