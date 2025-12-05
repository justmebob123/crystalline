# Crystalline UI System - Complete Guide

## Table of Contents

1. [Introduction](#introduction)
2. [Architecture Overview](#architecture-overview)
3. [Coordinate Systems](#coordinate-systems)
4. [Event Flow](#event-flow)
5. [UI Elements Reference](#ui-elements-reference)
6. [Creating a New Tab](#creating-a-new-tab)
7. [Debugging Tools](#debugging-tools)
8. [Troubleshooting Guide](#troubleshooting-guide)
9. [Best Practices](#best-practices)
10. [Common Patterns](#common-patterns)

---

## 1. Introduction

The Crystalline UI System is a custom UI framework built on top of SDL2 that provides both traditional rectangular UI elements and sacred geometry-based circular elements. This guide provides everything you need to understand, use, and debug the UI system.

### Key Features

- **Dual Style System**: Elements can be rendered as RECTANGULAR, CIRCULAR, or HYBRID
- **Pure Crystalline Mathematics**: Uses prime-based functions instead of standard math.h
- **Event-Driven Architecture**: Clean separation between rendering and event handling
- **Consistent API**: All elements follow the same creation and usage patterns
- **Built-in Animations**: Pulse, rotation, color transitions, and position animations

---

## 2. Architecture Overview

### 2.1 Layer Structure

```
┌─────────────────────────────────────────────────────────┐
│ Application Layer (app/ui/tabs/*.c)                     │
│ - Tab-specific logic                                    │
│ - Element creation and management                       │
│ - Callback implementations                              │
└─────────────────────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────┐
│ Crystalline UI Layer (app/ui/crystalline/*.c)           │
│ - Element implementations                               │
│ - Event handling                                        │
│ - Rendering                                             │
│ - Animations                                            │
└─────────────────────────────────────────────────────────┘
                         ↓
┌─────────────────────────────────────────────────────────┐
│ SDL2 Layer                                              │
│ - Window management                                     │
│ - Event system                                          │
│ - Rendering primitives                                  │
└─────────────────────────────────────────────────────────┘
```

### 2.2 File Organization

```
app/
├── ui/
│   ├── crystalline/
│   │   ├── elements.h/c      # All UI elements
│   │   ├── geometry.h/c      # Geometric primitives
│   │   ├── draw.h/c          # Drawing functions
│   │   ├── color.h/c         # Color utilities
│   │   ├── animation.h/c     # Animation system
│   │   ├── layout.h/c        # Layout helpers
│   │   ├── global_layout.h/c # Global layout system
│   │   └── textarea.h/c      # TextArea widget
│   └── tabs/
│       ├── tab_training.c    # Training tab (REFERENCE)
│       ├── tab_llm.c         # LLM tab (REFERENCE)
│       └── tab_*.c           # Other tabs
```

---

## 3. Coordinate Systems

### 3.1 SDL Window Coordinates (TOP-LEFT)

SDL uses a **TOP-LEFT** origin coordinate system:

```
(0,0) ────────────────────────> X
  │
  │    Window Content
  │
  │
  ↓
  Y
```

- Origin: Top-left corner (0, 0)
- X increases to the RIGHT
- Y increases DOWNWARD
- Used by: SDL events, SDL rendering

### 3.2 Crystalline UI Coordinates (CENTER)

Crystalline UI elements use **CENTER** coordinates internally:

```
        Element
    ┌─────────────┐
    │             │
    │   (cx, cy)  │  ← Center point
    │      •      │
    │             │
    └─────────────┘
```

- Origin: Center of the element
- Used by: All Crystalline UI elements
- Converted from TOP-LEFT when creating elements

### 3.3 Conversion Formulas

#### TOP-LEFT to CENTER
```c
float center_x = top_left_x + width / 2.0f;
float center_y = top_left_y + height / 2.0f;
```

#### CENTER to TOP-LEFT
```c
float top_left_x = center_x - width / 2.0f;
float top_left_y = center_y - height / 2.0f;
```

### 3.4 Layout Zones

The application window is divided into zones:

```
┌──────┬────────────────────────────────────────────┬──────────┐
│      │ SUBMENU (Y < 40, X >= 200)                 │          │
│ SIDE ├────────────────────────────────────────────┤ CONTROL  │
│ BAR  │                                            │ PANEL    │
│      │                                            │          │
│ X<200│  RENDER AREA (200 <= X < 1280)            │ X>=1280  │
│      │  (Tab content goes here)                   │          │
│      │                                            │          │
│      │                                            │          │
└──────┴────────────────────────────────────────────┴──────────┘
  0    200                                        1280       1600
```

**Constants:**
- `SIDEBAR_WIDTH = 200`
- `SUBMENU_HEIGHT = 40`
- `RENDER_OFFSET_X = 200`
- `RENDER_OFFSET_Y = 40`
- `RENDER_WIDTH = 1080` (1280 - 200)
- `CONTROL_PANEL_WIDTH = 320`
- `WINDOW_WIDTH = 1600`
- `WINDOW_HEIGHT = 900`

---

## 4. Event Flow

### 4.1 Complete Event Flow Diagram

```
SDL Event
    ↓
main.c: handle_events()
    ↓
┌─────────────────────────────────────────────────────────┐
│ Event Type Routing                                      │
├─────────────────────────────────────────────────────────┤
│ SDL_MOUSEBUTTONDOWN                                     │
│ SDL_MOUSEBUTTONUP                                       │
│ SDL_MOUSEMOTION                                         │
│ SDL_KEYDOWN                                             │
│ SDL_TEXTINPUT                                           │
└─────────────────────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────────────────────┐
│ Spatial Routing (by coordinates)                        │
├─────────────────────────────────────────────────────────┤
│ Priority 1: Submenu (Y < 40, X >= 200)                 │
│ Priority 2: Sidebar (X < 200)                           │
│ Priority 3: Content Area (Y >= 40, X >= 200)           │
└─────────────────────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────────────────────┐
│ Handler Routing                                         │
├─────────────────────────────────────────────────────────┤
│ Submenu → handle_submenu_click()                        │
│ Sidebar → handle_sidebar_click()                        │
│ Content → Tab-specific handlers                         │
└─────────────────────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────────────────────┐
│ Tab-Specific Event Handlers                             │
├─────────────────────────────────────────────────────────┤
│ handle_<tab>_mouse_down(state, x, y)                    │
│ handle_<tab>_mouse_up(state, x, y)                      │
│ handle_<tab>_mouse_motion(state, x, y)                  │
│ handle_<tab>_keydown(state, key)                        │
│ handle_<tab>_text_input(state, text)                    │
└─────────────────────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────────────────────┐
│ Crystalline UI Element Handlers                         │
├─────────────────────────────────────────────────────────┤
│ crystalline_button_handle_mouse(button, &event)         │
│ crystalline_slider_handle_mouse(slider, &event)         │
│ crystalline_input_handle_mouse(input, &event)           │
│ crystalline_dropdown_handle_mouse(dropdown, &event)     │
│ etc.                                                    │
└─────────────────────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────────────────────┐
│ Callbacks (Application Logic)                           │
├─────────────────────────────────────────────────────────┤
│ on_button_clicked(void* data)                           │
│ on_slider_changed(float value, void* data)              │
│ on_input_changed(const char* text, void* data)          │
│ on_dropdown_selected(int index, void* data)             │
│ etc.                                                    │
└─────────────────────────────────────────────────────────┘
```

### 4.2 Event Routing in main.c

**CRITICAL**: The event routing order matters!

```c
case SDL_MOUSEBUTTONDOWN:
    // Priority 1: Check submenu FIRST
    if (event->button.y < SUBMENU_HEIGHT && event->button.x >= SIDEBAR_WIDTH) {
        handle_mouse_click(state, x, y);  // Routes to handle_submenu_click()
    }
    // Priority 2: Check sidebar
    else if (event->button.x < SIDEBAR_WIDTH) {
        handle_mouse_click(state, x, y);  // Routes to handle_sidebar_click()
    }
    // Priority 3: Route to tab handlers
    else {
        if (state->current_tab == TAB_TRAINING) {
            handle_training_tab_mouse_down(state, x, y);
        } else if (state->current_tab == TAB_LLM) {
            handle_llm_tab_mouse_down(state, x, y);
        }
        // ... other tabs
    }
    break;
```

**Common Bug**: Forgetting to check submenu BEFORE sidebar causes tab clicks to not work!

### 4.3 Button Click Sequence

Buttons require **BOTH** DOWN and UP events to trigger:

```
1. User presses mouse button
   ↓ SDL_MOUSEBUTTONDOWN
2. handle_tab_mouse_down() called
   ↓ Creates dummy MOUSEBUTTONDOWN event
3. crystalline_button_handle_mouse() called
   ↓ Button state = ACTIVE
4. User releases mouse button
   ↓ SDL_MOUSEBUTTONUP
5. handle_tab_mouse_up() called
   ↓ Creates dummy MOUSEBUTTONUP event
6. crystalline_button_handle_mouse() called
   ↓ Checks: state == ACTIVE && inside && MOUSEBUTTONUP
7. button->on_click(user_data) called
   ↓ Callback executes
```

**Common Bug**: Only routing MOUSEBUTTONDOWN causes buttons to never fire callbacks!

---

## 5. UI Elements Reference

### 5.1 Button

**Purpose**: Clickable button with label

**Creation**:
```c
CrystallineButton* button = crystalline_button_create(
    CRYSTALLINE_STYLE_RECTANGULAR,  // or CIRCULAR, HYBRID
    center_x,                       // CENTER X coordinate
    center_y,                       // CENTER Y coordinate
    width,                          // Width (or radius for circular)
    height,                         // Height (unused for circular)
    "Click Me",                     // Label text
    font                            // TTF_Font*
);
```

**Set Callback**:
```c
void on_button_clicked(void* data) {
    AppState* state = (AppState*)data;
    // Handle button click
}

crystalline_button_set_callback(button, on_button_clicked, state);
```

**Event Handling**:
```c
// In mouse_down handler:
crystalline_button_handle_mouse(button, &event);

// In mouse_up handler:
crystalline_button_handle_mouse(button, &event);  // Triggers callback!

// In mouse_motion handler:
crystalline_button_handle_mouse(button, &event);  // Updates hover state
```

**Rendering**:
```c
crystalline_button_render(button, renderer);
```

**Cleanup**:
```c
crystalline_button_destroy(button);
```

### 5.2 Slider

**Purpose**: Adjustable value slider

**Creation**:
```c
CrystallineSlider* slider = crystalline_slider_create(
    CRYSTALLINE_STYLE_RECTANGULAR,  // or CIRCULAR (arc)
    center_x,
    center_y,
    width,                          // Track width (or arc radius)
    height,                         // Track height (or unused)
    min_value,                      // Minimum value
    max_value                       // Maximum value
);
```

**Set Value and Callback**:
```c
void on_slider_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    state->some_parameter = value;
}

crystalline_slider_set_value(slider, initial_value);
crystalline_slider_set_callback(slider, on_slider_changed, state);
```

**Get Value**:
```c
float value = crystalline_slider_get_value(slider);
```

**Event Handling**:
```c
// All three event types needed for dragging:
crystalline_slider_handle_mouse(slider, &event);
```

### 5.3 Input Field

**Purpose**: Text input field

**Creation**:
```c
CrystallineInput* input = crystalline_input_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    center_x,
    center_y,
    width,
    height,
    "Placeholder text...",          // Placeholder
    font
);
```

**Set Callbacks**:
```c
void on_input_changed(const char* text, void* data) {
    // Called on every character typed
}

void on_input_submitted(const char* text, void* data) {
    // Called when Enter is pressed
}

crystalline_input_set_callbacks(input, on_input_changed, on_input_submitted, state);
```

**Get/Set Text**:
```c
const char* text = crystalline_input_get_text(input);
crystalline_input_set_text(input, "New text");
```

**Event Handling**:
```c
// Mouse events (for focus):
crystalline_input_handle_mouse(input, &event);

// Keyboard events (for typing):
crystalline_input_handle_keyboard(input, &event);
```

### 5.4 Dropdown

**Purpose**: Selection dropdown menu

**Creation**:
```c
CrystallineDropdown* dropdown = crystalline_dropdown_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    center_x,
    center_y,
    width,
    height,
    font
);
```

**Set Options and Callback**:
```c
void on_dropdown_selected(int index, void* data) {
    AppState* state = (AppState*)data;
    // Handle selection
}

char* options[] = {"Option 1", "Option 2", "Option 3"};
crystalline_dropdown_set_options(dropdown, options, 3);
crystalline_dropdown_set_callback(dropdown, on_dropdown_selected, state);
```

**Event Handling**:
```c
// All three event types needed:
crystalline_dropdown_handle_mouse(dropdown, &event);
```

### 5.5 TextArea

**Purpose**: Multi-line scrollable text display (for chat)

**Creation**:
```c
CrystallineTextArea* textarea = crystalline_textarea_create(
    CRYSTALLINE_STYLE_RECTANGULAR,  // Only rectangular supported
    center_x,
    center_y,
    width,
    height,
    font
);
```

**Add Messages**:
```c
crystalline_textarea_add_message(
    textarea,
    CRYSTALLINE_MESSAGE_USER,       // or ASSISTANT, SYSTEM
    "Message text",
    "12:34:56"                      // Timestamp
);
```

**Clear and Scroll**:
```c
crystalline_textarea_clear(textarea);
crystalline_textarea_scroll(textarea, delta);  // Positive = down, negative = up
```

**Event Handling**:
```c
crystalline_textarea_handle_event(textarea, &event);
```

### 5.6 List

**Purpose**: Scrollable list with optional checkboxes

**Creation**:
```c
CrystallineList* list = crystalline_list_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    center_x,
    center_y,
    width,
    item_height,                    // Height per item (NOT total height)
    font
);
```

**Set Items**:
```c
char* items[] = {"Item 1", "Item 2", "Item 3"};
crystalline_list_set_items(list, items, 3);
```

**Enable Checkboxes**:
```c
void on_checkbox_changed(int index, bool checked, void* data) {
    // Handle checkbox toggle
}

crystalline_list_enable_checkboxes(list, 24.0f);  // Checkbox size
crystalline_list_set_check_callback(list, on_checkbox_changed, state);
```

**Event Handling**:
```c
crystalline_list_handle_mouse(list, &event);
```

### 5.7 Toggle

**Purpose**: On/off switch

**Creation**:
```c
CrystallineToggle* toggle = crystalline_toggle_create(
    CRYSTALLINE_STYLE_RECTANGULAR,  // or CIRCULAR (yin-yang)
    center_x,
    center_y,
    width,
    height,
    "Label",
    font
);
```

**Set Value and Callback**:
```c
void on_toggle_changed(bool value, void* data) {
    // Handle toggle
}

crystalline_toggle_set_value(toggle, true);
crystalline_toggle_set_callback(toggle, on_toggle_changed, state);
```

### 5.8 Progress Bar

**Purpose**: Progress indicator

**Creation**:
```c
CrystallineProgress* progress = crystalline_progress_create(
    CRYSTALLINE_STYLE_RECTANGULAR,  // or CIRCULAR (ring)
    center_x,
    center_y,
    width,
    height
);
```

**Set Value**:
```c
crystalline_progress_set_value(progress, 0.75f);  // 0.0 to 1.0
crystalline_progress_set_label(progress, "Loading...");
```

---

## 6. Creating a New Tab

### 6.1 Step-by-Step Guide

#### Step 1: Create Tab File

Create `app/ui/tabs/tab_mytab.c`:

```c
#include "../../app_common.h"
#include "../crystalline/elements.h"
#include "../crystalline/global_layout.h"

// UI State
static struct {
    CrystallineButton* btn_action;
    CrystallineSlider* slider_value;
    CrystallineInput* input_field;
    bool initialized;
} mytab_ui = {0};

// Callbacks
static void on_action_clicked(void* data) {
    AppState* state = (AppState*)data;
    // Handle action
}

static void on_value_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    // Handle value change
}

// Initialize tab
void init_mytab(AppState* state) {
    if (mytab_ui.initialized) return;
    
    extern TTF_Font* get_global_font(void);
    TTF_Font* font = get_global_font();
    
    // Calculate positions (TOP-LEFT)
    int x = RENDER_OFFSET_X + 10;
    int y = RENDER_OFFSET_Y + 10;
    int w = 200;
    int h = 40;
    
    // Convert to CENTER and create button
    mytab_ui.btn_action = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        x + w / 2.0f,  // CENTER X
        y + h / 2.0f,  // CENTER Y
        w, h,
        "Action",
        font
    );
    crystalline_button_set_callback(mytab_ui.btn_action, on_action_clicked, state);
    
    // Create more elements...
    
    mytab_ui.initialized = true;
}

// Draw tab
void draw_mytab(SDL_Renderer* renderer, AppState* state) {
    if (!mytab_ui.initialized) {
        init_mytab(state);
    }
    
    // Render elements
    if (mytab_ui.btn_action) {
        crystalline_button_render(mytab_ui.btn_action, renderer);
    }
    // Render more elements...
}

// Event handlers
void handle_mytab_mouse_down(AppState* state, int x, int y) {
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEBUTTONDOWN;
    dummy_event.button.x = x;
    dummy_event.button.y = y;
    dummy_event.button.button = SDL_BUTTON_LEFT;
    
    if (mytab_ui.btn_action) crystalline_button_handle_mouse(mytab_ui.btn_action, &dummy_event);
    // Handle more elements...
}

void handle_mytab_mouse_up(AppState* state, int x, int y) {
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEBUTTONUP;
    dummy_event.button.x = x;
    dummy_event.button.y = y;
    dummy_event.button.button = SDL_BUTTON_LEFT;
    
    if (mytab_ui.btn_action) crystalline_button_handle_mouse(mytab_ui.btn_action, &dummy_event);
    // Handle more elements...
}

void handle_mytab_mouse_motion(AppState* state, int x, int y) {
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEMOTION;
    dummy_event.motion.x = x;
    dummy_event.motion.y = y;
    
    if (mytab_ui.btn_action) crystalline_button_handle_mouse(mytab_ui.btn_action, &dummy_event);
    // Handle more elements...
}
```

#### Step 2: Add to main.c Event Loop

In `app/main.c`, add routing for your tab:

```c
case SDL_MOUSEBUTTONDOWN:
    // ... existing code ...
    else {
        if (state->current_tab == TAB_MYTAB) {
            extern void handle_mytab_mouse_down(AppState* state, int x, int y);
            handle_mytab_mouse_down(state, x, y);
        }
        // ... other tabs ...
    }
    break;

case SDL_MOUSEBUTTONUP:
    // ... existing code ...
    else {
        if (state->current_tab == TAB_MYTAB) {
            extern void handle_mytab_mouse_up(AppState* state, int x, int y);
            handle_mytab_mouse_up(state, x, y);
        }
        // ... other tabs ...
    }
    break;

case SDL_MOUSEMOTION:
    if (state->current_tab == TAB_MYTAB) {
        extern void handle_mytab_mouse_motion(AppState* state, int x, int y);
        handle_mytab_mouse_motion(state, x, y);
    }
    // ... other tabs ...
    break;
```

#### Step 3: Add to Makefile

In `app/Makefile`, add your tab to the sources:

```makefile
TAB_SOURCES = ui/tabs/tab_training.c \
              ui/tabs/tab_llm.c \
              ui/tabs/tab_mytab.c \
              # ... other tabs
```

---

## 7. Debugging Tools

### 7.1 UI Layout Analyzer (ASCII)

**Location**: `tools/ui_layout_analyzer.c`

**Purpose**: Visualize UI layout in ASCII art

**Usage**:
```bash
cd tools
gcc -o ui_layout_analyzer ui_layout_analyzer.c -lm
./ui_layout_analyzer
```

**Features**:
- ASCII visualization (160x90 characters, 10px per char)
- Interactive mode for click/movement simulation
- Hit-testing verification
- Layout data export

**Output Example**:
```
┌────────────────────────────────────────────────────────────┐
│S │ SUBMENU                                                  │
│I ├──────────────────────────────────────────────────────────┤
│D │                                                          │
│E │  RENDER AREA                                             │
│B │                                                          │
│A │                                                          │
│R │                                                          │
└──┴──────────────────────────────────────────────────────────┘
```

### 7.2 UI Layout Visualizer (Python)

**Location**: `tools/ui_layout_visualizer.py`

**Purpose**: Generate pixel-perfect PNG visualizations

**Usage**:
```bash
cd tools
python3 ui_layout_visualizer.py
```

**Features**:
- Pixel-perfect matplotlib visualization
- Generates PNG images with exact measurements
- Gap analysis with measurement lines
- Color-coded elements
- Much more accurate than ASCII tool

**Output**: `ui_layout_visualization.png`

### 7.3 Debug Printf Patterns

**Button Clicks**:
```c
static void on_button_clicked(void* data) {
    printf("=== BUTTON CLICKED ===\\n");
    printf("Data: %p\\n", data);
    // Your logic
    printf("=== BUTTON HANDLER COMPLETE ===\\n");
}
```

**Event Routing**:
```c
void handle_tab_mouse_down(AppState* state, int x, int y) {
    printf("=== MOUSE DOWN: x=%d, y=%d ===\\n", x, y);
    // Event handling
}
```

**Element Creation**:
```c
void init_tab(AppState* state) {
    printf("=== INITIALIZING TAB ===\\n");
    printf("  Creating button at (%d, %d)\\n", x, y);
    // Element creation
    printf("=== TAB INITIALIZED ===\\n");
}
```

### 7.4 AddressSanitizer

The application is built with AddressSanitizer enabled:

```bash
# Build with sanitizers (default)
make app

# Run and check for memory errors
LD_LIBRARY_PATH=.. ./hyper_prime_spiral
```

**Common Errors Detected**:
- Heap-use-after-free
- Stack-buffer-overflow
- Memory leaks
- Use of uninitialized memory

---

## 8. Troubleshooting Guide

### 8.1 Buttons Don't Work

**Symptom**: Clicking buttons does nothing, no callback fires

**Diagnosis**:
1. Check if BOTH mouse_down AND mouse_up handlers are called
2. Verify event routing in main.c includes your tab
3. Add debug printf in callback to verify it's registered

**Solution**:
```c
// Make sure BOTH handlers pass events to button:
void handle_tab_mouse_down(AppState* state, int x, int y) {
    // ... create event ...
    crystalline_button_handle_mouse(button, &event);  // Sets ACTIVE
}

void handle_tab_mouse_up(AppState* state, int x, int y) {
    // ... create event ...
    crystalline_button_handle_mouse(button, &event);  // Triggers callback!
}
```

### 8.2 Submenu Tabs Don't Work

**Symptom**: Can't click on tabs at the top of the screen

**Diagnosis**:
1. Check event routing order in main.c
2. Verify submenu check comes BEFORE sidebar check

**Solution**:
```c
// CORRECT order:
if (event->button.y < SUBMENU_HEIGHT && event->button.x >= SIDEBAR_WIDTH) {
    handle_mouse_click(state, x, y);  // Submenu FIRST
} else if (event->button.x < SIDEBAR_WIDTH) {
    handle_mouse_click(state, x, y);  // Sidebar SECOND
} else {
    // Tab handlers LAST
}
```

### 8.3 Elements Appear in Wrong Position

**Symptom**: Elements are offset or in completely wrong location

**Diagnosis**:
1. Check if you're using TOP-LEFT or CENTER coordinates
2. Verify coordinate conversion
3. Use visualization tools to see actual positions

**Solution**:
```c
// ALWAYS convert TOP-LEFT to CENTER:
int element_x = 100;  // TOP-LEFT X
int element_y = 50;   // TOP-LEFT Y
int element_w = 200;
int element_h = 40;

// Convert to CENTER:
float center_x = element_x + element_w / 2.0f;  // 100 + 100 = 200
float center_y = element_y + element_h / 2.0f;  // 50 + 20 = 70

// Create element with CENTER coordinates:
button = crystalline_button_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    center_x,  // 200
    center_y,  // 70
    element_w,
    element_h,
    "Label",
    font
);
```

### 8.4 Input Field Not Accepting Text

**Symptom**: Can't type in input field

**Diagnosis**:
1. Check if input field has focus (clicked)
2. Verify keyboard events are routed to input
3. Check if SDL_StartTextInput() is called

**Solution**:
```c
// In mouse_down handler:
crystalline_input_handle_mouse(input, &event);  // Gains focus

// In keyboard handler:
crystalline_input_handle_keyboard(input, &event);  // Processes keys

// In text input handler:
crystalline_input_handle_keyboard(input, &event);  // Processes characters
```

### 8.5 Dropdown Doesn't Expand

**Symptom**: Clicking dropdown does nothing

**Diagnosis**:
1. Check if dropdown has options set
2. Verify all three mouse event types are handled
3. Check if dropdown is rendered

**Solution**:
```c
// Set options:
char* options[] = {"Option 1", "Option 2"};
crystalline_dropdown_set_options(dropdown, options, 2);

// Handle ALL three event types:
crystalline_dropdown_handle_mouse(dropdown, &mouse_down_event);
crystalline_dropdown_handle_mouse(dropdown, &mouse_up_event);
crystalline_dropdown_handle_mouse(dropdown, &mouse_motion_event);

// Render:
crystalline_dropdown_render(dropdown, renderer);
```

### 8.6 Slider Doesn't Drag

**Symptom**: Slider doesn't respond to dragging

**Diagnosis**:
1. Check if all three mouse event types are handled
2. Verify slider is created with correct range

**Solution**:
```c
// Create with range:
slider = crystalline_slider_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    center_x, center_y,
    width, height,
    0.0f,   // min_value
    100.0f  // max_value
);

// Handle ALL three event types:
crystalline_slider_handle_mouse(slider, &mouse_down_event);   // Start drag
crystalline_slider_handle_mouse(slider, &mouse_motion_event); // Dragging
crystalline_slider_handle_mouse(slider, &mouse_up_event);     // End drag
```

---

## 9. Best Practices

### 9.1 Element Lifecycle

**Always follow this pattern**:

1. **Create** elements in init function
2. **Store** pointers in static struct
3. **Render** in draw function
4. **Handle events** in event handlers
5. **Destroy** in cleanup function

```c
// 1. Create
void init_tab(AppState* state) {
    if (tab_ui.initialized) return;
    tab_ui.button = crystalline_button_create(...);
    tab_ui.initialized = true;
}

// 2. Render
void draw_tab(SDL_Renderer* renderer, AppState* state) {
    if (!tab_ui.initialized) init_tab(state);
    crystalline_button_render(tab_ui.button, renderer);
}

// 3. Handle events
void handle_tab_mouse_down(AppState* state, int x, int y) {
    crystalline_button_handle_mouse(tab_ui.button, &event);
}

// 4. Cleanup
void cleanup_tab(void) {
    if (tab_ui.button) crystalline_button_destroy(tab_ui.button);
}
```

### 9.2 Coordinate Calculation

**Always calculate in this order**:

1. Calculate TOP-LEFT position
2. Calculate width and height
3. Convert to CENTER
4. Create element

```c
// 1. TOP-LEFT
int x = RENDER_OFFSET_X + 10;
int y = RENDER_OFFSET_Y + 50;

// 2. Size
int w = 200;
int h = 40;

// 3. CENTER
float cx = x + w / 2.0f;
float cy = y + h / 2.0f;

// 4. Create
button = crystalline_button_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    cx, cy, w, h,
    "Label", font
);
```

### 9.3 Event Handler Pattern

**Always implement all three handlers**:

```c
void handle_tab_mouse_down(AppState* state, int x, int y) {
    SDL_Event event = {0};
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Pass to ALL elements
    if (button) crystalline_button_handle_mouse(button, &event);
    if (slider) crystalline_slider_handle_mouse(slider, &event);
    if (input) crystalline_input_handle_mouse(input, &event);
    if (dropdown) crystalline_dropdown_handle_mouse(dropdown, &event);
}

void handle_tab_mouse_up(AppState* state, int x, int y) {
    SDL_Event event = {0};
    event.type = SDL_MOUSEBUTTONUP;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Pass to ALL elements - triggers callbacks!
    if (button) crystalline_button_handle_mouse(button, &event);
    if (slider) crystalline_slider_handle_mouse(slider, &event);
    if (dropdown) crystalline_dropdown_handle_mouse(dropdown, &event);
}

void handle_tab_mouse_motion(AppState* state, int x, int y) {
    SDL_Event event = {0};
    event.type = SDL_MOUSEMOTION;
    event.motion.x = x;
    event.motion.y = y;
    
    // Pass to ALL elements - updates hover states
    if (button) crystalline_button_handle_mouse(button, &event);
    if (slider) crystalline_slider_handle_mouse(slider, &event);
    if (dropdown) crystalline_dropdown_handle_mouse(dropdown, &event);
}
```

### 9.4 Callback Pattern

**Always use this callback signature**:

```c
static void on_element_action(TYPE value, void* data) {
    AppState* state = (AppState*)data;
    
    // 1. Validate
    if (!state) return;
    
    // 2. Update state
    state->some_value = value;
    
    // 3. Trigger actions
    // ... your logic ...
    
    // 4. Debug output (optional)
    printf("Element action: value=%TYPE\\n", value);
}
```

### 9.5 Layout Spacing

**Use consistent spacing**:

```c
#define SPACING_TINY 5.0f
#define SPACING_SMALL 10.0f
#define SPACING_MEDIUM 15.0f
#define SPACING_LARGE 20.0f
#define SPACING_XLARGE 30.0f

// Example:
int y = RENDER_OFFSET_Y + SPACING_SMALL;
button1 = create_button(..., y, ...);
y += button_height + SPACING_MEDIUM;
button2 = create_button(..., y, ...);
```

---

## 10. Common Patterns

### 10.1 Control Panel Layout

**Pattern**: Vertical stack of controls on the right side

```c
int ctrl_x = WINDOW_WIDTH - CONTROL_PANEL_WIDTH + 10;
int ctrl_w = CONTROL_PANEL_WIDTH - 20;
int ctrl_y = RENDER_OFFSET_Y + 10;
float center_x = ctrl_x + ctrl_w / 2.0f;

// Dropdown
dropdown = crystalline_dropdown_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    center_x, ctrl_y + 20,
    ctrl_w, 40,
    font
);
ctrl_y += 70;

// Slider 1
slider1 = crystalline_slider_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    center_x, ctrl_y,
    ctrl_w, 30,
    min, max
);
ctrl_y += 70;

// Slider 2
slider2 = crystalline_slider_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    center_x, ctrl_y,
    ctrl_w, 30,
    min, max
);
ctrl_y += 90;

// Button
button = crystalline_button_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    center_x, ctrl_y,
    ctrl_w, 40,
    "Action", font
);
```

### 10.2 Chat Interface Layout

**Pattern**: TextArea + Input + Send button

```c
// Chat area (top)
int chat_x = RENDER_OFFSET_X + 10;
int chat_y = RENDER_OFFSET_Y + 10;
int chat_w = RENDER_WIDTH - 20;
int chat_h = WINDOW_HEIGHT - RENDER_OFFSET_Y - 110;

chat_area = crystalline_textarea_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    chat_x + chat_w / 2.0f,
    chat_y + chat_h / 2.0f,
    chat_w, chat_h,
    font
);

// Input field (bottom)
int input_x = chat_x;
int input_y = RENDER_OFFSET_Y + WINDOW_HEIGHT - RENDER_OFFSET_Y - 90;
int input_w = chat_w - 110;
int input_h = 80;

input_field = crystalline_input_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    input_x + input_w / 2.0f,
    input_y + input_h / 2.0f,
    input_w, input_h,
    "Type message...", font
);

// Send button (next to input)
int send_x = input_x + input_w + 10;
int send_w = 100;

send_button = crystalline_button_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    send_x + send_w / 2.0f,
    input_y + input_h / 2.0f,
    send_w, input_h,
    "SEND", font
);
```

### 10.3 Form Layout

**Pattern**: Labels + inputs in vertical stack

```c
int form_x = RENDER_OFFSET_X + 20;
int form_y = RENDER_OFFSET_Y + 20;
int label_w = 100;
int input_w = 300;
int row_h = 40;

// Row 1: Name
draw_text(renderer, "Name:", form_x, form_y + 10, text_color);
input_name = crystalline_input_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    form_x + label_w + input_w / 2.0f,
    form_y + row_h / 2.0f,
    input_w, row_h,
    "Enter name...", font
);
form_y += row_h + SPACING_MEDIUM;

// Row 2: Email
draw_text(renderer, "Email:", form_x, form_y + 10, text_color);
input_email = crystalline_input_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    form_x + label_w + input_w / 2.0f,
    form_y + row_h / 2.0f,
    input_w, row_h,
    "Enter email...", font
);
form_y += row_h + SPACING_LARGE;

// Submit button
button_submit = crystalline_button_create(
    CRYSTALLINE_STYLE_RECTANGULAR,
    form_x + label_w + input_w / 2.0f,
    form_y + row_h / 2.0f,
    input_w, row_h,
    "Submit", font
);
```

---

## Conclusion

This guide covers everything you need to work with the Crystalline UI System. Remember:

1. **Always use CENTER coordinates** when creating elements
2. **Always handle all three mouse event types** (down, up, motion)
3. **Always check submenu BEFORE sidebar** in event routing
4. **Always use the visualization tools** when debugging layout issues
5. **Always follow the reference implementations** (Training tab, LLM tab)

For more examples, see:
- `app/ui/tabs/tab_training.c` - Complete reference implementation
- `app/ui/tabs/tab_llm.c` - Chat interface reference
- `app/ui/crystalline/elements.c` - Element implementations

---

**Last Updated**: 2024
**Version**: 1.0
**Status**: Complete