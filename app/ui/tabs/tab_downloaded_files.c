// app/ui/tabs/tab_downloaded_files.c - Downloaded Files Viewer Tab
#include "../../app_common.h"
#include "../../../src/crawler/crawler_url_manager.h"
#include "../../../src/crawler/url_database.h"
#include <SDL2/SDL_ttf.h>
#include <sys/stat.h>
#include <dirent.h>

extern TTF_Font* get_global_font(void);
extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);

// Use constants from app_common.h
#define SIDEBAR_WIDTH 200
#define SUBMENU_HEIGHT 40
#define BUTTON_HEIGHT 30
#define PADDING 10
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define MAX_FILES_DISPLAY 20
#define DOWNLOADS_DIR "data/crawler/downloads"

// File info structure
typedef struct {
    uint64_t url_id;
    char filename[256];
    char url[512];
    char domain[128];
    char file_type[32];
    time_t download_time;
    off_t file_size;
    bool file_exists;
} FileInfo;

// Downloaded Files state
typedef struct {
    CrawlerURLManager* url_manager;
    bool initialized;
    int selected_file_idx;
    int scroll_offset;
    char search_query[256];
    bool show_preview;
    
    // File list
    FileInfo* file_list;
    int file_count;
    bool file_list_dirty;
    
    // Preview
    char preview_content[4096];
    bool preview_loaded;
} DownloadedFilesState;

static DownloadedFilesState files_state = {0};

// Initialize state
static void init_downloaded_files_state(void) {
    if (files_state.initialized) return;
    
    // Create URL manager with database
    files_state.url_manager = crawler_url_manager_create("data/crawler/crawler.db");
    if (!files_state.url_manager) {
        fprintf(stderr, "ERROR: Failed to create URL manager for Downloaded Files tab\n");
        return;
    }
    
    files_state.initialized = true;
    files_state.selected_file_idx = -1;
    files_state.scroll_offset = 0;
    files_state.file_list_dirty = true;
    files_state.preview_loaded = false;
    
    // Create downloads directory if it doesn't exist
    mkdir("data", 0755);
    mkdir("data/crawler", 0755);
    mkdir(DOWNLOADS_DIR, 0755);
    
    printf("Downloaded Files tab: Initialized\n");
}

// Get file size
static off_t get_file_size(const char* filepath) {
    struct stat st;
    if (stat(filepath, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

// Format file size for display
static void format_file_size(off_t size, char* buffer, size_t buffer_size) {
    if (size < 1024) {
        snprintf(buffer, buffer_size, "%ld B", (long)size);
    } else if (size < 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f KB", size / 1024.0);
    } else if (size < 1024 * 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f MB", size / (1024.0 * 1024.0));
    } else {
        snprintf(buffer, buffer_size, "%.1f GB", size / (1024.0 * 1024.0 * 1024.0));
    }
}

// Refresh file list from database
static void refresh_file_list(void) {
    if (!files_state.url_manager) return;
    
    // Free old list if exists
    if (files_state.file_list) {
        free(files_state.file_list);
        files_state.file_list = NULL;
    }
    
    // Get database from manager
    URLDatabase* db = crawler_url_manager_get_database(files_state.url_manager);
    if (!db) return;
    
    // Query crawled URLs only
    int url_count = 0;
    URLEntry** urls = url_db_query(db, "status='crawled'", &url_count);
    
    if (!urls || url_count == 0) {
        files_state.file_count = 0;
        files_state.file_list_dirty = false;
        if (urls) url_db_free_entries(urls, url_count);
        return;
    }
    
    // Allocate file list
    files_state.file_list = (FileInfo*)calloc(url_count, sizeof(FileInfo));
    files_state.file_count = 0;
    
    // Build file list
    for (int i = 0; i < url_count; i++) {
        URLEntry* entry = urls[i];
        if (!entry) continue;
        
        FileInfo* file = &files_state.file_list[files_state.file_count];
        
        // Extract filename from URL (last part of path)
        const char* last_slash = strrchr(entry->path, '/');
        if (last_slash && last_slash[1] != '\0') {
            strncpy(file->filename, last_slash + 1, sizeof(file->filename) - 1);
        } else {
            snprintf(file->filename, sizeof(file->filename), "index.%s", entry->file_type);
        }
        
        // Copy URL info
        file->url_id = entry->id;
        strncpy(file->url, entry->url, sizeof(file->url) - 1);
        strncpy(file->domain, entry->domain, sizeof(file->domain) - 1);
        strncpy(file->file_type, entry->file_type, sizeof(file->file_type) - 1);
        file->download_time = entry->last_crawled;
        
        // Check if file exists locally
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", DOWNLOADS_DIR, file->filename);
        file->file_size = get_file_size(filepath);
        file->file_exists = (file->file_size > 0);
        
        files_state.file_count++;
    }
    
    url_db_free_entries(urls, url_count);
    files_state.file_list_dirty = false;
    
    printf("Downloaded Files: Loaded %d files\n", files_state.file_count);
}

// Draw file browser section
static void draw_file_browser(SDL_Renderer* renderer, int x, int y, int width, int height) {
    // Initialize if needed
    init_downloaded_files_state();
    
    // Refresh file list if dirty
    if (files_state.file_list_dirty) {
        refresh_file_list();
    }
    
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
    
    // Search bar (placeholder for now)
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
    SDL_Rect header_bg_rect = {x, header_y, width, 30};
    SDL_RenderFillRect(renderer, &header_bg_rect);
    
    SDL_Color col_color = {180, 200, 220, 255};
    draw_text(renderer, "Filename", x + PADDING, header_y + 7, col_color);
    draw_text(renderer, "Domain", x + 250, header_y + 7, col_color);
    draw_text(renderer, "Type", x + 400, header_y + 7, col_color);
    draw_text(renderer, "Size", x + 480, header_y + 7, col_color);
    draw_text(renderer, "Date", x + 560, header_y + 7, col_color);
    
    // File entries from database
    int entry_y = header_y + 35;
    SDL_Color text_color = {200, 200, 200, 255};
    SDL_Color selected_color = {100, 150, 200, 255};
    SDL_Color missing_color = {150, 150, 150, 255};
    
    if (files_state.file_list && files_state.file_count > 0) {
        int max_display = (height - 140) / 25;
        int start_idx = files_state.scroll_offset;
        int end_idx = start_idx + max_display;
        if (end_idx > files_state.file_count) {
            end_idx = files_state.file_count;
        }
        
        off_t total_size = 0;
        
        for (int i = start_idx; i < end_idx; i++) {
            FileInfo* file = &files_state.file_list[i];
            
            // Highlight selected file
            if (i == files_state.selected_file_idx) {
                SDL_Rect sel_rect = {x, entry_y - 2, width, 22};
                SDL_SetRenderDrawColor(renderer, 50, 70, 100, 255);
                SDL_RenderFillRect(renderer, &sel_rect);
            }
            
            // Use different color if file doesn't exist
            SDL_Color file_color = file->file_exists ? text_color : missing_color;
            
            // Truncate filename if too long
            char filename_display[40];
            if (strlen(file->filename) > 35) {
                strncpy(filename_display, file->filename, 32);
                filename_display[32] = '.';
                filename_display[33] = '.';
                filename_display[34] = '.';
                filename_display[35] = '\0';
            } else {
                strncpy(filename_display, file->filename, sizeof(filename_display) - 1);
            }
            
            draw_text(renderer, filename_display, x + PADDING, entry_y, file_color);
            draw_text(renderer, file->domain, x + 250, entry_y, file_color);
            draw_text(renderer, file->file_type, x + 400, entry_y, file_color);
            
            // Format file size
            char size_str[32];
            if (file->file_exists) {
                format_file_size(file->file_size, size_str, sizeof(size_str));
                total_size += file->file_size;
            } else {
                strcpy(size_str, "N/A");
            }
            draw_text(renderer, size_str, x + 480, entry_y, file_color);
            
            // Format date
            char date_str[32];
            struct tm* tm_info = localtime(&file->download_time);
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);
            draw_text(renderer, date_str, x + 560, entry_y, file_color);
            
            entry_y += 25;
        }
        
        // Status text with real statistics
        SDL_Color status_color = {150, 170, 190, 255};
        char status_text[256];
        char total_size_str[32];
        format_file_size(total_size, total_size_str, sizeof(total_size_str));
        snprintf(status_text, sizeof(status_text), 
                 "Total Files: %d | Total Size: %s",
                 files_state.file_count, total_size_str);
        draw_text(renderer, status_text, x + PADDING, y + height - 25, status_color);
    } else {
        // No files message
        SDL_Color msg_color = {150, 150, 150, 255};
        draw_text(renderer, "No downloaded files", x + PADDING, entry_y, msg_color);
        entry_y += 20;
        draw_text(renderer, "Crawl some URLs to see files here", x + PADDING, entry_y, msg_color);
        
        // Status text
        SDL_Color status_color = {150, 170, 190, 255};
        char status_text[128];
        snprintf(status_text, sizeof(status_text), "Total Files: 0 | Total Size: 0 B");
        draw_text(renderer, status_text, x + PADDING, y + height - 25, status_color);
    }
}

// Load file preview
static void load_file_preview(int file_idx) {
    if (file_idx < 0 || file_idx >= files_state.file_count) {
        files_state.preview_loaded = false;
        return;
    }
    
    FileInfo* file = &files_state.file_list[file_idx];
    
    // Build file path
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s", DOWNLOADS_DIR, file->filename);
    
    // Try to read file
    FILE* f = fopen(filepath, "r");
    if (!f) {
        snprintf(files_state.preview_content, sizeof(files_state.preview_content),
                 "Error: Could not open file\n%s", filepath);
        files_state.preview_loaded = true;
        return;
    }
    
    // Read first 4KB for preview
    size_t bytes_read = fread(files_state.preview_content, 1, 
                             sizeof(files_state.preview_content) - 1, f);
    files_state.preview_content[bytes_read] = '\0';
    fclose(f);
    
    files_state.preview_loaded = true;
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
    
    if (files_state.selected_file_idx < 0 || 
        files_state.selected_file_idx >= files_state.file_count) {
        // No file selected
        SDL_Color msg_color = {150, 150, 150, 255};
        draw_text(renderer, "Select a file to preview", x + PADDING, y + 60, msg_color);
        return;
    }
    
    FileInfo* file = &files_state.file_list[files_state.selected_file_idx];
    
    // File info
    int info_y = y + 40;
    SDL_Color label_color = {180, 200, 220, 255};
    SDL_Color value_color = {200, 200, 200, 255};
    
    draw_text(renderer, "URL:", x + PADDING, info_y, label_color);
    
    // Truncate URL if too long
    char url_display[80];
    if (strlen(file->url) > 70) {
        strncpy(url_display, file->url, 67);
        url_display[67] = '.';
        url_display[68] = '.';
        url_display[69] = '.';
        url_display[70] = '\0';
    } else {
        strncpy(url_display, file->url, sizeof(url_display) - 1);
    }
    draw_text(renderer, url_display, x + 60, info_y, value_color);
    info_y += 20;
    
    draw_text(renderer, "File:", x + PADDING, info_y, label_color);
    draw_text(renderer, file->filename, x + 60, info_y, value_color);
    info_y += 20;
    
    draw_text(renderer, "Size:", x + PADDING, info_y, label_color);
    char size_str[32];
    format_file_size(file->file_size, size_str, sizeof(size_str));
    draw_text(renderer, size_str, x + 60, info_y, value_color);
    info_y += 20;
    
    draw_text(renderer, "Type:", x + PADDING, info_y, label_color);
    draw_text(renderer, file->file_type, x + 60, info_y, value_color);
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
    
    // Load preview if not loaded
    if (!files_state.preview_loaded) {
        load_file_preview(files_state.selected_file_idx);
    }
    
    // Display preview content
    SDL_Color content_color = {180, 180, 180, 255};
    int content_y = info_y + 10;
    int max_lines = (content_rect.h - 20) / 18;
    
    if (files_state.preview_loaded) {
        char* line = files_state.preview_content;
        int line_count = 0;
        
        while (line && *line && line_count < max_lines) {
            char* newline = strchr(line, '\n');
            char line_buffer[120];
            
            if (newline) {
                size_t len = newline - line;
                if (len > 110) len = 110;
                strncpy(line_buffer, line, len);
                line_buffer[len] = '\0';
                line = newline + 1;
            } else {
                strncpy(line_buffer, line, 110);
                line_buffer[110] = '\0';
                line = NULL;
            }
            
            draw_text(renderer, line_buffer, x + PADDING + 5, content_y, content_color);
            content_y += 18;
            line_count++;
        }
    }
    
    // Action buttons
    int button_y = y + height - 60;
    int button_width = (width - PADDING * 4) / 3;
    
    // Open button (placeholder)
    SDL_Rect open_btn = {x + PADDING, button_y, button_width, BUTTON_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 70, 90, 130, 255);
    SDL_RenderFillRect(renderer, &open_btn);
    SDL_SetRenderDrawColor(renderer, 100, 120, 160, 255);
    SDL_RenderDrawRect(renderer, &open_btn);
    SDL_Color btn_color = {255, 255, 255, 255};
    draw_text(renderer, "Open", x + PADDING + 10, button_y + 7, btn_color);
    
    // Delete button (placeholder)
    SDL_Rect del_btn = {x + PADDING * 2 + button_width, button_y, button_width, BUTTON_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 130, 70, 70, 255);
    SDL_RenderFillRect(renderer, &del_btn);
    SDL_SetRenderDrawColor(renderer, 160, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &del_btn);
    draw_text(renderer, "Delete", x + PADDING * 2 + button_width + 10, button_y + 7, btn_color);
    
    // Re-crawl button (placeholder)
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
    
    // Initialize if needed
    init_downloaded_files_state();
    
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
    if (!state) return;
    
    init_downloaded_files_state();
    if (!files_state.url_manager) return;
    
    // Calculate layout (same as draw function)
    int content_x = SIDEBAR_WIDTH;
    int content_y = SUBMENU_HEIGHT;
    int content_width = WINDOW_WIDTH - SIDEBAR_WIDTH;
    
    int col1_width = (content_width * 40) / 100;
    int col1_x = content_x + PADDING;
    
    // Check if click is in file list (for selection)
    int list_y = content_y + PADDING + 115; // After header, search, and column headers
    int list_height = WINDOW_HEIGHT - SUBMENU_HEIGHT - PADDING * 2 - 140;
    
    if (x >= col1_x && x < col1_x + col1_width &&
        y >= list_y && y < list_y + list_height) {
        
        // Calculate which file was clicked
        int relative_y = y - list_y;
        int file_index = (relative_y / 25) + files_state.scroll_offset;
        
        if (file_index >= 0 && file_index < files_state.file_count) {
            files_state.selected_file_idx = file_index;
            files_state.preview_loaded = false; // Force reload preview
            printf("Selected file: %s\n", files_state.file_list[file_index].filename);
        }
        return;
    }
    
    // TODO: Add button click handlers (Open, Delete, Re-crawl)
}

// Cleanup function
void cleanup_downloaded_files_tab(void) {
    if (files_state.file_list) {
        free(files_state.file_list);
        files_state.file_list = NULL;
    }
    
    if (files_state.url_manager) {
        crawler_url_manager_destroy(files_state.url_manager);
        files_state.url_manager = NULL;
    }
    
    files_state.initialized = false;
}