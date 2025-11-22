// app/ui/tabs/tab_research.c - Research Data Browser Tab (Simplified)
#include "../../app_common.h"
#include <dirent.h>
#include <sys/stat.h>

#ifndef DT_REG
#define DT_REG 8
#endif

#define MAX_FILES 100
#define MAX_FILENAME_LENGTH 256
#define MAX_CONTENT_LENGTH 4096

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char full_path[512];
    long file_size;
} ResearchFile;

static ResearchFile research_files[MAX_FILES];
static int file_count = 0;
static int selected_file = -1;
static char file_content[MAX_CONTENT_LENGTH];

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
        
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
            ResearchFile* file = &research_files[file_count];
            snprintf(file->filename, MAX_FILENAME_LENGTH, "%s", entry->d_name);
            snprintf(file->full_path, sizeof(file->full_path), "%s", full_path);
            file->file_size = st.st_size;
            file_count++;
        }
    }
    closedir(dir);
    printf("Found %d research files\n", file_count);
}

static void load_file_content(const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        snprintf(file_content, sizeof(file_content), "Error: Could not open file");
        return;
    }
    
    size_t bytes_read = fread(file_content, 1, MAX_CONTENT_LENGTH - 1, f);
    file_content[bytes_read] = '\0';
    fclose(f);
}

void draw_research_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    int panel_x = RENDER_WIDTH;
    int panel_y = 60;
    int panel_width = CONTROL_PANEL_WIDTH;
    
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {40, 40, 50, 255};
    SDL_Color button_color = {60, 60, 80, 255};
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_Rect bg_rect = {panel_x, panel_y, panel_width, WINDOW_HEIGHT - 60};
    SDL_RenderFillRect(renderer, &bg_rect);
    
    int y = panel_y + 20;
    
    // Title
    draw_text(renderer, "Research", panel_x + 20, y, text_color);
    y += 40;
    
    // Scan button
    SDL_Rect scan_button = {panel_x + 20, y, panel_width - 40, 30};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &scan_button);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &scan_button);
    draw_text(renderer, "Scan Research Dir", scan_button.x + 60, scan_button.y + 8, text_color);
    y += 40;
    
    // File list
    char file_label[64];
    snprintf(file_label, sizeof(file_label), "Files (%d):", file_count);
    draw_text(renderer, file_label, panel_x + 20, y, text_color);
    y += 25;
    
    // File list box
    int list_height = WINDOW_HEIGHT - y - 20;
    SDL_Rect list_box = {panel_x + 20, y, panel_width - 40, list_height};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &list_box);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &list_box);
    
    // Draw files
    int file_y = list_box.y + 5;
    int max_visible = (list_height - 10) / 18;
    for (int i = 0; i < file_count && i < max_visible; i++) {
        SDL_Color file_color = (i == selected_file) ? 
            (SDL_Color){100, 200, 255, 255} : text_color;
        
        char display_name[64];
        snprintf(display_name, sizeof(display_name), "%.40s", research_files[i].filename);
        draw_text(renderer, display_name, list_box.x + 5, file_y, file_color);
        file_y += 18;
    }
    
    // Draw content viewer (left side)
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_Rect viz_rect = {0, 60, RENDER_WIDTH, WINDOW_HEIGHT - 60};
    SDL_RenderFillRect(renderer, &viz_rect);
    
    if (selected_file >= 0 && selected_file < file_count) {
        // Draw filename
        draw_text(renderer, research_files[selected_file].filename, 20, 80, text_color);
        
        // Draw file size
        char size_str[64];
        snprintf(size_str, sizeof(size_str), "Size: %ld bytes", 
                research_files[selected_file].file_size);
        draw_text(renderer, size_str, 20, 120, text_color);
        
        // Draw content (first few lines)
        if (strlen(file_content) > 0) {
            int content_y = 160;
            int line_height = 16;
            char line[256];
            const char* ptr = file_content;
            int lines_drawn = 0;
            int max_lines = (WINDOW_HEIGHT - 200) / line_height;
            
            while (*ptr && lines_drawn < max_lines) {
                int len = 0;
                while (*ptr && *ptr != '\n' && len < 200) {
                    line[len++] = *ptr++;
                }
                if (*ptr == '\n') ptr++;
                line[len] = '\0';
                
                draw_text(renderer, line, 20, content_y, text_color);
                content_y += line_height;
                lines_drawn++;
            }
        }
    } else {
        draw_text(renderer, "Select a research file to view", RENDER_WIDTH / 2 - 120, WINDOW_HEIGHT / 2, text_color);
    }
}

void handle_research_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    int panel_x = RENDER_WIDTH;
    
    // Scan button (y = 100)
    SDL_Rect scan_button = {panel_x + 20, 100, CONTROL_PANEL_WIDTH - 40, 30};
    if (x >= scan_button.x && x <= scan_button.x + scan_button.w &&
        y >= scan_button.y && y <= scan_button.y + scan_button.h) {
        scan_research_directory(state->research_directory);
    }
    
    // File list clicks
    int list_y = 165;
    int list_height = WINDOW_HEIGHT - list_y - 20;
    SDL_Rect list_box = {panel_x + 20, list_y, CONTROL_PANEL_WIDTH - 40, list_height};
    
    if (x >= list_box.x && x <= list_box.x + list_box.w &&
        y >= list_box.y && y <= list_box.y + list_box.h) {
        int file_index = (y - list_box.y - 5) / 18;
        if (file_index >= 0 && file_index < file_count) {
            selected_file = file_index;
            load_file_content(research_files[file_index].full_path);
            printf("Loaded file: %s\n", research_files[file_index].filename);
        }
    }
}