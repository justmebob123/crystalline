// app/ui/tabs/tab_downloaded_files.c - Downloaded Files Viewer Tab
#include "../../app_common.h"
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

// Downloaded Files state
typedef struct {
    int selected_file_id;
    int scroll_offset;
    char search_query[256];
    bool show_preview;
} DownloadedFilesState;

// TODO: Will be used for state management when backend integration is complete
// static DownloadedFilesState files_state = {0};

// Draw file browser section
static void draw_file_browser(SDL_Renderer* renderer, int x, int y, int width, int height) {
    // Background
    SDL_SetRenderDrawColor(renderer, 30, 35, 40, 255);
    SDL_Rect browser_bg = {x, y, width, height};
    SDL_RenderFillRect(renderer, &browser_bg);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 80, 90, 100, 255);
    SDL_RenderDrawRect(renderer, &browser_bg);
    
    // Header
    SDL_Color header_color = {200, 220, 255, 255};
    draw_text(renderer, "Downloaded Files", x + PADDING, y + PADDING, header_color);
    
    // Search bar
    int search_y = y + 40;
    SDL_SetRenderDrawColor(renderer, 20, 25, 30, 255);
    SDL_Rect search_rect = {x + PADDING, search_y, width - PADDING * 2, 30};
    SDL_RenderFillRect(renderer, &search_rect);
    SDL_SetRenderDrawColor(renderer, 100, 120, 140, 255);
    SDL_RenderDrawRect(renderer, &search_rect);
    
    SDL_Color placeholder_color = {100, 100, 100, 255};
    draw_text(renderer, "Search files...", x + PADDING + 5, search_y + 7, placeholder_color);
    
    // Column headers
    int header_y = search_y + 40;
    SDL_SetRenderDrawColor(renderer, 40, 45, 50, 255);
    SDL_Rect header_bg = {x, header_y, width, 30};
    SDL_RenderFillRect(renderer, &header_bg);
    
    SDL_Color col_color = {180, 200, 220, 255};
    draw_text(renderer, "Filename", x + PADDING, header_y + 7, col_color);
    draw_text(renderer, "Domain", x + 250, header_y + 7, col_color);
    draw_text(renderer, "Type", x + 400, header_y + 7, col_color);
    draw_text(renderer, "Size", x + 480, header_y + 7, col_color);
    draw_text(renderer, "Date", x + 560, header_y + 7, col_color);
    
    // File entries (placeholder)
    int entry_y = header_y + 35;
    SDL_Color text_color = {200, 200, 200, 255};
    
    // Example entries
    draw_text(renderer, "page1.html", x + PADDING, entry_y, text_color);
    draw_text(renderer, "example.com", x + 250, entry_y, text_color);
    draw_text(renderer, "HTML", x + 400, entry_y, text_color);
    draw_text(renderer, "12 KB", x + 480, entry_y, text_color);
    draw_text(renderer, "2024-12-29", x + 560, entry_y, text_color);
    
    entry_y += 25;
    draw_text(renderer, "article.html", x + PADDING, entry_y, text_color);
    draw_text(renderer, "test.org", x + 250, entry_y, text_color);
    draw_text(renderer, "HTML", x + 400, entry_y, text_color);
    draw_text(renderer, "8 KB", x + 480, entry_y, text_color);
    draw_text(renderer, "2024-12-29", x + 560, entry_y, text_color);
    
    entry_y += 25;
    draw_text(renderer, "document.pdf", x + PADDING, entry_y, text_color);
    draw_text(renderer, "docs.com", x + 250, entry_y, text_color);
    draw_text(renderer, "PDF", x + 400, entry_y, text_color);
    draw_text(renderer, "245 KB", x + 480, entry_y, text_color);
    draw_text(renderer, "2024-12-28", x + 560, entry_y, text_color);
    
    // Status text
    SDL_Color status_color = {150, 170, 190, 255};
    char status_text[128];
    snprintf(status_text, sizeof(status_text), "Total Files: 0 | Total Size: 0 KB");
    draw_text(renderer, status_text, x + PADDING, y + height - 25, status_color);
}

// Draw file preview section
static void draw_file_preview(SDL_Renderer* renderer, int x, int y, int width, int height) {
    // Background
    SDL_SetRenderDrawColor(renderer, 35, 40, 45, 255);
    SDL_Rect preview_bg = {x, y, width, height};
    SDL_RenderFillRect(renderer, &preview_bg);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 80, 90, 100, 255);
    SDL_RenderDrawRect(renderer, &preview_bg);
    
    // Header
    SDL_Color header_color = {200, 220, 255, 255};
    draw_text(renderer, "File Preview", x + PADDING, y + PADDING, header_color);
    
    // File info
    int info_y = y + 40;
    SDL_Color label_color = {180, 200, 220, 255};
    SDL_Color value_color = {200, 200, 200, 255};
    
    draw_text(renderer, "URL:", x + PADDING, info_y, label_color);
    draw_text(renderer, "https://example.com/page1", x + 60, info_y, value_color);
    info_y += 20;
    
    draw_text(renderer, "File:", x + PADDING, info_y, label_color);
    draw_text(renderer, "page1.html", x + 60, info_y, value_color);
    info_y += 20;
    
    draw_text(renderer, "Size:", x + PADDING, info_y, label_color);
    draw_text(renderer, "12 KB", x + 60, info_y, value_color);
    info_y += 20;
    
    draw_text(renderer, "Type:", x + PADDING, info_y, label_color);
    draw_text(renderer, "text/html", x + 60, info_y, value_color);
    info_y += 30;
    
    // Content preview
    draw_text(renderer, "Content Preview:", x + PADDING, info_y, label_color);
    info_y += 25;
    
    // Preview area
    SDL_SetRenderDrawColor(renderer, 20, 25, 30, 255);
    SDL_Rect content_rect = {x + PADDING, info_y, width - PADDING * 2, height - info_y - 80};
    SDL_RenderFillRect(renderer, &content_rect);
    SDL_SetRenderDrawColor(renderer, 60, 70, 80, 255);
    SDL_RenderDrawRect(renderer, &content_rect);
    
    // Sample content
    SDL_Color content_color = {180, 180, 180, 255};
    int content_y = info_y + 10;
    draw_text(renderer, "<!DOCTYPE html>", x + PADDING + 5, content_y, content_color);
    content_y += 20;
    draw_text(renderer, "<html>", x + PADDING + 5, content_y, content_color);
    content_y += 20;
    draw_text(renderer, "  <head>", x + PADDING + 5, content_y, content_color);
    content_y += 20;
    draw_text(renderer, "    <title>Example Page</title>", x + PADDING + 5, content_y, content_color);
    content_y += 20;
    draw_text(renderer, "  </head>", x + PADDING + 5, content_y, content_color);
    content_y += 20;
    draw_text(renderer, "  <body>", x + PADDING + 5, content_y, content_color);
    content_y += 20;
    draw_text(renderer, "    <h1>Hello World</h1>", x + PADDING + 5, content_y, content_color);
    
    // Action buttons
    int button_y = y + height - 60;
    int button_width = (width - PADDING * 4) / 3;
    
    // Open button
    SDL_Rect open_btn = {x + PADDING, button_y, button_width, BUTTON_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 70, 90, 130, 255);
    SDL_RenderFillRect(renderer, &open_btn);
    SDL_SetRenderDrawColor(renderer, 100, 120, 160, 255);
    SDL_RenderDrawRect(renderer, &open_btn);
    SDL_Color btn_color = {255, 255, 255, 255};
    draw_text(renderer, "Open", x + PADDING + 10, button_y + 7, btn_color);
    
    // Delete button
    SDL_Rect del_btn = {x + PADDING * 2 + button_width, button_y, button_width, BUTTON_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 130, 70, 70, 255);
    SDL_RenderFillRect(renderer, &del_btn);
    SDL_SetRenderDrawColor(renderer, 160, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &del_btn);
    draw_text(renderer, "Delete", x + PADDING * 2 + button_width + 10, button_y + 7, btn_color);
    
    // Re-crawl button
    SDL_Rect recrawl_btn = {x + PADDING * 3 + button_width * 2, button_y, button_width, BUTTON_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 70, 130, 90, 255);
    SDL_RenderFillRect(renderer, &recrawl_btn);
    SDL_SetRenderDrawColor(renderer, 100, 160, 120, 255);
    SDL_RenderDrawRect(renderer, &recrawl_btn);
    draw_text(renderer, "Re-crawl", x + PADDING * 3 + button_width * 2 + 10, button_y + 7, btn_color);
}

// Main render function
void draw_downloaded_files_tab(SDL_Renderer* renderer, AppState* state) {
    (void)state; // Unused for now
    
    // Calculate layout using window constants
    int content_x = SIDEBAR_WIDTH;
    int content_y = SUBMENU_HEIGHT;
    int content_width = WINDOW_WIDTH - SIDEBAR_WIDTH;
    int content_height = WINDOW_HEIGHT - SUBMENU_HEIGHT;
    
    // 2-column layout: 40% file browser, 60% preview
    int col1_width = (content_width * 40) / 100;
    int col2_width = (content_width * 60) / 100;
    
    int col1_x = content_x + PADDING;
    int col2_x = col1_x + col1_width + PADDING;
    
    // Draw columns
    draw_file_browser(renderer, col1_x, content_y + PADDING, col1_width - PADDING, content_height - PADDING * 2);
    draw_file_preview(renderer, col2_x, content_y + PADDING, col2_width - PADDING * 2, content_height - PADDING * 2);
}

// Handle click events
void handle_downloaded_files_click(AppState* state, int x, int y) {
    // TODO: Implement click handling
    (void)state;
    (void)x;
    (void)y;
}