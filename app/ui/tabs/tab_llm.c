// app/ui/tabs/tab_llm_chat.c - Modern Chat-Style LLM Interface
#include "../../app_common.h"
#include "../../cllm_integration.h"
#include <string.h>
#include <time.h>

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
static SDL_Rect g_model_btn;
static SDL_Rect g_settings_btn;

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
    
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {25, 25, 35, 255};
    SDL_Color button_color = {60, 60, 80, 255};
    SDL_Color active_color = {100, 150, 200, 255};
    
    // === TOP BAR (Model Status & Controls) ===
    int top_bar_height = 50;
    SDL_Rect top_bar = {0, 60, RENDER_WIDTH, top_bar_height};
    SDL_SetRenderDrawColor(renderer, 35, 35, 45, 255);
    SDL_RenderFillRect(renderer, &top_bar);
    
    // Model status
    const char* status = state->cllm_model ? "Model Loaded" : "No Model";
    SDL_Color status_color = state->cllm_model ? 
        (SDL_Color){100, 255, 100, 255} : (SDL_Color){255, 100, 100, 255};
    draw_text(renderer, status, 10, 75, status_color);
    
    if (state->cllm_model) {
        char info[128];
        snprintf(info, sizeof(info), "Vocab: %lu | Layers: %u", 
                (unsigned long)state->cllm_model->vocab_size,
                state->cllm_model->num_layers);
        draw_text(renderer, info, 120, 75, text_color);
    }
    
    // Model button
    g_model_btn = (SDL_Rect){RENDER_WIDTH - 220, 70, 100, 30};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_model_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_model_btn);
    draw_text(renderer, "Model", g_model_btn.x + 28, g_model_btn.y + 8, text_color);
    
    // Settings button
    g_settings_btn = (SDL_Rect){RENDER_WIDTH - 110, 70, 100, 30};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_settings_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_settings_btn);
    draw_text(renderer, "Settings", g_settings_btn.x + 20, g_settings_btn.y + 8, text_color);
    
    // === CHAT AREA ===
    int chat_y = 60 + top_bar_height;
    int input_height = 100;
    int chat_height = WINDOW_HEIGHT - chat_y - input_height - 10;
    
    g_chat_area = (SDL_Rect){10, chat_y, RENDER_WIDTH - 20, chat_height};
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &g_chat_area);
    
    // Draw chat messages
    int msg_y = g_chat_area.y + g_chat_area.h - 10 - chat_scroll_offset;
    for (int i = chat_message_count - 1; i >= 0; i--) {
        // Calculate message height
        int chars_per_line = (g_chat_area.w - 40) / 7;
        int num_lines = (strlen(chat_history[i].text) + chars_per_line - 1) / chars_per_line;
        int msg_height = num_lines * 16 + 30;
        
        msg_y -= msg_height;
        
        if (msg_y + msg_height < g_chat_area.y) break;  // Above visible area
        if (msg_y > g_chat_area.y + g_chat_area.h) continue;  // Below visible area
        
        draw_chat_message(renderer, &chat_history[i], g_chat_area.x, msg_y, g_chat_area.w);
        
        msg_y -= 10;  // Gap between messages
    }
    
    // Draw "No messages" if empty
    if (chat_message_count == 0) {
        draw_text(renderer, "Start a conversation...", 
                 g_chat_area.x + g_chat_area.w / 2 - 80,
                 g_chat_area.y + g_chat_area.h / 2,
                 (SDL_Color){100, 100, 100, 255});
    }
    
    // === INPUT AREA ===
    int input_y = WINDOW_HEIGHT - input_height;
    
    // Clear chat button
    g_clear_chat_btn = (SDL_Rect){10, input_y, 100, 25};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_clear_chat_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_clear_chat_btn);
    draw_text(renderer, "Clear Chat", g_clear_chat_btn.x + 15, g_clear_chat_btn.y + 6, text_color);
    
    // Input box
    g_input_rect = (SDL_Rect){10, input_y + 35, RENDER_WIDTH - 130, 55};
    SDL_Color input_bg = input_active ? (SDL_Color){50, 50, 70, 255} : (SDL_Color){40, 40, 50, 255};
    SDL_SetRenderDrawColor(renderer, input_bg.r, input_bg.g, input_bg.b, 255);
    SDL_RenderFillRect(renderer, &g_input_rect);
    SDL_SetRenderDrawColor(renderer, input_active ? active_color.r : text_color.r,
                           input_active ? active_color.g : text_color.g,
                           input_active ? active_color.b : text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_input_rect);
    
    // Draw input text
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
                 g_input_rect.x + 8, g_input_rect.y + 20,
                 (SDL_Color){100, 100, 100, 255});
    }
    
    // Send button
    g_send_btn = (SDL_Rect){RENDER_WIDTH - 110, input_y + 35, 100, 55};
    SDL_Color send_color = state->llm_generating ? 
        (SDL_Color){80, 80, 80, 255} : (SDL_Color){80, 150, 80, 255};
    SDL_SetRenderDrawColor(renderer, send_color.r, send_color.g, send_color.b, 255);
    SDL_RenderFillRect(renderer, &g_send_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_send_btn);
    
    const char* send_text = state->llm_generating ? "..." : "Send";
    draw_text(renderer, send_text, g_send_btn.x + 32, g_send_btn.y + 20, text_color);
}

bool handle_llm_tab_event(AppState* state, SDL_Event* event) {
    if (!state || !event) return false;
    
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int x = event->button.x;
        int y = event->button.y;
        
        // Input box click
        if (x >= g_input_rect.x && x <= g_input_rect.x + g_input_rect.w &&
            y >= g_input_rect.y && y <= g_input_rect.y + g_input_rect.h) {
            input_active = true;
            SDL_StartTextInput();
            return true;
        }
        
        // Send button
        if (x >= g_send_btn.x && x <= g_send_btn.x + g_send_btn.w &&
            y >= g_send_btn.y && y <= g_send_btn.y + g_send_btn.h) {
            
            if (!state->llm_generating && strlen(state->llm_input_text) > 0 && state->cllm_inference) {
                // Add user message to chat
                add_chat_message(state->llm_input_text, true);
                
                // Generate response (NO PRINTF - silent)
                state->llm_generating = true;
                char response[MAX_MESSAGE_LENGTH];
                int result = cllm_generate(state->cllm_inference, state->llm_input_text,
                                          response, sizeof(response));
                
                if (result > 0) {
                    // Add AI response to chat
                    add_chat_message(response, false);
                } else {
                    add_chat_message("Sorry, I couldn't generate a response.", false);
                }
                
                state->llm_generating = false;
                
                // Clear input
                state->llm_input_text[0] = '\0';
                input_cursor = 0;
            }
            return true;
        }
        
        // Clear chat button
        if (x >= g_clear_chat_btn.x && x <= g_clear_chat_btn.x + g_clear_chat_btn.w &&
            y >= g_clear_chat_btn.y && y <= g_clear_chat_btn.y + g_clear_chat_btn.h) {
            clear_chat_history();
            return true;
        }
        
        // Model button - TODO: Show model management dialog
        if (x >= g_model_btn.x && x <= g_model_btn.x + g_model_btn.w &&
            y >= g_model_btn.y && y <= g_model_btn.y + g_model_btn.h) {
            // TODO: Implement model management
            return true;
        }
        
        // Settings button - TODO: Show settings dialog
        if (x >= g_settings_btn.x && x <= g_settings_btn.x + g_settings_btn.w &&
            y >= g_settings_btn.y && y <= g_settings_btn.y + g_settings_btn.h) {
            // TODO: Implement settings
            return true;
        }
    }
    
    // Scroll in chat area
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
        // Send message on Enter
        if (strlen(state->llm_input_text) > 0 && state->cllm_inference && !state->llm_generating) {
            // Add user message
            add_chat_message(state->llm_input_text, true);
            
            // Generate response (SILENT)
            state->llm_generating = true;
            char response[MAX_MESSAGE_LENGTH];
            int result = cllm_generate(state->cllm_inference, state->llm_input_text,
                                      response, sizeof(response));
            
            if (result > 0) {
                add_chat_message(response, false);
            } else {
                add_chat_message("Sorry, I couldn't generate a response.", false);
            }
            
            state->llm_generating = false;
            
            // Clear input
            state->llm_input_text[0] = '\0';
            input_cursor = 0;
        }
    } else if (key == SDLK_ESCAPE) {
        input_active = false;
        SDL_StopTextInput();
    }
}

// Wrapper for compatibility with main.c
void handle_llm_tab_click(AppState* state, int x, int y) {
    SDL_Event event;
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.x = x;
    event.button.y = y;
    handle_llm_tab_event(state, &event);
}