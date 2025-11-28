# TODO: Comprehensive UI/Metrics Enhancement

## Context
User reported multiple UI issues after testing the training system:
- Progress bars not updating
- Epoch shows 0
- Thread count shows 0  
- Orange spheres not in legend
- Visualization too small
- Missing features: zoom, 3D view, time estimates, terminal output, collapsible panels, model export

See: UI_METRICS_DEEP_ANALYSIS.md for complete analysis

---

## Phase 1: Critical Bug Fixes (IMMEDIATE - 1 hour) ✅ COMPLETE

### 1.1 Fix Epoch Number Display ✅
**Problem**: Epoch always shows 0 because it's hardcoded in `threaded_train_epoch_lockfree()`

**Files to Modify**:
- [ ] `include/cllm_training_threaded.h` - Add epoch parameter to function signature
- [ ] `src/ai/cllm_training_threaded.c` - Accept epoch parameter, pass to metrics
- [ ] `app/training_thread.c` - Pass current epoch when calling function
- [ ] `src/crawler/continuous_training.c` - Update caller
- [ ] `demos/threaded_training_demo.c` - Update caller
- [ ] `tests/test_repo_training.c` - Update caller
- [ ] `tools/train_model.c` - Update caller

**Changes**:
```c
// Header
float threaded_train_epoch_lockfree(ThreadedTrainingSystem* system, int current_epoch);

// Implementation (line 1953, 1986)
cllm_metrics_update_training_progress(system->metrics, current_epoch, batches_pushed, total_batches_in_epoch);

// Caller
float loss = threaded_train_epoch_lockfree(g_threaded_system, state->training_current_epoch);
```

### 1.2 Fix Thread Count Display ✅
**Problem**: Shows 0 despite 7 active threads

**Investigation**:
- [ ] Check if `state->sphere_stats.active_spheres` is being updated
- [ ] Check if metrics callback is resetting the count
- [ ] Verify the count logic in `app/training_thread.c:72-75`

**Likely Issue**: `active_spheres` is reset to 0 at start of callback but not all threads are counted

**Fix**: Ensure all active threads are counted properly

### 1.3 Add Orange to Legend ✅ COMPLETE
**Problem**: Orange spheres visible but not in legend

**Root Cause**: Orange = "Very High Activity" (>90% of max batches)

**Fix**:
- [ ] Update legend in `app/ui/sphere_visualization.c` to show all 5 colors with labels
- [ ] Make legend more prominent and descriptive

### 1.4 Fix Loading Screen Display ✅ ENHANCED
**Problem**: Preprocessing status messages don't show in UI

**Files to Modify**:
- [ ] `app/training_thread.c` - Add `SDL_PumpEvents()` after status updates
- [ ] Force UI redraw during preprocessing

**Changes**:
```c
// After each status message update
state->training_preprocessing_progress = 0.1f;
snprintf(state->training_status_message, ...);
SDL_PumpEvents();  // Process events to trigger UI update
usleep(16666);     // 60 FPS = 16.6ms per frame
```

---

## Phase 2: Major Enhancements (HIGH PRIORITY - 2 hours) ⏳ IN PROGRESS

### 2.1 Expand Visualization Size ✅ COMPLETE
**Problem**: Visualization doesn't fill available space

**Current**: Fixed size calculation
**Target**: Dynamic sizing based on available space

**Files to Modify**:
- [ ] `app/ui/sphere_visualization.c` - Calculate bounds dynamically
- [ ] `app/ui/tabs/tab_training.c` - Pass larger bounds

**Changes**:
```c
// Use full visualization area
int viz_width = RENDER_WIDTH;
int viz_height = WINDOW_HEIGHT - 60;
SDL_Rect viz_bounds = {0, 60, viz_width, viz_height};
```

### 2.2 Add Time Estimates Display ✅ VERIFIED WORKING
**Problem**: Timing data exists but not displayed

**Files to Modify**:
- [ ] `app/ui/tabs/tab_training.c` - Add time display section

**Data Available**:
- `metrics->training.elapsed_time_seconds`
- `metrics->training.estimated_time_remaining_seconds`
- `metrics->training.batches_per_second`

**Display Format**:
```
Elapsed: 5m 23s | Remaining: 12m 45s | Speed: 45.3 batches/sec | ETA: 3:45 PM
```

### 2.3 Add Terminal Output Window ❌
**Problem**: Debug output only in console

**Files to Modify**:
- [ ] `app/ui/tabs/tab_training.c` - Add scrolling text box
- [ ] Create circular buffer for last 500 lines
- [ ] Capture printf output or duplicate to buffer

**Features**:
- Scrollable text area
- Auto-scroll toggle
- Search/filter capability
- Copy to clipboard

### 2.4 Add Collapsible Panels ❌
**Problem**: UI cluttered, can't focus on specific data

**Files to Modify**:
- [ ] `app/ui/tabs/tab_training.c` - Add expand/collapse buttons
- [ ] `app/app_common.h` - Add panel state flags

**Panels to Make Collapsible**:
1. Framework Status
2. Performance Metrics
3. Thread State Legend
4. Sphere Statistics Table
5. Terminal Output (when added)

**State Storage**:
```c
struct {
    bool framework_expanded;
    bool performance_expanded;
    bool legend_expanded;
    bool stats_table_expanded;
    bool terminal_expanded;
} panel_states;
```

---

## Phase 3: Advanced Features (NICE TO HAVE - 3-4 hours)

### 3.1 Recursive Sub-Spheres Visualization ❌
**Problem**: Only shows Level 1, not recursive hierarchy

**Requirements** (per MASTER_PLAN OBJECTIVE 6A):
- Detect when threads spawn children
- Draw nested spheres around parent
- Use different sizes for hierarchy levels
- Add connecting lines

**Implementation**:
- [ ] Extend metrics to track parent-child relationships
- [ ] Add hierarchy level to thread metrics
- [ ] Render sub-spheres recursively
- [ ] Scale size by level (Level 1 = large, Level 2 = medium, etc.)

### 3.2 Zoom and Mouse-Over ❌
**Problem**: Can't inspect individual spheres

**Features**:
- [ ] Mouse-over detection (sphere hit testing)
- [ ] Tooltip with sphere details:
  * Thread ID
  * State (WORKING/IDLE/etc)
  * Batches processed
  * Current loss
  * CPU usage
- [ ] Zoom controls (mouse wheel)
- [ ] Pan capability (drag with mouse)

**Files to Modify**:
- [ ] `app/ui/sphere_visualization.c` - Add interaction handlers
- [ ] `app/input_manager.c` - Handle mouse events

### 3.3 3D Visualization ❌
**Problem**: Only 2D projection available

**Options**:
1. Software 3D rendering (simpler)
2. OpenGL integration (better performance)

**Features**:
- [ ] 3D sphere rendering
- [ ] Rotation controls (drag to rotate)
- [ ] Perspective projection
- [ ] True spatial relationships

### 3.4 2D/3D Toggle ❌
**Files to Modify**:
- [ ] `app/ui/tabs/tab_training.c` - Add toggle button
- [ ] `app/app_common.h` - Add visualization mode flag

**State**:
```c
enum VisualizationMode {
    VIZ_MODE_2D,
    VIZ_MODE_3D
};
```

### 3.5 Model Export from LLM Tab ❌
**Problem**: Can't save trained models

**Files to Modify**:
- [ ] `app/ui/tabs/tab_llm.c` - Add "Export Model" button
- [ ] Add file dialog for save location
- [ ] Call `cllm_save_model()` with user-specified path

**Features**:
- Export to custom location
- Export options (format, compression)
- Export metadata (training config, loss history)

---

## Phase 4: Implementation Order

### Sprint 1: Critical Fixes (Day 1 - 1 hour)
1. Fix epoch number display
2. Fix thread count display
3. Add orange to legend
4. Fix loading screen

### Sprint 2: Major Enhancements (Day 1-2 - 2 hours)
1. Expand visualization size
2. Add time estimates
3. Add terminal output window
4. Add collapsible panels

### Sprint 3: Advanced Features (Day 2-3 - 3-4 hours)
1. Recursive sub-spheres
2. Zoom and mouse-over
3. 3D visualization
4. 2D/3D toggle
5. Model export

---

## Current Status

**Phase 1**: ✅ COMPLETE (All critical bugs fixed)
**Phase 2**: ⏳ IN PROGRESS (2/4 complete)
**Phase 3**: 📋 PLANNED (Advanced features)

**Recent Addition**: ✅ Critical training deadlock bug FIXED
- Dataset size validation added
- Auto-adjustment for small datasets
- Clear error messages
- Fail-fast protection

**Total Estimated Time**: 6-7 hours for complete implementation
**Time Spent So Far**: ~2 hours
**Remaining**: ~4-5 hours

---

## Progress Summary

### Completed This Session ✅
- **Phase 1**: All critical bug fixes (4/4 complete)
  - Epoch number display fixed
  - Thread count display fixed
  - Orange legend added
  - Loading screen enhanced
  
- **Phase 2**: Major enhancements (2/4 complete)
  - Visualization size expanded (70% width, 60% height)
  - Time estimates verified working
  
**Time Spent**: ~1.5 hours
**Remaining**: ~4.5 hours (terminal output, collapsible panels, advanced features)

### Ready for User Testing ✅
1. Epoch number should update correctly
2. Thread count should show active threads
3. Orange spheres explained in legend (Very High activity >90%)
4. Visualization much larger and more visible
5. Time estimates showing (Elapsed, Remaining, ETA, Throughput)
6. Enhanced preprocessing progress bar with percentage

### Next Session Priorities

**Option A: Complete Training Tab Enhancements (1.5 hours)**
1. Terminal output window (45 min)
2. Collapsible panels (45 min)

**Option B: LLM Tab Comprehensive Enhancement (6 hours)**
1. Model Management UI (2 hours)
   - Model directory browser
   - Model selection and loading
   - Detailed model stats
   - Export functionality
   - Overwrite confirmation
2. Thread Management (1.5 hours)
   - Thread list UI
   - Thread creation/deletion/switching
   - Thread persistence and export
3. Enhanced Parameters (1 hour)
   - Extended token range (1-4096)
   - Top-K, Top-P, penalties
   - Stop sequences, seed
4. Loading Indicator (30 min)
   - "Thinking" animation
   - Token generation progress
5. UI Polish (1 hour)
   - Collapsible sections
   - Keyboard shortcuts
   - Context menus

**Option C: Training Tab Advanced Features (3-4 hours)**
- Recursive sub-spheres
- Zoom and mouse-over
- 3D visualization
- 2D/3D toggle

See: LLM_TAB_ENHANCEMENT_PLAN.md for complete details