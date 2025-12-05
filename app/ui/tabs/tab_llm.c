/**
 * LLM Tab - Complete Rewrite with Crystalline UI System
 * 
 * COMPLETE REWRITE using the Training Tab as reference.
 * Uses PURE Crystalline UI - NO legacy SDL_Rect code.
 * 
 * Layout:
 * - Left Area (75%): Chat interface with message history and input
 * - Right Area (25%): Control panel with model selection and parameters
 * 
 * Reference: tab_training.c (the CORRECT implementation)
 */

#include "../../app_common.h"
#include "../crystalline/elements.h"
#include "../crystalline/global_layout.h"
#include "../crystalline/draw.h"
#include "../crystalline/geometry.h"
#include "../button_sizes.h"
#include "../../cllm_integration.h"
#include "../../../include/cllm_model_manager.h"
#include "../../../include/cllm_utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_CHAT_MESSAGES 100
#define MAX_MESSAGE_LENGTH 2048

// Chat message structure
typedef struct {
    char text[MAX_MESSAGE_LENGTH];
    bool is_user;
    time_t timestamp;
} ChatMessage;

// Chat history
static ChatMessage chat_history[MAX_CHAT_MESSAGES];
static int chat_message_count = 0;

// Selected model name
static char llm_selected_model_name[256] = {0};

// UI State - Pure Crystalline UI
static struct {
    // Main interface
    CrystallineTextArea* chat_area;
    CrystallineInput* message_input;
    CrystallineButton* btn_send;
    CrystallineButton* btn_clear;
    
    // Control panel sliders
    CrystallineSlider* slider_temperature;
    CrystallineSlider* slider_tokens;
    CrystallineSlider* slider_top_k;
    CrystallineSlider* slider_top_p;
    
    // Control panel buttons
    CrystallineButton* btn_browse_models;
    CrystallineButton* btn_new_thread;
    
    // State
    bool initialized;
    bool is_generating;
    
} llm_ui = {0};

/**
 * Add message to chat history
 */
static void add_chat_message(const char* text, bool is_user) {
    if (!text || chat_message_count >= MAX_CHAT_MESSAGES) return;
    
    ChatMessage* msg = &chat_history[chat_message_count++];
    strncpy(msg->text, text, MAX_MESSAGE_LENGTH - 1);
    msg->text[MAX_MESSAGE_LENGTH - 1] = '\0';
    msg->is_user = is_user;
    msg->timestamp = time(NULL);
    
    // Add to Crystalline TextArea
    if (llm_ui.chat_area) {
        CrystallineMessageType msg_type = is_user ? 
            CRYSTALLINE_MESSAGE_USER : CRYSTALLINE_MESSAGE_ASSISTANT;
        
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
        
        crystalline_textarea_add_message(llm_ui.chat_area, msg_type, text, timestamp);
    }
}

/**
 * Clear chat history
 */
void clear_chat_history(void) {
    chat_message_count = 0;
    
    if (llm_ui.chat_area) {
        crystalline_textarea_clear(llm_ui.chat_area);
    }
}

/**
 * Button Callbacks
 */
static void on_send_clicked(void* data) {
    AppState* state = (AppState*)data;
    if (!state || !llm_ui.message_input) return;
    
    const char* input_text = crystalline_input_get_text(llm_ui.message_input);
    if (!input_text || strlen(input_text) == 0) return;
    
    printf("=== SEND BUTTON CLICKED ===\n");
    printf("Input: %s\n", input_text);
    
    // Add user message to chat
    add_chat_message(input_text, true);
    
    // Clear input
    crystalline_input_set_text(llm_ui.message_input, "");
    
    // TODO: Generate AI response
    // For now, just echo back
    char response[MAX_MESSAGE_LENGTH];
    snprintf(response, sizeof(response), "Echo: %s", input_text);
    add_chat_message(response, false);
}

static void on_clear_clicked(void* data) {
    (void)data;
    printf("=== CLEAR BUTTON CLICKED ===\n");
    clear_chat_history();
}

static void on_browse_models_clicked(void* data) {
    (void)data;
    printf("=== BROWSE MODELS BUTTON CLICKED ===\n");
    // TODO: Show model browser dialog
}

static void on_new_thread_clicked(void* data) {
    (void)data;
    printf("=== NEW THREAD BUTTON CLICKED ===\n");
    clear_chat_history();
}

/**
 * Slider Callbacks
 */
static void on_temperature_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    state->llm_temperature = value;
    printf("Temperature: %.2f\n", value);
}

static void on_tokens_changed(float value, void* data) {
    AppState* state = (AppState*)data;
    if (!state) return;
    state->llm_max_tokens = (int)value;
    printf("Max Tokens: %d\n", (int)value);
}

/**
 * Initialize LLM Tab
 * Pattern copied from init_training_tab()
 */
void init_llm_tab(AppState* state) {
    if (llm_ui.initialized) return;
    if (!state) return;
    
    printf("=== INITIALIZING LLM TAB ===\n");
    
    extern TTF_Font* get_global_font(void);
    TTF_Font* font = get_global_font();
    
    // Create layout using global layout system
    CrystallineTabLayout layout = crystalline_tab_layout_create(
        CRYSTALLINE_TAB_LAYOUT_RENDER_ONLY  // LLM uses render area only (1080px)
    );
    
    // Split: Chat area (75%) + Control panel (25%)
    CrystallineRect chat_area, control_area;
    crystalline_tab_layout_split_horizontal(&layout, 0.75f, 20.0f,
                                            &chat_area, &control_area);
    
    printf("Chat area: center=(%.1f, %.1f) size=(%.1f x %.1f)\n",
           chat_area.center.x, chat_area.center.y, chat_area.width, chat_area.height);
    printf("Control area: center=(%.1f, %.1f) size=(%.1f x %.1f)\n",
           control_area.center.x, control_area.center.y, control_area.width, control_area.height);
    
    // === CHAT AREA ===
    
    // Chat message display (top 80% of chat area)
    float chat_display_height = chat_area.height * 0.8f;
    llm_ui.chat_area = crystalline_textarea_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        chat_area.center.x,
        chat_area.center.y - chat_area.height * 0.1f,  // Shift up slightly
        chat_area.width - 20,
        chat_display_height,
        font
    );
    
    // Input field (bottom 20% of chat area)
    float input_y = chat_area.center.y + chat_area.height * 0.35f;
    llm_ui.message_input = crystalline_input_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        chat_area.center.x - 60,  // Leave room for send button
        input_y,
        chat_area.width - 140,
        60,
        "Type your message...",
        font
    );
    crystalline_input_set_callbacks(llm_ui.message_input, NULL, NULL, state);
    
    // Send button (next to input)
    llm_ui.btn_send = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        chat_area.center.x + chat_area.width/2 - 60,
        input_y,
        100,
        60,
        "SEND",
        font
    );
    crystalline_button_set_callback(llm_ui.btn_send, on_send_clicked, state);
    
    // === CONTROL PANEL ===
    
    float ctrl_x = control_area.center.x;
    float ctrl_y = control_area.center.y - control_area.height/2 + 30;
    float ctrl_w = control_area.width - 20;
    float spacing = 70;
    
    // Temperature slider
    llm_ui.slider_temperature = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        ctrl_x,
        ctrl_y,
        ctrl_w,
        30,
        0.0f,
        2.0f
    );
    crystalline_slider_set_value(llm_ui.slider_temperature, state->llm_temperature);
    crystalline_slider_set_callback(llm_ui.slider_temperature, on_temperature_changed, state);
    ctrl_y += spacing;
    
    // Max tokens slider
    llm_ui.slider_tokens = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        ctrl_x,
        ctrl_y,
        ctrl_w,
        30,
        1.0f,
        2048.0f
    );
    crystalline_slider_set_value(llm_ui.slider_tokens, (float)state->llm_max_tokens);
    crystalline_slider_set_callback(llm_ui.slider_tokens, on_tokens_changed, state);
    ctrl_y += spacing;
    
    // Top-K slider
    llm_ui.slider_top_k = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        ctrl_x,
        ctrl_y,
        ctrl_w,
        30,
        1.0f,
        100.0f
    );
    crystalline_slider_set_value(llm_ui.slider_top_k, 50.0f);
    ctrl_y += spacing;
    
    // Top-P slider
    llm_ui.slider_top_p = crystalline_slider_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        ctrl_x,
        ctrl_y,
        ctrl_w,
        30,
        0.0f,
        1.0f
    );
    crystalline_slider_set_value(llm_ui.slider_top_p, 0.9f);
    ctrl_y += spacing + 20;
    
    // Browse Models button
    llm_ui.btn_browse_models = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        ctrl_x,
        ctrl_y,
        ctrl_w,
        40,
        "Browse Models",
        font
    );
    crystalline_button_set_callback(llm_ui.btn_browse_models, on_browse_models_clicked, state);
    ctrl_y += 60;
    
    // New Thread button
    llm_ui.btn_new_thread = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        ctrl_x,
        ctrl_y,
        ctrl_w,
        40,
        "New Thread",
        font
    );
    crystalline_button_set_callback(llm_ui.btn_new_thread, on_new_thread_clicked, state);
    ctrl_y += 60;
    
    // Clear button
    llm_ui.btn_clear = crystalline_button_create(
        CRYSTALLINE_STYLE_RECTANGULAR,
        ctrl_x,
        ctrl_y,
        ctrl_w,
        40,
        "Clear Chat",
        font
    );
    crystalline_button_set_callback(llm_ui.btn_clear, on_clear_clicked, state);
    
    llm_ui.initialized = true;
    printf("=== LLM TAB INITIALIZED ===\n");
}

/**
 * Draw LLM Tab
 * Pattern copied from draw_training_tab()
 */
void draw_llm_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    // Initialize on first draw
    if (!llm_ui.initialized) {
        init_llm_tab(state);
    }
    
    // Render all Crystalline UI elements
    // NO manual SDL_RenderFillRect calls!
    
    if (llm_ui.chat_area) {
        crystalline_textarea_render(llm_ui.chat_area, renderer);
    }
    
    if (llm_ui.message_input) {
        crystalline_input_render(llm_ui.message_input, renderer);
    }
    
    if (llm_ui.btn_send) {
        crystalline_button_render(llm_ui.btn_send, renderer);
    }
    
    if (llm_ui.btn_clear) {
        crystalline_button_render(llm_ui.btn_clear, renderer);
    }
    
    if (llm_ui.slider_temperature) {
        crystalline_slider_render(llm_ui.slider_temperature, renderer);
    }
    
    if (llm_ui.slider_tokens) {
        crystalline_slider_render(llm_ui.slider_tokens, renderer);
    }
    
    if (llm_ui.slider_top_k) {
        crystalline_slider_render(llm_ui.slider_top_k, renderer);
    }
    
    if (llm_ui.slider_top_p) {
        crystalline_slider_render(llm_ui.slider_top_p, renderer);
    }
    
    if (llm_ui.btn_browse_models) {
        crystalline_button_render(llm_ui.btn_browse_models, renderer);
    }
    
    if (llm_ui.btn_new_thread) {
        crystalline_button_render(llm_ui.btn_new_thread, renderer);
    }
    
    // Draw labels for sliders
    extern void draw_text(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color);
    SDL_Color text_color = {220, 220, 220, 255};
    
    // Get control area position for labels
    CrystallineTabLayout layout = crystalline_tab_layout_create(CRYSTALLINE_TAB_LAYOUT_RENDER_ONLY);
    CrystallineRect chat_area, control_area;
    crystalline_tab_layout_split_horizontal(&layout, 0.75f, 20.0f, &chat_area, &control_area);
    
    int label_x = (int)(control_area.center.x - control_area.width/2 + 10);
    int label_y = (int)(control_area.center.y - control_area.height/2 + 10);
    
    char label[64];
    
    // Temperature label
    snprintf(label, sizeof(label), "Temperature: %.2f", 
             crystalline_slider_get_value(llm_ui.slider_temperature));
    draw_text(renderer, label, label_x, label_y, text_color);
    label_y += 70;
    
    // Max tokens label
    snprintf(label, sizeof(label), "Max Tokens: %d", 
             (int)crystalline_slider_get_value(llm_ui.slider_tokens));
    draw_text(renderer, label, label_x, label_y, text_color);
    label_y += 70;
    
    // Top-K label
    snprintf(label, sizeof(label), "Top-K: %d", 
             (int)crystalline_slider_get_value(llm_ui.slider_top_k));
    draw_text(renderer, label, label_x, label_y, text_color);
    label_y += 70;
    
    // Top-P label
    snprintf(label, sizeof(label), "Top-P: %.2f", 
             crystalline_slider_get_value(llm_ui.slider_top_p));
    draw_text(renderer, label, label_x, label_y, text_color);
}

/**
 * Event Handlers
 * Pattern copied from training tab
 */
void handle_llm_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEBUTTONDOWN;
    dummy_event.button.x = x;
    dummy_event.button.y = y;
    dummy_event.button.button = SDL_BUTTON_LEFT;
    
    // Let Crystalline UI handle events
    // NO manual click detection!
    
    if (llm_ui.btn_send && crystalline_button_handle_mouse(llm_ui.btn_send, &dummy_event)) return;
    if (llm_ui.btn_clear && crystalline_button_handle_mouse(llm_ui.btn_clear, &dummy_event)) return;
    if (llm_ui.btn_browse_models && crystalline_button_handle_mouse(llm_ui.btn_browse_models, &dummy_event)) return;
    if (llm_ui.btn_new_thread && crystalline_button_handle_mouse(llm_ui.btn_new_thread, &dummy_event)) return;
    if (llm_ui.message_input && crystalline_input_handle_mouse(llm_ui.message_input, &dummy_event)) return;
    if (llm_ui.slider_temperature && crystalline_slider_handle_mouse(llm_ui.slider_temperature, &dummy_event)) return;
    if (llm_ui.slider_tokens && crystalline_slider_handle_mouse(llm_ui.slider_tokens, &dummy_event)) return;
    if (llm_ui.slider_top_k && crystalline_slider_handle_mouse(llm_ui.slider_top_k, &dummy_event)) return;
    if (llm_ui.slider_top_p && crystalline_slider_handle_mouse(llm_ui.slider_top_p, &dummy_event)) return;
}

void handle_llm_tab_mouse_down(AppState* state, int x, int y) {
    handle_llm_tab_click(state, x, y);
}

void handle_llm_tab_mouse_up(AppState* state, int x, int y) {
    if (!state) return;
    
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEBUTTONUP;
    dummy_event.button.x = x;
    dummy_event.button.y = y;
    dummy_event.button.button = SDL_BUTTON_LEFT;
    
    if (llm_ui.slider_temperature) crystalline_slider_handle_mouse(llm_ui.slider_temperature, &dummy_event);
    if (llm_ui.slider_tokens) crystalline_slider_handle_mouse(llm_ui.slider_tokens, &dummy_event);
    if (llm_ui.slider_top_k) crystalline_slider_handle_mouse(llm_ui.slider_top_k, &dummy_event);
    if (llm_ui.slider_top_p) crystalline_slider_handle_mouse(llm_ui.slider_top_p, &dummy_event);
}

void handle_llm_tab_mouse_motion(AppState* state, int x, int y) {
    if (!state) return;
    
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_MOUSEMOTION;
    dummy_event.motion.x = x;
    dummy_event.motion.y = y;
    
    if (llm_ui.slider_temperature) crystalline_slider_handle_mouse(llm_ui.slider_temperature, &dummy_event);
    if (llm_ui.slider_tokens) crystalline_slider_handle_mouse(llm_ui.slider_tokens, &dummy_event);
    if (llm_ui.slider_top_k) crystalline_slider_handle_mouse(llm_ui.slider_top_k, &dummy_event);
    if (llm_ui.slider_top_p) crystalline_slider_handle_mouse(llm_ui.slider_top_p, &dummy_event);
}

void handle_llm_tab_keydown(AppState* state, int key) {
    if (!state) return;
    
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_KEYDOWN;
    dummy_event.key.keysym.sym = key;
    
    if (llm_ui.message_input) {
        crystalline_input_handle_keyboard(llm_ui.message_input, &dummy_event);
    }
}

void handle_llm_tab_key(AppState* state, int key) {
    handle_llm_tab_keydown(state, key);
}

void handle_llm_tab_text_input(AppState* state, const char* text) {
    if (!state || !text) return;
    
    SDL_Event dummy_event = {0};
    dummy_event.type = SDL_TEXTINPUT;
    strncpy(dummy_event.text.text, text, sizeof(dummy_event.text.text) - 1);
    
    if (llm_ui.message_input) {
        crystalline_input_handle_keyboard(llm_ui.message_input, &dummy_event);
    }
}

void handle_llm_tab_scroll(AppState* state, int wheel_y) {
    if (!state) return;
    
    // TODO: Scroll chat area
    (void)wheel_y;
}

// Input callbacks for compatibility
void llm_input_on_change(const char* text, void* user_data) {
    (void)text;
    (void)user_data;
}

void llm_input_on_submit(const char* text, void* user_data) {
    if (!text || !user_data) return;
    
    // Trigger send button
    on_send_clicked(user_data);
}