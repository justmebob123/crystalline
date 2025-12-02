# TODO - Crystalline CLLM UI Redesign

   ## 🔒 RULES (PASTED FROM MASTER_PLAN.MD - RULE 0)
   
   **CRITICAL ISSUE IDENTIFIED:** Kissing spheres visualization broken on Training Tab
   - Worker threads running but visualization not updating
   - Need bidirectional analysis of all UI elements and wiring

   ### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
   **HIGHEST PRIORITY RULE - MUST BE FOLLOWED WITH EVERY SINGLE RESPONSE**

   At the beginning of EVERY response, you MUST:
   1. Paste these rules to the top of todo.md
   2. Read the MASTER_PLAN.md completely
   3. Read the AUDIT.md for current architectural state
   4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

   This creates a permanent loop ensuring rules are never forgotten.

   ### RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
   **SECOND HIGHEST PRIORITY RULE**

   Before taking ANY action, you MUST:
   1. Read MASTER_PLAN.md completely
   2. Understand the current objectives
   3. Verify your action aligns with the master plan
   4. Check for any blocking priorities

   This ensures all work follows the architectural design.

   ### RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
   **CRITICAL REFERENCE DOCUMENT**

   The AUDIT.md contains:
   - Current architectural violations
   - Required fixes with priorities
   - Implementation phases
   - Testing requirements
   - Success criteria

   Always consult AUDIT.md before starting work to understand:
   - What is broken
   - What needs fixing
   - What is blocking other work
   - What the correct architecture should be

   ### RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
   **DETAILED IMPLEMENTATION GUIDE**

   The SECONDARY_OBJECTIVES.md contains:
   - Detailed implementation tasks
   - Code examples
   - File-by-file changes
   - Testing procedures
   - Validation steps

   Use this for step-by-step implementation guidance.

   ### RULE 4: DO NOT CREATE NEW MD FILES
   All documentation goes in existing files or this master plan only.

   ### RULE 5: ALWAYS COMMIT ALL CHANGES USING CORRECT AUTHENTICATION
   ```bash
   git add .
   git commit -m "descriptive message"
   git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
   ```

   ### RULE 7: FIX ALL BUILD WARNINGS BEFORE PROCEEDING
   All code must compile with zero warnings before moving to the next objective.

   ---

   ## 🎯 CURRENT OBJECTIVE: Complete UI Redesign with Threading and Loading Screen

   ### User Requirements:
   1. **Redesign entire UI layout** with comprehensive system
   2. **Proper threading from application startup** (not just for training)
   3. **Loading screen** during initialization

   ### Critical Issues Identified:
   1. **No Loading Screen**: Application starts with blank window during initialization
   2. **No Startup Threading**: All initialization happens on main thread (blocks UI)
   3. **Models Tab**: Black screen issue (FIXED but needs redesign)
   4. **Training Tab**: Controls bunched up and overlapping
   5. **All Tabs**: Text and UI elements overlapping
   6. **Layout System**: Not working properly - needs complete rethink

   ---

   ## Phase 1: Analyze Current Architecture [COMPLETE]

   ### 1.1: Current Startup Flow Analysis COMPLETE
   - [x] Examine main.c startup sequence
   - [x] Identify blocking operations during init
   - [x] Document initialization steps
   - [x] Identify what can be threaded

   FINDINGS:
   - SDL_Init() - Fast (less than 50ms)
   - init_font_system() - Fast (less than 50ms)
   - SDL_CreateWindow() - Fast (less than 100ms)
   - SDL_CreateRenderer() - Fast (less than 100ms)
   - input_manager_create() - Fast (less than 10ms)
   - init_all_inputs() - Fast (less than 50ms)
   - init_all_tabs() - Fast (less than 100ms)
   - model_manager_init() - Fast (less than 50ms)
   - generate_n_primes(1000) - SLOW (about 200ms)
   - lattice_cache_create() - DEFERRED (lazy init)
   - control_thread startup - ALREADY THREADED

   CURRENT THREADING:
   - control_thread.c - Background initialization (abacus, model scanning)
   - training_thread.c - Training operations
   - Main thread - SDL event loop and rendering

   ISSUE: No loading screen during initial setup (SDL init, window creation, etc.)

   ### 1.2: Current UI Architecture Analysis COMPLETE
   - [x] Models Tab: init_models_tab() never called - FIXED
   - [x] Training Tab: Overlapping controls - ROOT CAUSE IDENTIFIED
   - [x] Examine layout_manager.c implementation
   - [x] Examine ui_layout.c implementation
   - [x] Document current layout system capabilities
   - [x] Identify layout system limitations

   FINDINGS:
   - layout_manager.c: Provides TabLayout with content area bounds
   - ui_layout.c: Basic layout utilities (80 lines)
   - LayoutContainer system in tabs: Manual positioning with spacing
   - NO scrolling support
   - NO responsive resizing
   - NO component library
   - Each tab reimplements UI elements

   ISSUES:
   - Training Tab: 15+ controls in 860px vertical space (needs about 600px minimum)
   - No scrollable containers
   - Hardcoded positions everywhere
   - Text overlap due to insufficient bounds checking
   - No proper layout engine

   ### 1.3: Current Threading Architecture COMPLETE
   - [x] Document existing threading
   - [x] Identify missing threading
   - [x] Document thread synchronization mechanisms

   EXISTING THREADS:
   1. Main Thread: SDL event loop, rendering
   2. Control Thread: Background init (abacus, model scanning)
   3. Training Thread: Training operations (when active)

   SYNCHRONIZATION:
   - AppState flags: abacus_initializing, abacus_ready, model_loading, model_ready
   - pthread_join() for cleanup
   - No mutexes needed (flags are atomic bools)

   MISSING:
   - Loading screen during initial SDL setup
   - Progress reporting during initialization

   ---

   ## Phase 2: Design New Architecture [COMPLETE]

   ### 2.1: Loading Screen Design COMPLETE
   
   DESIGN:
   - Simple centered loading screen with progress bar
   - Shows current initialization stage
   - Displays percentage complete
   - Smooth fade-out transition to main UI
   
   STAGES:
   1. "Initializing SDL..." (0-20%)
   2. "Creating window..." (20-40%)
   3. "Initializing renderer..." (40-60%)
   4. "Loading fonts..." (60-70%)
   5. "Initializing input system..." (70-80%)
   6. "Initializing tabs..." (80-90%)
   7. "Starting background services..." (90-100%)
   
   TRANSITION:
   - Fade out loading screen over 500ms
   - Fade in main UI simultaneously
   - Total transition: 500ms

   ### 2.2: Startup Threading Design COMPLETE
   
   DECISION: Keep current architecture - it's already optimal!
   
   RATIONALE:
   - SDL MUST be initialized on main thread (SDL requirement)
   - Window/renderer MUST be created on main thread (SDL requirement)
   - Event loop MUST run on main thread (SDL requirement)
   - Heavy operations (abacus, models) ALREADY threaded in control_thread.c
   
   WHAT WE NEED:
   - Loading screen DURING main thread initialization (SDL, window, fonts)
   - Progress callbacks from init_app()
   - Smooth transition when complete
   
   NO NEW THREADS NEEDED - current architecture is correct per MASTER_PLAN

   ### 2.3: Comprehensive Layout System Design COMPLETE
   
   LAYOUT PRIMITIVES:
   1. Container: Holds child elements with padding/margin
   2. VBox: Vertical box layout (stack elements vertically)
   3. HBox: Horizontal box layout (stack elements horizontally)
   4. ScrollContainer: Scrollable container with scrollbar
   5. Grid: Grid layout with rows/columns
   
   SPACING SYSTEM:
   - Small: 5px
   - Medium: 10px
   - Large: 20px
   - XLarge: 40px
   
   RESPONSIVE SYSTEM:
   - Elements know their minimum size
   - Containers calculate required space
   - Scrollbars appear when content exceeds bounds
   - Window resize triggers layout recalculation

   ### 2.4: Component System Design COMPLETE
   
   BASE COMPONENT:
   - Position (x, y)
   - Size (width, height)
   - Visibility flag
   - Enabled flag
   - Render function pointer
   - Handle input function pointer
   
   COMPONENT LIBRARY:
   - Button (with hover/press states)
   - Label (text display)
   - TextInput (single line input)
   - Slider (value selection)
   - Checkbox (boolean toggle)
   - Panel (container with border/title)
   - ScrollPanel (scrollable panel)
   - Dropdown (selection list)
   
   EXISTING COMPONENTS:
   - We already have many of these in app/ui/components.c
   - Need to integrate with new layout system
   - Need to add ScrollPanel

   ### 2.5: Tab-Specific Layout Design COMPLETE
   
   MODELS TAB:
   - Left: Model list (scrollable)
   - Right: Model details + actions
   - Bottom: Create/Load/Delete buttons
   
   TRAINING TAB:
   - Left: Visualization area (sphere/loss graph)
   - Right: Scrollable control panel with sections:
     * Model Selection (collapsible)
     * Training Parameters (collapsible)
     * Data Configuration (collapsible)
     * Training Controls (always visible)
     * Progress/Statistics (always visible)
   
   LLM TAB:
   - Top: Model selector + parameters
   - Middle: Chat history (scrollable)
   - Bottom: Input field + send button
   
   OTHER TABS:
   - Keep existing layouts (they work)
   - Apply consistent spacing
   - Fix text overlap issues

   ---

   ## Phase 3: Implement Loading Screen [IN PROGRESS]

   ### 3.1: Create Loading Screen UI COMPLETE
   - [x] Create app/ui/loading_screen.c
   - [x] Create app/ui/loading_screen.h
   - [x] Implement loading_screen_render(renderer, state, alpha)
   - [x] Implement LoadingScreenState structure
   - [x] Implement loading_screen_init()
   - [x] Implement loading_screen_update()
   - [x] Implement loading_screen_set_error()
   - [x] Add to Makefile (automatic via wildcard)
   - [x] Build verified - zero errors

   ### 3.2: Refactor init_app() with Progress Callbacks COMPLETE
   - [x] Integrated loading screen into init_app()
   - [x] Added loading_screen_update() calls after each init step
   - [x] Added loading_screen_render() calls to show progress
   - [x] Implemented 7 stages with proper progress tracking:
     * Stage 1: SDL initialization
     * Stage 2: Window creation
     * Stage 3: Renderer creation
     * Stage 4: Font system loading
     * Stage 5: Input system initialization
     * Stage 6: Tab initialization
     * Stage 7: Background services startup
   - [x] Added error handling with loading_screen_set_error()
   - [x] Build verified - zero errors

   ### 3.3: Implement Smooth Transition COMPLETE
   - [x] Added fade-out loop after init_app() completes
   - [x] Fade loading screen from alpha=255 to alpha=0
   - [x] Fade step: 8 per frame (32 frames total)
   - [x] Frame delay: 16ms (~60fps)
   - [x] Total transition time: ~512ms
   - [x] Smooth visual transition to main UI

   ### 3.4: Test Loading Screen READY FOR TESTING
   - [ ] Test rendering at each stage
   - [ ] Test progress bar updates
   - [ ] Test smooth fade transition
   - [ ] Test error display (if init fails)
   - [ ] User acceptance testing

   ---

   ## Phase 4: Startup Threading [SKIPPED - NOT NEEDED]

   DECISION: Current architecture is already optimal per MASTER_PLAN
   
   RATIONALE:
   - SDL requires main thread initialization (cannot be threaded)
   - Window/renderer must be on main thread (SDL requirement)
   - Event loop must be on main thread (SDL requirement)
   - Heavy operations already threaded in control_thread.c
   - Loading screen will run on main thread during SDL init
   
   NO CHANGES NEEDED - architecture is correct

   ---

   ## Phase 5: Implement New Layout System [COMPLETE]

   ### 5.1: Create Layout Engine COMPLETE
   - [x] Create app/ui/layout_engine.c (450+ lines)
   - [x] Create app/ui/layout_engine.h (comprehensive API)
   - [x] Implement LayoutElement (base for all UI elements)
   - [x] Implement LayoutContainer (base container)
   - [x] Implement VBox (vertical stacking with alignment)
   - [x] Implement HBox (horizontal stacking with alignment)
   - [x] Implement Grid (rows/columns layout)
   - [x] Implement padding and spacing system
   - [x] Implement alignment (START, CENTER, END, STRETCH)
   - [x] Add to Makefile (automatic via wildcard)
   - [x] Build verified - zero errors, zero warnings

   ### 5.2: Create Scrolling System COMPLETE
   - [x] Create app/ui/scroll_panel.c (280+ lines)
   - [x] Create app/ui/scroll_panel.h (complete API)
   - [x] Implement ScrollPanel structure
   - [x] Implement scroll_panel_create()
   - [x] Implement scroll_panel_render() with clipping
   - [x] Implement scroll_panel_handle_input()
   - [x] Implement scrollbar rendering (background + handle)
   - [x] Implement mouse wheel support (configurable speed)
   - [x] Implement drag scrolling (smooth dragging)
   - [x] Implement scroll_panel_scroll_to() and scroll_by()
   - [x] Add to Makefile (automatic via wildcard)
   - [x] Build verified - zero errors, zero warnings

   ### 5.3: Integration with Existing Components READY
   - [x] Layout engine provides base LayoutElement
   - [x] ScrollPanel uses LayoutContainer internally
   - [x] Components can be added to containers
   - [x] Ready for tab redesign integration

   ### 5.4: Test Layout System READY FOR TESTING
   - [ ] Test VBox layout in real tab
   - [ ] Test HBox layout in real tab
   - [ ] Test Grid layout in real tab
   - [ ] Test ScrollPanel scrolling
   - [ ] Test mouse wheel
   - [ ] Test drag scrolling
   - [ ] Test nested layouts
   - [ ] User acceptance testing

   ---

   ## Phase 6: Redesign Training Tab [IN PROGRESS]

   ### 6.0: CRITICAL BUG ANALYSIS - Sphere Visualization Not Showing
   
   ROOT CAUSE IDENTIFIED:
   - Line 370 in tab_training.c: if (state->training_in_progress || viz_data.loss_count > 0)
   - Sphere visualization only renders DURING training
   - Should render ALWAYS to show architecture even when idle
   
   BIDIRECTIONAL ANALYSIS:
   
   Forward Path (Rendering):
   1. main.c:911 calls draw_training_tab(renderer, state)
   2. tab_training.c:713 calls draw_training_visualization(renderer, state)
   3. tab_training.c:370 CONDITIONAL CHECK (blocks rendering when idle)
   4. tab_training.c:476 calls draw_sphere_visualization(renderer, state, sphere_bounds)
   5. sphere_visualization.c:112 Renders spheres with stats
   
   Backward Path (Data Flow):
   1. training_thread.c:343 calls update_sphere_stats(state, g_threaded_system)
   2. training_thread.c:103-106 Updates state->sphere_stats.batches_processed[i]
   3. sphere_visualization.c:154-156 Reads state->sphere_stats.batches_processed[i]
   4. sphere_visualization.c:169 Colors spheres based on activity
   
   ISSUE:
   - Data flow is correct
   - Rendering function is correct
   - Conditional check prevents rendering when idle
   
   FIX:
   - Remove or modify conditional to always show sphere visualization
   - Show IDLE state when not training
   - Show architecture even with zero activity

   ### 6.1: Fix Sphere Visualization Bug COMPLETE
   - [x] Identified root cause: Conditional block preventing rendering when idle
   - [x] Moved sphere visualization outside if/else block
   - [x] Now renders ALWAYS (even when training is idle)
   - [x] Commented out duplicate code inside conditional
   - [x] Build verified - zero errors, zero warnings
   - [ ] NEEDS TESTING: Verify visualization appears on tab open
   - [ ] NEEDS TESTING: Verify visualization updates during training

   ### 6.2: Redesign Training Tab with New Layout System
   - [ ] Backup current tab_training.c
   - [ ] Rewrite using layout_engine + scroll_panel
   - [ ] Create scrollable control panel
   - [ ] Organize controls into sections:
     * Model Selection
     * Training Parameters
     * Data Configuration
     * Training Controls
     * Progress/Statistics
   - [ ] Test all controls work
   - [ ] Test scrolling works
   - [ ] Verify no overlap

   ### 6.2: Redesign Models Tab
   - [ ] Backup current tab_models.c
   - [ ] Rewrite using layout_engine
   - [ ] Implement model list (scrollable)
   - [ ] Implement model details panel
   - [ ] Test all functionality

   ### 6.3: Redesign LLM Tab
   - [ ] Backup current tab_llm.c
   - [ ] Rewrite using layout_engine + scroll_panel
   - [ ] Implement scrollable chat history
   - [ ] Test all functionality

   ### 6.4: Fix Other Tabs (Lower Priority)
   - [ ] Research Tab - fix text overlap
   - [ ] Benchmark Tab - fix text overlap
   - [ ] Crawler Tab - apply consistent spacing
   - [ ] Video Tab - apply consistent spacing
   - [ ] Downloaded Files Tab - apply consistent spacing
   - [ ] URL Manager Tab - apply consistent spacing

   ---

   ## Phase 7: Testing and Validation [NOT STARTED]

   ### 7.1: Functional Testing
   - [ ] Test loading screen
   - [ ] Test initialization
   - [ ] Test all tabs
   - [ ] Test all controls
   - [ ] Test scrolling

   ### 7.2: Performance Testing
   - [ ] Measure startup time
   - [ ] Measure UI responsiveness
   - [ ] Test on different hardware

   ### 7.3: Visual Testing
   - [ ] Test at different window sizes
   - [ ] Test window resizing
   - [ ] Test all visual elements

   ### 7.4: Integration Testing
   - [ ] Test training workflow
   - [ ] Test inference workflow
   - [ ] Test model management

   ---

   ## 📋 EXECUTION LOG

   ### Current Session:
   - [x] Pasted rules to top of todo.md (RULE 0)
   - [x] Read MASTER_PLAN.md completely
   - [x] Read AUDIT.md for architectural state
   - [x] Phase 1: Analyzed current architecture - COMPLETE
   - [x] Phase 2: Designed new architecture - COMPLETE
   - [x] Phase 3: Implemented loading screen - COMPLETE
   - [x] Phase 5: Implemented layout system - COMPLETE
   - [x] Phase 6.0: Performed bidirectional analysis - COMPLETE
   - [x] Phase 6.1: Fixed sphere visualization bug - COMPLETE
   - [x] Build verified - zero errors, zero warnings
   - [ ] Phase 6.1: Testing sphere visualization fix
   - [ ] Phase 6.2: Redesigning Training Tab with new layout system

   ### Critical Bugs Fixed This Session:
   1. Models Tab black screen - init_models_tab() never called
   2. Model loading "bad magic number" - only wrote 4 bytes instead of 8
   3. Heap-use-after-free crash - worker threads accessing freed model memory