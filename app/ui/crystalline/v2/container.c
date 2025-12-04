#include "container.h"
#include <stdlib.h>
#include <math.h>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void container_compute_layout_func(CrystallineElement* element);
static void container_render_func(CrystallineElement* element, SDL_Renderer* renderer);
static bool container_handle_event_func(CrystallineElement* element, SDL_Event* event);
static void container_destroy_func(CrystallineElement* element);

// ============================================================================
// CONTAINER LIFECYCLE
// ============================================================================

CrystallineContainer* crystalline_container_create(
    CrystallineBounds bounds,
    CrystallineLayoutType layout_type,
    float spacing
) {
    CrystallineContainer* container = (CrystallineContainer*)calloc(1, sizeof(CrystallineContainer));
    if (!container) return NULL;
    
    // Initialize base element
    CrystallineElement* base = &container->base;
    base->type = CRYSTALLINE_ELEMENT_CONTAINER;
    base->bounds = bounds;
    base->padding = crystalline_padding_uniform(5.0f);  // Default padding
    base->visible = true;
    base->enabled = true;
    base->state = CRYSTALLINE_STATE_NORMAL;
    
    // Set colors
    base->bg_color = crystalline_color(30, 30, 40, 255);
    base->border_color = crystalline_color(80, 80, 100, 255);
    base->border_width = 2.0f;
    
    // Set callbacks
    base->compute_layout = container_compute_layout_func;
    base->render = container_render_func;
    base->handle_event = container_handle_event_func;
    base->destroy = container_destroy_func;
    
    // Initialize container-specific fields
    container->layout_type = layout_type;
    container->spacing = spacing;
    container->alignment = CRYSTALLINE_ALIGN_START;
    container->scrollable = false;
    container->scroll_offset = 0.0f;
    container->scroll_max = 0.0f;
    container->clip_children = true;
    
    return container;
}

void crystalline_container_destroy(CrystallineContainer* container) {
    if (!container) return;
    crystalline_element_destroy(&container->base);
}

// ============================================================================
// CHILD MANAGEMENT
// ============================================================================

void crystalline_container_add_child(
    CrystallineContainer* container,
    CrystallineElement* child
) {
    if (!container || !child) return;
    crystalline_element_add_child(&container->base, child);
}

void crystalline_container_remove_child(
    CrystallineContainer* container,
    CrystallineElement* child
) {
    if (!container || !child) return;
    crystalline_element_remove_child(&container->base, child);
}

void crystalline_container_clear_children(CrystallineContainer* container) {
    if (!container) return;
    
    CrystallineElement* base = &container->base;
    for (int i = 0; i < base->child_count; i++) {
        crystalline_element_destroy(base->children[i]);
    }
    base->child_count = 0;
}

// ============================================================================
// LAYOUT ALGORITHMS
// ============================================================================

static void compute_vertical_layout(CrystallineContainer* container) {
    CrystallineElement* base = &container->base;
    if (base->child_count == 0) return;
    
    CrystallineBounds content = crystalline_element_get_content_bounds(base);
    
    // Calculate total height needed
    float total_height = 0.0f;
    for (int i = 0; i < base->child_count; i++) {
        if (base->children[i]->visible) {
            total_height += base->children[i]->bounds.height;
        }
    }
    total_height += (base->child_count - 1) * container->spacing;
    
    // Calculate starting Y based on alignment
    float start_y;
    switch (container->alignment) {
        case CRYSTALLINE_ALIGN_START:
            start_y = content.center.y - content.height / 2.0f;
            break;
        case CRYSTALLINE_ALIGN_CENTER:
            start_y = content.center.y - total_height / 2.0f;
            break;
        case CRYSTALLINE_ALIGN_END:
            start_y = content.center.y + content.height / 2.0f - total_height;
            break;
        default:
            start_y = content.center.y - content.height / 2.0f;
    }
    
    // Position children
    float current_y = start_y;
    for (int i = 0; i < base->child_count; i++) {
        CrystallineElement* child = base->children[i];
        if (!child->visible) continue;
        
        // Set child position (CENTER coordinates relative to parent)
        child->bounds.center.x = content.center.x;
        child->bounds.center.y = current_y + child->bounds.height / 2.0f;
        
        current_y += child->bounds.height + container->spacing;
    }
    
    // Update scroll max
    if (container->scrollable) {
        container->scroll_max = fmaxf(0.0f, total_height - content.height);
    }
}

static void compute_horizontal_layout(CrystallineContainer* container) {
    CrystallineElement* base = &container->base;
    if (base->child_count == 0) return;
    
    CrystallineBounds content = crystalline_element_get_content_bounds(base);
    
    // Calculate total width needed
    float total_width = 0.0f;
    for (int i = 0; i < base->child_count; i++) {
        if (base->children[i]->visible) {
            total_width += base->children[i]->bounds.width;
        }
    }
    total_width += (base->child_count - 1) * container->spacing;
    
    // Calculate starting X based on alignment
    float start_x;
    switch (container->alignment) {
        case CRYSTALLINE_ALIGN_START:
            start_x = content.center.x - content.width / 2.0f;
            break;
        case CRYSTALLINE_ALIGN_CENTER:
            start_x = content.center.x - total_width / 2.0f;
            break;
        case CRYSTALLINE_ALIGN_END:
            start_x = content.center.x + content.width / 2.0f - total_width;
            break;
        default:
            start_x = content.center.x - content.width / 2.0f;
    }
    
    // Position children
    float current_x = start_x;
    for (int i = 0; i < base->child_count; i++) {
        CrystallineElement* child = base->children[i];
        if (!child->visible) continue;
        
        // Set child position (CENTER coordinates relative to parent)
        child->bounds.center.x = current_x + child->bounds.width / 2.0f;
        child->bounds.center.y = content.center.y;
        
        current_x += child->bounds.width + container->spacing;
    }
    
    // Update scroll max
    if (container->scrollable) {
        container->scroll_max = fmaxf(0.0f, total_width - content.width);
    }
}

static void compute_absolute_layout(CrystallineContainer* container) {
    // Absolute layout: children keep their specified positions
    // No automatic positioning
    (void)container;  // Unused
}

static void container_compute_layout_func(CrystallineElement* element) {
    CrystallineContainer* container = (CrystallineContainer*)element;
    
    switch (container->layout_type) {
        case CRYSTALLINE_LAYOUT_VERTICAL:
            compute_vertical_layout(container);
            break;
        case CRYSTALLINE_LAYOUT_HORIZONTAL:
            compute_horizontal_layout(container);
            break;
        case CRYSTALLINE_LAYOUT_ABSOLUTE:
            compute_absolute_layout(container);
            break;
        case CRYSTALLINE_LAYOUT_GRID:
            // TODO: Implement grid layout
            break;
    }
}

void crystalline_container_compute_layout(CrystallineContainer* container) {
    if (!container) return;
    crystalline_element_compute_layout(&container->base);
}

// ============================================================================
// LAYOUT CONFIGURATION
// ============================================================================

void crystalline_container_set_layout(
    CrystallineContainer* container,
    CrystallineLayoutType layout_type
) {
    if (!container) return;
    container->layout_type = layout_type;
    crystalline_element_invalidate_layout(&container->base);
}

void crystalline_container_set_spacing(
    CrystallineContainer* container,
    float spacing
) {
    if (!container) return;
    container->spacing = spacing;
    crystalline_element_invalidate_layout(&container->base);
}

void crystalline_container_set_alignment(
    CrystallineContainer* container,
    CrystallineAlignment alignment
) {
    if (!container) return;
    container->alignment = alignment;
    crystalline_element_invalidate_layout(&container->base);
}

// ============================================================================
// SCROLLING
// ============================================================================

void crystalline_container_set_scrollable(
    CrystallineContainer* container,
    bool scrollable
) {
    if (!container) return;
    container->scrollable = scrollable;
}

void crystalline_container_set_scroll_offset(
    CrystallineContainer* container,
    float offset
) {
    if (!container || !container->scrollable) return;
    
    container->scroll_offset = fmaxf(0.0f, fminf(offset, container->scroll_max));
    crystalline_element_invalidate_layout(&container->base);
}

void crystalline_container_scroll(
    CrystallineContainer* container,
    float delta
) {
    if (!container) return;
    crystalline_container_set_scroll_offset(container, container->scroll_offset + delta);
}

// ============================================================================
// RENDERING
// ============================================================================

static void container_render_func(CrystallineElement* element, SDL_Renderer* renderer) {
    CrystallineContainer* container = (CrystallineContainer*)element;
    
    // Set clipping if enabled
    SDL_Rect clip_rect;
    bool clipping = false;
    
    if (container->clip_children) {
        CrystallinePoint top_left = crystalline_bounds_top_left(element->bounds);
        clip_rect.x = (int)top_left.x;
        clip_rect.y = (int)top_left.y;
        clip_rect.w = (int)element->bounds.width;
        clip_rect.h = (int)element->bounds.height;
        
        SDL_RenderSetClipRect(renderer, &clip_rect);
        clipping = true;
    }
    
    // Children are rendered by base element render function
    
    // Restore clipping
    if (clipping) {
        SDL_RenderSetClipRect(renderer, NULL);
    }
}

void crystalline_container_render(
    CrystallineContainer* container,
    SDL_Renderer* renderer
) {
    if (!container) return;
    crystalline_element_render(&container->base, renderer);
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

static bool container_handle_event_func(CrystallineElement* element, SDL_Event* event) {
    CrystallineContainer* container = (CrystallineContainer*)element;
    
    // Handle scroll events
    if (container->scrollable && event->type == SDL_MOUSEWHEEL) {
        if (crystalline_element_contains_point(element, event->wheel.mouseX, event->wheel.mouseY)) {
            crystalline_container_scroll(container, -event->wheel.y * 20.0f);
            return true;
        }
    }
    
    return false;
}

bool crystalline_container_handle_event(
    CrystallineContainer* container,
    SDL_Event* event
) {
    if (!container) return false;
    return crystalline_element_handle_event(&container->base, event);
}

// ============================================================================
// CLEANUP
// ============================================================================

static void container_destroy_func(CrystallineElement* element) {
    // Container-specific cleanup (if needed)
    (void)element;
}