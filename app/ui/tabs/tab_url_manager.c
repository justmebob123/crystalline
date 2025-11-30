// app/ui/tabs/tab_url_manager.c - URL Manager Tab Implementation
#include "../../app_common.h"
#include "../../../src/crawler/crawler_url_manager.h"
#include "../../../src/crawler/url_database.h"
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
#define MAX_URLS_DISPLAY 20

// URL Manager state
typedef struct {
    CrawlerURLManager* url_manager;
    bool initialized;
    uint64_t selected_url_id;
    int scroll_offset;
    bool show_filters;
    bool show_blocked;
    char search_query[256];
    char add_url_input[2048];
    bool add_url_active;
    
    // Statistics
    int total_urls;
    int pending_urls;
    int crawled_urls;
    int blocked_urls;
    
    // URL list cache
    URLEntry** url_list;
    int url_list_count;
    bool url_list_dirty;
} URLManagerState;

static URLManagerState url_state = {0};

// Initialize URL manager
static void init_url_manager_state(void) {
    if (url_state.initialized) return;
    
    // Create URL manager with database
    url_state.url_manager = crawler_url_manager_create("data/crawler/crawler.db");
    if (!url_state.url_manager) {
        fprintf(stderr, "ERROR: Failed to create URL manager for URL Manager tab\n");
        return;
    }
    
    url_state.initialized = true;
    url_state.selected_url_id = 0;
    url_state.scroll_offset = 0;
    url_state.url_list_dirty = true;
    
    printf("URL Manager tab: Initialized with database\n");
}

// Update statistics from database
static void update_statistics(void) {
    if (!url_state.url_manager) return;
    
    crawler_url_manager_get_stats(url_state.url_manager, 
                                  &url_state.total_urls,
                                  &url_state.pending_urls,
                                  &url_state.crawled_urls,
                                  &url_state.blocked_urls);
}

// Refresh URL list from database
static void refresh_url_list(void) {
    if (!url_state.url_manager) return;
    
    // Free old list if exists
    if (url_state.url_list) {
        url_db_free_entries(url_state.url_list, url_state.url_list_count);
        url_state.url_list = NULL;
    }
    
    // Get database from manager
    URLDatabase* db = crawler_url_manager_get_database(url_state.url_manager);
    if (!db) return;
    
    // Query all URLs (no filter = get all)
    url_state.url_list = url_db_query(db, NULL, &url_state.url_list_count);
    
    url_state.url_list_dirty = false;
    
    printf("URL Manager: Loaded %d URLs from database\n", url_state.url_list_count);
}

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
    // Initialize if needed
    init_url_manager_state();
    
    // Refresh URL list if dirty
    if (url_state.url_list_dirty) {
        refresh_url_list();
        update_statistics();
    }
    
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
    SDL_Rect header_bg_rect = {x, header_y, width, 30};
    SDL_RenderFillRect(renderer, &header_bg_rect);
    
    SDL_Color col_color = {180, 200, 220, 255};
    draw_text(renderer, "URL", x + PADDING, header_y + 7, col_color);
    draw_text(renderer, "Domain", x + 400, header_y + 7, col_color);
    draw_text(renderer, "Status", x + 600, header_y + 7, col_color);
    draw_text(renderer, "Type", x + 700, header_y + 7, col_color);
    
    // URL entries from database
    int entry_y = header_y + 35;
    SDL_Color text_color = {200, 200, 200, 255};
    SDL_Color pending_color = {255, 200, 100, 255};
    SDL_Color crawled_color = {100, 255, 100, 255};
    SDL_Color blocked_color = {255, 100, 100, 255};
    
    if (url_state.url_list && url_state.url_list_count > 0) {
        int max_display = (height - 100) / 25; // Calculate how many URLs fit
        int start_idx = url_state.scroll_offset;
        int end_idx = start_idx + max_display;
        if (end_idx > url_state.url_list_count) {
            end_idx = url_state.url_list_count;
        }
        
        for (int i = start_idx; i < end_idx; i++) {
            URLEntry* entry = url_state.url_list[i];
            if (!entry) continue;
            
            // Highlight selected URL
            if (entry->id == url_state.selected_url_id) {
                SDL_Rect sel_rect = {x, entry_y - 2, width, 22};
                SDL_SetRenderDrawColor(renderer, 50, 70, 100, 255);
                SDL_RenderFillRect(renderer, &sel_rect);
            }
            
            // Truncate URL if too long
            char url_display[60];
            if (strlen(entry->url) > 55) {
                strncpy(url_display, entry->url, 52);
                url_display[52] = '.';
                url_display[53] = '.';
                url_display[54] = '.';
                url_display[55] = '\0';
            } else {
                strncpy(url_display, entry->url, sizeof(url_display) - 1);
                url_display[sizeof(url_display) - 1] = '\0';
            }
            
            draw_text(renderer, url_display, x + PADDING, entry_y, text_color);
            draw_text(renderer, entry->domain, x + 400, entry_y, text_color);
            
            // Status with color
            SDL_Color status_color = text_color;
            if (strcmp(entry->status, "pending") == 0) {
                status_color = pending_color;
            } else if (strcmp(entry->status, "crawled") == 0) {
                status_color = crawled_color;
            } else if (strcmp(entry->status, "blocked") == 0) {
                status_color = blocked_color;
            }
            draw_text(renderer, entry->status, x + 600, entry_y, status_color);
            draw_text(renderer, entry->file_type, x + 700, entry_y, text_color);
            
            entry_y += 25;
        }
    } else {
        // No URLs message
        SDL_Color msg_color = {150, 150, 150, 255};
        draw_text(renderer, "No URLs in database", x + PADDING, entry_y, msg_color);
        entry_y += 20;
        draw_text(renderer, "Add a URL to get started", x + PADDING, entry_y, msg_color);
    }
    
    // Status text with real statistics
    SDL_Color status_color = {150, 170, 190, 255};
    char status_text[256];
    snprintf(status_text, sizeof(status_text), 
             "Total URLs: %d | Pending: %d | Crawled: %d | Blocked: %d",
             url_state.total_urls, url_state.pending_urls, 
             url_state.crawled_urls, url_state.blocked_urls);
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
    
    // Initialize if needed
    init_url_manager_state();
    
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

// Handle keyboard input
void handle_url_manager_keyboard(AppState* state, SDL_Event* event) {
    if (!state || !event) return;
    
    // Handle text input for add URL field
    if (url_state.add_url_active && event->type == SDL_TEXTINPUT) {
        size_t len = strlen(url_state.add_url_input);
        if (len < sizeof(url_state.add_url_input) - 1) {
            strncat(url_state.add_url_input, event->text.text, 
                   sizeof(url_state.add_url_input) - len - 1);
        }
    }
    
    // Handle backspace
    if (url_state.add_url_active && event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_BACKSPACE) {
            size_t len = strlen(url_state.add_url_input);
            if (len > 0) {
                url_state.add_url_input[len - 1] = '\0';
            }
        }
    }
}

// Cleanup function
void cleanup_url_manager_tab(void) {
    if (url_state.url_list) {
        url_db_free_entries(url_state.url_list, url_state.url_list_count);
        url_state.url_list = NULL;
    }
    
    if (url_state.url_manager) {
        crawler_url_manager_destroy(url_state.url_manager);
        url_state.url_manager = NULL;
    }
    
    url_state.initialized = false;
}

// Handle click events
void handle_url_manager_click(AppState* state, int x, int y) {
    if (!state) return;
    
    init_url_manager_state();
    if (!url_state.url_manager) return;
    
    // Calculate layout (same as draw function)
    int content_x = SIDEBAR_WIDTH;
    int content_y = SUBMENU_HEIGHT;
    int content_width = WINDOW_WIDTH - SIDEBAR_WIDTH;
    
    int col1_width = (content_width * 50) / 100;
    int col2_width = (content_width * 25) / 100;
    
    int col1_x = content_x + PADDING;
    int col2_x = col1_x + col1_width + PADDING;
    
    // Check if click is in URL list (for selection)
    int list_y = content_y + PADDING + 75; // After header and column headers
    int list_height = WINDOW_HEIGHT - SUBMENU_HEIGHT - PADDING * 2 - 100;
    
    if (x >= col1_x && x < col1_x + col1_width &&
        y >= list_y && y < list_y + list_height) {
        
        // Calculate which URL was clicked
        int relative_y = y - list_y;
        int url_index = (relative_y / 25) + url_state.scroll_offset;
        
        if (url_index >= 0 && url_index < url_state.url_list_count) {
            url_state.selected_url_id = url_state.url_list[url_index]->id;
            printf("Selected URL ID: %lu\n", (unsigned long)url_state.selected_url_id);
        }
        return;
    }
    
    // Check Add URL button
    int button_y = content_y + PADDING + 73;
    SDL_Rect add_btn = {col2_x + PADDING, button_y, col2_width - PADDING * 2, BUTTON_HEIGHT};
    
    if (x >= add_btn.x && x < add_btn.x + add_btn.w &&
        y >= add_btn.y && y < add_btn.y + add_btn.h) {
        
        // Add URL from input
        if (strlen(url_state.add_url_input) > 0) {
            // Validate URL
            if (strstr(url_state.add_url_input, "http://") == url_state.add_url_input ||
                strstr(url_state.add_url_input, "https://") == url_state.add_url_input) {
                
                int result = crawler_url_manager_add(url_state.url_manager, 
                                                     url_state.add_url_input, 
                                                     "manual");
                if (result == 0) {
                    printf("Added URL: %s\n", url_state.add_url_input);
                    url_state.add_url_input[0] = '\0';
                    url_state.url_list_dirty = true;
                    update_statistics();
                } else {
                    printf("Failed to add URL (may already exist)\n");
                }
            } else {
                printf("Invalid URL: must start with http:// or https://\n");
            }
        }
        return;
    }
    
    // Check Remove Selected button
    button_y += BUTTON_HEIGHT + 10;
    SDL_Rect remove_btn = {col2_x + PADDING, button_y, col2_width - PADDING * 2, BUTTON_HEIGHT};
    
    if (x >= remove_btn.x && x < remove_btn.x + remove_btn.w &&
        y >= remove_btn.y && y < remove_btn.y + remove_btn.h) {
        
        if (url_state.selected_url_id >= 0) {
            URLDatabase* db = crawler_url_manager_get_database(url_state.url_manager);
            if (db) {
                int result = url_db_remove(db, url_state.selected_url_id);
                if (result == 0) {
                    printf("Removed URL ID: %lu\n", (unsigned long)url_state.selected_url_id);
                    url_state.selected_url_id = 0;
                    url_state.url_list_dirty = true;
                    update_statistics();
                } else {
                    printf("Failed to remove URL\n");
                }
            }
        } else {
            printf("No URL selected\n");
        }
        return;
    }
    
    // Check Block Selected button
    button_y += BUTTON_HEIGHT + 10;
    SDL_Rect block_btn = {col2_x + PADDING, button_y, col2_width - PADDING * 2, BUTTON_HEIGHT};
    
    if (x >= block_btn.x && x < block_btn.x + block_btn.w &&
        y >= block_btn.y && y < block_btn.y + block_btn.h) {
        
        if (url_state.selected_url_id >= 0) {
            URLDatabase* db = crawler_url_manager_get_database(url_state.url_manager);
            if (db) {
                int result = url_db_block(db, url_state.selected_url_id);
                if (result == 0) {
                    printf("Blocked URL ID: %d\n", url_state.selected_url_id);
                    url_state.url_list_dirty = true;
                    update_statistics();
                } else {
                    printf("Failed to block URL\n");
                }
            }
        } else {
            printf("No URL selected\n");
        }
        return;
    }
    
    // Check Export List button
    button_y += BUTTON_HEIGHT + 10;
    SDL_Rect export_btn = {col2_x + PADDING, button_y, col2_width - PADDING * 2, BUTTON_HEIGHT};
    
    if (x >= export_btn.x && x < export_btn.x + export_btn.w &&
        y >= export_btn.y && y < export_btn.y + export_btn.h) {
        
        printf("Export List clicked (not yet implemented)\n");
        // TODO: Implement export functionality
        return;
    }
    
    // Check Import List button
    button_y += BUTTON_HEIGHT + 10;
    SDL_Rect import_btn = {col2_x + PADDING, button_y, col2_width - PADDING * 2, BUTTON_HEIGHT};
    
    if (x >= import_btn.x && x < import_btn.x + import_btn.w &&
        y >= import_btn.y && y < import_btn.y + import_btn.h) {
        
        printf("Import List clicked (not yet implemented)\n");
        // TODO: Implement import functionality
        return;
    }
}