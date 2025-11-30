/**
 * Model Selector UI Component
 * 
 * Dropdown for selecting models across all tabs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "ui/model_selector.h"
#include "cllm_model_manager.h"

#define MAX_MODELS 32
#define DROPDOWN_ITEM_HEIGHT 30
#define MAX_VISIBLE_ITEMS 10

struct ModelSelector {
    int x, y, width, height;
    char selected_model[256];
    char** model_list;
    int num_models;
    int dropdown_open;
    int scroll_offset;
    void (*on_change)(const char* model_name, void* user_data);
    void* user_data;
    
    // Visual state
    int hover_index;
    SDL_Color bg_color;
    SDL_Color text_color;
    SDL_Color hover_color;
    SDL_Color border_color;
};

/**
 * Create model selector
 */
ModelSelector* model_selector_create(int x, int y, int width, int height) {
    ModelSelector* selector = calloc(1, sizeof(ModelSelector));
    if (!selector) return NULL;
    
    selector->x = x;
    selector->y = y;
    selector->width = width;
    selector->height = height;
    selector->dropdown_open = 0;
    selector->scroll_offset = 0;
    selector->hover_index = -1;
    
    // Default colors
    selector->bg_color = (SDL_Color){40, 40, 40, 255};
    selector->text_color = (SDL_Color){220, 220, 220, 255};
    selector->hover_color = (SDL_Color){60, 60, 60, 255};
    selector->border_color = (SDL_Color){100, 100, 100, 255};
    
    // Allocate model list
    selector->model_list = calloc(MAX_MODELS, sizeof(char*));
    if (!selector->model_list) {
        free(selector);
        return NULL;
    }
    
    // Initialize with empty selection
    strcpy(selector->selected_model, "No model selected");
    
    return selector;
}

/**
 * Update model list from model manager
 */
void model_selector_update_list(ModelSelector* selector) {
    if (!selector) return;
    
    // Free old list
    for (int i = 0; i < selector->num_models; i++) {
        free(selector->model_list[i]);
    }
    selector->num_models = 0;
    
    // Get models from model manager
    uint32_t count = 0;
    ManagedModel** models = model_manager_list(&count);
    
    if (models && count > 0) {
        for (uint32_t i = 0; i < count && i < MAX_MODELS; i++) {
            if (models[i]) {
                selector->model_list[i] = strdup(models[i]->name);
                selector->num_models++;
            }
        }
        
        // Set first model as selected if none selected
        if (strcmp(selector->selected_model, "No model selected") == 0 && count > 0) {
            if (models[0]) {
                strncpy(selector->selected_model, models[0]->name, 255);
                selector->selected_model[255] = '\0';
            }
        }
    }
}

/**
 * Render model selector
 */
void model_selector_render(ModelSelector* selector, SDL_Renderer* renderer) {
    if (!selector || !renderer) return;
    
    // Draw main button
    SDL_Rect button_rect = {selector->x, selector->y, selector->width, selector->height};
    
    // Background
    SDL_SetRenderDrawColor(renderer, 
                          selector->bg_color.r, 
                          selector->bg_color.g, 
                          selector->bg_color.b, 
                          selector->bg_color.a);
    SDL_RenderFillRect(renderer, &button_rect);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 
                          selector->border_color.r, 
                          selector->border_color.g, 
                          selector->border_color.b, 
                          selector->border_color.a);
    SDL_RenderDrawRect(renderer, &button_rect);
    
    // Text (selected model name)
    // Note: Actual text rendering would use SDL_ttf
    // For now, just draw the dropdown indicator
    
    // Draw dropdown arrow
    int arrow_x = selector->x + selector->width - 20;
    int arrow_y = selector->y + selector->height / 2;
    SDL_SetRenderDrawColor(renderer, 
                          selector->text_color.r, 
                          selector->text_color.g, 
                          selector->text_color.b, 
                          selector->text_color.a);
    
    if (selector->dropdown_open) {
        // Up arrow
        SDL_RenderDrawLine(renderer, arrow_x - 5, arrow_y, arrow_x, arrow_y - 5);
        SDL_RenderDrawLine(renderer, arrow_x, arrow_y - 5, arrow_x + 5, arrow_y);
    } else {
        // Down arrow
        SDL_RenderDrawLine(renderer, arrow_x - 5, arrow_y - 5, arrow_x, arrow_y);
        SDL_RenderDrawLine(renderer, arrow_x, arrow_y, arrow_x + 5, arrow_y - 5);
    }
    
    // Draw dropdown list if open
    if (selector->dropdown_open && selector->num_models > 0) {
        int dropdown_height = selector->num_models * DROPDOWN_ITEM_HEIGHT;
        if (dropdown_height > MAX_VISIBLE_ITEMS * DROPDOWN_ITEM_HEIGHT) {
            dropdown_height = MAX_VISIBLE_ITEMS * DROPDOWN_ITEM_HEIGHT;
        }
        
        SDL_Rect dropdown_rect = {
            selector->x, 
            selector->y + selector->height, 
            selector->width, 
            dropdown_height
        };
        
        // Background
        SDL_SetRenderDrawColor(renderer, 
                              selector->bg_color.r, 
                              selector->bg_color.g, 
                              selector->bg_color.b, 
                              selector->bg_color.a);
        SDL_RenderFillRect(renderer, &dropdown_rect);
        
        // Border
        SDL_SetRenderDrawColor(renderer, 
                              selector->border_color.r, 
                              selector->border_color.g, 
                              selector->border_color.b, 
                              selector->border_color.a);
        SDL_RenderDrawRect(renderer, &dropdown_rect);
        
        // Draw items
        int visible_items = (dropdown_height / DROPDOWN_ITEM_HEIGHT);
        for (int i = 0; i < visible_items && (i + selector->scroll_offset) < selector->num_models; i++) {
            int item_index = i + selector->scroll_offset;
            SDL_Rect item_rect = {
                selector->x,
                selector->y + selector->height + (i * DROPDOWN_ITEM_HEIGHT),
                selector->width,
                DROPDOWN_ITEM_HEIGHT
            };
            
            // Highlight hovered item
            if (item_index == selector->hover_index) {
                SDL_SetRenderDrawColor(renderer, 
                                      selector->hover_color.r, 
                                      selector->hover_color.g, 
                                      selector->hover_color.b, 
                                      selector->hover_color.a);
                SDL_RenderFillRect(renderer, &item_rect);
            }
            
            // Item text would be rendered here with SDL_ttf
        }
    }
}

/**
 * Handle click
 */
int model_selector_handle_click(ModelSelector* selector, int mouse_x, int mouse_y) {
    if (!selector) return 0;
    
    // Check if clicked on main button
    if (mouse_x >= selector->x && mouse_x <= selector->x + selector->width &&
        mouse_y >= selector->y && mouse_y <= selector->y + selector->height) {
        
        // Toggle dropdown
        selector->dropdown_open = !selector->dropdown_open;
        
        // Update list when opening
        if (selector->dropdown_open) {
            model_selector_update_list(selector);
        }
        
        return 1;
    }
    
    // Check if clicked on dropdown item
    if (selector->dropdown_open && selector->num_models > 0) {
        int dropdown_height = selector->num_models * DROPDOWN_ITEM_HEIGHT;
        if (dropdown_height > MAX_VISIBLE_ITEMS * DROPDOWN_ITEM_HEIGHT) {
            dropdown_height = MAX_VISIBLE_ITEMS * DROPDOWN_ITEM_HEIGHT;
        }
        
        if (mouse_x >= selector->x && mouse_x <= selector->x + selector->width &&
            mouse_y >= selector->y + selector->height && 
            mouse_y <= selector->y + selector->height + dropdown_height) {
            
            // Calculate clicked item
            int item_y = mouse_y - (selector->y + selector->height);
            int item_index = (item_y / DROPDOWN_ITEM_HEIGHT) + selector->scroll_offset;
            
            if (item_index >= 0 && item_index < selector->num_models) {
                // Select this model
                strncpy(selector->selected_model, selector->model_list[item_index], 255);
                selector->selected_model[255] = '\0';
                
                // Call callback
                if (selector->on_change) {
                    selector->on_change(selector->selected_model, selector->user_data);
                }
                
                // Close dropdown
                selector->dropdown_open = 0;
                
                return 1;
            }
        }
    }
    
    // Click outside - close dropdown
    if (selector->dropdown_open) {
        selector->dropdown_open = 0;
        return 1;
    }
    
    return 0;
}

/**
 * Handle mouse motion (for hover effects)
 */
void model_selector_handle_motion(ModelSelector* selector, int mouse_x, int mouse_y) {
    if (!selector || !selector->dropdown_open) return;
    
    selector->hover_index = -1;
    
    int dropdown_height = selector->num_models * DROPDOWN_ITEM_HEIGHT;
    if (dropdown_height > MAX_VISIBLE_ITEMS * DROPDOWN_ITEM_HEIGHT) {
        dropdown_height = MAX_VISIBLE_ITEMS * DROPDOWN_ITEM_HEIGHT;
    }
    
    if (mouse_x >= selector->x && mouse_x <= selector->x + selector->width &&
        mouse_y >= selector->y + selector->height && 
        mouse_y <= selector->y + selector->height + dropdown_height) {
        
        int item_y = mouse_y - (selector->y + selector->height);
        int item_index = (item_y / DROPDOWN_ITEM_HEIGHT) + selector->scroll_offset;
        
        if (item_index >= 0 && item_index < selector->num_models) {
            selector->hover_index = item_index;
        }
    }
}

/**
 * Get selected model
 */
const char* model_selector_get_selected(ModelSelector* selector) {
    if (!selector) return NULL;
    return selector->selected_model;
}

/**
 * Set selected model
 */
void model_selector_set_selected(ModelSelector* selector, const char* model_name) {
    if (!selector || !model_name) return;
    
    strncpy(selector->selected_model, model_name, 255);
    selector->selected_model[255] = '\0';
}

/**
 * Set change callback
 */
void model_selector_set_callback(ModelSelector* selector, 
                                void (*callback)(const char*, void*), 
                                void* user_data) {
    if (!selector) return;
    selector->on_change = callback;
    selector->user_data = user_data;
}

/**
 * Destroy model selector
 */
void model_selector_destroy(ModelSelector* selector) {
    if (!selector) return;
    
    // Free model list
    for (int i = 0; i < selector->num_models; i++) {
        free(selector->model_list[i]);
    }
    free(selector->model_list);
    
    free(selector);
}