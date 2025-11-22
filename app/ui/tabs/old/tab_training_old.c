// app/ui/tabs/tab_training.c - Training Tab
#include "../../app_common.h"
#include <SDL2/SDL_ttf.h>
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
    int panel_height = WINDOW_HEIGHT - 60;
    
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {40, 40, 50, 255};
    SDL_Color button_color = {60, 60, 80, 255};
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_Rect bg_rect = {panel_x, panel_y, panel_width, panel_height};
    SDL_RenderFillRect(renderer, &bg_rect);
    
    int y_offset = panel_y + 20;
    
    // Title
    if (font_large) {
        SDL_Surface* surface = TTF_RenderText_Blended(font_large, "Training", text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {panel_x + 20, y_offset, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    y_offset += 40;
    
    // Training status
    const char* status = state->training_in_progress ? "Status: Training..." : "Status: Ready";
    SDL_Color status_color = state->training_in_progress ? 
        (SDL_Color){255, 200, 100, 255} : (SDL_Color){100, 255, 100, 255};
    
    if (font_small) {
        SDL_Surface* surface = TTF_RenderText_Blended(font_small, status, status_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {panel_x + 20, y_offset, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    y_offset += 30;
    
    // Scan directory button
    SDL_Rect scan_button = {panel_x + 20, y_offset, panel_width - 40, 30};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &scan_button);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &scan_button);
    
    if (font_small) {
        SDL_Surface* surface = TTF_RenderText_Blended(font_small, "Scan Data Directory", text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {scan_button.x + (scan_button.w - surface->w) / 2, 
                               scan_button.y + (scan_button.h - surface->h) / 2, 
                               surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    y_offset += 40;
    
    // File list
    if (font_medium) {
        char file_label[64];
        snprintf(file_label, sizeof(file_label), "Training Files (%d):", file_count);
        SDL_Surface* surface = TTF_RenderText_Blended(font_medium, file_label, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {panel_x + 20, y_offset, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    y_offset += 25;
    
    // File list box
    int list_height = 200;
    SDL_Rect list_box = {panel_x + 20, y_offset, panel_width - 40, list_height};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &list_box);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &list_box);
    
    // Draw files
    int file_y = list_box.y + 5;
    for (int i = 0; i < file_count && i < 10; i++) {
        SDL_Color file_color = (i == selected_file) ? 
            (SDL_Color){100, 200, 255, 255} : text_color;
        
        if (font_small) {
            SDL_Surface* surface = TTF_RenderText_Blended(font_small, 
                training_files[i].filename, file_color);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect dest = {list_box.x + 5, file_y, surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, NULL, &dest);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
        file_y += 18;
    }
    y_offset += list_height + 10;
    
    // Training parameters
    if (font_medium) {
        SDL_Surface* surface = TTF_RenderText_Blended(font_medium, "Parameters:", text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {panel_x + 20, y_offset, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    y_offset += 25;
    
    // Epochs
    char epochs_label[64];
    snprintf(epochs_label, sizeof(epochs_label), "Epochs: %d", state->training_epochs);
    if (font_small) {
        SDL_Surface* surface = TTF_RenderText_Blended(font_small, epochs_label, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {panel_x + 20, y_offset, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    y_offset += 20;
    
    // Learning rate
    char lr_label[64];
    snprintf(lr_label, sizeof(lr_label), "Learning Rate: %.4f", state->training_learning_rate);
    if (font_small) {
        SDL_Surface* surface = TTF_RenderText_Blended(font_small, lr_label, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {panel_x + 20, y_offset, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    y_offset += 30;
    
    // Start Training Button
    SDL_Rect train_button = {panel_x + 20, y_offset, panel_width - 40, 35};
    SDL_Color train_color = state->training_in_progress ? 
        (SDL_Color){100, 100, 120, 255} : (SDL_Color){80, 150, 80, 255};
    SDL_SetRenderDrawColor(renderer, train_color.r, train_color.g, train_color.b, 255);
    SDL_RenderFillRect(renderer, &train_button);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &train_button);
    
    const char* train_text = state->training_in_progress ? "Training..." : "Start Training";
    if (font_medium) {
        SDL_Surface* surface = TTF_RenderText_Blended(font_medium, train_text, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {train_button.x + (train_button.w - surface->w) / 2, 
                               train_button.y + (train_button.h - surface->h) / 2, 
                               surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    y_offset += 50;
    
    // Training progress
    if (state->training_in_progress) {
        char progress[128];
        snprintf(progress, sizeof(progress), "Epoch: %d/%d", 
                state->training_current_epoch, state->training_epochs);
        if (font_small) {
            SDL_Surface* surface = TTF_RenderText_Blended(font_small, progress, text_color);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect dest = {panel_x + 20, y_offset, surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, NULL, &dest);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
        y_offset += 20;
        
        char loss_str[64];
        snprintf(loss_str, sizeof(loss_str), "Loss: %.4f", state->training_loss);
        if (font_small) {
            SDL_Surface* surface = TTF_RenderText_Blended(font_small, loss_str, text_color);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect dest = {panel_x + 20, y_offset, surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, NULL, &dest);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
    }
    
    // Draw visualization area
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_Rect viz_rect = {0, 60, RENDER_WIDTH, WINDOW_HEIGHT - 60};
    SDL_RenderFillRect(renderer, &viz_rect);
    
    // Draw training visualization
    if (state->cllm_training) {
        int cx = RENDER_WIDTH / 2;
        int cy = (WINDOW_HEIGHT - 60) / 2 + 60;
        
        // Draw loss curve placeholder
        if (font_large) {
            SDL_Surface* surface = TTF_RenderText_Blended(font_large, 
                "Training Loss Visualization", text_color);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect dest = {20, 80, surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, NULL, &dest);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
        
        // Draw simple loss curve
        SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
        for (int i = 0; i < 100; i++) {
            int x = 50 + i * 10;
            int y = cy - (int)(100 * exp(-i * 0.02));
            SDL_RenderDrawPoint(renderer, x, y);
        }
    } else {
        if (font_medium) {
            const char* msg = "Select training data and start training";
            SDL_Surface* surface = TTF_RenderText_Blended(font_medium, msg, text_color);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect dest = {(RENDER_WIDTH - surface->w) / 2, 
                                   (WINDOW_HEIGHT - surface->h) / 2, 
                                   surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, NULL, &dest);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
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
            
            // Create training instance if needed
            if (!state->cllm_training && state->cllm_model) {
                state->cllm_training = cllm_training_create(state->cllm_model, 
                    state->training_learning_rate, 32);
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