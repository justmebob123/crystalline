#ifndef CRYSTALLINE_V2_CORE_H
#define CRYSTALLINE_V2_CORE_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/**
 * Crystalline UI Library V2 - Core Types
 * 
 * Design Principles:
 * 1. Single coordinate system: CENTER for all elements
 * 2. Explicit, type-safe APIs
 * 3. Parent-child hierarchy
 * 4. Automatic layout management
 * 5. Unified event handling
 */

// ============================================================================
// CORE TYPES
// ============================================================================

/**
 * Point - Always represents CENTER coordinates
 */
typedef struct {
    float x;
    float y;
} CrystallinePoint;

/**
 * Bounds - CENTER position + dimensions
 */
typedef struct {
    CrystallinePoint center;
    float width;
    float height;
} CrystallineBounds;

/**
 * Padding - Explicit padding on all sides
 */
typedef struct {
    float top;
    float right;
    float bottom;
    float left;
} CrystallinePadding;

/**
 * Color - RGBA color
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} CrystallineColor;

// ============================================================================
// ENUMS
// ============================================================================

/**
 * Element types
 */
typedef enum {
    CRYSTALLINE_ELEMENT_CONTAINER,
    CRYSTALLINE_ELEMENT_BUTTON,
    CRYSTALLINE_ELEMENT_LIST,
    CRYSTALLINE_ELEMENT_SLIDER,
    CRYSTALLINE_ELEMENT_DROPDOWN,
    CRYSTALLINE_ELEMENT_PANEL,
    CRYSTALLINE_ELEMENT_LABEL,
    CRYSTALLINE_ELEMENT_INPUT
} CrystallineElementType;

/**
 * Element state
 */
typedef enum {
    CRYSTALLINE_STATE_NORMAL,
    CRYSTALLINE_STATE_HOVER,
    CRYSTALLINE_STATE_ACTIVE,
    CRYSTALLINE_STATE_DISABLED
} CrystallineElementState;

/**
 * Layout types
 */
typedef enum {
    CRYSTALLINE_LAYOUT_VERTICAL,    // Stack top-to-bottom
    CRYSTALLINE_LAYOUT_HORIZONTAL,  // Stack left-to-right
    CRYSTALLINE_LAYOUT_GRID,        // Grid layout
    CRYSTALLINE_LAYOUT_ABSOLUTE     // Manual positioning
} CrystallineLayoutType;

/**
 * Alignment options
 */
typedef enum {
    CRYSTALLINE_ALIGN_START,   // Top/Left
    CRYSTALLINE_ALIGN_CENTER,  // Center
    CRYSTALLINE_ALIGN_END      // Bottom/Right
} CrystallineAlignment;

/**
 * Size presets
 */
typedef enum {
    CRYSTALLINE_SIZE_TINY,     // 15px radius / 30px width
    CRYSTALLINE_SIZE_SMALL,    // 20px radius / 40px width
    CRYSTALLINE_SIZE_MEDIUM,   // 25px radius / 50px width
    CRYSTALLINE_SIZE_LARGE     // 40px radius / 80px width
} CrystallineSize;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Create a point at CENTER coordinates
 */
static inline CrystallinePoint crystalline_point(float x, float y) {
    return (CrystallinePoint){x, y};
}

/**
 * Create bounds from CENTER + dimensions
 */
static inline CrystallineBounds crystalline_bounds(float center_x, float center_y, 
                                                     float width, float height) {
    return (CrystallineBounds){
        .center = {center_x, center_y},
        .width = width,
        .height = height
    };
}

/**
 * Create uniform padding
 */
static inline CrystallinePadding crystalline_padding_uniform(float padding) {
    return (CrystallinePadding){padding, padding, padding, padding};
}

/**
 * Create padding with different values
 */
static inline CrystallinePadding crystalline_padding(float top, float right, 
                                                       float bottom, float left) {
    return (CrystallinePadding){top, right, bottom, left};
}

/**
 * Create color from RGBA
 */
static inline CrystallineColor crystalline_color(uint8_t r, uint8_t g, 
                                                   uint8_t b, uint8_t a) {
    return (CrystallineColor){r, g, b, a};
}

/**
 * Convert CrystallineColor to SDL_Color
 */
static inline SDL_Color crystalline_color_to_sdl(CrystallineColor color) {
    return (SDL_Color){color.r, color.g, color.b, color.a};
}

/**
 * Get top-left corner from bounds (for SDL rendering)
 */
static inline CrystallinePoint crystalline_bounds_top_left(CrystallineBounds bounds) {
    return crystalline_point(
        bounds.center.x - bounds.width / 2.0f,
        bounds.center.y - bounds.height / 2.0f
    );
}

/**
 * Get bottom-right corner from bounds
 */
static inline CrystallinePoint crystalline_bounds_bottom_right(CrystallineBounds bounds) {
    return crystalline_point(
        bounds.center.x + bounds.width / 2.0f,
        bounds.center.y + bounds.height / 2.0f
    );
}

/**
 * Check if point is inside bounds
 */
static inline bool crystalline_bounds_contains_point(CrystallineBounds bounds, 
                                                       CrystallinePoint point) {
    CrystallinePoint top_left = crystalline_bounds_top_left(bounds);
    CrystallinePoint bottom_right = crystalline_bounds_bottom_right(bounds);
    
    return point.x >= top_left.x && point.x <= bottom_right.x &&
           point.y >= top_left.y && point.y <= bottom_right.y;
}

/**
 * Get size value in pixels
 */
static inline float crystalline_size_to_pixels(CrystallineSize size) {
    switch (size) {
        case CRYSTALLINE_SIZE_TINY:   return 15.0f;
        case CRYSTALLINE_SIZE_SMALL:  return 20.0f;
        case CRYSTALLINE_SIZE_MEDIUM: return 25.0f;
        case CRYSTALLINE_SIZE_LARGE:  return 40.0f;
        default: return 25.0f;
    }
}

#endif // CRYSTALLINE_V2_CORE_H