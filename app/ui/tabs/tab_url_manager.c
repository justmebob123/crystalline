// app/ui/tabs/tab_url_manager.c - URL Manager Tab Implementation
#include "../../app_common.h"
#include "../../../src/crawler/crawler_url_manager.h"
#include <SDL2/SDL_ttf.h>

extern TTF_Font* get_global_font(void);
extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);

// Use constants from app_common.h
#define SIDEBAR_WIDTH 200
#define SUBMENU_HEIGHT 40
#define BUTTON_HEIGHT 30
#define PADDING 10
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

// URL Manager state
typedef struct {
    int selected_url_id;
    int scroll_offset;
    bool show_filters;
    bool show_blocked;
    char search_query[256];
    char add_url_input[2048];
    bool add_url_active;
} URLManagerState;

static URLManagerState url_state = {0};

// Draw a button and return true if clicked
static bool draw_button(SDL_Renderer* renderer, const char* label, int x, int y, int width, int height, bool enabled) {
    SDL_Rect button = {x, y, width, height};
    
    if (enabled) {
        SDL_SetRenderDrawColor(renderer, 70, 90, 130, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 50, 50, 60, 255);
    }
    
    SDL_RenderFillRect(renderer, &button);
    
    SDL_SetRenderDrawColor(renderer, 100, 120, 160, 255);
    SDL_RenderDrawRect(renderer, &button);
    
    SDL_Color text_color = enabled ? (SDL_Color){255, 255, 255, 255} : (SDL_Color){150, 150, 150, 255};
    draw_text(renderer, label, x + 10, y + 7, text_color);
    
    return false; // Click detection handled elsewhere
}

// Draw URL list section
static void draw_url_list(SDL_Renderer* renderer, int x, int y, int width, int height) {
    // Background
    SDL_SetRenderDrawColor(renderer, 30, 35, 40, 255);
    SDL_Rect list_bg = {x, y, width, height};
    SDL_RenderFillRect(renderer, &list_bg);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 80, 90, 100, 255);
    SDL_RenderDrawRect(renderer, &list_bg);
    
    // Header
    SDL_Color header_color = {200, 220, 255, 255};
    draw_text(renderer, "URL List", x + PADDING, y + PADDING, header_color);
    
    // Column headers
    int header_y = y + 40;
    SDL_SetRenderDrawColor(renderer, 40, 45, 50, 255);
    SDL_Rect header_bg = {x, header_y, width, 30};
    SDL_RenderFillRect(renderer, &header_bg);
    
    SDL_Color col_color = {180, 200, 220, 255};
    draw_text(renderer, "URL", x + PADDING, header_y + 7, col_color);
    draw_text(renderer, "Domain", x + 400, header_y + 7, col_color);
    draw_text(renderer, "Status", x + 600, header_y + 7, col_color);
    draw_text(renderer, "Type", x + 700, header_y + 7, col_color);
    
    // URL entries (placeholder - will integrate with backend)
    int entry_y = header_y + 35;
    SDL_Color text_color = {200, 200, 200, 255};
    
    // Example entries
    draw_text(renderer, "https://example.com/page1", x + PADDING, entry_y, text_color);
    draw_text(renderer, "example.com", x + 400, entry_y, text_color);
    draw_text(renderer, "Pending", x + 600, entry_y, text_color);
    draw_text(renderer, "HTML", x + 700, entry_y, text_color);
    
    entry_y += 25;
    draw_text(renderer, "https://test.org/article", x + PADDING, entry_y, text_color);
    draw_text(renderer, "test.org", x + 400, entry_y, text_color);
    draw_text(renderer, "Crawled", x + 600, entry_y, text_color);
    draw_text(renderer, "HTML", x + 700, entry_y, text_color);
    
    // Status text
    SDL_Color status_color = {150, 170, 190, 255};
    char status_text[128];
    snprintf(status_text, sizeof(status_text), "Total URLs: 0 | Pending: 0 | Crawled: 0 | Blocked: 0");
    draw_text(renderer, status_text, x + PADDING, y + height - 25, status_color);
}

// Draw actions panel
static void draw_actions_panel(SDL_Renderer* renderer, int x, int y, int width, int height) {
    // Background
    SDL_SetRenderDrawColor(renderer, 35, 40, 45, 255);
    SDL_Rect panel_bg = {x, y, width, height};
    SDL_RenderFillRect(renderer, &panel_bg);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 80, 90, 100, 255);
    SDL_RenderDrawRect(renderer, &panel_bg);
    
    // Header
    SDL_Color header_color = {200, 220, 255, 255};
    draw_text(renderer, "Actions", x + PADDING, y + PADDING, header_color);
    
    int button_y = y + 40;
    int button_width = width - (PADDING * 2);
    
    // Add URL section
    SDL_Color label_color = {180, 200, 220, 255};
    draw_text(renderer, "Add URL:", x + PADDING, button_y, label_color);
    button_y += 25;
    
    // Input field
    SDL_SetRenderDrawColor(renderer, 20, 25, 30, 255);
    SDL_Rect input_rect = {x + PADDING, button_y, button_width, 30};
    SDL_RenderFillRect(renderer, &input_rect);
    SDL_SetRenderDrawColor(renderer, 100, 120, 140, 255);
    SDL_RenderDrawRect(renderer, &input_rect);
    
    SDL_Color input_color = {200, 200, 200, 255};
    if (url_state.add_url_input[0] != '\0') {
        draw_text(renderer, url_state.add_url_input, x + PADDING + 5, button_y + 7, input_color);
    } else {
        SDL_Color placeholder_color = {100, 100, 100, 255};
        draw_text(renderer, "Enter URL...", x + PADDING + 5, button_y + 7, placeholder_color);
    }
    
    button_y += 40;
    
    // Action buttons
    draw_button(renderer, "Add URL", x + PADDING, button_y, button_width, BUTTON_HEIGHT, true);
    button_y += BUTTON_HEIGHT + 10;
    
    draw_button(renderer, "Remove Selected", x + PADDING, button_y, button_width, BUTTON_HEIGHT, false);
    button_y += BUTTON_HEIGHT + 10;
    
    draw_button(renderer, "Block Selected", x + PADDING, button_y, button_width, BUTTON_HEIGHT, false);
    button_y += BUTTON_HEIGHT + 10;
    
    draw_button(renderer, "Export List", x + PADDING, button_y, button_width, BUTTON_HEIGHT, true);
    button_y += BUTTON_HEIGHT + 10;
    
    draw_button(renderer, "Import List", x + PADDING, button_y, button_width, BUTTON_HEIGHT, true);
}

// Draw filters panel
static void draw_filters_panel(SDL_Renderer* renderer, int x, int y, int width, int height) {
    // Background
    SDL_SetRenderDrawColor(renderer, 35, 40, 45, 255);
    SDL_Rect panel_bg = {x, y, width, height};
    SDL_RenderFillRect(renderer, &panel_bg);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 80, 90, 100, 255);
    SDL_RenderDrawRect(renderer, &panel_bg);
    
    // Header
    SDL_Color header_color = {200, 220, 255, 255};
    draw_text(renderer, "Filters", x + PADDING, y + PADDING, header_color);
    
    int filter_y = y + 40;
    
    // File type filters
    SDL_Color label_color = {180, 200, 220, 255};
    draw_text(renderer, "File Types:", x + PADDING, filter_y, label_color);
    filter_y += 25;
    
    // Checkboxes (placeholder)
    const char* file_types[] = {"HTML", "PDF", "TXT", "DOC", "Images", "Video", "Audio", "Archives", "Code", "Data"};
    for (int i = 0; i < 10; i++) {
        // Checkbox
        SDL_Rect checkbox = {x + PADDING, filter_y, 15, 15};
        SDL_SetRenderDrawColor(renderer, 60, 70, 80, 255);
        SDL_RenderFillRect(renderer, &checkbox);
        SDL_SetRenderDrawColor(renderer, 100, 120, 140, 255);
        SDL_RenderDrawRect(renderer, &checkbox);
        
        // Checked state (all checked by default)
        SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
        SDL_Rect check = {x + PADDING + 3, filter_y + 3, 9, 9};
        SDL_RenderFillRect(renderer, &check);
        
        // Label
        SDL_Color type_color = {200, 200, 200, 255};
        draw_text(renderer, file_types[i], x + PADDING + 25, filter_y, type_color);
        
        filter_y += 20;
    }
    
    filter_y += 10;
    
    // Domain filters
    draw_text(renderer, "Domain Filter:", x + PADDING, filter_y, label_color);
    filter_y += 25;
    
    draw_button(renderer, "Whitelist", x + PADDING, filter_y, (width - PADDING * 3) / 2, BUTTON_HEIGHT, true);
    draw_button(renderer, "Blacklist", x + PADDING + (width - PADDING * 3) / 2 + 10, filter_y, (width - PADDING * 3) / 2, BUTTON_HEIGHT, true);
}

// Main render function
void draw_url_manager_tab(SDL_Renderer* renderer, AppState* state) {
    (void)state; // Unused for now
    
    // Calculate layout using window constants
    int content_x = SIDEBAR_WIDTH;
    int content_y = SUBMENU_HEIGHT;
    int content_width = WINDOW_WIDTH - SIDEBAR_WIDTH;
    int content_height = WINDOW_HEIGHT - SUBMENU_HEIGHT;
    
    // 3-column layout
    int col1_width = (content_width * 50) / 100;  // 50% - URL list
    int col2_width = (content_width * 25) / 100;  // 25% - Actions
    int col3_width = (content_width * 25) / 100;  // 25% - Filters
    
    int col1_x = content_x + PADDING;
    int col2_x = col1_x + col1_width + PADDING;
    int col3_x = col2_x + col2_width + PADDING;
    
    // Draw columns
    draw_url_list(renderer, col1_x, content_y + PADDING, col1_width - PADDING, content_height - PADDING * 2);
    draw_actions_panel(renderer, col2_x, content_y + PADDING, col2_width - PADDING, content_height - PADDING * 2);
    draw_filters_panel(renderer, col3_x, content_y + PADDING, col3_width - PADDING * 2, content_height - PADDING * 2);
}

// Handle click events
void handle_url_manager_click(AppState* state, int x, int y) {
    // TODO: Implement click handling for buttons and input fields
    (void)state;
    (void)x;
    (void)y;
}