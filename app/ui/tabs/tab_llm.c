// app/ui/tabs/tab_llm.c - Chat Interface with Control Panel
#include "../../app_common.h"
#include "../../cllm_integration.h"
#include <string.h>
#include <time.h>
#include <stdio.h>

#define MAX_CHAT_MESSAGES 100
#define MAX_MESSAGE_LENGTH 2048

typedef struct {
    char text[MAX_MESSAGE_LENGTH];
    bool is_user;  // true = user message, false = AI message
    time_t timestamp;
} ChatMessage;

// Chat state
static ChatMessage chat_history[MAX_CHAT_MESSAGES];
static int chat_message_count = 0;
static int chat_scroll_offset = 0;

// UI state
static bool input_active = false;
static int input_cursor = 0;

// Store button positions
static SDL_Rect g_send_btn;
static SDL_Rect g_clear_chat_btn;
static SDL_Rect g_input_rect;
static SDL_Rect g_chat_area;
static SDL_Rect g_create_btn;
static SDL_Rect g_load_btn;
static SDL_Rect g_save_btn;
static SDL_Rect g_temp_slider;
static SDL_Rect g_tokens_slider;

// Add message to chat history
void add_chat_message(const char* text, bool is_user) {
    if (chat_message_count >= MAX_CHAT_MESSAGES) {
        // Shift messages up
        for (int i = 0; i < MAX_CHAT_MESSAGES - 1; i++) {
            chat_history[i] = chat_history[i + 1];
        }
        chat_message_count = MAX_CHAT_MESSAGES - 1;
    }
    
    strncpy(chat_history[chat_message_count].text, text, MAX_MESSAGE_LENGTH - 1);
    chat_history[chat_message_count].text[MAX_MESSAGE_LENGTH - 1] = '\0';
    chat_history[chat_message_count].is_user = is_user;
    chat_history[chat_message_count].timestamp = time(NULL);
    chat_message_count++;
    
    // Auto-scroll to bottom
    chat_scroll_offset = 0;
}

// Clear chat history
void clear_chat_history(void) {
    chat_message_count = 0;
    chat_scroll_offset = 0;
}

// Draw a chat message bubble
void draw_chat_message(SDL_Renderer* renderer, ChatMessage* msg, int x, int y, int width) {
    SDL_Color user_bg = {70, 100, 180, 255};
    SDL_Color ai_bg = {50, 50, 60, 255};
    SDL_Color text_color = {220, 220, 220, 255};
    
    // Calculate message height (word-wrapped)
    int chars_per_line = (width - 20) / 7;  // Approximate
    int num_lines = (strlen(msg->text) + chars_per_line - 1) / chars_per_line;
    int msg_height = num_lines * 16 + 20;
    
    // Draw message bubble
    SDL_Rect bubble;
    if (msg->is_user) {
        // User messages on right
        bubble = (SDL_Rect){x + width / 4, y, width * 3 / 4 - 10, msg_height};
        SDL_SetRenderDrawColor(renderer, user_bg.r, user_bg.g, user_bg.b, 255);
    } else {
        // AI messages on left
        bubble = (SDL_Rect){x + 10, y, width * 3 / 4 - 10, msg_height};
        SDL_SetRenderDrawColor(renderer, ai_bg.r, ai_bg.g, ai_bg.b, 255);
    }
    SDL_RenderFillRect(renderer, &bubble);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
    SDL_RenderDrawRect(renderer, &bubble);
    
    // Draw label
    const char* label = msg->is_user ? "You" : "AI";
    draw_text(renderer, label, bubble.x + 5, bubble.y + 3, 
             msg->is_user ? (SDL_Color){200, 220, 255, 255} : (SDL_Color){150, 200, 150, 255});
    
    // Draw message text (word-wrapped)
    int text_y = bubble.y + 18;
    for (size_t i = 0; i < strlen(msg->text); i += chars_per_line) {
        char line[256];
        int len = (int)strlen(msg->text + i);
        if (len > chars_per_line) len = chars_per_line;
        memcpy(line, msg->text + i, (size_t)len);
        line[len] = '\0';
        draw_text(renderer, line, bubble.x + 8, text_y, text_color);
        text_y += 16;
    }
}

void draw_llm_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    int panel_x = RENDER_WIDTH;
    int panel_y = 60;
    int panel_width = CONTROL_PANEL_WIDTH;
    
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {40, 40, 50, 255};
    SDL_Color button_color = {60, 60, 80, 255};
    SDL_Color active_color = {100, 150, 200, 255};
    
    // === CONTROL PANEL ===
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_Rect panel_rect = {panel_x, panel_y, panel_width, WINDOW_HEIGHT - panel_y};
    SDL_RenderFillRect(renderer, &panel_rect);
    
    int y = panel_y + 10;
    
    // Model Status
    draw_text(renderer, "CLLM MODEL", panel_x + 10, y, text_color);
    y += 20;
    
    const char* status = state->cllm_model ? "Loaded" : "Not Loaded";
    SDL_Color status_color = state->cllm_model ? 
        (SDL_Color){100, 255, 100, 255} : (SDL_Color){255, 100, 100, 255};
    char status_text[64];
    snprintf(status_text, sizeof(status_text), "Status: %s", status);
    draw_text(renderer, status_text, panel_x + 10, y, status_color);
    y += 18;
    
    if (state->cllm_model) {
        char info[128];
        snprintf(info, sizeof(info), "Vocab: %lu", (unsigned long)state->cllm_model->vocab_size);
        draw_text(renderer, info, panel_x + 10, y, text_color);
        y += 16;
        snprintf(info, sizeof(info), "Layers: %u", state->cllm_model->num_layers);
        draw_text(renderer, info, panel_x + 10, y, text_color);
        y += 20;
    } else {
        y += 20;
    }
    
    // Model buttons
    g_create_btn = (SDL_Rect){panel_x + 10, y, (panel_width - 30) / 2, 28};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_create_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_create_btn);
    draw_text(renderer, "Create", g_create_btn.x + 25, g_create_btn.y + 7, text_color);
    
    g_load_btn = (SDL_Rect){panel_x + 15 + (panel_width - 30) / 2, y, (panel_width - 30) / 2, 28};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_load_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_load_btn);
    draw_text(renderer, "Load", g_load_btn.x + 30, g_load_btn.y + 7, text_color);
    y += 35;
    
    if (state->cllm_model) {
        g_save_btn = (SDL_Rect){panel_x + 10, y, panel_width - 20, 28};
        SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
        SDL_RenderFillRect(renderer, &g_save_btn);
        SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
        SDL_RenderDrawRect(renderer, &g_save_btn);
        draw_text(renderer, "Save Model", g_save_btn.x + 70, g_save_btn.y + 7, text_color);
        y += 35;
    }
    
    // Parameters
    draw_text(renderer, "PARAMETERS", panel_x + 10, y, text_color);
    y += 20;
    
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
    y += 30;
    
    // Clear chat button
    g_clear_chat_btn = (SDL_Rect){panel_x + 10, y, panel_width - 20, 25};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_clear_chat_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_clear_chat_btn);
    draw_text(renderer, "Clear Chat", g_clear_chat_btn.x + 70, g_clear_chat_btn.y + 6, text_color);
    
    // === CHAT AREA (LEFT SIDE) ===
    int chat_width = RENDER_WIDTH - 20;
    int input_height = 80;
    int chat_height = WINDOW_HEIGHT - 60 - input_height - 20;
    
    g_chat_area = (SDL_Rect){10, 70, chat_width, chat_height};
    SDL_SetRenderDrawColor(renderer, 25, 25, 35, 255);
    SDL_RenderFillRect(renderer, &g_chat_area);
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderDrawRect(renderer, &g_chat_area);
    
    // Draw chat messages
    int msg_y = g_chat_area.y + g_chat_area.h - 10 - chat_scroll_offset;
    for (int i = chat_message_count - 1; i >= 0; i--) {
        int chars_per_line = (g_chat_area.w - 40) / 7;
        int num_lines = (strlen(chat_history[i].text) + chars_per_line - 1) / chars_per_line;
        int msg_height = num_lines * 16 + 30;
        
        msg_y -= msg_height;
        
        if (msg_y + msg_height < g_chat_area.y) break;
        if (msg_y > g_chat_area.y + g_chat_area.h) continue;
        
        draw_chat_message(renderer, &chat_history[i], g_chat_area.x, msg_y, g_chat_area.w);
        msg_y -= 10;
    }
    
    if (chat_message_count == 0) {
        draw_text(renderer, "Start a conversation...", 
                 g_chat_area.x + g_chat_area.w / 2 - 80,
                 g_chat_area.y + g_chat_area.h / 2,
                 (SDL_Color){100, 100, 100, 255});
    }
    
    // === INPUT AREA ===
    int input_y = WINDOW_HEIGHT - input_height - 10;
    
    g_input_rect = (SDL_Rect){10, input_y, chat_width - 110, input_height};
    SDL_Color input_bg = input_active ? (SDL_Color){50, 50, 70, 255} : (SDL_Color){40, 40, 50, 255};
    SDL_SetRenderDrawColor(renderer, input_bg.r, input_bg.g, input_bg.b, 255);
    SDL_RenderFillRect(renderer, &g_input_rect);
    SDL_SetRenderDrawColor(renderer, input_active ? active_color.r : text_color.r,
                           input_active ? active_color.g : text_color.g,
                           input_active ? active_color.b : text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_input_rect);
    
    if (strlen(state->llm_input_text) > 0) {
        int text_y = g_input_rect.y + 8;
        int chars_per_line = (g_input_rect.w - 20) / 7;
        for (size_t i = 0; i < strlen(state->llm_input_text); i += chars_per_line) {
            char line[256];
            int len = (int)strlen(state->llm_input_text + i);
            if (len > chars_per_line) len = chars_per_line;
            memcpy(line, state->llm_input_text + i, (size_t)len);
            line[len] = '\0';
            draw_text(renderer, line, g_input_rect.x + 8, text_y, text_color);
            text_y += 16;
            if (text_y > g_input_rect.y + g_input_rect.h - 10) break;
        }
    } else {
        draw_text(renderer, "Type your message...", 
                 g_input_rect.x + 8, g_input_rect.y + 30,
                 (SDL_Color){100, 100, 100, 255});
    }
    
    g_send_btn = (SDL_Rect){chat_width - 90, input_y, 100, input_height};
    SDL_Color send_color = state->llm_generating ? 
        (SDL_Color){80, 80, 80, 255} : (SDL_Color){80, 150, 80, 255};
    SDL_SetRenderDrawColor(renderer, send_color.r, send_color.g, send_color.b, 255);
    SDL_RenderFillRect(renderer, &g_send_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_send_btn);
    const char* send_text = state->llm_generating ? "..." : "Send";
    draw_text(renderer, send_text, g_send_btn.x + 32, g_send_btn.y + 32, text_color);
}

void handle_llm_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    // Input box
    if (x >= g_input_rect.x && x <= g_input_rect.x + g_input_rect.w &&
        y >= g_input_rect.y && y <= g_input_rect.y + g_input_rect.h) {
        input_active = true;
        SDL_StartTextInput();
        return;
    }
    
    // Send button
    if (x >= g_send_btn.x && x <= g_send_btn.x + g_send_btn.w &&
        y >= g_send_btn.y && y <= g_send_btn.y + g_send_btn.h) {
        
        if (!state->llm_generating && strlen(state->llm_input_text) > 0 && state->cllm_inference) {
            add_chat_message(state->llm_input_text, true);
            
            state->llm_generating = true;
            char response[MAX_MESSAGE_LENGTH];
            
            printf("=== GENERATING RESPONSE ===\n");
            printf("Prompt: %s\n", state->llm_input_text);
            
            int result = cllm_generate(state->cllm_inference, state->llm_input_text,
                                      response, sizeof(response));
            
            printf("Generated %d tokens\n", result);
            printf("Response: %s\n", response);
            printf("===========================\n");
            
            if (result > 0) {
                add_chat_message(response, false);
            } else {
                add_chat_message("Sorry, I couldn't generate a response.", false);
            }
            
            state->llm_generating = false;
            state->llm_input_text[0] = '\0';
            input_cursor = 0;
        }
        return;
    }
    
    // Clear chat
    if (x >= g_clear_chat_btn.x && x <= g_clear_chat_btn.x + g_clear_chat_btn.w &&
        y >= g_clear_chat_btn.y && y <= g_clear_chat_btn.y + g_clear_chat_btn.h) {
        clear_chat_history();
        return;
    }
    
    // Create button
    if (x >= g_create_btn.x && x <= g_create_btn.x + g_create_btn.w &&
        y >= g_create_btn.y && y <= g_create_btn.y + g_create_btn.h) {
        printf("Creating new CLLM model...\n");
        state->cllm_model = app_create_cllm_model_default();
        if (state->cllm_model) {
            printf("✓ Model created\n");
            if (state->cllm_inference) {
                cllm_inference_cleanup(state->cllm_inference);
            }
            state->cllm_inference = cllm_inference_init(state->cllm_model);
        }
        return;
    }
    
    // Load button
    if (x >= g_load_btn.x && x <= g_load_btn.x + g_load_btn.w &&
        y >= g_load_btn.y && y <= g_load_btn.y + g_load_btn.h) {
        printf("Loading CLLM model...\n");
        // Try to load from default locations
        const char* model_paths[] = {
            "data/models/default_model.cllm",
            "models/default_model.cllm",
            "../data/models/default_model.cllm"
        };
        for (int i = 0; i < 3; i++) {
            CLLMModel* loaded = cllm_read_model(model_paths[i]);
            if (loaded) {
                if (state->cllm_model) {
                    // TODO: Free old model
                }
                state->cllm_model = loaded;
                if (state->cllm_inference) {
                    cllm_inference_cleanup(state->cllm_inference);
                }
                state->cllm_inference = cllm_inference_init(state->cllm_model);
                printf("✓ Model loaded from: %s\n", model_paths[i]);
                break;
            }
        }
        return;
    }
    
    // Save button
    if (state->cllm_model && x >= g_save_btn.x && x <= g_save_btn.x + g_save_btn.w &&
        y >= g_save_btn.y && y <= g_save_btn.y + g_save_btn.h) {
        printf("Saving model...\n");
        if (cllm_write_model(state->cllm_model, "models/saved_model.cllm") == 0) {
            printf("✓ Model saved\n");
        }
        return;
    }
    
    // Temperature slider
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
    
    // Max tokens slider
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
}

bool handle_llm_tab_event(AppState* state, SDL_Event* event) {
    if (!state || !event) return false;
    
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        handle_llm_tab_click(state, event->button.x, event->button.y);
        return true;
    }
    
    if (event->type == SDL_MOUSEWHEEL) {
        if (event->wheel.y > 0) {
            chat_scroll_offset += 30;
        } else if (event->wheel.y < 0) {
            chat_scroll_offset -= 30;
            if (chat_scroll_offset < 0) chat_scroll_offset = 0;
        }
        return true;
    }
    
    return false;
}

void handle_llm_tab_text_input(AppState* state, const char* text) {
    if (!state || !text || !input_active) return;
    
    size_t current_len = strlen(state->llm_input_text);
    size_t text_len = strlen(text);
    
    if (current_len + text_len < sizeof(state->llm_input_text) - 1) {
        strcat(state->llm_input_text, text);
        input_cursor += (int)text_len;
    }
}

void handle_llm_tab_key(AppState* state, SDL_Keycode key) {
    if (!state || !input_active) return;
    
    if (key == SDLK_BACKSPACE && strlen(state->llm_input_text) > 0) {
        state->llm_input_text[strlen(state->llm_input_text) - 1] = '\0';
        input_cursor--;
        if (input_cursor < 0) input_cursor = 0;
    } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        if (strlen(state->llm_input_text) > 0 && state->cllm_inference && !state->llm_generating) {
            add_chat_message(state->llm_input_text, true);
            
            state->llm_generating = true;
            char response[MAX_MESSAGE_LENGTH];
            
            printf("=== GENERATING RESPONSE ===\n");
            printf("Prompt: %s\n", state->llm_input_text);
            
            int result = cllm_generate(state->cllm_inference, state->llm_input_text,
                                      response, sizeof(response));
            
            printf("Generated %d tokens\n", result);
            printf("Response: %s\n", response);
            printf("===========================\n");
            
            if (result > 0) {
                add_chat_message(response, false);
            } else {
                add_chat_message("Sorry, I couldn't generate a response.", false);
            }
            
            state->llm_generating = false;
            state->llm_input_text[0] = '\0';
            input_cursor = 0;
        }
    } else if (key == SDLK_ESCAPE) {
        input_active = false;
        SDL_StopTextInput();
    }
}