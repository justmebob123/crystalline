#include "dropdown.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void dropdown_render_func(CrystallineElement* element, SDL_Renderer* renderer);
static bool dropdown_handle_event_func(CrystallineElement* element, SDL_Event* event);
static void dropdown_destroy_func(CrystallineElement* element);

// ============================================================================
// DROPDOWN LIFECYCLE
// ============================================================================

CrystallineDropdown* crystalline_dropdown_create(
    CrystallineBounds bounds,
    CrystallineDropdownCallback on_select,
    void* user_data
) {
    CrystallineDropdown* dropdown = (CrystallineDropdown*)calloc(1, sizeof(CrystallineDropdown));
    if (!dropdown) return NULL;
    
    // Initialize base element
    CrystallineElement* base = &dropdown->base;
    base->type = CRYSTALLINE_ELEMENT_DROPDOWN;
    base->bounds = bounds;
    base->padding = crystalline_padding_uniform(5.0f);
    base->visible = true;
    base->enabled = true;
    base->state = CRYSTALLINE_STATE_NORMAL;
    
    // Set colors
    base->bg_color = crystalline_color(40, 40, 50, 255);
    base->border_color = crystalline_color(100, 100, 120, 255);
    base->border_width = 2.0f;
    
    // Set callbacks
    base->render = dropdown_render_func;
    base->handle_event = dropdown_handle_event_func;
    base->destroy = dropdown_destroy_func;
    
    // Initialize dropdown-specific fields
    dropdown->options = NULL;
    dropdown->option_count = 0;
    dropdown->option_capacity = 0;
    dropdown->selected_index = -1;
    dropdown->hover_index = -1;
    dropdown->expanded = false;
    dropdown->item_height = 30.0f;
    dropdown->max_visible_items = 5;
    dropdown->on_select = on_select;
    dropdown->callback_data = user_data;
    dropdown->font = NULL;
    
    // Set default colors
    dropdown->item_color = crystalline_color(60, 60, 80, 255);
    dropdown->hover_color = crystalline_color(80, 80, 100, 255);
    dropdown->selected_color = crystalline_color(100, 150, 200, 255);
    dropdown->text_color = crystalline_color(220, 220, 220, 255);
    
    return dropdown;
}

void crystalline_dropdown_destroy(CrystallineDropdown* dropdown) {
    if (!dropdown) return;
    crystalline_dropdown_clear_options(dropdown);
    crystalline_element_destroy(&dropdown->base);
}

// ============================================================================
// DROPDOWN CONFIGURATION
// ============================================================================

void crystalline_dropdown_set_options(
    CrystallineDropdown* dropdown,
    const char** options,
    int count
) {
    if (!dropdown) return;
    
    // Clear existing options
    crystalline_dropdown_clear_options(dropdown);
    
    // Allocate array
    dropdown->options = (char**)calloc(count, sizeof(char*));
    if (!dropdown->options) return;
    
    dropdown->option_capacity = count;
    dropdown->option_count = count;
    
    // Copy options
    for (int i = 0; i < count; i++) {
        dropdown->options[i] = options[i] ? strdup(options[i]) : NULL;
    }
}

void crystalline_dropdown_add_option(
    CrystallineDropdown* dropdown,
    const char* option
) {
    if (!dropdown) return;
    
    // Grow array if needed
    if (dropdown->option_count >= dropdown->option_capacity) {
        int new_capacity = dropdown->option_capacity == 0 ? 4 : dropdown->option_capacity * 2;
        char** new_options = (char**)realloc(dropdown->options, new_capacity * sizeof(char*));
        if (!new_options) return;
        
        dropdown->options = new_options;
        dropdown->option_capacity = new_capacity;
    }
    
    // Add option
    dropdown->options[dropdown->option_count++] = option ? strdup(option) : NULL;
}

void crystalline_dropdown_clear_options(CrystallineDropdown* dropdown) {
    if (!dropdown) return;
    
    // Free option strings
    for (int i = 0; i < dropdown->option_count; i++) {
        if (dropdown->options[i]) {
            free(dropdown->options[i]);
        }
    }
    
    // Free array
    if (dropdown->options) {
        free(dropdown->options);
        dropdown->options = NULL;
    }
    
    dropdown->option_count = 0;
    dropdown->option_capacity = 0;
    dropdown->selected_index = -1;
    dropdown->hover_index = -1;
}

void crystalline_dropdown_set_selected(
    CrystallineDropdown* dropdown,
    int index
) {
    if (!dropdown || index < -1 || index >= dropdown->option_count) return;
    
    int old_index = dropdown->selected_index;
    dropdown->selected_index = index;
    
    // Trigger callback if selection changed
    if (old_index != index && dropdown->on_select && index >= 0) {
        dropdown->on_select(index, dropdown->options[index], dropdown->callback_data);
    }
}

int crystalline_dropdown_get_selected(CrystallineDropdown* dropdown) {
    return dropdown ? dropdown->selected_index : -1;
}

const char* crystalline_dropdown_get_selected_text(CrystallineDropdown* dropdown) {
    if (!dropdown || dropdown->selected_index < 0 || dropdown->selected_index >= dropdown->option_count) {
        return NULL;
    }
    return dropdown->options[dropdown->selected_index];
}

void crystalline_dropdown_set_callback(
    CrystallineDropdown* dropdown,
    CrystallineDropdownCallback on_select,
    void* user_data
) {
    if (!dropdown) return;
    
    dropdown->on_select = on_select;
    dropdown->callback_data = user_data;
}

void crystalline_dropdown_set_font(
    CrystallineDropdown* dropdown,
    TTF_Font* font
) {
    if (!dropdown) return;
    dropdown->font = font;
}

// ============================================================================
// RENDERING
// ============================================================================

static void dropdown_render_func(CrystallineElement* element, SDL_Renderer* renderer) {
    CrystallineDropdown* dropdown = (CrystallineDropdown*)element;
    
    CrystallinePoint top_left = crystalline_bounds_top_left(element->bounds);
    
    // Draw main dropdown box
    SDL_Rect main_rect = {
        (int)top_left.x,
        (int)top_left.y,
        (int)element->bounds.width,
        (int)element->bounds.height
    };
    
    SDL_Color bg_color = crystalline_color_to_sdl(element->bg_color);
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderFillRect(renderer, &main_rect);
    
    SDL_Color border_color = crystalline_color_to_sdl(element->border_color);
    SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, border_color.a);
    SDL_RenderDrawRect(renderer, &main_rect);
    
    // Draw selected text
    if (dropdown->selected_index >= 0 && dropdown->font) {
        const char* text = dropdown->options[dropdown->selected_index];
        SDL_Color text_color = crystalline_color_to_sdl(dropdown->text_color);
        SDL_Surface* surface = TTF_RenderText_Blended(dropdown->font, text, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {
                    (int)(element->bounds.center.x - surface->w / 2),
                    (int)(element->bounds.center.y - surface->h / 2),
                    surface->w,
                    surface->h
                };
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    
    // Draw dropdown arrow
    int arrow_x = (int)(top_left.x + element->bounds.width - 20);
    int arrow_y = (int)element->bounds.center.y;
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (int i = 0; i < 5; i++) {
        SDL_RenderDrawLine(renderer, arrow_x - i, arrow_y - 2 + i, arrow_x + i, arrow_y - 2 + i);
    }
    
    // Draw expanded options
    if (dropdown->expanded && dropdown->option_count > 0) {
        int visible_count = dropdown->option_count < dropdown->max_visible_items ? 
                           dropdown->option_count : dropdown->max_visible_items;
        
        float options_y = top_left.y + element->bounds.height;
        
        for (int i = 0; i < visible_count; i++) {
            if (!dropdown->options[i]) continue;
            
            float item_y = options_y + i * dropdown->item_height;
            
            // Determine item color
            CrystallineColor item_color;
            if (i == dropdown->hover_index) {
                item_color = dropdown->hover_color;
            } else if (i == dropdown->selected_index) {
                item_color = dropdown->selected_color;
            } else {
                item_color = dropdown->item_color;
            }
            
            // Draw item background
            SDL_Rect item_rect = {
                (int)top_left.x,
                (int)item_y,
                (int)element->bounds.width,
                (int)dropdown->item_height
            };
            SDL_Color item_sdl = crystalline_color_to_sdl(item_color);
            SDL_SetRenderDrawColor(renderer, item_sdl.r, item_sdl.g, item_sdl.b, item_sdl.a);
            SDL_RenderFillRect(renderer, &item_rect);
            
            // Draw item border
            SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, border_color.a);
            SDL_RenderDrawRect(renderer, &item_rect);
            
            // Draw item text
            if (dropdown->font) {
                SDL_Color text_color = crystalline_color_to_sdl(dropdown->text_color);
                SDL_Surface* surface = TTF_RenderText_Blended(dropdown->font, dropdown->options[i], text_color);
                if (surface) {
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                    if (texture) {
                        SDL_Rect dest = {
                            (int)(top_left.x + 10),
                            (int)(item_y + dropdown->item_height / 2 - surface->h / 2),
                            surface->w,
                            surface->h
                        };
                        SDL_RenderCopy(renderer, texture, NULL, &dest);
                        SDL_DestroyTexture(texture);
                    }
                    SDL_FreeSurface(surface);
                }
            }
        }
    }
}

void crystalline_dropdown_render(
    CrystallineDropdown* dropdown,
    SDL_Renderer* renderer
) {
    if (!dropdown) return;
    crystalline_element_render(&dropdown->base, renderer);
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

static bool dropdown_handle_event_func(CrystallineElement* element, SDL_Event* event) {
    CrystallineDropdown* dropdown = (CrystallineDropdown*)element;
    
    if (!element->enabled) return false;
    
    CrystallinePoint top_left = crystalline_bounds_top_left(element->bounds);
    
    if (event->type == SDL_MOUSEMOTION) {
        if (dropdown->expanded) {
            // Update hover index for expanded options
            float options_y = top_left.y + element->bounds.height;
            int visible_count = dropdown->option_count < dropdown->max_visible_items ? 
                               dropdown->option_count : dropdown->max_visible_items;
            
            dropdown->hover_index = -1;
            for (int i = 0; i < visible_count; i++) {
                float item_y = options_y + i * dropdown->item_height;
                if (event->motion.y >= item_y && event->motion.y < item_y + dropdown->item_height &&
                    event->motion.x >= top_left.x && event->motion.x < top_left.x + element->bounds.width) {
                    dropdown->hover_index = i;
                    break;
                }
            }
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (dropdown->expanded) {
            // Check if clicking on an option
            if (dropdown->hover_index >= 0) {
                crystalline_dropdown_set_selected(dropdown, dropdown->hover_index);
                dropdown->expanded = false;
                dropdown->hover_index = -1;
                return true;
            }
            // Click outside - close dropdown
            dropdown->expanded = false;
            dropdown->hover_index = -1;
            return true;
        } else {
            // Check if clicking on main dropdown box
            if (crystalline_element_contains_point(element, event->button.x, event->button.y)) {
                dropdown->expanded = true;
                return true;
            }
        }
    }
    
    return false;
}

bool crystalline_dropdown_handle_event(
    CrystallineDropdown* dropdown,
    SDL_Event* event
) {
    if (!dropdown) return false;
    return crystalline_element_handle_event(&dropdown->base, event);
}

// ============================================================================
// CLEANUP
// ============================================================================

static void dropdown_destroy_func(CrystallineElement* element) {
    CrystallineDropdown* dropdown = (CrystallineDropdown*)element;
    crystalline_dropdown_clear_options(dropdown);
}