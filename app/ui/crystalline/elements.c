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
/*
 * Progress Bar Implementation
 */

CrystallineProgress* crystalline_progress_create(CrystallineElementStyle style,
                                                  float x, float y,
                                                  float size_param1,
                                                  float size_param2) {
    CrystallineProgress* progress = (CrystallineProgress*)malloc(sizeof(CrystallineProgress));
    if (!progress) return NULL;
    
    memset(progress, 0, sizeof(CrystallineProgress));
    
    // Initialize base properties
    progress->base.style = style;
    progress->base.state = CRYSTALLINE_STATE_NORMAL;
    progress->base.position = crystalline_point_cartesian(x, y);
    progress->base.visible = true;
    progress->base.enabled = true;
    
    // Set default colors
    progress->base.color = crystalline_color_rgb(100, 200, 100);
    progress->base.hover_color = progress->base.color;
    progress->base.active_color = progress->base.color;
    progress->base.disabled_color = crystalline_color_rgb(100, 100, 100);
    
    // Initialize progress
    progress->progress = 0.0f;
    progress->show_percentage = true;
    
    // Set size based on style
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        progress->radius = size_param1;
        progress->thickness = size_param2 > 0 ? size_param2 : 10.0f;
        progress->start_angle = -M_PI / 2.0f;  // Start at top
        progress->base.bounds = crystalline_rect_circle(x, y, size_param1);
    } else {
        progress->width = size_param1;
        progress->height = size_param2;
        progress->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
    }
    
    // Initialize color animation
    progress->color_anim = crystalline_anim_color(
        crystalline_color_activity(0.0f),
        crystalline_color_activity(1.0f),
        1.0f
    );
    
    return progress;
}

void crystalline_progress_destroy(CrystallineProgress* progress) {
    if (progress) {
        free(progress);
    }
}

void crystalline_progress_render(CrystallineProgress* progress, SDL_Renderer* renderer) {
    if (!progress || !progress->base.visible) return;
    
    // Get color based on progress (activity-based)
    SDL_Color color = crystalline_color_activity(progress->progress);
    
    if (progress->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Circular progress (ring)
        
        // Draw background ring
        CrystallineDrawStyle bg_style = crystalline_draw_style_stroked(
            crystalline_color_brightness(color, 0.3f), progress->thickness
        );
        crystalline_draw_circle(renderer, progress->base.position, progress->radius, bg_style);
        
        // Draw progress arc
        float end_angle = progress->start_angle + CRYSTALLINE_TWO_PI * progress->progress;
        CrystallineDrawStyle progress_style = crystalline_draw_style_stroked(
            color, progress->thickness
        );
        crystalline_draw_arc(renderer, progress->base.position, progress->radius,
                            progress->start_angle, end_angle, progress_style);
        
        // Draw percentage text
        if (progress->show_percentage && progress->font) {
            char text[32];
            snprintf(text, sizeof(text), "%.0f%%", progress->progress * 100.0f);
            crystalline_draw_text_centered(renderer, text, progress->base.position,
                                          crystalline_color_rgb(255, 255, 255),
                                          progress->font);
        }
    } else {
        // Rectangular progress bar
        
        // Draw background
        CrystallineDrawStyle bg_style = crystalline_draw_style_filled(
            crystalline_color_brightness(color, 0.3f)
        );
        crystalline_draw_rect(renderer, progress->base.bounds, bg_style);
        
        // Draw progress fill
        float filled_width = progress->width * progress->progress;
        CrystallineRect filled = crystalline_rect_create(
            progress->base.position.x - progress->width / 2.0f + filled_width / 2.0f,
            progress->base.position.y,
            filled_width,
            progress->height
        );
        CrystallineDrawStyle fill_style = crystalline_draw_style_filled(color);
        crystalline_draw_rect(renderer, filled, fill_style);
        
        // Draw border
        CrystallineDrawStyle border_style = crystalline_draw_style_stroked(
            crystalline_color_brightness(color, 0.8f), 2.0f
        );
        crystalline_draw_rect(renderer, progress->base.bounds, border_style);
        
        // Draw percentage text
        if (progress->show_percentage && progress->font) {
            char text[32];
            snprintf(text, sizeof(text), "%.0f%%", progress->progress * 100.0f);
            crystalline_draw_text_centered(renderer, text, progress->base.position,
                                          crystalline_color_rgb(255, 255, 255),
                                          progress->font);
        }
    }
}

void crystalline_progress_update(CrystallineProgress* progress, float delta_time) {
    if (!progress) return;
    
    // Update color animation based on progress
    crystalline_anim_color_update(&progress->color_anim, delta_time);
}

void crystalline_progress_set_value(CrystallineProgress* progress, float value) {
    if (!progress) return;
    progress->progress = crystalline_clamp(value, 0.0f, 1.0f);
}

void crystalline_progress_set_label(CrystallineProgress* progress, const char* label) {
    if (!progress || !label) return;
    strncpy(progress->label, label, sizeof(progress->label) - 1);
    progress->label[sizeof(progress->label) - 1] = '\0';
}

float crystalline_progress_get_value(CrystallineProgress* progress) {
    if (!progress) return 0.0f;
    return progress->progress;
}

void crystalline_progress_set_style(CrystallineProgress* progress, CrystallineElementStyle style) {
    if (!progress) return;
    progress->base.style = style;
    
    // Recalculate bounds
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        progress->base.bounds = crystalline_rect_circle(
            progress->base.position.x,
            progress->base.position.y,
            progress->radius
        );
    } else {
        progress->base.bounds = crystalline_rect_create(
            progress->base.position.x,
            progress->base.position.y,
            progress->width,
            progress->height
        );
    }
}

/*
 * Input Field Implementation (Stub - Full implementation would be extensive)
 */

CrystallineInput* crystalline_input_create(CrystallineElementStyle style,
                                            float x, float y,
                                            float size_param1,
                                            float size_param2,
                                            const char* placeholder,
                                            TTF_Font* font) {
    CrystallineInput* input = (CrystallineInput*)malloc(sizeof(CrystallineInput));
    if (!input) return NULL;
    
    memset(input, 0, sizeof(CrystallineInput));
    
    input->base.style = style;
    input->base.state = CRYSTALLINE_STATE_NORMAL;
    input->base.position = crystalline_point_cartesian(x, y);
    input->base.visible = true;
    input->base.enabled = true;
    
    input->base.color = crystalline_color_rgb(80, 80, 100);
    input->base.hover_color = crystalline_color_rgb(100, 100, 120);
    input->base.active_color = crystalline_color_rgb(120, 120, 140);
    input->base.disabled_color = crystalline_color_rgb(60, 60, 60);
    
    if (placeholder) {
        strncpy(input->placeholder, placeholder, sizeof(input->placeholder) - 1);
    }
    input->font = font;
    
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        input->radius = size_param1;
        input->sides = 12;  // Dodecagon
        input->base.bounds = crystalline_rect_circle(x, y, size_param1);
    } else {
        input->width = size_param1;
        input->height = size_param2;
        input->padding = 5.0f;
        input->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
    }
    
    input->cursor_anim = crystalline_anim_pulse(1.0f, 1.0f);
    
    return input;
}

void crystalline_input_destroy(CrystallineInput* input) {
    if (input) {
        free(input);
    }
}

void crystalline_input_render(CrystallineInput* input, SDL_Renderer* renderer) {
    if (!input || !input->base.visible) return;
    
    SDL_Color color = get_element_color(&input->base);
    
    if (input->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        // Draw dodecagon border
        CrystallineDrawStyle style = crystalline_draw_style_stroked(color, 2.0f);
        crystalline_draw_polygon(renderer, input->base.position, input->radius, 
                                input->sides, 0.0f, style);
    } else {
        // Draw rectangular border
        CrystallineDrawStyle bg_style = crystalline_draw_style_filled(
            crystalline_color_brightness(color, 0.2f)
        );
        crystalline_draw_rect(renderer, input->base.bounds, bg_style);
        
        CrystallineDrawStyle border_style = crystalline_draw_style_stroked(color, 2.0f);
        crystalline_draw_rect(renderer, input->base.bounds, border_style);
    }
    
    // Draw text or placeholder
    const char* display_text = input->text[0] ? input->text : input->placeholder;
    if (input->font && display_text[0]) {
        SDL_Color text_color = input->text[0] ? 
            crystalline_color_rgb(255, 255, 255) : 
            crystalline_color_rgb(150, 150, 150);
        crystalline_draw_text_centered(renderer, display_text, input->base.position,
                                      text_color, input->font);
    }
}

void crystalline_input_update(CrystallineInput* input, float delta_time) {
    if (!input) return;
    crystalline_anim_pulse_update(&input->cursor_anim, delta_time);
}

bool crystalline_input_handle_mouse(CrystallineInput* input, SDL_Event* event) {
    if (!input || !input->base.visible || !input->base.enabled) return false;
    
    float mouse_x = (float)event->button.x;
    float mouse_y = (float)event->button.y;
    CrystallinePoint mouse_pos = crystalline_point_cartesian(mouse_x, mouse_y);
    
    bool inside = crystalline_rect_contains_point(input->base.bounds, mouse_pos);
    
    if (event->type == SDL_MOUSEBUTTONDOWN && inside) {
        input->base.state = CRYSTALLINE_STATE_FOCUSED;
        return true;
    }
    
    return false;
}

bool crystalline_input_handle_keyboard(CrystallineInput* input, SDL_Event* event) {
    if (!input || input->base.state != CRYSTALLINE_STATE_FOCUSED) return false;
    
    if (event->type == SDL_KEYDOWN) {
        // Basic text input handling (simplified)
        if (event->key.keysym.sym == SDLK_BACKSPACE && input->cursor_pos > 0) {
            input->text[--input->cursor_pos] = '\0';
            if (input->on_change) {
                input->on_change(input->text, input->base.user_data);
            }
            return true;
        } else if (event->key.keysym.sym == SDLK_RETURN) {
            if (input->on_submit) {
                input->on_submit(input->text, input->base.user_data);
            }
            return true;
        }
    } else if (event->type == SDL_TEXTINPUT) {
        // Add character
        if (input->cursor_pos < (int)sizeof(input->text) - 1) {
            input->text[input->cursor_pos++] = event->text.text[0];
            input->text[input->cursor_pos] = '\0';
            if (input->on_change) {
                input->on_change(input->text, input->base.user_data);
            }
            return true;
        }
    }
    
    return false;
}

void crystalline_input_set_text(CrystallineInput* input, const char* text) {
    if (!input || !text) return;
    strncpy(input->text, text, sizeof(input->text) - 1);
    input->text[sizeof(input->text) - 1] = '\0';
    input->cursor_pos = strlen(input->text);
}

void crystalline_input_set_placeholder(CrystallineInput* input, const char* placeholder) {
    if (!input || !placeholder) return;
    strncpy(input->placeholder, placeholder, sizeof(input->placeholder) - 1);
    input->placeholder[sizeof(input->placeholder) - 1] = '\0';
}

void crystalline_input_set_password_mode(CrystallineInput* input, bool enabled) {
    if (!input) return;
    input->password_mode = enabled;
}

void crystalline_input_set_callbacks(CrystallineInput* input,
                                     void (*on_change)(const char* text, void* data),
                                     void (*on_submit)(const char* text, void* data),
                                     void* data) {
    if (!input) return;
    input->on_change = on_change;
    input->on_submit = on_submit;
    input->base.user_data = data;
}

const char* crystalline_input_get_text(CrystallineInput* input) {
    if (!input) return "";
    return input->text;
}

void crystalline_input_set_style(CrystallineInput* input, CrystallineElementStyle style) {
    if (!input) return;
    input->base.style = style;
}

/*
 * Stub implementations for remaining elements
 * (Panel, List, Toggle, Dropdown - to be fully implemented)
 */

CrystallinePanel* crystalline_panel_create(CrystallineElementStyle style,
                                            float x, float y,
                                            float size_param1,
                                            float size_param2,
                                            const char* title,
                                            TTF_Font* title_font) {
    CrystallinePanel* panel = (CrystallinePanel*)malloc(sizeof(CrystallinePanel));
    if (!panel) return NULL;
    memset(panel, 0, sizeof(CrystallinePanel));
    
    panel->base.style = style;
    panel->base.position = crystalline_point_cartesian(x, y);
    panel->base.visible = true;
    panel->base.enabled = true;
    
    if (title) strncpy(panel->title, title, sizeof(panel->title) - 1);
    panel->title_font = title_font;
    
    if (style == CRYSTALLINE_STYLE_CIRCULAR) {
        panel->radius = size_param1;
        panel->base.bounds = crystalline_rect_circle(x, y, size_param1);
    } else {
        panel->width = size_param1;
        panel->height = size_param2;
        panel->corner_radius = 5.0f;
        panel->border_width = 2.0f;
        panel->padding = 10.0f;
        panel->base.bounds = crystalline_rect_create(x, y, size_param1, size_param2);
    }
    
    return panel;
}

void crystalline_panel_destroy(CrystallinePanel* panel) {
    if (panel) free(panel);
}

void crystalline_panel_render(CrystallinePanel* panel, SDL_Renderer* renderer) {
    if (!panel || !panel->base.visible) return;
    
    SDL_Color color = crystalline_color_rgb(40, 40, 50);
    SDL_Color border_color = crystalline_color_rgb(100, 100, 120);
    
    if (panel->base.style == CRYSTALLINE_STYLE_CIRCULAR) {
        CrystallineDrawStyle style = crystalline_draw_style_both(color, border_color, 2.0f);
        crystalline_draw_circle(renderer, panel->base.position, panel->radius, style);
        
        if (panel->show_flower_border) {
            CrystallineDrawStyle flower_style = crystalline_draw_style_stroked(border_color, 1.0f);
            crystalline_draw_flower_of_life(renderer, panel->base.position, 
                                           panel->radius / 3.0f, 1, flower_style);
        }
    } else {
        CrystallineDrawStyle bg_style = crystalline_draw_style_filled(color);
        crystalline_draw_rect(renderer, panel->base.bounds, bg_style);
        
        CrystallineDrawStyle border_style = crystalline_draw_style_stroked(border_color, panel->border_width);
        crystalline_draw_rect(renderer, panel->base.bounds, border_style);
    }
    
    // Draw title
    if (panel->title_font && panel->title[0]) {
        CrystallinePoint title_pos = crystalline_point_cartesian(
            panel->base.position.x,
            panel->base.position.y - panel->height / 2.0f + 20.0f
        );
        crystalline_draw_text_centered(renderer, panel->title, title_pos,
                                      crystalline_color_rgb(255, 255, 255),
                                      panel->title_font);
    }
}

void crystalline_panel_set_style(CrystallinePanel* panel, CrystallineElementStyle style) {
    if (!panel) return;
    panel->base.style = style;
}

// Stub implementations for List, Toggle, Dropdown
CrystallineList* crystalline_list_create(CrystallineElementStyle style, float x, float y,
                                          float size_param1, float size_param2, TTF_Font* font) {
    CrystallineList* list = (CrystallineList*)malloc(sizeof(CrystallineList));
    if (!list) return NULL;
    memset(list, 0, sizeof(CrystallineList));
    list->base.style = style;
    list->base.position = crystalline_point_cartesian(x, y);
    list->font = font;
    list->selected_index = -1;
    list->hover_index = -1;
    return list;
}

void crystalline_list_destroy(CrystallineList* list) { if (list) free(list); }
void crystalline_list_render(CrystallineList* list, SDL_Renderer* renderer) { (void)list; (void)renderer; }
bool crystalline_list_handle_mouse(CrystallineList* list, SDL_Event* event) { (void)list; (void)event; return false; }
bool crystalline_list_handle_keyboard(CrystallineList* list, SDL_Event* event) { (void)list; (void)event; return false; }
void crystalline_list_set_items(CrystallineList* list, char** items, int count) { (void)list; (void)items; (void)count; }
void crystalline_list_add_item(CrystallineList* list, const char* item) { (void)list; (void)item; }
void crystalline_list_remove_item(CrystallineList* list, int index) { (void)list; (void)index; }
void crystalline_list_clear(CrystallineList* list) { (void)list; }
void crystalline_list_set_selected(CrystallineList* list, int index) { if (list) list->selected_index = index; }
void crystalline_list_set_callback(CrystallineList* list, void (*callback)(int, void*), void* data) { 
    if (list) { list->on_select = callback; list->base.user_data = data; }
}
int crystalline_list_get_selected(CrystallineList* list) { return list ? list->selected_index : -1; }
void crystalline_list_set_style(CrystallineList* list, CrystallineElementStyle style) { if (list) list->base.style = style; }

CrystallineToggle* crystalline_toggle_create(CrystallineElementStyle style, float x, float y,
                                              float size_param1, float size_param2,
                                              const char* label, TTF_Font* font) {
    CrystallineToggle* toggle = (CrystallineToggle*)malloc(sizeof(CrystallineToggle));
    if (!toggle) return NULL;
    memset(toggle, 0, sizeof(CrystallineToggle));
    toggle->base.style = style;
    toggle->base.position = crystalline_point_cartesian(x, y);
    if (label) strncpy(toggle->label, label, sizeof(toggle->label) - 1);
    toggle->font = font;
    return toggle;
}

void crystalline_toggle_destroy(CrystallineToggle* toggle) { if (toggle) free(toggle); }
void crystalline_toggle_render(CrystallineToggle* toggle, SDL_Renderer* renderer) { (void)toggle; (void)renderer; }
void crystalline_toggle_update(CrystallineToggle* toggle, float delta_time) { (void)toggle; (void)delta_time; }
bool crystalline_toggle_handle_mouse(CrystallineToggle* toggle, SDL_Event* event) { (void)toggle; (void)event; return false; }
void crystalline_toggle_set_value(CrystallineToggle* toggle, bool value) { if (toggle) toggle->value = value; }
void crystalline_toggle_set_callback(CrystallineToggle* toggle, void (*callback)(bool, void*), void* data) {
    if (toggle) { toggle->on_change = callback; toggle->base.user_data = data; }
}
bool crystalline_toggle_get_value(CrystallineToggle* toggle) { return toggle ? toggle->value : false; }
void crystalline_toggle_set_style(CrystallineToggle* toggle, CrystallineElementStyle style) { if (toggle) toggle->base.style = style; }

CrystallineDropdown* crystalline_dropdown_create(CrystallineElementStyle style, float x, float y,
                                                  float size_param1, float size_param2, TTF_Font* font) {
    CrystallineDropdown* dropdown = (CrystallineDropdown*)malloc(sizeof(CrystallineDropdown));
    if (!dropdown) return NULL;
    memset(dropdown, 0, sizeof(CrystallineDropdown));
    dropdown->base.style = style;
    dropdown->base.position = crystalline_point_cartesian(x, y);
    dropdown->font = font;
    dropdown->selected_index = -1;
    dropdown->hover_index = -1;
    return dropdown;
}

void crystalline_dropdown_destroy(CrystallineDropdown* dropdown) { if (dropdown) free(dropdown); }
void crystalline_dropdown_render(CrystallineDropdown* dropdown, SDL_Renderer* renderer) { (void)dropdown; (void)renderer; }
void crystalline_dropdown_update(CrystallineDropdown* dropdown, float delta_time) { (void)dropdown; (void)delta_time; }
bool crystalline_dropdown_handle_mouse(CrystallineDropdown* dropdown, SDL_Event* event) { (void)dropdown; (void)event; return false; }
bool crystalline_dropdown_handle_keyboard(CrystallineDropdown* dropdown, SDL_Event* event) { (void)dropdown; (void)event; return false; }
void crystalline_dropdown_set_options(CrystallineDropdown* dropdown, char** options, int count) { (void)dropdown; (void)options; (void)count; }
void crystalline_dropdown_set_selected(CrystallineDropdown* dropdown, int index) { if (dropdown) dropdown->selected_index = index; }
void crystalline_dropdown_set_callback(CrystallineDropdown* dropdown, void (*callback)(int, void*), void* data) {
    if (dropdown) { dropdown->on_select = callback; dropdown->base.user_data = data; }
}
int crystalline_dropdown_get_selected(CrystallineDropdown* dropdown) { return dropdown ? dropdown->selected_index : -1; }
void crystalline_dropdown_set_style(CrystallineDropdown* dropdown, CrystallineElementStyle style) { if (dropdown) dropdown->base.style = style; }
