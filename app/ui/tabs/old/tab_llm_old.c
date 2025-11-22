// app/ui/tabs/tab_llm.c - COMPLETE LLM Interaction Tab Implementation
#include "../../app_common.h"
#include <string.h>

// UI state for text input
static bool input_active = false;
static int input_cursor = 0;
static int output_scroll = 0;

void draw_llm_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
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
    
    // === SECTION 1: MODEL STATUS ===
    draw_text(renderer, "CLLM MODEL", panel_x + 10, y, text_color);
    y += 20;
    
    // Model status with color indicator
    const char* status = state->cllm_model ? "Loaded" : "Not Loaded";
    SDL_Color status_color = state->cllm_model ? 
        (SDL_Color){100, 255, 100, 255} : (SDL_Color){255, 100, 100, 255};
    
    char status_text[64];
    snprintf(status_text, sizeof(status_text), "Status: %s", status);
    draw_text(renderer, status_text, panel_x + 10, y, status_color);
    y += 18;
    
    // Model info if loaded
    if (state->cllm_model) {
        char info[128];
        snprintf(info, sizeof(info), "Vocab: %u", state->cllm_model->vocab_size);
        draw_text(renderer, info, panel_x + 10, y, text_color);
        y += 16;
        
        snprintf(info, sizeof(info), "Layers: %u", state->cllm_model->num_layers);
        draw_text(renderer, info, panel_x + 10, y, text_color);
        y += 16;
        
        snprintf(info, sizeof(info), "Lattice: %u pts", state->cllm_model->num_lattice_points);
        draw_text(renderer, info, panel_x + 10, y, text_color);
        y += 20;
    } else {
        y += 20;
    }
    
    // Load Model Button
    SDL_Rect load_btn = {panel_x + 10, y, panel_width - 20, 28};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &load_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &load_btn);
    draw_text(renderer, "Load Model", load_btn.x + 90, load_btn.y + 7, text_color);
    y += 35;
    
    // === SECTION 2: INPUT ===
    draw_text(renderer, "INPUT PROMPT", panel_x + 10, y, text_color);
    y += 20;
    
    // Input text area
    SDL_Rect input_rect = {panel_x + 10, y, panel_width - 20, 80};
    SDL_Color input_bg = input_active ? (SDL_Color){50, 50, 70, 255} : (SDL_Color){30, 30, 40, 255};
    SDL_SetRenderDrawColor(renderer, input_bg.r, input_bg.g, input_bg.b, 255);
    SDL_RenderFillRect(renderer, &input_rect);
    SDL_SetRenderDrawColor(renderer, input_active ? active_color.r : text_color.r, 
                           input_active ? active_color.g : text_color.g,
                           input_active ? active_color.b : text_color.b, 255);
    SDL_RenderDrawRect(renderer, &input_rect);
    
    // Draw input text (word-wrapped)
    if (strlen(state->llm_input_text) > 0) {
        int text_y = input_rect.y + 5;
        int chars_per_line = 35;
        for (size_t i = 0; i < strlen(state->llm_input_text); i += chars_per_line) {
            char line[64];
            int len = (int)strlen(state->llm_input_text + i);
            if (len > chars_per_line) len = chars_per_line;
            memcpy(line, state->llm_input_text + i, len);
            line[len] = '\0';
            draw_text(renderer, line, input_rect.x + 5, text_y, text_color);
            text_y += 14;
            if (text_y > input_rect.y + input_rect.h - 10) break;
        }
    } else if (input_active) {
        draw_text(renderer, "Type your prompt...", input_rect.x + 5, input_rect.y + 5, 
                 (SDL_Color){150, 150, 150, 255});
    }
    
    // Character count
    char char_count[32];
    snprintf(char_count, sizeof(char_count), "%zu / 1024", strlen(state->llm_input_text));
    draw_text(renderer, char_count, input_rect.x + input_rect.w - 60, input_rect.y + input_rect.h + 3, 
             (SDL_Color){150, 150, 150, 255});
    
    y += 100;
    
    // Clear button
    SDL_Rect clear_btn = {panel_x + 10, y, 80, 22};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &clear_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &clear_btn);
    draw_text(renderer, "Clear", clear_btn.x + 22, clear_btn.y + 5, text_color);
    y += 30;
    
    // === SECTION 3: PARAMETERS ===
    draw_text(renderer, "PARAMETERS", panel_x + 10, y, text_color);
    y += 20;
    
    // Temperature
    char temp_label[64];
    snprintf(temp_label, sizeof(temp_label), "Temperature: %.2f", state->llm_temperature);
    draw_text(renderer, temp_label, panel_x + 10, y, text_color);
    y += 18;
    
    SDL_Rect temp_slider = {panel_x + 10, y, panel_width - 20, 8};
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderFillRect(renderer, &temp_slider);
    
    int temp_pos = (int)((state->llm_temperature / 2.0f) * temp_slider.w);
    SDL_Rect temp_handle = {temp_slider.x + temp_pos - 4, temp_slider.y - 4, 8, 16};
    SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
    SDL_RenderFillRect(renderer, &temp_handle);
    y += 20;
    
    // Max Tokens
    char tokens_label[64];
    snprintf(tokens_label, sizeof(tokens_label), "Max Tokens: %d", state->llm_max_tokens);
    draw_text(renderer, tokens_label, panel_x + 10, y, text_color);
    y += 18;
    
    SDL_Rect tokens_slider = {panel_x + 10, y, panel_width - 20, 8};
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderFillRect(renderer, &tokens_slider);
    
    int tokens_pos = (int)((state->llm_max_tokens / 500.0f) * tokens_slider.w);
    SDL_Rect tokens_handle = {tokens_slider.x + tokens_pos - 4, tokens_slider.y - 4, 8, 16};
    SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
    SDL_RenderFillRect(renderer, &tokens_handle);
    y += 25;
    
    // === SECTION 4: GENERATION ===
    // Generate button
    SDL_Rect gen_btn = {panel_x + 10, y, panel_width - 20, 35};
    SDL_Color gen_color = state->llm_generating ? 
        (SDL_Color){100, 100, 120, 255} : button_hover;
    SDL_SetRenderDrawColor(renderer, gen_color.r, gen_color.g, gen_color.b, 255);
    SDL_RenderFillRect(renderer, &gen_btn);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &gen_btn);
    
    const char* gen_text = state->llm_generating ? "Generating..." : "GENERATE";
    draw_text(renderer, gen_text, gen_btn.x + 85, gen_btn.y + 11, 
             (SDL_Color){255, 255, 255, 255});
    y += 45;
    
    // === SECTION 5: OUTPUT ===
    draw_text(renderer, "OUTPUT", panel_x + 10, y, text_color);
    y += 20;
    
    // Output text area
    int output_height = WINDOW_HEIGHT - y - 50;
    SDL_Rect output_rect = {panel_x + 10, y, panel_width - 20, output_height};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &output_rect);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &output_rect);
    
    // Draw output text (word-wrapped with scroll)
    if (strlen(state->llm_output_text) > 0) {
        int text_y = output_rect.y + 5 - output_scroll;
        int chars_per_line = 35;
        for (size_t i = 0; i < strlen(state->llm_output_text); i += chars_per_line) {
            if (text_y > output_rect.y + output_rect.h) break;
            if (text_y + 14 < output_rect.y) {
                text_y += 14;
                continue;
            }
            
            char line[64];
            int len = (int)strlen(state->llm_output_text + i);
            if (len > chars_per_line) len = chars_per_line;
            memcpy(line, state->llm_output_text + i, len);
            line[len] = '\0';
            draw_text(renderer, line, output_rect.x + 5, text_y, text_color);
            text_y += 14;
        }
    }
    
    y += output_height + 5;
    
    // Copy and Save buttons
    SDL_Rect copy_btn = {panel_x + 10, y, 90, 22};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &copy_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &copy_btn);
    draw_text(renderer, "Copy", copy_btn.x + 28, copy_btn.y + 5, text_color);
    
    SDL_Rect save_btn = {panel_x + 110, y, 90, 22};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &save_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &save_btn);
    draw_text(renderer, "Save", save_btn.x + 28, save_btn.y + 5, text_color);
    
    // === VISUALIZATION AREA ===
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_Rect viz_rect = {0, 60, RENDER_WIDTH, WINDOW_HEIGHT - 60};
    SDL_RenderFillRect(renderer, &viz_rect);
    
    if (state->cllm_model && state->cllm_model->num_lattice_points > 0) {
        draw_text(renderer, "Crystalline Lattice Visualization", 20, 80, text_color);
        
        int cx = RENDER_WIDTH / 2;
        int cy = (WINDOW_HEIGHT - 60) / 2 + 60;
        
        // Draw lattice points
        for (uint32_t i = 0; i < state->cllm_model->num_lattice_points && i < 2000; i++) {
            LatticePoint* pt = &state->cllm_model->lattice_points[i];
            
            int x = cx + (int)(pt->coords[0] * 250);
            int y_pos = cy + (int)(pt->coords[1] * 250);
            
            if (x < 0 || x >= RENDER_WIDTH || y_pos < 60 || y_pos >= WINDOW_HEIGHT) continue;
            
            uint8_t r = (uint8_t)((pt->prime_factor * 137) % 200 + 55);
            uint8_t g = (uint8_t)((pt->prime_factor * 211) % 200 + 55);
            uint8_t b = (uint8_t)((pt->prime_factor * 173) % 200 + 55);
            
            SDL_SetRenderDrawColor(renderer, r, g, b, 180);
            SDL_Rect point = {x - 2, y_pos - 2, 4, 4};
            SDL_RenderFillRect(renderer, &point);
        }
        
        // Draw connections for nearby points
        SDL_SetRenderDrawColor(renderer, 80, 80, 120, 60);
        for (uint32_t i = 0; i < state->cllm_model->num_lattice_points && i < 500; i++) {
            LatticePoint* pt1 = &state->cllm_model->lattice_points[i];
            int x1 = cx + (int)(pt1->coords[0] * 250);
            int y1 = cy + (int)(pt1->coords[1] * 250);
            
            for (uint32_t j = 0; j < pt1->neighbor_count && j < 12; j++) {
                uint32_t neighbor_id = pt1->neighbors[j];
                if (neighbor_id >= state->cllm_model->num_lattice_points) continue;
                
                LatticePoint* pt2 = &state->cllm_model->lattice_points[neighbor_id];
                int x2 = cx + (int)(pt2->coords[0] * 250);
                int y2 = cy + (int)(pt2->coords[1] * 250);
                
                SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            }
        }
    } else {
        draw_text(renderer, "Load a CLLM model to see visualization", 
                 RENDER_WIDTH / 2 - 150, WINDOW_HEIGHT / 2, text_color);
        draw_text(renderer, "Click 'Load Model' in the panel ->", 
                 RENDER_WIDTH / 2 - 130, WINDOW_HEIGHT / 2 + 30, 
                 (SDL_Color){150, 150, 150, 255});
    }
}

void handle_llm_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    int panel_x = RENDER_WIDTH;
    int panel_width = CONTROL_PANEL_WIDTH;
    
    // Load Model Button (y ≈ 110)
    SDL_Rect load_btn = {panel_x + 10, 110, panel_width - 20, 28};
    if (x >= load_btn.x && x <= load_btn.x + load_btn.w &&
        y >= load_btn.y && y <= load_btn.y + load_btn.h) {
        
        if (!state->cllm_model) {
            printf("Loading CLLM model...\n");
            
            // Try multiple model paths (relative to app directory)
            const char* model_paths[] = {
                "../data/models/cllm_pretrained.cllm",
                "../data/models/demo_model.cllm",
                "../models/demo_model.cllm",
                "../checkpoints/latest.cllm",
                NULL
            };
            
            for (int i = 0; model_paths[i] != NULL; i++) {
                printf("Trying: %s\n", model_paths[i]);
                state->cllm_model = cllm_read(model_paths[i]);
                if (state->cllm_model) {
                    printf("✓ Loaded model from: %s\n", model_paths[i]);
                    break;
                }
            }
            
            if (state->cllm_model) {
                state->cllm_inference = cllm_inference_init(state->cllm_model);
                if (state->cllm_inference) {
                    // Set default parameters
                    cllm_set_temperature(state->cllm_inference, state->llm_temperature);
                    cllm_set_max_tokens(state->cllm_inference, state->llm_max_tokens);
                    printf("Model loaded successfully!\n");
                    snprintf(state->llm_output_text, sizeof(state->llm_output_text),
                            "Model loaded successfully!\nVocab: %u | Layers: %u | Lattice: %u points\nReady for inference.",
                            state->cllm_model->vocab_size,
                            state->cllm_model->num_layers,
                            state->cllm_model->num_lattice_points);
                } else {
                    printf("Failed to initialize inference engine\n");
                    strcpy(state->llm_output_text, "Error: Failed to initialize inference engine.");
                }
            } else {
                printf("No model file found\n");
                strcpy(state->llm_output_text, "No model found. The application will create a default model on next startup, or you can train one in the Training tab.");
            }
        }
        return;
    }
    
    // Input text area (y ≈ 160-240)
    SDL_Rect input_rect = {panel_x + 10, 160, panel_width - 20, 80};
    if (x >= input_rect.x && x <= input_rect.x + input_rect.w &&
        y >= input_rect.y && y <= input_rect.y + input_rect.h) {
        input_active = true;
        SDL_StartTextInput();
        return;
    }
    
    // Clear button (y ≈ 260)
    SDL_Rect clear_btn = {panel_x + 10, 260, 80, 22};
    if (x >= clear_btn.x && x <= clear_btn.x + clear_btn.w &&
        y >= clear_btn.y && y <= clear_btn.y + clear_btn.h) {
        state->llm_input_text[0] = '\0';
        input_cursor = 0;
        return;
    }
    
    // Temperature slider (y ≈ 310)
    SDL_Rect temp_slider = {panel_x + 10, 310, panel_width - 20, 8};
    if (x >= temp_slider.x && x <= temp_slider.x + temp_slider.w &&
        y >= temp_slider.y - 5 && y <= temp_slider.y + temp_slider.h + 5) {
        float ratio = (float)(x - temp_slider.x) / temp_slider.w;
        state->llm_temperature = ratio * 2.0f;
        if (state->llm_temperature < 0.0f) state->llm_temperature = 0.0f;
        if (state->llm_temperature > 2.0f) state->llm_temperature = 2.0f;
        if (state->cllm_inference) {
            cllm_set_temperature(state->cllm_inference, state->llm_temperature);
        }
        return;
    }
    
    // Max tokens slider (y ≈ 346)
    SDL_Rect tokens_slider = {panel_x + 10, 346, panel_width - 20, 8};
    if (x >= tokens_slider.x && x <= tokens_slider.x + tokens_slider.w &&
        y >= tokens_slider.y - 5 && y <= tokens_slider.y + tokens_slider.h + 5) {
        float ratio = (float)(x - tokens_slider.x) / tokens_slider.w;
        state->llm_max_tokens = (int)(ratio * 500);
        if (state->llm_max_tokens < 1) state->llm_max_tokens = 1;
        if (state->llm_max_tokens > 500) state->llm_max_tokens = 500;
        if (state->cllm_inference) {
            cllm_set_max_tokens(state->cllm_inference, state->llm_max_tokens);
        }
        return;
    }
    
    // Generate button (y ≈ 376)
    SDL_Rect gen_btn = {panel_x + 10, 376, panel_width - 20, 35};
    if (x >= gen_btn.x && x <= gen_btn.x + gen_btn.w &&
        y >= gen_btn.y && y <= gen_btn.y + gen_btn.h) {
        
        if (state->cllm_inference && !state->llm_generating && strlen(state->llm_input_text) > 0) {
            state->llm_generating = true;
            printf("Generating response...\n");
            
            // Call the actual generation function
            int result = cllm_generate(state->cllm_inference, state->llm_input_text, 
                                      state->llm_output_text, sizeof(state->llm_output_text));
            
            if (result > 0) {
                printf("Generated %d tokens\n", result);
            } else {
                strcpy(state->llm_output_text, "Generation failed. Please try again.");
            }
            
            state->llm_generating = false;
            output_scroll = 0;
        } else if (!state->cllm_inference) {
            strcpy(state->llm_output_text, "Please load a model first.");
        } else if (strlen(state->llm_input_text) == 0) {
            strcpy(state->llm_output_text, "Please enter a prompt first.");
        }
        return;
    }
    
    // Copy button (y ≈ bottom)
    // Save button (y ≈ bottom)
    // (Implementation would go here)
}

void handle_llm_tab_text_input(AppState* state, const char* text) {
    if (!state || !input_active) return;
    
    size_t current_len = strlen(state->llm_input_text);
    size_t text_len = strlen(text);
    
    if (current_len + text_len < sizeof(state->llm_input_text) - 1) {
        strcat(state->llm_input_text, text);
        input_cursor += text_len;
    }
}

void handle_llm_tab_key(AppState* state, SDL_Keycode key) {
    if (!state || !input_active) return;
    
    if (key == SDLK_BACKSPACE && strlen(state->llm_input_text) > 0) {
        state->llm_input_text[strlen(state->llm_input_text) - 1] = '\0';
        if (input_cursor > 0) input_cursor--;
    } else if (key == SDLK_RETURN) {
        // Add newline
        size_t len = strlen(state->llm_input_text);
        if (len < sizeof(state->llm_input_text) - 2) {
            state->llm_input_text[len] = '\n';
            state->llm_input_text[len + 1] = '\0';
        }
    } else if (key == SDLK_ESCAPE) {
        input_active = false;
        SDL_StopTextInput();
    }
}