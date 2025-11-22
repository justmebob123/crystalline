// app/ui/tabs/tab_llm.c - FIXED LLM Interaction Tab Implementation
#include "../../app_common.h"
#include "../../cllm_integration.h"
#include <string.h>

// UI state for text input
static bool input_active = false;
static int input_cursor = 0;
static int output_scroll = 0;

// Store button positions for click detection
static SDL_Rect g_load_btn;
static SDL_Rect g_create_btn;
static SDL_Rect g_save_btn;
static SDL_Rect g_input_rect;
static SDL_Rect g_clear_btn;
static SDL_Rect g_temp_slider;
static SDL_Rect g_tokens_slider;
static SDL_Rect g_gen_btn;

void draw_llm_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    int panel_x = RENDER_WIDTH;
    int panel_y = 60;
    int panel_width = CONTROL_PANEL_WIDTH;
    
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {40, 40, 50, 255};
    SDL_Color button_color = {60, 60, 80, 255};
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
        snprintf(info, sizeof(info), "Vocab: %lu", (unsigned long)state->cllm_model->vocab_size);
        draw_text(renderer, info, panel_x + 10, y, text_color);
        y += 16;
        
        snprintf(info, sizeof(info), "Layers: %u", state->cllm_model->num_layers);
        draw_text(renderer, info, panel_x + 10, y, text_color);
        y += 16;
        
        snprintf(info, sizeof(info), "Lattice: %lu pts", (unsigned long)state->cllm_model->num_lattice_points);
        draw_text(renderer, info, panel_x + 10, y, text_color);
        y += 20;
    } else {
        y += 20;
    }
    
    // Create Model Button - STORE POSITION
    g_create_btn = (SDL_Rect){panel_x + 10, y, (panel_width - 30) / 2, 28};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_create_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_create_btn);
    draw_text(renderer, "Create New", g_create_btn.x + 30, g_create_btn.y + 7, text_color);
    
    // Load Model Button - STORE POSITION
    g_load_btn = (SDL_Rect){panel_x + 15 + (panel_width - 30) / 2, y, (panel_width - 30) / 2, 28};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_load_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_load_btn);
    draw_text(renderer, "Load", g_load_btn.x + 35, g_load_btn.y + 7, text_color);
    y += 35;
    
    // Save Model Button (only if model loaded)
    if (state->cllm_model) {
        g_save_btn = (SDL_Rect){panel_x + 10, y, panel_width - 20, 28};
        SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
        SDL_RenderFillRect(renderer, &g_save_btn);
        SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
        SDL_RenderDrawRect(renderer, &g_save_btn);
        draw_text(renderer, "Save Model", g_save_btn.x + 80, g_save_btn.y + 7, text_color);
        y += 35;
    }
    
    // === SECTION 2: INPUT ===
    draw_text(renderer, "INPUT PROMPT", panel_x + 10, y, text_color);
    y += 20;
    
    // Input text area - STORE POSITION
    g_input_rect = (SDL_Rect){panel_x + 10, y, panel_width - 20, 80};
    SDL_Color input_bg = input_active ? (SDL_Color){50, 50, 70, 255} : (SDL_Color){30, 30, 40, 255};
    SDL_SetRenderDrawColor(renderer, input_bg.r, input_bg.g, input_bg.b, 255);
    SDL_RenderFillRect(renderer, &g_input_rect);
    SDL_SetRenderDrawColor(renderer, input_active ? active_color.r : text_color.r, 
                           input_active ? active_color.g : text_color.g,
                           input_active ? active_color.b : text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_input_rect);
    
    // Draw input text (word-wrapped)
    if (strlen(state->llm_input_text) > 0) {
        int text_y = g_input_rect.y + 5;
        int chars_per_line = 35;
        for (size_t i = 0; i < strlen(state->llm_input_text); i += chars_per_line) {
            char line[64];
            int len = (int)strlen(state->llm_input_text + i);
            if (len > chars_per_line) len = chars_per_line;
            memcpy(line, state->llm_input_text + i, (size_t)len);
            line[len] = '\0';
            draw_text(renderer, line, g_input_rect.x + 5, text_y, text_color);
            text_y += 14;
            if (text_y > g_input_rect.y + g_input_rect.h - 10) break;
        }
    } else if (input_active) {
        draw_text(renderer, "Type your prompt...", g_input_rect.x + 5, g_input_rect.y + 5, 
                 (SDL_Color){120, 120, 120, 255});
    }
    y += 85;
    
    // Clear button - STORE POSITION
    g_clear_btn = (SDL_Rect){panel_x + 10, y, 80, 22};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_clear_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_clear_btn);
    draw_text(renderer, "Clear", g_clear_btn.x + 22, g_clear_btn.y + 4, text_color);
    y += 30;
    
    // === SECTION 3: PARAMETERS ===
    draw_text(renderer, "PARAMETERS", panel_x + 10, y, text_color);
    y += 20;
    
    // Temperature slider - STORE POSITION
    char temp_label[64];
    snprintf(temp_label, sizeof(temp_label), "Temperature: %.2f", state->llm_temperature);
    draw_text(renderer, temp_label, panel_x + 10, y, text_color);
    y += 16;
    
    g_temp_slider = (SDL_Rect){panel_x + 10, y, panel_width - 20, 8};
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(renderer, &g_temp_slider);
    
    int temp_handle_x = g_temp_slider.x + (int)((state->llm_temperature / 2.0f) * g_temp_slider.w);
    SDL_Rect temp_handle = {temp_handle_x - 4, g_temp_slider.y - 4, 8, 16};
    SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
    SDL_RenderFillRect(renderer, &temp_handle);
    y += 20;
    
    // Max tokens slider - STORE POSITION
    char tokens_label[64];
    snprintf(tokens_label, sizeof(tokens_label), "Max Tokens: %d", state->llm_max_tokens);
    draw_text(renderer, tokens_label, panel_x + 10, y, text_color);
    y += 16;
    
    g_tokens_slider = (SDL_Rect){panel_x + 10, y, panel_width - 20, 8};
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(renderer, &g_tokens_slider);
    
    int tokens_handle_x = g_tokens_slider.x + (int)((float)state->llm_max_tokens / 500.0f * g_tokens_slider.w);
    SDL_Rect tokens_handle = {tokens_handle_x - 4, g_tokens_slider.y - 4, 8, 16};
    SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
    SDL_RenderFillRect(renderer, &tokens_handle);
    y += 20;
    
    // === SECTION 4: GENERATE ===
    // Generate button - STORE POSITION
    g_gen_btn = (SDL_Rect){panel_x + 10, y, panel_width - 20, 35};
    SDL_Color gen_color = state->llm_generating ? 
        (SDL_Color){80, 80, 80, 255} : (SDL_Color){80, 120, 180, 255};
    SDL_SetRenderDrawColor(renderer, gen_color.r, gen_color.g, gen_color.b, 255);
    SDL_RenderFillRect(renderer, &g_gen_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_gen_btn);
    
    const char* gen_text = state->llm_generating ? "Generating..." : "Generate Response";
    draw_text(renderer, gen_text, g_gen_btn.x + 60, g_gen_btn.y + 10, text_color);
    y += 45;
    
    // === SECTION 5: OUTPUT ===
    draw_text(renderer, "OUTPUT", panel_x + 10, y, text_color);
    y += 20;
    
    // Output text area
    SDL_Rect output_rect = {panel_x + 10, y, panel_width - 20, WINDOW_HEIGHT - y - 10};
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
            memcpy(line, state->llm_output_text + i, (size_t)len);
            line[len] = '\0';
            draw_text(renderer, line, output_rect.x + 5, text_y, text_color);
            text_y += 14;
        }
    }
    
    // === VISUALIZATION AREA ===
    if (state->cllm_model && state->cllm_model->lattice_points) {
        int cx = RENDER_WIDTH / 2;
        int cy = WINDOW_HEIGHT / 2;
        
        // Draw lattice points
        for (uint32_t i = 0; i < state->cllm_model->num_lattice_points && i < 1000; i++) {
            CLLMLatticePoint* pt = &state->cllm_model->lattice_points[i];
            
            int x = cx + (int)(pt->coords[0] * 250);
            int y_pos = cy + (int)(pt->coords[1] * 250);
            
            if (x < 0 || x >= RENDER_WIDTH || y_pos < 0 || y_pos >= WINDOW_HEIGHT) continue;
            
            uint8_t r = (uint8_t)((pt->prime * 137) % 200 + 55);
            uint8_t g = (uint8_t)((pt->prime * 211) % 200 + 55);
            uint8_t b = (uint8_t)((pt->prime * 173) % 200 + 55);
            
            SDL_SetRenderDrawColor(renderer, r, g, b, 180);
            SDL_Rect point = {x - 2, y_pos - 2, 4, 4};
            SDL_RenderFillRect(renderer, &point);
        }
        
        // Draw connections for nearby points
        SDL_SetRenderDrawColor(renderer, 80, 80, 120, 60);
        for (uint32_t i = 0; i < state->cllm_model->num_lattice_points && i < 500; i++) {
            CLLMLatticePoint* pt1 = &state->cllm_model->lattice_points[i];
            int x1 = cx + (int)(pt1->coords[0] * 250);
            int y1 = cy + (int)(pt1->coords[1] * 250);
            
            for (uint32_t j = 0; j < pt1->neighbor_count && j < 12; j++) {
                uint32_t neighbor_id = pt1->neighbors[j];
                if (neighbor_id >= state->cllm_model->num_lattice_points) continue;
                
                CLLMLatticePoint* pt2 = &state->cllm_model->lattice_points[neighbor_id];
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
    
    // Create New Model Button
    if (x >= g_create_btn.x && x <= g_create_btn.x + g_create_btn.w &&
        y >= g_create_btn.y && y <= g_create_btn.y + g_create_btn.h) {
        
        printf("Creating new CLLM model...\n");
        
        // Free existing model if any
        if (state->cllm_model) {
            if (state->cllm_inference) {
                cllm_inference_cleanup(state->cllm_inference);
                state->cllm_inference = NULL;
            }
            if (state->cllm_training) {
                cllm_training_free(state->cllm_training);
                state->cllm_training = NULL;
            }
            app_free_cllm_model(state->cllm_model);
            state->cllm_model = NULL;
        }
        
        // Create new model
        state->cllm_model = app_create_cllm_model_default();
        
        if (state->cllm_model) {
            printf("✓ Model created\n");
            state->cllm_inference = app_init_inference(state->cllm_model);
            
            if (state->cllm_inference) {
                char info[512];
                app_get_model_info(state->cllm_model, info, sizeof(info));
                snprintf(state->llm_output_text, sizeof(state->llm_output_text),
                        "New model created!\n\n%s\n\nReady for inference or training.", info);
            }
        } else {
            strncpy(state->llm_output_text, "Failed to create model.", sizeof(state->llm_output_text) - 1);
        }
        return;
    }
    
    // Save Model Button
    if (state->cllm_model && 
        x >= g_save_btn.x && x <= g_save_btn.x + g_save_btn.w &&
        y >= g_save_btn.y && y <= g_save_btn.y + g_save_btn.h) {
        
        printf("Saving model...\n");
        int ret = system("mkdir -p models");
        (void)ret;  // Explicitly ignore return value
        
        const char* save_path = "models/saved_model.cllm";
        int result = app_save_model(state->cllm_model, save_path);
        
        if (result == 0) {
            printf("✓ Model saved\n");
            snprintf(state->llm_output_text, sizeof(state->llm_output_text),
                    "Model saved to:\n%s", save_path);
        } else {
            strncpy(state->llm_output_text, "Failed to save model.", sizeof(state->llm_output_text) - 1);
        }
        return;
    }
    
    // Load Model Button - use stored position
    if (x >= g_load_btn.x && x <= g_load_btn.x + g_load_btn.w &&
        y >= g_load_btn.y && y <= g_load_btn.y + g_load_btn.h) {
        
        if (!state->cllm_model) {
            printf("Loading CLLM model...\n");
            
            // Try multiple model paths (relative to app directory)
            const char* model_paths[] = {
                   "data/models/default_model.cllm",
                   "data/models/demo_model.cllm",
                   "../data/models/default_model.cllm",
                   "../data/models/demo_model.cllm",
                   "../data/models/cllm_pretrained.cllm"
            };
            int num_paths = sizeof(model_paths) / sizeof(model_paths[0]);
            
            for (int i = 0; i < num_paths; i++) {
                printf("Trying: %s\n", model_paths[i]);
                // Create models directory if it doesn't exist
                char models_dir[512];
                snprintf(models_dir, sizeof(models_dir), "%s/.cllm_models", 
                        getenv("HOME") ? getenv("HOME") : ".");
                int mkdir_ret = system("mkdir -p ~/.cllm_models");
                (void)mkdir_ret;  // Explicitly ignore return value
                
                // Load the selected model
                char full_path[1024];  // Increased size to avoid truncation
                snprintf(full_path, sizeof(full_path), "%s/%s", models_dir, model_paths[i]);
                state->cllm_model = cllm_read_model(full_path);
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
                            "Model loaded successfully!\nVocab: %lu | Layers: %lu | Lattice: %lu points\nReady for inference.",
                            (unsigned long)state->cllm_model->vocab_size,
                            (unsigned long)state->cllm_model->num_layers,
                            (unsigned long)state->cllm_model->num_lattice_points);
                } else {
                    printf("Failed to initialize inference engine\n");
                    strncpy(state->llm_output_text, "Error: Failed to initialize inference engine.", sizeof(state->llm_output_text) - 1);
                }
            } else {
                printf("No model file found\n");
                strncpy(state->llm_output_text, "No model found. The application will create a default model on next startup, or you can train one in the Training tab.", sizeof(state->llm_output_text) - 1);
            }
        }
        return;
    }
    
    // Input text area - use stored position
    if (x >= g_input_rect.x && x <= g_input_rect.x + g_input_rect.w &&
        y >= g_input_rect.y && y <= g_input_rect.y + g_input_rect.h) {
        input_active = true;
        SDL_StartTextInput();
        return;
    }
    
    // Clear button - use stored position
    if (x >= g_clear_btn.x && x <= g_clear_btn.x + g_clear_btn.w &&
        y >= g_clear_btn.y && y <= g_clear_btn.y + g_clear_btn.h) {
        state->llm_input_text[0] = '\0';
        input_cursor = 0;
        return;
    }
    
    // Temperature slider - use stored position
    if (x >= g_temp_slider.x && x <= g_temp_slider.x + g_temp_slider.w &&
        y >= g_temp_slider.y - 5 && y <= g_temp_slider.y + g_temp_slider.h + 5) {
        float ratio = (float)(x - g_temp_slider.x) / g_temp_slider.w;
        state->llm_temperature = ratio * 2.0f;
        if (state->llm_temperature < 0.0f) state->llm_temperature = 0.0f;
        if (state->llm_temperature > 2.0f) state->llm_temperature = 2.0f;
        if (state->cllm_inference) {
            cllm_set_temperature(state->cllm_inference, state->llm_temperature);
        }
        return;
    }
    
    // Max tokens slider - use stored position
    if (x >= g_tokens_slider.x && x <= g_tokens_slider.x + g_tokens_slider.w &&
        y >= g_tokens_slider.y - 5 && y <= g_tokens_slider.y + g_tokens_slider.h + 5) {
        float ratio = (float)(x - g_tokens_slider.x) / g_tokens_slider.w;
        state->llm_max_tokens = (int)(ratio * 500);
        if (state->llm_max_tokens < 1) state->llm_max_tokens = 1;
        if (state->llm_max_tokens > 500) state->llm_max_tokens = 500;
        if (state->cllm_inference) {
            cllm_set_max_tokens(state->cllm_inference, state->llm_max_tokens);
        }
        return;
    }
    
    // Generate button - use stored position
    if (x >= g_gen_btn.x && x <= g_gen_btn.x + g_gen_btn.w &&
        y >= g_gen_btn.y && y <= g_gen_btn.y + g_gen_btn.h) {
        
        if (state->cllm_inference && !state->llm_generating && strlen(state->llm_input_text) > 0) {
            state->llm_generating = true;
            printf("Generating response...\n");
            
            // Use our integration function
            int result = app_generate_text(state, state->llm_input_text, 
                                          state->llm_output_text, sizeof(state->llm_output_text));
            
            if (result > 0) {
                printf("✓ Generated %d tokens\n", result);
            } else {
                strncpy(state->llm_output_text, "Generation failed. Please try again.", sizeof(state->llm_output_text) - 1);
            }
            
            state->llm_generating = false;
            output_scroll = 0;
        } else if (!state->cllm_inference) {
            strncpy(state->llm_output_text, "Please create or load a model first.", sizeof(state->llm_output_text) - 1);
        } else if (strlen(state->llm_input_text) == 0) {
            strncpy(state->llm_output_text, "Please enter a prompt first.", sizeof(state->llm_output_text) - 1);
        }
        return;
    }
}

void handle_llm_tab_text_input(AppState* state, const char* text) {
    if (!state || !input_active) return;
    
    size_t current_len = strlen(state->llm_input_text);
    size_t text_len = strlen(text);
    
    if (current_len + text_len < sizeof(state->llm_input_text) - 1) {
        strncat(state->llm_input_text, text, sizeof(state->llm_input_text) - current_len - 1);
        input_cursor += (int)text_len;
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