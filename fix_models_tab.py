#!/usr/bin/env python3
"""Fix Models Tab Implementation - Complete all TODOs"""

# Read the file
with open('app/ui/tabs/tab_models.c', 'r') as f:
    content = f.read()

# Fix 1: Implement create dialog text rendering
content = content.replace(
    '       // Title\n       // TODO: Render "Create New Model" title\n       current_y += 40;',
    '''       // Title
       SDL_Color title_color = {220, 220, 255, 255};
       extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
       draw_text(state->renderer, "Create New Model", x + PANEL_PADDING, current_y, title_color);
       current_y += 40;'''
)

content = content.replace(
    '       // Model name input\n       // TODO: Render "Model Name:" label\n       // TODO: Render text input field\n       current_y += 50;',
    '''       // Model name input
       SDL_Color label_color = {180, 180, 180, 255};
       draw_text(state->renderer, "Model Name:", x + PANEL_PADDING, current_y, label_color);
       current_y += 20;
       
       // Input field background
       SDL_Rect name_input = {x + PANEL_PADDING, current_y, width - 2 * PANEL_PADDING, 25};
       SDL_SetRenderDrawColor(state->renderer, 60, 60, 60, 255);
       SDL_RenderFillRect(state->renderer, &name_input);
       SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
       SDL_RenderDrawRect(state->renderer, &name_input);
       
       // Input text
       SDL_Color input_color = {220, 220, 220, 255};
       draw_text(state->renderer, g_models_state.create_model_name, 
                name_input.x + 5, name_input.y + 5, input_color);
       current_y += 30;'''
)

content = content.replace(
    '       // Vocabulary size input\n       // TODO: Render "Vocabulary Size:" label\n       // TODO: Render number input field\n       current_y += 50;',
    '''       // Vocabulary size input
       draw_text(state->renderer, "Vocabulary Size:", x + PANEL_PADDING, current_y, label_color);
       current_y += 20;
       
       SDL_Rect vocab_input = {x + PANEL_PADDING, current_y, width - 2 * PANEL_PADDING, 25};
       SDL_SetRenderDrawColor(state->renderer, 60, 60, 60, 255);
       SDL_RenderFillRect(state->renderer, &vocab_input);
       SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
       SDL_RenderDrawRect(state->renderer, &vocab_input);
       
       char vocab_str[32];
       snprintf(vocab_str, sizeof(vocab_str), "%u", g_models_state.create_vocab_size);
       draw_text(state->renderer, vocab_str, vocab_input.x + 5, vocab_input.y + 5, input_color);
       current_y += 30;'''
)

content = content.replace(
    '       // Embedding dimension input\n       // TODO: Render "Embedding Dimension:" label\n       // TODO: Render number input field\n       current_y += 50;',
    '''       // Embedding dimension input
       draw_text(state->renderer, "Embedding Dimension:", x + PANEL_PADDING, current_y, label_color);
       current_y += 20;
       
       SDL_Rect embed_input = {x + PANEL_PADDING, current_y, width - 2 * PANEL_PADDING, 25};
       SDL_SetRenderDrawColor(state->renderer, 60, 60, 60, 255);
       SDL_RenderFillRect(state->renderer, &embed_input);
       SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
       SDL_RenderDrawRect(state->renderer, &embed_input);
       
       char embed_str[32];
       snprintf(embed_str, sizeof(embed_str), "%u", g_models_state.create_embedding_dim);
       draw_text(state->renderer, embed_str, embed_input.x + 5, embed_input.y + 5, input_color);
       current_y += 30;'''
)

content = content.replace(
    '       // Number of layers input\n       // TODO: Render "Number of Layers:" label\n       // TODO: Render number input field\n       current_y += 50;',
    '''       // Number of layers input
       draw_text(state->renderer, "Number of Layers:", x + PANEL_PADDING, current_y, label_color);
       current_y += 20;
       
       SDL_Rect layers_input = {x + PANEL_PADDING, current_y, width - 2 * PANEL_PADDING, 25};
       SDL_SetRenderDrawColor(state->renderer, 60, 60, 60, 255);
       SDL_RenderFillRect(state->renderer, &layers_input);
       SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
       SDL_RenderDrawRect(state->renderer, &layers_input);
       
       char layers_str[32];
       snprintf(layers_str, sizeof(layers_str), "%u", g_models_state.create_num_layers);
       draw_text(state->renderer, layers_str, layers_input.x + 5, layers_input.y + 5, input_color);
       current_y += 30;'''
)

content = content.replace(
    '       // Number of heads input\n       // TODO: Render "Number of Heads:" label\n       // TODO: Render number input field\n       current_y += 50;',
    '''       // Number of heads input
       draw_text(state->renderer, "Number of Heads:", x + PANEL_PADDING, current_y, label_color);
       current_y += 20;
       
       SDL_Rect heads_input = {x + PANEL_PADDING, current_y, width - 2 * PANEL_PADDING, 25};
       SDL_SetRenderDrawColor(state->renderer, 60, 60, 60, 255);
       SDL_RenderFillRect(state->renderer, &heads_input);
       SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
       SDL_RenderDrawRect(state->renderer, &heads_input);
       
       char heads_str[32];
       snprintf(heads_str, sizeof(heads_str), "%u", g_models_state.create_num_heads);
       draw_text(state->renderer, heads_str, heads_input.x + 5, heads_input.y + 5, input_color);
       current_y += 30;'''
)

content = content.replace(
    '       // Create button\n       SDL_SetRenderDrawColor(state->renderer, 0, 150, 0, 255);\n       SDL_RenderFillRect(state->renderer, &create_button);\n       // TODO: Render "Create" text',
    '''       // Create button
       SDL_SetRenderDrawColor(state->renderer, 0, 150, 0, 255);
       SDL_RenderFillRect(state->renderer, &create_button);
       SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
       SDL_RenderDrawRect(state->renderer, &create_button);
       SDL_Color btn_text = {255, 255, 255, 255};
       draw_text(state->renderer, "Create", create_button.x + 30, create_button.y + 8, btn_text);'''
)

content = content.replace(
    '       // Cancel button\n       SDL_SetRenderDrawColor(state->renderer, 150, 0, 0, 255);\n       SDL_RenderFillRect(state->renderer, &cancel_button);\n       // TODO: Render "Cancel" text',
    '''       // Cancel button
       SDL_SetRenderDrawColor(state->renderer, 150, 0, 0, 255);
       SDL_RenderFillRect(state->renderer, &cancel_button);
       SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
       SDL_RenderDrawRect(state->renderer, &cancel_button);
       draw_text(state->renderer, "Cancel", cancel_button.x + 30, cancel_button.y + 8, btn_text);'''
)

# Fix 2: Implement model details panel text rendering
content = content.replace(
    '       if (g_models_state.selected_model_index < 0) {\n           // No model selected message\n           // TODO: Render "Select a model to view details"\n           return;\n       }',
    '''       if (g_models_state.selected_model_index < 0) {
           // No model selected message
           SDL_Color msg_color = {150, 150, 150, 255};
           extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
           draw_text(state->renderer, "Select a model to view details", 
                    x + PANEL_PADDING, y + PANEL_PADDING + 40, msg_color);
           return;
       }'''
)

content = content.replace(
    '       int current_y = y + PANEL_PADDING;\n       \n       // Model details\n       // TODO: Render model name\n       current_y += 40;\n       \n       // TODO: Render vocabulary size\n       current_y += 30;\n       \n       // TODO: Render embedding dimension\n       current_y += 30;\n       \n       // TODO: Render number of layers\n       current_y += 30;\n       \n       // TODO: Render number of heads\n       current_y += 30;\n       \n       // TODO: Render model status\n       current_y += 30;\n       \n       // TODO: Render file path\n       current_y += 30;',
    '''       int current_y = y + PANEL_PADDING;
       
       SDL_Color title_color = {220, 220, 255, 255};
       SDL_Color label_color = {180, 180, 180, 255};
       SDL_Color value_color = {220, 220, 220, 255};
       extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
       
       // Model details
       draw_text(state->renderer, "Model Details", x + PANEL_PADDING, current_y, title_color);
       current_y += 40;
       
       // Model name
       draw_text(state->renderer, "Name:", x + PANEL_PADDING, current_y, label_color);
       draw_text(state->renderer, model->name, x + PANEL_PADDING + 150, current_y, value_color);
       current_y += 30;
       
       // Vocabulary size
       char vocab_info[64];
       snprintf(vocab_info, sizeof(vocab_info), "%u", model->vocab_size);
       draw_text(state->renderer, "Vocabulary Size:", x + PANEL_PADDING, current_y, label_color);
       draw_text(state->renderer, vocab_info, x + PANEL_PADDING + 150, current_y, value_color);
       current_y += 30;
       
       // Embedding dimension
       char embed_info[64];
       snprintf(embed_info, sizeof(embed_info), "%u", model->embedding_dim);
       draw_text(state->renderer, "Embedding Dim:", x + PANEL_PADDING, current_y, label_color);
       draw_text(state->renderer, embed_info, x + PANEL_PADDING + 150, current_y, value_color);
       current_y += 30;
       
       // Number of layers
       char layers_info[64];
       snprintf(layers_info, sizeof(layers_info), "%u", model->num_layers);
       draw_text(state->renderer, "Layers:", x + PANEL_PADDING, current_y, label_color);
       draw_text(state->renderer, layers_info, x + PANEL_PADDING + 150, current_y, value_color);
       current_y += 30;
       
       // Number of heads
       char heads_info[64];
       snprintf(heads_info, sizeof(heads_info), "%u", model->num_heads);
       draw_text(state->renderer, "Attention Heads:", x + PANEL_PADDING, current_y, label_color);
       draw_text(state->renderer, heads_info, x + PANEL_PADDING + 150, current_y, value_color);
       current_y += 30;
       
       // Model status
       const char* status = model->read_count > 0 ? "In Use" : "Idle";
       SDL_Color status_color = model->read_count > 0 ? 
           (SDL_Color){100, 255, 100, 255} : (SDL_Color){150, 150, 150, 255};
       draw_text(state->renderer, "Status:", x + PANEL_PADDING, current_y, label_color);
       draw_text(state->renderer, status, x + PANEL_PADDING + 150, current_y, status_color);
       current_y += 30;
       
       // File path (truncate if too long)
       char path_display[256];
       if (strlen(model->name) > 40) {
           snprintf(path_display, sizeof(path_display), "models/%.37s...", model->name);
       } else {
           snprintf(path_display, sizeof(path_display), "models/%s.cllm", model->name);
       }
       draw_text(state->renderer, "File:", x + PANEL_PADDING, current_y, label_color);
       draw_text(state->renderer, path_display, x + PANEL_PADDING + 150, current_y, value_color);
       current_y += 30;'''
)

content = content.replace(
    '       SDL_Rect export_button = {x + PANEL_PADDING, current_y, 120, BUTTON_HEIGHT};\n       SDL_SetRenderDrawColor(state->renderer, 0, 100, 150, 255);\n       SDL_RenderFillRect(state->renderer, &export_button);\n       // TODO: Render "Export" text\n       \n       SDL_Rect save_button = {x + PANEL_PADDING + 130, current_y, 120, BUTTON_HEIGHT};\n       SDL_SetRenderDrawColor(state->renderer, 0, 150, 0, 255);\n       SDL_RenderFillRect(state->renderer, &save_button);\n       // TODO: Render "Save" text',
    '''       SDL_Rect export_button = {x + PANEL_PADDING, current_y, 120, BUTTON_HEIGHT};
       SDL_SetRenderDrawColor(state->renderer, 0, 100, 150, 255);
       SDL_RenderFillRect(state->renderer, &export_button);
       SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
       SDL_RenderDrawRect(state->renderer, &export_button);
       SDL_Color btn_text = {255, 255, 255, 255};
       draw_text(state->renderer, "Export", export_button.x + 35, export_button.y + 8, btn_text);
       
       SDL_Rect save_button = {x + PANEL_PADDING + 130, current_y, 120, BUTTON_HEIGHT};
       SDL_SetRenderDrawColor(state->renderer, 0, 150, 0, 255);
       SDL_RenderFillRect(state->renderer, &save_button);
       SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
       SDL_RenderDrawRect(state->renderer, &save_button);
       draw_text(state->renderer, "Save", save_button.x + 40, save_button.y + 8, btn_text);'''
)

# Fix 3: Implement status message rendering
content = content.replace(
    '       // Status bar\n       SDL_Rect status_rect = {x, y, width, 40};\n       SDL_SetRenderDrawColor(state->renderer, 50, 50, 100, 255);\n       SDL_RenderFillRect(state->renderer, &status_rect);\n       \n       // TODO: Render status message text',
    '''       // Status bar
       SDL_Rect status_rect = {x, y, width, 40};
       SDL_SetRenderDrawColor(state->renderer, 50, 50, 100, 255);
       SDL_RenderFillRect(state->renderer, &status_rect);
       SDL_SetRenderDrawColor(state->renderer, 100, 100, 120, 255);
       SDL_RenderDrawRect(state->renderer, &status_rect);
       
       // Render status message text
       SDL_Color msg_color = {220, 220, 255, 255};
       extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
       draw_text(state->renderer, g_models_state.status_message, 
                status_rect.x + 10, status_rect.y + 12, msg_color);'''
)

# Write the fixed content
with open('app/ui/tabs/tab_models.c', 'w') as f:
    f.write(content)

print("✅ Fixed Models tab rendering:")
print("  1. Implemented create dialog text rendering (all fields)")
print("  2. Implemented model details panel text rendering")
print("  3. Implemented status message rendering")
print("  4. Added proper borders and styling")
print("  5. All TODOs for rendering are now complete")
print("\nNote: Click handlers still need implementation (separate task)")