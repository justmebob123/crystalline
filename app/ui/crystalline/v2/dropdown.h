#ifndef CRYSTALLINE_V2_DROPDOWN_H
#define CRYSTALLINE_V2_DROPDOWN_H

#include "element.h"

/**
 * Dropdown callback type
 */
typedef void (*CrystallineDropdownCallback)(int index, const char* option, void* user_data);

/**
 * Dropdown element
 */
typedef struct {
    CrystallineElement base;
    
    // Options
    char** options;
    int option_count;
    int option_capacity;
    
    // Selection
    int selected_index;
    int hover_index;
    
    // State
    bool expanded;
    
    // Visual
    float item_height;
    int max_visible_items;
    
    // Callback
    CrystallineDropdownCallback on_select;
    void* callback_data;
    
    // Font
    TTF_Font* font;
    
    // Colors
    CrystallineColor item_color;
    CrystallineColor hover_color;
    CrystallineColor selected_color;
    CrystallineColor text_color;
} CrystallineDropdown;

// ============================================================================
// DROPDOWN LIFECYCLE
// ============================================================================

/**
 * Create a dropdown
 */
CrystallineDropdown* crystalline_dropdown_create(
    CrystallineBounds bounds,
    CrystallineDropdownCallback on_select,
    void* user_data
);

/**
 * Destroy dropdown
 */
void crystalline_dropdown_destroy(CrystallineDropdown* dropdown);

// ============================================================================
// DROPDOWN CONFIGURATION
// ============================================================================

/**
 * Set dropdown options
 */
void crystalline_dropdown_set_options(
    CrystallineDropdown* dropdown,
    const char** options,
    int count
);

/**
 * Add option to dropdown
 */
void crystalline_dropdown_add_option(
    CrystallineDropdown* dropdown,
    const char* option
);

/**
 * Clear all options
 */
void crystalline_dropdown_clear_options(CrystallineDropdown* dropdown);

/**
 * Set selected index
 */
void crystalline_dropdown_set_selected(
    CrystallineDropdown* dropdown,
    int index
);

/**
 * Get selected index
 */
int crystalline_dropdown_get_selected(CrystallineDropdown* dropdown);

/**
 * Get selected option text
 */
const char* crystalline_dropdown_get_selected_text(CrystallineDropdown* dropdown);

/**
 * Set callback
 */
void crystalline_dropdown_set_callback(
    CrystallineDropdown* dropdown,
    CrystallineDropdownCallback on_select,
    void* user_data
);

/**
 * Set font
 */
void crystalline_dropdown_set_font(
    CrystallineDropdown* dropdown,
    TTF_Font* font
);

// ============================================================================
// RENDERING
// ============================================================================

/**
 * Render dropdown
 */
void crystalline_dropdown_render(
    CrystallineDropdown* dropdown,
    SDL_Renderer* renderer
);

// ============================================================================
// EVENT HANDLING
// ============================================================================

/**
 * Handle dropdown events
 */
bool crystalline_dropdown_handle_event(
    CrystallineDropdown* dropdown,
    SDL_Event* event
);

#endif // CRYSTALLINE_V2_DROPDOWN_H