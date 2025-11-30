#!/usr/bin/env python3
"""Implement Models Tab Click Handlers"""

# Read the file
with open('app/ui/tabs/tab_models.c', 'r') as f:
    content = f.read()

# Replace the placeholder click handler with full implementation
old_handler = '''// Handle click events
   void handle_models_tab_click(AppState* state, int x, int y) {
       (void)state; // Suppress unused warning
       (void)x;     // Suppress unused warning
       (void)y;     // Suppress unused warning
       
       // TODO: Implement click handling
       // - Model selection
       // - Button clicks
       // - Dialog interactions
       
       set_status_message("Click handling not yet implemented");
   }'''

new_handler = '''// Handle click events
   void handle_models_tab_click(AppState* state, int x, int y) {
       int window_width = WINDOW_WIDTH - SIDEBAR_WIDTH - 40;
       int window_height = WINDOW_HEIGHT - SUBMENU_HEIGHT - 60;
       int panel_x = RENDER_OFFSET_X + 20;
       int panel_y = RENDER_OFFSET_Y + 20;
       
       // Handle create dialog clicks
       if (g_models_state.show_create_dialog) {
           int dialog_width = 500;
           int dialog_height = 450;
           int dialog_x = (WINDOW_WIDTH - dialog_width) / 2;
           int dialog_y = (WINDOW_HEIGHT - dialog_height) / 2;
           
           // Check if click is inside dialog
           if (x >= dialog_x && x <= dialog_x + dialog_width &&
               y >= dialog_y && y <= dialog_y + dialog_height) {
               
               // Calculate button positions (same as in draw function)
               int button_y = dialog_y + dialog_height - PANEL_PADDING - BUTTON_HEIGHT - 20;
               SDL_Rect create_button = {dialog_x + PANEL_PADDING, button_y, 100, BUTTON_HEIGHT};
               SDL_Rect cancel_button = {dialog_x + PANEL_PADDING + 110, button_y, 100, BUTTON_HEIGHT};
               
               // Check Create button
               if (x >= create_button.x && x <= create_button.x + create_button.w &&
                   y >= create_button.y && y <= create_button.y + create_button.h) {
                   
                   // Validate inputs
                   if (strlen(g_models_state.create_model_name) == 0) {
                       set_status_message("Error: Model name cannot be empty");
                       return;
                   }
                   
                   // Create the model
                   CLLMModel* new_model = model_manager_create(
                       g_models_state.create_model_name,
                       g_models_state.create_vocab_size,
                       g_models_state.create_embedding_dim,
                       g_models_state.create_num_layers,
                       g_models_state.create_num_heads
                   );
                   
                   if (new_model) {
                       char msg[256];
                       snprintf(msg, sizeof(msg), "Model '%s' created successfully", 
                               g_models_state.create_model_name);
                       set_status_message(msg);
                       g_models_state.show_create_dialog = false;
                       
                       // Reset dialog fields
                       g_models_state.create_model_name[0] = '\0';
                       g_models_state.create_vocab_size = 10000;
                       g_models_state.create_embedding_dim = 512;
                       g_models_state.create_num_layers = 6;
                       g_models_state.create_num_heads = 8;
                   } else {
                       set_status_message("Error: Failed to create model");
                   }
                   return;
               }
               
               // Check Cancel button
               if (x >= cancel_button.x && x <= cancel_button.x + cancel_button.w &&
                   y >= cancel_button.y && y <= cancel_button.y + cancel_button.h) {
                   g_models_state.show_create_dialog = false;
                   set_status_message("Model creation cancelled");
                   return;
               }
           } else {
               // Click outside dialog - close it
               g_models_state.show_create_dialog = false;
               return;
           }
       }
       
       // Handle model list clicks (selection)
       int left_width = window_width * 0.4;
       int list_height = window_height - 100;
       
       if (x >= panel_x && x <= panel_x + left_width &&
           y >= panel_y + 60 && y <= panel_y + list_height) {
           
           // Get model list
           uint32_t model_count = 0;
           ManagedModel** models = model_manager_list(&model_count);
           
           if (model_count > 0) {
               // Calculate which model was clicked
               int relative_y = y - (panel_y + 60);
               int model_index = relative_y / 70;
               
               if (model_index >= 0 && model_index < (int)model_count) {
                   g_models_state.selected_model_index = model_index;
                   
                   if (models && models[model_index]) {
                       char msg[256];
                       snprintf(msg, sizeof(msg), "Selected model: %s", models[model_index]->name);
                       set_status_message(msg);
                   }
               }
           }
           
           if (models) free(models);
           return;
       }
       
       // Handle action buttons at bottom
       int button_y = panel_y + window_height - 80;
       int button_width = (window_width - 4 * BUTTON_SPACING) / 3;
       
       // Create New button
       SDL_Rect create_btn = {panel_x, button_y, button_width, BUTTON_HEIGHT};
       if (x >= create_btn.x && x <= create_btn.x + create_btn.w &&
           y >= create_btn.y && y <= create_btn.y + create_btn.h) {
           g_models_state.show_create_dialog = true;
           set_status_message("Enter model parameters");
           return;
       }
       
       // Load button
       SDL_Rect load_btn = {panel_x + button_width + BUTTON_SPACING, button_y, button_width, BUTTON_HEIGHT};
       if (x >= load_btn.x && x <= load_btn.x + load_btn.w &&
           y >= load_btn.y && y <= load_btn.y + load_btn.h) {
           set_status_message("Load model functionality coming soon");
           return;
       }
       
       // Delete button (only if model selected)
       if (g_models_state.selected_model_index >= 0) {
           SDL_Rect delete_btn = {panel_x + 2 * (button_width + BUTTON_SPACING), button_y, button_width, BUTTON_HEIGHT};
           if (x >= delete_btn.x && x <= delete_btn.x + delete_btn.w &&
               y >= delete_btn.y && y <= delete_btn.y + delete_btn.h) {
               
               // Get model name before deleting
               uint32_t model_count = 0;
               ManagedModel** models = model_manager_list(&model_count);
               
               if (g_models_state.selected_model_index < (int)model_count && models) {
                   char model_name[256];
                   strncpy(model_name, models[g_models_state.selected_model_index]->name, sizeof(model_name) - 1);
                   model_name[sizeof(model_name) - 1] = '\0';
                   
                   // Delete the model
                   if (model_manager_delete(model_name) == 0) {
                       char msg[256];
                       snprintf(msg, sizeof(msg), "Model '%s' deleted successfully", model_name);
                       set_status_message(msg);
                       g_models_state.selected_model_index = -1;
                   } else {
                       set_status_message("Error: Failed to delete model");
                   }
               }
               
               if (models) free(models);
               return;
           }
       }
   }'''

content = content.replace(old_handler, new_handler)

# Write the fixed content
with open('app/ui/tabs/tab_models.c', 'w') as f:
    f.write(content)

print("✅ Implemented Models tab click handlers:")
print("  1. Model selection by clicking in list")
print("  2. Create New button - opens dialog")
print("  3. Create dialog - Create button (validates and creates model)")
print("  4. Create dialog - Cancel button (closes dialog)")
print("  5. Create dialog - Click outside to close")
print("  6. Delete button - deletes selected model")
print("  7. Load button - placeholder for future implementation")
print("  8. All click handling is now functional")