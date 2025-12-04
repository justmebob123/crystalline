#ifndef CRYSTALLINE_V2_PANEL_H
#define CRYSTALLINE_V2_PANEL_H

#include "element.h"

/**
 * Panel element - visual grouping container
 */
typedef struct {
    CrystallineElement base;
    
    // Title
    const char* title;
    TTF_Font* font;
    bool show_title;
    
    // Colors
    CrystallineColor title_color;
} CrystallinePanel;

// ============================================================================
// PANEL LIFECYCLE
// ============================================================================

/**
 * Create a panel
 */
CrystallinePanel* crystalline_panel_create(
    CrystallineBounds bounds,
    const char* title
);

/**
 * Destroy panel
 */
void crystalline_panel_destroy(CrystallinePanel* panel);

// ============================================================================
// PANEL CONFIGURATION
// ============================================================================

/**
 * Set panel title
 */
void crystalline_panel_set_title(
    CrystallinePanel* panel,
    const char* title
);

/**
 * Show/hide title
 */
void crystalline_panel_set_show_title(
    CrystallinePanel* panel,
    bool show
);

/**
 * Set title font
 */
void crystalline_panel_set_font(
    CrystallinePanel* panel,
    TTF_Font* font
);

// ============================================================================
// RENDERING
// ============================================================================

/**
 * Render panel
 */
void crystalline_panel_render(
    CrystallinePanel* panel,
    SDL_Renderer* renderer
);

#endif // CRYSTALLINE_V2_PANEL_H