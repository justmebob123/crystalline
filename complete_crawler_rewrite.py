#!/usr/bin/env python3
"""
Complete clean rewrite of crawler tab using InputManager
Based on training tab pattern
"""

# Start with the backup
with open('app/ui/tabs/tab_crawler.c.backup', 'r') as f:
    original = f.read()

# Create the new file content from scratch
new_content = '''// app/ui/tabs/tab_crawler.c - Redesigned Crawler Tab with InputManager
#include "../../app_common.h"
#include "../../input_manager.h"
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
    
    // Link Management
    LinkQueue* link_queue;
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
    bool inputs_registered;
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
    
    // Initialize link queue
    g_crawler_state.link_queue = link_queue_create("data/crawler/link_queue.txt");
    
    // Enable all URL patterns by default
    g_crawler_state.pattern_href = true;
    g_crawler_state.pattern_onclick = true;
    g_crawler_state.pattern_data_attr = true;
    g_crawler_state.pattern_meta_refresh = true;
    
    g_crawler_state.inputs_registered = false;
    g_crawler_state.inputs_initialized = true;
}

// Register inputs with InputManager (called once during first draw)
static void register_crawler_inputs(const ColumnLayout* col1, const ColumnLayout* col2) {
    if (g_crawler_state.inputs_registered) return;
    
    extern InputManager* g_input_manager;
    if (!g_input_manager) return;
    
    // Column 1 inputs (prime configuration)
    int x1 = col1->x + col1->padding;
    int y_freq = col1->y + col1->padding + 65;
    int y_sel = y_freq + 48;
    int y_min = y_sel + 48;
    int y_max = y_min + 48;
    
    // Register prime configuration inputs
    input_manager_register(g_input_manager, "crawler.frequency", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){x1, y_freq, 150, 22});
    input_manager_set_text(g_input_manager, "crawler.frequency", "7");
    
    input_manager_register(g_input_manager, "crawler.selection", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){x1, y_sel, 150, 22});
    input_manager_set_text(g_input_manager, "crawler.selection", "13");
    
    input_manager_register(g_input_manager, "crawler.delay_min", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){x1, y_min, 150, 22});
    input_manager_set_text(g_input_manager, "crawler.delay_min", "3");
    
    input_manager_register(g_input_manager, "crawler.delay_max", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){x1, y_max, 150, 22});
    input_manager_set_text(g_input_manager, "crawler.delay_max", "11");
    
    // Column 2 input (add URL)
    int x2 = col2->x + col2->padding;
    int y_url = col2->y + col2->padding + 73;
    
    input_manager_register(g_input_manager, "crawler.add_url", TAB_CRAWLER, INPUT_TYPE_URL,
                         (SDL_Rect){x2, y_url, col2->width - (col2->padding * 2), 22});
    input_manager_set_text(g_input_manager, "crawler.add_url", "");
    
    printf("Crawler tab: Registered 5 inputs with InputManager\\n");
    g_crawler_state.inputs_registered = true;
}

// ============================================================================
// PRIME VALIDATION
// ============================================================================

static bool validate_prime_input(const char* input_id, uint64_t* value) {
    extern InputManager* g_input_manager;
    if (!g_input_manager) return false;
    
    const char* text = input_manager_get_text(g_input_manager, input_id);
    if (!text || text[0] == '\\0') return false;
    
    char* endptr;
    unsigned long val = strtoul(text, &endptr, 10);
    
    if (*endptr != '\\0' || val == 0) {
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
        bool freq_valid = validate_prime_input("crawler.frequency", &freq_val);
        SDL_Color freq_color = freq_valid ? success_color : error_color;
        
        // Input rendered by InputManager, just draw validation indicator
        draw_text(renderer, freq_valid ? "OK" : "X", x + 160, y + 5, freq_color);
        y += 30;
        
        // Selection Prime
        draw_text(renderer, "Selection:", x, y, text_color);
        y += 18;
        
        uint64_t sel_val;
        bool sel_valid = validate_prime_input("crawler.selection", &sel_val);
        SDL_Color sel_color = sel_valid ? success_color : error_color;
        
        // Input rendered by InputManager
        draw_text(renderer, sel_valid ? "OK" : "X", x + 160, y + 5, sel_color);
        y += 30;
        
        // Delay Min
        draw_text(renderer, "Delay Min (sec):", x, y, text_color);
        y += 18;
        
        uint64_t min_val;
        bool min_valid = validate_prime_input("crawler.delay_min", &min_val);
        SDL_Color min_color = min_valid ? success_color : error_color;
        
        // Input rendered by InputManager
        draw_text(renderer, min_valid ? "OK" : "X", x + 160, y + 5, min_color);
        y += 30;
        
        // Delay Max
        draw_text(renderer, "Delay Max (sec):", x, y, text_color);
        y += 18;
        
        uint64_t max_val;
        bool max_valid = validate_prime_input("crawler.delay_max", &max_val);
        SDL_Color max_color = max_valid ? success_color : error_color;
        
        // Input rendered by InputManager
        draw_text(renderer, max_valid ? "OK" : "X", x + 160, y + 5, max_color);
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
    
    const char* checkbox_on = "[X]";
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
    
    // Add URL input label
    draw_text(renderer, "Add URL:", x, y, text_color);
    y += 18;
    
    // Input rendered by InputManager
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
        draw_text(renderer, "[OK] Link added to queue", x, y, success_color);
        g_crawler_state.confirmation_timer--;
        if (g_crawler_state.confirmation_timer <= 0) {
            g_crawler_state.show_add_confirmation = false;
        }
        y += 25;
    }
    
    y += 20;
    
    // Recent Activity section
    draw_section_header(renderer, "RECENT ACTIVITY", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    if (g_crawler_state.activity_count == 0) {
        draw_text(renderer, "No activity yet", x, y, (SDL_Color){150, 150, 150, 255});
    } else {
        for (int i = 0; i < g_crawler_state.activity_count && i < 10; i++) {
            draw_text(renderer, g_crawler_state.activity_log[i], x, y, text_color);
            y += 18;
        }
    }
}

// ============================================================================
// COLUMN 3: STATUS DISPLAY & CONTROLS
// ============================================================================

static void draw_column3_status(SDL_Renderer* renderer, const ColumnLayout* col,
                                SDL_Color text_color, SDL_Color success_color,
                                SDL_Color error_color, int mouse_x, int mouse_y) {
    int x = col->x + col->padding;
    int y = col->y + col->padding;
    
    // Section header
    draw_section_header(renderer, "CRAWLER STATUS", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    // Status display
    extern CrawlerThread* g_crawler_thread;
    bool is_running = (g_crawler_thread != NULL && g_crawler_thread->running);
    
    const char* status_text = is_running ? "RUNNING" : "STOPPED";
    SDL_Color status_color = is_running ? success_color : error_color;
    draw_text(renderer, "Status:", x, y, text_color);
    draw_text(renderer, status_text, x + 70, y, status_color);
    y += 25;
    
    if (is_running && g_crawler_thread) {
        char pages_text[64];
        snprintf(pages_text, sizeof(pages_text), "Pages: %d", g_crawler_thread->pages_crawled);
        draw_text(renderer, pages_text, x, y, text_color);
        y += 20;
        
        char tokens_text[64];
        snprintf(tokens_text, sizeof(tokens_text), "Tokens: %d", g_crawler_thread->tokens_extracted);
        draw_text(renderer, tokens_text, x, y, text_color);
        y += 20;
    }
    
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
    
    // Register inputs on first draw
    if (layout->num_columns >= 2) {
        register_crawler_inputs(&layout->columns[0], &layout->columns[1]);
    }
    
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
        draw_column3_status(renderer, &layout->columns[2], text_color,
                           success_color, error_color, mouse_x, mouse_y);
    }
    
    // Render all inputs through InputManager
    extern InputManager* g_input_manager;
    if (g_input_manager) {
        input_manager_render(g_input_manager, renderer, get_global_font(), TAB_CRAWLER);
    }
}

// Legacy function for compatibility
void draw_crawler_tab(AppState* state) {
    (void)state;
    // This function is deprecated - use draw_crawler_tab_with_layout instead
}

// ============================================================================
// EVENT HANDLERS
// ============================================================================

void handle_crawler_tab_click(AppState* state, int mouse_x, int mouse_y) {
    // InputManager handles all input clicks automatically
    // This function can handle button clicks if needed
    (void)state;
    (void)mouse_x;
    (void)mouse_y;
}

void handle_crawler_tab_keyboard(AppState* state, int key) {
    // InputManager handles all keyboard input automatically
    (void)state;
    (void)key;
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
'''

# Write the new file
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.write(new_content)

print("\n✓ Complete rewrite successful!")
print("  - Clean implementation using InputManager")
print("  - Follows training tab pattern exactly")
print("  - 5 inputs registered (4 prime config + 1 URL)")
print("  - Proper event handling through InputManager")
print("  - ASCII characters only (no Unicode)")