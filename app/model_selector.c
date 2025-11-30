// model_selector.c - Reusable Model Selector Widget Implementation
#include "model_selector.h"
#include "../include/cllm_model_manager.h"
#include <string.h>

#define DROPDOWN_ITEM_HEIGHT 30
#define MAX_VISIBLE_ITEMS 5

// Initialize model selector
void model_selector_init(ModelSelectorState* selector) {
    selector->selected_index = -1;
    selector->show_dropdown = false;
    selector->dropdown_scroll = 0;
    selector->selected_model_name[0] = '\0';
}

// Draw model selector widget
bool model_selector_draw(AppState* state, ModelSelectorState* selector, 
                        int x, int y, int width, int height) {
    (void)state; // Will be used for rendering
    
    bool selection_changed = false;
    
    // Draw selector button
    SDL_Rect button_rect = {x, y, width, height};
    SDL_SetRenderDrawColor(state->renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(state->renderer, &button_rect);
    
    // Draw border
    SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(state->renderer, &button_rect);
    
    // TODO: Render selected model name or "Select Model..."
    // TODO: Render dropdown arrow
    
    // Draw dropdown if shown
    if (selector->show_dropdown) {
        uint32_t model_count = 0;
        ManagedModel** models = model_manager_list(&model_count);
        
        int dropdown_height = (model_count < MAX_VISIBLE_ITEMS ? model_count : MAX_VISIBLE_ITEMS) 
                             * DROPDOWN_ITEM_HEIGHT;
        
        SDL_Rect dropdown_rect = {x, y + height, width, dropdown_height};
        SDL_SetRenderDrawColor(state->renderer, 40, 40, 40, 255);
        SDL_RenderFillRect(state->renderer, &dropdown_rect);
        
        // Draw border
        SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
        SDL_RenderDrawRect(state->renderer, &dropdown_rect);
        
        // Draw items
        for (uint32_t i = 0; i < model_count && i < MAX_VISIBLE_ITEMS; i++) {
            uint32_t model_idx = i + selector->dropdown_scroll;
            if (model_idx >= model_count) break;
            
            SDL_Rect item_rect = {
                x + 2,
                y + height + (int)i * DROPDOWN_ITEM_HEIGHT + 2,
                width - 4,
                DROPDOWN_ITEM_HEIGHT - 4
            };
            
            // Highlight if selected
            if ((int)model_idx == selector->selected_index) {
                SDL_SetRenderDrawColor(state->renderer, 60, 60, 100, 255);
            } else {
                SDL_SetRenderDrawColor(state->renderer, 50, 50, 50, 255);
            }
            SDL_RenderFillRect(state->renderer, &item_rect);
            
            // TODO: Render model name
        }
        
        if (models) free(models);
    }
    
    return selection_changed;
}

// Handle click on model selector
bool model_selector_handle_click(AppState* state, ModelSelectorState* selector,
                                int x, int y, int click_x, int click_y) {
    (void)state; // Suppress unused warning
    
    int width = 200; // Default width
    int height = 30; // Default height
    
    // Check if click is on button
    if (click_x >= x && click_x <= x + width &&
        click_y >= y && click_y <= y + height) {
        selector->show_dropdown = !selector->show_dropdown;
        return true;
    }
    
    // Check if click is on dropdown
    if (selector->show_dropdown) {
        uint32_t model_count = 0;
        ManagedModel** models = model_manager_list(&model_count);
        
        int dropdown_height = (model_count < MAX_VISIBLE_ITEMS ? model_count : MAX_VISIBLE_ITEMS) 
                             * DROPDOWN_ITEM_HEIGHT;
        
        if (click_x >= x && click_x <= x + width &&
            click_y >= y + height && click_y <= y + height + dropdown_height) {
            
            // Calculate which item was clicked
            int item_index = (click_y - (y + height)) / DROPDOWN_ITEM_HEIGHT;
            uint32_t model_idx = item_index + selector->dropdown_scroll;
            
            if (model_idx < model_count) {
                selector->selected_index = (int)model_idx;
                strncpy(selector->selected_model_name, models[model_idx]->name, 
                       sizeof(selector->selected_model_name) - 1);
                selector->selected_model_name[sizeof(selector->selected_model_name) - 1] = '\0';
                selector->show_dropdown = false;
                
                if (models) free(models);
                return true;
            }
        }
        
        if (models) free(models);
    }
    
    // Click outside dropdown - close it
    if (selector->show_dropdown) {
        selector->show_dropdown = false;
        return true;
    }
    
    return false;
}

// Get currently selected model name
const char* model_selector_get_selected(ModelSelectorState* selector) {
    if (selector->selected_index < 0 || selector->selected_model_name[0] == '\0') {
        return NULL;
    }
    return selector->selected_model_name;
}

// Set selected model by name
void model_selector_set_selected(ModelSelectorState* selector, const char* name) {
    if (!name) {
        selector->selected_index = -1;
        selector->selected_model_name[0] = '\0';
        return;
    }
    
    uint32_t model_count = 0;
    ManagedModel** models = model_manager_list(&model_count);
    
    for (uint32_t i = 0; i < model_count; i++) {
        if (strcmp(models[i]->name, name) == 0) {
            selector->selected_index = (int)i;
            strncpy(selector->selected_model_name, name, 
                   sizeof(selector->selected_model_name) - 1);
            selector->selected_model_name[sizeof(selector->selected_model_name) - 1] = '\0';
            break;
        }
    }
    
    if (models) free(models);
}