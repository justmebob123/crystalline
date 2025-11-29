// app/ui/tabs/tab_crawler_redesigned.c - Redesigned Crawler Tab with 3-Column Layout
#include "../../app_common.h"
#include "../../text_input.h"
#include "../../crawler_thread.h"
#include "../layout_manager.h"
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
    
    // Activity Log
    char activity_log[10][256];
    int activity_count;
    
    // UI State
    bool inputs_initialized;
    int active_input; // 0=none, 1-4=prime inputs, 5=add_link
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
// ACTIVITY LOG
// ============================================================================

static void add_activity(const char* message) {
    if (g_crawler_state.activity_count >= 10) {
        // Shift log up
        for (int i = 0; i < 9; i++) {
            strcpy(g_crawler_state.activity_log[i], g_crawler_state.activity_log[i + 1]);
        }
        g_crawler_state.activity_count = 9;
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[16];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
    
    // Add new entry
    snprintf(g_crawler_state.activity_log[g_crawler_state.activity_count],
             sizeof(g_crawler_state.activity_log[0]),
             "%s - %s", time_str, message);
    g_crawler_state.activity_count++;
}

// ============================================================================
// DRAWING HELPERS
// ============================================================================

static void draw_panel_background(SDL_Renderer* renderer, const ColumnLayout* col, SDL_Color bg_color) {
    SDL_Rect panel = {col->x, col->y, col->width, col->height};
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &panel);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255);
    SDL_RenderDrawRect(renderer, &panel);
}

static void draw_section_header(SDL_Renderer* renderer, const char* title, int x, int y, SDL_Color color) {
    draw_text(renderer, title, x, y, color);
    
    // Underline
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderDrawLine(renderer, x, y + 18, x + strlen(title) * 8, y + 18);
}

static bool draw_button_rect(SDL_Renderer* renderer, SDL_Rect rect, const char* text,
                             SDL_Color bg_color, SDL_Color text_color, int mouse_x, int mouse_y) {
    bool is_hovered = (mouse_x >= rect.x && mouse_x < rect.x + rect.w &&
                       mouse_y >= rect.y && mouse_y < rect.y + rect.h);
    
    if (is_hovered) {
        SDL_SetRenderDrawColor(renderer, bg_color.r + 20, bg_color.g + 20, bg_color.b + 20, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    }
    SDL_RenderFillRect(renderer, &rect);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Text (centered)
    int text_x = rect.x + (rect.w - strlen(text) * 7) / 2;
    int text_y = rect.y + (rect.h - 12) / 2;
    draw_text(renderer, text, text_x, text_y, text_color);
    
    return is_hovered;
}

// ============================================================================
// COLUMN 1: PRIME CONFIGURATION & URL PATTERNS
// ============================================================================

static void draw_column1_prime_config(SDL_Renderer* renderer, const ColumnLayout* col,
                                      SDL_Color text_color, SDL_Color success_color,
                                      SDL_Color error_color, int mouse_x, int mouse_y) {
    int x = col->x + col->padding;
    int y = col->y + col->padding;
    
    // Section header
    draw_section_header(renderer, "PRIME CONFIGURATION", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    // Enable/Disable toggle
    SDL_Rect toggle_rect = {x, y, 120, 25};
    const char* toggle_text = g_crawler_state.prime_enabled ? "Enabled" : "Disabled";
    SDL_Color toggle_color = g_crawler_state.prime_enabled ? success_color : error_color;
    draw_button_rect(renderer, toggle_rect, toggle_text, toggle_color, text_color, mouse_x, mouse_y);
    y += 35;
    
    if (g_crawler_state.prime_enabled) {
        // Frequency Prime
        draw_text(renderer, "Frequency:", x, y, text_color);
        y += 18;
        
        uint64_t freq_val;
        bool freq_valid = validate_prime_input(g_crawler_state.frequency_input, &freq_val);
        SDL_Color freq_color = freq_valid ? success_color : error_color;
        
        SDL_Rect freq_rect = {x, y, 150, 22};
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderFillRect(renderer, &freq_rect);
        SDL_SetRenderDrawColor(renderer, freq_color.r, freq_color.g, freq_color.b, 255);
        SDL_RenderDrawRect(renderer, &freq_rect);
        draw_text(renderer, g_crawler_state.frequency_input, x + 5, y + 5, text_color);
        draw_text(renderer, freq_valid ? "✓" : "✗", x + 160, y + 5, freq_color);
        y += 30;
        
        // Selection Prime
        draw_text(renderer, "Selection:", x, y, text_color);
        y += 18;
        
        uint64_t sel_val;
        bool sel_valid = validate_prime_input(g_crawler_state.selection_input, &sel_val);
        SDL_Color sel_color = sel_valid ? success_color : error_color;
        
        SDL_Rect sel_rect = {x, y, 150, 22};
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderFillRect(renderer, &sel_rect);
        SDL_SetRenderDrawColor(renderer, sel_color.r, sel_color.g, sel_color.b, 255);
        SDL_RenderDrawRect(renderer, &sel_rect);
        draw_text(renderer, g_crawler_state.selection_input, x + 5, y + 5, text_color);
        draw_text(renderer, sel_valid ? "✓" : "✗", x + 160, y + 5, sel_color);
        y += 30;
        
        // Delay Min
        draw_text(renderer, "Delay Min (sec):", x, y, text_color);
        y += 18;
        
        uint64_t min_val;
        bool min_valid = validate_prime_input(g_crawler_state.delay_min_input, &min_val);
        SDL_Color min_color = min_valid ? success_color : error_color;
        
        SDL_Rect min_rect = {x, y, 150, 22};
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderFillRect(renderer, &min_rect);
        SDL_SetRenderDrawColor(renderer, min_color.r, min_color.g, min_color.b, 255);
        SDL_RenderDrawRect(renderer, &min_rect);
        draw_text(renderer, g_crawler_state.delay_min_input, x + 5, y + 5, text_color);
        draw_text(renderer, min_valid ? "✓" : "✗", x + 160, y + 5, min_color);
        y += 30;
        
        // Delay Max
        draw_text(renderer, "Delay Max (sec):", x, y, text_color);
        y += 18;
        
        uint64_t max_val;
        bool max_valid = validate_prime_input(g_crawler_state.delay_max_input, &max_val);
        SDL_Color max_color = max_valid ? success_color : error_color;
        
        SDL_Rect max_rect = {x, y, 150, 22};
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderFillRect(renderer, &max_rect);
        SDL_SetRenderDrawColor(renderer, max_color.r, max_color.g, max_color.b, 255);
        SDL_RenderDrawRect(renderer, &max_rect);
        draw_text(renderer, g_crawler_state.delay_max_input, x + 5, y + 5, text_color);
        draw_text(renderer, max_valid ? "✓" : "✗", x + 160, y + 5, max_color);
        y += 30;
        
        // Apply button
        bool all_valid = freq_valid && sel_valid && min_valid && max_valid;
        if (all_valid) {
            SDL_Rect apply_rect = {x, y, 150, 25};
            draw_button_rect(renderer, apply_rect, "Apply", (SDL_Color){60, 60, 80, 255},
                           text_color, mouse_x, mouse_y);
            y += 30;
        }
    }
    
    y += 20;
    
    // URL Patterns section
    draw_section_header(renderer, "URL PATTERNS", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    const char* checkbox_on = "[✓]";
    const char* checkbox_off = "[ ]";
    
    // Pattern checkboxes
    draw_text(renderer, g_crawler_state.pattern_href ? checkbox_on : checkbox_off, x, y, text_color);
    draw_text(renderer, "Standard href", x + 35, y, text_color);
    y += 22;
    
    draw_text(renderer, g_crawler_state.pattern_onclick ? checkbox_on : checkbox_off, x, y, text_color);
    draw_text(renderer, "JavaScript onclick", x + 35, y, text_color);
    y += 22;
    
    draw_text(renderer, g_crawler_state.pattern_data_attr ? checkbox_on : checkbox_off, x, y, text_color);
    draw_text(renderer, "Data attributes", x + 35, y, text_color);
    y += 22;
    
    draw_text(renderer, g_crawler_state.pattern_meta_refresh ? checkbox_on : checkbox_off, x, y, text_color);
    draw_text(renderer, "Meta refresh", x + 35, y, text_color);
}

// ============================================================================
// COLUMN 2: LINK MANAGEMENT & ACTIVITY LOG
// ============================================================================

static void draw_column2_link_management(SDL_Renderer* renderer, const ColumnLayout* col,
                                        SDL_Color text_color, SDL_Color success_color,
                                        int mouse_x, int mouse_y) {
    int x = col->x + col->padding;
    int y = col->y + col->padding;
    
    // Section header
    draw_section_header(renderer, "LINK MANAGEMENT", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    // Queue size
    int queue_size = g_crawler_state.link_queue ? 
                     link_queue_size(g_crawler_state.link_queue) : 0;
    char queue_text[64];
    snprintf(queue_text, sizeof(queue_text), "Queue Size: %d", queue_size);
    draw_text(renderer, queue_text, x, y, text_color);
    y += 25;
    
    // Add URL input
    draw_text(renderer, "Add URL:", x, y, text_color);
    y += 18;
    
    SDL_Rect input_rect = {x, y, col->width - (col->padding * 2), 22};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &input_rect);
    SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
    SDL_RenderDrawRect(renderer, &input_rect);
    
    // Truncate if too long
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
    SDL_Rect add_btn = {x, y, 80, 25};
    draw_button_rect(renderer, add_btn, "Add", (SDL_Color){60, 60, 80, 255},
                    text_color, mouse_x, mouse_y);
    
    SDL_Rect clear_btn = {x + 90, y, 80, 25};
    draw_button_rect(renderer, clear_btn, "Clear", (SDL_Color){60, 60, 80, 255},
                    text_color, mouse_x, mouse_y);
    y += 35;
    
    // Confirmation message
    if (g_crawler_state.show_add_confirmation) {
        draw_text(renderer, "✓ Link added to queue", x, y, success_color);
        g_crawler_state.confirmation_timer--;
        if (g_crawler_state.confirmation_timer <= 0) {
            g_crawler_state.show_add_confirmation = false;
        }
        y += 25;
    }
    
    y += 20;
    
    // Activity Log section
    draw_section_header(renderer, "RECENT ACTIVITY", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    if (g_crawler_state.activity_count == 0) {
        draw_text(renderer, "No activity yet", x, y, (SDL_Color){150, 150, 150, 255});
    } else {
        for (int i = 0; i < g_crawler_state.activity_count && i < 8; i++) {
            draw_text(renderer, g_crawler_state.activity_log[i], x, y, (SDL_Color){200, 200, 200, 255});
            y += 18;
        }
    }
}

// ============================================================================
// COLUMN 3: STATUS & CONTROLS
// ============================================================================

static void draw_column3_status_controls(SDL_Renderer* renderer, const ColumnLayout* col,
                                        SDL_Color text_color, SDL_Color success_color,
                                        SDL_Color error_color, int mouse_x, int mouse_y) {
    int x = col->x + col->padding;
    int y = col->y + col->padding;
    
    // Section header
    draw_section_header(renderer, "STATUS", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    // Get crawler status
    bool is_running = is_crawler_running();
    int pages_crawled = 0;
    int queue_size = 0;
    get_crawler_status(&pages_crawled, &queue_size);
    
    // Status indicator
    const char* status_text = is_running ? "Running" : "Stopped";
    SDL_Color status_color = is_running ? success_color : (SDL_Color){150, 150, 150, 255};
    
    draw_text(renderer, "Crawler:", x, y, text_color);
    draw_text(renderer, status_text, x + 70, y, status_color);
    y += 25;
    
    // Pages crawled
    char pages_text[64];
    snprintf(pages_text, sizeof(pages_text), "Pages: %d", pages_crawled);
    draw_text(renderer, pages_text, x, y, text_color);
    y += 22;
    
    // Links in queue
    int link_count = g_crawler_state.link_queue ? 
                     link_queue_size(g_crawler_state.link_queue) : 0;
    char links_text[64];
    snprintf(links_text, sizeof(links_text), "Queue: %d", link_count);
    draw_text(renderer, links_text, x, y, text_color);
    y += 30;
    
    // Prime randomization status
    if (g_crawler_state.prime_enabled) {
        draw_text(renderer, "Prime Mode: ON", x, y, success_color);
        y += 20;
        
        char freq_text[64];
        snprintf(freq_text, sizeof(freq_text), "Freq: %lu",
                (unsigned long)g_crawler_state.prime_config.frequency_prime);
        draw_text(renderer, freq_text, x + 10, y, (SDL_Color){180, 180, 180, 255});
        y += 18;
        
        char delay_text[64];
        snprintf(delay_text, sizeof(delay_text), "Delay: %lu-%lu sec",
                (unsigned long)g_crawler_state.prime_config.delay_min_prime,
                (unsigned long)g_crawler_state.prime_config.delay_max_prime);
        draw_text(renderer, delay_text, x + 10, y, (SDL_Color){180, 180, 180, 255});
        y += 30;
    }
    
    y += 20;
    
    // Control buttons section
    draw_section_header(renderer, "CONTROLS", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    // Start/Stop button
    if (!is_running) {
        SDL_Rect start_btn = {x, y, col->width - (col->padding * 2), 35};
        draw_button_rect(renderer, start_btn, "START CRAWLER", success_color,
                        text_color, mouse_x, mouse_y);
    } else {
        SDL_Rect stop_btn = {x, y, col->width - (col->padding * 2), 35};
        draw_button_rect(renderer, stop_btn, "STOP CRAWLER", error_color,
                        text_color, mouse_x, mouse_y);
    }
    y += 45;
    
    // Save Config button
    SDL_Rect save_btn = {x, y, col->width - (col->padding * 2), 30};
    draw_button_rect(renderer, save_btn, "Save Config", (SDL_Color){60, 60, 80, 255},
                    text_color, mouse_x, mouse_y);
    y += 40;
    
    // Load Config button
    SDL_Rect load_btn = {x, y, col->width - (col->padding * 2), 30};
    draw_button_rect(renderer, load_btn, "Load Config", (SDL_Color){60, 60, 80, 255},
                    text_color, mouse_x, mouse_y);
}

// ============================================================================
// MAIN DRAWING FUNCTION
// ============================================================================

void draw_crawler_tab_with_layout(AppState* state, const TabLayout* layout) {
    if (!state || !state->renderer || !layout) return;
    
    init_crawler_tab_state();
    
    SDL_Renderer* renderer = state->renderer;
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {40, 40, 50, 255};
    SDL_Color success_color = {100, 200, 100, 255};
    SDL_Color error_color = {200, 100, 100, 255};
    
    // Get mouse position
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    
    // Draw background for each column
    for (int i = 0; i < layout->num_columns; i++) {
        draw_panel_background(renderer, &layout->columns[i], bg_color);
    }
    
    // Draw main title
    draw_text(renderer, "WEB CRAWLER CONTROL CENTER", layout->content_area.x + 20,
              layout->content_area.y - 35, (SDL_Color){200, 200, 220, 255});
    
    // Draw each column
    if (layout->num_columns >= 1) {
        draw_column1_prime_config(renderer, &layout->columns[0], text_color,
                                  success_color, error_color, mouse_x, mouse_y);
    }
    
    if (layout->num_columns >= 2) {
        draw_column2_link_management(renderer, &layout->columns[1], text_color,
                                     success_color, mouse_x, mouse_y);
    }
    
    if (layout->num_columns >= 3) {
        draw_column3_status_controls(renderer, &layout->columns[2], text_color,
                                     success_color, error_color, mouse_x, mouse_y);
    }
}

// ============================================================================
// CLICK HANDLER
// ============================================================================

void handle_crawler_tab_click(AppState* state, int mouse_x, int mouse_y) {
    // TODO: Implement click handling for buttons and inputs
    (void)state;
    (void)mouse_x;
    (void)mouse_y;
}

// ============================================================================
// KEYBOARD HANDLER
// ============================================================================

void handle_crawler_tab_keyboard(AppState* state, int key) {
    (void)state;
    
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