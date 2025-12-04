#ifndef CRYSTALLINE_V2_CONTAINER_H
#define CRYSTALLINE_V2_CONTAINER_H

#include "element.h"

/**
 * Container element with automatic layout management
 */
typedef struct {
    CrystallineElement base;
    
    // Layout configuration
    CrystallineLayoutType layout_type;
    float spacing;                    // Space between children
    CrystallineAlignment alignment;   // Child alignment
    
    // Scrolling
    bool scrollable;
    float scroll_offset;
    float scroll_max;
    
    // Clipping
    bool clip_children;               // Clip children to bounds
} CrystallineContainer;

// ============================================================================
// CONTAINER LIFECYCLE
// ============================================================================

/**
 * Create a new container
 */
CrystallineContainer* crystalline_container_create(
    CrystallineBounds bounds,
    CrystallineLayoutType layout_type,
    float spacing
);

/**
 * Destroy container
 */
void crystalline_container_destroy(CrystallineContainer* container);

// ============================================================================
// CHILD MANAGEMENT
// ============================================================================

/**
 * Add child to container
 * Child will be positioned according to container's layout
 */
void crystalline_container_add_child(
    CrystallineContainer* container,
    CrystallineElement* child
);

/**
 * Remove child from container
 */
void crystalline_container_remove_child(
    CrystallineContainer* container,
    CrystallineElement* child
);

/**
 * Clear all children
 */
void crystalline_container_clear_children(CrystallineContainer* container);

// ============================================================================
// LAYOUT
// ============================================================================

/**
 * Compute layout for all children
 * This positions children according to layout_type
 */
void crystalline_container_compute_layout(CrystallineContainer* container);

/**
 * Set layout type
 */
void crystalline_container_set_layout(
    CrystallineContainer* container,
    CrystallineLayoutType layout_type
);

/**
 * Set spacing between children
 */
void crystalline_container_set_spacing(
    CrystallineContainer* container,
    float spacing
);

/**
 * Set child alignment
 */
void crystalline_container_set_alignment(
    CrystallineContainer* container,
    CrystallineAlignment alignment
);

// ============================================================================
// SCROLLING
// ============================================================================

/**
 * Enable/disable scrolling
 */
void crystalline_container_set_scrollable(
    CrystallineContainer* container,
    bool scrollable
);

/**
 * Set scroll offset
 */
void crystalline_container_set_scroll_offset(
    CrystallineContainer* container,
    float offset
);

/**
 * Scroll by delta
 */
void crystalline_container_scroll(
    CrystallineContainer* container,
    float delta
);

// ============================================================================
// RENDERING
// ============================================================================

/**
 * Render container and children
 */
void crystalline_container_render(
    CrystallineContainer* container,
    SDL_Renderer* renderer
);

// ============================================================================
// EVENT HANDLING
// ============================================================================

/**
 * Handle events for container and children
 */
bool crystalline_container_handle_event(
    CrystallineContainer* container,
    SDL_Event* event
);

#endif // CRYSTALLINE_V2_CONTAINER_H