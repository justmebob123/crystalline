// app/ui/tabs/tab_training.c - Training Tab (Simplified)
#include "../../app_common.h"
#include <dirent.h>
#include <sys/stat.h>

#ifndef DT_REG
#define DT_REG 8
#endif

#define MAX_FILES 100
#define MAX_FILENAME_LENGTH 256

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char full_path[512];
} TrainingFile;

static TrainingFile training_files[MAX_FILES];
static int file_count = 0;
static int selected_file = -1;

static void scan_training_directory(const char* dir_path) {
    file_count = 0;
    DIR* dir = opendir(dir_path);
    if (!dir) return;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        if (entry->d_name[0] == '.') continue;
        
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
            TrainingFile* file = &training_files[file_count];
            snprintf(file->filename, MAX_FILENAME_LENGTH, "%s", entry->d_name);
            snprintf(file->full_path, sizeof(file->full_path), "%s", full_path);
            file_count++;
        }
    }
    closedir(dir);
}

void draw_training_tab(SDL_Renderer* renderer, AppState* state) {
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
    draw_text(renderer, "Training", panel_x + 20, y, text_color);
    y += 40;
    
    // Status
    const char* status = state->training_in_progress ? "Status: Training..." : "Status: Ready";
    SDL_Color status_color = state->training_in_progress ? 
        (SDL_Color){255, 200, 100, 255} : (SDL_Color){100, 255, 100, 255};
    draw_text(renderer, status, panel_x + 20, y, status_color);
    y += 30;
    
    // Scan button
    SDL_Rect scan_button = {panel_x + 20, y, panel_width - 40, 30};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &scan_button);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &scan_button);
    draw_text(renderer, "Scan Data Directory", scan_button.x + 50, scan_button.y + 8, text_color);
    y += 40;
    
    // File list
    char file_label[64];
    snprintf(file_label, sizeof(file_label), "Training Files (%d):", file_count);
    draw_text(renderer, file_label, panel_x + 20, y, text_color);
    y += 25;
    
    // File list box
    int list_height = 200;
    SDL_Rect list_box = {panel_x + 20, y, panel_width - 40, list_height};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &list_box);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &list_box);
    
    // Draw files
    int file_y = list_box.y + 5;
    for (int i = 0; i < file_count && i < 10; i++) {
        SDL_Color file_color = (i == selected_file) ? 
            (SDL_Color){100, 200, 255, 255} : text_color;
        draw_text(renderer, training_files[i].filename, list_box.x + 5, file_y, file_color);
        file_y += 18;
    }
    y += list_height + 10;
    
    // Parameters
    draw_text(renderer, "Parameters:", panel_x + 20, y, text_color);
    y += 25;
    
    char epochs_label[64];
    snprintf(epochs_label, sizeof(epochs_label), "Epochs: %d", state->training_epochs);
    draw_text(renderer, epochs_label, panel_x + 20, y, text_color);
    y += 20;
    
    char lr_label[64];
    snprintf(lr_label, sizeof(lr_label), "Learning Rate: %.4f", state->training_learning_rate);
    draw_text(renderer, lr_label, panel_x + 20, y, text_color);
    y += 30;
    
    // Start Training Button
    SDL_Rect train_button = {panel_x + 20, y, panel_width - 40, 35};
    SDL_Color train_color = state->training_in_progress ? 
        (SDL_Color){100, 100, 120, 255} : (SDL_Color){80, 150, 80, 255};
    SDL_SetRenderDrawColor(renderer, train_color.r, train_color.g, train_color.b, 255);
    SDL_RenderFillRect(renderer, &train_button);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &train_button);
    const char* train_text = state->training_in_progress ? "Training..." : "Start Training";
    draw_text(renderer, train_text, train_button.x + 80, train_button.y + 10, text_color);
    y += 50;
    
    // Training progress
    if (state->training_in_progress) {
        char progress[128];
        snprintf(progress, sizeof(progress), "Epoch: %d/%d", 
                state->training_current_epoch, state->training_epochs);
        draw_text(renderer, progress, panel_x + 20, y, text_color);
        y += 20;
        
        char loss_str[64];
        snprintf(loss_str, sizeof(loss_str), "Loss: %.4f", state->training_loss);
        draw_text(renderer, loss_str, panel_x + 20, y, text_color);
    }
    
    // Draw visualization area
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_Rect viz_rect = {0, 60, RENDER_WIDTH, WINDOW_HEIGHT - 60};
    SDL_RenderFillRect(renderer, &viz_rect);
    
    if (state->cllm_training) {
        draw_text(renderer, "Training Loss Visualization", 20, 80, text_color);
    } else {
        draw_text(renderer, "Select training data and start training", RENDER_WIDTH / 2 - 150, WINDOW_HEIGHT / 2, text_color);
    }
}

void handle_training_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    int panel_x = RENDER_WIDTH;
    
    // Scan button (y = 90)
    SDL_Rect scan_button = {panel_x + 20, 90, CONTROL_PANEL_WIDTH - 40, 30};
    if (x >= scan_button.x && x <= scan_button.x + scan_button.w &&
        y >= scan_button.y && y <= scan_button.y + scan_button.h) {
        scan_training_directory("data/training");
        printf("Scanned training directory: %d files found\n", file_count);
    }
    
    // File list clicks (y = 155 to 355)
    SDL_Rect list_box = {panel_x + 20, 155, CONTROL_PANEL_WIDTH - 40, 200};
    if (x >= list_box.x && x <= list_box.x + list_box.w &&
        y >= list_box.y && y <= list_box.y + list_box.h) {
        int file_index = (y - list_box.y - 5) / 18;
        if (file_index >= 0 && file_index < file_count) {
            selected_file = file_index;
            strcpy(state->training_data_path, training_files[file_index].full_path);
            printf("Selected training file: %s\n", training_files[file_index].filename);
        }
    }
    
    // Start Training button (y = 410)
    SDL_Rect train_button = {panel_x + 20, 410, CONTROL_PANEL_WIDTH - 40, 35};
    if (x >= train_button.x && x <= train_button.x + train_button.w &&
        y >= train_button.y && y <= train_button.y + train_button.h) {
        
        if (!state->training_in_progress && selected_file >= 0) {
            printf("Starting training with file: %s\n", state->training_data_path);
            
            if (!state->cllm_training && state->cllm_model) {
                CLLMTrainingConfig config = {
                    .num_epochs = state->training_epochs,
                    .batch_size = 32,
                    .sequence_length = 128,
                    .learning_rate = state->training_learning_rate,
                    .weight_decay = 0.01f,
                    .dropout = 0.1f,
                    .gradient_accum_steps = 1,
                    .save_every = 1,
                    .eval_every = 100,
                    .use_mixed_precision = false
                };
                state->cllm_training = cllm_training_init(state->cllm_model, &config);
            }
            
            if (state->cllm_training) {
                state->training_in_progress = true;
                state->training_current_epoch = 0;
                state->training_loss = 1.0f;
                printf("Training started!\n");
            }
        }
    }
}