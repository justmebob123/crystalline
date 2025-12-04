#include "button.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void button_render_func(CrystallineElement* element, SDL_Renderer* renderer);
static bool button_handle_event_func(CrystallineElement* element, SDL_Event* event);
static void button_destroy_func(CrystallineElement* element);

// ============================================================================
// BUTTON LIFECYCLE
// ============================================================================

CrystallineButton* crystalline_button_create(
    const char* label,
    CrystallineSize size,
    bool circular,
    CrystallineButtonCallback on_click,
    void* user_data
) {
    // Calculate bounds from size
    float radius = crystalline_size_to_pixels(size);
    float dimension = circular ? radius * 2.0f : radius * 2.0f;
    
    CrystallineBounds bounds = crystalline_bounds(0, 0, dimension, dimension);
    
    return crystalline_button_create_custom(label, bounds, circular, on_click, user_data);
}

CrystallineButton* crystalline_button_create_custom(
    const char* label,
    CrystallineBounds bounds,
    bool circular,
    CrystallineButtonCallback on_click,
    void* user_data
) {
    CrystallineButton* button = (CrystallineButton*)calloc(1, sizeof(CrystallineButton));
    if (!button) return NULL;
    
    // Initialize base element
    CrystallineElement* base = &button->base;
    base->type = CRYSTALLINE_ELEMENT_BUTTON;
    base->bounds = bounds;
    base->padding = crystalline_padding_uniform(5.0f);
    base->visible = true;
    base->enabled = true;
    base->state = CRYSTALLINE_STATE_NORMAL;
    
    // Set callbacks
    base->render = button_render_func;
    base->handle_event = button_handle_event_func;
    base->destroy = button_destroy_func;
    
    // Initialize button-specific fields
    button->label = label ? strdup(label) : NULL;
    button->font = NULL;  // Will be set by user
    button->circular = circular;
    button->on_click = on_click;
    button->callback_data = user_data;
    
    // Set default colors
    button->normal_color = crystalline_color(60, 60, 80, 255);
    button->hover_color = crystalline_color(80, 80, 100, 255);
    button->active_color = crystalline_color(100, 150, 200, 255);
    button->disabled_color = crystalline_color(40, 40, 40, 255);
    button->text_color = crystalline_color(220, 220, 220, 255);
    
    return button;
}

void crystalline_button_destroy(CrystallineButton* button) {
    if (!button) return;
    
    if (button->label) {
        free((void*)button->label);
    }
    
    crystalline_element_destroy(&button->base);
}

// ============================================================================
// BUTTON CONFIGURATION
// ============================================================================

void crystalline_button_set_label(
    CrystallineButton* button,
    const char* label
) {
    if (!button) return;
    
    if (button->label) {
        free((void*)button->label);
    }
    
    button->label = label ? strdup(label) : NULL;
}

void crystalline_button_set_callback(
    CrystallineButton* button,
    CrystallineButtonCallback on_click,
    void* user_data
) {
    if (!button) return;
    
    button->on_click = on_click;
    button->callback_data = user_data;
}

void crystalline_button_set_colors(
    CrystallineButton* button,
    CrystallineColor normal,
    CrystallineColor hover,
    CrystallineColor active,
    CrystallineColor disabled
) {
    if (!button) return;
    
    button->normal_color = normal;
    button->hover_color = hover;
    button->active_color = active;
    button->disabled_color = disabled;
}

void crystalline_button_set_font(
    CrystallineButton* button,
    TTF_Font* font
) {
    if (!button) return;
    button->font = font;
}

// ============================================================================
// RENDERING
// ============================================================================

static void draw_circle_filled(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

static void draw_circle_outline(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

static void button_render_func(CrystallineElement* element, SDL_Renderer* renderer) {
    CrystallineButton* button = (CrystallineButton*)element;
    
    // Determine color based on state
    CrystallineColor bg_color;
    switch (element->state) {
        case CRYSTALLINE_STATE_HOVER:
            bg_color = button->hover_color;
            break;
        case CRYSTALLINE_STATE_ACTIVE:
            bg_color = button->active_color;
            break;
        case CRYSTALLINE_STATE_DISABLED:
            bg_color = button->disabled_color;
            break;
        default:
            bg_color = button->normal_color;
    }
    
    CrystallinePoint top_left = crystalline_bounds_top_left(element->bounds);
    int cx = (int)element->bounds.center.x;
    int cy = (int)element->bounds.center.y;
    
    if (button->circular) {
        // Draw circular button
        int radius = (int)(element->bounds.width / 2.0f);
        draw_circle_filled(renderer, cx, cy, radius, crystalline_color_to_sdl(bg_color));
        draw_circle_outline(renderer, cx, cy, radius, crystalline_color_to_sdl(element->border_color));
    } else {
        // Draw rectangular button
        SDL_Rect rect = {
            (int)top_left.x,
            (int)top_left.y,
            (int)element->bounds.width,
            (int)element->bounds.height
        };
        
        SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        SDL_RenderFillRect(renderer, &rect);
        
        SDL_Color border = crystalline_color_to_sdl(element->border_color);
        SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
        SDL_RenderDrawRect(renderer, &rect);
    }
    
    // Draw label
    if (button->label && button->font) {
        SDL_Color text_color = crystalline_color_to_sdl(button->text_color);
        SDL_Surface* surface = TTF_RenderText_Blended(button->font, button->label, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {
                    cx - surface->w / 2,
                    cy - surface->h / 2,
                    surface->w,
                    surface->h
                };
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
}

void crystalline_button_render(
    CrystallineButton* button,
    SDL_Renderer* renderer
) {
    if (!button) return;
    crystalline_element_render(&button->base, renderer);
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

static bool button_handle_event_func(CrystallineElement* element, SDL_Event* event) {
    CrystallineButton* button = (CrystallineButton*)element;
    
    if (!element->enabled) return false;
    
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (crystalline_element_contains_point(element, event->button.x, event->button.y)) {
            element->state = CRYSTALLINE_STATE_ACTIVE;
            return true;
        }
    } else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
        if (element->state == CRYSTALLINE_STATE_ACTIVE) {
            element->state = CRYSTALLINE_STATE_HOVER;
            
            // Trigger callback if click is still inside button
            if (crystalline_element_contains_point(element, event->button.x, event->button.y)) {
                if (button->on_click) {
                    button->on_click(button->callback_data);
                }
            }
            return true;
        }
    }
    
    return false;
}

bool crystalline_button_handle_event(
    CrystallineButton* button,
    SDL_Event* event
) {
    if (!button) return false;
    return crystalline_element_handle_event(&button->base, event);
}

// ============================================================================
// CLEANUP
// ============================================================================

static void button_destroy_func(CrystallineElement* element) {
    CrystallineButton* button = (CrystallineButton*)element;
    
    if (button->label) {
        free((void*)button->label);
        button->label = NULL;
    }
}