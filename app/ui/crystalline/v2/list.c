#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void list_render_func(CrystallineElement* element, SDL_Renderer* renderer);
static bool list_handle_event_func(CrystallineElement* element, SDL_Event* event);
static void list_destroy_func(CrystallineElement* element);

// ============================================================================
// LIST LIFECYCLE
// ============================================================================

CrystallineList* crystalline_list_create(
    CrystallineBounds bounds,
    float item_height,
    bool show_checkboxes
) {
    CrystallineList* list = (CrystallineList*)calloc(1, sizeof(CrystallineList));
    if (!list) return NULL;
    
    // Initialize base element
    CrystallineElement* base = &list->base;
    base->type = CRYSTALLINE_ELEMENT_LIST;
    base->bounds = bounds;
    base->padding = crystalline_padding_uniform(5.0f);
    base->visible = true;
    base->enabled = true;
    base->state = CRYSTALLINE_STATE_NORMAL;
    
    // Set colors
    base->bg_color = crystalline_color(30, 30, 40, 255);
    base->border_color = crystalline_color(100, 100, 120, 255);
    base->border_width = 2.0f;
    
    // Set callbacks
    base->render = list_render_func;
    base->handle_event = list_handle_event_func;
    base->destroy = list_destroy_func;
    
    // Initialize list-specific fields
    list->items = NULL;
    list->item_count = 0;
    list->item_capacity = 0;
    list->item_height = item_height;
    list->visible_items = (int)((bounds.height - 10.0f) / item_height);  // Account for padding
    list->selected_index = -1;
    list->hover_index = -1;
    list->show_checkboxes = show_checkboxes;
    list->item_checked = NULL;
    list->checkbox_size = 12.0f;
    list->scroll_offset = 0.0f;
    list->scroll_max = 0.0f;
    list->on_select = NULL;
    list->on_check = NULL;
    list->callback_data = NULL;
    list->font = NULL;
    
    // Set default colors
    list->item_color = crystalline_color(60, 60, 80, 255);
    list->hover_color = crystalline_color(80, 80, 100, 255);
    list->selected_color = crystalline_color(100, 150, 200, 255);
    list->text_color = crystalline_color(220, 220, 220, 255);
    list->checkbox_checked_color = crystalline_color(100, 255, 100, 255);
    list->checkbox_unchecked_color = crystalline_color(80, 80, 80, 255);
    
    return list;
}

void crystalline_list_destroy(CrystallineList* list) {
    if (!list) return;
    crystalline_list_clear_items(list);
    crystalline_element_destroy(&list->base);
}

// ============================================================================
// LIST CONFIGURATION
// ============================================================================

void crystalline_list_set_items(
    CrystallineList* list,
    const char** items,
    int count
) {
    if (!list) return;
    
    // Clear existing items
    crystalline_list_clear_items(list);
    
    // Allocate arrays
    list->items = (char**)calloc(count, sizeof(char*));
    list->item_checked = (bool*)calloc(count, sizeof(bool));
    if (!list->items || !list->item_checked) return;
    
    list->item_capacity = count;
    list->item_count = count;
    
    // Copy items
    for (int i = 0; i < count; i++) {
        list->items[i] = items[i] ? strdup(items[i]) : NULL;
        list->item_checked[i] = false;
    }
    
    // Update scroll max
    float total_height = count * list->item_height;
    float visible_height = list->base.bounds.height - 10.0f;  // Account for padding
    list->scroll_max = fmaxf(0.0f, total_height - visible_height);
}

void crystalline_list_add_item(
    CrystallineList* list,
    const char* item
) {
    if (!list) return;
    
    // Grow arrays if needed
    if (list->item_count >= list->item_capacity) {
        int new_capacity = list->item_capacity == 0 ? 4 : list->item_capacity * 2;
        
        char** new_items = (char**)realloc(list->items, new_capacity * sizeof(char*));
        bool* new_checked = (bool*)realloc(list->item_checked, new_capacity * sizeof(bool));
        
        if (!new_items || !new_checked) return;
        
        list->items = new_items;
        list->item_checked = new_checked;
        list->item_capacity = new_capacity;
    }
    
    // Add item
    list->items[list->item_count] = item ? strdup(item) : NULL;
    list->item_checked[list->item_count] = false;
    list->item_count++;
    
    // Update scroll max
    float total_height = list->item_count * list->item_height;
    float visible_height = list->base.bounds.height - 10.0f;
    list->scroll_max = fmaxf(0.0f, total_height - visible_height);
}

void crystalline_list_clear_items(CrystallineList* list) {
    if (!list) return;
    
    // Free item strings
    for (int i = 0; i < list->item_count; i++) {
        if (list->items[i]) {
            free(list->items[i]);
        }
    }
    
    // Free arrays
    if (list->items) {
        free(list->items);
        list->items = NULL;
    }
    if (list->item_checked) {
        free(list->item_checked);
        list->item_checked = NULL;
    }
    
    list->item_count = 0;
    list->item_capacity = 0;
    list->scroll_offset = 0.0f;
    list->scroll_max = 0.0f;
}

void crystalline_list_set_item_checked(
    CrystallineList* list,
    int index,
    bool checked
) {
    if (!list || index < 0 || index >= list->item_count || !list->item_checked) return;
    list->item_checked[index] = checked;
}

bool crystalline_list_get_item_checked(
    CrystallineList* list,
    int index
) {
    if (!list || index < 0 || index >= list->item_count || !list->item_checked) {
        return false;
    }
    return list->item_checked[index];
}

void crystalline_list_set_select_callback(
    CrystallineList* list,
    CrystallineListSelectCallback on_select,
    void* user_data
) {
    if (!list) return;
    list->on_select = on_select;
    list->callback_data = user_data;
}

void crystalline_list_set_check_callback(
    CrystallineList* list,
    CrystallineListCheckCallback on_check,
    void* user_data
) {
    if (!list) return;
    list->on_check = on_check;
    list->callback_data = user_data;
}

void crystalline_list_set_font(
    CrystallineList* list,
    TTF_Font* font
) {
    if (!list) return;
    list->font = font;
}

// ============================================================================
// SCROLLING
// ============================================================================

void crystalline_list_scroll(
    CrystallineList* list,
    float delta
) {
    if (!list) return;
    crystalline_list_set_scroll_offset(list, list->scroll_offset + delta);
}

void crystalline_list_set_scroll_offset(
    CrystallineList* list,
    float offset
) {
    if (!list) return;
    list->scroll_offset = fmaxf(0.0f, fminf(offset, list->scroll_max));
}

// ============================================================================
// RENDERING
// ============================================================================

static void draw_circle_filled(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

static void list_render_func(CrystallineElement* element, SDL_Renderer* renderer) {
    CrystallineList* list = (CrystallineList*)element;
    
    // Get content bounds (with padding)
    CrystallineBounds content = crystalline_element_get_content_bounds(element);
    CrystallinePoint content_top_left = crystalline_bounds_top_left(content);
    
    // Calculate visible items
    int start_index = (int)(list->scroll_offset / list->item_height);
    int end_index = start_index + list->visible_items;
    if (end_index > list->item_count) end_index = list->item_count;
    
    // Calculate starting Y position
    float item_y_start = content_top_left.y - (start_index * list->item_height - list->scroll_offset);
    
    // Render items
    for (int i = start_index; i < end_index; i++) {
        if (!list->items[i]) continue;
        
        float item_y = item_y_start + (i - start_index) * list->item_height;
        float item_center_y = item_y + list->item_height / 2.0f;
        
        // Determine item color
        CrystallineColor item_color;
        if (i == list->selected_index) {
            item_color = list->selected_color;
        } else if (i == list->hover_index) {
            item_color = list->hover_color;
        } else {
            item_color = list->item_color;
        }
        
        // Draw item background
        SDL_Rect item_rect = {
            (int)content_top_left.x,
            (int)item_y,
            (int)content.width,
            (int)list->item_height
        };
        SDL_SetRenderDrawColor(renderer, item_color.r, item_color.g, item_color.b, item_color.a);
        SDL_RenderFillRect(renderer, &item_rect);
        
        // Draw checkbox if enabled
        float text_x = content_top_left.x + 5.0f;
        if (list->show_checkboxes && list->item_checked) {
            float checkbox_x = content_top_left.x + 15.0f;
            int checkbox_cx = (int)checkbox_x;
            int checkbox_cy = (int)item_center_y;
            
            CrystallineColor checkbox_color = list->item_checked[i] ? 
                list->checkbox_checked_color : list->checkbox_unchecked_color;
            
            draw_circle_filled(renderer, checkbox_cx, checkbox_cy, 
                             (int)list->checkbox_size, crystalline_color_to_sdl(checkbox_color));
            
            text_x = checkbox_x + list->checkbox_size + 10.0f;
        }
        
        // Draw item text
        if (list->font) {
            SDL_Color text_color = crystalline_color_to_sdl(list->text_color);
            SDL_Surface* surface = TTF_RenderText_Blended(list->font, list->items[i], text_color);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect dest = {
                        (int)text_x,
                        (int)(item_center_y - surface->h / 2),
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

void crystalline_list_render(
    CrystallineList* list,
    SDL_Renderer* renderer
) {
    if (!list) return;
    crystalline_element_render(&list->base, renderer);
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

static bool list_handle_event_func(CrystallineElement* element, SDL_Event* event) {
    CrystallineList* list = (CrystallineList*)element;
    
    if (!element->enabled) return false;
    
    // Handle scroll wheel
    if (event->type == SDL_MOUSEWHEEL) {
        if (crystalline_element_contains_point(element, event->wheel.mouseX, event->wheel.mouseY)) {
            crystalline_list_scroll(list, -event->wheel.y * list->item_height);
            return true;
        }
    }
    
    // Handle mouse motion (hover)
    if (event->type == SDL_MOUSEMOTION) {
        if (crystalline_element_contains_point(element, event->motion.x, event->motion.y)) {
            CrystallineBounds content = crystalline_element_get_content_bounds(element);
            CrystallinePoint content_top_left = crystalline_bounds_top_left(content);
            
            float relative_y = event->motion.y - content_top_left.y + list->scroll_offset;
            int index = (int)(relative_y / list->item_height);
            
            if (index >= 0 && index < list->item_count) {
                list->hover_index = index;
            } else {
                list->hover_index = -1;
            }
            return true;
        } else {
            list->hover_index = -1;
        }
    }
    
    // Handle mouse click
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (list->hover_index >= 0) {
            // Check if click is on checkbox
            if (list->show_checkboxes && list->item_checked) {
                CrystallineBounds content = crystalline_element_get_content_bounds(element);
                CrystallinePoint content_top_left = crystalline_bounds_top_left(content);
                
                float item_y_start = content_top_left.y - list->scroll_offset;
                float item_y = item_y_start + list->hover_index * list->item_height;
                float item_center_y = item_y + list->item_height / 2.0f;
                
                float checkbox_x = content_top_left.x + 15.0f;
                float dx = event->button.x - checkbox_x;
                float dy = event->button.y - item_center_y;
                float dist = sqrtf(dx*dx + dy*dy);
                
                if (dist <= list->checkbox_size + 5.0f) {
                    // Toggle checkbox
                    list->item_checked[list->hover_index] = !list->item_checked[list->hover_index];
                    
                    if (list->on_check) {
                        list->on_check(list->hover_index, list->item_checked[list->hover_index], 
                                     list->callback_data);
                    }
                    return true;
                }
            }
            
            // Regular item selection
            list->selected_index = list->hover_index;
            if (list->on_select) {
                list->on_select(list->selected_index, list->callback_data);
            }
            return true;
        }
    }
    
    return false;
}

bool crystalline_list_handle_event(
    CrystallineList* list,
    SDL_Event* event
) {
    if (!list) return false;
    return crystalline_element_handle_event(&list->base, event);
}

// ============================================================================
// CLEANUP
// ============================================================================

static void list_destroy_func(CrystallineElement* element) {
    CrystallineList* list = (CrystallineList*)element;
    crystalline_list_clear_items(list);
}