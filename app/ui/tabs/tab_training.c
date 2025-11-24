/**
 * Training Tab - Redesigned with Dynamic Layout System
 * 
 * Features:
 * - Dynamic layout management (no hardcoded positions)
 * - Crawler integration
 * - Training visualization in display area
 * - Professional UI design
 * - Proper hit detection
 */

#include "../../app_common.h"
#include "../../text_input.h"
#include "../../training_thread.h"
#include "../../ui_layout.h"
#include "cllm_format.h"
#include "cllm_training.h"
#include "cllm_training_parallel.h"
#include "cllm_vocab_builder.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <math.h>

#define MAX_TRAINING_FILES 100
#define MAX_LOSS_HISTORY 100

// Training file structure
typedef struct {
    char filename[256];
    char filepath[512];
    bool selected;
    size_t size;
} TrainingFile;

// UI Button structure
typedef struct {
    SDL_Rect bounds;
    char label[64];
    bool enabled;
    bool visible;
} UIButton;

// Training visualization data
typedef struct {
    float loss_history[MAX_LOSS_HISTORY];
    int loss_count;
    int current_epoch;
    int total_epochs;
    float current_loss;
    float best_loss;
} TrainingVisualization;

// Static state
static TrainingFile training_files[MAX_TRAINING_FILES];
static int file_count = 0;
static int scroll_offset = 0;
static TrainingVisualization viz_data = {0};

// UI Elements with dynamic positioning
static UIButton btn_scan_dir;
static UIButton btn_select_all;
static UIButton btn_start_training;
static UIButton btn_start_crawler;
static UIButton btn_save;
static UIButton btn_load;

static SDL_Rect file_list_rect;
static SDL_Rect viz_area_rect;

// Text input fields
static TextInput learning_rate_input;
static TextInput epochs_input;
static TextInput batch_size_input;
static TextInput thread_count_input;
static TextInput crawler_url_input;
static bool inputs_initialized = false;

// Crawler state
static bool crawler_running = false;
static int pages_crawled = 0;
static int pages_in_queue = 0;

/**
 * Load crawl queue from file
 * Creates file with start URL if it doesn't exist
 */
static void load_crawl_queue(AppState* state) {
    if (!state) return;
    
    char queue_path[1024];
    snprintf(queue_path, sizeof(queue_path), "%s/links_to_crawl.txt", state->crawler_data_dir);
    
    // Create directory if it doesn't exist
    char mkdir_cmd[1100];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", state->crawler_data_dir);
    (void)system(mkdir_cmd);
    
    FILE* f = fopen(queue_path, "r");
    if (f) {
        // File exists - count lines
        state->crawler_queue_size = 0;
        char line[2048];
        while (fgets(line, sizeof(line), f)) {
            state->crawler_queue_size++;
        }
        fclose(f);
        printf("Loaded crawl queue: %d URLs\n", state->crawler_queue_size);
    } else {
        // File doesn't exist - create it with start URL
        f = fopen(queue_path, "w");
        if (f) {
            fprintf(f, "%s\n", state->crawler_start_url);
            fclose(f);
            state->crawler_queue_size = 1;
            printf("Created new crawl queue with start URL: %s\n", state->crawler_start_url);
        } else {
            printf("Error: Could not create crawl queue file: %s\n", queue_path);
        }
    }
}

/**
 * Add URL to crawl queue
 */
static void add_url_to_queue(AppState* state, const char* url) {
    if (!state || !url || url[0] == '\0') return;
    
    char queue_path[1024];
    snprintf(queue_path, sizeof(queue_path), "%s/links_to_crawl.txt", state->crawler_data_dir);
    
    FILE* f = fopen(queue_path, "a");
    if (f) {
        fprintf(f, "%s\n", url);
        fclose(f);
        state->crawler_queue_size++;
        printf("Added URL to queue: %s\n", url);
    }
}

/**
 * Scan training directory for files
 */
void scan_training_directory(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("Could not open directory: %s\n", dir_path);
        return;
    }
    
    file_count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL && file_count < MAX_TRAINING_FILES) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        const char* ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".txt") == 0) {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
            
            struct stat st;
            if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
                strncpy(training_files[file_count].filename, entry->d_name, 
                       sizeof(training_files[file_count].filename) - 1);
                training_files[file_count].filename[sizeof(training_files[file_count].filename) - 1] = '\0';
                strncpy(training_files[file_count].filepath, full_path,
                       sizeof(training_files[file_count].filepath) - 1);
                training_files[file_count].filepath[sizeof(training_files[file_count].filepath) - 1] = '\0';
                training_files[file_count].selected = false;
                training_files[file_count].size = st.st_size;
                
                file_count++;
            }
        }
    }
    
    closedir(dir);
    printf("Found %d training files in %s\n", file_count, dir_path);
}

/**
 * Initialize training tab
 */
void init_training_tab(AppState* state) {
    if (!state) return;
    
    // Scan default training directory
    char training_dir[1024];
    extern void workspace_get_training_dir(AppState* state, char* output, size_t output_size);
    workspace_get_training_dir(state, training_dir, sizeof(training_dir));
    scan_training_directory(training_dir);
    
    // Initialize visualization area (left side of screen)
    viz_area_rect.x = 10;
    viz_area_rect.y = 70;
    viz_area_rect.w = RENDER_WIDTH - 20;
    viz_area_rect.h = WINDOW_HEIGHT - 80;
    
    // Initialize text inputs if not done
    if (!inputs_initialized) {
        int panel_x = RENDER_WIDTH + 10;
        int input_width = CONTROL_PANEL_WIDTH - 20;
        
        text_input_init(&learning_rate_input, "Learning Rate:", panel_x, 200, input_width, 25);
        text_input_set_text(&learning_rate_input, "0.001");
        
        text_input_init(&epochs_input, "Epochs:", panel_x, 240, input_width, 25);
        text_input_set_text(&epochs_input, "10");
        
        text_input_init(&batch_size_input, "Batch Size:", panel_x, 280, input_width, 25);
        text_input_set_text(&batch_size_input, "32");
        
        text_input_init(&thread_count_input, "Threads (0=auto):", panel_x, 320, input_width, 25);
        text_input_set_text(&thread_count_input, "0");
        
        text_input_init(&crawler_url_input, "Crawler Start URL:", panel_x, 360, input_width, 25);
        text_input_set_text(&crawler_url_input, "https://en.wikipedia.org/wiki/Main_Page");
        
        inputs_initialized = true;
    }
    
    // Initialize visualization data
    viz_data.loss_count = 0;
    viz_data.current_epoch = 0;
    viz_data.total_epochs = 0;
    viz_data.current_loss = 0.0f;
    viz_data.best_loss = INFINITY;
}

/**
 * Draw training visualization in display area
 */
void draw_training_visualization(SDL_Renderer* renderer, AppState* state) {
    if (!state) return;
    
    SDL_Color bg_color = {20, 20, 25, 255};
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color grid_color = {40, 40, 50, 255};
    SDL_Color loss_color = {100, 200, 255, 255};
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &viz_area_rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
    SDL_RenderDrawRect(renderer, &viz_area_rect);
    
    int content_x = viz_area_rect.x + 20;
    int content_y = viz_area_rect.y + 20;
    int content_w = viz_area_rect.w - 40;
    int content_h = viz_area_rect.h - 40;
    
    // Title
    draw_text(renderer, "TRAINING VISUALIZATION", content_x, content_y, text_color);
    content_y += 30;
    
    if (state->training_in_progress || viz_data.loss_count > 0) {
        // Training metrics
        char metrics[256];
        snprintf(metrics, sizeof(metrics), "Epoch: %d / %d  |  Loss: %.4f  |  Best: %.4f",
                viz_data.current_epoch, viz_data.total_epochs, 
                viz_data.current_loss, viz_data.best_loss);
        draw_text(renderer, metrics, content_x, content_y, text_color);
        content_y += 25;
        
        // Progress bar
        SDL_Rect progress_bg = {content_x, content_y, content_w, 20};
        SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
        SDL_RenderFillRect(renderer, &progress_bg);
        SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
        SDL_RenderDrawRect(renderer, &progress_bg);
        
        if (viz_data.total_epochs > 0) {
            float progress = (float)viz_data.current_epoch / viz_data.total_epochs;
            SDL_Rect progress_fill = {progress_bg.x, progress_bg.y, 
                                     (int)(progress_bg.w * progress), progress_bg.h};
            SDL_SetRenderDrawColor(renderer, loss_color.r, loss_color.g, loss_color.b, 255);
            SDL_RenderFillRect(renderer, &progress_fill);
        }
        content_y += 35;
        
        // Loss graph
        if (viz_data.loss_count > 1) {
            int graph_h = content_h - (content_y - viz_area_rect.y - 20);
            SDL_Rect graph_rect = {content_x, content_y, content_w, graph_h};
            
            // Draw graph background
            SDL_SetRenderDrawColor(renderer, 30, 30, 35, 255);
            SDL_RenderFillRect(renderer, &graph_rect);
            SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
            SDL_RenderDrawRect(renderer, &graph_rect);
            
            // Draw grid lines
            for (int i = 1; i < 5; i++) {
                int grid_y = graph_rect.y + (graph_rect.h * i) / 5;
                SDL_SetRenderDrawColor(renderer, 40, 40, 45, 255);
                SDL_RenderDrawLine(renderer, graph_rect.x, grid_y, graph_rect.x + graph_rect.w, grid_y);
            }
            
            // Find min/max loss for scaling
            float min_loss = viz_data.loss_history[0];
            float max_loss = viz_data.loss_history[0];
            for (int i = 1; i < viz_data.loss_count; i++) {
                if (viz_data.loss_history[i] < min_loss) min_loss = viz_data.loss_history[i];
                if (viz_data.loss_history[i] > max_loss) max_loss = viz_data.loss_history[i];
            }
            
            float loss_range = max_loss - min_loss;
            if (loss_range < 0.001f) loss_range = 0.001f;
            
            // Draw loss curve
            SDL_SetRenderDrawColor(renderer, loss_color.r, loss_color.g, loss_color.b, 255);
            for (int i = 1; i < viz_data.loss_count; i++) {
                int x1 = graph_rect.x + ((i - 1) * graph_rect.w) / (MAX_LOSS_HISTORY - 1);
                int y1 = graph_rect.y + graph_rect.h - 
                          (int)(((viz_data.loss_history[i - 1] - min_loss) / loss_range) * graph_rect.h);
                int x2 = graph_rect.x + (i * graph_rect.w) / (MAX_LOSS_HISTORY - 1);
                int y2 = graph_rect.y + graph_rect.h - 
                          (int)(((viz_data.loss_history[i] - min_loss) / loss_range) * graph_rect.h);
                
                SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            }
            
            // Draw axis labels
            char min_label[32], max_label[32];
            snprintf(min_label, sizeof(min_label), "%.4f", min_loss);
            snprintf(max_label, sizeof(max_label), "%.4f", max_loss);
            draw_text(renderer, max_label, graph_rect.x - 60, graph_rect.y, text_color);
            draw_text(renderer, min_label, graph_rect.x - 60, graph_rect.y + graph_rect.h - 10, text_color);
        }
    } else {
        // No training data - show instructions
        draw_text(renderer, "No training in progress", content_x, content_y, text_color);
        content_y += 25;
        draw_text(renderer, "1. Select training files from the panel", content_x, content_y, text_color);
        content_y += 20;
        draw_text(renderer, "2. Configure parameters", content_x, content_y, text_color);
        content_y += 20;
        draw_text(renderer, "3. Click START TRAINING", content_x, content_y, text_color);
        content_y += 30;
        draw_text(renderer, "Or use START CRAWLER to automatically", content_x, content_y, text_color);
        content_y += 20;
        draw_text(renderer, "crawl web pages and train continuously", content_x, content_y, text_color);
    }
    
    // Crawler status (if running)
    if (crawler_running || state->crawler_running) {
        int crawler_y = viz_area_rect.y + viz_area_rect.h - 80;
        SDL_Rect crawler_status = {content_x, crawler_y, content_w, 70};
        SDL_SetRenderDrawColor(renderer, 40, 60, 40, 255);
        SDL_RenderFillRect(renderer, &crawler_status);
        SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
        SDL_RenderDrawRect(renderer, &crawler_status);
        
        char crawler_text[128];
        snprintf(crawler_text, sizeof(crawler_text), 
                "CRAWLER ACTIVE | Pages: %d | Queue: %d", 
                state->crawler_pages_crawled, state->crawler_queue_size);
        draw_text(renderer, crawler_text, crawler_status.x + 10, crawler_status.y + 10, 
                 (SDL_Color){100, 255, 100, 255});
        
        // Show current URL being crawled
        if (state->crawler_current_url[0] != '\0') {
            char url_display[100];
            snprintf(url_display, sizeof(url_display), "Current: %.80s", state->crawler_current_url);
            draw_text(renderer, url_display, crawler_status.x + 10, crawler_status.y + 28, text_color);
        } else {
            draw_text(renderer, "Initializing crawler...", 
                     crawler_status.x + 10, crawler_status.y + 28, text_color);
        }
        
        draw_text(renderer, "Continuously crawling and training...", 
                 crawler_status.x + 10, crawler_status.y + 46, text_color);
    }
}

/**
 * Draw control panel with dynamic layout
 */
void draw_training_tab(SDL_Renderer* renderer, AppState* state) {
    if (!state) return;
    
    // Draw visualization area first
    draw_training_visualization(renderer, state);
    
    // Setup control panel layout
    int panel_x = RENDER_WIDTH;
    int panel_y = 60;
    int panel_width = CONTROL_PANEL_WIDTH;
    int panel_height = WINDOW_HEIGHT - panel_y;
    
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {40, 40, 50, 255};
    SDL_Color button_color = {60, 60, 80, 255};
    SDL_Color active_color = {100, 150, 200, 255};
    
    // Draw panel background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_Rect panel_rect = {panel_x, panel_y, panel_width, panel_height};
    SDL_RenderFillRect(renderer, &panel_rect);
    
    // Initialize layout system
    LayoutContainer layout;
    layout_init(&layout, panel_rect, LAYOUT_VERTICAL, 10, 8);
    
    // === SECTION 1: STATUS ===
    SDL_Rect status_label = layout_add_label(&layout, "STATUS", 20);
    draw_text(renderer, "STATUS", status_label.x, status_label.y, text_color);
    
    const char* status = state->training_in_progress ? "Training..." : "Ready";
    SDL_Color status_color = state->training_in_progress ? 
        (SDL_Color){255, 200, 100, 255} : (SDL_Color){100, 255, 100, 255};
    
    char status_text[64];
    snprintf(status_text, sizeof(status_text), "* %s", status);
    SDL_Rect status_rect = layout_add_label(&layout, status_text, 18);
    draw_text(renderer, status_text, status_rect.x, status_rect.y, status_color);
    
    const char* model_status = state->cllm_model ? "Model: Loaded" : "Model: Not Loaded";
    SDL_Color model_color = state->cllm_model ? 
        (SDL_Color){100, 255, 100, 255} : (SDL_Color){255, 100, 100, 255};
    SDL_Rect model_rect = layout_add_label(&layout, model_status, 18);
    draw_text(renderer, model_status, model_rect.x, model_rect.y, model_color);
    
    layout_add_spacing(&layout, 10);
    
    // === SECTION 2: TRAINING DATA ===
    SDL_Rect data_label = layout_add_label(&layout, "TRAINING DATA", 20);
    draw_text(renderer, "TRAINING DATA", data_label.x, data_label.y, text_color);
    
    // Buttons row
    int button_width = (panel_width - 30) / 2;
    SDL_Rect btn_row = layout_add_element(&layout, 0, 25);
    
    btn_scan_dir.bounds = (SDL_Rect){btn_row.x, btn_row.y, button_width, 25};
    btn_scan_dir.enabled = true;
    btn_scan_dir.visible = true;
    strncpy(btn_scan_dir.label, "Scan Dir", sizeof(btn_scan_dir.label) - 1);
    
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &btn_scan_dir.bounds);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &btn_scan_dir.bounds);
    draw_text(renderer, btn_scan_dir.label, btn_scan_dir.bounds.x + 25, btn_scan_dir.bounds.y + 6, text_color);
    
    btn_select_all.bounds = (SDL_Rect){btn_row.x + button_width + 10, btn_row.y, button_width, 25};
    btn_select_all.enabled = true;
    btn_select_all.visible = true;
    strncpy(btn_select_all.label, "Select All", sizeof(btn_select_all.label) - 1);
    
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &btn_select_all.bounds);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &btn_select_all.bounds);
    draw_text(renderer, btn_select_all.label, btn_select_all.bounds.x + 15, btn_select_all.bounds.y + 6, text_color);
    
    // File count
    int selected_count = 0;
    for (int i = 0; i < file_count; i++) {
        if (training_files[i].selected) selected_count++;
    }
    
    char file_info[64];
    snprintf(file_info, sizeof(file_info), "Files: %d (%d selected)", file_count, selected_count);
    SDL_Rect file_info_rect = layout_add_label(&layout, file_info, 18);
    draw_text(renderer, file_info, file_info_rect.x, file_info_rect.y, text_color);
    
    // File list
    file_list_rect = layout_add_element(&layout, 0, 100);
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &file_list_rect);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &file_list_rect);
    
    // Draw files
    int file_y = file_list_rect.y + 5;
    int max_visible = (file_list_rect.h - 10) / 16;
    for (int i = scroll_offset; i < file_count && i < scroll_offset + max_visible; i++) {
        SDL_Color file_color = training_files[i].selected ? 
            (SDL_Color){100, 200, 255, 255} : text_color;
        
        // Checkbox
        SDL_Rect checkbox = {file_list_rect.x + 5, file_y, 10, 10};
        SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
        SDL_RenderFillRect(renderer, &checkbox);
        SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
        SDL_RenderDrawRect(renderer, &checkbox);
        
        if (training_files[i].selected) {
            SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
            SDL_Rect check = {checkbox.x + 2, checkbox.y + 2, 6, 6};
            SDL_RenderFillRect(renderer, &check);
        }
        
        // Filename (truncated)
        char display_name[40];
        snprintf(display_name, sizeof(display_name), "%.32s", training_files[i].filename);
        draw_text(renderer, display_name, file_list_rect.x + 20, file_y, file_color);
        
        file_y += 16;
    }
    
    layout_add_spacing(&layout, 10);
    
    // === SECTION 3: PARAMETERS ===
    SDL_Rect params_label = layout_add_label(&layout, "PARAMETERS", 20);
    draw_text(renderer, "PARAMETERS", params_label.x, params_label.y, text_color);
    
    // Show current crawler URL
    char url_display[100];
    const char* current_url = text_input_get_text(&crawler_url_input);
    snprintf(url_display, sizeof(url_display), "Crawler URL: %.70s", 
             current_url && current_url[0] ? current_url : "(not set)");
    SDL_Rect url_info = layout_add_label(&layout, url_display, 16);
    draw_text(renderer, url_display, url_info.x, url_info.y, 
             (SDL_Color){150, 200, 255, 255});
    
    layout_add_spacing(&layout, 5);
    
    // Render text inputs
    text_input_render(&learning_rate_input, renderer, get_global_font());
    text_input_render(&epochs_input, renderer, get_global_font());
    text_input_render(&batch_size_input, renderer, get_global_font());
    text_input_render(&thread_count_input, renderer, get_global_font());
    text_input_render(&crawler_url_input, renderer, get_global_font());
    
    // Skip past input fields (now 5 inputs instead of 4)
    layout_add_spacing(&layout, 200);
    
    // === SECTION 4: ACTIONS ===
    SDL_Rect actions_label = layout_add_label(&layout, "ACTIONS", 20);
    draw_text(renderer, "ACTIONS", actions_label.x, actions_label.y, text_color);
    
    // Start Training button
    btn_start_training.bounds = layout_add_button(&layout, NULL, 0, 35);
    btn_start_training.enabled = true;
    btn_start_training.visible = true;
    
    SDL_Color train_color = state->training_in_progress ? 
        (SDL_Color){200, 80, 80, 255} : (SDL_Color){80, 100, 120, 255};
    SDL_SetRenderDrawColor(renderer, train_color.r, train_color.g, train_color.b, 255);
    SDL_RenderFillRect(renderer, &btn_start_training.bounds);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &btn_start_training.bounds);
    
    const char* train_text = state->training_in_progress ? "STOP TRAINING" : "START TRAINING";
    int text_offset = state->training_in_progress ? 45 : 50;
    draw_text(renderer, train_text, btn_start_training.bounds.x + text_offset, 
             btn_start_training.bounds.y + 11, (SDL_Color){255, 255, 255, 255});
    
    // Start Crawler button (NEW!)
    btn_start_crawler.bounds = layout_add_button(&layout, NULL, 0, 35);
    btn_start_crawler.enabled = true;
    btn_start_crawler.visible = true;
    
    SDL_Color crawler_color = crawler_running ? 
        (SDL_Color){200, 80, 80, 255} : (SDL_Color){60, 120, 60, 255};
    SDL_SetRenderDrawColor(renderer, crawler_color.r, crawler_color.g, crawler_color.b, 255);
    SDL_RenderFillRect(renderer, &btn_start_crawler.bounds);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &btn_start_crawler.bounds);
    
    const char* crawler_text = crawler_running ? "STOP CRAWLER" : "START CRAWLER";
    int crawler_offset = crawler_running ? 50 : 55;
    draw_text(renderer, crawler_text, btn_start_crawler.bounds.x + crawler_offset, 
             btn_start_crawler.bounds.y + 11, (SDL_Color){255, 255, 255, 255});
    
    // Save/Load buttons
    SDL_Rect save_load_row = layout_add_element(&layout, 0, 25);
    
    btn_save.bounds = (SDL_Rect){save_load_row.x, save_load_row.y, button_width, 25};
    btn_save.enabled = true;
    btn_save.visible = true;
    
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &btn_save.bounds);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &btn_save.bounds);
    draw_text(renderer, "Save", btn_save.bounds.x + 35, btn_save.bounds.y + 6, text_color);
    
    btn_load.bounds = (SDL_Rect){save_load_row.x + button_width + 10, save_load_row.y, button_width, 25};
    btn_load.enabled = true;
    btn_load.visible = true;
    
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &btn_load.bounds);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &btn_load.bounds);
    draw_text(renderer, "Load", btn_load.bounds.x + 35, btn_load.bounds.y + 6, text_color);
}

/**
 * Update visualization data
 */
void update_training_visualization(AppState* state) {
    if (!state) return;
    
    viz_data.current_epoch = state->training_current_epoch;
    viz_data.total_epochs = state->training_epochs;
    viz_data.current_loss = state->training_loss;
    
    // Add to loss history
    if (viz_data.current_loss > 0.0f && viz_data.current_loss < 1000.0f) {
        if (viz_data.loss_count < MAX_LOSS_HISTORY) {
            viz_data.loss_history[viz_data.loss_count++] = viz_data.current_loss;
        } else {
            // Shift array and add new value
            for (int i = 0; i < MAX_LOSS_HISTORY - 1; i++) {
                viz_data.loss_history[i] = viz_data.loss_history[i + 1];
            }
            viz_data.loss_history[MAX_LOSS_HISTORY - 1] = viz_data.current_loss;
        }
        
        if (viz_data.current_loss < viz_data.best_loss) {
            viz_data.best_loss = viz_data.current_loss;
        }
    }
}

/**
 * Handle text input events
 */
bool handle_training_tab_event(AppState* state, SDL_Event* event) {
    if (!state || !event) return false;
    
    // Handle text input events
    if (text_input_handle_event(&learning_rate_input, event)) {
        if (!text_input_is_active(&learning_rate_input)) {
            state->training_learning_rate = (float)text_input_get_number(&learning_rate_input);
        }
        return true;
    }
    
    if (text_input_handle_event(&epochs_input, event)) {
        if (!text_input_is_active(&epochs_input)) {
            state->training_epochs = (int)text_input_get_number(&epochs_input);
        }
        return true;
    }
    
    if (text_input_handle_event(&batch_size_input, event)) {
        return true;
    }
    
    if (text_input_handle_event(&thread_count_input, event)) {
        if (!text_input_is_active(&thread_count_input)) {
            state->training_thread_count = (int)text_input_get_number(&thread_count_input);
        }
        return true;
    }
    
    if (text_input_handle_event(&crawler_url_input, event)) {
        if (!text_input_is_active(&crawler_url_input)) {
            // Update the crawler start URL in state
            strncpy(state->crawler_start_url, text_input_get_text(&crawler_url_input), 
                    sizeof(state->crawler_start_url) - 1);
            state->crawler_start_url[sizeof(state->crawler_start_url) - 1] = '\0';
        }
        return true;
    }
    
    return false;
}

/**
 * Handle mouse clicks with dynamic hit detection
 */
void handle_training_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    // Update visualization
    update_training_visualization(state);
    
    // Check text input clicks
    if (inputs_initialized) {
        if (rect_contains_point(learning_rate_input.bounds, x, y)) {
            text_input_activate(&learning_rate_input);
            text_input_deactivate(&epochs_input);
            text_input_deactivate(&batch_size_input);
            text_input_deactivate(&thread_count_input);
            SDL_StartTextInput();
            return;
        }
        if (rect_contains_point(epochs_input.bounds, x, y)) {
            text_input_activate(&epochs_input);
            text_input_deactivate(&learning_rate_input);
            text_input_deactivate(&batch_size_input);
            text_input_deactivate(&thread_count_input);
            SDL_StartTextInput();
            return;
        }
        if (rect_contains_point(batch_size_input.bounds, x, y)) {
            text_input_activate(&batch_size_input);
            text_input_deactivate(&learning_rate_input);
            text_input_deactivate(&epochs_input);
            text_input_deactivate(&thread_count_input);
            SDL_StartTextInput();
            return;
        }
        if (rect_contains_point(thread_count_input.bounds, x, y)) {
            text_input_activate(&thread_count_input);
            text_input_deactivate(&learning_rate_input);
            text_input_deactivate(&epochs_input);
            text_input_deactivate(&batch_size_input);
            text_input_deactivate(&crawler_url_input);
            SDL_StartTextInput();
            return;
        }
        if (rect_contains_point(crawler_url_input.bounds, x, y)) {
            text_input_activate(&crawler_url_input);
            text_input_deactivate(&learning_rate_input);
            text_input_deactivate(&epochs_input);
            text_input_deactivate(&batch_size_input);
            text_input_deactivate(&thread_count_input);
            SDL_StartTextInput();
            return;
        }
    }
    
    // Check button clicks using dynamic bounds
    if (rect_contains_point(btn_scan_dir.bounds, x, y)) {
        printf("Scan directory button clicked\n");
        char training_dir[1024];
        extern void workspace_get_training_dir(AppState* state, char* output, size_t output_size);
        workspace_get_training_dir(state, training_dir, sizeof(training_dir));
        scan_training_directory(training_dir);
        return;
    }
    
    if (rect_contains_point(btn_select_all.bounds, x, y)) {
        bool all_selected = true;
        for (int i = 0; i < file_count; i++) {
            if (!training_files[i].selected) {
                all_selected = false;
                break;
            }
        }
        for (int i = 0; i < file_count; i++) {
            training_files[i].selected = !all_selected;
        }
        printf("Select all toggled\n");
        return;
    }
    
    // File list clicks
    if (rect_contains_point(file_list_rect, x, y)) {
        int file_index = scroll_offset + (y - file_list_rect.y - 5) / 16;
        if (file_index >= 0 && file_index < file_count) {
            training_files[file_index].selected = !training_files[file_index].selected;
            printf("Toggled file: %s\n", training_files[file_index].filename);
        }
        return;
    }
    
    // Start Training button
    if (rect_contains_point(btn_start_training.bounds, x, y)) {
        if (state->training_in_progress) {
            stop_training_thread(state);
            printf("Stopping training...\n");
        } else {
            // Start training (existing logic)
            int selected_count = 0;
            for (int i = 0; i < file_count; i++) {
                if (training_files[i].selected) selected_count++;
            }
            
            if (selected_count == 0) {
                printf("No files selected for training\n");
                return;
            }
            
            // Create model if needed
            if (!state->cllm_model) {
                printf("Creating new model...\n");
                system("mkdir -p models");
                extern CLLMModel* app_create_cllm_model_default(void);
                state->cllm_model = app_create_cllm_model_default();
                if (!state->cllm_model) {
                    printf("ERROR: Failed to create model\n");
                    return;
                }
                printf("✓ Model created\n");
            }
            
            // Initialize training
            if (!state->cllm_training) {
                CLLMTrainingConfig config = {
                    .num_epochs = state->training_epochs,
                    .batch_size = 4,
                    .sequence_length = 32,
                    .learning_rate = state->training_learning_rate,
                    .weight_decay = 0.01f,
                    .gradient_clip = 1.0f,
                    .warmup_steps = 100,
                    .save_every = 5,
                    .eval_interval = 100,
                    .max_steps = 10000
                };
                strcpy(config.optimizer, "adam");
                
                // Build vocabulary
                printf("Building vocabulary...\n");
                for (int i = 0; i < file_count; i++) {
                    if (training_files[i].selected) {
                        cllm_build_vocabulary_from_file(state->cllm_model, training_files[i].filepath);
                        break;
                    }
                }
                
                state->cllm_training = cllm_training_init(state->cllm_model, &config);
                
                // Load training data
                if (state->cllm_training) {
                    printf("Loading training data...\n");
                    for (int i = 0; i < file_count; i++) {
                        if (training_files[i].selected) {
                            cllm_load_training_data(state->cllm_training, training_files[i].filepath);
                        }
                    }
                }
            }
            
            // Start training thread
            printf("Starting training...\n");
            start_training_thread(state);
        }
        return;
    }
    
    // Start Crawler button (NEW!)
    if (rect_contains_point(btn_start_crawler.bounds, x, y)) {
        if (crawler_running || state->crawler_running) {
            printf("Stopping crawler...\n");
            crawler_running = false;
            state->crawler_running = false;
            state->crawler_current_url[0] = '\0';
            // TODO: Stop crawler thread
        } else {
            // Get URL from input field
            const char* start_url = text_input_get_text(&crawler_url_input);
            printf("DEBUG: Retrieved URL from input: '%s'\n", start_url ? start_url : "(NULL)");
            if (start_url && start_url[0] != '\0') {
                printf("Starting crawler from URL: %s\n", start_url);
                
                // Update state
                strncpy(state->crawler_start_url, start_url, sizeof(state->crawler_start_url) - 1);
                state->crawler_start_url[sizeof(state->crawler_start_url) - 1] = '\0';
                state->crawler_running = true;
                state->crawler_pages_crawled = 0;
                strncpy(state->crawler_current_url, start_url, sizeof(state->crawler_current_url) - 1);
                
                crawler_running = true;
                pages_crawled = 0;
                
                // Load or create crawl queue
                load_crawl_queue(state);
                pages_in_queue = state->crawler_queue_size;
                
                // TODO: Start crawler thread with URL
                printf("Crawler will start from: %s\n", start_url);
                printf("Data directory: %s\n", state->crawler_data_dir);
                printf("Queue file: %s/links_to_crawl.txt\n", state->crawler_data_dir);
                printf("Crawler functionality will be integrated with tools/cllm_crawler\n");
            } else {
                printf("Error: Please enter a valid URL to start crawling\n");
            }
        }
        return;
    }
    
    // Save button
    if (rect_contains_point(btn_save.bounds, x, y)) {
        printf("Save checkpoint\n");
        if (state->cllm_model) {
            system("mkdir -p checkpoints");
            char checkpoint_path[512];
            snprintf(checkpoint_path, sizeof(checkpoint_path), 
                    "checkpoints/checkpoint_epoch_%d.cllm", state->training_current_epoch);
            if (cllm_write_model(state->cllm_model, checkpoint_path) == 0) {
                printf("✓ Checkpoint saved: %s\n", checkpoint_path);
            }
        }
        return;
    }
    
    // Load button
    if (rect_contains_point(btn_load.bounds, x, y)) {
        printf("Load checkpoint (not yet implemented)\n");
        return;
    }
}

/**
 * Handle keyboard input
 */
void handle_training_tab_keydown(AppState* state, int key) {
    if (!state) return;
    
    if (key == SDLK_ESCAPE) {
        text_input_deactivate(&learning_rate_input);
        text_input_deactivate(&epochs_input);
        text_input_deactivate(&batch_size_input);
        text_input_deactivate(&thread_count_input);
        SDL_StopTextInput();
    } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        if (text_input_is_active(&learning_rate_input)) {
            text_input_deactivate(&learning_rate_input);
            state->training_learning_rate = (float)text_input_get_number(&learning_rate_input);
            SDL_StopTextInput();
        } else if (text_input_is_active(&epochs_input)) {
            text_input_deactivate(&epochs_input);
            state->training_epochs = (int)text_input_get_number(&epochs_input);
            SDL_StopTextInput();
        } else if (text_input_is_active(&batch_size_input)) {
            text_input_deactivate(&batch_size_input);
            SDL_StopTextInput();
        } else if (text_input_is_active(&thread_count_input)) {
            text_input_deactivate(&thread_count_input);
            state->training_thread_count = (int)text_input_get_number(&thread_count_input);
            SDL_StopTextInput();
        }
    }
}

/**
 * Handle text input
 */
void handle_training_tab_text_input(AppState* state, const char* text) {
    (void)state;
    (void)text;
}

/**
 * Cleanup
 */
void cleanup_training_tab(void) {
    // Cleanup if needed
}