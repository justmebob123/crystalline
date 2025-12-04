#ifndef CRYSTALLINE_V2_ELEMENT_H
#define CRYSTALLINE_V2_ELEMENT_H

#include "core.h"

/**
 * Forward declarations
 */
typedef struct CrystallineElement CrystallineElement;

/**
 * Callback types
 */
typedef void (*CrystallineRenderFunc)(CrystallineElement* element, SDL_Renderer* renderer);
typedef bool (*CrystallineEventFunc)(CrystallineElement* element, SDL_Event* event);
typedef void (*CrystallineLayoutFunc)(CrystallineElement* element);
typedef void (*CrystallineDestroyFunc)(CrystallineElement* element);

/**
 * Base element structure
 * 
 * All UI elements inherit from this base structure.
 * This provides common functionality for hierarchy, rendering, events, and layout.
 */
struct CrystallineElement {
    // ========================================================================
    // IDENTITY
    // ========================================================================
    const char* id;                      // Unique identifier (optional)
    CrystallineElementType type;         // Element type
    
    // ========================================================================
    // HIERARCHY
    // ========================================================================
    CrystallineElement* parent;          // Parent element (NULL for root)
    CrystallineElement** children;       // Array of child elements
    int child_count;                     // Number of children
    int child_capacity;                  // Allocated capacity for children
    
    // ========================================================================
    // GEOMETRY (always CENTER coordinates)
    // ========================================================================
    CrystallineBounds bounds;            // Position and size (CENTER-based)
    CrystallinePadding padding;          // Internal padding
    
    // ========================================================================
    // STATE
    // ========================================================================
    bool visible;                        // Is element visible?
    bool enabled;                        // Is element enabled?
    bool focused;                        // Is element focused?
    CrystallineElementState state;       // Current state (NORMAL, HOVER, etc.)
    
    // ========================================================================
    // RENDERING
    // ========================================================================
    CrystallineRenderFunc render;        // Custom render function
    CrystallineColor bg_color;           // Background color
    CrystallineColor border_color;       // Border color
    float border_width;                  // Border width
    
    // ========================================================================
    // EVENTS
    // ========================================================================
    CrystallineEventFunc handle_event;   // Custom event handler
    
    // ========================================================================
    // LAYOUT
    // ========================================================================
    CrystallineLayoutFunc compute_layout; // Custom layout function
    bool needs_layout;                   // Does layout need recomputation?
    
    // ========================================================================
    // LIFECYCLE
    // ========================================================================
    CrystallineDestroyFunc destroy;      // Custom cleanup function
    
    // ========================================================================
    // USER DATA
    // ========================================================================
    void* user_data;                     // User-defined data
};

// ============================================================================
// ELEMENT LIFECYCLE
// ============================================================================

/**
 * Create a new element
 */
CrystallineElement* crystalline_element_create(
    CrystallineElementType type,
    CrystallineBounds bounds
);

/**
 * Destroy an element and all its children
 */
void crystalline_element_destroy(CrystallineElement* element);

// ============================================================================
// HIERARCHY MANAGEMENT
// ============================================================================

/**
 * Add a child element
 * Child will be positioned relative to parent
 */
void crystalline_element_add_child(
    CrystallineElement* parent,
    CrystallineElement* child
);

/**
 * Remove a child element
 */
void crystalline_element_remove_child(
    CrystallineElement* parent,
    CrystallineElement* child
);

/**
 * Get child by index
 */
CrystallineElement* crystalline_element_get_child(
    CrystallineElement* parent,
    int index
);

// ============================================================================
// RENDERING
// ============================================================================

/**
 * Render element and all its children
 * Handles clipping, visibility, and render order
 */
void crystalline_element_render(
    CrystallineElement* element,
    SDL_Renderer* renderer
);

/**
 * Render element background (border and fill)
 */
void crystalline_element_render_background(
    CrystallineElement* element,
    SDL_Renderer* renderer
);

// ============================================================================
// EVENT HANDLING
// ============================================================================

/**
 * Handle event for element and its children
 * Returns true if event was handled
 */
bool crystalline_element_handle_event(
    CrystallineElement* element,
    SDL_Event* event
);

/**
 * Update element state based on mouse position
 */
void crystalline_element_update_hover(
    CrystallineElement* element,
    int mouse_x,
    int mouse_y
);

// ============================================================================
// LAYOUT
// ============================================================================

/**
 * Compute layout for element and its children
 */
void crystalline_element_compute_layout(CrystallineElement* element);

/**
 * Mark element as needing layout recomputation
 */
void crystalline_element_invalidate_layout(CrystallineElement* element);

// ============================================================================
// GEOMETRY HELPERS
// ============================================================================

/**
 * Get absolute bounds (accounting for parent position)
 */
CrystallineBounds crystalline_element_get_absolute_bounds(
    CrystallineElement* element
);

/**
 * Get content bounds (bounds minus padding)
 */
CrystallineBounds crystalline_element_get_content_bounds(
    CrystallineElement* element
);

/**
 * Check if point is inside element
 */
bool crystalline_element_contains_point(
    CrystallineElement* element,
    int x,
    int y
);

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

/**
 * Set element visibility
 */
void crystalline_element_set_visible(
    CrystallineElement* element,
    bool visible
);

/**
 * Set element enabled state
 */
void crystalline_element_set_enabled(
    CrystallineElement* element,
    bool enabled
);

/**
 * Set element focus
 */
void crystalline_element_set_focused(
    CrystallineElement* element,
    bool focused
);

#endif // CRYSTALLINE_V2_ELEMENT_H