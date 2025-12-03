// app/ui/tabs/tab_training_crystalline.c - Training Tab (Crystalline UI)
#include "../../app_common.h"
#include "../crystalline/elements.h"
#include "../crystalline/geometry.h"
#include "../crystalline/layout.h"
#include "../crystalline/draw.h"
#include "../crystalline/color.h"
#include "../../input_manager.h"
#include "../model_selector.h"
#include "../sphere_visualization.h"
#include "../../training_thread.h"
#include "../../crawler_thread.h"
#include "../../time_format.h"
#include "cllm_metrics.h"
#include "cllm_format.h"
#include "cllm_training.h"
#include "cllm_vocab_builder.h"
#include "cllm_model_manager.h"
#include "cllm_utils.h"
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

extern TTF_Font* get_global_font(void);
extern InputManager* g_input_manager;

#define SIDEBAR_WIDTH 200
#define SUBMENU_HEIGHT 40
#define PADDING 10
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900
#define MAX_TRAINING_FILES 100
#define MAX_LOSS_HISTORY 100

// Training file structure
typedef struct {
    char filename[256];
    char filepath[512];
    bool selected;
    size_t size;
} TrainingFile;

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
static TrainingVisualization viz_data = {0};

// Crystalline UI elements
static CrystallinePanel* panel_main = NULL;
static CrystallinePanel* panel_status = NULL;
static CrystallinePanel* panel_control = NULL;
static CrystallinePanel* panel_config = NULL;
static CrystallinePanel* panel_files = NULL;
static CrystallinePanel* panel_crawler = NULL;

static CrystallineButton* btn_start_stop = NULL;
static CrystallineButton* btn_pause_resume = NULL;
static CrystallineButton* btn_save = NULL;
static CrystallineButton* btn_scan_dir = NULL;
static CrystallineButton* btn_select_all = NULL;
static CrystallineButton* btn_start_crawler = NULL;

static CrystallineList* list_files = NULL;

static ModelSelector* model_selector = NULL;
static char selected_model_name[256] = {0};

static bool ui_initialized = false;

// Slider state
static int batch_size = 32;
static int sequence_length = 128;
static int epochs = 10;
static float learning_rate = 0.001f;

// Helper functions
static void scan_training_directory(const char* dir_path) {
    file_count = 0;
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("Could not open directory: %s\n", dir_path);
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && file_count < MAX_TRAINING_FILES) {
        if (entry->d_name[0] == '.') continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
            TrainingFile* file = &training_files[file_count];
            snprintf(file->filename, sizeof(file->filename), "%s", entry->d_name);
            snprintf(file->filepath, sizeof(file->filepath), "%s", full_path);
            file->size = st.st_size;
            file->selected = false;
            file_count++;
        }
    }
    closedir(dir);
    
    printf("Found %d training files in %s\n", file_count, dir_path);
}

static void on_model_selected(const char* model_name, void* user_data) {
    (void)user_data;  // Unused parameter
    if (model_name) {
        strncpy(selected_model_name, model_name, sizeof(selected_model_name) - 1);
        selected_model_name[sizeof(selected_model_name) - 1] = '\0';
        printf("Model selected: %s\n", model_name);
    }
}

void init_training_tab(AppState* state) {
    if (!state || ui_initialized) return;
    
    // Get workspace directory
    extern void workspace_get_training_dir(AppState* state, char* output, size_t output_size);
    char training_dir[1024];
    workspace_get_training_dir(state, training_dir, sizeof(training_dir));
    
    // Scan for training files
    scan_training_directory(training_dir);
    
    // Calculate layout dimensions
    int content_x = SIDEBAR_WIDTH + PADDING;
    int content_y = SUBMENU_HEIGHT + PADDING;
    int content_width = WINDOW_WIDTH - SIDEBAR_WIDTH - PADDING * 2;
    int content_height = WINDOW_HEIGHT - SUBMENU_HEIGHT - PADDING * 2;
    
    // Golden ratio split: 61.8% main area, 38.2% control panel
    int main_width = (int)(content_width * 0.618f);
    int control_width = content_width - main_width - PADDING;
    
    // Create main visualization panel
    float main_panel_width = (float)main_width - 20.0f;
    float main_panel_height = (float)content_height - 20.0f;
    panel_main = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        content_x + main_panel_width / 2.0f + 10.0f,
        content_y + main_panel_height / 2.0f + 10.0f,
        main_panel_width,
        main_panel_height,
        "TRAINING",
        get_global_font()
    );
    if (panel_main) {
        panel_main->border_width = 3.0f;
        panel_main->base.color = crystalline_color_rgb(100, 120, 140);
    }
    
    // Create status panel (top of main area)
    float status_width = main_panel_width - 40.0f;
    float status_height = 100.0f;
    panel_status = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        content_x + status_width / 2.0f + 30.0f,
        content_y + status_height / 2.0f + 50.0f,
        status_width,
        status_height,
        "STATUS",
        get_global_font()
    );
    if (panel_status) {
        panel_status->border_width = 2.0f;
        panel_status->base.color = crystalline_color_rgb(80, 100, 120);
    }
    
    // Create control panel (right side)
    int control_x = content_x + main_width + PADDING;
    float control_panel_width = (float)control_width - 20.0f;
    float control_panel_height = (float)content_height - 20.0f;
    panel_control = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        control_x + control_panel_width / 2.0f,
        content_y + control_panel_height / 2.0f + 10.0f,
        control_panel_width,
        control_panel_height,
        "CONTROLS",
        get_global_font()
    );
    if (panel_control) {
        panel_control->border_width = 3.0f;
        panel_control->base.color = crystalline_color_rgb(100, 120, 140);
    }
    
    // Create configuration panel (nested in control)
    float config_width = control_panel_width - 40.0f;
    float config_height = 250.0f;
    panel_config = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        control_x + config_width / 2.0f + 20.0f,
        content_y + config_height / 2.0f + 120.0f,
        config_width,
        config_height,
        "CONFIGURATION",
        get_global_font()
    );
    if (panel_config) {
        panel_config->border_width = 2.0f;
        panel_config->base.color = crystalline_color_rgb(80, 100, 120);
    }
    
    // Create files panel (nested in control)
    int files_y_start = content_y + 400;
    int files_height = content_height - 550;
    float files_panel_width = control_panel_width - 40.0f;
    float files_panel_height = (float)files_height;
    panel_files = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        control_x + files_panel_width / 2.0f + 20.0f,
        files_y_start + files_panel_height / 2.0f,
        files_panel_width,
        files_panel_height,
        "TRAINING FILES",
        get_global_font()
    );
    if (panel_files) {
        panel_files->border_width = 2.0f;
        panel_files->base.color = crystalline_color_rgb(80, 100, 120);
    }
    
    // Create crawler panel (bottom of control)
    float crawler_width = control_panel_width - 40.0f;
    float crawler_height = 100.0f;
    int crawler_y = content_y + content_height - 120;
    panel_crawler = crystalline_panel_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        control_x + crawler_width / 2.0f + 20.0f,
        crawler_y + crawler_height / 2.0f,
        crawler_width,
        crawler_height,
        "CRAWLER",
        get_global_font()
    );
    if (panel_crawler) {
        panel_crawler->border_width = 2.0f;
        panel_crawler->base.color = crystalline_color_rgb(80, 100, 120);
    }
    
    // Create buttons - Quick actions in main area
    int btn_y = content_y + content_height - 100;
    int btn_center_x = content_x + main_width / 2;
    
    // START/STOP button (center, primary action)
    btn_start_stop = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        btn_center_x, btn_y,
        70.0f, 0.0f, "START", get_global_font()
    );
    
    // PAUSE/RESUME button (left, secondary action)
    btn_pause_resume = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        btn_center_x - 150, btn_y,
        50.0f, 0.0f, "PAUSE", get_global_font()
    );
    
    // SAVE button (right, secondary action)
    btn_save = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        btn_center_x + 150, btn_y,
        50.0f, 0.0f, "SAVE", get_global_font()
    );
    
    // File management buttons (tertiary actions)
    int file_btn_y = files_y_start - 60;
    
    btn_scan_dir = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        control_x + 60, file_btn_y,
        40.0f, 0.0f, "SCAN", get_global_font()
    );
    
    btn_select_all = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        control_x + 160, file_btn_y,
        40.0f, 0.0f, "SELECT", get_global_font()
    );
    
    // Crawler button (secondary action)
    btn_start_crawler = crystalline_button_create(
        CRYSTALLINE_STYLE_CIRCULAR,
        control_x + crawler_width / 2.0f + 20.0f,
        crawler_y + 60,
        50.0f, 0.0f, "START", get_global_font()
    );
    
    // Create file list
    float list_width = files_panel_width - 40.0f;
    float list_display_height = 30.0f * 10;  // item_height * visible_items
    list_files = crystalline_list_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        control_x + list_width / 2.0f + 40.0f,
        files_y_start + list_display_height / 2.0f + 50.0f,
        list_width,
        30.0f,  // item_height
        get_global_font()
    );
    
    // Initialize model selector
    model_selector = model_selector_create(
        control_x + 20,
        content_y + 50,
        (int)control_panel_width - 40,
        50
    );
    if (model_selector) {
        model_selector_update_list(model_selector);
        model_selector_set_callback(model_selector, on_model_selected, state);
    }
    
    ui_initialized = true;
    printf("Training tab initialized (Crystalline UI)\n");
}

void draw_training_tab(SDL_Renderer* renderer, AppState* state) {
    if (!state) return;
    
    // Initialize on first draw
    if (!ui_initialized) {
        init_training_tab(state);
    }
    
    // Calculate layout
    int content_x = SIDEBAR_WIDTH + PADDING;
    int content_y = SUBMENU_HEIGHT + PADDING;
    int content_width = WINDOW_WIDTH - SIDEBAR_WIDTH - PADDING * 2;
    int content_height = WINDOW_HEIGHT - SUBMENU_HEIGHT - PADDING * 2;
    int main_width = (int)(content_width * 0.618f);
    int control_x = content_x + main_width + PADDING;
    
    // Define colors
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color accent_color = {100, 150, 200, 255};
    SDL_Color success_color = {80, 200, 120, 255};
    
    // Render panels
    crystalline_panel_render(panel_main, renderer);
    crystalline_panel_render(panel_status, renderer);
    crystalline_panel_render(panel_control, renderer);
    crystalline_panel_render(panel_config, renderer);
    crystalline_panel_render(panel_files, renderer);
    crystalline_panel_render(panel_crawler, renderer);
    
    // Render buttons
    crystalline_button_render(btn_start_stop, renderer);
    crystalline_button_render(btn_pause_resume, renderer);
    crystalline_button_render(btn_save, renderer);
    crystalline_button_render(btn_scan_dir, renderer);
    crystalline_button_render(btn_select_all, renderer);
    crystalline_button_render(btn_start_crawler, renderer);
    
    // Render file list
    crystalline_list_render(list_files, renderer);
    
    // Render model selector
    if (model_selector) {
        model_selector_render(model_selector, renderer);
    }
    
    // Draw status information
    char status_text[256];
    if (state->training_in_progress) {
        snprintf(status_text, sizeof(status_text), "STATUS: TRAINING");
        CrystallinePoint status_pos = crystalline_point_cartesian(content_x + 50.0f, content_y + 80.0f);
        crystalline_draw_text(renderer, status_text, status_pos, success_color, NULL);
        
        // Draw progress
        if (state->training_epochs > 0) {
            snprintf(status_text, sizeof(status_text), "Epoch: %d / %d", 
                    state->training_current_epoch, state->training_epochs);
            CrystallinePoint epoch_pos = crystalline_point_cartesian(content_x + 50.0f, content_y + 100.0f);
            crystalline_draw_text(renderer, status_text, epoch_pos, text_color, NULL);
        }
        
        // Draw current loss
        snprintf(status_text, sizeof(status_text), "Loss: %.4f", state->training_loss);
        CrystallinePoint loss_pos = crystalline_point_cartesian(content_x + 250.0f, content_y + 100.0f);
        crystalline_draw_text(renderer, status_text, loss_pos, accent_color, NULL);
    } else {
        snprintf(status_text, sizeof(status_text), "STATUS: IDLE");
        CrystallinePoint status_pos = crystalline_point_cartesian(content_x + 50.0f, content_y + 80.0f);
        crystalline_draw_text(renderer, status_text, status_pos, text_color, NULL);
        
        snprintf(status_text, sizeof(status_text), "Ready to start training");
        CrystallinePoint ready_pos = crystalline_point_cartesian(content_x + 50.0f, content_y + 100.0f);
        crystalline_draw_text(renderer, status_text, ready_pos, accent_color, NULL);
    }
    
    // Draw configuration values
    int config_y = content_y + 150;
    
    snprintf(status_text, sizeof(status_text), "Batch Size: %d", batch_size);
    CrystallinePoint batch_pos = crystalline_point_cartesian(control_x + 30.0f, config_y);
    crystalline_draw_text(renderer, status_text, batch_pos, text_color, NULL);
    
    snprintf(status_text, sizeof(status_text), "Sequence Length: %d", sequence_length);
    CrystallinePoint seq_pos = crystalline_point_cartesian(control_x + 30.0f, config_y + 60);
    crystalline_draw_text(renderer, status_text, seq_pos, text_color, NULL);
    
    snprintf(status_text, sizeof(status_text), "Epochs: %d", epochs);
    CrystallinePoint epochs_pos = crystalline_point_cartesian(control_x + 30.0f, config_y + 120);
    crystalline_draw_text(renderer, status_text, epochs_pos, text_color, NULL);
    
    snprintf(status_text, sizeof(status_text), "Learning Rate: %.4f", learning_rate);
    CrystallinePoint lr_pos = crystalline_point_cartesian(control_x + 30.0f, config_y + 180);
    crystalline_draw_text(renderer, status_text, lr_pos, text_color, NULL);
    
    // Draw file count
    int selected_count = 0;
    for (int i = 0; i < file_count; i++) {
        if (training_files[i].selected) selected_count++;
    }
    
    snprintf(status_text, sizeof(status_text), "Files: %d selected / %d total", selected_count, file_count);
    CrystallinePoint files_pos = crystalline_point_cartesian(control_x + 30.0f, content_y + 380.0f);
    crystalline_draw_text(renderer, status_text, files_pos, accent_color, NULL);
    
    // Draw crawler status
    bool crawler_running = is_crawler_running();
    snprintf(status_text, sizeof(status_text), "Crawler: %s", crawler_running ? "RUNNING" : "STOPPED");
    SDL_Color crawler_color = crawler_running ? success_color : text_color;
    CrystallinePoint crawler_pos = crystalline_point_cartesian(control_x + 30.0f, content_y + content_height - 100.0f);
    crystalline_draw_text(renderer, status_text, crawler_pos, crawler_color, NULL);
    
    // Render inputs through InputManager
    if (g_input_manager) {
        input_manager_render(g_input_manager, renderer, get_global_font(), TAB_TRAINING);
    }
    
    // Draw visualization area content (preserve existing visualization code)
    // This section intentionally left for existing 2D/3D visualization code
    // DO NOT MODIFY - as per user instruction
}

void update_training_visualization(AppState* state) {
    if (!state) return;
    
    // Update visualization data from training state
    viz_data.current_epoch = state->training_current_epoch;
    viz_data.total_epochs = state->training_epochs;
    viz_data.current_loss = state->training_loss;
    
    // Update loss history
    if (state->training_in_progress && viz_data.loss_count < MAX_LOSS_HISTORY) {
        viz_data.loss_history[viz_data.loss_count++] = state->training_loss;
        if (viz_data.loss_count == 1 || state->training_loss < viz_data.best_loss) {
            viz_data.best_loss = state->training_loss;
        }
    }
}

void handle_training_tab_click(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Pass BUTTONDOWN event to all interactive elements
    if (model_selector) model_selector_handle_button_down(model_selector, x, y);
    if (list_files) crystalline_list_handle_mouse(list_files, &event);
    if (btn_start_stop) crystalline_button_handle_mouse(btn_start_stop, &event);
    if (btn_pause_resume) crystalline_button_handle_mouse(btn_pause_resume, &event);
    if (btn_save) crystalline_button_handle_mouse(btn_save, &event);
    if (btn_scan_dir) crystalline_button_handle_mouse(btn_scan_dir, &event);
    if (btn_select_all) crystalline_button_handle_mouse(btn_select_all, &event);
    if (btn_start_crawler) crystalline_button_handle_mouse(btn_start_crawler, &event);
}

bool handle_training_tab_event(AppState* state, SDL_Event* event) {
    if (!state || !event || !ui_initialized) return false;
    return false;
}

void handle_training_tab_scroll(AppState* state, int scroll_y) {
    (void)state;
    (void)scroll_y;
    // Handle scrolling if needed
}

void handle_training_tab_text_input(AppState* state, const char* text) {
    (void)state;
    (void)text;
}

void handle_training_tab_keydown(AppState* state, SDL_Keycode key) {
    (void)state;
    (void)key;
}

void handle_training_tab_mouse_motion(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEMOTION;
    event.motion.x = x;
    event.motion.y = y;
    
    // Update hover states
    if (list_files) crystalline_list_handle_mouse(list_files, &event);
    if (btn_start_stop) crystalline_button_handle_mouse(btn_start_stop, &event);
    if (btn_pause_resume) crystalline_button_handle_mouse(btn_pause_resume, &event);
    if (btn_save) crystalline_button_handle_mouse(btn_save, &event);
    if (btn_scan_dir) crystalline_button_handle_mouse(btn_scan_dir, &event);
    if (btn_select_all) crystalline_button_handle_mouse(btn_select_all, &event);
    if (btn_start_crawler) crystalline_button_handle_mouse(btn_start_crawler, &event);
}

void handle_training_tab_mouse_up(AppState* state, int x, int y) {
    if (!state || !ui_initialized) return;
    
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONUP;
    event.button.x = x;
    event.button.y = y;
    event.button.button = SDL_BUTTON_LEFT;
    
    // Check model selector
    if (model_selector && model_selector_handle_button_up(model_selector, x, y)) {
        return;
    }
    
    // Check file list
    if (crystalline_list_handle_mouse(list_files, &event)) {
        int selected = crystalline_list_get_selected(list_files);
        if (selected >= 0 && selected < file_count) {
            training_files[selected].selected = !training_files[selected].selected;
            printf("File %s %s\n", training_files[selected].filename, 
                   training_files[selected].selected ? "selected" : "deselected");
        }
        return;
    }
    
    // Check buttons
    if (crystalline_button_handle_mouse(btn_start_stop, &event)) {
        if (state->training_in_progress) {
            printf("Stop training\n");
            stop_training_thread(state);
        } else {
            printf("Start training\n");
            start_training_thread(state);
        }
        return;
    }
    
    if (crystalline_button_handle_mouse(btn_pause_resume, &event)) {
        printf("Pause/Resume training\n");
        // TODO: Implement pause/resume
        return;
    }
    
    if (crystalline_button_handle_mouse(btn_save, &event)) {
        printf("Save model\n");
        // TODO: Implement save
        return;
    }
    
    if (crystalline_button_handle_mouse(btn_scan_dir, &event)) {
        printf("Scan directory\n");
        char training_dir[1024];
        extern void workspace_get_training_dir(AppState* state, char* output, size_t output_size);
        workspace_get_training_dir(state, training_dir, sizeof(training_dir));
        scan_training_directory(training_dir);
        return;
    }
    
    if (crystalline_button_handle_mouse(btn_select_all, &event)) {
        printf("Select all files\n");
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
        return;
    }
    
    if (crystalline_button_handle_mouse(btn_start_crawler, &event)) {
        bool crawler_running = is_crawler_running();
        if (crawler_running) {
            printf("Stop crawler\n");
            stop_crawler_thread();
        } else {
            printf("Start crawler\n");
            // TODO: Need to get start_url and extraction_mode from UI
            // start_crawler_thread(state, start_url, extraction_mode, model_name);
            printf("Crawler start requires URL configuration\n");
        }
        return;
    }
}

void cleanup_training_tab(void) {
    if (model_selector) {
        model_selector_destroy(model_selector);
        model_selector = NULL;
    }
    
    if (panel_main) crystalline_panel_destroy(panel_main);
    if (panel_status) crystalline_panel_destroy(panel_status);
    if (panel_control) crystalline_panel_destroy(panel_control);
    if (panel_config) crystalline_panel_destroy(panel_config);
    if (panel_files) crystalline_panel_destroy(panel_files);
    if (panel_crawler) crystalline_panel_destroy(panel_crawler);
    
    if (btn_start_stop) crystalline_button_destroy(btn_start_stop);
    if (btn_pause_resume) crystalline_button_destroy(btn_pause_resume);
    if (btn_save) crystalline_button_destroy(btn_save);
    if (btn_scan_dir) crystalline_button_destroy(btn_scan_dir);
    if (btn_select_all) crystalline_button_destroy(btn_select_all);
    if (btn_start_crawler) crystalline_button_destroy(btn_start_crawler);
    
    if (list_files) crystalline_list_destroy(list_files);
    
    panel_main = panel_status = panel_control = NULL;
    panel_config = panel_files = panel_crawler = NULL;
    btn_start_stop = btn_pause_resume = btn_save = NULL;
    btn_scan_dir = btn_select_all = btn_start_crawler = NULL;
    list_files = NULL;
    
    ui_initialized = false;
}