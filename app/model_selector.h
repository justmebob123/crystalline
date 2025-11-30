// model_selector.h - Reusable Model Selector Widget
#ifndef MODEL_SELECTOR_H
#define MODEL_SELECTOR_H

#include "app_common.h"
#include <stdbool.h>

// Model selector state
typedef struct {
    int selected_index;
    bool show_dropdown;
    int dropdown_scroll;
    char selected_model_name[256];
} ModelSelectorState;

// Initialize model selector
void model_selector_init(ModelSelectorState* selector);

// Draw model selector widget
// Returns true if selection changed
bool model_selector_draw(AppState* state, ModelSelectorState* selector, 
                         int x, int y, int width, int height);

// Handle click on model selector
// Returns true if click was handled
bool model_selector_handle_click(AppState* state, ModelSelectorState* selector,
                                 int x, int y, int click_x, int click_y);

// Get currently selected model name
const char* model_selector_get_selected(ModelSelectorState* selector);

// Set selected model by name
void model_selector_set_selected(ModelSelectorState* selector, const char* name);

#endif // MODEL_SELECTOR_H