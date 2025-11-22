# TEXT INPUT INTEGRATION GUIDE

## Overview
This guide explains how to integrate the new text input component into the Training and Research tabs.

---

## Text Input Component

### Files Created:
- `app/text_input.h` - Header file with API
- `app/text_input.c` - Implementation

### Features:
- Click to activate/deactivate
- Keyboard input with full editing support
- Numeric-only mode with validation
- Visual feedback (active/inactive states)
- Label support
- Min/max value constraints

---

## Integration Steps

### Step 1: Include the Header

Add to your tab file:
```c
#include "text_input.h"
```

### Step 2: Declare Input Fields

In your tab's static variables:
```c
static TextInput learning_rate_input;
static TextInput batch_size_input;
static TextInput epochs_input;
static bool inputs_initialized = false;
```

### Step 3: Initialize Inputs

In your tab's render function, initialize once:
```c
if (!inputs_initialized) {
    text_input_init(&learning_rate_input, "Learning Rate:", 50, 100, 200, 30);
    text_input_set_numeric(&learning_rate_input, 0.0001, 1.0);
    text_input_set_text(&learning_rate_input, "0.001");
    
    text_input_init(&batch_size_input, "Batch Size:", 50, 150, 200, 30);
    text_input_set_numeric(&batch_size_input, 1, 1024);
    text_input_set_text(&batch_size_input, "32");
    
    text_input_init(&epochs_input, "Epochs:", 50, 200, 200, 30);
    text_input_set_numeric(&epochs_input, 1, 1000);
    text_input_set_text(&epochs_input, "10");
    
    inputs_initialized = true;
}
```

### Step 4: Handle Events

In your tab's event handler:
```c
void tab_training_handle_event(AppState* state, SDL_Event* event) {
    // Handle text input events
    if (text_input_handle_event(&learning_rate_input, event)) return;
    if (text_input_handle_event(&batch_size_input, event)) return;
    if (text_input_handle_event(&epochs_input, event)) return;
    
    // ... rest of event handling
}
```

### Step 5: Render Inputs

In your tab's render function:
```c
void tab_training_render(AppState* state, SDL_Renderer* renderer, TTF_Font* font) {
    // ... other rendering
    
    // Render text inputs
    text_input_render(&learning_rate_input, renderer, font);
    text_input_render(&batch_size_input, renderer, font);
    text_input_render(&epochs_input, renderer, font);
    
    // ... rest of rendering
}
```

### Step 6: Use Input Values

When starting training:
```c
void start_training(AppState* state) {
    double learning_rate = text_input_get_number(&learning_rate_input);
    int batch_size = (int)text_input_get_number(&batch_size_input);
    int epochs = (int)text_input_get_number(&epochs_input);
    
    printf("Training with: LR=%.4f, Batch=%d, Epochs=%d\n", 
           learning_rate, batch_size, epochs);
    
    // Use these values in your training configuration
    state->training_learning_rate = learning_rate;
    state->training_batch_size = batch_size;
    state->training_epochs = epochs;
}
```

---

## Example: Training Tab Integration

### Complete Example:

```c
// At the top of tab_training.c
#include "text_input.h"

// Static variables
static TextInput learning_rate_input;
static TextInput batch_size_input;
static TextInput epochs_input;
static bool inputs_initialized = false;

// In render function
void tab_training_render(AppState* state, SDL_Renderer* renderer, TTF_Font* font) {
    // Initialize inputs once
    if (!inputs_initialized) {
        text_input_init(&learning_rate_input, "Learning Rate:", 50, 100, 200, 30);
        text_input_set_numeric(&learning_rate_input, 0.0001, 1.0);
        text_input_set_text(&learning_rate_input, "0.001");
        
        text_input_init(&batch_size_input, "Batch Size:", 50, 150, 200, 30);
        text_input_set_numeric(&batch_size_input, 1, 1024);
        text_input_set_text(&batch_size_input, "32");
        
        text_input_init(&epochs_input, "Epochs:", 50, 200, 200, 30);
        text_input_set_numeric(&epochs_input, 1, 1000);
        text_input_set_text(&epochs_input, "10");
        
        inputs_initialized = true;
    }
    
    // Render inputs
    text_input_render(&learning_rate_input, renderer, font);
    text_input_render(&batch_size_input, renderer, font);
    text_input_render(&epochs_input, renderer, font);
    
    // ... rest of rendering
}

// In event handler
void tab_training_handle_event(AppState* state, SDL_Event* event) {
    // Handle text input events first
    if (text_input_handle_event(&learning_rate_input, event)) return;
    if (text_input_handle_event(&batch_size_input, event)) return;
    if (text_input_handle_event(&epochs_input, event)) return;
    
    // Handle other events
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        // Check if "Start Training" button clicked
        // ...
        if (start_button_clicked) {
            double lr = text_input_get_number(&learning_rate_input);
            int batch = (int)text_input_get_number(&batch_size_input);
            int epochs = (int)text_input_get_number(&epochs_input);
            
            // Start training with these parameters
            start_training(state, lr, batch, epochs);
        }
    }
}
```

---

## Example: Research Tab Integration

### Search Input:

```c
// At the top of tab_research.c
#include "text_input.h"

// Static variables
static TextInput search_input;
static bool search_initialized = false;

// In render function
void tab_research_render(AppState* state, SDL_Renderer* renderer, TTF_Font* font) {
    // Initialize search input once
    if (!search_initialized) {
        text_input_init(&search_input, "Search:", 50, 50, 300, 30);
        search_initialized = true;
    }
    
    // Render search input
    text_input_render(&search_input, renderer, font);
    
    // ... rest of rendering
}

// In event handler
void tab_research_handle_event(AppState* state, SDL_Event* event) {
    // Handle search input
    if (text_input_handle_event(&search_input, event)) {
        // If Enter was pressed (input deactivated), perform search
        if (!text_input_is_active(&search_input)) {
            const char* query = text_input_get_text(&search_input);
            perform_search(query);
        }
        return;
    }
    
    // ... rest of event handling
}
```

---

## API Reference

### Initialization
```c
void text_input_init(TextInput* input, const char* label, int x, int y, int w, int h);
void text_input_set_numeric(TextInput* input, double min, double max);
```

### Event Handling
```c
bool text_input_handle_event(TextInput* input, SDL_Event* event);
```
Returns `true` if event was handled, `false` otherwise.

### Rendering
```c
void text_input_render(TextInput* input, SDL_Renderer* renderer, TTF_Font* font);
```

### Getting Values
```c
const char* text_input_get_text(TextInput* input);
double text_input_get_number(TextInput* input);
```

### Setting Values
```c
void text_input_set_text(TextInput* input, const char* text);
void text_input_clear(TextInput* input);
```

### State Management
```c
bool text_input_is_active(TextInput* input);
void text_input_activate(TextInput* input);
void text_input_deactivate(TextInput* input);
```

---

## Tips and Best Practices

1. **Always initialize inputs once** - Use a static bool flag to ensure initialization happens only once

2. **Handle events before other UI elements** - Text inputs should get first chance to handle events

3. **Use numeric mode for numbers** - Set min/max constraints to prevent invalid input

4. **Provide default values** - Use `text_input_set_text()` to set sensible defaults

5. **Check if input is active** - Use `text_input_is_active()` to determine when to process input

6. **Deactivate on Enter** - The component automatically deactivates when Enter is pressed

7. **Position carefully** - Leave space for labels (20 pixels above input field)

8. **Use consistent styling** - All inputs use the same visual style automatically

---

## Troubleshooting

### Input not responding to keyboard
- Make sure `text_input_handle_event()` is called in your event handler
- Verify the input is activated (click on it first)

### Text not rendering
- Ensure you're passing a valid TTF_Font pointer
- Check that the font is loaded before rendering

### Numeric validation not working
- Call `text_input_set_numeric()` after initialization
- Use `text_input_get_number()` instead of `text_input_get_text()`

### Input field not visible
- Check x, y coordinates are within window bounds
- Verify width and height are reasonable (e.g., 200x30)

---

## Future Enhancements

Potential improvements for the text input component:

1. Multi-line text support
2. Password masking
3. Auto-complete functionality
4. Copy/paste support
5. Undo/redo functionality
6. Custom styling options
7. Placeholder text
8. Input validation callbacks

---

## Conclusion

The text input component is production-ready and can be integrated into any tab that needs user text input. Follow the examples above to add input fields to your tabs.

For questions or issues, refer to the source code in `app/text_input.c` which is well-commented and easy to understand.