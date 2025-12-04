#ifndef CRYSTALLINE_V2_LIST_H
#define CRYSTALLINE_V2_LIST_H

#include "element.h"

/**
 * List callback types
 */
typedef void (*CrystallineListSelectCallback)(int index, void* user_data);
typedef void (*CrystallineListCheckCallback)(int index, bool checked, void* user_data);

/**
 * List element
 */
typedef struct {
    CrystallineElement base;
    
    // Items
    char** items;
    int item_count;
    int item_capacity;
    
    // Layout
    float item_height;
    int visible_items;
    
    // Selection
    int selected_index;
    int hover_index;
    
    // Checkboxes
    bool show_checkboxes;
    bool* item_checked;
    float checkbox_size;
    
    // Scrolling
    float scroll_offset;
    float scroll_max;
    
    // Callbacks
    CrystallineListSelectCallback on_select;
    CrystallineListCheckCallback on_check;
    void* callback_data;
    
    // Font
    TTF_Font* font;
    
    // Colors
    CrystallineColor item_color;
    CrystallineColor hover_color;
    CrystallineColor selected_color;
    CrystallineColor text_color;
    CrystallineColor checkbox_checked_color;
    CrystallineColor checkbox_unchecked_color;
} CrystallineList;

// ============================================================================
// LIST LIFECYCLE
// ============================================================================

/**
 * Create a list
 */
CrystallineList* crystalline_list_create(
    CrystallineBounds bounds,
    float item_height,
    bool show_checkboxes
);

/**
 * Destroy list
 */
void crystalline_list_destroy(CrystallineList* list);

// ============================================================================
// LIST CONFIGURATION
// ============================================================================

/**
 * Set list items
 */
void crystalline_list_set_items(
    CrystallineList* list,
    const char** items,
    int count
);

/**
 * Add item to list
 */
void crystalline_list_add_item(
    CrystallineList* list,
    const char* item
);

/**
 * Clear all items
 */
void crystalline_list_clear_items(CrystallineList* list);

/**
 * Set item checked state
 */
void crystalline_list_set_item_checked(
    CrystallineList* list,
    int index,
    bool checked
);

/**
 * Get item checked state
 */
bool crystalline_list_get_item_checked(
    CrystallineList* list,
    int index
);

/**
 * Set selection callback
 */
void crystalline_list_set_select_callback(
    CrystallineList* list,
    CrystallineListSelectCallback on_select,
    void* user_data
);

/**
 * Set checkbox callback
 */
void crystalline_list_set_check_callback(
    CrystallineList* list,
    CrystallineListCheckCallback on_check,
    void* user_data
);

/**
 * Set list font
 */
void crystalline_list_set_font(
    CrystallineList* list,
    TTF_Font* font
);

// ============================================================================
// SCROLLING
// ============================================================================

/**
 * Scroll list by delta
 */
void crystalline_list_scroll(
    CrystallineList* list,
    float delta
);

/**
 * Set scroll offset
 */
void crystalline_list_set_scroll_offset(
    CrystallineList* list,
    float offset
);

// ============================================================================
// RENDERING
// ============================================================================

/**
 * Render list
 */
void crystalline_list_render(
    CrystallineList* list,
    SDL_Renderer* renderer
);

// ============================================================================
// EVENT HANDLING
// ============================================================================

/**
 * Handle list events
 */
bool crystalline_list_handle_event(
    CrystallineList* list,
    SDL_Event* event
);

#endif // CRYSTALLINE_V2_LIST_H