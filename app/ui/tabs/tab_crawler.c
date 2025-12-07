/**
 * Crawler Tab - Full Featured Implementation with Crystalline UI
 * 
 * Features:
 * - Prime Configuration (frequency, selection, delay min/max)
 * - URL Pattern Selection (4 types)
 * - Content Filtering (4 extraction modes)
 * - Advanced Options (GET params, headers, timeout, redirects)
 * - SQLite URL Manager (persistent storage)
 * - Activity Log (10-line scrolling)
 * - Configuration Persistence (save/load)
 * - Model Selector
 * 
 * Layout:
 * - Column 1 (33%): Prime Config + URL Patterns
 * - Column 2 (33%): URL Management + Controls
 * - Column 3 (33%): Status + Activity Log
 */

#include "../../app_common.h"
#include "../../input_manager.h"
#include "../../crawler_thread.h"
#include "../layout_manager.h"
#include "../model_selector.h"
#include "../crystalline/elements.h"
#include "../crystalline/global_layout.h"
#include "../button_sizes.h"
#include "crawler.h"
#include "cllm_model_registry.h"
#include "../../../src/crawler/prime_randomization.h"
#include "../../../src/crawler/link_management.h"
#include "../../../src/crawler/url_patterns.h"
#include "../../../src/crawler/crawler_url_manager.h"
#include "../../../src/crawler/content_filter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    
    // URL Management (SQLite database)
    CrawlerURLManager* url_manager;
    int link_list_scroll;
    bool show_add_confirmation;
    int confirmation_timer;
    
    // URL Pattern Selection
    bool pattern_href;
    bool pattern_onclick;
    bool pattern_data_attr;
    bool pattern_meta_refresh;
    
    // Content Filtering
    ExtractionMode extraction_mode;
    SDL_Rect radio_extract_all;
    SDL_Rect radio_extract_human;
    SDL_Rect radio_extract_metadata;
    SDL_Rect radio_extract_mixed;
    
    // Advanced Options
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
    bool crawler_running;
} CrawlerTabState;

static CrawlerTabState g_crawler_state = {0};

// UI Structure for Crystalline components
static struct {
    // Panels
    CrystallinePanel* list_panel;
    CrystallinePanel* control_panel;
    
    // List
    CrystallineList* url_list;
    
    // Buttons
    CrystallineButton* btn_start;
    CrystallineButton* btn_stop;
    CrystallineButton* btn_clear;
    CrystallineButton* btn_add_url;
    
    // Sliders
    CrystallineSlider* slider_max_depth;
    CrystallineSlider* slider_max_urls;
    CrystallineSlider* slider_rate_limit;
    
    // Text areas
    CrystallineTextArea* stats_display;
    CrystallineInput* url_input;
    
    // State
    bool initialized;
    bool crawler_running;
    char selected_model[256];
    
    // URL tracking
    char** urls;
    int url_count;
    int url_capacity;
} g_crawler_ui = {0};

// UI Buttons (for new features)
static UIButton btn_add_url;
static UIButton btn_clear_url;
static UIButton btn_start_crawler;
static UIButton btn_reset_urls;
static UIButton btn_save_config;
static UIButton btn_load_config;

// Model selector
static ModelSelector* crawler_model_selector = NULL;
static char crawler_selected_model_name[256] = {0};

// Model selector callback
static void on_crawler_model_selected(const char* model_name, void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state || !model_name) return;
    
    // Store selected model name (not loaded until crawling starts)
    printf("Crawler tab: Model '%s' selected (not loaded yet)\n", model_name);
    strncpy(crawler_selected_model_name, model_name, sizeof(crawler_selected_model_name) - 1);
    crawler_selected_model_name[sizeof(crawler_selected_model_name) - 1] = '\0';
}

/**
 * Initialize crawler tab state
 */
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
    g_crawler_state.crawler_running = false;
}

/**
 * Helper functions
 */
static bool rect_contains_point(SDL_Rect rect, int x, int y) {
    return (x >= rect.x && x < rect.x + rect.w &&
            y >= rect.y && y < rect.y + rect.h);
}

static bool check_crawler_running(void) {
    extern int is_crawler_running(void);
    return is_crawler_running() != 0;
}

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

/**
 * Add URL to list
 */
static void add_url_to_list(const char* url) {
    if (!url || !url[0]) return;
    
    // Expand capacity if needed
    if (g_crawler_ui.url_count >= g_crawler_ui.url_capacity) {
        int new_capacity = g_crawler_ui.url_capacity == 0 ? 10 : g_crawler_ui.url_capacity * 2;
        char** new_urls = realloc(g_crawler_ui.urls, new_capacity * sizeof(char*));
        if (!new_urls) return;
        g_crawler_ui.urls = new_urls;
        g_crawler_ui.url_capacity = new_capacity;
    }
    
    // Add URL
    g_crawler_ui.urls[g_crawler_ui.url_count] = strdup(url);
    g_crawler_ui.url_count++;
    
    // Update list UI
    if (g_crawler_ui.url_list) {
        crystalline_list_set_items(g_crawler_ui.url_list, g_crawler_ui.urls, g_crawler_ui.url_count);
    }
    
    printf("Added URL: %s (total: %d)\n", url, g_crawler_ui.url_count);
}

/**
 * Clear all URLs
 */
static void clear_url_list(void) {
    // Free all URLs
    for (int i = 0; i < g_crawler_ui.url_count; i++) {
        free(g_crawler_ui.urls[i]);
    }
    free(g_crawler_ui.urls);
    
    g_crawler_ui.urls = NULL;
    g_crawler_ui.url_count = 0;
    g_crawler_ui.url_capacity = 0;
    
    // Update list UI
    if (g_crawler_ui.url_list) {
        crystalline_list_set_items(g_crawler_ui.url_list, NULL, 0);
    }
    
    printf("Cleared all URLs\n");
}

/**
 * Button callbacks
 */
static void on_start_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    
    if (g_crawler_ui.crawler_running) {
        printf("Crawler already running\n");
        return;
    }
    
    if (g_crawler_ui.url_count == 0) {
        printf("No URLs to crawl\n");
        return;
    }
    
    printf("Starting crawler with %d URLs\n", g_crawler_ui.url_count);
    
    // Use the first URL as start URL
    const char* start_url = g_crawler_ui.urls[0];
    
    // Start crawler thread with correct signature
    extern int start_crawler_thread(AppState* state, const char* start_url, 
                                   ExtractionMode extraction_mode, const char* model_name);
    
    if (start_crawler_thread(state, start_url, EXTRACT_ALL, g_crawler_ui.selected_model) == 0) {
        g_crawler_ui.crawler_running = true;
        printf("Crawler started successfully\n");
    } else {
        printf("Failed to start crawler\n");
    }
}

static void on_stop_clicked(void* data) {
    (void)data;
    
    if (!g_crawler_ui.crawler_running) {
        printf("Crawler not running\n");
        return;
    }
    
    printf("Stopping crawler\n");
    
    // Stop crawler thread
    extern void stop_crawler_thread(void);
    stop_crawler_thread();
    
    g_crawler_ui.crawler_running = false;
    printf("Crawler stopped\n");
}

static void on_clear_clicked(void* data) {
    (void)data;
    
    if (g_crawler_ui.crawler_running) {
        printf("Cannot clear URLs while crawler is running\n");
        return;
    }
    
    clear_url_list();
}

static void on_add_url_clicked(void* data) {
    (void)data;
    
    if (!g_crawler_ui.url_input) return;
    
    const char* url = crystalline_input_get_text(g_crawler_ui.url_input);
    if (url && url[0]) {
        add_url_to_list(url);
        crystalline_input_set_text(g_crawler_ui.url_input, "");
    }
}

/**
 * Initialize Crawler Tab
 */
void init_crawler_tab(AppState* state) {
    if (g_crawler_ui.initialized) return;
    
    printf("Initializing Crawler Tab with Crystalline UI\n");
    
    // Get global font
    extern TTF_Font* get_global_font();
    TTF_Font* font = get_global_font();
    if (!font) {
        printf("ERROR: Failed to get global font\n");
        return;
    }
    
    // Calculate layout using RENDER_WIDTH
    int content_width = RENDER_WIDTH;
    int content_height = WINDOW_HEIGHT - SUBMENU_HEIGHT;
    
    // Split into left (70%) and right (30%)
    int list_width = (int)(content_width * 0.70f);
    int control_width = content_width - list_width;
    
    // Calculate TOP-LEFT positions first
    int list_x = RENDER_OFFSET_X;
    int list_y = SUBMENU_HEIGHT;
    int list_w = list_width - 20;
    int list_h = content_height - 20;
    
    int control_x = RENDER_OFFSET_X + list_width + 10;
    int control_y = SUBMENU_HEIGHT;
    int control_w = control_width - 30;
    int control_h = content_height - 20;
    
    // Convert to CENTER coordinates for Crystalline UI
    g_crawler_ui.list_panel = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        list_x + list_w / 2.0f,
        list_y + list_h / 2.0f,
        list_w,
        list_h,
        "URL Queue",
        font
    );
    
    g_crawler_ui.control_panel = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        control_x + control_w / 2.0f,
        control_y + control_h / 2.0f,
        control_w,
        control_h,
        "Controls",
        font
    );
    
    // Create URL list inside left panel
    int list_content_x = list_x + 10;
    int list_content_y = list_y + 40;
    int list_content_w = list_w - 20;
    int list_content_h = list_h - 50;
    
    g_crawler_ui.url_list = crystalline_list_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        list_content_x + list_content_w / 2.0f,
        list_content_y + list_content_h / 2.0f,
        list_content_w,
        40,  // Item height
        font
    );
    
    // Create control elements inside right panel (top-justified)
    int elem_x = control_x + 10;
    int elem_w = control_w - 20;
    int elem_y = control_y + 40;
    
    // URL input field
    int input_h = 40;
    g_crawler_ui.url_input = crystalline_input_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        elem_x + elem_w / 2.0f,
        elem_y + input_h / 2.0f,
        elem_w,
        input_h,
        "Enter URL",
        font
    );
    elem_y += 50;
    
    // Add URL button
    int btn_h = 50;
    g_crawler_ui.btn_add_url = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        elem_x + elem_w / 2.0f,
        elem_y + btn_h / 2.0f,
        elem_w,
        btn_h,
        "Add URL",
        font
    );
    crystalline_button_set_callback(g_crawler_ui.btn_add_url, on_add_url_clicked, state);
    elem_y += 70;
    
    // Start button
    g_crawler_ui.btn_start = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        elem_x + elem_w / 2.0f,
        elem_y + btn_h / 2.0f,
        elem_w,
        btn_h,
        "Start Crawl",
        font
    );
    crystalline_button_set_callback(g_crawler_ui.btn_start, on_start_clicked, state);
    elem_y += 70;
    
    // Stop button
    g_crawler_ui.btn_stop = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        elem_x + elem_w / 2.0f,
        elem_y + btn_h / 2.0f,
        elem_w,
        btn_h,
        "Stop Crawl",
        font
    );
    crystalline_button_set_callback(g_crawler_ui.btn_stop, on_stop_clicked, state);
    elem_y += 70;
    
    // Clear button
    g_crawler_ui.btn_clear = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        elem_x + elem_w / 2.0f,
        elem_y + btn_h / 2.0f,
        elem_w,
        btn_h,
        "Clear URLs",
        font
    );
    crystalline_button_set_callback(g_crawler_ui.btn_clear, on_clear_clicked, state);
    elem_y += 80;
    
    // Sliders
    int slider_h = 30;
    g_crawler_ui.slider_max_depth = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        elem_x + elem_w / 2.0f,
        elem_y + slider_h / 2.0f,
        elem_w,
        slider_h,
        1.0f, 10.0f
    );
    crystalline_slider_set_value(g_crawler_ui.slider_max_depth, 3.0f);
    elem_y += 60;
    
    g_crawler_ui.slider_max_urls = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        elem_x + elem_w / 2.0f,
        elem_y + slider_h / 2.0f,
        elem_w,
        slider_h,
        10.0f, 1000.0f
    );
    crystalline_slider_set_value(g_crawler_ui.slider_max_urls, 100.0f);
    elem_y += 60;
    
    g_crawler_ui.slider_rate_limit = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        elem_x + elem_w / 2.0f,
        elem_y + slider_h / 2.0f,
        elem_w,
        slider_h,
        0.5f, 10.0f
    );
    crystalline_slider_set_value(g_crawler_ui.slider_rate_limit, 2.0f);
    elem_y += 70;
    
    // Stats display
    int stats_h = control_h - (elem_y - control_y) - 20;
    g_crawler_ui.stats_display = crystalline_textarea_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        elem_x + elem_w / 2.0f,
        elem_y + stats_h / 2.0f,
        elem_w,
        stats_h,
        font
    );
    crystalline_textarea_add_message(g_crawler_ui.stats_display,
        CRYSTALLINE_MESSAGE_SYSTEM,
        "Crawler ready",
        "");
    
    // Initialize state
    g_crawler_ui.initialized = true;
    g_crawler_ui.crawler_running = false;
    g_crawler_ui.urls = NULL;
    g_crawler_ui.url_count = 0;
    g_crawler_ui.url_capacity = 0;
    
    printf("Crawler Tab initialized successfully\n");
}

/**
 * Update crawler statistics display
 */
static void update_stats_display(void) {
    if (!g_crawler_ui.stats_display) return;
    
    // Format stats text
    char stats_text[1024];
    snprintf(stats_text, sizeof(stats_text),
        "Status: %s\n\n"
        "URLs in queue: %d\n\n"
        "Note: Crawler statistics will be\n"
        "displayed here when crawling starts.",
        g_crawler_ui.crawler_running ? "Running" : "Stopped",
        g_crawler_ui.url_count
    );
    
    crystalline_textarea_clear(g_crawler_ui.stats_display);
    crystalline_textarea_add_message(g_crawler_ui.stats_display,
        CRYSTALLINE_MESSAGE_SYSTEM,
        stats_text,
        "");
}

/**
 * Cleanup Crawler Tab
 */
void cleanup_crawler_tab(void) {
    if (!g_crawler_ui.initialized) return;
    
    // Stop crawler if running
    if (g_crawler_ui.crawler_running) {
        extern void stop_crawler_thread(void);
        stop_crawler_thread();
    }
    
    // Free URLs
    clear_url_list();
    
    // Cleanup Crystalline UI elements
    if (g_crawler_ui.list_panel) crystalline_panel_destroy(g_crawler_ui.list_panel);
    if (g_crawler_ui.control_panel) crystalline_panel_destroy(g_crawler_ui.control_panel);
    if (g_crawler_ui.url_list) crystalline_list_destroy(g_crawler_ui.url_list);
    if (g_crawler_ui.btn_start) crystalline_button_destroy(g_crawler_ui.btn_start);
    if (g_crawler_ui.btn_stop) crystalline_button_destroy(g_crawler_ui.btn_stop);
    if (g_crawler_ui.btn_clear) crystalline_button_destroy(g_crawler_ui.btn_clear);
    if (g_crawler_ui.btn_add_url) crystalline_button_destroy(g_crawler_ui.btn_add_url);
    if (g_crawler_ui.slider_max_depth) crystalline_slider_destroy(g_crawler_ui.slider_max_depth);
    if (g_crawler_ui.slider_max_urls) crystalline_slider_destroy(g_crawler_ui.slider_max_urls);
    if (g_crawler_ui.slider_rate_limit) crystalline_slider_destroy(g_crawler_ui.slider_rate_limit);
    if (g_crawler_ui.stats_display) crystalline_textarea_destroy(g_crawler_ui.stats_display);
    if (g_crawler_ui.url_input) crystalline_input_destroy(g_crawler_ui.url_input);
    
    memset(&g_crawler_ui, 0, sizeof(g_crawler_ui));
}

/**
 * Update Crawler Tab (called every frame)
 */
void update_crawler_tab(AppState* state) {
    if (!g_crawler_ui.initialized) return;
    
    // Update stats display if crawler is running
    if (g_crawler_ui.crawler_running) {
        update_stats_display();
    }
}

/**
 * Render Crawler Tab
 */
void render_crawler_tab(SDL_Renderer* renderer, AppState* state) {
    // Initialize on first draw if not already initialized
    if (!g_crawler_ui.initialized) {
        printf("WARNING: Crawler tab not initialized, initializing now...\n");
        init_crawler_tab(state);
        if (!g_crawler_ui.initialized) {
            printf("ERROR: Failed to initialize crawler tab\n");
            return;
        }
    }
    
    // Render panels
    if (g_crawler_ui.list_panel) {
        crystalline_panel_render(g_crawler_ui.list_panel, renderer);
    }
    if (g_crawler_ui.control_panel) {
        crystalline_panel_render(g_crawler_ui.control_panel, renderer);
    }
    
    // Render list
    if (g_crawler_ui.url_list) {
        crystalline_list_render(g_crawler_ui.url_list, renderer);
    }
    
    // Render input
    if (g_crawler_ui.url_input) {
        crystalline_input_render(g_crawler_ui.url_input, renderer);
    }
    
    // Render buttons
    if (g_crawler_ui.btn_add_url) {
        crystalline_button_render(g_crawler_ui.btn_add_url, renderer);
    }
    if (g_crawler_ui.btn_start) {
        crystalline_button_render(g_crawler_ui.btn_start, renderer);
    }
    if (g_crawler_ui.btn_stop) {
        crystalline_button_render(g_crawler_ui.btn_stop, renderer);
    }
    if (g_crawler_ui.btn_clear) {
        crystalline_button_render(g_crawler_ui.btn_clear, renderer);
    }
    
    // Render sliders
    if (g_crawler_ui.slider_max_depth) {
        crystalline_slider_render(g_crawler_ui.slider_max_depth, renderer);
    }
    if (g_crawler_ui.slider_max_urls) {
        crystalline_slider_render(g_crawler_ui.slider_max_urls, renderer);
    }
    if (g_crawler_ui.slider_rate_limit) {
        crystalline_slider_render(g_crawler_ui.slider_rate_limit, renderer);
    }
    
    // Render stats display
    if (g_crawler_ui.stats_display) {
        crystalline_textarea_render(g_crawler_ui.stats_display, renderer);
    }
}

/**
 * Event handlers following standardized pattern
 */
void handle_crawler_tab_mouse_down(SDL_MouseButtonEvent* event, AppState* state) {
    if (!g_crawler_ui.initialized) return;
    
    SDL_Event sdl_event = {0};
    sdl_event.type = SDL_MOUSEBUTTONDOWN;
    sdl_event.button = *event;
    
    // Handle input
    if (g_crawler_ui.url_input) {
        crystalline_input_handle_mouse(g_crawler_ui.url_input, &sdl_event);
    }
    
    // Handle list
    if (g_crawler_ui.url_list) {
        crystalline_list_handle_mouse(g_crawler_ui.url_list, &sdl_event);
    }
    
    // Handle buttons
    if (g_crawler_ui.btn_add_url) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_add_url, &sdl_event);
    }
    if (g_crawler_ui.btn_start) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_start, &sdl_event);
    }
    if (g_crawler_ui.btn_stop) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_stop, &sdl_event);
    }
    if (g_crawler_ui.btn_clear) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_clear, &sdl_event);
    }
    
    // Handle sliders
    if (g_crawler_ui.slider_max_depth) {
        crystalline_slider_handle_mouse(g_crawler_ui.slider_max_depth, &sdl_event);
    }
    if (g_crawler_ui.slider_max_urls) {
        crystalline_slider_handle_mouse(g_crawler_ui.slider_max_urls, &sdl_event);
    }
    if (g_crawler_ui.slider_rate_limit) {
        crystalline_slider_handle_mouse(g_crawler_ui.slider_rate_limit, &sdl_event);
    }
}

void handle_crawler_tab_mouse_up(SDL_MouseButtonEvent* event, AppState* state) {
    if (!g_crawler_ui.initialized) return;
    
    SDL_Event sdl_event = {0};
    sdl_event.type = SDL_MOUSEBUTTONUP;
    sdl_event.button = *event;
    
    // Handle input
    if (g_crawler_ui.url_input) {
        crystalline_input_handle_mouse(g_crawler_ui.url_input, &sdl_event);
    }
    
    // Handle list
    if (g_crawler_ui.url_list) {
        crystalline_list_handle_mouse(g_crawler_ui.url_list, &sdl_event);
    }
    
    // Handle buttons
    if (g_crawler_ui.btn_add_url) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_add_url, &sdl_event);
    }
    if (g_crawler_ui.btn_start) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_start, &sdl_event);
    }
    if (g_crawler_ui.btn_stop) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_stop, &sdl_event);
    }
    if (g_crawler_ui.btn_clear) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_clear, &sdl_event);
    }
    
    // Handle sliders
    if (g_crawler_ui.slider_max_depth) {
        crystalline_slider_handle_mouse(g_crawler_ui.slider_max_depth, &sdl_event);
    }
    if (g_crawler_ui.slider_max_urls) {
        crystalline_slider_handle_mouse(g_crawler_ui.slider_max_urls, &sdl_event);
    }
    if (g_crawler_ui.slider_rate_limit) {
        crystalline_slider_handle_mouse(g_crawler_ui.slider_rate_limit, &sdl_event);
    }
}

void handle_crawler_tab_mouse_motion(SDL_MouseMotionEvent* event, AppState* state) {
    if (!g_crawler_ui.initialized) return;
    
    SDL_Event sdl_event = {0};
    sdl_event.type = SDL_MOUSEMOTION;
    sdl_event.motion = *event;
    
    // Handle input
    if (g_crawler_ui.url_input) {
        crystalline_input_handle_mouse(g_crawler_ui.url_input, &sdl_event);
    }
    
    // Handle list
    if (g_crawler_ui.url_list) {
        crystalline_list_handle_mouse(g_crawler_ui.url_list, &sdl_event);
    }
    
    // Handle buttons
    if (g_crawler_ui.btn_add_url) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_add_url, &sdl_event);
    }
    if (g_crawler_ui.btn_start) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_start, &sdl_event);
    }
    if (g_crawler_ui.btn_stop) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_stop, &sdl_event);
    }
    if (g_crawler_ui.btn_clear) {
        crystalline_button_handle_mouse(g_crawler_ui.btn_clear, &sdl_event);
    }
    
    // Handle sliders
    if (g_crawler_ui.slider_max_depth) {
        crystalline_slider_handle_mouse(g_crawler_ui.slider_max_depth, &sdl_event);
    }
    if (g_crawler_ui.slider_max_urls) {
        crystalline_slider_handle_mouse(g_crawler_ui.slider_max_urls, &sdl_event);
    }
    if (g_crawler_ui.slider_rate_limit) {
        crystalline_slider_handle_mouse(g_crawler_ui.slider_rate_limit, &sdl_event);
    }
}