// tab_models.c - Model Management Tab
// Provides UI for creating, loading, saving, and managing CLLM models

#include "../../app_common.h"
#include "../../../include/cllm_model_manager.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

// UI State
typedef struct {
    int selected_model_index;
    bool show_create_dialog;
    bool show_delete_confirm;
    char create_model_name[256];
    uint32_t create_vocab_size;
    uint32_t create_embedding_dim;
    uint32_t create_num_layers;
    uint32_t create_num_heads;
    char status_message[512];
    float status_message_time;
} ModelsTabState;

static ModelsTabState g_models_state = {
    .selected_model_index = -1,
    .show_create_dialog = false,
    .show_delete_confirm = false,
    .create_model_name = "",
    .create_vocab_size = 10000,
    .create_embedding_dim = 512,
    .create_num_layers = 6,
    .create_num_heads = 8,
    .status_message = "",
    .status_message_time = 0.0f
};

// Button positions
#define BUTTON_HEIGHT 30
#define BUTTON_SPACING 10
#define PANEL_PADDING 20

// Set status message
static void set_status_message(const char* message) {
    snprintf(g_models_state.status_message, sizeof(g_models_state.status_message), "%s", message);
    g_models_state.status_message_time = 3.0f; // Show for 3 seconds
}

// Draw create model dialog
static void draw_create_dialog(AppState* state, int x, int y, int width, int height) {
    // Dialog background
    SDL_Rect dialog_rect = {x, y, width, height};
    SDL_SetRenderDrawColor(state->renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(state->renderer, &dialog_rect);
    
    // Dialog border
    SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(state->renderer, &dialog_rect);
    
    int current_y = y + PANEL_PADDING;
    
    // Title
    // TODO: Render "Create New Model" title
    current_y += 40;
    
    // Model name input
    // TODO: Render "Model Name:" label
    // TODO: Render text input field
    current_y += 50;
    
    // Vocabulary size input
    // TODO: Render "Vocabulary Size:" label
    // TODO: Render number input field
    current_y += 50;
    
    // Embedding dimension input
    // TODO: Render "Embedding Dimension:" label
    // TODO: Render number input field
    current_y += 50;
    
    // Number of layers input
    // TODO: Render "Number of Layers:" label
    // TODO: Render number input field
    current_y += 50;
    
    // Number of heads input
    // TODO: Render "Number of Heads:" label
    // TODO: Render number input field
    current_y += 50;
    
    // Buttons
    SDL_Rect create_button = {x + PANEL_PADDING, current_y, 100, BUTTON_HEIGHT};
    SDL_Rect cancel_button = {x + PANEL_PADDING + 110, current_y, 100, BUTTON_HEIGHT};
    
    // Create button
    SDL_SetRenderDrawColor(state->renderer, 0, 150, 0, 255);
    SDL_RenderFillRect(state->renderer, &create_button);
    // TODO: Render "Create" text
    
    // Cancel button
    SDL_SetRenderDrawColor(state->renderer, 150, 0, 0, 255);
    SDL_RenderFillRect(state->renderer, &cancel_button);
    // TODO: Render "Cancel" text
}

// Draw model list
static void draw_model_list(AppState* state, int x, int y, int width, int height) {
    // Panel background
    SDL_Rect panel_rect = {x, y, width, height};
    SDL_SetRenderDrawColor(state->renderer, 30, 30, 30, 255);
    SDL_RenderFillRect(state->renderer, &panel_rect);
    
    // Panel border
    SDL_SetRenderDrawColor(state->renderer, 80, 80, 80, 255);
    SDL_RenderDrawRect(state->renderer, &panel_rect);
    
    int current_y = y + PANEL_PADDING;
    
    // Title
    // TODO: Render "Available Models" title
    current_y += 40;
    
    // Get model list
    uint32_t model_count = 0;
    ManagedModel** models = model_manager_list(&model_count);
    
    if (model_count == 0) {
        // No models message
        // TODO: Render "No models available. Create a new model to get started."
        current_y += 30;
    } else {
        // Draw each model
        for (uint32_t i = 0; i < model_count; i++) {
            SDL_Rect model_rect = {
                x + PANEL_PADDING,
                current_y,
                width - 2 * PANEL_PADDING,
                60
            };
            
            // Background (highlight if selected)
            if ((int)i == g_models_state.selected_model_index) {
                SDL_SetRenderDrawColor(state->renderer, 60, 60, 100, 255);
            } else {
                SDL_SetRenderDrawColor(state->renderer, 40, 40, 40, 255);
            }
            SDL_RenderFillRect(state->renderer, &model_rect);
            
            // Border
            SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
            SDL_RenderDrawRect(state->renderer, &model_rect);
            
            // Model info
            // TODO: Render model name
            // TODO: Render model status (training/idle/in-use)
            // TODO: Render model size info
            
            current_y += 70;
        }
    }
    
    // Free model list
    if (models) {
        free(models);
    }
}

// Draw action buttons
static void draw_action_buttons(AppState* state, int x, int y, int width) {
    int button_width = (width - 4 * BUTTON_SPACING) / 3;
    int current_x = x;
    
    // Create button
    SDL_Rect create_button = {current_x, y, button_width, BUTTON_HEIGHT};
    SDL_SetRenderDrawColor(state->renderer, 0, 120, 0, 255);
    SDL_RenderFillRect(state->renderer, &create_button);
    // TODO: Render "Create New" text
    current_x += button_width + BUTTON_SPACING;
    
    // Load button
    SDL_Rect load_button = {current_x, y, button_width, BUTTON_HEIGHT};
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 120, 255);
    SDL_RenderFillRect(state->renderer, &load_button);
    // TODO: Render "Load" text
    current_x += button_width + BUTTON_SPACING;
    
    // Delete button (only if model selected)
    if (g_models_state.selected_model_index >= 0) {
        SDL_Rect delete_button = {current_x, y, button_width, BUTTON_HEIGHT};
        SDL_SetRenderDrawColor(state->renderer, 120, 0, 0, 255);
        SDL_RenderFillRect(state->renderer, &delete_button);
        // TODO: Render "Delete" text
    }
}

// Draw model details panel
static void draw_model_details(AppState* state, int x, int y, int width, int height) {
    // Panel background
    SDL_Rect panel_rect = {x, y, width, height};
    SDL_SetRenderDrawColor(state->renderer, 30, 30, 30, 255);
    SDL_RenderFillRect(state->renderer, &panel_rect);
    
    // Panel border
    SDL_SetRenderDrawColor(state->renderer, 80, 80, 80, 255);
    SDL_RenderDrawRect(state->renderer, &panel_rect);
    
    if (g_models_state.selected_model_index < 0) {
        // No model selected message
        // TODO: Render "Select a model to view details"
        return;
    }
    
    // Get selected model info
    uint32_t model_count = 0;
    ManagedModel** models = model_manager_list(&model_count);
    
    if (g_models_state.selected_model_index >= (int)model_count) {
        if (models) free(models);
        return;
    }
    
    ManagedModel* model = models[g_models_state.selected_model_index];
    (void)model; // Suppress unused warning - will be used when rendering is implemented
    
    int current_y = y + PANEL_PADDING;
    
    // Model details
    // TODO: Render model name
    current_y += 40;
    
    // TODO: Render vocabulary size
    current_y += 30;
    
    // TODO: Render embedding dimension
    current_y += 30;
    
    // TODO: Render number of layers
    current_y += 30;
    
    // TODO: Render number of heads
    current_y += 30;
    
    // TODO: Render model status
    current_y += 30;
    
    // TODO: Render file path
    current_y += 30;
    
    // Action buttons
    current_y += 20;
    
    SDL_Rect export_button = {x + PANEL_PADDING, current_y, 120, BUTTON_HEIGHT};
    SDL_SetRenderDrawColor(state->renderer, 0, 100, 150, 255);
    SDL_RenderFillRect(state->renderer, &export_button);
    // TODO: Render "Export" text
    
    SDL_Rect save_button = {x + PANEL_PADDING + 130, current_y, 120, BUTTON_HEIGHT};
    SDL_SetRenderDrawColor(state->renderer, 0, 150, 0, 255);
    SDL_RenderFillRect(state->renderer, &save_button);
    // TODO: Render "Save" text
    
    // Free model list
    if (models) {
        free(models);
    }
}

// Draw status message
static void draw_status_message(AppState* state, int x, int y, int width) {
    if (g_models_state.status_message_time <= 0.0f) {
        return;
    }
    
    // Status bar
    SDL_Rect status_rect = {x, y, width, 40};
    SDL_SetRenderDrawColor(state->renderer, 50, 50, 100, 255);
    SDL_RenderFillRect(state->renderer, &status_rect);
    
    // TODO: Render status message text
    
    // Decrease timer
    g_models_state.status_message_time -= 0.016f; // Assuming 60 FPS
}

// Main draw function
void draw_models_tab(AppState* state) {
    int window_width = WINDOW_WIDTH - SIDEBAR_WIDTH - 40;  // Fixed: account for sidebar
    int window_height = WINDOW_HEIGHT - SUBMENU_HEIGHT - 60;  // Fixed: account for submenu
    int x = RENDER_OFFSET_X + 20;  // Fixed: account for sidebar
    int y = RENDER_OFFSET_Y + 20;  // Fixed: account for submenu
    
    // Two-column layout
    int left_width = window_width * 0.4;
    int right_width = window_width * 0.6 - BUTTON_SPACING;
    
    // Left column: Model list
    draw_model_list(state, x, y, left_width, window_height - 100);
    
    // Right column: Model details
    draw_model_details(state, x + left_width + BUTTON_SPACING, y, right_width, window_height - 100);
    
    // Action buttons at bottom
    draw_action_buttons(state, x, y + window_height - 80, window_width);
    
    // Status message
    draw_status_message(state, x, y + window_height - 30, window_width);
    
    // Create dialog (if shown)
    if (g_models_state.show_create_dialog) {
        int dialog_width = 500;
        int dialog_height = 450;
        int dialog_x = (WINDOW_WIDTH - dialog_width) / 2;
        int dialog_y = (WINDOW_HEIGHT - dialog_height) / 2;
        draw_create_dialog(state, dialog_x, dialog_y, dialog_width, dialog_height);
    }
}

// Handle click events
void handle_models_tab_click(AppState* state, int x, int y) {
    (void)state; // Suppress unused warning
    (void)x;     // Suppress unused warning
    (void)y;     // Suppress unused warning
    
    // TODO: Implement click handling
    // - Model selection
    // - Button clicks
    // - Dialog interactions
    
    set_status_message("Click handling not yet implemented");
}

// Initialize models tab
void init_models_tab(AppState* state) {
    (void)state; // Suppress unused warning
    g_models_state.selected_model_index = -1;
    g_models_state.show_create_dialog = false;
    g_models_state.show_delete_confirm = false;
    g_models_state.status_message_time = 0.0f;
}

// Cleanup models tab
void cleanup_models_tab(AppState* state) {
    (void)state; // Suppress unused warning
    // Nothing to cleanup yet
}