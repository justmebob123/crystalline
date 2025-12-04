#ifndef CRYSTALLINE_V2_LABEL_H
#define CRYSTALLINE_V2_LABEL_H

#include "element.h"

/**
 * Text alignment
 */
typedef enum {
    CRYSTALLINE_TEXT_ALIGN_LEFT,
    CRYSTALLINE_TEXT_ALIGN_CENTER,
    CRYSTALLINE_TEXT_ALIGN_RIGHT
} CrystallineTextAlign;

/**
 * Label element - text display
 */
typedef struct {
    CrystallineElement base;
    
    // Text
    const char* text;
    TTF_Font* font;
    
    // Alignment
    CrystallineTextAlign alignment;
    
    // Colors
    CrystallineColor text_color;
} CrystallineLabel;

// ============================================================================
// LABEL LIFECYCLE
// ============================================================================

/**
 * Create a label
 */
CrystallineLabel* crystalline_label_create(
    CrystallineBounds bounds,
    const char* text,
    TTF_Font* font
);

/**
 * Destroy label
 */
void crystalline_label_destroy(CrystallineLabel* label);

// ============================================================================
// LABEL CONFIGURATION
// ============================================================================

/**
 * Set label text
 */
void crystalline_label_set_text(
    CrystallineLabel* label,
    const char* text
);

/**
 * Set label font
 */
void crystalline_label_set_font(
    CrystallineLabel* label,
    TTF_Font* font
);

/**
 * Set text alignment
 */
void crystalline_label_set_alignment(
    CrystallineLabel* label,
    CrystallineTextAlign alignment
);

/**
 * Set text color
 */
void crystalline_label_set_color(
    CrystallineLabel* label,
    CrystallineColor color
);

// ============================================================================
// RENDERING
// ============================================================================

/**
 * Render label
 */
void crystalline_label_render(
    CrystallineLabel* label,
    SDL_Renderer* renderer
);

#endif // CRYSTALLINE_V2_LABEL_H