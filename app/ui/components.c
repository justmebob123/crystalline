/**
 * UI Components Implementation
 */

#include "components.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool ui_point_in_rect(int x, int y, const SDL_Rect* rect) {
    return x >= rect->x && x < rect->x + rect->w &&
           y >= rect->y && y < rect->y + rect->h;
}

float ui_clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// ============================================================================
// BUTTON COMPONENT
// ============================================================================

UIButton* ui_button_create(int x, int y, int w, int h, const char* label) {
    UIButton* button = calloc(1, sizeof(UIButton));
    if (!button) return NULL;
    
    button->bounds = (SDL_Rect){x, y, w, h};
    if (label) {
        strncpy(button->label, label, sizeof(button->label) - 1);
    }
    
    button->state = UI_STATE_NORMAL;
    button->visible = true;
    
    // Default colors
    button->bg_normal = (SDL_Color){60, 60, 60, 255};
    button->bg_hover = (SDL_Color){80, 80, 80, 255};
    button->bg_active = (SDL_Color){100, 100, 100, 255};
    button->bg_disabled = (SDL_Color){40, 40, 40, 255};
    button->text_color = (SDL_Color){220, 220, 220, 255};
    button->border_color = (SDL_Color){100, 100, 100, 255};
    
    return button;
}

void ui_button_destroy(UIButton* button) {
    free(button);
}

void ui_button_render(UIButton* button, SDL_Renderer* renderer) {
    if (!button || !button->visible || !renderer) return;
    
    // Select background color based on state
    SDL_Color bg;
    switch (button->state) {
        case UI_STATE_HOVER:
            bg = button->bg_hover;
            break;
        case UI_STATE_ACTIVE:
            bg = button->bg_active;
            break;
        case UI_STATE_DISABLED:
            bg = button->bg_disabled;
            break;
        default:
            bg = button->bg_normal;
            break;
    }
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(renderer, &button->bounds);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 
                          button->border_color.r,
                          button->border_color.g,
                          button->border_color.b,
                          button->border_color.a);
    SDL_RenderDrawRect(renderer, &button->bounds);
    
    // Draw text (centered)
    if (button->label[0] != '\0') {
        int text_x = button->bounds.x + (button->bounds.w / 2) - (strlen(button->label) * 4);
        int text_y = button->bounds.y + (button->bounds.h / 2) - 6;
        draw_text(renderer, button->label, text_x, text_y, button->text_color);
    }
}

bool ui_button_handle_event(UIButton* button, SDL_Event* event) {
    if (!button || !button->visible || button->state == UI_STATE_DISABLED) {
        return false;
    }
    
    switch (event->type) {
        case SDL_MOUSEMOTION: {
            bool inside = ui_point_in_rect(event->motion.x, event->motion.y, &button->bounds);
            if (inside && button->state == UI_STATE_NORMAL) {
                button->state = UI_STATE_HOVER;
                return true;
            } else if (!inside && button->state == UI_STATE_HOVER) {
                button->state = UI_STATE_NORMAL;
                return true;
            }
            break;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                if (ui_point_in_rect(event->button.x, event->button.y, &button->bounds)) {
                    button->state = UI_STATE_ACTIVE;
                    return true;
                }
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                if (button->state == UI_STATE_ACTIVE) {
                    if (ui_point_in_rect(event->button.x, event->button.y, &button->bounds)) {
                        // Button clicked!
                        if (button->on_click) {
                            button->on_click(button->user_data);
                        }
                        button->state = UI_STATE_HOVER;
                    } else {
                        button->state = UI_STATE_NORMAL;
                    }
                    return true;
                }
            }
            break;
        }
    }
    
    return false;
}

void ui_button_set_callback(UIButton* button, ButtonCallback callback, void* user_data) {
    if (!button) return;
    button->on_click = callback;
    button->user_data = user_data;
}

void ui_button_set_enabled(UIButton* button, bool enabled) {
    if (!button) return;
    button->state = enabled ? UI_STATE_NORMAL : UI_STATE_DISABLED;
}

void ui_button_set_position(UIButton* button, int x, int y) {
    if (!button) return;
    button->bounds.x = x;
    button->bounds.y = y;
}

// ============================================================================
// TEXT INPUT COMPONENT
// ============================================================================

UITextInput* ui_text_input_create(int x, int y, int w, int h, const char* placeholder) {
    UITextInput* input = calloc(1, sizeof(UITextInput));
    if (!input) return NULL;
    
    input->bounds = (SDL_Rect){x, y, w, h};
    if (placeholder) {
        strncpy(input->placeholder, placeholder, sizeof(input->placeholder) - 1);
    }
    
    input->cursor_pos = 0;
    input->focused = false;
    input->visible = true;
    input->state = UI_STATE_NORMAL;
    
    // Default colors
    input->bg_color = (SDL_Color){30, 30, 30, 255};
    input->text_color = (SDL_Color){220, 220, 220, 255};
    input->placeholder_color = (SDL_Color){100, 100, 100, 255};
    input->border_color = (SDL_Color){80, 80, 80, 255};
    input->cursor_color = (SDL_Color){220, 220, 220, 255};
    
    return input;
}

void ui_text_input_destroy(UITextInput* input) {
    free(input);
}

void ui_text_input_render(UITextInput* input, SDL_Renderer* renderer) {
    if (!input || !input->visible || !renderer) return;
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, 
                          input->bg_color.r,
                          input->bg_color.g,
                          input->bg_color.b,
                          input->bg_color.a);
    SDL_RenderFillRect(renderer, &input->bounds);
    
    // Draw border (thicker if focused)
    SDL_Color border = input->border_color;
    if (input->focused) {
        border = (SDL_Color){120, 150, 200, 255};
    }
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(renderer, &input->bounds);
    
    // Draw text or placeholder
    int text_x = input->bounds.x + 5;
    int text_y = input->bounds.y + (input->bounds.h / 2) - 6;
    
    if (input->text[0] != '\0') {
        draw_text(renderer, input->text, text_x, text_y, input->text_color);
        
        // Draw cursor if focused
        if (input->focused) {
            int cursor_x = text_x + (input->cursor_pos * 8);
            SDL_SetRenderDrawColor(renderer, 
                                  input->cursor_color.r,
                                  input->cursor_color.g,
                                  input->cursor_color.b,
                                  input->cursor_color.a);
            SDL_RenderDrawLine(renderer, cursor_x, text_y, cursor_x, text_y + 12);
        }
    } else if (input->placeholder[0] != '\0') {
        draw_text(renderer, input->placeholder, text_x, text_y, input->placeholder_color);
    }
}

bool ui_text_input_handle_event(UITextInput* input, SDL_Event* event) {
    if (!input || !input->visible) return false;
    
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                bool inside = ui_point_in_rect(event->button.x, event->button.y, &input->bounds);
                input->focused = inside;
                return inside;
            }
            break;
        }
        
        case SDL_TEXTINPUT: {
            if (input->focused) {
                size_t len = strlen(input->text);
                if (len < sizeof(input->text) - 1) {
                    strcat(input->text, event->text.text);
                    input->cursor_pos = strlen(input->text);
                    
                    if (input->on_change) {
                        input->on_change(input->text, input->user_data);
                    }
                }
                return true;
            }
            break;
        }
        
        case SDL_KEYDOWN: {
            if (input->focused) {
                if (event->key.keysym.sym == SDLK_BACKSPACE) {
                    size_t len = strlen(input->text);
                    if (len > 0) {
                        input->text[len - 1] = '\0';
                        input->cursor_pos = len - 1;
                        
                        if (input->on_change) {
                            input->on_change(input->text, input->user_data);
                        }
                    }
                    return true;
                } else if (event->key.keysym.sym == SDLK_RETURN) {
                    if (input->on_submit) {
                        input->on_submit(input->text, input->user_data);
                    }
                    return true;
                }
            }
            break;
        }
    }
    
    return false;
}

void ui_text_input_set_text(UITextInput* input, const char* text) {
    if (!input) return;
    if (text) {
        strncpy(input->text, text, sizeof(input->text) - 1);
        input->text[sizeof(input->text) - 1] = '\0';
        input->cursor_pos = strlen(input->text);
    } else {
        input->text[0] = '\0';
        input->cursor_pos = 0;
    }
}

const char* ui_text_input_get_text(const UITextInput* input) {
    return input ? input->text : "";
}

void ui_text_input_set_callback(UITextInput* input, TextInputCallback on_change,
                                TextInputCallback on_submit, void* user_data) {
    if (!input) return;
    input->on_change = on_change;
    input->on_submit = on_submit;
    input->user_data = user_data;
}

void ui_text_input_set_validator(UITextInput* input, bool (*validator)(const char*)) {
    if (!input) return;
    input->validator = validator;
}

void ui_text_input_set_focus(UITextInput* input, bool focused) {
    if (!input) return;
    input->focused = focused;
}

// ============================================================================
// SLIDER COMPONENT
// ============================================================================

UISlider* ui_slider_create(int x, int y, int w, int h, float min, float max, const char* label) {
    UISlider* slider = calloc(1, sizeof(UISlider));
    if (!slider) return NULL;
    
    slider->bounds = (SDL_Rect){x, y, w, h};
    slider->value = 0.5f;
    slider->min_value = min;
    slider->max_value = max;
    slider->dragging = false;
    slider->visible = true;
    slider->state = UI_STATE_NORMAL;
    slider->show_value = true;
    
    if (label) {
        strncpy(slider->label, label, sizeof(slider->label) - 1);
    }
    
    // Default colors
    slider->track_color = (SDL_Color){60, 60, 60, 255};
    slider->fill_color = (SDL_Color){100, 150, 200, 255};
    slider->handle_color = (SDL_Color){220, 220, 220, 255};
    slider->text_color = (SDL_Color){220, 220, 220, 255};
    
    return slider;
}

void ui_slider_destroy(UISlider* slider) {
    free(slider);
}

void ui_slider_render(UISlider* slider, SDL_Renderer* renderer) {
    if (!slider || !slider->visible || !renderer) return;
    
    // Draw label
    if (slider->label[0] != '\0') {
        draw_text(renderer, slider->label, slider->bounds.x, slider->bounds.y - 20, slider->text_color);
    }
    
    // Draw track
    SDL_Rect track = {
        slider->bounds.x,
        slider->bounds.y + slider->bounds.h / 2 - 2,
        slider->bounds.w,
        4
    };
    SDL_SetRenderDrawColor(renderer, 
                          slider->track_color.r,
                          slider->track_color.g,
                          slider->track_color.b,
                          slider->track_color.a);
    SDL_RenderFillRect(renderer, &track);
    
    // Draw fill
    SDL_Rect fill = track;
    fill.w = (int)(slider->value * slider->bounds.w);
    SDL_SetRenderDrawColor(renderer,
                          slider->fill_color.r,
                          slider->fill_color.g,
                          slider->fill_color.b,
                          slider->fill_color.a);
    SDL_RenderFillRect(renderer, &fill);
    
    // Draw handle
    int handle_x = slider->bounds.x + (int)(slider->value * slider->bounds.w);
    int handle_y = slider->bounds.y + slider->bounds.h / 2;
    int handle_size = 12;
    
    SDL_Rect handle = {
        handle_x - handle_size / 2,
        handle_y - handle_size / 2,
        handle_size,
        handle_size
    };
    
    SDL_SetRenderDrawColor(renderer,
                          slider->handle_color.r,
                          slider->handle_color.g,
                          slider->handle_color.b,
                          slider->handle_color.a);
    SDL_RenderFillRect(renderer, &handle);
    
    // Draw value
    if (slider->show_value) {
        char value_str[32];
        float actual_value = slider->min_value + slider->value * (slider->max_value - slider->min_value);
        snprintf(value_str, sizeof(value_str), "%.1f", actual_value);
        draw_text(renderer, value_str, slider->bounds.x + slider->bounds.w + 10, 
                 slider->bounds.y + slider->bounds.h / 2 - 6, slider->text_color);
    }
}

bool ui_slider_handle_event(UISlider* slider, SDL_Event* event) {
    if (!slider || !slider->visible) return false;
    
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                if (ui_point_in_rect(event->button.x, event->button.y, &slider->bounds)) {
                    slider->dragging = true;
                    
                    // Update value
                    float new_value = (float)(event->button.x - slider->bounds.x) / slider->bounds.w;
                    slider->value = ui_clamp(new_value, 0.0f, 1.0f);
                    
                    if (slider->on_change) {
                        float actual_value = slider->min_value + slider->value * (slider->max_value - slider->min_value);
                        slider->on_change(actual_value, slider->user_data);
                    }
                    
                    return true;
                }
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                if (slider->dragging) {
                    slider->dragging = false;
                    return true;
                }
            }
            break;
        }
        
        case SDL_MOUSEMOTION: {
            if (slider->dragging) {
                float new_value = (float)(event->motion.x - slider->bounds.x) / slider->bounds.w;
                slider->value = ui_clamp(new_value, 0.0f, 1.0f);
                
                if (slider->on_change) {
                    float actual_value = slider->min_value + slider->value * (slider->max_value - slider->min_value);
                    slider->on_change(actual_value, slider->user_data);
                }
                
                return true;
            }
            break;
        }
    }
    
    return false;
}

void ui_slider_set_value(UISlider* slider, float value) {
    if (!slider) return;
    
    // Convert actual value to 0-1 range
    float normalized = (value - slider->min_value) / (slider->max_value - slider->min_value);
    slider->value = ui_clamp(normalized, 0.0f, 1.0f);
}

float ui_slider_get_value(const UISlider* slider) {
    if (!slider) return 0.0f;
    return slider->min_value + slider->value * (slider->max_value - slider->min_value);
}

int ui_slider_get_value_int(const UISlider* slider) {
    return (int)ui_slider_get_value(slider);
}

void ui_slider_set_callback(UISlider* slider, SliderCallback callback, void* user_data) {
    if (!slider) return;
    slider->on_change = callback;
    slider->user_data = user_data;
}

// ============================================================================
// PROGRESS BAR COMPONENT
// ============================================================================

UIProgressBar* ui_progress_bar_create(int x, int y, int w, int h) {
    UIProgressBar* bar = calloc(1, sizeof(UIProgressBar));
    if (!bar) return NULL;
    
    bar->bounds = (SDL_Rect){x, y, w, h};
    bar->progress = 0.0f;
    bar->visible = true;
    bar->show_percentage = true;
    
    // Default colors
    bar->bg_color = (SDL_Color){40, 40, 40, 255};
    bar->fill_color = (SDL_Color){100, 200, 100, 255};
    bar->text_color = (SDL_Color){220, 220, 220, 255};
    bar->border_color = (SDL_Color){80, 80, 80, 255};
    
    return bar;
}

void ui_progress_bar_destroy(UIProgressBar* bar) {
    free(bar);
}

void ui_progress_bar_render(UIProgressBar* bar, SDL_Renderer* renderer) {
    if (!bar || !bar->visible || !renderer) return;
    
    // Draw background
    SDL_SetRenderDrawColor(renderer,
                          bar->bg_color.r,
                          bar->bg_color.g,
                          bar->bg_color.b,
                          bar->bg_color.a);
    SDL_RenderFillRect(renderer, &bar->bounds);
    
    // Draw fill
    SDL_Rect fill = bar->bounds;
    fill.w = (int)(bar->progress * bar->bounds.w);
    SDL_SetRenderDrawColor(renderer,
                          bar->fill_color.r,
                          bar->fill_color.g,
                          bar->fill_color.b,
                          bar->fill_color.a);
    SDL_RenderFillRect(renderer, &fill);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer,
                          bar->border_color.r,
                          bar->border_color.g,
                          bar->border_color.b,
                          bar->border_color.a);
    SDL_RenderDrawRect(renderer, &bar->bounds);
    
    // Draw text
    char text[256];
    if (bar->show_percentage) {
        snprintf(text, sizeof(text), "%s %.1f%%", bar->label, bar->progress * 100.0f);
    } else {
        strncpy(text, bar->label, sizeof(text) - 1);
    }
    
    if (text[0] != '\0') {
        int text_x = bar->bounds.x + (bar->bounds.w / 2) - (strlen(text) * 4);
        int text_y = bar->bounds.y + (bar->bounds.h / 2) - 6;
        draw_text(renderer, text, text_x, text_y, bar->text_color);
    }
}

void ui_progress_bar_set_progress(UIProgressBar* bar, float progress) {
    if (!bar) return;
    bar->progress = ui_clamp(progress, 0.0f, 1.0f);
}

void ui_progress_bar_set_label(UIProgressBar* bar, const char* label) {
    if (!bar) return;
    if (label) {
        strncpy(bar->label, label, sizeof(bar->label) - 1);
        bar->label[sizeof(bar->label) - 1] = '\0';
    } else {
        bar->label[0] = '\0';
    }
}

// ============================================================================
// LABEL COMPONENT
// ============================================================================

UILabel* ui_label_create(int x, int y, int w, int h, const char* text) {
    UILabel* label = calloc(1, sizeof(UILabel));
    if (!label) return NULL;
    
    label->bounds = (SDL_Rect){x, y, w, h};
    if (text) {
        strncpy(label->text, text, sizeof(label->text) - 1);
    }
    
    label->visible = true;
    label->text_color = (SDL_Color){220, 220, 220, 255};
    label->bg_color = (SDL_Color){0, 0, 0, 0};
    label->has_background = false;
    label->align = LABEL_ALIGN_LEFT;
    
    return label;
}

void ui_label_destroy(UILabel* label) {
    free(label);
}

void ui_label_render(UILabel* label, SDL_Renderer* renderer) {
    if (!label || !label->visible || !renderer) return;
    
    // Draw background if enabled
    if (label->has_background) {
        SDL_SetRenderDrawColor(renderer,
                              label->bg_color.r,
                              label->bg_color.g,
                              label->bg_color.b,
                              label->bg_color.a);
        SDL_RenderFillRect(renderer, &label->bounds);
    }
    
    // Draw text
    if (label->text[0] != '\0') {
        int text_x = label->bounds.x;
        
        // Apply alignment
        if (label->align == LABEL_ALIGN_CENTER) {
            text_x = label->bounds.x + (label->bounds.w / 2) - (strlen(label->text) * 4);
        } else if (label->align == LABEL_ALIGN_RIGHT) {
            text_x = label->bounds.x + label->bounds.w - (strlen(label->text) * 8);
        }
        
        int text_y = label->bounds.y + (label->bounds.h / 2) - 6;
        draw_text(renderer, label->text, text_x, text_y, label->text_color);
    }
}

void ui_label_set_text(UILabel* label, const char* text) {
    if (!label) return;
    if (text) {
        strncpy(label->text, text, sizeof(label->text) - 1);
        label->text[sizeof(label->text) - 1] = '\0';
    } else {
        label->text[0] = '\0';
    }
}

void ui_label_set_color(UILabel* label, SDL_Color color) {
    if (!label) return;
    label->text_color = color;
}

// ============================================================================
// PANEL COMPONENT
// ============================================================================

UIPanel* ui_panel_create(int x, int y, int w, int h, const char* title) {
    UIPanel* panel = calloc(1, sizeof(UIPanel));
    if (!panel) return NULL;
    
    panel->bounds = (SDL_Rect){x, y, w, h};
    if (title) {
        strncpy(panel->title, title, sizeof(panel->title) - 1);
    }
    
    panel->visible = true;
    panel->collapsible = false;
    panel->collapsed = false;
    
    // Default colors
    panel->bg_color = (SDL_Color){30, 30, 30, 255};
    panel->border_color = (SDL_Color){80, 80, 80, 255};
    panel->title_bg_color = (SDL_Color){50, 50, 50, 255};
    panel->title_text_color = (SDL_Color){200, 220, 255, 255};
    
    return panel;
}

void ui_panel_destroy(UIPanel* panel) {
    free(panel);
}

void ui_panel_render(UIPanel* panel, SDL_Renderer* renderer) {
    if (!panel || !panel->visible || !renderer) return;
    
    int title_height = 30;
    
    // Draw title bar
    SDL_Rect title_rect = {panel->bounds.x, panel->bounds.y, panel->bounds.w, title_height};
    SDL_SetRenderDrawColor(renderer,
                          panel->title_bg_color.r,
                          panel->title_bg_color.g,
                          panel->title_bg_color.b,
                          panel->title_bg_color.a);
    SDL_RenderFillRect(renderer, &title_rect);
    
    // Draw title text
    if (panel->title[0] != '\0') {
        draw_text(renderer, panel->title, panel->bounds.x + 10, panel->bounds.y + 8, 
                 panel->title_text_color);
    }
    
    // Draw collapse indicator if collapsible
    if (panel->collapsible) {
        const char* indicator = panel->collapsed ? "+" : "-";
        draw_text(renderer, indicator, panel->bounds.x + panel->bounds.w - 20, 
                 panel->bounds.y + 8, panel->title_text_color);
    }
    
    // Draw content area if not collapsed
    if (!panel->collapsed) {
        SDL_Rect content_rect = {
            panel->bounds.x,
            panel->bounds.y + title_height,
            panel->bounds.w,
            panel->bounds.h - title_height
        };
        
        SDL_SetRenderDrawColor(renderer,
                              panel->bg_color.r,
                              panel->bg_color.g,
                              panel->bg_color.b,
                              panel->bg_color.a);
        SDL_RenderFillRect(renderer, &content_rect);
    }
    
    // Draw border
    SDL_SetRenderDrawColor(renderer,
                          panel->border_color.r,
                          panel->border_color.g,
                          panel->border_color.b,
                          panel->border_color.a);
    SDL_RenderDrawRect(renderer, &panel->bounds);
}

bool ui_panel_handle_event(UIPanel* panel, SDL_Event* event) {
    if (!panel || !panel->visible || !panel->collapsible) return false;
    
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        SDL_Rect title_rect = {panel->bounds.x, panel->bounds.y, panel->bounds.w, 30};
        if (ui_point_in_rect(event->button.x, event->button.y, &title_rect)) {
            panel->collapsed = !panel->collapsed;
            return true;
        }
    }
    
    return false;
}

void ui_panel_set_collapsible(UIPanel* panel, bool collapsible) {
    if (!panel) return;
    panel->collapsible = collapsible;
}

void ui_panel_set_collapsed(UIPanel* panel, bool collapsed) {
    if (!panel) return;
    panel->collapsed = collapsed;
}

bool ui_panel_is_collapsed(const UIPanel* panel) {
    return panel ? panel->collapsed : false;
}

SDL_Rect ui_panel_get_content_bounds(const UIPanel* panel) {
    if (!panel || panel->collapsed) {
        return (SDL_Rect){0, 0, 0, 0};
    }
    
    return (SDL_Rect){
        panel->bounds.x + 5,
        panel->bounds.y + 35,
        panel->bounds.w - 10,
        panel->bounds.h - 40
    };
}