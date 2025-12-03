/*
 * Crystalline UI Elements - Implementation
 * 
 * Custom UI elements supporting both crystalline and rectangular styles
 * ALL calculations use prime_* functions
 */

#include "elements.h"
#include <stdlib.h>
#include <string.h>

/*
 * Helper Functions
 */

static SDL_Color get_element_color(CrystallineElementBase* base) {
    switch (base->state) {
        case CRYSTALLINE_STATE_HOVER:
            return base->hover_color;
        case CRYSTALLINE_STATE_ACTIVE:
            return base->active_color;
        case CRYSTALLINE_STATE_DISABLED:
            return base->disabled_color;
        case CRYSTALLINE_STATE_FOCUSED:
        case CRYSTALLINE_STATE_NORMAL:
        default:
            return base->color;
    }
}

/*
 * Button Implementation
 */

CrystallineButton* crystalline_button_create(CrystallineElementStyle style,
                                              float x, float y,
                                              float size_param1,
                                              float size_param2,
                                              const char* label,
                                              TTF_Font* font) {
    CrystallineButton* button = (CrystallineButton*)malloc(sizeof(CrystallineButton));
    if (!button) return NULL;
    
    memset(button, 0, sizeof(CrystallineButton));
    
    // Initialize base properties
    button->base.style = style;
    button->base.state = CRYSTALLINE_STATE_NORMAL;
    button->base.position = crystalline_point_cartesian(x, y);
    button->base.visible = true;
    button->base.enabled = true;
    
    // Set default colors
    button->base.color = crystalline_color_rgb(100, 150, 200);
    button->base.hover_color = crystalline_color_rgb(120, 170, 220);
    button->base.active_color = crystalline_color_rgb(80, 130, 180);
    button->base.disabled_color = crystalline_color_rgb(100, 100, 100);
    
    // Set label and font
    if (label) {
        strncpy(button->label, label, sizeof(button->label) - 1);
    }
    button->font = font;
    
    // Set size based on style
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        button->radius = size_param1;
        button->width = size_param1 * 2.0f;
        button->height = size_param1 * 2.0f;
        button->base.bounds = crystalline_rect_circle(x, y, size_param1);
    } else {
        button->width = size_param1;
        button->height = size_param2;
        button->radius = prime_fminf(size_param1, size_param2) / 2.0f;
        button->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
    }
    
    // Initialize pulse animation
    button->pulse = crystalline_anim_pulse(2.0f, 0.1f);  // 2 Hz, 10% amplitude
    button->pulse.base_value = 1.0f;
    
    return button;
}

void crystalline_button_destroy(CrystallineButton* button) {
    if (button) {
        free(button);
    }
}

void crystalline_button_render(CrystallineButton* button, SDL_Renderer* renderer) {
    if (!button || !button->base.visible) return;
    
    SDL_Color color = get_element_color(&button->base);
    
    // Get pulse value for animation
    float pulse_scale = crystalline_pulse_current(&button->pulse);
    
    if (button->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Circular button rendering
        float radius = button->radius * pulse_scale;
        
        // Draw button circle
        CrystallineDrawStyle style = crystalline_draw_style_both(
            color,
            crystalline_color_brightness(color, 0.8f),
            2.0f
        );
        crystalline_draw_circle(renderer, button->base.position, radius, style);
        
        // Draw glow effect on hover
        if (button->base.state == CRYSTALLINE_STATE_HOVER) {
            crystalline_draw_glow(renderer, button->base.position, radius, 
                                 button->base.hover_color, 0.3f);
        }
        
        // Draw label
        if (button->font && button->label[0]) {
            crystalline_draw_text_centered(renderer, button->label, 
                                          button->base.position,
                                          crystalline_color_rgb(255, 255, 255),
                                          button->font);
        }
    } else {
        // Rectangular button rendering
        float width = button->width * pulse_scale;
        float height = button->height * pulse_scale;
        
        CrystallineRect rect = crystalline_rect_create(
            button->base.position.x,
            button->base.position.y,
            width,
            height
        );
        
        // Draw button rectangle
        CrystallineDrawStyle style = crystalline_draw_style_both(
            color,
            crystalline_color_brightness(color, 0.8f),
            2.0f
        );
        crystalline_draw_rect(renderer, rect, style);
        
        // Draw label
        if (button->font && button->label[0]) {
            crystalline_draw_text_centered(renderer, button->label,
                                          button->base.position,
                                          crystalline_color_rgb(255, 255, 255),
                                          button->font);
        }
    }
}

void crystalline_button_update(CrystallineButton* button, float delta_time) {
    if (!button) return;
    
    // Update pulse animation only when hovered or active
    if (button->base.state == CRYSTALLINE_STATE_HOVER || 
        button->base.state == CRYSTALLINE_STATE_ACTIVE) {
        if (button->pulse.base.state != CRYSTALLINE_ANIM_PLAYING) {
            crystalline_anim_start(&button->pulse.base);
        }
        crystalline_anim_pulse_update(&button->pulse, delta_time);
    } else {
        if (button->pulse.base.state == CRYSTALLINE_ANIM_PLAYING) {
            crystalline_anim_stop(&button->pulse.base);
        }
    }
}

bool crystalline_button_handle_mouse(CrystallineButton* button, SDL_Event* event) {
    if (!button || !button->base.visible || !button->base.enabled) return false;
    
    float mouse_x = (float)event->button.x;
    float mouse_y = (float)event->button.y;
    
    bool inside = crystalline_rect_contains_point(button->base.bounds, 
                                                   crystalline_point_cartesian(mouse_x, mouse_y));
    
    switch (event->type) {
        case SDL_MOUSEMOTION:
            if (inside) {
                if (button->base.state == CRYSTALLINE_STATE_NORMAL) {
                    button->base.state = CRYSTALLINE_STATE_HOVER;
                }
            } else {
                if (button->base.state == CRYSTALLINE_STATE_HOVER) {
                    button->base.state = CRYSTALLINE_STATE_NORMAL;
                }
            }
            return inside;
            
        case SDL_MOUSEBUTTONDOWN:
            if (inside && event->button.button == SDL_BUTTON_LEFT) {
                button->base.state = CRYSTALLINE_STATE_ACTIVE;
                return true;
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (button->base.state == CRYSTALLINE_STATE_ACTIVE) {
                if (inside && event->button.button == SDL_BUTTON_LEFT) {
                    // Trigger click callback
                    if (button->on_click) {
                        button->on_click(button->base.user_data);
                    }
                }
                button->base.state = inside ? CRYSTALLINE_STATE_HOVER : CRYSTALLINE_STATE_NORMAL;
                return true;
            }
            break;
    }
    
    return false;
}

void crystalline_button_set_label(CrystallineButton* button, const char* label) {
    if (!button || !label) return;
    strncpy(button->label, label, sizeof(button->label) - 1);
    button->label[sizeof(button->label) - 1] = '\0';
}

void crystalline_button_set_callback(CrystallineButton* button, 
                                     void (*callback)(void* data), void* data) {
    if (!button) return;
    button->on_click = callback;
    button->base.user_data = data;
}

void crystalline_button_set_style(CrystallineButton* button, CrystallineElementStyle style) {
    if (!button) return;
    
    button->base.style = style;
    
    // Recalculate bounds based on new style
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        button->base.bounds = crystalline_rect_circle(
            button->base.position.x,
            button->base.position.y,
            button->radius
        );
    } else {
        button->base.bounds = crystalline_rect_create(
            button->base.position.x,
            button->base.position.y,
            button->width,
            button->height
        );
    }
}

/*
 * Slider Implementation
 */

CrystallineSlider* crystalline_slider_create(CrystallineElementStyle style,
                                              float x, float y,
                                              float size_param1,
                                              float size_param2,
                                              float min_value,
                                              float max_value) {
    CrystallineSlider* slider = (CrystallineSlider*)malloc(sizeof(CrystallineSlider));
    if (!slider) return NULL;
    
    memset(slider, 0, sizeof(CrystallineSlider));
    
    // Initialize base properties
    slider->base.style = style;
    slider->base.state = CRYSTALLINE_STATE_NORMAL;
    slider->base.position = crystalline_point_cartesian(x, y);
    slider->base.visible = true;
    slider->base.enabled = true;
    
    // Set default colors
    slider->base.color = crystalline_color_rgb(100, 150, 200);
    slider->base.hover_color = crystalline_color_rgb(120, 170, 220);
    slider->base.active_color = crystalline_color_rgb(80, 130, 180);
    slider->base.disabled_color = crystalline_color_rgb(100, 100, 100);
    
    // Set value range
    slider->min_value = min_value;
    slider->max_value = max_value;
    slider->value = 0.5f;  // Start at middle
    slider->step = 0.0f;   // Continuous by default
    
    // Set size based on style
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        slider->radius = size_param1;
        slider->start_angle = -M_PI * 0.75f;  // Start at 7:30
        slider->end_angle = M_PI * 0.75f;     // End at 4:30
        slider->base.bounds = crystalline_rect_circle(x, y, size_param1 + 20.0f);
    } else {
        slider->width = size_param1;
        slider->height = size_param2;
        slider->handle_size = prime_fminf(size_param1 * 0.1f, size_param2);
        slider->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
    }
    
    slider->dragging = false;
    
    return slider;
}

void crystalline_slider_destroy(CrystallineSlider* slider) {
    if (slider) {
        free(slider);
    }
}

void crystalline_slider_render(CrystallineSlider* slider, SDL_Renderer* renderer) {
    if (!slider || !slider->base.visible) return;
    
    SDL_Color color = get_element_color(&slider->base);
    
    if (slider->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Circular slider rendering (arc)
        
        // Draw track arc
        CrystallineDrawStyle track_style = crystalline_draw_style_stroked(
            crystalline_color_brightness(color, 0.5f), 4.0f
        );
        crystalline_draw_arc(renderer, slider->base.position, slider->radius,
                            slider->start_angle, slider->end_angle, track_style);
        
        // Draw filled arc up to current value
        float value_angle = slider->start_angle + 
                           (slider->end_angle - slider->start_angle) * slider->value;
        CrystallineDrawStyle value_style = crystalline_draw_style_stroked(color, 6.0f);
        crystalline_draw_arc(renderer, slider->base.position, slider->radius,
                            slider->start_angle, value_angle, value_style);
        
        // Draw handle at current position
        CrystallinePoint handle_pos = crystalline_point_polar(value_angle, slider->radius);
        handle_pos = crystalline_point_add(handle_pos, slider->base.position);
        
        CrystallineDrawStyle handle_style = crystalline_draw_style_both(
            color,
            crystalline_color_brightness(color, 0.8f),
            2.0f
        );
        crystalline_draw_circle(renderer, handle_pos, 8.0f, handle_style);
        
        // Draw glow on hover/active
        if (slider->base.state == CRYSTALLINE_STATE_HOVER || 
            slider->base.state == CRYSTALLINE_STATE_ACTIVE) {
            crystalline_draw_glow(renderer, handle_pos, 8.0f, color, 0.5f);
        }
    } else {
        // Rectangular slider rendering (horizontal bar)
        
        // Draw track
        CrystallineRect track = crystalline_rect_create(
            slider->base.position.x,
            slider->base.position.y,
            slider->width,
            slider->height
        );
        CrystallineDrawStyle track_style = crystalline_draw_style_both(
            crystalline_color_brightness(color, 0.3f),
            crystalline_color_brightness(color, 0.5f),
            1.0f
        );
        crystalline_draw_rect(renderer, track, track_style);
        
        // Draw filled portion
        float filled_width = slider->width * slider->value;
        CrystallineRect filled = crystalline_rect_create(
            slider->base.position.x - slider->width / 2.0f + filled_width / 2.0f,
            slider->base.position.y,
            filled_width,
            slider->height
        );
        CrystallineDrawStyle filled_style = crystalline_draw_style_filled(color);
        crystalline_draw_rect(renderer, filled, filled_style);
        
        // Draw handle
        float handle_x = slider->base.position.x - slider->width / 2.0f + 
                        slider->width * slider->value;
        CrystallinePoint handle_pos = crystalline_point_cartesian(
            handle_x, slider->base.position.y
        );
        
        CrystallineDrawStyle handle_style = crystalline_draw_style_both(
            color,
            crystalline_color_brightness(color, 0.8f),
            2.0f
        );
        crystalline_draw_circle(renderer, handle_pos, slider->handle_size, handle_style);
    }
    
    // Draw label if present
    if (slider->font && slider->label[0]) {
        CrystallinePoint label_pos = crystalline_point_cartesian(
            slider->base.position.x,
            slider->base.position.y - slider->radius - 20.0f
        );
        crystalline_draw_text_centered(renderer, slider->label, label_pos,
                                      crystalline_color_rgb(255, 255, 255),
                                      slider->font);
    }
}

void crystalline_slider_update(CrystallineSlider* slider, float delta_time) {
    if (!slider) return;
    // Slider doesn't need continuous updates (yet)
    (void)delta_time;
}

bool crystalline_slider_handle_mouse(CrystallineSlider* slider, SDL_Event* event) {
    if (!slider || !slider->base.visible || !slider->base.enabled) return false;
    
    float mouse_x = (float)event->button.x;
    float mouse_y = (float)event->button.y;
    CrystallinePoint mouse_pos = crystalline_point_cartesian(mouse_x, mouse_y);
    
    switch (event->type) {
        case SDL_MOUSEMOTION: {
            if (slider->dragging) {
                // Update value based on mouse position
                if (slider->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
                    // Calculate angle from center to mouse
                    float angle = crystalline_angle_between(slider->base.position, mouse_pos);
                    
                    // Clamp to slider range
                    float range = slider->end_angle - slider->start_angle;
                    float normalized = (angle - slider->start_angle) / range;
                    slider->value = crystalline_clamp(normalized, 0.0f, 1.0f);
                } else {
                    // Calculate position along track
                    float track_start = slider->base.position.x - slider->width / 2.0f;
                    float normalized = (mouse_x - track_start) / slider->width;
                    slider->value = crystalline_clamp(normalized, 0.0f, 1.0f);
                }
                
                // Apply step if set
                if (slider->step > 0.0f) {
                    float range = slider->max_value - slider->min_value;
                    float steps = range / slider->step;
                    slider->value = prime_round(slider->value * steps) / steps;
                }
                
                // Trigger callback
                if (slider->on_change) {
                    float actual_value = slider->min_value + 
                                       (slider->max_value - slider->min_value) * slider->value;
                    slider->on_change(actual_value, slider->base.user_data);
                }
                
                return true;
            } else {
                // Check for hover
                bool inside = crystalline_rect_contains_point(slider->base.bounds, mouse_pos);
                slider->base.state = inside ? CRYSTALLINE_STATE_HOVER : CRYSTALLINE_STATE_NORMAL;
                return inside;
            }
        }
            
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                bool inside = crystalline_rect_contains_point(slider->base.bounds, mouse_pos);
                if (inside) {
                    slider->dragging = true;
                    slider->base.state = CRYSTALLINE_STATE_ACTIVE;
                    return true;
                }
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_LEFT && slider->dragging) {
                slider->dragging = false;
                bool inside = crystalline_rect_contains_point(slider->base.bounds, mouse_pos);
                slider->base.state = inside ? CRYSTALLINE_STATE_HOVER : CRYSTALLINE_STATE_NORMAL;
                return true;
            }
            break;
    }
    
    return false;
}

void crystalline_slider_set_value(CrystallineSlider* slider, float value) {
    if (!slider) return;
    
    // Normalize to [0, 1]
    slider->value = (value - slider->min_value) / (slider->max_value - slider->min_value);
    slider->value = crystalline_clamp(slider->value, 0.0f, 1.0f);
}

void crystalline_slider_set_range(CrystallineSlider* slider, float min, float max) {
    if (!slider) return;
    slider->min_value = min;
    slider->max_value = max;
}

void crystalline_slider_set_callback(CrystallineSlider* slider,
                                     void (*callback)(float value, void* data), void* data) {
    if (!slider) return;
    slider->on_change = callback;
    slider->base.user_data = data;
}

float crystalline_slider_get_value(CrystallineSlider* slider) {
    if (!slider) return 0.0f;
    return slider->min_value + (slider->max_value - slider->min_value) * slider->value;
}

void crystalline_slider_set_style(CrystallineSlider* slider, CrystallineElementStyle style) {
    if (!slider) return;
    slider->base.style = style;
    
    // Recalculate bounds
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        slider->base.bounds = crystalline_rect_circle(
            slider->base.position.x,
            slider->base.position.y,
            slider->radius + 20.0f
        );
    } else {
        slider->base.bounds = crystalline_rect_create(
            slider->base.position.x,
            slider->base.position.y,
            slider->width,
            slider->height
        );
    }
}

/*
 * Utility Functions
 */

bool crystalline_element_contains_point(CrystallineElementBase* base, float x, float y) {
    if (!base) return false;
    CrystallinePoint point = crystalline_point_cartesian(x, y);
    return crystalline_rect_contains_point(base->bounds, point);
}

void crystalline_element_set_state(CrystallineElementBase* base, CrystallineElementState state) {
    if (!base) return;
    base->state = state;
}

void crystalline_element_set_enabled(CrystallineElementBase* base, bool enabled) {
    if (!base) return;
    base->enabled = enabled;
    if (!enabled) {
        base->state = CRYSTALLINE_STATE_DISABLED;
    }
}

void crystalline_element_set_visible(CrystallineElementBase* base, bool visible) {
    if (!base) return;
    base->visible = visible;
}

void crystalline_element_set_colors(CrystallineElementBase* base,
                                    SDL_Color normal,
                                    SDL_Color hover,
                                    SDL_Color active,
                                    SDL_Color disabled) {
    if (!base) return;
    base->color = normal;
    base->hover_color = hover;
    base->active_color = active;
    base->disabled_color = disabled;
}