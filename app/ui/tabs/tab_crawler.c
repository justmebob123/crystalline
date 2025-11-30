// app/ui/tabs/tab_crawler.c - Redesigned Crawler Tab with InputManager
#include "../../app_common.h"
#include "../../input_manager.h"
#include "../../crawler_thread.h"
#include "../layout_manager.h"
#include "../../../src/crawler/prime_randomization.h"
#include "../../../src/crawler/link_management.h"
#include "../../../src/crawler/url_patterns.h"
#include "../../../src/crawler/crawler_url_manager.h"
#include "../../../src/crawler/content_filter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

// UI Button structure
typedef struct {
    SDL_Rect bounds;
    char label[64];
    bool enabled;
    bool visible;
} UIButton;

// Crawler tab state
typedef struct {
    // Prime Configuration
    CrawlerPrimeConfig prime_config;
    bool prime_enabled;
    
    // URL Management (NEW - using SQLite database)
    CrawlerURLManager* url_manager;
    int link_list_scroll;
    bool show_add_confirmation;
    int confirmation_timer;
    
    // URL Pattern Selection
    bool pattern_href;
    bool pattern_onclick;
    bool pattern_data_attr;
    bool pattern_meta_refresh;
    
    // Content Filtering (NEW - Phase 4 Feature 1)
    ExtractionMode extraction_mode;
    SDL_Rect radio_extract_all;
    SDL_Rect radio_extract_human;
    SDL_Rect radio_extract_metadata;
    SDL_Rect radio_extract_mixed;
    
    // Advanced Options (NEW)
    bool show_advanced_options;
    char get_parameters[256];
    char custom_headers[512];
    int timeout_seconds;
    int max_redirects;
    SDL_Rect advanced_toggle_rect;

    
    // Activity Log
    char activity_log[10][256];
    int activity_count;
    
    // UI State
    bool inputs_initialized;
    bool inputs_registered;
} CrawlerTabState;

static CrawlerTabState g_crawler_state = {0};

// UI Buttons (stored globally for click detection)
static UIButton btn_add_url;
static UIButton btn_clear_url;
static UIButton btn_start_crawler;
static UIButton btn_save_config;
static UIButton btn_load_config;

// ============================================================================
// INITIALIZATION
// ============================================================================

static void init_crawler_tab_state(void) {
    if (g_crawler_state.inputs_initialized) return;
    
    // Initialize prime config with defaults
    prime_config_init_default(&g_crawler_state.prime_config);
    g_crawler_state.prime_enabled = true;
    
    // Initialize URL manager with SQLite database
    g_crawler_state.url_manager = crawler_url_manager_create("data/crawler");
    if (!g_crawler_state.url_manager) {
        fprintf(stderr, "ERROR: Failed to create URL manager\n");
    }
    
    // Enable all URL patterns by default
    g_crawler_state.pattern_href = true;
    g_crawler_state.pattern_onclick = true;
    g_crawler_state.pattern_data_attr = true;
    g_crawler_state.pattern_meta_refresh = true;
    
    // Set default extraction mode
    g_crawler_state.extraction_mode = EXTRACT_ALL;
    
    // Initialize advanced options
    g_crawler_state.show_advanced_options = false;
    g_crawler_state.get_parameters[0] = '\0';
    g_crawler_state.custom_headers[0] = '\0';
    g_crawler_state.timeout_seconds = 30;
    g_crawler_state.max_redirects = 5;

    
    g_crawler_state.inputs_registered = false;
    g_crawler_state.inputs_initialized = true;
}

// Helper function to check if point is in rect (same as training tab)
static bool rect_contains_point(SDL_Rect rect, int x, int y) {
    return (x >= rect.x && x < rect.x + rect.w &&
            y >= rect.y && y < rect.y + rect.h);
}

// Helper function to check if crawler is running
static bool check_crawler_running(void) {
    extern int is_crawler_running(void);
    return is_crawler_running() != 0;
}

// Helper function to add activity log entry
static void add_activity_log(const char* message) {
    if (!message) return;
    
    // Shift existing messages down
    if (g_crawler_state.activity_count >= 10) {
        for (int i = 0; i < 9; i++) {
            strncpy(g_crawler_state.activity_log[i], g_crawler_state.activity_log[i + 1], 255);
            g_crawler_state.activity_log[i][255] = '\0';
        }
        g_crawler_state.activity_count = 9;
    }
    
    // Add new message at the end
    strncpy(g_crawler_state.activity_log[g_crawler_state.activity_count], message, 255);
    g_crawler_state.activity_log[g_crawler_state.activity_count][255] = '\0';
    g_crawler_state.activity_count++;
}

// Note: Column layout calculation is now handled by draw_crawler_tab_with_layout()
// which receives the layout from the layout manager

// Register inputs with InputManager (called once during first draw)
static void register_crawler_inputs(const ColumnLayout* col1, const ColumnLayout* col2) {
    if (g_crawler_state.inputs_registered) return;
    
    extern InputManager* g_input_manager;
    if (!g_input_manager) return;
    
    // CRITICAL: Validate layout before registering inputs
    if (!col1 || !col2) {
        printf("ERROR: Cannot register crawler inputs - layout not initialized\\n");
        return;
    }
    if (col1->width <= 0 || col2->width <= 0) {
        printf("ERROR: Invalid column widths: col1=%d, col2=%d\\n", col1->width, col2->width);
        return;
    }
    
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
    
    printf("Crawler tab: Registered 5 inputs with InputManager\n");
    g_crawler_state.inputs_registered = true;
}

// ============================================================================
// PRIME VALIDATION
// ============================================================================

static bool validate_prime_input(const char* input_id, uint64_t* value) {
    extern InputManager* g_input_manager;
    if (!g_input_manager) return false;
    
    const char* text = input_manager_get_text(g_input_manager, input_id);
    if (!text || text[0] == '\0') return false;
    
    char* endptr;
    unsigned long val = strtoul(text, &endptr, 10);
    
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
    y += 30;
    
        // Content Filtering section (NEW - Phase 4 Feature 1)
    draw_section_header(renderer, "CONTENT FILTERING", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    const char* radio_on = "(*)";
    const char* radio_off = "( )";
    
    // Extraction mode radio buttons (store bounds for click detection)
    int radio_width = 200;
    int radio_height = 20;
    
    // Extract All
    g_crawler_state.radio_extract_all = (SDL_Rect){x, y, radio_width, radio_height};
    draw_text(renderer, g_crawler_state.extraction_mode == EXTRACT_ALL ? radio_on : radio_off, x, y, text_color);
    draw_text(renderer, "Extract All (default)", x + 35, y, text_color);
    y += 22;
    
    // Human Text Only
    g_crawler_state.radio_extract_human = (SDL_Rect){x, y, radio_width, radio_height};
    draw_text(renderer, g_crawler_state.extraction_mode == EXTRACT_HUMAN_TEXT ? radio_on : radio_off, x, y, text_color);
    draw_text(renderer, "Human Text Only", x + 35, y, success_color);
    y += 22;
    
    // Metadata Only
    g_crawler_state.radio_extract_metadata = (SDL_Rect){x, y, radio_width, radio_height};
    draw_text(renderer, g_crawler_state.extraction_mode == EXTRACT_METADATA ? radio_on : radio_off, x, y, text_color);
    draw_text(renderer, "Metadata Only", x + 35, y, text_color);
    y += 22;
    
    // Mixed
    g_crawler_state.radio_extract_mixed = (SDL_Rect){x, y, radio_width, radio_height};
    draw_text(renderer, g_crawler_state.extraction_mode == EXTRACT_MIXED ? radio_on : radio_off, x, y, text_color);
    draw_text(renderer, "Mixed (Content + Meta)", x + 35, y, text_color);
    y += 30;
    
    // Advanced Options section (NEW - Phase 4 Feature 3)
    draw_section_header(renderer, "ADVANCED OPTIONS", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    // Toggle button for showing/hiding advanced options
    const char* advanced_toggle_text = g_crawler_state.show_advanced_options ? "[-] Hide" : "[+] Show";
    g_crawler_state.advanced_toggle_rect = (SDL_Rect){x, y, 100, 20};
    draw_text(renderer, advanced_toggle_text, x, y, (SDL_Color){100, 200, 255, 255});
    y += 25;
    
    // Show advanced options if expanded
    if (g_crawler_state.show_advanced_options) {
        // GET Parameters
        draw_text(renderer, "GET Parameters:", x, y, text_color);
        y += 18;
        draw_text(renderer, "(e.g., key1=val1&key2=val2)", x, y, (SDL_Color){150, 150, 150, 255});
        y += 18;
        if (g_crawler_state.get_parameters[0]) {
            draw_text(renderer, g_crawler_state.get_parameters, x, y, success_color);
        } else {
            draw_text(renderer, "[None]", x, y, (SDL_Color){150, 150, 150, 255});
        }
        y += 25;
        
        // Custom Headers
        draw_text(renderer, "Custom Headers:", x, y, text_color);
        y += 18;
        draw_text(renderer, "(e.g., User-Agent: MyBot)", x, y, (SDL_Color){150, 150, 150, 255});
        y += 18;
        if (g_crawler_state.custom_headers[0]) {
            draw_text(renderer, g_crawler_state.custom_headers, x, y, success_color);
        } else {
            draw_text(renderer, "[None]", x, y, (SDL_Color){150, 150, 150, 255});
        }
        y += 25;
        
        // Timeout
        char timeout_str[64];
        snprintf(timeout_str, sizeof(timeout_str), "Timeout: %d seconds", g_crawler_state.timeout_seconds);
        draw_text(renderer, timeout_str, x, y, text_color);
        y += 22;
        
        // Max Redirects
        char redirects_str[64];
        snprintf(redirects_str, sizeof(redirects_str), "Max Redirects: %d", g_crawler_state.max_redirects);
        draw_text(renderer, redirects_str, x, y, text_color);
        y += 22;
        
        draw_text(renderer, "Note: Advanced options coming soon", x, y, (SDL_Color){150, 150, 150, 255});
    }
}

// ============================================================================
// COLUMN 2: LINK MANAGEMENT & ACTIVITY LOG
// ============================================================================

static void draw_column2_link_management(SDL_Renderer* renderer, const ColumnLayout* col,
                                        SDL_Color text_color, SDL_Color success_color,
                                        int mouse_x, int mouse_y) {
    int x = col->x + col->padding;
    int y = col->y + col->padding;
    int content_width = col->width - (col->padding * 2);
    
    // Section header
    draw_section_header(renderer, "LINK MANAGEMENT", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    // Queue size (from SQLite database)
    int total = 0, pending = 0, crawled = 0, blocked = 0;
    if (g_crawler_state.url_manager) {
        crawler_url_manager_get_stats(g_crawler_state.url_manager, &total, &pending, &crawled, &blocked);
    }
    char queue_text[64];
    snprintf(queue_text, sizeof(queue_text), "Pending URLs: %d", pending);
    draw_text(renderer, queue_text, x, y, text_color);
    y += 25;
    
    // Show crawled count
    snprintf(queue_text, sizeof(queue_text), "Crawled: %d", crawled);
    draw_text(renderer, queue_text, x, y, text_color);
    y += 25;
    
    // Add URL input label
    draw_text(renderer, "Add URL:", x, y, text_color);
    y += 18;
    
    // Input rendered by InputManager
    y += 30;
    
    // Add and Clear buttons - USE COLUMN WIDTH FOR SIZING
    int button_width = (content_width - 10) / 2;  // Split column width between two buttons
    btn_add_url.bounds = (SDL_Rect){x, y, button_width, 25};
    btn_add_url.enabled = true;
    btn_add_url.visible = true;
    strncpy(btn_add_url.label, "Add", sizeof(btn_add_url.label) - 1);
    draw_button_rect(renderer, btn_add_url.bounds, "Add", (SDL_Color){60, 60, 80, 255},
                    text_color, mouse_x, mouse_y);
    
    btn_clear_url.bounds = (SDL_Rect){x + button_width + 10, y, button_width, 25};
    btn_clear_url.enabled = true;
    btn_clear_url.visible = true;
    strncpy(btn_clear_url.label, "Clear", sizeof(btn_clear_url.label) - 1);
    draw_button_rect(renderer, btn_clear_url.bounds, "Clear", (SDL_Color){60, 60, 80, 255},
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
                                SDL_Color __attribute__((unused)) error_color, 
                                int mouse_x, int mouse_y) {
    int x = col->x + col->padding;
    int y = col->y + col->padding;
    int content_width = col->width - (col->padding * 2);
    
    // Section header
    draw_section_header(renderer, "CRAWLER STATUS", x, y, (SDL_Color){180, 180, 200, 255});
    y += 30;
    
    // Status display (simplified - no crawler thread access for now)
    draw_text(renderer, "Status:", x, y, text_color);
    draw_text(renderer, "READY", x + 70, y, text_color);
    y += 25;
    
    draw_text(renderer, "Pages: 0", x, y, text_color);
    y += 20;
    
    draw_text(renderer, "Tokens: 0", x, y, text_color);
    y += 20;
    
    y += 30;
    
    // Start/Stop button - USE COLUMN WIDTH FOR SIZING
    btn_start_crawler.bounds = (SDL_Rect){x, y, content_width, 35};
    btn_start_crawler.enabled = true;
    btn_start_crawler.visible = true;
    
    // Change label and color based on crawler state
    bool crawler_running = check_crawler_running();
    const char* button_label = crawler_running ? "STOP CRAWLER" : "START CRAWLER";
    SDL_Color button_color = crawler_running ? (SDL_Color){255, 100, 100, 255} : success_color;
    
    strncpy(btn_start_crawler.label, button_label, sizeof(btn_start_crawler.label) - 1);
    draw_button_rect(renderer, btn_start_crawler.bounds, button_label, button_color,
                    text_color, mouse_x, mouse_y);
    y += 45;
    
    // Save Config button - USE COLUMN WIDTH FOR SIZING
    btn_save_config.bounds = (SDL_Rect){x, y, content_width, 30};
    btn_save_config.enabled = true;
    btn_save_config.visible = true;
    strncpy(btn_save_config.label, "Save Config", sizeof(btn_save_config.label) - 1);
    draw_button_rect(renderer, btn_save_config.bounds, "Save Config", (SDL_Color){60, 60, 80, 255},
                    text_color, mouse_x, mouse_y);
    y += 40;
    
    // Load Config button - USE COLUMN WIDTH FOR SIZING
    btn_load_config.bounds = (SDL_Rect){x, y, content_width, 30};
    btn_load_config.enabled = true;
    btn_load_config.visible = true;
    strncpy(btn_load_config.label, "Load Config", sizeof(btn_load_config.label) - 1);
    draw_button_rect(renderer, btn_load_config.bounds, "Load Config", (SDL_Color){60, 60, 80, 255},
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
    
    // Draw main title (positioned below submenu to avoid overlap)
    draw_text(renderer, "WEB CRAWLER CONTROL CENTER", layout->content_area.x + 20,
              layout->content_area.y + 30, (SDL_Color){200, 200, 220, 255});
    
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

// ============================================================================
// EVENT HANDLERS
// ============================================================================

void handle_crawler_tab_click(AppState* state, int mouse_x, int mouse_y) {
    if (!state) return;
    
    // Check Add URL button
    if (btn_add_url.visible && btn_add_url.enabled && 
        rect_contains_point(btn_add_url.bounds, mouse_x, mouse_y)) {
        
        // Get URL from input
        extern InputManager* g_input_manager;
        if (g_input_manager) {
            const char* url = input_manager_get_text(g_input_manager, "crawler.add_url");
            if (url && strlen(url) > 0) {
                // Validate URL (basic check)
                if (strstr(url, "http://") == url || strstr(url, "https://") == url) {
                    // Add to URL manager database
                    if (g_crawler_state.url_manager) {
                        int added = crawler_url_manager_add(g_crawler_state.url_manager, url, "manual");
                        
                        if (added == 0) {
                            // Add to activity log
                            char log_msg[512];
                            snprintf(log_msg, sizeof(log_msg), "Added URL: %s", url);
                            add_activity_log(log_msg);
                            
                            // Clear input
                            input_manager_set_text(g_input_manager, "crawler.add_url", "");
                            
                            // Show confirmation
                            g_crawler_state.show_add_confirmation = true;
                            g_crawler_state.confirmation_timer = 60;
                        } else {
                            add_activity_log("Error: Failed to add URL to database");
                        }
                    } else {
                        add_activity_log("Error: URL manager not initialized");
                    }
                } else {
                    add_activity_log("Error: URL must start with http:// or https://");
                }
            } else {
                add_activity_log("Error: Please enter a URL");
            }
        }
        return;
    }
    
    // Check Clear URL button
    if (btn_clear_url.visible && btn_clear_url.enabled &&
        rect_contains_point(btn_clear_url.bounds, mouse_x, mouse_y)) {
        
        extern InputManager* g_input_manager;
        if (g_input_manager) {
            input_manager_set_text(g_input_manager, "crawler.add_url", "");
            add_activity_log("Cleared URL input");
        }
        return;
    }
    
    // Check Start/Stop Crawler button
    if (btn_start_crawler.visible && btn_start_crawler.enabled &&
        rect_contains_point(btn_start_crawler.bounds, mouse_x, mouse_y)) {
        
        // Check if crawler is currently running
        if (check_crawler_running()) {
            // Stop the crawler
            extern void stop_crawler_thread(void);
            stop_crawler_thread();
            add_activity_log("Crawler stopped");
        } else {
            // Start the crawler
            
            // Check if we have URLs in the database
            int total = 0, pending = 0, crawled = 0, blocked = 0;
            if (g_crawler_state.url_manager) {
                crawler_url_manager_get_stats(g_crawler_state.url_manager, &total, &pending, &crawled, &blocked);
            }
            
            if (!g_crawler_state.url_manager || pending == 0) {
                add_activity_log("Error: No URLs in queue. Add a URL first.");
                return;
            }
            
            // Get the next URL from the database
            URLEntry* url_entry = crawler_url_manager_get_next(g_crawler_state.url_manager);
            if (!url_entry || url_entry->url[0] == '\0') {
                add_activity_log("Error: Failed to get URL from database");
                if (url_entry) free(url_entry);
                return;
            }
            
            // Start the crawler thread
            extern int start_crawler_thread(AppState* state, const char* start_url, ExtractionMode extraction_mode);
            if (start_crawler_thread(state, url_entry->url, g_crawler_state.extraction_mode) == 0) {
                char log_msg[512];
                int written = snprintf(log_msg, sizeof(log_msg), "Crawler started with URL: %s", url_entry->url);
                if (written >= (int)sizeof(log_msg)) {
                    strcpy(log_msg + sizeof(log_msg) - 4, "...");
                }
                add_activity_log(log_msg);
            } else {
                add_activity_log("Error: Failed to start crawler");
            }
            
            // Free the URL entry (URLEntry is a struct with fixed-size arrays)
            if (url_entry) {
                free(url_entry);
            }
        }
        return;
    }
    
    // Check Save Config button
    if (btn_save_config.visible && btn_save_config.enabled &&
        rect_contains_point(btn_save_config.bounds, mouse_x, mouse_y)) {
        add_activity_log("Save Config clicked (not yet implemented)");
        return;
    }
    
        // Check Load Config button
    if (btn_load_config.visible && btn_load_config.enabled &&
        rect_contains_point(btn_load_config.bounds, mouse_x, mouse_y)) {
        add_activity_log("Load Config clicked (not yet implemented)");
        return;
    }
    
    // Check extraction mode radio buttons
    if (rect_contains_point(g_crawler_state.radio_extract_all, mouse_x, mouse_y)) {
        g_crawler_state.extraction_mode = EXTRACT_ALL;
        add_activity_log("Extraction mode: Extract All");
        return;
    }
    
    if (rect_contains_point(g_crawler_state.radio_extract_human, mouse_x, mouse_y)) {
        g_crawler_state.extraction_mode = EXTRACT_HUMAN_TEXT;
        add_activity_log("Extraction mode: Human Text Only");
        return;
    }
    
    if (rect_contains_point(g_crawler_state.radio_extract_metadata, mouse_x, mouse_y)) {
        g_crawler_state.extraction_mode = EXTRACT_METADATA;
        add_activity_log("Extraction mode: Metadata Only");
        return;
    }
    
    if (rect_contains_point(g_crawler_state.radio_extract_mixed, mouse_x, mouse_y)) {
        g_crawler_state.extraction_mode = EXTRACT_MIXED;
        add_activity_log("Extraction mode: Mixed (Content + Metadata)");
        return;
    }
    
    // Check Advanced Options toggle
    if (rect_contains_point(g_crawler_state.advanced_toggle_rect, mouse_x, mouse_y)) {
        g_crawler_state.show_advanced_options = !g_crawler_state.show_advanced_options;
        if (g_crawler_state.show_advanced_options) {
            add_activity_log("Advanced Options: Expanded");
        } else {
            add_activity_log("Advanced Options: Collapsed");
        }
        return;
    }
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
    if (g_crawler_state.url_manager) {
        crawler_url_manager_destroy(g_crawler_state.url_manager);
        g_crawler_state.url_manager = NULL;
    }
}

// Get URL manager from crawler tab
void* get_crawler_url_manager(void) {
    extern CrawlerTabState g_crawler_state;
    return g_crawler_state.url_manager;
}
