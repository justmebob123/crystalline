# TODO - Crystalline CLLM Integration - Tab Rewrite Phase

## Current Status: Models & Crawler Tab Rewrite IN PROGRESS
- **Phase**: Complete tab rewrite using pure Crystalline UI
- **Progress**: 85% Complete
- **Status**: Fixing API mismatches in rewritten tabs

## IMMEDIATE TASK: Model Manager Concurrent Access Fix

### Problem FIXED ✅
The LLM tab was loading a separate copy of the model from disk, causing training to stop when trying to use the model for inference.

### Solution IMPLEMENTED ✅
Changed LLM tab to use `model_manager_acquire_read()` instead of `app_load_model()`, enabling:
- Concurrent read access while training continues
- Single model instance in memory
- Proper read/write lock coordination

### Changes Made
1. ✅ Added `active_model_name` field to track read lock
2. ✅ Changed model loading to use `model_manager_prepare()` + `model_manager_acquire_read()`
3. ✅ Added `model_manager_release_read()` to cleanup function
4. ✅ Added proper error handling and user feedback

## Completed Tasks ✅

### Task 1: Model Manager Concurrent Access ✅ COMPLETE
- [x] Identified root cause (LLM tab loading separate model copy)
- [x] Changed to use model_manager_acquire_read()
- [x] Added active_model_name tracking
- [x] Added model_manager_release_read() to cleanup
- [x] Added proper error handling
- [x] Created comprehensive documentation

### Task 1.5: Model Loading Fix ✅ COMPLETE
- [x] Performed depth-3 analysis of model manager
- [x] Identified root cause: prepare() doesn't load model
- [x] Fixed LLM tab to use model_manager_reload()
- [x] Fixed Models tab to use model_manager_reload()
- [x] Enhanced Models tab info display
- [x] Fixed header file signature mismatch
- [x] Build succeeds with zero errors
- [x] Created comprehensive documentation

### Task 2: Fix Models Tab API ✅ COMPLETE
- [x] Added CrystallineElementStyle parameter to all element creations
- [x] Fixed panel creation calls
- [x] Fixed list creation calls
- [x] Fixed button creation calls
- [x] Fixed textarea type name (CrystallineTextArea)
- [x] Fixed event handlers to use unified handle_mouse API
- [x] Fixed model info display to use correct field names
- [x] Build succeeds

### Task 3: Fix Crawler Tab API ✅ COMPLETE
- [x] Fixed CrystallineTextarea → CrystallineTextArea type name
- [x] Added CrystallineElementStyle parameter to all element creations
- [x] Fixed all element creation calls
- [x] Fixed crawler function signatures
- [x] Fixed CrawlerStatus field names
- [x] Simplified stats display (removed get_crawler_state dependency)
- [x] Fixed event handlers to use unified handle_mouse API
- [x] Build succeeds

### Task 4: Wire Tabs into Main Event Loop ✅ COMPLETE
- [x] Added Models tab to all event handlers
- [x] Added Crawler tab to all event handlers
- [x] Updated render switch cases
- [x] Removed legacy click handler

### Task 5: Delete Legacy Code ⚠️ TODO (NEXT)
- [ ] Delete app/ui/components/ directory
- [ ] Remove legacy includes from Makefile
- [ ] Verify build succeeds

### Task 6: Final Testing ⚠️ TODO
- [ ] Test concurrent training + inference
- [ ] Test Models tab
- [ ] Test Crawler tab
- [ ] Test all tabs work correctly

## Reference: Correct Crystalline UI API

### Panel Creation
```c
CrystallinePanel* crystalline_panel_create(
    CrystallineElementStyle style,  // CRYSTALLINE_STYLE_RECTANGULAR
    float x, float y,                // CENTER coordinates
    float width, float height,
    const char* title,
    TTF_Font* title_font
);
```

### List Creation
```c
CrystallineList* crystalline_list_create(
    CrystallineElementStyle style,
    float x, float y,
    float width, float height,
    TTF_Font* font
);
```

### Button Creation
```c
CrystallineButton* crystalline_button_create(
    CrystallineElementStyle style,
    float x, float y,
    float width, float height,
    const char* label,
    TTF_Font* font
);
```

### Slider Creation
```c
CrystallineSlider* crystalline_slider_create(
    CrystallineElementStyle style,
    float x, float y,
    float width, float height,
    float min_value, float max_value
);
```

### Input Creation
```c
CrystallineInput* crystalline_input_create(
    CrystallineElementStyle style,
    float x, float y,
    float width, float height,
    const char* placeholder,
    TTF_Font* font
);
```

### TextArea Creation
```c
CrystallineTextArea* crystalline_textarea_create(
    CrystallineElementStyle style,
    float x, float y,
    float width, float height,
    TTF_Font* font
);
```

## Build Status
- ❌ Compilation errors in crawler tab
- ⚠️ Models tab not yet tested
- ✅ Main event loop updated
- ✅ All other tabs building successfully

## Completed Work

### Phase 6-8: UI System Fixes ✅ COMPLETE
- [x] Fixed all layout issues
- [x] Fixed event system
- [x] Standardized all tabs
- [x] Created comprehensive documentation
- [x] Fixed LLM tab completely

### Tab Status (7 of 9 Complete)
1. ✅ Training - Reference implementation
2. ✅ LLM - Complete with model dropdown
3. ✅ Video Generator - Event handlers fixed
4. ✅ Research - Event handlers fixed
5. ✅ URL Manager - Event handlers fixed
6. ✅ Downloaded Files - Event handlers fixed
7. ✅ Benchmark - Event handlers fixed
8. ⚠️ Models - Rewritten, needs API fixes
9. ⚠️ Crawler - Rewritten, needs API fixes

## Next Steps
1. Fix Models tab API mismatches
2. Fix Crawler tab API mismatches
3. Build and test both tabs
4. Delete legacy code
5. Final verification