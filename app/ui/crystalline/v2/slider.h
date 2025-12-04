#ifndef CRYSTALLINE_V2_SLIDER_H
#define CRYSTALLINE_V2_SLIDER_H

#include "element.h"

/**
 * Slider callback type
 */
typedef void (*CrystallineSliderCallback)(float value, void* user_data);

/**
 * Slider element
 */
typedef struct {
    CrystallineElement base;
    
    // Value range
    float min_value;
    float max_value;
    float current_value;
    
    // Visual
    float track_height;
    float handle_size;
    
    // Interaction
    bool dragging;
    
    // Callback
    CrystallineSliderCallback on_change;
    void* callback_data;
    
    // Font (for value display)
    TTF_Font* font;
    bool show_value;
    
    // Colors
    CrystallineColor track_color;
    CrystallineColor fill_color;
    CrystallineColor handle_color;
    CrystallineColor handle_hover_color;
    CrystallineColor text_color;
} CrystallineSlider;

// ============================================================================
// SLIDER LIFECYCLE
// ============================================================================

/**
 * Create a slider
 */
CrystallineSlider* crystalline_slider_create(
    CrystallineBounds bounds,
    float min_value,
    float max_value,
    float initial_value,
    CrystallineSliderCallback on_change,
    void* user_data
);

/**
 * Destroy slider
 */
void crystalline_slider_destroy(CrystallineSlider* slider);

// ============================================================================
// SLIDER CONFIGURATION
// ============================================================================

/**
 * Set slider value
 */
void crystalline_slider_set_value(
    CrystallineSlider* slider,
    float value
);

/**
 * Get slider value
 */
float crystalline_slider_get_value(CrystallineSlider* slider);

/**
 * Set value range
 */
void crystalline_slider_set_range(
    CrystallineSlider* slider,
    float min_value,
    float max_value
);

/**
 * Set callback
 */
void crystalline_slider_set_callback(
    CrystallineSlider* slider,
    CrystallineSliderCallback on_change,
    void* user_data
);

/**
 * Set font for value display
 */
void crystalline_slider_set_font(
    CrystallineSlider* slider,
    TTF_Font* font
);

/**
 * Show/hide value display
 */
void crystalline_slider_set_show_value(
    CrystallineSlider* slider,
    bool show
);

// ============================================================================
// RENDERING
// ============================================================================

/**
 * Render slider
 */
void crystalline_slider_render(
    CrystallineSlider* slider,
    SDL_Renderer* renderer
);

// ============================================================================
// EVENT HANDLING
// ============================================================================

/**
 * Handle slider events
 */
bool crystalline_slider_handle_event(
    CrystallineSlider* slider,
    SDL_Event* event
);

#endif // CRYSTALLINE_V2_SLIDER_H