// app/ui/tabs/tab_research.c - COMPLETE Research Data Browser Tab
#include "../../app_common.h"
#include "../../input_manager.h"
#include "../../text_input.h"
#include "../../ui_layout.h"
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#ifndef DT_REG
#define DT_REG 8
#endif

#define MAX_FILES 200
#define MAX_FILENAME_LENGTH 256
#define MAX_CONTENT_LENGTH 100000

typedef enum {
    SORT_NAME,
    SORT_SIZE,
    SORT_DATE,
    SORT_TYPE
} SortMode;

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char full_path[1024];
    long file_size;
    time_t modified_time;
    char file_type[32];
} ResearchFile;

static ResearchFile research_files[MAX_FILES];
static int file_count = 0;
static int selected_file = -1;
static char file_content[MAX_CONTENT_LENGTH];
static int content_scroll = 0;
static int file_list_scroll = 0;
static SortMode sort_mode = SORT_NAME;
static bool sort_ascending = true;
static char current_directory[512] = "docs/research";
static char search_query[128] = "";

// Text input for search
static TextInput search_input;
static bool search_input_initialized = false;

static const char* get_file_type(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return "Unknown";
    
    if (strcmp(ext, ".txt") == 0) return "Text";
    if (strcmp(ext, ".md") == 0) return "Markdown";
    if (strcmp(ext, ".json") == 0) return "JSON";
    if (strcmp(ext, ".csv") == 0) return "CSV";
    if (strcmp(ext, ".log") == 0) return "Log";
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) return "C Code";
    if (strcmp(ext, ".py") == 0) return "Python";
    if (strcmp(ext, ".sh") == 0) return "Shell";
    
    return "Other";
}

static int compare_files(const void* a, const void* b) {
    ResearchFile* f1 = (ResearchFile*)a;
    ResearchFile* f2 = (ResearchFile*)b;
    
    int result = 0;
    switch (sort_mode) {
        case SORT_NAME:
            result = strcmp(f1->filename, f2->filename);
            break;
        case SORT_SIZE:
            result = (f1->file_size > f2->file_size) - (f1->file_size < f2->file_size);
            break;
        case SORT_DATE:
            result = (f1->modified_time > f2->modified_time) - (f1->modified_time < f2->modified_time);
            break;
        case SORT_TYPE:
            result = strcmp(f1->file_type, f2->file_type);
            break;
    }
    
    return sort_ascending ? result : -result;
}

static void scan_research_directory(const char* dir_path) {
    file_count = 0;
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("Could not open directory: %s\n", dir_path);
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        if (entry->d_name[0] == '.') continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
            // Apply search filter
            if (strlen(search_query) > 0) {
                if (strstr(entry->d_name, search_query) == NULL) continue;
            }
            
            ResearchFile* file = &research_files[file_count];
            snprintf(file->filename, MAX_FILENAME_LENGTH, "%s", entry->d_name);
            snprintf(file->full_path, sizeof(file->full_path), "%s", full_path);
            file->file_size = st.st_size;
            file->modified_time = st.st_mtime;
            snprintf(file->file_type, sizeof(file->file_type), "%s", get_file_type(entry->d_name));
            file_count++;
        }
    }
    closedir(dir);
    
    // Sort files
    qsort(research_files, file_count, sizeof(ResearchFile), compare_files);
    
    printf("Found %d research files in %s\n", file_count, dir_path);
}

static void load_file_content(const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        snprintf(file_content, sizeof(file_content), "Error: Could not open file\n%s", filepath);
        return;
    }
    
    size_t bytes_read = fread(file_content, 1, MAX_CONTENT_LENGTH - 1, f);
    file_content[bytes_read] = '\0';
    fclose(f);
    content_scroll = 0;
    
    printf("Loaded file: %s (%zu bytes)\n", filepath, bytes_read);
}

static const char* format_file_size(long size) {
    static char buffer[32];
    if (size < 1024) {
        snprintf(buffer, sizeof(buffer), "%ld B", size);
    } else if (size < 1024 * 1024) {
        snprintf(buffer, sizeof(buffer), "%.1f KB", size / 1024.0);
    } else {
        snprintf(buffer, sizeof(buffer), "%.1f MB", size / (1024.0 * 1024.0));
    }
    return buffer;
}

static const char* format_time(time_t time) {
    static char buffer[64];
    struct tm* tm_info = localtime(&time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", tm_info);
    return buffer;
}

void draw_research_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    // Initialize search input once
    if (!search_input_initialized) {
        text_input_init(&search_input, "Search:", RENDER_WIDTH + 10, 150, 260, 25);
        text_input_set_text(&search_input, search_query);
        search_input_initialized = true;
    }
    
    int panel_x = RENDER_WIDTH;
    int panel_y = 60;
    int panel_width = CONTROL_PANEL_WIDTH;
    
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {40, 40, 50, 255};
    SDL_Color button_color = {60, 60, 80, 255};
    SDL_Color active_color = {100, 150, 200, 255};
    SDL_Color header_color = {180, 180, 200, 255};
    
    // Draw panel background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_Rect panel_rect = {panel_x, panel_y, panel_width, WINDOW_HEIGHT - panel_y};
    SDL_RenderFillRect(renderer, &panel_rect);
    
    int y = panel_y + 10;
    
    // === SECTION 1: HEADER ===
    draw_text(renderer, "RESEARCH BROWSER", panel_x + 10, y, text_color);
    y += 20;
    
    // Current directory (truncated)
    char dir_display[40];
    snprintf(dir_display, sizeof(dir_display), "%.35s", current_directory);
    draw_text(renderer, dir_display, panel_x + 10, y, (SDL_Color){150, 150, 150, 255});
    y += 20;
    
    // === SECTION 2: CONTROLS ===
    // Scan Button
    SDL_Rect scan_btn = {panel_x + 10, y, 80, 22};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &scan_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &scan_btn);
    draw_text(renderer, "Scan", scan_btn.x + 24, scan_btn.y + 5, text_color);
    
    // Refresh Button
    SDL_Rect refresh_btn = {panel_x + 100, y, 80, 22};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &refresh_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &refresh_btn);
    draw_text(renderer, "Refresh", refresh_btn.x + 16, refresh_btn.y + 5, text_color);
    
    // Up Directory Button
    SDL_Rect up_btn = {panel_x + 190, y, 80, 22};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &up_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &up_btn);
    draw_text(renderer, "Up", up_btn.x + 28, up_btn.y + 5, text_color);
    y += 28;
    
    // Search input (using text_input component)
    text_input_render(&search_input, renderer, get_global_font());
    y += 50;  // Skip past the search input
    
    // === SECTION 3: SORT CONTROLS ===
    draw_text(renderer, "Sort by:", panel_x + 10, y, text_color);
    y += 16;
    
    const char* sort_labels[] = {"Name", "Size", "Date", "Type"};
    int btn_width = (panel_width - 30) / 4;
    for (int i = 0; i < 4; i++) {
        SDL_Rect sort_btn = {panel_x + 10 + i * (btn_width + 3), y, btn_width, 20};
        SDL_Color btn_color = (sort_mode == (SortMode)i) ? active_color : button_color;
        SDL_SetRenderDrawColor(renderer, btn_color.r, btn_color.g, btn_color.b, 255);
        SDL_RenderFillRect(renderer, &sort_btn);
        SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
        SDL_RenderDrawRect(renderer, &sort_btn);
        draw_text(renderer, sort_labels[i], sort_btn.x + 12, sort_btn.y + 4, text_color);
    }
    y += 26;
    
    // File count
    char count_text[64];
    snprintf(count_text, sizeof(count_text), "Files: %d", file_count);
    draw_text(renderer, count_text, panel_x + 10, y, text_color);
    y += 20;
    
    // === SECTION 4: FILE LIST ===
    int list_height = WINDOW_HEIGHT - y - 20;
    SDL_Rect list_rect = {panel_x + 10, y, panel_width - 20, list_height};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &list_rect);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &list_rect);
    
    // Column headers
    int header_y = list_rect.y + 3;
    draw_text(renderer, "Name", list_rect.x + 5, header_y, header_color);
    draw_text(renderer, "Size", list_rect.x + 180, header_y, header_color);
    draw_text(renderer, "Type", list_rect.x + 230, header_y, header_color);
    
    // Draw files
    int file_y = list_rect.y + 20;
    int max_visible = (list_height - 25) / 14;
    for (int i = file_list_scroll; i < file_count && i < file_list_scroll + max_visible; i++) {
        SDL_Color file_color = (i == selected_file) ? 
            (SDL_Color){100, 200, 255, 255} : text_color;
        
        // Filename (truncated)
        char display_name[28];
        snprintf(display_name, sizeof(display_name), "%.24s", research_files[i].filename);
        draw_text(renderer, display_name, list_rect.x + 5, file_y, file_color);
        
        // Size
        draw_text(renderer, format_file_size(research_files[i].file_size), 
                 list_rect.x + 180, file_y, file_color);
        
        // Type
        draw_text(renderer, research_files[i].file_type, list_rect.x + 230, file_y, file_color);
        
        file_y += 14;
    }
    
    // Scroll indicator
    if (file_count > max_visible) {
        int scroll_h = (max_visible * list_height) / file_count;
        int scroll_y = list_rect.y + 20 + (file_list_scroll * (list_height - 20)) / file_count;
        SDL_Rect scroll_bar = {list_rect.x + list_rect.w - 8, scroll_y, 6, scroll_h};
        SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 200);
        SDL_RenderFillRect(renderer, &scroll_bar);
    }
    
    // === VISUALIZATION AREA (FILE VIEWER) ===
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_Rect viz_rect = {0, 60, RENDER_WIDTH, WINDOW_HEIGHT - 60};
    SDL_RenderFillRect(renderer, &viz_rect);
    
    if (selected_file >= 0 && selected_file < file_count) {
        // File header
        draw_text(renderer, research_files[selected_file].filename, 20, 80, text_color);
        
        char info[128];
        snprintf(info, sizeof(info), "%s | %s | Modified: %s", 
                research_files[selected_file].file_type,
                format_file_size(research_files[selected_file].file_size),
                format_time(research_files[selected_file].modified_time));
        draw_text(renderer, info, 20, 100, (SDL_Color){150, 150, 150, 255});
        
        // Separator line
        SDL_SetRenderDrawColor(renderer, 80, 80, 100, 255);
        SDL_RenderDrawLine(renderer, 20, 120, RENDER_WIDTH - 20, 120);
        
        // File content
        if (strlen(file_content) > 0) {
            int content_y = 130;
            int line_height = 14;
            int max_lines = (WINDOW_HEIGHT - 150) / line_height;
            
            // Count total lines
            int total_lines = 1;
            for (size_t i = 0; i < strlen(file_content); i++) {
                if (file_content[i] == '\n') total_lines++;
            }
            
            // Draw visible lines
            const char* ptr = file_content;
            int current_line = 0;
            int lines_drawn = 0;
            
            while (*ptr && lines_drawn < max_lines) {
                // Skip lines before scroll offset
                if (current_line < content_scroll) {
                    while (*ptr && *ptr != '\n') ptr++;
                    if (*ptr == '\n') ptr++;
                    current_line++;
                    continue;
                }
                
                // Draw line
                char line[256];
                int len = 0;
                while (*ptr && *ptr != '\n' && len < 200) {
                    line[len++] = *ptr++;
                }
                if (*ptr == '\n') ptr++;
                line[len] = '\0';
                
                // Line number
                char line_num[16];
                snprintf(line_num, sizeof(line_num), "%3d", current_line + 1);
                draw_text(renderer, line_num, 20, content_y, (SDL_Color){100, 100, 120, 255});
                
                // Line content
                draw_text(renderer, line, 50, content_y, text_color);
                
                content_y += line_height;
                current_line++;
                lines_drawn++;
            }
            
            // Scroll indicator
            if (total_lines > max_lines) {
                char scroll_info[64];
                snprintf(scroll_info, sizeof(scroll_info), "Lines %d-%d of %d", 
                        content_scroll + 1, 
                        content_scroll + lines_drawn, 
                        total_lines);
                draw_text(renderer, scroll_info, RENDER_WIDTH - 150, WINDOW_HEIGHT - 30, 
                         (SDL_Color){150, 150, 150, 255});
            }
        } else {
            draw_text(renderer, "Empty file or loading...", 50, 150, 
                     (SDL_Color){150, 150, 150, 255});
        }
    } else {
        // No file selected
        draw_text(renderer, "Select a file to view its contents", 
                 RENDER_WIDTH / 2 - 140, WINDOW_HEIGHT / 2, text_color);
        draw_text(renderer, "Use the file list on the right ->", 
                 RENDER_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 + 30, 
                 (SDL_Color){150, 150, 150, 255});
    }
    
    // Render all inputs through InputManager
    extern InputManager* g_input_manager;
    if (g_input_manager) {
        input_manager_render(g_input_manager, renderer, get_global_font(), TAB_RESEARCH);
    }
}

// Handle SDL events for search input
bool handle_research_tab_event(AppState* state, SDL_Event* event) {
    if (!state || !event) return false;
    
    // Handle search input events
    if (text_input_handle_event(&search_input, event)) {
        // Update search query when input changes
        if (!text_input_is_active(&search_input)) {
            const char* query = text_input_get_text(&search_input);
            strncpy(search_query, query, sizeof(search_query) - 1);
            search_query[sizeof(search_query) - 1] = '\0';
            // Rescan directory with new search query
            scan_research_directory(current_directory);
        }
        return true;
    }
    
    return false;
}

void handle_research_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    int panel_x = RENDER_WIDTH;
    
    // Scan button (y ≈ 90)
    SDL_Rect scan_btn = {panel_x + 10, 90, 80, 22};
    if (x >= scan_btn.x && x <= scan_btn.x + scan_btn.w &&
        y >= scan_btn.y && y <= scan_btn.y + scan_btn.h) {
        scan_research_directory(current_directory);
        return;
    }
    
    // Refresh button (y ≈ 90)
    SDL_Rect refresh_btn = {panel_x + 100, 90, 80, 22};
    if (x >= refresh_btn.x && x <= refresh_btn.x + refresh_btn.w &&
        y >= refresh_btn.y && y <= refresh_btn.y + refresh_btn.h) {
        scan_research_directory(current_directory);
        return;
    }
    
    // Up directory button (y ≈ 90)
    SDL_Rect up_btn = {panel_x + 190, 90, 80, 22};
    if (x >= up_btn.x && x <= up_btn.x + up_btn.w &&
        y >= up_btn.y && y <= up_btn.y + up_btn.h) {
        // Go up one directory
        char* last_slash = strrchr(current_directory, '/');
        if (last_slash && last_slash != current_directory) {
            *last_slash = '\0';
            scan_research_directory(current_directory);
        }
        return;
    }
    
    // Sort buttons (y ≈ 150)
    int btn_width = (CONTROL_PANEL_WIDTH - 30) / 4;
    for (int i = 0; i < 4; i++) {
        SDL_Rect sort_btn = {panel_x + 10 + i * (btn_width + 3), 150, btn_width, 20};
        if (x >= sort_btn.x && x <= sort_btn.x + sort_btn.w &&
            y >= sort_btn.y && y <= sort_btn.y + sort_btn.h) {
            if (sort_mode == (SortMode)i) {
                sort_ascending = !sort_ascending;
            } else {
                sort_mode = (SortMode)i;
                sort_ascending = true;
            }
            scan_research_directory(current_directory);
            return;
        }
    }
    
    // File list clicks
    int list_y = 196;
    int list_height = WINDOW_HEIGHT - list_y - 20;
    SDL_Rect list_rect = {panel_x + 10, list_y, CONTROL_PANEL_WIDTH - 20, list_height};
    
    if (x >= list_rect.x && x <= list_rect.x + list_rect.w &&
        y >= list_rect.y + 20 && y <= list_rect.y + list_rect.h) {
        int file_index = file_list_scroll + (y - list_rect.y - 20) / 14;
        if (file_index >= 0 && file_index < file_count) {
            selected_file = file_index;
            load_file_content(research_files[file_index].full_path);
        }
        return;
    }
}

void handle_research_tab_scroll(AppState* state, int scroll_y) {
    (void)state;
    
    if (selected_file >= 0) {
        // Scroll file content
        content_scroll -= scroll_y;
        if (content_scroll < 0) content_scroll = 0;
        
        // Count total lines
        int total_lines = 1;
        for (size_t i = 0; i < strlen(file_content); i++) {
            if (file_content[i] == '\n') total_lines++;
        }
        
        int max_lines = (WINDOW_HEIGHT - 150) / 14;
        int max_scroll = total_lines - max_lines;
        if (max_scroll < 0) max_scroll = 0;
        if (content_scroll > max_scroll) content_scroll = max_scroll;
    } else {
        // Scroll file list
        file_list_scroll -= scroll_y;
        if (file_list_scroll < 0) file_list_scroll = 0;
        
        int list_height = WINDOW_HEIGHT - 196 - 20;
        int max_visible = (list_height - 25) / 14;
        int max_scroll = file_count - max_visible;
        if (max_scroll < 0) max_scroll = 0;
        if (file_list_scroll > max_scroll) file_list_scroll = max_scroll;
    }
}

void handle_research_tab_text_input(AppState* state, const char* text) {
    if (!state || !text || !search_input_initialized) return;
    
    if (search_input.active) {
        size_t len = strlen(search_input.text);
        if (len < MAX_INPUT_LENGTH - 1) {
            strncat(search_input.text, text, MAX_INPUT_LENGTH - len - 1);
            search_input.cursor_pos = strlen(search_input.text);
            strncpy(search_query, search_input.text, sizeof(search_query) - 1);
            search_query[sizeof(search_query) - 1] = '\0';
        }
    }
}

void handle_research_tab_keydown(AppState* state, SDL_Keycode key) {
    if (!state || !search_input_initialized) return;
    
    if (search_input.active) {
        if (key == SDLK_BACKSPACE && search_input.cursor_pos > 0) {
            search_input.text[--search_input.cursor_pos] = '\0';
            strncpy(search_query, search_input.text, sizeof(search_query) - 1);
            search_query[sizeof(search_query) - 1] = '\0';
        } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            text_input_deactivate(&search_input);
            SDL_StopTextInput();
            // Trigger search
            scan_research_directory(current_directory);
        }
    }
}