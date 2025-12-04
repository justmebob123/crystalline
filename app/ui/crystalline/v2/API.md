# Crystalline UI V2 - API Documentation

## Overview

Crystalline UI V2 is a comprehensive UI library with:
- **Single coordinate system** (CENTER for all elements)
- **Automatic layout management** (vertical, horizontal, grid)
- **Unified event handling** (automatic routing and bubbling)
- **Parent-child hierarchy** (proper relationships)
- **Type-safe APIs** (clear enums, no ambiguity)

## Quick Start

### Basic Example
```c
#include "crystalline/v2/container.h"
#include "crystalline/v2/button.h"
#include "crystalline/v2/slider.h"

// Create container with vertical layout
CrystallineContainer* panel = crystalline_container_create(
    crystalline_bounds(600, 400, 400, 600),  // center_x, center_y, width, height
    CRYSTALLINE_LAYOUT_VERTICAL,              // stack top-to-bottom
    10.0f                                     // 10px spacing
);

// Add widgets - they position automatically!
CrystallineButton* btn = crystalline_button_create(
    "Click Me", CRYSTALLINE_SIZE_MEDIUM, true, on_click, NULL
);
crystalline_button_set_font(btn, font);
crystalline_container_add_child(panel, (CrystallineElement*)btn);

CrystallineSlider* slider = crystalline_slider_create(
    crystalline_bounds(0, 0, 360, 30),
    0.0f, 100.0f, 50.0f,
    on_slider_change, NULL
);
crystalline_container_add_child(panel, (CrystallineElement*)slider);

// Compute layout - positions everything!
crystalline_container_compute_layout(panel);

// Render
crystalline_container_render(panel, renderer);

// Handle events - automatic routing!
crystalline_container_handle_event(panel, &event);
```

## Core Types

### CrystallinePoint
```c
typedef struct {
    float x, y;
} CrystallinePoint;

// Create point
CrystallinePoint p = crystalline_point(100.0f, 200.0f);
```

### CrystallineBounds
```c
typedef struct {
    CrystallinePoint center;
    float width, height;
} CrystallineBounds;

// Create bounds (CENTER coordinates)
CrystallineBounds b = crystalline_bounds(
    600.0f,  // center_x
    400.0f,  // center_y
    400.0f,  // width
    600.0f   // height
);
```

### CrystallinePadding
```c
typedef struct {
    float top, right, bottom, left;
} CrystallinePadding;

// Uniform padding
CrystallinePadding p = crystalline_padding_uniform(10.0f);

// Custom padding
CrystallinePadding p = crystalline_padding(5.0f, 10.0f, 5.0f, 10.0f);
```

### CrystallineColor
```c
typedef struct {
    uint8_t r, g, b, a;
} CrystallineColor;

// Create color
CrystallineColor c = crystalline_color(100, 150, 200, 255);
```

## Enums

### CrystallineSize
```c
typedef enum {
    CRYSTALLINE_SIZE_TINY,     // 15px radius / 30px width
    CRYSTALLINE_SIZE_SMALL,    // 20px radius / 40px width
    CRYSTALLINE_SIZE_MEDIUM,   // 25px radius / 50px width
    CRYSTALLINE_SIZE_LARGE     // 40px radius / 80px width
} CrystallineSize;
```

### CrystallineLayoutType
```c
typedef enum {
    CRYSTALLINE_LAYOUT_VERTICAL,    // Stack top-to-bottom
    CRYSTALLINE_LAYOUT_HORIZONTAL,  // Stack left-to-right
    CRYSTALLINE_LAYOUT_GRID,        // Grid layout
    CRYSTALLINE_LAYOUT_ABSOLUTE     // Manual positioning
} CrystallineLayoutType;
```

### CrystallineAlignment
```c
typedef enum {
    CRYSTALLINE_ALIGN_START,   // Top/Left
    CRYSTALLINE_ALIGN_CENTER,  // Center
    CRYSTALLINE_ALIGN_END      // Bottom/Right
} CrystallineAlignment;
```

## Widgets

### Text Input

**Create:**
```c
CrystallineTextInput* input = crystalline_textinput_create(
    crystalline_bounds(400, 300, 300, 40),  // bounds
    "Enter text here...",                    // placeholder
    256,                                     // max_length (0 = unlimited)
    on_text_change,                          // on_change callback
    on_text_submit,                          // on_submit callback
    user_data                                // user data
);
```

**Configure:**
```c
crystalline_textinput_set_text(input, "Initial text");
crystalline_textinput_set_font(input, font);
crystalline_textinput_set_focus(input, true);
crystalline_textinput_set_colors(input, normal, focused, text, placeholder);
```

**Callbacks:**
```c
void on_text_change(const char* text, void* user_data) {
    printf("Text changed: %s\n", text);
}

void on_text_submit(const char* text, void* user_data) {
    printf("Text submitted: %s\n", text);
}
```

**Features:**
- Single-line text input
- Cursor with blinking animation
- Placeholder text when empty
- Focus management (click to focus, ESC to unfocus)
- Text editing: Backspace, Delete, Home, End, Arrow keys
- Submit on Enter key
- Visual feedback (border color changes when focused)

---

### Text Area

**Create:**
```c
CrystallineTextArea* area = crystalline_textarea_create(
    crystalline_bounds(400, 300, 600, 400),  // bounds
    true                                      // auto_scroll
);
```

**Add Messages:**
```c
crystalline_textarea_add_message(
    area,
    CRYSTALLINE_MESSAGE_USER,
    "Hello, how are you?",
    "12:34 PM"
);

crystalline_textarea_add_message(
    area,
    CRYSTALLINE_MESSAGE_ASSISTANT,
    "I'm doing well, thank you!",
    "12:35 PM"
);
```

**Configure:**
```c
crystalline_textarea_set_font(area, font);
crystalline_textarea_set_colors(area, bg, user_color, assistant_color, system_color);
crystalline_textarea_set_auto_scroll(area, true);
crystalline_textarea_scroll_to_bottom(area);
```

**Features:**
- Multi-line message display
- Three message types: USER, ASSISTANT, SYSTEM
- Color-coded message bubbles
- Timestamp support
- Auto-scroll to bottom
- Mouse wheel scrolling
- Message spacing and padding

---

### Button

**Create:**
```c
CrystallineButton* btn = crystalline_button_create(
    "Label",                      // Button text
    CRYSTALLINE_SIZE_MEDIUM,      // Size preset
    true,                         // Circular (false for rectangular)
    on_click_callback,            // Callback function
    user_data                     // User data passed to callback
);
```

**Configure:**
```c
crystalline_button_set_label(btn, "New Label");
crystalline_button_set_font(btn, font);
crystalline_button_set_colors(btn, normal, hover, active, disabled);
```

**Callback:**
```c
void on_click_callback(void* user_data) {
    printf("Button clicked!\n");
}
```

### List

**Create:**
```c
CrystallineList* list = crystalline_list_create(
    bounds,
    25.0f,    // Item height
    true      // Show checkboxes
);
```

**Configure:**
```c
const char* items[] = {"Item 1", "Item 2", "Item 3"};
crystalline_list_set_items(list, items, 3);
crystalline_list_set_font(list, font);
crystalline_list_set_select_callback(list, on_select, user_data);
crystalline_list_set_check_callback(list, on_check, user_data);
```

**Callbacks:**
```c
void on_select(int index, void* user_data) {
    printf("Item selected: %d\n", index);
}

void on_check(int index, bool checked, void* user_data) {
    printf("Item %d checked: %d\n", index, checked);
}
```

### Slider

**Create:**
```c
CrystallineSlider* slider = crystalline_slider_create(
    bounds,
    0.0f,      // Min value
    100.0f,    // Max value
    50.0f,     // Initial value
    on_change,
    user_data
);
```

**Configure:**
```c
crystalline_slider_set_value(slider, 75.0f);
crystalline_slider_set_range(slider, 0.0f, 200.0f);
crystalline_slider_set_font(slider, font);
crystalline_slider_set_show_value(slider, true);
```

**Callback:**
```c
void on_change(float value, void* user_data) {
    printf("Slider value: %.1f\n", value);
}
```

### Dropdown

**Create:**
```c
CrystallineDropdown* dropdown = crystalline_dropdown_create(
    bounds,
    on_select,
    user_data
);
```

**Configure:**
```c
const char* options[] = {"Option 1", "Option 2", "Option 3"};
crystalline_dropdown_set_options(dropdown, options, 3);
crystalline_dropdown_set_font(dropdown, font);
crystalline_dropdown_set_selected(dropdown, 0);
```

**Callback:**
```c
void on_select(int index, const char* option, void* user_data) {
    printf("Selected: %d - %s\n", index, option);
}
```

### Panel

**Create:**
```c
CrystallinePanel* panel = crystalline_panel_create(
    bounds,
    "Panel Title"  // Optional title
);
```

**Configure:**
```c
crystalline_panel_set_title(panel, "New Title");
crystalline_panel_set_show_title(panel, true);
crystalline_panel_set_font(panel, font);
```

### Label

**Create:**
```c
CrystallineLabel* label = crystalline_label_create(
    bounds,
    "Label Text",
    font
);
```

**Configure:**
```c
crystalline_label_set_text(label, "New Text");
crystalline_label_set_alignment(label, CRYSTALLINE_TEXT_ALIGN_CENTER);
crystalline_label_set_color(label, crystalline_color(200, 200, 200, 255));
```

## Container System

### Create Container
```c
CrystallineContainer* container = crystalline_container_create(
    bounds,
    CRYSTALLINE_LAYOUT_VERTICAL,  // Layout type
    10.0f                         // Spacing between children
);
```

### Add Children
```c
crystalline_container_add_child(container, child1);
crystalline_container_add_child(container, child2);
crystalline_container_add_child(container, child3);
```

### Compute Layout
```c
// This positions all children automatically!
crystalline_container_compute_layout(container);
```

### Configure Layout
```c
crystalline_container_set_layout(container, CRYSTALLINE_LAYOUT_HORIZONTAL);
crystalline_container_set_spacing(container, 15.0f);
crystalline_container_set_alignment(container, CRYSTALLINE_ALIGN_CENTER);
```

### Scrolling
```c
crystalline_container_set_scrollable(container, true);
crystalline_container_scroll(container, 20.0f);  // Scroll by delta
```

### Rendering and Events
```c
// Render container and all children
crystalline_container_render(container, renderer);

// Handle events for container and all children
crystalline_container_handle_event(container, &event);
```

## Layout Types

### Vertical Layout
Stacks children top-to-bottom with spacing:
```
┌─────────────┐
│   Child 1   │
├─────────────┤ ← spacing
│   Child 2   │
├─────────────┤ ← spacing
│   Child 3   │
└─────────────┘
```

### Horizontal Layout
Stacks children left-to-right with spacing:
```
┌────┬─┬────┬─┬────┐
│ C1 │ │ C2 │ │ C3 │
└────┴─┴────┴─┴────┘
      ↑      ↑
   spacing spacing
```

### Absolute Layout
Children keep their specified positions (no automatic positioning).

## Event Handling

### Automatic Event Routing
Events are automatically routed to the correct element:

1. Container receives event
2. Container tries children (front to back)
3. If child handles event, returns true
4. If no child handles, container tries itself
5. If not handled, returns false (bubbles to parent)

### Event Types Handled
- `SDL_MOUSEMOTION` - Hover detection
- `SDL_MOUSEBUTTONDOWN` - Click detection
- `SDL_MOUSEBUTTONUP` - Release detection
- `SDL_MOUSEWHEEL` - Scrolling

### No Manual Routing Needed!
```c
// OLD WAY (manual routing)
if (button1) handle_button_event(button1, event);
if (button2) handle_button_event(button2, event);
if (slider1) handle_slider_event(slider1, event);
// ... repeat for every element

// NEW WAY (automatic routing)
crystalline_container_handle_event(container, &event);
// That's it! Events route automatically.
```

## Memory Management

### Automatic Cleanup
When you destroy a container, all children are automatically destroyed:

```c
CrystallineContainer* container = crystalline_container_create(...);
crystalline_container_add_child(container, button);
crystalline_container_add_child(container, slider);
crystalline_container_add_child(container, list);

// Destroy container - automatically destroys all children!
crystalline_container_destroy(container);
```

### Manual Cleanup
You can also destroy individual elements:
```c
crystalline_button_destroy(button);
crystalline_list_destroy(list);
```

## Best Practices

### 1. Use Size Presets
```c
// GOOD
crystalline_button_create("OK", CRYSTALLINE_SIZE_MEDIUM, true, on_click, NULL);

// AVOID
crystalline_button_create_custom("OK", custom_bounds, true, on_click, NULL);
```

### 2. Let Layout System Position Elements
```c
// GOOD - let container position children
crystalline_container_add_child(panel, button);
crystalline_container_compute_layout(panel);

// AVOID - manual positioning
button->base.bounds.center.x = 100;
button->base.bounds.center.y = 200;
```

### 3. Use Containers for Grouping
```c
// GOOD - group related buttons in container
CrystallineContainer* button_row = crystalline_container_create(
    bounds, CRYSTALLINE_LAYOUT_HORIZONTAL, 10.0f
);
crystalline_container_add_child(button_row, btn1);
crystalline_container_add_child(button_row, btn2);
crystalline_container_add_child(main_panel, (CrystallineElement*)button_row);

// AVOID - positioning buttons individually
```

### 4. Set Fonts After Creation
```c
// Create widgets
CrystallineButton* btn = crystalline_button_create(...);
CrystallineLabel* label = crystalline_label_create(...);

// Set fonts
crystalline_button_set_font(btn, font);
crystalline_label_set_font(label, font);
```

### 5. Compute Layout After Adding All Children
```c
// Add all children first
crystalline_container_add_child(panel, child1);
crystalline_container_add_child(panel, child2);
crystalline_container_add_child(panel, child3);

// Then compute layout once
crystalline_container_compute_layout(panel);
```

## Migration Guide

### Old System → New System

**Old Way:**
```c
// Manual positioning
int x = 100, y = 200;
button1 = create_button(..., x, y, ...);
button2 = create_button(..., x, y + 60, ...);

// Manual event routing
void handle_events(SDL_Event* event) {
    if (button1) handle_button_event(button1, event);
    if (button2) handle_button_event(button2, event);
}
```

**New Way:**
```c
// Automatic positioning
CrystallineContainer* panel = crystalline_container_create(
    bounds, CRYSTALLINE_LAYOUT_VERTICAL, 10.0f
);
crystalline_container_add_child(panel, button1);
crystalline_container_add_child(panel, button2);
crystalline_container_compute_layout(panel);

// Automatic event routing
void handle_events(SDL_Event* event) {
    crystalline_container_handle_event(panel, event);
}
```

**Code Reduction: 75%**

## Common Patterns

### Pattern 1: Control Panel with Sliders
```c
CrystallineContainer* control_panel = crystalline_container_create(
    bounds, CRYSTALLINE_LAYOUT_VERTICAL, 10.0f
);

// Add label
CrystallineLabel* label = crystalline_label_create(
    crystalline_bounds(0, 0, 360, 20), "Settings:", font
);
crystalline_container_add_child(control_panel, (CrystallineElement*)label);

// Add sliders
for (int i = 0; i < 4; i++) {
    CrystallineSlider* slider = crystalline_slider_create(
        crystalline_bounds(0, 0, 360, 30),
        0.0f, 100.0f, 50.0f,
        on_slider_change, (void*)(intptr_t)i
    );
    crystalline_container_add_child(control_panel, (CrystallineElement*)slider);
}

crystalline_container_compute_layout(control_panel);
```

### Pattern 2: Button Row
```c
CrystallineContainer* button_row = crystalline_container_create(
    bounds, CRYSTALLINE_LAYOUT_HORIZONTAL, 10.0f
);
button_row->base.bg_color = crystalline_color(0, 0, 0, 0);  // Transparent
button_row->base.border_width = 0.0f;

crystalline_container_add_child(button_row, btn1);
crystalline_container_add_child(button_row, btn2);
crystalline_container_add_child(button_row, btn3);

crystalline_container_compute_layout(button_row);
```

### Pattern 3: File List with Checkboxes
```c
CrystallineList* file_list = crystalline_list_create(
    bounds, 25.0f, true  // 25px per item, show checkboxes
);
crystalline_list_set_font(file_list, font);
crystalline_list_set_items(file_list, file_names, file_count);
crystalline_list_set_check_callback(file_list, on_file_checked, user_data);
```

### Pattern 4: Nested Containers
```c
// Main container (vertical)
CrystallineContainer* main = crystalline_container_create(
    bounds, CRYSTALLINE_LAYOUT_VERTICAL, 15.0f
);

// Add dropdown
crystalline_container_add_child(main, dropdown);

// Add nested button row (horizontal)
CrystallineContainer* buttons = crystalline_container_create(
    bounds, CRYSTALLINE_LAYOUT_HORIZONTAL, 10.0f
);
crystalline_container_add_child(buttons, btn1);
crystalline_container_add_child(buttons, btn2);
crystalline_container_add_child(main, (CrystallineElement*)buttons);

// Add file list
crystalline_container_add_child(main, file_list);

// Compute layout - positions everything recursively!
crystalline_container_compute_layout(main);
```

## Troubleshooting

### Issue: Elements Not Visible
**Solution:** Check visibility and bounds
```c
element->base.visible = true;
printf("Bounds: center=(%.1f, %.1f) size=(%.1fx%.1f)\n",
       element->base.bounds.center.x, element->base.bounds.center.y,
       element->base.bounds.width, element->base.bounds.height);
```

### Issue: Events Not Working
**Solution:** Ensure event routing is set up
```c
// Make sure you're calling handle_event
crystalline_container_handle_event(container, &event);

// Check if element is enabled
element->base.enabled = true;
```

### Issue: Layout Not Updating
**Solution:** Call compute_layout after changes
```c
crystalline_container_add_child(panel, new_child);
crystalline_container_compute_layout(panel);  // Recompute!
```

### Issue: Elements Overlapping
**Solution:** Use proper spacing
```c
// Increase spacing between elements
crystalline_container_set_spacing(container, 15.0f);
```

## Performance

### Rendering
- Elements only render if visible
- Children clipped to parent bounds
- No overdraw

### Event Handling
- Early exit when event is handled
- Front-to-back traversal (top elements first)
- Efficient hit testing

### Layout
- Only recomputes when invalidated
- Caches computed positions
- Efficient algorithms

## Comparison: Old vs New

### Code Size
- **Training Tab (Old):** ~200 lines of positioning code
- **Training Tab (New):** ~50 lines with automatic layout
- **Reduction:** 75%

### Maintainability
- **Old:** Change position → update 10+ lines
- **New:** Change position → update 1 line (bounds)

### Consistency
- **Old:** Each tab has different conventions
- **New:** All tabs use same system

### Type Safety
- **Old:** Ambiguous parameters (`size_param1`)
- **New:** Clear enums (`CRYSTALLINE_SIZE_MEDIUM`)

## Summary

Crystalline UI V2 provides:
- ✅ 75% code reduction
- ✅ Automatic layout management
- ✅ Unified event handling
- ✅ Type-safe APIs
- ✅ Consistent coordinate system
- ✅ Parent-child hierarchy
- ✅ Zero manual positioning
- ✅ Zero manual event routing

**Result: A TRUE unified UI library.**