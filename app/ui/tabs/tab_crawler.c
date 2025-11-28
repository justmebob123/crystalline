// app/ui/tabs/tab_crawler.c - Comprehensive Crawler Control Tab
#include "../../app_common.h"
#include "../../text_input.h"
#include "../../ui_layout.h"
#include "../../crawler_thread.h"
#include "../../../src/crawler/prime_randomization.h"
#include "../../../src/crawler/link_management.h"
#include "../../../src/crawler/url_patterns.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

typedef struct {
    // Prime Configuration
    CrawlerPrimeConfig prime_config;
    bool prime_enabled;
    char frequency_input[32];
    char selection_input[32];
    char delay_min_input[32];
    char delay_max_input[32];
    
    // Link Management
    LinkQueue* link_queue;
    char add_link_input[512];
    int link_list_scroll;
    bool show_add_confirmation;
    int confirmation_timer;
    
    // URL Pattern Selection
    bool pattern_href;
    bool pattern_onclick;
    bool pattern_data_attr;
    bool pattern_meta_refresh;
    
    // Status Display
    int pages_crawled;
    int links_in_queue;
    char current_url[512];
    uint64_t next_crawl_time;
    
    // UI State
    bool show_prime_panel;
    bool show_link_panel;
    bool show_pattern_panel;
    bool show_status_panel;
    
    // Text inputs
    TextInput url_input;
    TextInput add_link_text_input;
    bool inputs_initialized;
    
    // Active input field
    int active_input; // 0=none, 1=frequency, 2=selection, 3=delay_min, 4=delay_max, 5=add_link
} CrawlerTabState;

static CrawlerTabState g_crawler_state = {0};

// ============================================================================
// INITIALIZATION
// ============================================================================

static void init_crawler_tab_state(void) {
    if (g_crawler_state.inputs_initialized) return;
    
    // Initialize prime config with defaults
    prime_config_init_default(&g_crawler_state.prime_config);
    g_crawler_state.prime_enabled = true;
    
    // Convert defaults to strings
    snprintf(g_crawler_state.frequency_input, sizeof(g_crawler_state.frequency_input), 
             "%lu", (unsigned long)g_crawler_state.prime_config.frequency_prime);
    snprintf(g_crawler_state.selection_input, sizeof(g_crawler_state.selection_input),
             "%lu", (unsigned long)g_crawler_state.prime_config.link_selection_prime);
    snprintf(g_crawler_state.delay_min_input, sizeof(g_crawler_state.delay_min_input),
             "%lu", (unsigned long)g_crawler_state.prime_config.delay_min_prime);
    snprintf(g_crawler_state.delay_max_input, sizeof(g_crawler_state.delay_max_input),
             "%lu", (unsigned long)g_crawler_state.prime_config.delay_max_prime);
    
    // Initialize link queue
    g_crawler_state.link_queue = link_queue_create("data/crawler/link_queue.txt");
    
    // Enable all URL patterns by default
    g_crawler_state.pattern_href = true;
    g_crawler_state.pattern_onclick = true;
    g_crawler_state.pattern_data_attr = true;
    g_crawler_state.pattern_meta_refresh = true;
    
    // Initialize UI panels (all expanded by default)
    g_crawler_state.show_prime_panel = true;
    g_crawler_state.show_link_panel = true;
    g_crawler_state.show_pattern_panel = true;
    g_crawler_state.show_status_panel = true;
    
    g_crawler_state.inputs_initialized = true;
}

// ============================================================================
// PRIME VALIDATION
// ============================================================================

static bool validate_prime_input(const char* input, uint64_t* value) {
    char* endptr;
    unsigned long val = strtoul(input, &endptr, 10);
    
    if (*endptr != '\0' || val == 0) {
        return false;
    }
    
    if (!is_prime(val)) {
        return false;
    }
    
    *value = val;
    return true;
}

// ============================================================================
// DRAWING HELPERS
// ============================================================================

static bool is_mouse_over_rect(SDL_Rect rect, int mouse_x, int mouse_y) {
    return mouse_x >= rect.x && mouse_x < rect.x + rect.w &&
           mouse_y >= rect.y && mouse_y < rect.y + rect.h;
}

static void draw_button_rect(SDL_Renderer* renderer, SDL_Rect rect, const char* text, 
                             SDL_Color bg_color, SDL_Color text_color, bool is_hovered) {
    if (is_hovered) {
        SDL_SetRenderDrawColor(renderer, 
            bg_color.r + 20, bg_color.g + 20, bg_color.b + 20, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    }
    SDL_RenderFillRect(renderer, &rect);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Text (centered)
    draw_text(renderer, text, rect.x + (rect.w - strlen(text) * 7) / 2, 
              rect.y + (rect.h - 12) / 2, text_color);
}

// ============================================================================
// MAIN DRAWING FUNCTION
// ============================================================================

void draw_crawler_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    init_crawler_tab_state();
    
    int panel_x = RENDER_WIDTH;
    int panel_y = 60;
    int panel_width = CONTROL_PANEL_WIDTH;
    
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {40, 40, 50, 255};
    SDL_Color button_color = {60, 60, 80, 255};
    SDL_Color active_color = {100, 150, 200, 255};
    SDL_Color success_color = {100, 200, 100, 255};
    SDL_Color error_color = {200, 100, 100, 255};
    SDL_Color dim_color = {150, 150, 150, 255};
    
    // Draw panel background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_Rect panel_rect = {panel_x, panel_y, panel_width, WINDOW_HEIGHT - panel_y};
    SDL_RenderFillRect(renderer, &panel_rect);
    
    int y = panel_y + 10;
    int x = panel_x + 10;
    
    // === HEADER ===
    draw_text(renderer, "WEB CRAWLER", x, y, text_color);
    y += 25;
    
    // === STATUS SECTION ===
    bool is_running = is_crawler_running();
    const char* status_text = is_running ? "Running" : "Stopped";
    SDL_Color status_color = is_running ? success_color : dim_color;
    
    draw_text(renderer, "Status:", x, y, text_color);
    draw_text(renderer, status_text, x + 60, y, status_color);
    y += 20;
    
    // Get current stats
    int pages_crawled = 0;
    int queue_size = 0;
    get_crawler_status(&pages_crawled, &queue_size);
    
    char stats_text[64];
    snprintf(stats_text, sizeof(stats_text), "Pages: %d", pages_crawled);
    draw_text(renderer, stats_text, x, y, text_color);
    y += 20;
    
    int link_count = g_crawler_state.link_queue ? 
                     link_queue_size(g_crawler_state.link_queue) : 0;
    snprintf(stats_text, sizeof(stats_text), "Queue: %d", link_count);
    draw_text(renderer, stats_text, x, y, text_color);
    y += 25;
    
    // === PRIME CONFIGURATION ===
    draw_text(renderer, "PRIME CONFIG", x, y, text_color);
    y += 20;
    
    // Enable/Disable toggle
    SDL_Rect toggle_rect = {x, y, 100, 22};
    const char* toggle_text = g_crawler_state.prime_enabled ? "Enabled" : "Disabled";
    SDL_Color toggle_color = g_crawler_state.prime_enabled ? success_color : error_color;
    draw_button_rect(renderer, toggle_rect, toggle_text, toggle_color, text_color, false);
    y += 30;
    
    if (g_crawler_state.prime_enabled) {
        // Frequency Prime
        draw_text(renderer, "Frequency:", x, y, text_color);
        y += 18;
        
        SDL_Rect freq_rect = {x, y, 120, 22};
        uint64_t freq_val;
        bool freq_valid = validate_prime_input(g_crawler_state.frequency_input, &freq_val);
        SDL_Color freq_color = freq_valid ? success_color : error_color;
        
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderFillRect(renderer, &freq_rect);
        SDL_SetRenderDrawColor(renderer, freq_color.r, freq_color.g, freq_color.b, 255);
        SDL_RenderDrawRect(renderer, &freq_rect);
        
        draw_text(renderer, g_crawler_state.frequency_input, x + 5, y + 5, text_color);
        
        if (freq_valid) {
            draw_text(renderer, "OK", x + 130, y + 5, success_color);
        } else {
            draw_text(renderer, "!", x + 130, y + 5, error_color);
        }
        y += 30;
        
        // Selection Prime
        draw_text(renderer, "Selection:", x, y, text_color);
        y += 18;
        
        SDL_Rect sel_rect = {x, y, 120, 22};
        uint64_t sel_val;
        bool sel_valid = validate_prime_input(g_crawler_state.selection_input, &sel_val);
        SDL_Color sel_color = sel_valid ? success_color : error_color;
        
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderFillRect(renderer, &sel_rect);
        SDL_SetRenderDrawColor(renderer, sel_color.r, sel_color.g, sel_color.b, 255);
        SDL_RenderDrawRect(renderer, &sel_rect);
        
        draw_text(renderer, g_crawler_state.selection_input, x + 5, y + 5, text_color);
        
        if (sel_valid) {
            draw_text(renderer, "OK", x + 130, y + 5, success_color);
        } else {
            draw_text(renderer, "!", x + 130, y + 5, error_color);
        }
        y += 30;
        
        // Delay Min
        draw_text(renderer, "Delay Min:", x, y, text_color);
        y += 18;
        
        SDL_Rect min_rect = {x, y, 120, 22};
        uint64_t min_val;
        bool min_valid = validate_prime_input(g_crawler_state.delay_min_input, &min_val);
        SDL_Color min_color = min_valid ? success_color : error_color;
        
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderFillRect(renderer, &min_rect);
        SDL_SetRenderDrawColor(renderer, min_color.r, min_color.g, min_color.b, 255);
        SDL_RenderDrawRect(renderer, &min_rect);
        
        draw_text(renderer, g_crawler_state.delay_min_input, x + 5, y + 5, text_color);
        
        if (min_valid) {
            draw_text(renderer, "OK", x + 130, y + 5, success_color);
        } else {
            draw_text(renderer, "!", x + 130, y + 5, error_color);
        }
        y += 30;
        
        // Delay Max
        draw_text(renderer, "Delay Max:", x, y, text_color);
        y += 18;
        
        SDL_Rect max_rect = {x, y, 120, 22};
        uint64_t max_val;
        bool max_valid = validate_prime_input(g_crawler_state.delay_max_input, &max_val);
        SDL_Color max_color = max_valid ? success_color : error_color;
        
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderFillRect(renderer, &max_rect);
        SDL_SetRenderDrawColor(renderer, max_color.r, max_color.g, max_color.b, 255);
        SDL_RenderDrawRect(renderer, &max_rect);
        
        draw_text(renderer, g_crawler_state.delay_max_input, x + 5, y + 5, text_color);
        
        if (max_valid) {
            draw_text(renderer, "OK", x + 130, y + 5, success_color);
        } else {
            draw_text(renderer, "!", x + 130, y + 5, error_color);
        }
        y += 30;
        
        // Apply button
        bool all_valid = freq_valid && sel_valid && min_valid && max_valid;
        if (all_valid) {
            SDL_Rect apply_rect = {x, y, 120, 25};
            draw_button_rect(renderer, apply_rect, "Apply", button_color, text_color, false);
            y += 30;
        }
    }
    
    y += 10;
    
    // === LINK MANAGEMENT ===
    draw_text(renderer, "LINK QUEUE", x, y, text_color);
    y += 20;
    
    draw_text(renderer, "Add URL:", x, y, text_color);
    y += 18;
    
    SDL_Rect input_rect = {x, y, panel_width - 30, 22};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &input_rect);
    SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
    SDL_RenderDrawRect(renderer, &input_rect);
    
    // Truncate if too long for display
    char display_url[50];
    if (strlen(g_crawler_state.add_link_input) > 45) {
        strncpy(display_url, g_crawler_state.add_link_input, 42);
        display_url[42] = '.';
        display_url[43] = '.';
        display_url[44] = '.';
        display_url[45] = '\0';
    } else {
        strcpy(display_url, g_crawler_state.add_link_input);
    }
    
    draw_text(renderer, display_url, x + 5, y + 5, text_color);
    y += 30;
    
    // Add and Clear buttons
    SDL_Rect add_btn = {x, y, 60, 22};
    draw_button_rect(renderer, add_btn, "Add", button_color, text_color, false);
    
    SDL_Rect clear_btn = {x + 70, y, 60, 22};
    draw_button_rect(renderer, clear_btn, "Clear", button_color, text_color, false);
    y += 30;
    
    // Confirmation message
    if (g_crawler_state.show_add_confirmation) {
        draw_text(renderer, "Link added!", x, y, success_color);
        g_crawler_state.confirmation_timer--;
        if (g_crawler_state.confirmation_timer <= 0) {
            g_crawler_state.show_add_confirmation = false;
        }
        y += 20;
    }
    
    y += 10;
    
    // === URL PATTERNS ===
    draw_text(renderer, "URL PATTERNS", x, y, text_color);
    y += 20;
    
    const char* checkbox_on = "[X]";
    const char* checkbox_off = "[ ]";
    
    // HREF
    const char* href_box = g_crawler_state.pattern_href ? checkbox_on : checkbox_off;
    draw_text(renderer, href_box, x, y, text_color);
    draw_text(renderer, "href", x + 30, y, text_color);
    y += 18;
    
    // onclick
    const char* onclick_box = g_crawler_state.pattern_onclick ? checkbox_on : checkbox_off;
    draw_text(renderer, onclick_box, x, y, text_color);
    draw_text(renderer, "onclick", x + 30, y, text_color);
    y += 18;
    
    // data-attr
    const char* data_box = g_crawler_state.pattern_data_attr ? checkbox_on : checkbox_off;
    draw_text(renderer, data_box, x, y, text_color);
    draw_text(renderer, "data-*", x + 30, y, text_color);
    y += 18;
    
    // meta-refresh
    const char* meta_box = g_crawler_state.pattern_meta_refresh ? checkbox_on : checkbox_off;
    draw_text(renderer, meta_box, x, y, text_color);
    draw_text(renderer, "meta", x + 30, y, text_color);
    y += 25;
    
    // === CONTROL BUTTONS ===
    y = WINDOW_HEIGHT - 100;
    
    if (!is_running) {
        SDL_Rect start_btn = {x, y, 120, 30};
        draw_button_rect(renderer, start_btn, "Start", success_color, text_color, false);
    } else {
        SDL_Rect stop_btn = {x, y, 120, 30};
        draw_button_rect(renderer, stop_btn, "Stop", error_color, text_color, false);
    }
    
    y += 35;
    
    SDL_Rect save_btn = {x, y, 120, 25};
    draw_button_rect(renderer, save_btn, "Save Config", button_color, text_color, false);
}

// ============================================================================
// CLICK HANDLER
// ============================================================================

void handle_crawler_tab_click(AppState* state, int mouse_x, int mouse_y) {
    int panel_x = RENDER_WIDTH;
    int panel_y = 60;
    int x = panel_x + 10;
    int y = panel_y + 10;
    
    // Skip header
    y += 25;
    
    // Skip status
    y += 60;
    
    // Skip prime config header
    y += 20;
    
    // Toggle button
    SDL_Rect toggle_rect = {x, y, 100, 22};
    if (is_mouse_over_rect(toggle_rect, mouse_x, mouse_y)) {
        g_crawler_state.prime_enabled = !g_crawler_state.prime_enabled;
        return;
    }
    y += 30;
    
    if (g_crawler_state.prime_enabled) {
        // Skip all prime input fields for now
        y += 18 + 30 + 18 + 30 + 18 + 30 + 18 + 30;
        
        // Apply button
        uint64_t freq_val, sel_val, min_val, max_val;
        bool all_valid = validate_prime_input(g_crawler_state.frequency_input, &freq_val) &&
                        validate_prime_input(g_crawler_state.selection_input, &sel_val) &&
                        validate_prime_input(g_crawler_state.delay_min_input, &min_val) &&
                        validate_prime_input(g_crawler_state.delay_max_input, &max_val);
        
        if (all_valid) {
            SDL_Rect apply_rect = {x, y, 120, 25};
            if (is_mouse_over_rect(apply_rect, mouse_x, mouse_y)) {
                g_crawler_state.prime_config.frequency_prime = freq_val;
                g_crawler_state.prime_config.link_selection_prime = sel_val;
                g_crawler_state.prime_config.delay_min_prime = min_val;
                g_crawler_state.prime_config.delay_max_prime = max_val;
                g_crawler_state.prime_config.use_prime_randomization = true;
                return;
            }
            y += 30;
        }
    }
    
    y += 10;
    
    // Skip link management header
    y += 20 + 18 + 30;
    
    // Add button
    SDL_Rect add_btn = {x, y, 60, 22};
    if (is_mouse_over_rect(add_btn, mouse_x, mouse_y)) {
        if (strlen(g_crawler_state.add_link_input) > 0 && g_crawler_state.link_queue) {
            link_queue_add(g_crawler_state.link_queue, 
                          g_crawler_state.add_link_input, 5, "manual");
            g_crawler_state.add_link_input[0] = '\0';
            g_crawler_state.show_add_confirmation = true;
            g_crawler_state.confirmation_timer = 60;
        }
        return;
    }
    
    // Clear button
    SDL_Rect clear_btn = {x + 70, y, 60, 22};
    if (is_mouse_over_rect(clear_btn, mouse_x, mouse_y)) {
        g_crawler_state.add_link_input[0] = '\0';
        return;
    }
    
    // Control buttons at bottom
    y = WINDOW_HEIGHT - 100;
    
    bool is_running = is_crawler_running();
    
    if (!is_running) {
        SDL_Rect start_btn = {x, y, 120, 30};
        if (is_mouse_over_rect(start_btn, mouse_x, mouse_y)) {
            start_crawler_thread(state, "https://example.com");
            return;
        }
    } else {
        SDL_Rect stop_btn = {x, y, 120, 30};
        if (is_mouse_over_rect(stop_btn, mouse_x, mouse_y)) {
            stop_crawler_thread();
            return;
        }
    }
}

// ============================================================================
// KEYBOARD HANDLER
// ============================================================================

void handle_crawler_tab_keyboard(AppState* state, int key) {
    (void)state; // Unused
    
    // Handle text input for add link field
    if (key >= 32 && key <= 126) {
        size_t len = strlen(g_crawler_state.add_link_input);
        if (len < sizeof(g_crawler_state.add_link_input) - 1) {
            g_crawler_state.add_link_input[len] = (char)key;
            g_crawler_state.add_link_input[len + 1] = '\0';
        }
    } else if (key == SDLK_BACKSPACE) {
        size_t len = strlen(g_crawler_state.add_link_input);
        if (len > 0) {
            g_crawler_state.add_link_input[len - 1] = '\0';
        }
    }
}

// ============================================================================
// CLEANUP
// ============================================================================

void cleanup_crawler_tab(void) {
    if (g_crawler_state.link_queue) {
        link_queue_destroy(g_crawler_state.link_queue);
        g_crawler_state.link_queue = NULL;
    }
}