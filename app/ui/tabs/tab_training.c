#include "../../app_common.h"
#include "../../text_input.h"
#include "cllm_format.h"
#include "cllm_training.h"
#include "cllm_vocab_builder.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

#define MAX_TRAINING_FILES 100

typedef struct {
    char filename[256];
    char filepath[512];
    bool selected;
    size_t size;
} TrainingFile;

static TrainingFile training_files[MAX_TRAINING_FILES];
static int file_count = 0;
static int scroll_offset = 0;

// Text input fields
static TextInput learning_rate_input;
static TextInput epochs_input;
static TextInput batch_size_input;
static bool inputs_initialized = false;

void scan_training_directory(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("Could not open directory: %s\n", dir_path);
        return;
    }
    
    file_count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL && file_count < MAX_TRAINING_FILES) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Check if it's a text file
        const char* ext = strrchr(entry->d_name, '.');
        if (ext && strcmp(ext, ".txt") == 0) {
            // Build full path
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
            
            // Check if it's a regular file
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

void init_training_tab(AppState* state) {
    (void)state;
    
    // Scan default training directory
    scan_training_directory("data/training");
    
    // Initialize text inputs
    if (!inputs_initialized) {
        text_input_init(&learning_rate_input, "Learning Rate:", RENDER_WIDTH + 10, 143, CONTROL_PANEL_WIDTH - 20, 25);
        text_input_set_text(&learning_rate_input, "0.001");
        
        text_input_init(&epochs_input, "Epochs:", RENDER_WIDTH + 10, 193, CONTROL_PANEL_WIDTH - 20, 25);
        text_input_set_text(&epochs_input, "10");
        
        text_input_init(&batch_size_input, "Batch Size:", RENDER_WIDTH + 10, 243, CONTROL_PANEL_WIDTH - 20, 25);
        text_input_set_text(&batch_size_input, "32");
        
        inputs_initialized = true;
    }
}

void draw_training_tab(SDL_Renderer* renderer, AppState* state) {
    if (!state) return;
    
    // Initialize text inputs if not done
    if (!inputs_initialized) {
        text_input_init(&learning_rate_input, "Learning Rate:", RENDER_WIDTH + 10, 143, CONTROL_PANEL_WIDTH - 20, 25);
        text_input_set_text(&learning_rate_input, "0.001");
        
        text_input_init(&epochs_input, "Epochs:", RENDER_WIDTH + 10, 193, CONTROL_PANEL_WIDTH - 20, 25);
        text_input_set_text(&epochs_input, "10");
        
        text_input_init(&batch_size_input, "Batch Size:", RENDER_WIDTH + 10, 243, CONTROL_PANEL_WIDTH - 20, 25);
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
            SDL_Rect progress_fill = {progress_bg.x, progress_bg.y, 
                                     (int)(progress_bg.w * progress_ratio), progress_bg.h};
            SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
            SDL_RenderFillRect(renderer, &progress_fill);
        }
        y += 18;
        
        // Loss
        char loss_text[64];
        snprintf(loss_text, sizeof(loss_text), "Loss: %.4f", state->training_loss);
        draw_text(renderer, loss_text, panel_x + 10, y, text_color);
    }
}

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
    
    // Scan Directory Button (y = 320)
    SDL_Rect scan_btn = {panel_x + 10, 320, (panel_width - 30) / 2, 25};
    if (x >= scan_btn.x && x <= scan_btn.x + scan_btn.w &&
        y >= scan_btn.y && y <= scan_btn.y + scan_btn.h) {
        printf("Scan directory button clicked\n");
        scan_training_directory("data/training");
        return;
    }
    
    // Select All Button (y = 320)
    SDL_Rect select_all_btn = {panel_x + 10 + (panel_width - 30) / 2 + 10, 320, (panel_width - 30) / 2, 25};
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
        printf("Select all toggled (all_selected was: %d)\n", all_selected);
        return;
    }
    
    // File list clicks (y = 372 to 492)
    SDL_Rect list_rect = {panel_x + 10, 372, panel_width - 20, 120};
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
    
    // Epochs slider (y = 528)
    SDL_Rect epochs_slider = {panel_x + 10, 528, panel_width - 20, 8};
    if (x >= epochs_slider.x && x <= epochs_slider.x + epochs_slider.w &&
        y >= epochs_slider.y - 5 && y <= epochs_slider.y + epochs_slider.h + 5) {
        float ratio = (float)(x - epochs_slider.x) / epochs_slider.w;
        state->training_epochs = (int)(ratio * 100) + 1;
        if (state->training_epochs < 1) state->training_epochs = 1;
        if (state->training_epochs > 100) state->training_epochs = 100;
        return;
    }
    
    // Learning rate slider (y = 562)
    SDL_Rect lr_slider = {panel_x + 10, 562, panel_width - 20, 8};
    if (x >= lr_slider.x && x <= lr_slider.x + lr_slider.w &&
        y >= lr_slider.y - 5 && y <= lr_slider.y + lr_slider.h + 5) {
        float ratio = (float)(x - lr_slider.x) / lr_slider.w;
        state->training_learning_rate = 0.0001f + ratio * (0.01f - 0.0001f);
        return;
    }
    
    // Start/Stop Training Button (y = 587)
    SDL_Rect train_btn = {panel_x + 10, 587, panel_width - 20, 35};
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
                    .batch_size = 4,        // Reduced from 32 for small datasets
                    .sequence_length = 32,  // Reduced from 128 for small datasets
                    .learning_rate = state->training_learning_rate,
                    .weight_decay = 0.01f,
                    .gradient_clip = 1.0f,
                    .warmup_steps = 100,
                    .save_every = 5,
                    .eval_interval = 100,
                    .max_steps = 10000
                };
                strcpy(config.optimizer, "adam");
                
                // STEP 1: Build vocabulary from training files FIRST
                printf("=== BUILDING VOCABULARY ===\n");
                for (int i = 0; i < file_count && i < MAX_TRAINING_FILES; i++) {
                    if (training_files[i].selected) {
                        printf("  Building vocab from: %s\n", training_files[i].filepath);
                        int vocab_size = cllm_build_vocabulary_from_file(state->cllm_model, 
                                                                          training_files[i].filepath);
                        if (vocab_size > 0) {
                            printf("  ✓ Vocabulary built: %d unique tokens\n", vocab_size);
                            break; // Only need to build vocab once
                        } else {
                            printf("  ✗ Failed to build vocabulary\n");
                        }
                    }
                }
                
                // STEP 2: Initialize training with vocabulary-enabled model
                state->cllm_training = cllm_training_init(state->cllm_model, &config);
                
                // STEP 3: Load training data from selected files
                if (state->cllm_training) {
                    printf("=== LOADING TRAINING DATA ===\n");
                    for (int i = 0; i < file_count && i < MAX_TRAINING_FILES; i++) {
                        if (training_files[i].selected) {
                            printf("  Loading: %s\n", training_files[i].filepath);
                            int tokens_loaded = cllm_load_training_data(state->cllm_training, 
                                                                        training_files[i].filepath);
                            if (tokens_loaded > 0) {
                                printf("  ✓ Loaded %d tokens\n", tokens_loaded);
                            } else {
                                printf("  ✗ Failed to load file\n");
                            }
                        }
                    }
                }
            }
            
            state->training_in_progress = true;
            state->training_current_epoch = 0;
            printf("=== STARTING TRAINING ===\n");
            printf("Training with %d files...\n", selected_count);
        }
        return;
    }
    
    // Save/Load Checkpoint Buttons (y = 629)
    SDL_Rect save_btn = {panel_x + 10, 629, (panel_width - 30) / 2, 25};
    if (x >= save_btn.x && x <= save_btn.x + save_btn.w &&
        y >= save_btn.y && y <= save_btn.y + save_btn.h) {
        printf("Save checkpoint button clicked\n");
        if (state->cllm_model) {
            // Create checkpoints directory
            int mkdir_result = system("mkdir -p checkpoints");
            (void)mkdir_result; // Explicitly ignore
            
            // Save model
            char checkpoint_path[512];
            snprintf(checkpoint_path, sizeof(checkpoint_path), 
                    "checkpoints/checkpoint_epoch_%d.cllm", state->training_current_epoch);
            
            if (cllm_write_model(state->cllm_model, checkpoint_path) == 0) {
                printf("✓ Checkpoint saved: %s\n", checkpoint_path);
            } else {
                printf("✗ Failed to save checkpoint\n");
            }
        } else {
            printf("No model to save\n");
        }
        return;
    }
    
    SDL_Rect load_btn = {panel_x + 10 + (panel_width - 30) / 2 + 10, 629, (panel_width - 30) / 2, 25};
    if (x >= load_btn.x && x <= load_btn.x + load_btn.w &&
        y >= load_btn.y && y <= load_btn.y + load_btn.h) {
        printf("Load checkpoint button clicked\n");
        // TODO: Implement checkpoint loading UI
        printf("Checkpoint loading not yet implemented\n");
        return;
    }
}

void cleanup_training_tab(void) {
    // Cleanup if needed
}

void handle_training_tab_text_input(AppState* state, const char* text) {
    // Text input is handled by the text_input_handle_event function
    (void)state;
    (void)text;
}

void handle_training_tab_keydown(AppState* state, int key) {
    if (!state) return;
    
    // Handle escape to deactivate inputs
    if (key == SDLK_ESCAPE) {
        text_input_deactivate(&learning_rate_input);
        text_input_deactivate(&epochs_input);
        text_input_deactivate(&batch_size_input);
        SDL_StopTextInput();
    }
    // Handle enter to confirm input
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
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
        }
    }
}