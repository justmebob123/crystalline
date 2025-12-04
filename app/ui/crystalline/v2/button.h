#ifndef CRYSTALLINE_V2_BUTTON_H
#define CRYSTALLINE_V2_BUTTON_H

#include "element.h"

/**
 * Button callback type
 */
typedef void (*CrystallineButtonCallback)(void* user_data);

/**
 * Button element
 */
typedef struct {
    CrystallineElement base;
    
    // Label
    const char* label;
    TTF_Font* font;
    
    // Size
    CrystallineSize size;
    bool circular;  // Circular or rectangular
    
    // Callback
    CrystallineButtonCallback on_click;
    void* callback_data;
    
    // Colors
    CrystallineColor normal_color;
    CrystallineColor hover_color;
    CrystallineColor active_color;
    CrystallineColor disabled_color;
    CrystallineColor text_color;
} CrystallineButton;

// ============================================================================
// BUTTON LIFECYCLE
// ============================================================================

/**
 * Create a button with automatic sizing
 */
CrystallineButton* crystalline_button_create(
    const char* label,
    CrystallineSize size,
    bool circular,
    CrystallineButtonCallback on_click,
    void* user_data
);

/**
 * Create a button with custom bounds
 */
CrystallineButton* crystalline_button_create_custom(
    const char* label,
    CrystallineBounds bounds,
    bool circular,
    CrystallineButtonCallback on_click,
    void* user_data
);

/**
 * Destroy button
 */
void crystalline_button_destroy(CrystallineButton* button);

// ============================================================================
// BUTTON CONFIGURATION
// ============================================================================

/**
 * Set button label
 */
void crystalline_button_set_label(
    CrystallineButton* button,
    const char* label
);

/**
 * Set button callback
 */
void crystalline_button_set_callback(
    CrystallineButton* button,
    CrystallineButtonCallback on_click,
    void* user_data
);

/**
 * Set button colors
 */
void crystalline_button_set_colors(
    CrystallineButton* button,
    CrystallineColor normal,
    CrystallineColor hover,
    CrystallineColor active,
    CrystallineColor disabled
);

/**
 * Set button font
 */
void crystalline_button_set_font(
    CrystallineButton* button,
    TTF_Font* font
);

// ============================================================================
// RENDERING
// ============================================================================

/**
 * Render button
 */
void crystalline_button_render(
    CrystallineButton* button,
    SDL_Renderer* renderer
);

// ============================================================================
// EVENT HANDLING
// ============================================================================

/**
 * Handle button events
 */
bool crystalline_button_handle_event(
    CrystallineButton* button,
    SDL_Event* event
);

#endif // CRYSTALLINE_V2_BUTTON_H