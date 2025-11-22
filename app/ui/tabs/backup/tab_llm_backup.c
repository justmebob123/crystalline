// app/ui/tabs/tab_llm.c - LLM Interaction Tab (Simplified)
#include "../../app_common.h"

void draw_llm_tab(SDL_Renderer* renderer, AppState* state) {
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
    draw_text(renderer, "CLLM Interface", panel_x + 20, y, text_color);
    y += 40;
    
    // Model status
    const char* status_text = state->cllm_model ? "Model: Loaded" : "Model: Not Loaded";
    SDL_Color status_color = state->cllm_model ? (SDL_Color){100, 255, 100, 255} : (SDL_Color){255, 100, 100, 255};
    draw_text(renderer, status_text, panel_x + 20, y, status_color);
    y += 30;
    
    // Load Model Button
    SDL_Rect load_button = {panel_x + 20, y, panel_width - 40, 30};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &load_button);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &load_button);
    draw_text(renderer, "Load Model", load_button.x + 80, load_button.y + 8, text_color);
    y += 50;
    
    // Input section
    draw_text(renderer, "Input:", panel_x + 20, y, text_color);
    y += 25;
    
    // Input box
    SDL_Rect input_box = {panel_x + 20, y, panel_width - 40, 80};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &input_box);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &input_box);
    if (strlen(state->llm_input_text) > 0) {
        draw_text(renderer, state->llm_input_text, input_box.x + 5, input_box.y + 5, text_color);
    }
    y += 90;
    
    // Temperature
    char temp_label[64];
    snprintf(temp_label, sizeof(temp_label), "Temperature: %.2f", state->llm_temperature);
    draw_text(renderer, temp_label, panel_x + 20, y, text_color);
    y += 55;
    
    // Generate Button
    SDL_Rect gen_button = {panel_x + 20, y, panel_width - 40, 35};
    SDL_Color gen_color = state->llm_generating ? (SDL_Color){100, 100, 120, 255} : (SDL_Color){80, 150, 80, 255};
    SDL_SetRenderDrawColor(renderer, gen_color.r, gen_color.g, gen_color.b, 255);
    SDL_RenderFillRect(renderer, &gen_button);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &gen_button);
    const char* gen_text = state->llm_generating ? "Generating..." : "Generate";
    draw_text(renderer, gen_text, gen_button.x + 100, gen_button.y + 10, text_color);
    y += 50;
    
    // Output section
    draw_text(renderer, "Output:", panel_x + 20, y, text_color);
    y += 25;
    
    // Output box
    int output_height = WINDOW_HEIGHT - y - 20;
    SDL_Rect output_box = {panel_x + 20, y, panel_width - 40, output_height};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &output_box);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &output_box);
    if (strlen(state->llm_output_text) > 0) {
        draw_text(renderer, state->llm_output_text, output_box.x + 5, output_box.y + 5, text_color);
    }
    
    // Draw visualization area
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_Rect viz_rect = {0, 60, RENDER_WIDTH, WINDOW_HEIGHT - 60};
    SDL_RenderFillRect(renderer, &viz_rect);
    
    if (state->cllm_model && state->cllm_model->num_lattice_points > 0) {
        draw_text(renderer, "Crystalline Lattice Visualization", 20, 80, text_color);
        
        int cx = RENDER_WIDTH / 2;
        int cy = (WINDOW_HEIGHT - 60) / 2 + 60;
        
        // Draw lattice points
        for (uint32_t i = 0; i < state->cllm_model->num_lattice_points && i < 1000; i++) {
            LatticePoint* pt = &state->cllm_model->lattice_points[i];
            
            int x = cx + (int)(pt->coords[0] * 200);
            int y_pos = cy + (int)(pt->coords[1] * 200);
            
            uint8_t r = (uint8_t)((pt->prime_factor * 137) % 255);
            uint8_t g = (uint8_t)((pt->prime_factor * 211) % 255);
            uint8_t b = (uint8_t)((pt->prime_factor * 173) % 255);
            
            SDL_SetRenderDrawColor(renderer, r, g, b, 200);
            SDL_Rect point = {x - 2, y_pos - 2, 4, 4};
            SDL_RenderFillRect(renderer, &point);
        }
    } else {
        draw_text(renderer, "Load a CLLM model to begin", RENDER_WIDTH / 2 - 100, WINDOW_HEIGHT / 2, text_color);
    }
}

void handle_llm_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    int panel_x = RENDER_WIDTH;
    
    // Load Model Button (y = 90)
    SDL_Rect load_button = {panel_x + 20, 90, CONTROL_PANEL_WIDTH - 40, 30};
    if (x >= load_button.x && x <= load_button.x + load_button.w &&
        y >= load_button.y && y <= load_button.y + load_button.h) {
        
        if (!state->cllm_model) {
            printf("Attempting to load CLLM model...\n");
            state->cllm_model = cllm_read("models/demo_model.cllm");
            if (state->cllm_model) {
                state->cllm_inference = cllm_inference_init(state->cllm_model);
                printf("Model loaded successfully!\n");
                strcpy(state->llm_output_text, "Model loaded. Ready for inference.");
            } else {
                printf("No model file found.\n");
                strcpy(state->llm_output_text, "No model found. Train a model in the Training tab first.");
            }
        }
    }
    
    // Generate Button (y = 280)
    SDL_Rect gen_button = {panel_x + 20, 280, CONTROL_PANEL_WIDTH - 40, 35};
    if (x >= gen_button.x && x <= gen_button.x + gen_button.w &&
        y >= gen_button.y && y <= gen_button.y + gen_button.h) {
        
        if (state->cllm_inference && !state->llm_generating) {
            state->llm_generating = true;
            
            if (strlen(state->llm_input_text) > 0) {
                snprintf(state->llm_output_text, sizeof(state->llm_output_text),
                        "Generated response to: '%s'\n\nThis is a demo response.",
                        state->llm_input_text);
            } else {
                strcpy(state->llm_output_text, "Please enter input text first.");
            }
            
            state->llm_generating = false;
        }
    }
}