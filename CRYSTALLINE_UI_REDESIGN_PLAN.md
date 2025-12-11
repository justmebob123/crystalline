# Crystalline UI Library - Comprehensive Redesign Plan

## Objective
Create a TRUE unified UI library that:
1. Requires minimal per-tab customization
2. Handles all rendering, events, and layout automatically
3. Uses consistent coordinate systems and conventions
4. Provides type-safe, clear APIs
5. Implements proper parent-child relationships
6. Manages bounds, clipping, and scrolling automatically

## Current Problems (Summary)

1. **Mixed Coordinate Systems** - CENTER vs TOP-LEFT confusion
2. **No Layout Management** - Manual positioning everywhere
3. **Fragmented Event Handling** - Each tab routes events manually
4. **No Bounds Management** - Elements render outside containers
5. **Parameter Ambiguity** - Unclear function parameters
6. **No Parent-Child Relationships** - No relative positioning
7. **Inconsistent Size Handling** - Size constants ignored

## Design Principles

### Principle 1: Single Coordinate System
**Decision: Use CENTER coordinates for ALL elements**
- Consistent with sacred geometry (center-based)
- Easier for circular/geometric layouts
- All functions convert internally if needed

### Principle 2: Automatic Layout
**Container-based layout system**
- Containers manage child positioning
- Support vertical, horizontal, grid layouts
- Automatic spacing and alignment

### Principle 3: Unified Event System
**Event bubbling and capture**
- Events route automatically to correct element
- Parent containers handle child events
- Focus management built-in

### Principle 4: Proper Hierarchy
**Parent-child relationships**
- Children positioned relative to parents
- Parents clip children to bounds
- Automatic scroll management

### Principle 5: Type Safety
**Clear, explicit APIs**
- No ambiguous parameters
- Enums for sizes, styles, alignments
- Compile-time type checking

## Architecture

### Layer 1: Core Types (geometry.h/c)
```c
// Point (always CENTER coordinates)
typedef struct {
    float x, y;
} CrystallinePoint;

// Bounds (CENTER + dimensions)
typedef struct {
    CrystallinePoint center;
    float width, height;
} CrystallineBounds;

// Padding (explicit)
typedef struct {
    float top, right, bottom, left;
} CrystallinePadding;
```

### Layer 2: Base Element (element.h/c)
```c
typedef struct CrystallineElement CrystallineElement;

struct CrystallineElement {
    // Identity
    const char* id;
    CrystallineElementType type;
    
    // Hierarchy
    CrystallineElement* parent;
    CrystallineElement** children;
    int child_count;
    
    // Geometry (always CENTER coordinates)
    CrystallineBounds bounds;
    CrystallinePadding padding;
    
    // State
    bool visible;
    bool enabled;
    bool focused;
    CrystallineElementState state;  // NORMAL, HOVER, ACTIVE, DISABLED
    
    // Rendering
    void (*render)(CrystallineElement* self, SDL_Renderer* renderer);
    
    // Events
    bool (*handle_event)(CrystallineElement* self, SDL_Event* event);
    
    // Layout
    void (*compute_layout)(CrystallineElement* self);
    
    // User data
    void* user_data;
};
```

### Layer 3: Containers (container.h/c)
```c
typedef enum {
    CRYSTALLINE_LAYOUT_VERTICAL,
    CRYSTALLINE_LAYOUT_HORIZONTAL,
    CRYSTALLINE_LAYOUT_GRID,
    CRYSTALLINE_LAYOUT_ABSOLUTE
} CrystallineLayoutType;

typedef struct {
    CrystallineElement base;
    
    // Layout
    CrystallineLayoutType layout_type;
    float spacing;
    CrystallineAlignment alignment;
    
    // Scrolling
    bool scrollable;
    float scroll_offset;
    float scroll_max;
    
    // Clipping
    bool clip_children;
} CrystallineContainer;

// API
CrystallineContainer* crystalline_container_create(
    CrystallineBounds bounds,
    CrystallineLayoutType layout,
    float spacing
);

void crystalline_container_add_child(
    CrystallineContainer* container,
    CrystallineElement* child
);

void crystalline_container_compute_layout(CrystallineContainer* container);
void crystalline_container_render(CrystallineContainer* container, SDL_Renderer* renderer);
bool crystalline_container_handle_event(CrystallineContainer* container, SDL_Event* event);
```

### Layer 4: Widgets (button.h/c, list.h/c, etc.)
```c
// Button
typedef enum {
    CRYSTALLINE_BUTTON_SIZE_SMALL,
    CRYSTALLINE_BUTTON_SIZE_MEDIUM,
    CRYSTALLINE_BUTTON_SIZE_LARGE
} CrystallineButtonSize;

CrystallineElement* crystalline_button_create(
    const char* label,
    CrystallineButtonSize size,
    void (*on_click)(void* user_data),
    void* user_data
);

// List
CrystallineElement* crystalline_list_create(
    CrystallineBounds bounds,
    float item_height,
    bool show_checkboxes
);

void crystalline_list_set_items(
    CrystallineElement* list,
    const char** items,
    int count
);

// Slider
CrystallineElement* crystalline_slider_create(
    float min_value,
    float max_value,
    float initial_value,
    void (*on_change)(float value, void* user_data),
    void* user_data
);
```

### Layer 5: Tab System (tab.h/c)
```c
typedef struct {
    CrystallineContainer* root;
    const char* name;
    void (*init)(AppState* state);
    void (*update)(AppState* state);
} CrystallineTab;

CrystallineTab* crystalline_tab_create(const char* name);
void crystalline_tab_render(CrystallineTab* tab, SDL_Renderer* renderer);
bool crystalline_tab_handle_event(CrystallineTab* tab, SDL_Event* event);
```

## Implementation Plan

### Phase 1: Core Foundation (2-3 hours)
1. Create new files:
   - `app/ui/crystalline/v2/core.h/c` - Core types
   - `app/ui/crystalline/v2/element.h/c` - Base element
   - `app/ui/crystalline/v2/container.h/c` - Container system

2. Implement:
   - CrystallinePoint, CrystallineBounds, CrystallinePadding
   - CrystallineElement base structure
   - CrystallineContainer with layout management
   - Event routing system

3. Test with simple example

### Phase 2: Widget Library (3-4 hours)
1. Create widget files:
   - `app/ui/crystalline/v2/button.h/c`
   - `app/ui/crystalline/v2/list.h/c`
   - `app/ui/crystalline/v2/slider.h/c`
   - `app/ui/crystalline/v2/dropdown.h/c`
   - `app/ui/crystalline/v2/panel.h/c`

2. Implement each widget:
   - Proper rendering with bounds respect
   - Event handling
   - State management
   - Size enforcement

3. Test each widget individually

### Phase 3: Layout System (2-3 hours)
1. Implement layout algorithms:
   - Vertical layout (stack top-to-bottom)
   - Horizontal layout (stack left-to-right)
   - Grid layout (rows and columns)
   - Absolute layout (manual positioning)

2. Implement automatic spacing
3. Implement alignment options
4. Test with complex layouts

### Phase 4: Tab Migration (4-5 hours)
1. Create migration guide
2. Migrate Training Tab (proof of concept)
3. Compare code size and complexity
4. Migrate remaining tabs
5. Remove old UI code

### Phase 5: Polish and Testing (2-3 hours)
1. Add animations
2. Add focus management
3. Add keyboard navigation
4. Comprehensive testing
5. Documentation

## Example: Training Tab with New System

### Old Way (Current - ~200 lines of positioning code)
```c
void init_training_tab(AppState* state) {
    // Manual calculation of every position
    int slider_x = RENDER_OFFSET_X + viz_width + 20;
    int slider_w = control_width - 60;
    float slider_center_x = slider_x + slider_w / 2.0f;
    
    // Create each element with manual positioning
    g_training_ui.slider_batch = crystalline_slider_create(...);
    g_training_ui.slider_sequence = crystalline_slider_create(...);
    // ... repeat for every element
    
    // Manual event routing in 3 separate functions
    void handle_training_tab_mouse_down(...) { /* route to each element */ }
    void handle_training_tab_mouse_up(...) { /* route to each element */ }
    void handle_training_tab_mouse_motion(...) { /* route to each element */ }
}
```

### New Way (Proposed - ~50 lines)
```c
void init_training_tab(AppState* state) {
    // Create root container
    CrystallineTab* tab = crystalline_tab_create("Training");
    
    // Create control panel with vertical layout
    CrystallineContainer* control_panel = crystalline_container_create(
        crystalline_bounds(1332, 450, 475, 700),
        CRYSTALLINE_LAYOUT_VERTICAL,
        10.0f  // spacing
    );
    
    // Add widgets - they position automatically
    crystalline_container_add_child(control_panel,
        crystalline_dropdown_create("Model", model_names, model_count, on_model_selected, state));
    
    crystalline_container_add_child(control_panel,
        crystalline_slider_create(1, 128, 32, on_batch_size_changed, state));
    
    crystalline_container_add_child(control_panel,
        crystalline_slider_create(1, 512, 128, on_sequence_length_changed, state));
    
    // Button row (horizontal layout)
    CrystallineContainer* button_row = crystalline_container_create(
        crystalline_bounds(0, 0, 200, 50),  // Width auto-calculated
        CRYSTALLINE_LAYOUT_HORIZONTAL,
        10.0f
    );
    crystalline_container_add_child(button_row,
        crystalline_button_create("SCAN", CRYSTALLINE_BUTTON_SIZE_SMALL, on_scan_clicked, state));
    crystalline_container_add_child(button_row,
        crystalline_button_create("SELECT", CRYSTALLINE_BUTTON_SIZE_SMALL, on_select_all_clicked, state));
    
    crystalline_container_add_child(control_panel, button_row);
    
    // File list
    CrystallineElement* file_list = crystalline_list_create(
        crystalline_bounds(0, 0, 475, 200),  // Position auto-calculated
        25.0f,  // item height
        true    // show checkboxes
    );
    crystalline_list_set_check_callback(file_list, on_file_checkbox_changed, state);
    crystalline_container_add_child(control_panel, file_list);
    
    // Compute layout - positions everything automatically
    crystalline_container_compute_layout(control_panel);
    
    // Add to tab
    crystalline_tab_add_container(tab, control_panel);
    
    // Events handled automatically - no manual routing needed!
}

void draw_training_tab(SDL_Renderer* renderer, AppState* state) {
    crystalline_tab_render(state->training_tab, renderer);
}

void handle_training_tab_event(AppState* state, SDL_Event* event) {
    crystalline_tab_handle_event(state->training_tab, event);
}
```

## Benefits

### Code Reduction
- Training Tab: ~200 lines → ~50 lines (75% reduction)
- No manual positioning calculations
- No manual event routing
- No coordinate system confusion

### Maintainability
- Add new element: 1 line of code
- Change layout: Change layout type
- Reorder elements: Reorder add_child calls
- No risk of overlap or positioning errors

### Consistency
- All tabs use same system
- Same coordinate conventions
- Same event handling
- Same layout logic

### Type Safety
- Clear enums for sizes, layouts, alignments
- No ambiguous parameters
- Compile-time error checking

## Migration Strategy

### Step 1: Build New System Alongside Old
- Create `app/ui/crystalline/v2/` directory
- Implement new system
- Keep old system working

### Step 2: Migrate One Tab (Training)
- Rewrite Training Tab with new system
- Compare functionality
- Verify all features work
- Measure code reduction

### Step 3: Migrate Remaining Tabs
- Use Training Tab as template
- Migrate one tab at a time
- Test each migration

### Step 4: Remove Old System
- Delete old UI code
- Update documentation
- Clean up

## Timeline

- **Phase 1 (Core):** 2-3 hours
- **Phase 2 (Widgets):** 3-4 hours
- **Phase 3 (Layout):** 2-3 hours
- **Phase 4 (Migration):** 4-5 hours
- **Phase 5 (Polish):** 2-3 hours

**Total: 13-18 hours of focused work**

## Success Criteria

1. ✅ Training Tab code reduced by >70%
2. ✅ Zero manual positioning calculations
3. ✅ Zero manual event routing
4. ✅ All features working correctly
5. ✅ Zero errors, zero warnings
6. ✅ Consistent behavior across all tabs
7. ✅ Easy to add new UI elements

## Next Steps

1. Get approval for redesign approach
2. Create v2 directory structure
3. Implement Phase 1 (Core Foundation)
4. Test and iterate
5. Proceed with remaining phases

This is a comprehensive redesign that will solve all the systematic problems and create a TRUE unified UI library.