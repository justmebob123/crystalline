# Crawler Tab Restoration Plan

## Critical Finding
**Original Implementation:** 953 lines (commit 7f6e854^)
**Current Implementation:** 644 lines (current)
**Reduction:** 309 lines (32% smaller)

## Complete Feature Comparison

### Original Features (7f6e854^)

#### 1. Prime Configuration Panel ✓ EXISTED
```c
typedef struct {
    CrawlerPrimeConfig prime_config;
    bool prime_enabled;
} CrawlerTabState;
```
**Features:**
- Frequency Prime input with validation
- Selection Prime input with validation
- Delay Min input with validation
- Delay Max input with validation
- Enable/Disable toggle button
- Real-time prime validation (OK/X indicators)
- Integration with `prime_randomization.h`

#### 2. URL Pattern Selection ✓ EXISTED
```c
bool pattern_href;
bool pattern_onclick;
bool pattern_data_attr;
bool pattern_meta_refresh;
```
**Features:**
- 4 checkboxes for different link extraction types
- Integration with `url_patterns.h`
- Configurable link extraction behavior

#### 3. Content Filtering ✓ EXISTED
```c
ExtractionMode extraction_mode;
SDL_Rect radio_extract_all;
SDL_Rect radio_extract_human;
SDL_Rect radio_extract_metadata;
SDL_Rect radio_extract_mixed;
```
**Features:**
- 4 radio buttons for extraction modes
- Integration with `content_filter.h`
- Configurable content extraction

#### 4. Advanced Options ✓ EXISTED
```c
bool show_advanced_options;
char get_parameters[256];
char custom_headers[512];
int timeout_seconds;
int max_redirects;
SDL_Rect advanced_toggle_rect;
```
**Features:**
- Collapsible advanced options panel
- GET parameters input
- Custom headers input
- Timeout configuration
- Max redirects configuration

#### 5. SQLite URL Manager ✓ EXISTED
```c
CrawlerURLManager* url_manager;
```
**Features:**
- Persistent URL storage in `data/crawler/crawler.db`
- URL statistics (total, pending, crawled, blocked)
- URL status tracking
- URL deduplication
- Integration with `crawler_url_manager.h`

#### 6. Activity Log ✓ EXISTED
```c
char activity_log[10][256];
int activity_count;
```
**Features:**
- 10-line scrolling activity log
- Real-time status updates
- Action confirmations
- Error messages

#### 7. Configuration Persistence ✓ EXISTED
```c
static UIButton btn_save_config;
static UIButton btn_load_config;
```
**Features:**
- Save configuration to file
- Load configuration from file
- Persistent settings across sessions

#### 8. Model Selector ✓ EXISTED
```c
static ModelSelector* crawler_model_selector = NULL;
static char crawler_selected_model_name[256] = {0};
```
**Features:**
- Model dropdown selector
- On-demand model loading (not loaded until crawling starts)
- Prevents OOM issues

#### 9. Additional Buttons ✓ EXISTED
```c
static UIButton btn_reset_urls;  // Reset all URLs to pending
```

#### 10. 3-Column Layout ✓ EXISTED
```c
draw_column1_prime_config()
draw_column2_link_management()
draw_column3_status()
```

### Current Features (Simplified)

#### What EXISTS:
- ✅ URL input field
- ✅ Add URL button
- ✅ Clear URLs button
- ✅ Start/Stop buttons
- ✅ URL list display
- ✅ Basic stats display
- ✅ Sliders (max_depth, max_urls, rate_limit)
- ✅ Model selection (via global state)

#### What's MISSING:
- ❌ Prime Configuration Panel (5 inputs + toggle)
- ❌ URL Pattern Selection (4 checkboxes)
- ❌ Content Filtering (4 radio buttons)
- ❌ Advanced Options (collapsible panel with 4 inputs)
- ❌ Activity Log (10-line scrolling log)
- ❌ Save/Load Config buttons
- ❌ Reset URLs button
- ❌ Model Selector (proper dropdown)
- ❌ 3-column layout (current: 2-column)
- ❌ Input validation indicators

## Restoration Plan

### Phase 1: Restore Core Features (2-3 hours)
**Priority: CRITICAL**

1. **Add Prime Configuration Panel** (45 min)
   - Add CrawlerPrimeConfig to state
   - Create 4 input fields (frequency, selection, delay_min, delay_max)
   - Add enable/disable toggle
   - Implement prime validation
   - Add OK/X indicators

2. **Add URL Pattern Selection** (30 min)
   - Add 4 boolean fields to state
   - Create 4 checkboxes
   - Wire to crawler configuration

3. **Restore SQLite Integration** (30 min)
   - Verify CrawlerURLManager is properly initialized
   - Add URL statistics display
   - Implement proper URL status tracking

4. **Add Activity Log** (30 min)
   - Add activity_log array to state
   - Create scrolling log display
   - Wire to crawler events

### Phase 2: Restore Advanced Features (1-2 hours)
**Priority: HIGH**

5. **Add Content Filtering** (30 min)
   - Add ExtractionMode to state
   - Create 4 radio buttons
   - Wire to crawler configuration

6. **Add Advanced Options Panel** (45 min)
   - Add collapsible panel
   - Create 4 input fields
   - Add toggle button

7. **Add Configuration Persistence** (30 min)
   - Implement save_config()
   - Implement load_config()
   - Add Save/Load buttons

### Phase 3: Restore UI Features (1 hour)
**Priority: MEDIUM**

8. **Expand to 3-Column Layout** (30 min)
   - Restructure layout
   - Move components to appropriate columns
   - Adjust spacing and sizing

9. **Add Model Selector** (30 min)
   - Create ModelSelector instance
   - Wire to crawler thread
   - Implement on-demand loading

### Phase 4: Testing & Verification (1 hour)
**Priority: HIGH**

10. **Test All Features**
    - Prime configuration
    - URL patterns
    - Content filtering
    - Advanced options
    - Activity log
    - Config save/load
    - Model selection

## Implementation Strategy

### Approach 1: Incremental Restoration (RECOMMENDED)
- Restore features one by one
- Test after each feature
- Commit after each working feature
- Easier to debug and verify

**Estimated Time:** 5-7 hours
**Risk:** LOW
**Benefit:** Stable, tested implementation

### Approach 2: Full Restoration
- Restore all features at once
- Test everything together
- Single large commit

**Estimated Time:** 4-5 hours
**Risk:** MEDIUM
**Benefit:** Faster completion

### Approach 3: Hybrid (Copy + Adapt)
- Copy original implementation
- Adapt to Crystalline UI system
- Test and refine

**Estimated Time:** 3-4 hours
**Risk:** MEDIUM
**Benefit:** Preserves all original logic

## Recommended Approach

**Use Approach 3 (Hybrid):**
1. Copy the original crawler tab implementation
2. Systematically convert each section to Crystalline UI
3. Preserve ALL functionality
4. Test thoroughly
5. Commit to main (no feature branches per MASTER_PLAN.md)

## Files to Reference

### Original Implementation:
- `app/ui/tabs/tab_crawler.c` (commit 7f6e854^) - 953 lines
- Full feature set with all integrations

### Current Implementation:
- `app/ui/tabs/tab_crawler.c` (current) - 644 lines
- Simplified version missing major features

### Required Headers:
- `src/crawler/prime_randomization.h`
- `src/crawler/url_patterns.h`
- `src/crawler/content_filter.h`
- `src/crawler/crawler_url_manager.h`
- `ui/model_selector.h`

## Success Criteria

### Must Have:
- ✅ All 10 original features restored
- ✅ Build successful (0 errors)
- ✅ All buttons functional
- ✅ All inputs working
- ✅ SQLite integration working
- ✅ Model selector working

### Nice to Have:
- ✅ Improved visual design
- ✅ Better error handling
- ✅ Enhanced user feedback

## Status
**Current:** Analysis complete
**Next:** Begin restoration (Approach 3 recommended)
**Priority:** CRITICAL - Core functionality missing
**Estimated Time:** 3-4 hours for full restoration