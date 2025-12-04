#include "element.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// ELEMENT LIFECYCLE
// ============================================================================

CrystallineElement* crystalline_element_create(
    CrystallineElementType type,
    CrystallineBounds bounds
) {
    CrystallineElement* element = (CrystallineElement*)calloc(1, sizeof(CrystallineElement));
    if (!element) return NULL;
    
    element->type = type;
    element->bounds = bounds;
    element->padding = crystalline_padding_uniform(0.0f);
    
    element->visible = true;
    element->enabled = true;
    element->focused = false;
    element->state = CRYSTALLINE_STATE_NORMAL;
    
    element->bg_color = crystalline_color(40, 40, 50, 255);
    element->border_color = crystalline_color(100, 100, 120, 255);
    element->border_width = 2.0f;
    
    element->parent = NULL;
    element->children = NULL;
    element->child_count = 0;
    element->child_capacity = 0;
    
    element->needs_layout = true;
    
    element->render = NULL;
    element->handle_event = NULL;
    element->compute_layout = NULL;
    element->destroy = NULL;
    element->user_data = NULL;
    
    return element;
}

void crystalline_element_destroy(CrystallineElement* element) {
    if (!element) return;
    
    // Destroy all children first
    for (int i = 0; i < element->child_count; i++) {
        crystalline_element_destroy(element->children[i]);
    }
    
    // Free children array
    if (element->children) {
        free(element->children);
    }
    
    // Call custom destroy function if provided
    if (element->destroy) {
        element->destroy(element);
    }
    
    // Free the element itself
    free(element);
}

// ============================================================================
// HIERARCHY MANAGEMENT
// ============================================================================

void crystalline_element_add_child(
    CrystallineElement* parent,
    CrystallineElement* child
) {
    if (!parent || !child) return;
    
    // Grow children array if needed
    if (parent->child_count >= parent->child_capacity) {
        int new_capacity = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
        CrystallineElement** new_children = (CrystallineElement**)realloc(
            parent->children,
            new_capacity * sizeof(CrystallineElement*)
        );
        if (!new_children) return;
        
        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }
    
    // Add child
    parent->children[parent->child_count++] = child;
    child->parent = parent;
    
    // Invalidate layout
    crystalline_element_invalidate_layout(parent);
}

void crystalline_element_remove_child(
    CrystallineElement* parent,
    CrystallineElement* child
) {
    if (!parent || !child) return;
    
    // Find child index
    int index = -1;
    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) {
            index = i;
            break;
        }
    }
    
    if (index < 0) return;
    
    // Remove child by shifting array
    for (int i = index; i < parent->child_count - 1; i++) {
        parent->children[i] = parent->children[i + 1];
    }
    parent->child_count--;
    
    child->parent = NULL;
    
    // Invalidate layout
    crystalline_element_invalidate_layout(parent);
}

CrystallineElement* crystalline_element_get_child(
    CrystallineElement* parent,
    int index
) {
    if (!parent || index < 0 || index >= parent->child_count) {
        return NULL;
    }
    return parent->children[index];
}

// ============================================================================
// RENDERING
// ============================================================================

void crystalline_element_render_background(
    CrystallineElement* element,
    SDL_Renderer* renderer
) {
    if (!element || !renderer) return;
    
    CrystallinePoint top_left = crystalline_bounds_top_left(element->bounds);
    
    SDL_Rect rect = {
        (int)top_left.x,
        (int)top_left.y,
        (int)element->bounds.width,
        (int)element->bounds.height
    };
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, 
        element->bg_color.r, element->bg_color.g, 
        element->bg_color.b, element->bg_color.a);
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw border
    if (element->border_width > 0) {
        SDL_SetRenderDrawColor(renderer,
            element->border_color.r, element->border_color.g,
            element->border_color.b, element->border_color.a);
        SDL_RenderDrawRect(renderer, &rect);
    }
}

void crystalline_element_render(
    CrystallineElement* element,
    SDL_Renderer* renderer
) {
    if (!element || !renderer || !element->visible) return;
    
    // Render background
    crystalline_element_render_background(element, renderer);
    
    // Call custom render function if provided
    if (element->render) {
        element->render(element, renderer);
    }
    
    // Render children
    for (int i = 0; i < element->child_count; i++) {
        crystalline_element_render(element->children[i], renderer);
    }
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

void crystalline_element_update_hover(
    CrystallineElement* element,
    int mouse_x,
    int mouse_y
) {
    if (!element || !element->enabled) return;
    
    bool contains = crystalline_element_contains_point(element, mouse_x, mouse_y);
    
    if (contains && element->state == CRYSTALLINE_STATE_NORMAL) {
        element->state = CRYSTALLINE_STATE_HOVER;
    } else if (!contains && element->state == CRYSTALLINE_STATE_HOVER) {
        element->state = CRYSTALLINE_STATE_NORMAL;
    }
    
    // Update children
    for (int i = 0; i < element->child_count; i++) {
        crystalline_element_update_hover(element->children[i], mouse_x, mouse_y);
    }
}

bool crystalline_element_handle_event(
    CrystallineElement* element,
    SDL_Event* event
) {
    if (!element || !event || !element->visible || !element->enabled) {
        return false;
    }
    
    // Update hover state on mouse motion
    if (event->type == SDL_MOUSEMOTION) {
        crystalline_element_update_hover(element, event->motion.x, event->motion.y);
    }
    
    // Try children first (front to back)
    for (int i = element->child_count - 1; i >= 0; i--) {
        if (crystalline_element_handle_event(element->children[i], event)) {
            return true;  // Event handled by child
        }
    }
    
    // Try custom event handler
    if (element->handle_event) {
        return element->handle_event(element, event);
    }
    
    return false;
}

// ============================================================================
// LAYOUT
// ============================================================================

void crystalline_element_compute_layout(CrystallineElement* element) {
    if (!element || !element->needs_layout) return;
    
    // Call custom layout function if provided
    if (element->compute_layout) {
        element->compute_layout(element);
    }
    
    // Compute layout for children
    for (int i = 0; i < element->child_count; i++) {
        crystalline_element_compute_layout(element->children[i]);
    }
    
    element->needs_layout = false;
}

void crystalline_element_invalidate_layout(CrystallineElement* element) {
    if (!element) return;
    
    element->needs_layout = true;
    
    // Invalidate parent layout too
    if (element->parent) {
        crystalline_element_invalidate_layout(element->parent);
    }
}

// ============================================================================
// GEOMETRY HELPERS
// ============================================================================

CrystallineBounds crystalline_element_get_absolute_bounds(
    CrystallineElement* element
) {
    if (!element) {
        return crystalline_bounds(0, 0, 0, 0);
    }
    
    CrystallineBounds bounds = element->bounds;
    
    // Add parent offsets
    CrystallineElement* parent = element->parent;
    while (parent) {
        bounds.center.x += parent->bounds.center.x;
        bounds.center.y += parent->bounds.center.y;
        parent = parent->parent;
    }
    
    return bounds;
}

CrystallineBounds crystalline_element_get_content_bounds(
    CrystallineElement* element
) {
    if (!element) {
        return crystalline_bounds(0, 0, 0, 0);
    }
    
    float content_width = element->bounds.width - 
                         element->padding.left - element->padding.right;
    float content_height = element->bounds.height - 
                          element->padding.top - element->padding.bottom;
    
    float center_x = element->bounds.center.x + 
                    (element->padding.left - element->padding.right) / 2.0f;
    float center_y = element->bounds.center.y + 
                    (element->padding.top - element->padding.bottom) / 2.0f;
    
    return crystalline_bounds(center_x, center_y, content_width, content_height);
}

bool crystalline_element_contains_point(
    CrystallineElement* element,
    int x,
    int y
) {
    if (!element) return false;
    
    CrystallineBounds abs_bounds = crystalline_element_get_absolute_bounds(element);
    return crystalline_bounds_contains_point(abs_bounds, crystalline_point(x, y));
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

void crystalline_element_set_visible(
    CrystallineElement* element,
    bool visible
) {
    if (element) {
        element->visible = visible;
    }
}

void crystalline_element_set_enabled(
    CrystallineElement* element,
    bool enabled
) {
    if (element) {
        element->enabled = enabled;
        if (!enabled) {
            element->state = CRYSTALLINE_STATE_DISABLED;
        } else if (element->state == CRYSTALLINE_STATE_DISABLED) {
            element->state = CRYSTALLINE_STATE_NORMAL;
        }
    }
}

void crystalline_element_set_focused(
    CrystallineElement* element,
    bool focused
) {
    if (element) {
        element->focused = focused;
    }
}