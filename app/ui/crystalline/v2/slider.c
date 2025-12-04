#include "slider.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void slider_render_func(CrystallineElement* element, SDL_Renderer* renderer);
static bool slider_handle_event_func(CrystallineElement* element, SDL_Event* event);
static void slider_destroy_func(CrystallineElement* element);

// ============================================================================
// SLIDER LIFECYCLE
// ============================================================================

CrystallineSlider* crystalline_slider_create(
    CrystallineBounds bounds,
    float min_value,
    float max_value,
    float initial_value,
    CrystallineSliderCallback on_change,
    void* user_data
) {
    CrystallineSlider* slider = (CrystallineSlider*)calloc(1, sizeof(CrystallineSlider));
    if (!slider) return NULL;
    
    // Initialize base element
    CrystallineElement* base = &slider->base;
    base->type = CRYSTALLINE_ELEMENT_SLIDER;
    base->bounds = bounds;
    base->padding = crystalline_padding_uniform(5.0f);
    base->visible = true;
    base->enabled = true;
    base->state = CRYSTALLINE_STATE_NORMAL;
    
    // Set colors
    base->bg_color = crystalline_color(30, 30, 40, 255);
    base->border_color = crystalline_color(100, 100, 120, 255);
    base->border_width = 2.0f;
    
    // Set callbacks
    base->render = slider_render_func;
    base->handle_event = slider_handle_event_func;
    base->destroy = slider_destroy_func;
    
    // Initialize slider-specific fields
    slider->min_value = min_value;
    slider->max_value = max_value;
    slider->current_value = fmaxf(min_value, fminf(initial_value, max_value));
    slider->track_height = 6.0f;
    slider->handle_size = 12.0f;
    slider->dragging = false;
    slider->on_change = on_change;
    slider->callback_data = user_data;
    slider->font = NULL;
    slider->show_value = false;
    
    // Set default colors
    slider->track_color = crystalline_color(60, 60, 80, 255);
    slider->fill_color = crystalline_color(100, 150, 200, 255);
    slider->handle_color = crystalline_color(150, 150, 170, 255);
    slider->handle_hover_color = crystalline_color(180, 180, 200, 255);
    slider->text_color = crystalline_color(220, 220, 220, 255);
    
    return slider;
}

void crystalline_slider_destroy(CrystallineSlider* slider) {
    if (!slider) return;
    crystalline_element_destroy(&slider->base);
}

// ============================================================================
// SLIDER CONFIGURATION
// ============================================================================

void crystalline_slider_set_value(
    CrystallineSlider* slider,
    float value
) {
    if (!slider) return;
    
    float old_value = slider->current_value;
    slider->current_value = fmaxf(slider->min_value, fminf(value, slider->max_value));
    
    // Trigger callback if value changed
    if (slider->current_value != old_value && slider->on_change) {
        slider->on_change(slider->current_value, slider->callback_data);
    }
}

float crystalline_slider_get_value(CrystallineSlider* slider) {
    return slider ? slider->current_value : 0.0f;
}

void crystalline_slider_set_range(
    CrystallineSlider* slider,
    float min_value,
    float max_value
) {
    if (!slider) return;
    
    slider->min_value = min_value;
    slider->max_value = max_value;
    
    // Clamp current value to new range
    slider->current_value = fmaxf(min_value, fminf(slider->current_value, max_value));
}

void crystalline_slider_set_callback(
    CrystallineSlider* slider,
    CrystallineSliderCallback on_change,
    void* user_data
) {
    if (!slider) return;
    
    slider->on_change = on_change;
    slider->callback_data = user_data;
}

void crystalline_slider_set_font(
    CrystallineSlider* slider,
    TTF_Font* font
) {
    if (!slider) return;
    slider->font = font;
}

void crystalline_slider_set_show_value(
    CrystallineSlider* slider,
    bool show
) {
    if (!slider) return;
    slider->show_value = show;
}

// ============================================================================
// RENDERING
// ============================================================================

static void slider_render_func(CrystallineElement* element, SDL_Renderer* renderer) {
    CrystallineSlider* slider = (CrystallineSlider*)element;
    
    CrystallineBounds content = crystalline_element_get_content_bounds(element);
    CrystallinePoint content_top_left = crystalline_bounds_top_left(content);
    
    // Calculate track position (centered vertically)
    float track_y = content.center.y - slider->track_height / 2.0f;
    
    // Draw track background
    SDL_Rect track_rect = {
        (int)content_top_left.x,
        (int)track_y,
        (int)content.width,
        (int)slider->track_height
    };
    SDL_Color track_color = crystalline_color_to_sdl(slider->track_color);
    SDL_SetRenderDrawColor(renderer, track_color.r, track_color.g, track_color.b, track_color.a);
    SDL_RenderFillRect(renderer, &track_rect);
    
    // Calculate fill width based on value
    float range = slider->max_value - slider->min_value;
    float normalized = (slider->current_value - slider->min_value) / range;
    float fill_width = content.width * normalized;
    
    // Draw filled portion
    SDL_Rect fill_rect = {
        (int)content_top_left.x,
        (int)track_y,
        (int)fill_width,
        (int)slider->track_height
    };
    SDL_Color fill_color = crystalline_color_to_sdl(slider->fill_color);
    SDL_SetRenderDrawColor(renderer, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
    SDL_RenderFillRect(renderer, &fill_rect);
    
    // Calculate handle position
    float handle_x = content_top_left.x + fill_width;
    float handle_y = content.center.y;
    
    // Draw handle
    CrystallineColor handle_color = (element->state == CRYSTALLINE_STATE_HOVER || slider->dragging) ?
        slider->handle_hover_color : slider->handle_color;
    
    SDL_Color handle_sdl = crystalline_color_to_sdl(handle_color);
    SDL_SetRenderDrawColor(renderer, handle_sdl.r, handle_sdl.g, handle_sdl.b, handle_sdl.a);
    
    // Draw handle as circle
    int handle_radius = (int)slider->handle_size;
    for (int y = -handle_radius; y <= handle_radius; y++) {
        for (int x = -handle_radius; x <= handle_radius; x++) {
            if (x*x + y*y <= handle_radius*handle_radius) {
                SDL_RenderDrawPoint(renderer, (int)handle_x + x, (int)handle_y + y);
            }
        }
    }
    
    // Draw value text if enabled
    if (slider->show_value && slider->font) {
        char value_text[32];
        snprintf(value_text, sizeof(value_text), "%.1f", slider->current_value);
        
        SDL_Color text_color = crystalline_color_to_sdl(slider->text_color);
        SDL_Surface* surface = TTF_RenderText_Blended(slider->font, value_text, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {
                    (int)(content.center.x - surface->w / 2),
                    (int)(content_top_left.y - surface->h - 5),
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

void crystalline_slider_render(
    CrystallineSlider* slider,
    SDL_Renderer* renderer
) {
    if (!slider) return;
    crystalline_element_render(&slider->base, renderer);
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

static void update_slider_value_from_mouse(CrystallineSlider* slider, int mouse_x) {
    CrystallineBounds content = crystalline_element_get_content_bounds(&slider->base);
    CrystallinePoint content_top_left = crystalline_bounds_top_left(content);
    
    // Calculate normalized position (0.0 to 1.0)
    float relative_x = mouse_x - content_top_left.x;
    float normalized = fmaxf(0.0f, fminf(relative_x / content.width, 1.0f));
    
    // Calculate value
    float range = slider->max_value - slider->min_value;
    float new_value = slider->min_value + normalized * range;
    
    crystalline_slider_set_value(slider, new_value);
}

static bool slider_handle_event_func(CrystallineElement* element, SDL_Event* event) {
    CrystallineSlider* slider = (CrystallineSlider*)element;
    
    if (!element->enabled) return false;
    
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        if (crystalline_element_contains_point(element, event->button.x, event->button.y)) {
            slider->dragging = true;
            element->state = CRYSTALLINE_STATE_ACTIVE;
            update_slider_value_from_mouse(slider, event->button.x);
            return true;
        }
    } else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
        if (slider->dragging) {
            slider->dragging = false;
            element->state = CRYSTALLINE_STATE_NORMAL;
            return true;
        }
    } else if (event->type == SDL_MOUSEMOTION) {
        if (slider->dragging) {
            update_slider_value_from_mouse(slider, event->motion.x);
            return true;
        }
    }
    
    return false;
}

bool crystalline_slider_handle_event(
    CrystallineSlider* slider,
    SDL_Event* event
) {
    if (!slider) return false;
    return crystalline_element_handle_event(&slider->base, event);
}

// ============================================================================
// CLEANUP
// ============================================================================

static void slider_destroy_func(CrystallineElement* element) {
    // Slider-specific cleanup (if needed)
    (void)element;
}