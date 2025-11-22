// app/ui/tabs/tab_training.c - COMPLETE Training Tab Implementation
#include "../../app_common.h"
#include "../../cllm_integration.h"
#include "../../text_input.h"
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>

#ifndef DT_REG
#define DT_REG 8
#endif

#define MAX_FILES 100
#define MAX_FILENAME_LENGTH 256

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char full_path[512];
    long file_size;
    bool selected;
} TrainingFile;

static TrainingFile training_files[MAX_FILES];
static int file_count = 0;
static int scroll_offset = 0;

// Training thread state
static pthread_t training_thread;
static bool training_thread_running = false;

// Training history for visualization
#define MAX_HISTORY 1000
static float loss_history[MAX_HISTORY];
static int history_count = 0;

// Text input fields for training parameters
static TextInput learning_rate_input;
static TextInput epochs_input;
static TextInput batch_size_input;
static bool inputs_initialized = false;

static void scan_training_directory(const char* dir_path) {
    file_count = 0;
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("Could not open directory: %s\n", dir_path);
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        if (entry->d_name[0] == '.') continue;
        
        // Only show text files
        const char* ext = strrchr(entry->d_name, '.');
        if (!ext || (strcmp(ext, ".txt") != 0 && strcmp(ext, ".json") != 0 && 
                     strcmp(ext, ".csv") != 0)) continue;
        
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
            TrainingFile* file = &training_files[file_count];
            snprintf(file->filename, MAX_FILENAME_LENGTH, "%s", entry->d_name);
            snprintf(file->full_path, sizeof(file->full_path), "%s", full_path);
            file->file_size = st.st_size;
            file->selected = false;
            file_count++;
        }
    }
    closedir(dir);
    printf("Found %d training files\n", file_count);
}

// Training thread function
static void* training_thread_func(void* arg) {
    AppState* state = (AppState*)arg;
    
    printf("Training thread started\n");
    
    // Train for specified epochs
    for (int epoch = 0; epoch < state->training_epochs && state->training_in_progress; epoch++) {
        state->training_current_epoch = epoch + 1;
        
        // Train one epoch using our integration function
        float loss = app_train_epoch(state);
        state->training_loss = loss;
        
        // Add to history
        if (history_count < MAX_HISTORY) {
            loss_history[history_count++] = loss;
        } else {
            // Shift history
            for (int i = 0; i < MAX_HISTORY - 1; i++) {
                loss_history[i] = loss_history[i + 1];
            }
            loss_history[MAX_HISTORY - 1] = loss;
        }
        
        printf("Epoch %d/%d - Loss: %.4f\n", epoch + 1, state->training_epochs, loss);
        
        // Save checkpoint every N epochs
        if ((epoch + 1) % 5 == 0) {
            char checkpoint_path[512];
            snprintf(checkpoint_path, sizeof(checkpoint_path), "checkpoints/checkpoint_epoch_%d", epoch + 1);
            cllm_save_checkpoint(state->cllm_training, checkpoint_path);
            printf("Saved checkpoint: %s\n", checkpoint_path);
        }
    }
    
    state->training_in_progress = false;
    training_thread_running = false;
    printf("Training complete!\n");
    
    return NULL;
}

void draw_training_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    // Initialize text inputs once
    if (!inputs_initialized) {
        text_input_init(&learning_rate_input, "Learning Rate:", RENDER_WIDTH + 10, 150, 200, 30);
        text_input_set_numeric(&learning_rate_input, 0.0001, 1.0);
        char lr_buf[32];
        snprintf(lr_buf, sizeof(lr_buf), "%.4f", state->training_learning_rate);
        text_input_set_text(&learning_rate_input, lr_buf);
        
        text_input_init(&epochs_input, "Epochs:", RENDER_WIDTH + 10, 200, 200, 30);
        text_input_set_numeric(&epochs_input, 1, 10000);
        char ep_buf[32];
        snprintf(ep_buf, sizeof(ep_buf), "%d", state->training_epochs);
        text_input_set_text(&epochs_input, ep_buf);
        
        text_input_init(&batch_size_input, "Batch Size:", RENDER_WIDTH + 10, 250, 200, 30);
        text_input_set_numeric(&batch_size_input, 1, 1024);
        text_input_set_text(&batch_size_input, "32");
        
        inputs_initialized = true;
    }
    
    int panel_x = RENDER_WIDTH;
    int panel_y = 60;
    int panel_width = CONTROL_PANEL_WIDTH;
    
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {40, 40, 50, 255};
    SDL_Color button_color = {60, 60, 80, 255};
    SDL_Color button_hover = {80, 100, 120, 255};
    SDL_Color active_color = {100, 150, 200, 255};
    
    // Draw panel background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_Rect panel_rect = {panel_x, panel_y, panel_width, WINDOW_HEIGHT - panel_y};
    SDL_RenderFillRect(renderer, &panel_rect);
    
    int y = panel_y + 10;
    
    // === SECTION 1: STATUS ===
    draw_text(renderer, "TRAINING", panel_x + 10, y, text_color);
    y += 20;
    
    // Status indicator
    const char* status = state->training_in_progress ? "Training..." : "Ready";
    SDL_Color status_color = state->training_in_progress ? 
        (SDL_Color){255, 200, 100, 255} : (SDL_Color){100, 255, 100, 255};
    
    char status_text[64];
    snprintf(status_text, sizeof(status_text), "Status: %s", status);
    draw_text(renderer, status_text, panel_x + 10, y, status_color);
    y += 18;
    
    // Model status
    if (state->cllm_model) {
        draw_text(renderer, "Model: Loaded", panel_x + 10, y, (SDL_Color){100, 255, 100, 255});
    } else {
        draw_text(renderer, "Model: Not Loaded", panel_x + 10, y, (SDL_Color){255, 100, 100, 255});
    }
    y += 25;
    
    // === SECTION 1.5: TRAINING PARAMETERS ===
    draw_text(renderer, "PARAMETERS", panel_x + 10, y, text_color);
    y += 20;
    
    // Render text input fields
    text_input_render(&learning_rate_input, renderer, get_global_font());
    text_input_render(&epochs_input, renderer, get_global_font());
    text_input_render(&batch_size_input, renderer, get_global_font());
    
    y = 300; // Skip past the input fields
    
    // === SECTION 2: DATA FILES ===
    draw_text(renderer, "TRAINING DATA", panel_x + 10, y, text_color);
    y += 20;
    
    // Scan Directory Button
    SDL_Rect scan_btn = {panel_x + 10, y, (panel_width - 30) / 2, 25};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &scan_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &scan_btn);
    draw_text(renderer, "Scan Dir", scan_btn.x + 30, scan_btn.y + 6, text_color);
    
    // Select All Button
    SDL_Rect select_all_btn = {panel_x + 10 + (panel_width - 30) / 2 + 10, y, (panel_width - 30) / 2, 25};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &select_all_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &select_all_btn);
    draw_text(renderer, "Select All", select_all_btn.x + 20, select_all_btn.y + 6, text_color);
    y += 32;
    
    // File count
    int selected_count = 0;
    for (int i = 0; i < file_count; i++) {
        if (training_files[i].selected) selected_count++;
    }
    
    char file_info[64];
    snprintf(file_info, sizeof(file_info), "Files: %d (%d selected)", file_count, selected_count);
    draw_text(renderer, file_info, panel_x + 10, y, text_color);
    y += 20;
    
    // File list
    int list_height = 120;
    SDL_Rect list_rect = {panel_x + 10, y, panel_width - 20, list_height};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &list_rect);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &list_rect);
    
    // Draw files
    int file_y = list_rect.y + 5;
    int max_visible = (list_height - 10) / 16;
    for (int i = scroll_offset; i < file_count && i < scroll_offset + max_visible; i++) {
        SDL_Color file_color = training_files[i].selected ? 
            (SDL_Color){100, 200, 255, 255} : text_color;
        
        // Checkbox
        SDL_Rect checkbox = {list_rect.x + 5, file_y, 10, 10};
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
        draw_text(renderer, display_name, list_rect.x + 20, file_y, file_color);
        
        file_y += 16;
    }
    y += list_height + 10;
    
    // === SECTION 3: PARAMETERS ===
    draw_text(renderer, "PARAMETERS", panel_x + 10, y, text_color);
    y += 20;
    
    // Epochs
    char epochs_label[64];
    snprintf(epochs_label, sizeof(epochs_label), "Epochs: %d", state->training_epochs);
    draw_text(renderer, epochs_label, panel_x + 10, y, text_color);
    y += 16;
    
    SDL_Rect epochs_slider = {panel_x + 10, y, panel_width - 20, 8};
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderFillRect(renderer, &epochs_slider);
    
    int epochs_pos = (int)((state->training_epochs / 100.0f) * epochs_slider.w);
    SDL_Rect epochs_handle = {epochs_slider.x + epochs_pos - 4, epochs_slider.y - 4, 8, 16};
    SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
    SDL_RenderFillRect(renderer, &epochs_handle);
    y += 18;
    
    // Learning Rate
    char lr_label[64];
    snprintf(lr_label, sizeof(lr_label), "Learning Rate: %.4f", state->training_learning_rate);
    draw_text(renderer, lr_label, panel_x + 10, y, text_color);
    y += 16;
    
    SDL_Rect lr_slider = {panel_x + 10, y, panel_width - 20, 8};
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderFillRect(renderer, &lr_slider);
    
    float lr_ratio = (state->training_learning_rate - 0.0001f) / (0.01f - 0.0001f);
    int lr_pos = (int)(lr_ratio * lr_slider.w);
    SDL_Rect lr_handle = {lr_slider.x + lr_pos - 4, lr_slider.y - 4, 8, 16};
    SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
    SDL_RenderFillRect(renderer, &lr_handle);
    y += 25;
    
    // === SECTION 4: TRAINING CONTROLS ===
    // Start/Stop Button
    SDL_Rect train_btn = {panel_x + 10, y, panel_width - 20, 35};
    SDL_Color train_color = state->training_in_progress ? 
        (SDL_Color){200, 80, 80, 255} : button_hover;
    SDL_SetRenderDrawColor(renderer, train_color.r, train_color.g, train_color.b, 255);
    SDL_RenderFillRect(renderer, &train_btn);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &train_btn);
    
    const char* train_text = state->training_in_progress ? "STOP TRAINING" : "START TRAINING";
    draw_text(renderer, train_text, train_btn.x + 60, train_btn.y + 11, 
             (SDL_Color){255, 255, 255, 255});
    y += 42;
    
    // Save/Load Checkpoint Buttons
    SDL_Rect save_btn = {panel_x + 10, y, (panel_width - 30) / 2, 25};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &save_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &save_btn);
    draw_text(renderer, "Save", save_btn.x + 35, save_btn.y + 6, text_color);
    
    SDL_Rect load_btn = {panel_x + 10 + (panel_width - 30) / 2 + 10, y, (panel_width - 30) / 2, 25};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &load_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &load_btn);
    draw_text(renderer, "Load", load_btn.x + 35, load_btn.y + 6, text_color);
    y += 32;
    
    // === SECTION 5: PROGRESS ===
    if (state->training_in_progress || state->training_current_epoch > 0) {
        draw_text(renderer, "PROGRESS", panel_x + 10, y, text_color);
        y += 20;
        
        // Epoch progress
        char progress[64];
        snprintf(progress, sizeof(progress), "Epoch: %d / %d", 
                state->training_current_epoch, state->training_epochs);
        draw_text(renderer, progress, panel_x + 10, y, text_color);
        y += 16;
        
        // Progress bar
        SDL_Rect progress_bg = {panel_x + 10, y, panel_width - 20, 12};
        SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
        SDL_RenderFillRect(renderer, &progress_bg);
        SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
        SDL_RenderDrawRect(renderer, &progress_bg);
        
        if (state->training_epochs > 0) {
            float progress_ratio = (float)state->training_current_epoch / state->training_epochs;
            int progress_width = (int)(progress_ratio * (progress_bg.w - 4));
            SDL_Rect progress_fill = {progress_bg.x + 2, progress_bg.y + 2, progress_width, progress_bg.h - 4};
            SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
            SDL_RenderFillRect(renderer, &progress_fill);
        }
        y += 18;
        
        // Loss
        char loss_str[64];
        snprintf(loss_str, sizeof(loss_str), "Loss: %.4f", state->training_loss);
        draw_text(renderer, loss_str, panel_x + 10, y, text_color);
    }
    
    // === VISUALIZATION AREA ===
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_Rect viz_rect = {0, 60, RENDER_WIDTH, WINDOW_HEIGHT - 60};
    SDL_RenderFillRect(renderer, &viz_rect);
    
    if (history_count > 0) {
        draw_text(renderer, "Training Loss Curve", 20, 80, text_color);
        
        // Draw axes
        int graph_x = 50;
        int graph_y = 120;
        int graph_w = RENDER_WIDTH - 100;
        int graph_h = WINDOW_HEIGHT - 200;
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
        SDL_RenderDrawLine(renderer, graph_x, graph_y, graph_x, graph_y + graph_h);
        SDL_RenderDrawLine(renderer, graph_x, graph_y + graph_h, graph_x + graph_w, graph_y + graph_h);
        
        // Find min/max loss for scaling
        float min_loss = loss_history[0];
        float max_loss = loss_history[0];
        for (int i = 1; i < history_count; i++) {
            if (loss_history[i] < min_loss) min_loss = loss_history[i];
            if (loss_history[i] > max_loss) max_loss = loss_history[i];
        }
        
        float loss_range = max_loss - min_loss;
        if (loss_range < 0.001f) loss_range = 0.001f;
        
        // Draw loss curve
        SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
        for (int i = 1; i < history_count; i++) {
            int x1 = graph_x + (int)(((i - 1) / (float)history_count) * graph_w);
            int y1 = graph_y + graph_h - (int)(((loss_history[i - 1] - min_loss) / loss_range) * graph_h);
            int x2 = graph_x + (int)((i / (float)history_count) * graph_w);
            int y2 = graph_y + graph_h - (int)(((loss_history[i] - min_loss) / loss_range) * graph_h);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
        
        // Draw labels
        char min_label[32], max_label[32];
        snprintf(min_label, sizeof(min_label), "%.4f", min_loss);
        snprintf(max_label, sizeof(max_label), "%.4f", max_loss);
        draw_text(renderer, max_label, graph_x - 45, graph_y, text_color);
        draw_text(renderer, min_label, graph_x - 45, graph_y + graph_h - 10, text_color);
        
        char epoch_label[32];
        snprintf(epoch_label, sizeof(epoch_label), "Epoch %d", history_count);
        draw_text(renderer, epoch_label, graph_x + graph_w - 60, graph_y + graph_h + 15, text_color);
    } else {
        draw_text(renderer, "Select training data and start training", 
                 RENDER_WIDTH / 2 - 150, WINDOW_HEIGHT / 2, text_color);
        draw_text(renderer, "Loss curve will appear here during training", 
                 RENDER_WIDTH / 2 - 170, WINDOW_HEIGHT / 2 + 30, 
                 (SDL_Color){150, 150, 150, 255});
    }
}

// Handle SDL events for text inputs
bool handle_training_tab_event(AppState* state, SDL_Event* event) {
    if (!state || !event) return false;
    
    // Handle text input events
    if (text_input_handle_event(&learning_rate_input, event)) {
        // Update state when input changes
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
    
    return false;
}

void handle_training_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    int panel_x = RENDER_WIDTH;
    int panel_width = CONTROL_PANEL_WIDTH;
    
    // Check text input clicks first
    if (inputs_initialized) {
        // Check if click is in any text input bounds
        if (x >= learning_rate_input.bounds.x && x <= learning_rate_input.bounds.x + learning_rate_input.bounds.w &&
            y >= learning_rate_input.bounds.y && y <= learning_rate_input.bounds.y + learning_rate_input.bounds.h) {
            text_input_activate(&learning_rate_input);
            text_input_deactivate(&epochs_input);
            text_input_deactivate(&batch_size_input);
            SDL_StartTextInput();
            return;
        }
        if (x >= epochs_input.bounds.x && x <= epochs_input.bounds.x + epochs_input.bounds.w &&
            y >= epochs_input.bounds.y && y <= epochs_input.bounds.y + epochs_input.bounds.h) {
            text_input_activate(&epochs_input);
            text_input_deactivate(&learning_rate_input);
            text_input_deactivate(&batch_size_input);
            SDL_StartTextInput();
            return;
        }
        if (x >= batch_size_input.bounds.x && x <= batch_size_input.bounds.x + batch_size_input.bounds.w &&
            y >= batch_size_input.bounds.y && y <= batch_size_input.bounds.y + batch_size_input.bounds.h) {
            text_input_activate(&batch_size_input);
            text_input_deactivate(&learning_rate_input);
            text_input_deactivate(&epochs_input);
            SDL_StartTextInput();
            return;
        }
    }
    
    // Scan Directory Button (y ≈ 118)
    SDL_Rect scan_btn = {panel_x + 10, 118, (panel_width - 30) / 2, 25};
    if (x >= scan_btn.x && x <= scan_btn.x + scan_btn.w &&
        y >= scan_btn.y && y <= scan_btn.y + scan_btn.h) {
        scan_training_directory("data/training");
        return;
    }
    
    // Select All Button (y ≈ 118)
    SDL_Rect select_all_btn = {panel_x + 10 + (panel_width - 30) / 2 + 10, 118, (panel_width - 30) / 2, 25};
    if (x >= select_all_btn.x && x <= select_all_btn.x + select_all_btn.w &&
        y >= select_all_btn.y && y <= select_all_btn.y + select_all_btn.h) {
        bool all_selected = true;
        for (int i = 0; i < file_count; i++) {
            if (!training_files[i].selected) {
                all_selected = false;
                break;
            }
        }
        // Toggle: if all selected, deselect all; otherwise select all
        for (int i = 0; i < file_count; i++) {
            training_files[i].selected = !all_selected;
        }
        return;
    }
    
    // File list clicks (y ≈ 170-290)
    SDL_Rect list_rect = {panel_x + 10, 170, panel_width - 20, 120};
    if (x >= list_rect.x && x <= list_rect.x + list_rect.w &&
        y >= list_rect.y && y <= list_rect.y + list_rect.h) {
        int file_index = scroll_offset + (y - list_rect.y - 5) / 16;
        if (file_index >= 0 && file_index < file_count) {
            training_files[file_index].selected = !training_files[file_index].selected;
            printf("Toggled file: %s (selected: %d)\n", 
                   training_files[file_index].filename, training_files[file_index].selected);
        }
        return;
    }
    
    // Epochs slider (y ≈ 326)
    SDL_Rect epochs_slider = {panel_x + 10, 326, panel_width - 20, 8};
    if (x >= epochs_slider.x && x <= epochs_slider.x + epochs_slider.w &&
        y >= epochs_slider.y - 5 && y <= epochs_slider.y + epochs_slider.h + 5) {
        float ratio = (float)(x - epochs_slider.x) / epochs_slider.w;
        state->training_epochs = (int)(ratio * 100) + 1;
        if (state->training_epochs < 1) state->training_epochs = 1;
        if (state->training_epochs > 100) state->training_epochs = 100;
        return;
    }
    
    // Learning rate slider (y ≈ 360)
    SDL_Rect lr_slider = {panel_x + 10, 360, panel_width - 20, 8};
    if (x >= lr_slider.x && x <= lr_slider.x + lr_slider.w &&
        y >= lr_slider.y - 5 && y <= lr_slider.y + lr_slider.h + 5) {
        float ratio = (float)(x - lr_slider.x) / lr_slider.w;
        state->training_learning_rate = 0.0001f + ratio * (0.01f - 0.0001f);
        return;
    }
    
    // Start/Stop Training Button (y ≈ 393)
    SDL_Rect train_btn = {panel_x + 10, 393, panel_width - 20, 35};
    if (x >= train_btn.x && x <= train_btn.x + train_btn.w &&
        y >= train_btn.y && y <= train_btn.y + train_btn.h) {
        
        if (state->training_in_progress) {
            // Stop training
            state->training_in_progress = false;
            printf("Stopping training...\n");
        } else {
            // Start training
            // Count selected files
            int selected_count = 0;
            for (int i = 0; i < file_count; i++) {
                if (training_files[i].selected) selected_count++;
            }
            
            if (selected_count == 0) {
                printf("No files selected for training\n");
                return;
            }
            
            if (!state->cllm_model) {
                printf("No model loaded. Creating new model...\n");
                // Create a new model with default parameters
                // Create models directory if it doesn't exist
                int mkdir_ret = system("mkdir -p ~/.cllm_models");
                (void)mkdir_ret;  // Explicitly ignore return value
                
                // Load base model
                char model_path[512];
                snprintf(model_path, sizeof(model_path), "%s/.cllm_models/base_model.cllm",
                        getenv("HOME") ? getenv("HOME") : ".");
                state->cllm_model = cllm_read_model(model_path);
                if (!state->cllm_model) {
                    printf("Failed to load base model\n");
                    return;
                }
            }
            
            if (!state->cllm_training) {
                CLLMTrainingConfig config = {
                    .num_epochs = state->training_epochs,
                    .batch_size = 32,
                    .sequence_length = 128,
                    .learning_rate = state->training_learning_rate,
                    .weight_decay = 0.01f,
                    .gradient_clip = 1.0f,
                    .warmup_steps = 100,
                    .save_every = 5,
                    .eval_interval = 100,
                    .max_steps = 10000
                };
                strcpy(config.optimizer, "adam");
                state->cllm_training = cllm_training_init(state->cllm_model, &config);
            }
            
            if (state->cllm_training) {
                // Load training data from selected files
                for (int i = 0; i < file_count; i++) {
                    if (training_files[i].selected) {
                        printf("Loading training data: %s\n", training_files[i].full_path);
                        cllm_load_training_data(state->cllm_training, training_files[i].full_path);
                    }
                }
                
                state->training_in_progress = true;
                state->training_current_epoch = 0;
                state->training_loss = 0.0f;
                history_count = 0;
                
                // Start training in background thread
                if (!training_thread_running) {
                    training_thread_running = true;
                    pthread_create(&training_thread, NULL, training_thread_func, state);
                    pthread_detach(training_thread);
                }
                
                printf("Training started with %d files\n", selected_count);
            }
        }
        return;
    }
    
    // Save Checkpoint Button (y ≈ 470)
    SDL_Rect save_btn = {panel_x + 10, 470, (panel_width - 30) / 2, 25};
    if (x >= save_btn.x && x <= save_btn.x + save_btn.w &&
        y >= save_btn.y && y <= save_btn.y + save_btn.h) {
        if (state->cllm_training) {
            char checkpoint_path[512];
            snprintf(checkpoint_path, sizeof(checkpoint_path), "checkpoints/manual_checkpoint");
            int result = cllm_save_checkpoint(state->cllm_training, checkpoint_path);
            if (result == 0) {
                printf("Checkpoint saved: %s\n", checkpoint_path);
            } else {
                printf("Failed to save checkpoint\n");
            }
        }
        return;
    }
    
    // Load Checkpoint Button (y ≈ 470)
    SDL_Rect load_btn = {panel_x + 10 + (panel_width - 30) / 2 + 10, 470, (panel_width - 30) / 2, 25};
    if (x >= load_btn.x && x <= load_btn.x + load_btn.w &&
        y >= load_btn.y && y <= load_btn.y + load_btn.h) {
        if (state->cllm_training) {
            char checkpoint_path[512];
            snprintf(checkpoint_path, sizeof(checkpoint_path), "checkpoints/manual_checkpoint");
            int result = cllm_load_checkpoint(state->cllm_training, checkpoint_path);
            if (result == 0) {
                printf("Checkpoint loaded: %s\n", checkpoint_path);
            } else {
                printf("Failed to load checkpoint\n");
            }
        }
        return;
    }
}

void handle_training_tab_text_input(AppState* state, const char* text) {
    if (!state || !text || !inputs_initialized) return;
    
    // Append text to active input
    if (learning_rate_input.active) {
        size_t len = strlen(learning_rate_input.text);
        if (len < MAX_INPUT_LENGTH - 1) {
            strncat(learning_rate_input.text, text, MAX_INPUT_LENGTH - len - 1);
            learning_rate_input.cursor_pos = strlen(learning_rate_input.text);
            state->training_learning_rate = atof(learning_rate_input.text);
        }
    } else if (epochs_input.active) {
        size_t len = strlen(epochs_input.text);
        if (len < MAX_INPUT_LENGTH - 1) {
            strncat(epochs_input.text, text, MAX_INPUT_LENGTH - len - 1);
            epochs_input.cursor_pos = strlen(epochs_input.text);
            state->training_epochs = atoi(epochs_input.text);
        }
    } else if (batch_size_input.active) {
        size_t len = strlen(batch_size_input.text);
        if (len < MAX_INPUT_LENGTH - 1) {
            strncat(batch_size_input.text, text, MAX_INPUT_LENGTH - len - 1);
            batch_size_input.cursor_pos = strlen(batch_size_input.text);
        }
    }
}

void handle_training_tab_keydown(AppState* state, SDL_Keycode key) {
    if (!state || !inputs_initialized) return;
    
    // Handle backspace
    if (key == SDLK_BACKSPACE) {
        if (learning_rate_input.active && learning_rate_input.cursor_pos > 0) {
            learning_rate_input.text[--learning_rate_input.cursor_pos] = '\0';
            state->training_learning_rate = atof(learning_rate_input.text);
        } else if (epochs_input.active && epochs_input.cursor_pos > 0) {
            epochs_input.text[--epochs_input.cursor_pos] = '\0';
            state->training_epochs = atoi(epochs_input.text);
        } else if (batch_size_input.active && batch_size_input.cursor_pos > 0) {
            batch_size_input.text[--batch_size_input.cursor_pos] = '\0';
        }
    }
    // Handle return/enter - deactivate input
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        text_input_deactivate(&learning_rate_input);
        text_input_deactivate(&epochs_input);
        text_input_deactivate(&batch_size_input);
        SDL_StopTextInput();
    }
}