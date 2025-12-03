/**
 * Models Tab - Complete Rewrite Using New Component System
 * 
 * Provides proper model management with full wiring
 */

#include "../../app_common.h"
#include "../components.h"
#include "../state_manager.h"
#include "../event_system.h"
#include "../../../include/cllm_model_manager.h"
#include <stdio.h>
#include <string.h>

// UI Components
static UIButton* btn_create = NULL;
static UIButton* btn_load = NULL;
static UIButton* btn_save = NULL;
static UIButton* btn_delete = NULL;
static UIPanel* panel_model_list = NULL;
static UIPanel* panel_details = NULL;
static UIDialog* dialog_delete_confirm = NULL;

// Create model dialog inputs
static UITextInput* input_model_name = NULL;
static UISlider* slider_vocab_size = NULL;
static UISlider* slider_embedding_dim = NULL;
static UISlider* slider_num_layers = NULL;
static UISlider* slider_num_heads = NULL;
static UISlider* slider_hidden_dim = NULL;
static UIButton* btn_create_confirm = NULL;
static UIButton* btn_create_cancel = NULL;

// State
static int selected_model_index = -1;

// Double-click detection
static uint32_t last_click_time = 0;
static int last_clicked_index = -1;
#define DOUBLE_CLICK_MS 500
static bool show_create_dialog = false;
static char status_message[256] = "";
static float status_message_timer = 0.0f;

// Forward declarations
static void on_create_button_click(void* user_data);
static void on_load_button_click(void* user_data);
static void on_save_button_click(void* user_data);
static void on_delete_button_click(void* user_data);
static void on_create_confirm_click(void* user_data);
static void on_create_cancel_click(void* user_data);
static void on_delete_confirm(DialogResult result, void* user_data);
static void on_model_state_changed(const Event* event, void* user_data);
static void on_model_state_changed_simple(void* user_data);

// ============================================================================
// INITIALIZATION
// ============================================================================

void init_models_tab(AppState* state) {
    (void)state;
    
    // Create buttons
    btn_create = ui_button_create(20, 900, 150, 35, "Create New");
    btn_load = ui_button_create(180, 900, 150, 35, "Load Model");
    btn_save = ui_button_create(340, 900, 150, 35, "Save Model");
    btn_delete = ui_button_create(500, 900, 150, 35, "Delete");
    
    // Set button callbacks
    ui_button_set_callback(btn_create, on_create_button_click, state);
    ui_button_set_callback(btn_load, on_load_button_click, state);
    ui_button_set_callback(btn_save, on_save_button_click, state);
    ui_button_set_callback(btn_delete, on_delete_button_click, state);
    
    // Initially disable save/delete (no model selected)
    ui_button_set_enabled(btn_save, false);
    ui_button_set_enabled(btn_delete, false);
    
    // Create panels
    panel_model_list = ui_panel_create(20, 100, 600, 750, "Available Models");
    panel_details = ui_panel_create(640, 100, 600, 750, "Model Details");
    
    // Create delete confirmation dialog
    dialog_delete_confirm = ui_dialog_create(500, 300, 400, 200,
                                             "Confirm Delete",
                                             "Are you sure you want to delete this model?",
                                             DIALOG_YES_NO);
    ui_dialog_set_callback(dialog_delete_confirm, on_delete_confirm, state);
    
    // Create model creation dialog components
    input_model_name = ui_text_input_create(0, 0, 300, 30, "Enter model name");
    slider_vocab_size = ui_slider_create(0, 0, 300, 30, 1000, 50000, "Vocabulary Size");
    slider_embedding_dim = ui_slider_create(0, 0, 300, 30, 128, 2048, "Embedding Dimension");
    slider_num_layers = ui_slider_create(0, 0, 300, 30, 1, 32, "Number of Layers");
    slider_num_heads = ui_slider_create(0, 0, 300, 30, 1, 32, "Number of Heads");
    slider_hidden_dim = ui_slider_create(0, 0, 300, 30, 256, 4096, "Hidden Dimension");
    
    // Set default values
    ui_slider_set_value(slider_vocab_size, 10000);
    ui_slider_set_value(slider_embedding_dim, 512);
    ui_slider_set_value(slider_num_layers, 6);
    ui_slider_set_value(slider_num_heads, 8);
    ui_slider_set_value(slider_hidden_dim, 2048);
    
    btn_create_confirm = ui_button_create(0, 0, 100, 30, "Create");
    btn_create_cancel = ui_button_create(0, 0, 100, 30, "Cancel");
    
    ui_button_set_callback(btn_create_confirm, on_create_confirm_click, state);
    ui_button_set_callback(btn_create_cancel, on_create_cancel_click, state);
    
    // Register for model state changes
    StateManager* state_mgr = state_manager_get_instance();
    state_register_model_callback(state_mgr, on_model_state_changed_simple, state);
    
    // Register for model events
    EventSystem* event_sys = event_system_get_instance();
    event_register(event_sys, EVENT_MODEL_LOADED, on_model_state_changed, state);
    event_register(event_sys, EVENT_MODEL_CREATED, on_model_state_changed, state);
    event_register(event_sys, EVENT_MODEL_SAVED, on_model_state_changed, state);
    event_register(event_sys, EVENT_MODEL_DELETED, on_model_state_changed, state);
    
    selected_model_index = -1;
}

void cleanup_models_tab(AppState* state) {
    (void)state;
    
    // Cleanup buttons
    if (btn_create) ui_button_destroy(btn_create);
    if (btn_load) ui_button_destroy(btn_load);
    if (btn_save) ui_button_destroy(btn_save);
    if (btn_delete) ui_button_destroy(btn_delete);
    
    // Cleanup panels
    if (panel_model_list) ui_panel_destroy(panel_model_list);
    if (panel_details) ui_panel_destroy(panel_details);
    
    // Cleanup dialogs
    if (dialog_delete_confirm) ui_dialog_destroy(dialog_delete_confirm);
    
    // Cleanup create dialog components
    if (input_model_name) ui_text_input_destroy(input_model_name);
    if (slider_vocab_size) ui_slider_destroy(slider_vocab_size);
    if (slider_embedding_dim) ui_slider_destroy(slider_embedding_dim);
    if (slider_num_layers) ui_slider_destroy(slider_num_layers);
    if (slider_num_heads) ui_slider_destroy(slider_num_heads);
    if (slider_hidden_dim) ui_slider_destroy(slider_hidden_dim);
    if (btn_create_confirm) ui_button_destroy(btn_create_confirm);
    if (btn_create_cancel) ui_button_destroy(btn_create_cancel);
}

// ============================================================================
// BUTTON CALLBACKS
// ============================================================================

static void on_create_button_click(void* user_data) {
    (void)user_data;
    show_create_dialog = true;
    
    // Reset inputs
    ui_text_input_set_text(input_model_name, "");
    ui_slider_set_value(slider_vocab_size, 10000);
    ui_slider_set_value(slider_embedding_dim, 512);
    ui_slider_set_value(slider_num_layers, 6);
    ui_slider_set_value(slider_num_heads, 8);
    ui_slider_set_value(slider_hidden_dim, 2048);
}

static void on_load_button_click(void* user_data) {
    AppState* state = (AppState*)user_data;
    
    // TODO: Implement proper file picker dialog
    // For now, use a simple hardcoded path for testing
    const char* test_path = "models/test_model.cllm";
    const char* model_name = "test_model";
    
    printf("Loading model from: %s\n", test_path);
    
    // Check if model already exists in manager
    if (model_manager_exists(model_name)) {
        snprintf(status_message, sizeof(status_message), 
                 "Model '%s' already loaded", model_name);
        status_message_timer = 3.0f;
        return;
    }
    
    // Load model using model_manager
    ManagedModel* managed_model = model_manager_load(model_name, test_path);
    
    if (managed_model == NULL) {
        snprintf(status_message, sizeof(status_message), 
                 "Failed to load model from '%s'", test_path);
        status_message_timer = 3.0f;
        return;
    }
    
    // Update state manager
    StateManager* state_mgr = state_manager_get_instance();
    // Load the model through model manager (requires both name and path)
    ManagedModel* managed = model_manager_load(model_name, "");
    if (managed && managed->model) {
        state_set_model(state_mgr, managed->model, model_name, "");
    }
    state_update_model_config(state_mgr, 
                             managed_model->vocab_size,
                             managed_model->embedding_dim,
                             0,  // ff_dim not in metadata
                             managed_model->num_layers,
                             managed_model->num_heads);
    
    // Dispatch MODEL_LOADED event for cross-tab synchronization
    EventSystem* event_sys = event_system_get_instance();
    event_model_loaded(event_sys, model_name);
    
    snprintf(status_message, sizeof(status_message), 
             "Model '%s' loaded successfully", model_name);
    status_message_timer = 3.0f;
    
    (void)state;
}

static void on_save_button_click(void* user_data) {
    AppState* state = (AppState*)user_data;
    
    StateManager* state_mgr = state_manager_get_instance();
    const ModelState* model_state = state_get_model(state_mgr);
    
    if (!model_state || !model_state->model_loaded) {
        snprintf(status_message, sizeof(status_message), "No model loaded to save");
        status_message_timer = 3.0f;
        return;
    }
    
    const char* model_name = model_state->model_name;
    if (model_name[0] == '\0') {
        snprintf(status_message, sizeof(status_message), "No model selected");
        status_message_timer = 3.0f;
        return;
    }
    
    printf("Saving model: %s\n", model_name);
    
    // Save model using model_manager
    bool success = model_manager_save(model_name);
    
    if (!success) {
        snprintf(status_message, sizeof(status_message), 
                 "Failed to save model '%.200s'", model_name);
        status_message_timer = 3.0f;
        return;
    }
    
    // Dispatch MODEL_SAVED event for cross-tab synchronization
    EventSystem* event_sys = event_system_get_instance();
    event_model_saved(event_sys, model_name);
    
    snprintf(status_message, sizeof(status_message), 
             "Model '%.200s' saved successfully", model_name);
    status_message_timer = 3.0f;
    
    (void)state;
}

static void on_delete_button_click(void* user_data) {
    (void)user_data;
    
    if (selected_model_index < 0) {
        snprintf(status_message, sizeof(status_message), "No model selected");
        status_message_timer = 3.0f;
        return;
    }
    
    // Show confirmation dialog
    ui_dialog_show(dialog_delete_confirm);
}

static void on_create_confirm_click(void* user_data) {
    AppState* state = (AppState*)user_data;
    
    const char* name = ui_text_input_get_text(input_model_name);
    if (name[0] == '\0') {
        snprintf(status_message, sizeof(status_message), "Please enter a model name");
        status_message_timer = 3.0f;
        return;
    }
    
    // Check if model already exists
    if (model_manager_exists(name)) {
        snprintf(status_message, sizeof(status_message), "Model '%s' already exists", name);
        status_message_timer = 3.0f;
        return;
    }
    
    // Get configuration from sliders
    uint32_t vocab_size = ui_slider_get_value_int(slider_vocab_size);
    uint32_t embedding_dim = ui_slider_get_value_int(slider_embedding_dim);
    uint32_t num_layers = ui_slider_get_value_int(slider_num_layers);
    uint32_t num_heads = ui_slider_get_value_int(slider_num_heads);
    uint32_t hidden_dim = ui_slider_get_value_int(slider_hidden_dim);
    
    // Validate configuration
    if (embedding_dim % num_heads != 0) {
        snprintf(status_message, sizeof(status_message), 
                 "Embedding dimension must be divisible by number of heads");
        status_message_timer = 3.0f;
        return;
    }
    
    printf("Creating model: %s (vocab=%u, emb=%u, layers=%u, heads=%u, hidden=%u)\n",
           name, vocab_size, embedding_dim, num_layers, num_heads, hidden_dim);
    
    // Create CLLMConfig structure
    CLLMConfig config = {
        .vocab_size = vocab_size,
        .embedding_dim = embedding_dim,
        .num_layers = num_layers,
        .num_heads = num_heads,
        .ff_dim = hidden_dim,
        .max_seq_len = 512,  // Default sequence length
        .dropout = 0.1f      // Default dropout rate
    };
    
    // Create model using model_manager
    ManagedModel* managed_model = model_manager_create(name, &config);
    
    if (managed_model == NULL) {
        snprintf(status_message, sizeof(status_message), 
                 "Failed to create model '%s'", name);
        status_message_timer = 3.0f;
        return;
    }
    
    // Update state manager
    StateManager* state_mgr = state_manager_get_instance();
    state_update_model_config(state_mgr, vocab_size, embedding_dim, hidden_dim, num_layers, num_heads);
    // Model will be set when actually created/loaded
    state_set_model(state_mgr, NULL, name, "");
    
    // Dispatch MODEL_CREATED event for cross-tab synchronization
    EventSystem* event_sys = event_system_get_instance();
    event_model_created(event_sys, name);
    
    snprintf(status_message, sizeof(status_message), 
             "Model '%s' created successfully", name);
    status_message_timer = 3.0f;
    
    show_create_dialog = false;
    (void)state;
}

static void on_create_cancel_click(void* user_data) {
    (void)user_data;
    show_create_dialog = false;
}

static void on_delete_confirm(DialogResult result, void* user_data) {
    (void)user_data;
    
    if (result == DIALOG_RESULT_YES) {
        if (selected_model_index >= 0) {
            uint32_t model_count = 0;
            ManagedModel** models = model_manager_list(&model_count);
            
            if (selected_model_index < (int)model_count && models[selected_model_index]) {
                const char* model_name = models[selected_model_index]->name;
                
                printf("Deleting model: %s\n", model_name);
                
                // Delete model using model_manager (delete file too)
                bool success = model_manager_delete(model_name, true);
                
                if (!success) {
                    snprintf(status_message, sizeof(status_message), 
                             "Failed to delete model '%.200s'", model_name);
                    status_message_timer = 3.0f;
                    return;
                }
                
                // Update state manager
                StateManager* state_mgr = state_manager_get_instance();
                const ModelState* model_state = state_get_model(state_mgr);
                if (model_state && strcmp(model_state->model_name, model_name) == 0) {
                    state_set_model(state_mgr, NULL, "", "");
                }
                
                // Dispatch MODEL_DELETED event for cross-tab synchronization
                EventSystem* event_sys = event_system_get_instance();
                event_model_deleted(event_sys, model_name);
                
                snprintf(status_message, sizeof(status_message), 
                         "Model '%.200s' deleted successfully", model_name);
                status_message_timer = 3.0f;
                
                selected_model_index = -1;
                ui_button_set_enabled(btn_save, false);
                ui_button_set_enabled(btn_delete, false);
            }
        }
    }
}

static void on_model_state_changed(const Event* event, void* user_data) {
    (void)event;
    (void)user_data;
    
    // Update button states based on model state
    StateManager* state_mgr = state_manager_get_instance();
    const ModelState* model_state = state_get_model(state_mgr);
    
    if (model_state && model_state->model_loaded) {
        ui_button_set_enabled(btn_save, true);
    } else {
        ui_button_set_enabled(btn_save, false);
    }
}

// Simple wrapper for StateChangeCallback (no Event parameter)
static void on_model_state_changed_simple(void* user_data) {
    on_model_state_changed(NULL, user_data);
}

// ============================================================================
// RENDERING
// ============================================================================

void draw_models_tab(AppState* state) {
    if (!state || !state->renderer) return;
    
    // Update status message timer
    if (status_message_timer > 0.0f) {
        status_message_timer -= 0.016f;  // Assume ~60 FPS
    }
    
    // Draw panels
    ui_panel_render(panel_model_list, state->renderer);
    ui_panel_render(panel_details, state->renderer);
    
    // Draw model list inside panel
    SDL_Rect content = ui_panel_get_content_bounds(panel_model_list);
    if (content.w > 0 && content.h > 0) {
        uint32_t model_count = 0;
        ManagedModel** models = model_manager_list(&model_count);
        
        int item_height = 70;
        int current_y = content.y;
        
        extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
        
        if (model_count == 0) {
            SDL_Color msg_color = {150, 150, 150, 255};
            draw_text(state->renderer, "No models available.", content.x + 10, current_y, msg_color);
            draw_text(state->renderer, "Create a new model to get started.", content.x + 10, current_y + 20, msg_color);
        } else {
            for (uint32_t i = 0; i < model_count; i++) {
                if (models[i]) {
                    SDL_Rect item_rect = {content.x, current_y, content.w, item_height};
                    
                    // Highlight if selected (brighter color for visibility)
                    if ((int)i == selected_model_index) {
                        SDL_SetRenderDrawColor(state->renderer, 80, 120, 180, 255);  // Bright blue
                        SDL_RenderFillRect(state->renderer, &item_rect);
                    }
                    
                    // Draw model info
                    SDL_Color name_color = {220, 220, 220, 255};
                    SDL_Color info_color = {150, 170, 190, 255};
                    
                    draw_text(state->renderer, models[i]->name, item_rect.x + 10, item_rect.y + 10, name_color);
                    
                    char info[128];
                    snprintf(info, sizeof(info), "Vocab: %u | Layers: %u", 
                            models[i]->vocab_size, models[i]->num_layers);
                    draw_text(state->renderer, info, item_rect.x + 10, item_rect.y + 35, info_color);
                    
                    current_y += item_height + 5;
                }
            }
        }
    }
    
    // Draw buttons
    ui_button_render(btn_create, state->renderer);
    ui_button_render(btn_load, state->renderer);
    ui_button_render(btn_save, state->renderer);
    ui_button_render(btn_delete, state->renderer);
    
    // Draw status message
    if (status_message_timer > 0.0f && status_message[0] != '\0') {
        extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
        SDL_Color status_color = {255, 255, 100, 255};
        draw_text(state->renderer, status_message, 20, 950, status_color);
    }
    
    // Draw create dialog if open
    if (show_create_dialog) {
        // Draw semi-transparent overlay
        SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 128);
        SDL_Rect overlay = {0, 0, 1920, 1080};
        SDL_RenderFillRect(state->renderer, &overlay);
        
        // Draw dialog background
        SDL_Rect dialog_rect = {400, 200, 600, 600};
        SDL_SetRenderDrawColor(state->renderer, 40, 40, 40, 255);
        SDL_RenderFillRect(state->renderer, &dialog_rect);
        SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
        SDL_RenderDrawRect(state->renderer, &dialog_rect);
        
        // Draw title
        extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
        SDL_Color title_color = {200, 220, 255, 255};
        draw_text(state->renderer, "Create New Model", dialog_rect.x + 20, dialog_rect.y + 20, title_color);
        
        // Position and render inputs
        int input_x = dialog_rect.x + 50;
        int input_y = dialog_rect.y + 70;
        int input_spacing = 60;
        
        SDL_Color label_color = {180, 180, 180, 255};
        
        draw_text(state->renderer, "Model Name:", input_x, input_y - 20, label_color);
        ui_text_input_set_focus(input_model_name, true);
        input_model_name->bounds = (SDL_Rect){input_x, input_y, 500, 30};
        ui_text_input_render(input_model_name, state->renderer);
        input_y += input_spacing;
        
        slider_vocab_size->bounds = (SDL_Rect){input_x, input_y, 400, 30};
        ui_slider_render(slider_vocab_size, state->renderer);
        input_y += input_spacing;
        
        slider_embedding_dim->bounds = (SDL_Rect){input_x, input_y, 400, 30};
        ui_slider_render(slider_embedding_dim, state->renderer);
        input_y += input_spacing;
        
        slider_num_layers->bounds = (SDL_Rect){input_x, input_y, 400, 30};
        ui_slider_render(slider_num_layers, state->renderer);
        input_y += input_spacing;
        
        slider_num_heads->bounds = (SDL_Rect){input_x, input_y, 400, 30};
        ui_slider_render(slider_num_heads, state->renderer);
        input_y += input_spacing;
        
        slider_hidden_dim->bounds = (SDL_Rect){input_x, input_y, 400, 30};
        ui_slider_render(slider_hidden_dim, state->renderer);
        input_y += input_spacing + 20;
        
        // Buttons
        btn_create_confirm->bounds = (SDL_Rect){input_x, input_y, 120, 35};
        btn_create_cancel->bounds = (SDL_Rect){input_x + 140, input_y, 120, 35};
        ui_button_render(btn_create_confirm, state->renderer);
        ui_button_render(btn_create_cancel, state->renderer);
    }
    
    // Draw delete confirmation dialog
    ui_dialog_render(dialog_delete_confirm, state->renderer);
}

// ============================================================================
// EVENT HANDLING
// ============================================================================

void handle_models_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    SDL_Event fake_event;
    fake_event.type = SDL_MOUSEBUTTONDOWN;
    fake_event.button.button = SDL_BUTTON_LEFT;
    fake_event.button.x = x;
    fake_event.button.y = y;
    
    // Handle dialog events first (if visible)
    if (ui_dialog_is_visible(dialog_delete_confirm)) {
        ui_dialog_handle_event(dialog_delete_confirm, &fake_event);
        return;
    }
    
    // Handle create dialog events
    if (show_create_dialog) {
        ui_text_input_handle_event(input_model_name, &fake_event);
        ui_slider_handle_event(slider_vocab_size, &fake_event);
        ui_slider_handle_event(slider_embedding_dim, &fake_event);
        ui_slider_handle_event(slider_num_layers, &fake_event);
        ui_slider_handle_event(slider_num_heads, &fake_event);
        ui_slider_handle_event(slider_hidden_dim, &fake_event);
        ui_button_handle_event(btn_create_confirm, &fake_event);
        ui_button_handle_event(btn_create_cancel, &fake_event);
        return;
    }
    
    // Handle button clicks
    ui_button_handle_event(btn_create, &fake_event);
    ui_button_handle_event(btn_load, &fake_event);
    ui_button_handle_event(btn_save, &fake_event);
    ui_button_handle_event(btn_delete, &fake_event);
    
    // Handle model selection in list
    SDL_Rect content = ui_panel_get_content_bounds(panel_model_list);
    if (ui_point_in_rect(x, y, &content)) {
        uint32_t model_count = 0;
        ManagedModel** models = model_manager_list(&model_count);
        
        int item_height = 70;
        int relative_y = y - content.y;
        int clicked_index = relative_y / (item_height + 5);
        
        if (clicked_index >= 0 && clicked_index < (int)model_count) {
            // Check for double-click
            uint32_t current_time = SDL_GetTicks();
            bool is_double_click = (clicked_index == last_clicked_index) && 
                                   ((current_time - last_click_time) < DOUBLE_CLICK_MS);
            
            last_clicked_index = clicked_index;
            last_click_time = current_time;
            
            selected_model_index = clicked_index;
            ui_button_set_enabled(btn_delete, true);
            
            // Update state manager
            if (models[clicked_index]) {
                StateManager* state_mgr = state_manager_get_instance();
                state_set_model(state_mgr, NULL, models[clicked_index]->name, "");
                
                // Double-click loads the model
                if (is_double_click) {
                    printf("Double-click detected - loading model '%s'\n", models[clicked_index]->name);
                    
                    // Load the model (same as Load button)
                       // Reload model if not already loaded
                       model_manager_reload(models[clicked_index]->name);

                    CLLMModel* loaded_model = model_manager_acquire_read(models[clicked_index]->name);
                    if (loaded_model) {
                        // Release old model if any
                        if (state->cllm_model) {
                            // TODO: Get old model name to release properly
                        }
                        
                        state->cllm_model = loaded_model;
                        
                        // Show success message (truncate model name if needed)
                        char safe_name[200];
                        snprintf(safe_name, sizeof(safe_name), "%.190s", models[clicked_index]->name);
                        snprintf(status_message, sizeof(status_message), 
                                "Model '%s' loaded successfully", safe_name);
                        status_message_timer = 3.0f;
                        
                        // Dispatch event
                        EventSystem* evt_sys = event_system_get_instance();
                        if (evt_sys) {
                            event_dispatch(evt_sys, EVENT_MODEL_LOADED, 
                                         (void*)models[clicked_index]->name, 
                                         strlen(models[clicked_index]->name) + 1, 
                                         "tab_models");
                        }
                    } else {
                        // Show error message (truncate model name if needed)
                        char safe_name[200];
                        snprintf(safe_name, sizeof(safe_name), "%.190s", models[clicked_index]->name);
                        snprintf(status_message, sizeof(status_message), 
                                "Failed to load model '%s'", safe_name);
                        status_message_timer = 3.0f;
                    }
                }
            }
        }
    }
}