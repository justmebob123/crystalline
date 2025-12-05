// app/ui/tabs/tab_llm.c - Chat Interface with Control Panel
#include "../../app_common.h"
#include "../../cllm_integration.h"
#include "../../ui_layout.h"
#include "../../input_manager.h"
#include "../model_selector.h"
#include "../crystalline/elements.h"
#include "../button_sizes.h"
#include "../../../include/cllm_utils.h"
#include "../../../include/cllm_model_manager.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

// Forward declarations for callbacks (non-static so they can be used in input_registration.c)
void llm_input_on_change(const char* text, void* user_data);
void llm_input_on_submit(const char* text, void* user_data);
void clear_chat_history(void);

#define MAX_CHAT_MESSAGES 100
#define MAX_MESSAGE_LENGTH 2048
#define MAX_MODEL_FILES 50
#define MAX_CONVERSATION_THREADS 20

typedef struct {
    char text[MAX_MESSAGE_LENGTH];
    bool is_user;  // true = user message, false = AI message
    time_t timestamp;
} ChatMessage;

// Model file information
typedef struct {
    char filename[256];
    char full_path[512];
    size_t file_size;
    time_t modified_time;
    bool is_selected;
    // Model metadata (extracted from file if possible)
    int vocab_size;
    int num_layers;
    int embed_dim;
    int num_heads;
} ModelFileInfo;

// Model browser state
typedef struct {
    char directory_path[512];
    ModelFileInfo models[MAX_MODEL_FILES];
    int model_count;
    int selected_index;
    int scroll_offset;
    bool needs_refresh;
} ModelBrowser;

// Conversation thread
typedef struct {
    char name[128];
    ChatMessage messages[MAX_CHAT_MESSAGES];
    int message_count;
    time_t created_time;
    time_t last_modified;
    bool is_active;
} ConversationThread;

// Thread manager
typedef struct {
    ConversationThread threads[MAX_CONVERSATION_THREADS];
    int thread_count;
    int active_thread_index;
} ThreadManager;

// Chat state
static ChatMessage chat_history[MAX_CHAT_MESSAGES];
static int chat_message_count = 0;

// Model selector
static ModelSelector* llm_model_selector = NULL;

// Selected model name (NOT loaded until inference starts)
static char llm_selected_model_name[256] = {0};

// Currently loaded model name (for releasing)
static char llm_loaded_model_name[256] = {0};

// Model selector callback for LLM
static void on_llm_model_selected(const char* model_name, void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state || !model_name) return;
    
    // CRITICAL FIX: Do NOT load model here - only store the name
    // Models should only be loaded when inference actually starts
    // This prevents massive memory consumption when just browsing models
    
    printf("LLM tab: Model '%s' selected (not loaded yet)\n", model_name);
    
    // Store selected model name
    strncpy(llm_selected_model_name, model_name, sizeof(llm_selected_model_name) - 1);
    llm_selected_model_name[sizeof(llm_selected_model_name) - 1] = '\0';
    
    // Do NOT call model_manager_acquire_read() here
    // Model will be loaded on-demand when "Send" button is clicked
}
static int chat_scroll_offset = 0;

// UI state - Crystalline UI Elements
static struct {
    // Main chat interface
    CrystallineTextArea* chat_area;
    CrystallineInput* message_input;
    CrystallineButton* btn_send;
    CrystallineButton* btn_clear;
    
    // Model browser dialog
    CrystallinePanel* browser_panel;
    CrystallineList* browser_list;
    CrystallineButton* btn_browser_refresh;
    CrystallineButton* btn_browser_load;
    CrystallineButton* btn_browser_export;
    CrystallineButton* btn_browser_close;
    
    // Model size dialog
    CrystallinePanel* size_panel;
    CrystallineList* size_list;
    CrystallineButton* btn_size_cancel;
    
    // Thread list dialog
    CrystallinePanel* thread_panel;
    CrystallineList* thread_list;
    CrystallineButton* btn_thread_close;
    
    // Control buttons
    CrystallineButton* btn_browse_models;
    CrystallineButton* btn_thread_list;
    CrystallineButton* btn_new_thread;
    
    // Sliders
    CrystallineSlider* slider_temperature;
    CrystallineSlider* slider_tokens;
    CrystallineSlider* slider_top_k;
    CrystallineSlider* slider_top_p;
} llm_ui = {0};

// Model browser state
static ModelBrowser model_browser = {0};
static bool model_browser_visible = false;

// Thread manager state
static ThreadManager thread_manager = {0};
static bool thread_list_visible = false;

// Model size selection
static bool model_size_dialog_visible = false;
typedef enum {
    MODEL_SIZE_SMALL,   // 117M params
    MODEL_SIZE_MEDIUM,  // 345M params
    MODEL_SIZE_LARGE    // 762M params
} ModelSize;

// Enhanced parameters
static int top_k = 50;
static float top_p = 0.9f;
// TODO: Implement these features in Phase 4.4
// static float repetition_penalty = 1.0f;
// static float frequency_penalty = 0.0f;
// static float presence_penalty = 0.0f;
// static char stop_sequences[256] = "";
// static int random_seed = -1;  // -1 = random

// TODO: Implement generation state tracking
// static bool is_generating = false;
// static int tokens_generated = 0;
// static int tokens_total = 0;

// Store button positions
static SDL_Rect g_send_btn;
// static SDL_Rect g_clear_chat_btn;  // Removed - using Crystalline UI
static SDL_Rect g_input_rect;
static SDL_Rect g_chat_area;
static SDL_Rect g_create_btn;
static SDL_Rect g_load_btn;
static SDL_Rect g_save_btn;
static SDL_Rect g_temp_slider;

// Helper function to acquire model for inference
static bool acquire_model_for_inference(AppState* state, const char* model_name, 
                                       uint32_t vocab_size, uint32_t embedding_dim,
                                       uint32_t num_layers, uint32_t num_heads, uint32_t ff_dim) {
    // Release previous model if any
    if (state->cllm_model) {
        // TODO: Track which model name was used to release correctly
        // For now, we'll just set to NULL
        state->cllm_model = NULL;
    }
    
    // Try to acquire existing model
       // Reload model if it exists but is not loaded yet
       if (model_manager_exists(model_name)) {
           model_manager_reload(model_name);
       }

    state->cllm_model = model_manager_acquire_read(model_name);
    
    if (!state->cllm_model) {
        // Model doesn't exist, create it
        printf("Model not found, creating %s...\n", model_name);
        CLLMConfig* config = cllm_create_config(vocab_size, embedding_dim, num_layers, num_heads, ff_dim);
        
        if (config) {
            ManagedModel* managed = model_manager_create(model_name, config);
            cllm_free_config(config);
            
            if (managed) {
                state->cllm_model = model_manager_acquire_read(model_name);
            }
        }
    }
    
    if (state->cllm_model) {
        printf("✓ Model acquired for inference\n");
        if (state->cllm_inference) cllm_inference_cleanup(state->cllm_inference);
        state->cllm_inference = cllm_inference_init(state->cllm_model);
        return (state->cllm_inference != NULL);
    }
    
    printf("ERROR: Failed to acquire model\n");
    return false;
}
static SDL_Rect g_tokens_slider;
static SDL_Rect g_browse_models_btn;
// TODO: Implement model export feature
// static SDL_Rect g_export_model_btn;
// static SDL_Rect g_new_thread_btn;
static SDL_Rect g_thread_list_btn;
static SDL_Rect g_top_k_slider;
static SDL_Rect g_top_p_slider;

// Initialize model browser
static void init_model_browser(AppState* state) {
    if (!state) return;
    
    // Get models directory
    extern void workspace_get_models_dir(AppState* state, char* output, size_t output_size);
    workspace_get_models_dir(state, model_browser.directory_path, sizeof(model_browser.directory_path));
    
    model_browser.model_count = 0;
    model_browser.selected_index = -1;
    model_browser.scroll_offset = 0;
    model_browser.needs_refresh = true;
}

// Scan models directory
static void scan_models_directory(void) {
    DIR* dir = opendir(model_browser.directory_path);
    if (!dir) {
        printf("Failed to open models directory: %s\n", model_browser.directory_path);
        return;
    }
    
    model_browser.model_count = 0;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && model_browser.model_count < MAX_MODEL_FILES) {
        // Check if file ends with .cllm
        size_t len = strlen(entry->d_name);
        if (len < 5 || strcmp(entry->d_name + len - 5, ".cllm") != 0) {
            continue;
        }
        
        ModelFileInfo* info = &model_browser.models[model_browser.model_count];
        
        // Store filename
        strncpy(info->filename, entry->d_name, sizeof(info->filename) - 1);
        info->filename[sizeof(info->filename) - 1] = '\0';
        
        // Build full path (safely truncate if needed)
        int written = snprintf(info->full_path, sizeof(info->full_path), "%s/%s", 
                model_browser.directory_path, entry->d_name);
        if (written >= (int)sizeof(info->full_path)) {
            // Path was truncated, skip this file
            continue;
        }
        
        // Get file stats
        struct stat st;
        if (stat(info->full_path, &st) == 0) {
            info->file_size = st.st_size;
            info->modified_time = st.st_mtime;
        } else {
            info->file_size = 0;
            info->modified_time = 0;
        }
        
        info->is_selected = false;
        
        // TODO: Extract model metadata from file
        info->vocab_size = 0;
        info->num_layers = 0;
        info->embed_dim = 0;
        info->num_heads = 0;
        
        model_browser.model_count++;
    }
    
    closedir(dir);
    model_browser.needs_refresh = false;
    
    printf("Found %d model files in %s\n", model_browser.model_count, model_browser.directory_path);
}

// Format file size for display
static void format_file_size(size_t bytes, char* output, size_t output_size) {
    if (bytes < 1024) {
        snprintf(output, output_size, "%zu B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(output, output_size, "%.1f KB", bytes / 1024.0);
    } else if (bytes < 1024 * 1024 * 1024) {
        snprintf(output, output_size, "%.1f MB", bytes / (1024.0 * 1024.0));
    } else {
        snprintf(output, output_size, "%.1f GB", bytes / (1024.0 * 1024.0 * 1024.0));
    }
}

// Initialize thread manager
static void init_thread_manager(void) {
    thread_manager.thread_count = 1;  // Start with one default thread
    thread_manager.active_thread_index = 0;
    
    ConversationThread* thread = &thread_manager.threads[0];
    strcpy(thread->name, "Conversation 1");
    thread->message_count = 0;
    thread->created_time = time(NULL);
    thread->last_modified = time(NULL);
    thread->is_active = true;
}

// TODO: Implement thread management in Phase 4.4
// Create new conversation thread
static void create_new_thread(void) __attribute__((unused));
static void create_new_thread(void) {
    if (thread_manager.thread_count >= MAX_CONVERSATION_THREADS) {
        printf("Maximum number of threads reached\n");
        return;
    }
    
    ConversationThread* thread = &thread_manager.threads[thread_manager.thread_count];
    snprintf(thread->name, sizeof(thread->name), "Conversation %d", thread_manager.thread_count + 1);
    thread->message_count = 0;
    thread->created_time = time(NULL);
    thread->last_modified = time(NULL);
    thread->is_active = false;
    
    thread_manager.thread_count++;
}

// Switch to a different thread
static void switch_to_thread(int thread_index) __attribute__((unused));
static void switch_to_thread(int thread_index) {
    if (thread_index < 0 || thread_index >= thread_manager.thread_count) {
        return;
    }
    
    // Save current thread's messages
    if (thread_manager.active_thread_index >= 0) {
        ConversationThread* current = &thread_manager.threads[thread_manager.active_thread_index];
        current->message_count = chat_message_count;
        for (int i = 0; i < chat_message_count && i < MAX_CHAT_MESSAGES; i++) {
            current->messages[i] = chat_history[i];
        }
        current->is_active = false;
    }
    
    // Load new thread's messages
    thread_manager.active_thread_index = thread_index;
    ConversationThread* new_thread = &thread_manager.threads[thread_index];
    new_thread->is_active = true;
    new_thread->last_modified = time(NULL);
    
    chat_message_count = new_thread->message_count;
    for (int i = 0; i < new_thread->message_count && i < MAX_CHAT_MESSAGES; i++) {
        chat_history[i] = new_thread->messages[i];
    }
    chat_scroll_offset = 0;
}

// Button callbacks
static void on_send_clicked(void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state) return;
    
    printf("Send button clicked\n");
    
    // Get text from input field
    if (llm_ui.message_input && llm_ui.message_input->text[0] != '\0') {
        // Trigger the submit callback
        llm_input_on_submit(llm_ui.message_input->text, state);
        
        // Clear input field
        llm_ui.message_input->text[0] = '\0';
        llm_ui.message_input->cursor_pos = 0;
    }
}

static void on_clear_clicked(void* user_data) {
    (void)user_data;
    printf("Clear button clicked\n");
    clear_chat_history();
}

static void on_browse_models_clicked(void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state) return;
    
    printf("Browse models button clicked\n");
    if (!model_browser_visible) {
        init_model_browser(state);
        scan_models_directory();
        
        // Populate the list with model filenames
        if (llm_ui.browser_list) {
            char** items = (char**)malloc(model_browser.model_count * sizeof(char*));
            for (int i = 0; i < model_browser.model_count; i++) {
                items[i] = model_browser.models[i].filename;
            }
            crystalline_list_set_items(llm_ui.browser_list, items, model_browser.model_count);
            free(items);
        }
    }
    model_browser_visible = !model_browser_visible;
}

static void on_thread_list_clicked(void* user_data) {
    (void)user_data;
    printf("Thread list button clicked\n");
    if (thread_manager.thread_count == 0) {
        init_thread_manager();
    }
    thread_list_visible = !thread_list_visible;
}

static void on_browser_refresh_clicked(void* user_data) {
    (void)user_data;
    printf("Browser refresh button clicked\n");
    scan_models_directory();
    
    // Update list with model filenames
    if (llm_ui.browser_list) {
        char** items = (char**)malloc(model_browser.model_count * sizeof(char*));
        for (int i = 0; i < model_browser.model_count; i++) {
            items[i] = model_browser.models[i].filename;
        }
        crystalline_list_set_items(llm_ui.browser_list, items, model_browser.model_count);
        free(items);
    }
}

static void on_browser_load_clicked(void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state) return;
    
    printf("Browser load button clicked\n");
    
    if (llm_ui.browser_list) {
        int selected = crystalline_list_get_selected(llm_ui.browser_list);
        if (selected >= 0 && selected < model_browser.model_count) {
            ModelFileInfo* model = &model_browser.models[selected];
            printf("Loading model: %s\n", model->filename);
            
            // Extract model name (remove .cllm extension)
            char model_name[256];
            strncpy(model_name, model->filename, sizeof(model_name) - 1);
            model_name[sizeof(model_name) - 1] = '\0';
            char* ext = strstr(model_name, ".cllm");
            if (ext) *ext = '\0';
            
            // Load the model
            on_llm_model_selected(model_name, state);
            
            // Close browser
            model_browser_visible = false;
        }
    }
}

static void on_browser_export_clicked(void* user_data) {
    (void)user_data;
    printf("Browser export button clicked\n");
    // TODO: Implement model export functionality
}

static void on_browser_close_clicked(void* user_data) {
    (void)user_data;
    printf("Browser close button clicked\n");
    model_browser_visible = false;
}

static void on_size_selected(int index, void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state) return;
    
    printf("Model size selected: index=%d\n", index);
    
    // Model size parameters based on selection
    uint32_t vocab_sizes[] = {10000, 30000, 50000, 50000, 50000, 50000, 50000};
    uint32_t embed_dims[] = {256, 768, 1024, 1280, 1600, 2048, 4096};
    uint32_t num_layers[] = {6, 12, 24, 36, 48, 64, 32};
    uint32_t num_heads[] = {4, 12, 16, 20, 25, 32, 32};
    uint32_t ff_dims[] = {1024, 3072, 4096, 5120, 6400, 8192, 11008};
    
    if (index >= 0 && index < 7) {
        // Create model with selected size
        char model_name[64];
        const char* size_names[] = {"tiny", "small", "medium", "large", "huge", "massive", "astronomical"};
        snprintf(model_name, sizeof(model_name), "model_%s", size_names[index]);
        
        printf("Creating model: %s\n", model_name);
        printf("  Vocab: %u, Embed: %u, Layers: %u, Heads: %u, FF: %u\n",
               vocab_sizes[index], embed_dims[index], num_layers[index], 
               num_heads[index], ff_dims[index]);
        
        // TODO: Actually create the model using these parameters
        // For now, just close the dialog
        model_size_dialog_visible = false;
    }
}

static void on_size_cancel_clicked(void* user_data) {
    (void)user_data;
    printf("Size dialog cancel button clicked\n");
    model_size_dialog_visible = false;
}

static void on_thread_selected(int index, void* user_data) {
    (void)user_data;
    printf("Thread selected: index=%d\n", index);
    
    if (index >= 0 && index < thread_manager.thread_count) {
        // Switch to selected thread
        thread_manager.active_thread_index = index;
        
        // Load thread messages into chat history
        ConversationThread* thread = &thread_manager.threads[index];
        chat_message_count = thread->message_count;
        for (int i = 0; i < thread->message_count && i < MAX_CHAT_MESSAGES; i++) {
            chat_history[i] = thread->messages[i];
        }
        
        // Update Crystalline TextArea
        if (llm_ui.chat_area) {
            crystalline_textarea_clear(llm_ui.chat_area);
            for (int i = 0; i < thread->message_count; i++) {
                CrystallineMessageType msg_type = thread->messages[i].is_user ? 
                    CRYSTALLINE_MESSAGE_USER : CRYSTALLINE_MESSAGE_ASSISTANT;
                
                time_t timestamp = thread->messages[i].timestamp;
                struct tm* tm_info = localtime(&timestamp);
                char time_str[32];
                strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
                
                crystalline_textarea_add_message(llm_ui.chat_area, 
                    msg_type, thread->messages[i].text, time_str);
            }
        }
        
        printf("Switched to thread: %s (%d messages)\n", thread->name, thread->message_count);
        thread_list_visible = false;
    }
}

static void on_thread_close_clicked(void* user_data) {
    (void)user_data;
    printf("Thread list close button clicked\n");
    thread_list_visible = false;
}

// Add message to chat history
void add_chat_message(const char* text, bool is_user) {
    // Add to old array for compatibility
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
    
    // Add to Crystalline TextArea
    if (llm_ui.chat_area) {
        CrystallineMessageType msg_type = is_user ? 
            CRYSTALLINE_MESSAGE_USER : CRYSTALLINE_MESSAGE_ASSISTANT;
        
        // Format timestamp
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
        
        crystalline_textarea_add_message(llm_ui.chat_area, msg_type, text, timestamp);
    }
    
    // Auto-scroll to bottom
    chat_scroll_offset = 0;
}

// Clear chat history
void clear_chat_history(void) {
    chat_message_count = 0;
    chat_scroll_offset = 0;
    
    // Clear Crystalline TextArea
    if (llm_ui.chat_area) {
        crystalline_textarea_clear(llm_ui.chat_area);
    }
}

void draw_llm_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    int panel_x = RENDER_OFFSET_X + RENDER_WIDTH;  // Fixed: account for sidebar
    int panel_y = RENDER_OFFSET_Y;  // Fixed: account for submenu
    int panel_width = CONTROL_PANEL_WIDTH;
    
    // Initialize model selector on first draw
    if (!llm_model_selector) {
        llm_model_selector = model_selector_create(panel_x + 10, panel_y + 50, panel_width - 20, 30);
        model_selector_update_list(llm_model_selector);
        
        // Set callback to load model when selected
        model_selector_set_callback(llm_model_selector, on_llm_model_selected, state);
        
        // Initialize model browser state
        init_model_browser(state);
        init_thread_manager();
    }
    
    // Initialize Crystalline UI elements on first draw
    if (!llm_ui.chat_area) {
        // Get font for UI elements
        extern TTF_Font* get_global_font(void);
        TTF_Font* font = get_global_font();
        
        // Chat area - main message display
        // Use RENDER_WIDTH to account for control panel on right
        int content_width = RENDER_WIDTH;  // 1080px (accounts for sidebar + control panel)
        int chat_x = RENDER_OFFSET_X;  // 200px (after sidebar)
        int chat_y = RENDER_OFFSET_Y;  // 40px (after submenu)
        int chat_width = content_width - 20;  // Leave 20px margin
        int chat_height = WINDOW_HEIGHT - RENDER_OFFSET_Y - 120;  // Leave room for input
        
        llm_ui.chat_area = crystalline_textarea_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            chat_x + chat_width / 2.0f,
            chat_y + chat_height / 2.0f,
            chat_width,
            chat_height,
            font
        );
        
        // Message input field (FIXED AT BOTTOM)
        int input_y = WINDOW_HEIGHT - 70;  // Fixed position at bottom of screen
        int input_width = chat_width - 120;
        
        llm_ui.message_input = crystalline_input_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            chat_x + input_width / 2.0f,
            input_y + 20.0f,
            input_width,
            40.0f,
            "Type your message...",
            font
        );
        
        // Send button (BOTTOM LEFT, next to input)
        llm_ui.btn_send = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            chat_x + input_width + 30.0f,
            input_y + 20.0f,
            BUTTON_RADIUS_SECONDARY,
            0.0f,
            "SEND",
            font
        );
        crystalline_button_set_callback(llm_ui.btn_send, on_send_clicked, state);
        
        // Clear button (BOTTOM LEFT, next to send)
        llm_ui.btn_clear = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            chat_x + input_width + 80.0f,
            input_y + 20.0f,
            BUTTON_RADIUS_TERTIARY,
            0.0f,
            "CLEAR",
            font
        );
        crystalline_button_set_callback(llm_ui.btn_clear, on_clear_clicked, state);
        
        // Model Browser Panel (centered, created but not visible initially)
        int browser_w = 600;
        int browser_h = 500;
        int browser_x = WINDOW_WIDTH / 2;
        int browser_y = WINDOW_HEIGHT / 2;
        
        llm_ui.browser_panel = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            browser_x,
            browser_y,
            browser_w,
            browser_h,
            "MODEL BROWSER",
            font
        );
        
        // Model list inside browser panel
        llm_ui.browser_list = crystalline_list_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            browser_x,
            browser_y + 40.0f,
            browser_w - 40.0f,
            browser_h - 150.0f,
            font
        );
        
        // Browser buttons
        float btn_y = browser_y + browser_h / 2.0f - 30.0f;
        float btn_spacing = 110.0f;
        float btn_start_x = browser_x - btn_spacing * 1.5f;
        
        llm_ui.btn_browser_refresh = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x,
            btn_y,
            BUTTON_RADIUS_TERTIARY,
            0.0f,
            "REFRESH",
            font
        );
        crystalline_button_set_callback(llm_ui.btn_browser_refresh, on_browser_refresh_clicked, state);
        
        llm_ui.btn_browser_load = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x + btn_spacing,
            btn_y,
            BUTTON_RADIUS_TERTIARY,
            0.0f,
            "LOAD",
            font
        );
        crystalline_button_set_callback(llm_ui.btn_browser_load, on_browser_load_clicked, state);
        
        llm_ui.btn_browser_export = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x + btn_spacing * 2,
            btn_y,
            BUTTON_RADIUS_TERTIARY,
            0.0f,
            "EXPORT",
            font
        );
        crystalline_button_set_callback(llm_ui.btn_browser_export, on_browser_export_clicked, state);
        
        llm_ui.btn_browser_close = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            btn_start_x + btn_spacing * 3,
            btn_y,
            BUTTON_RADIUS_TERTIARY,
            0.0f,
            "CLOSE",
            font
        );
        crystalline_button_set_callback(llm_ui.btn_browser_close, on_browser_close_clicked, state);
        
        // Model Size Dialog (centered, created but not visible initially)
        int size_w = 500;
        int size_h = 600;
        int size_x = WINDOW_WIDTH / 2;
        int size_y = WINDOW_HEIGHT / 2;
        
        llm_ui.size_panel = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            size_x,
            size_y,
            size_w,
            size_h,
            "SELECT MODEL SIZE",
            font
        );
        
        // Model size list with 7 options
        llm_ui.size_list = crystalline_list_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            size_x,
            size_y + 20.0f,
            size_w - 40.0f,
            size_h - 120.0f,
            font
        );
        
        // Populate size list with 7 model sizes
        char* size_options[] = {
            "TINY - 25M params (RECOMMENDED)",
            "SMALL - 117M params",
            "MEDIUM - 345M params (RECOMMENDED)",
            "LARGE - 762M params",
            "HUGE - 1.5B params",
            "MASSIVE - 3B params",
            "ASTRONOMICAL - 7B params"
        };
        crystalline_list_set_items(llm_ui.size_list, size_options, 7);
        crystalline_list_set_callback(llm_ui.size_list, on_size_selected, state);
        
        // Cancel button for size dialog
        llm_ui.btn_size_cancel = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            size_x,
            size_y + size_h / 2.0f - 30.0f,
            BUTTON_RADIUS_TERTIARY,
            0.0f,
            "CANCEL",
            font
        );
        crystalline_button_set_callback(llm_ui.btn_size_cancel, on_size_cancel_clicked, state);
        
        // Thread List Panel (centered, created but not visible initially)
        int thread_w = 400;
        int thread_h = 450;
        int thread_x = WINDOW_WIDTH / 2;
        int thread_y = WINDOW_HEIGHT / 2;
        
        llm_ui.thread_panel = crystalline_panel_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            thread_x,
            thread_y,
            thread_w,
            thread_h,
            "CONVERSATIONS",
            font
        );
        
        // Thread list
        llm_ui.thread_list = crystalline_list_create(
            CRYSTALLINE_STYLE_RECTANGULAR,
            thread_x,
            thread_y + 20.0f,
            thread_w - 40.0f,
            thread_h - 120.0f,
            font
        );
        crystalline_list_set_callback(llm_ui.thread_list, on_thread_selected, state);
        
        // Thread list close button
        llm_ui.btn_thread_close = crystalline_button_create(
            CRYSTALLINE_STYLE_CIRCULAR,
            thread_x,
            thread_y + thread_h / 2.0f - 30.0f,
            BUTTON_RADIUS_TERTIARY,
            0.0f,
            "CLOSE",
            font
        );
        crystalline_button_set_callback(llm_ui.btn_thread_close, on_thread_close_clicked, state);
    }
    
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {40, 40, 50, 255};
    SDL_Color button_color = {60, 60, 80, 255};
    SDL_Color active_color = {100, 150, 200, 255};
    
    // === CONTROL PANEL ===
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_Rect panel_rect = {panel_x, panel_y, panel_width, WINDOW_HEIGHT - panel_y};
    SDL_RenderFillRect(renderer, &panel_rect);
    
    // Initialize dynamic layout
    LayoutContainer layout;
    layout_init(&layout, panel_rect, LAYOUT_VERTICAL, 10, 8);
    
    // === SECTION 0: MODEL SELECTOR ===
    SDL_Rect selector_label = layout_add_label(&layout, "SELECT MODEL", 20);
    draw_text(renderer, "SELECT MODEL", selector_label.x, selector_label.y, text_color);
    
    // Render model selector
    if (llm_model_selector) {
        model_selector_render(llm_model_selector, renderer);
    }
    layout_add_spacing(&layout, 40); // Space for model selector
    layout_add_spacing(&layout, 10);
    
    // === SECTION 1: MODEL STATUS ===
    SDL_Rect model_label = layout_add_label(&layout, "CLLM MODEL", 20);
    draw_text(renderer, "CLLM MODEL", model_label.x, model_label.y, text_color);
    
    const char* status = state->cllm_model ? "Loaded" : "Not Loaded";
    SDL_Color status_color = state->cllm_model ? 
        (SDL_Color){100, 255, 100, 255} : (SDL_Color){255, 100, 100, 255};
    char status_text[64];
    snprintf(status_text, sizeof(status_text), "Status: %s", status);
    SDL_Rect status_rect = layout_add_label(&layout, status_text, 18);
    draw_text(renderer, status_text, status_rect.x, status_rect.y, status_color);
    
    if (state->cllm_model) {
        char info[128];
        snprintf(info, sizeof(info), "Vocab: %lu", (unsigned long)state->cllm_model->vocab_size);
        SDL_Rect vocab_rect = layout_add_label(&layout, info, 16);
        draw_text(renderer, info, vocab_rect.x, vocab_rect.y, text_color);
        
        snprintf(info, sizeof(info), "Layers: %u", state->cllm_model->num_layers);
        SDL_Rect layers_rect = layout_add_label(&layout, info, 16);
        draw_text(renderer, info, layers_rect.x, layers_rect.y, text_color);
    }
    
    layout_add_spacing(&layout, 10);
    
    // Model buttons row
    SDL_Rect btn_row = layout_add_element(&layout, 0, 28);
    int button_width = (panel_width - 30) / 2;
    
    g_create_btn = (SDL_Rect){btn_row.x, btn_row.y, button_width, 28};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_create_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_create_btn);
    draw_text(renderer, "Create", g_create_btn.x + 25, g_create_btn.y + 7, text_color);
    
    g_load_btn = (SDL_Rect){btn_row.x + button_width + 10, btn_row.y, button_width, 28};
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_load_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_load_btn);
    draw_text(renderer, "Load", g_load_btn.x + 30, g_load_btn.y + 7, text_color);
    
    if (state->cllm_model) {
        g_save_btn = layout_add_button(&layout, NULL, 0, 28);
        SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
        SDL_RenderFillRect(renderer, &g_save_btn);
        SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
        SDL_RenderDrawRect(renderer, &g_save_btn);
        draw_text(renderer, "Save Model", g_save_btn.x + 70, g_save_btn.y + 7, text_color);
    }
    
    layout_add_spacing(&layout, 10);
    
    // === SECTION 2: PARAMETERS ===
    SDL_Rect params_label = layout_add_label(&layout, "PARAMETERS", 20);
    draw_text(renderer, "PARAMETERS", params_label.x, params_label.y, text_color);
    
    // Temperature slider
    char temp_label[64];
    snprintf(temp_label, sizeof(temp_label), "Temperature: %.2f", state->llm_temperature);
    SDL_Rect temp_label_rect = layout_add_label(&layout, temp_label, 16);
    draw_text(renderer, temp_label, temp_label_rect.x, temp_label_rect.y, text_color);
    
    g_temp_slider = layout_add_element(&layout, 0, 8);
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(renderer, &g_temp_slider);
    int temp_handle_x = g_temp_slider.x + (int)((state->llm_temperature / 2.0f) * g_temp_slider.w);
    SDL_Rect temp_handle = {temp_handle_x - 4, g_temp_slider.y - 4, 8, 16};
    SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
    SDL_RenderFillRect(renderer, &temp_handle);
    
    layout_add_spacing(&layout, 5);
    
    // Max tokens slider
    char tokens_label[64];
    snprintf(tokens_label, sizeof(tokens_label), "Max Tokens: %d", state->llm_max_tokens);
    SDL_Rect tokens_label_rect = layout_add_label(&layout, tokens_label, 16);
    draw_text(renderer, tokens_label, tokens_label_rect.x, tokens_label_rect.y, text_color);
    
    g_tokens_slider = layout_add_element(&layout, 0, 8);
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(renderer, &g_tokens_slider);
    int tokens_handle_x = g_tokens_slider.x + (int)((float)state->llm_max_tokens / 2048.0f * g_tokens_slider.w);
    SDL_Rect tokens_handle = {tokens_handle_x - 4, g_tokens_slider.y - 4, 8, 16};
    SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
    SDL_RenderFillRect(renderer, &tokens_handle);
    
    layout_add_spacing(&layout, 5);
    
    // Top-K slider
    char top_k_label[64];
    snprintf(top_k_label, sizeof(top_k_label), "Top-K: %d", top_k);
    SDL_Rect top_k_label_rect = layout_add_label(&layout, top_k_label, 16);
    draw_text(renderer, top_k_label, top_k_label_rect.x, top_k_label_rect.y, text_color);
    
    g_top_k_slider = layout_add_element(&layout, 0, 8);
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(renderer, &g_top_k_slider);
    int top_k_handle_x = g_top_k_slider.x + (int)((float)top_k / 100.0f * g_top_k_slider.w);
    SDL_Rect top_k_handle = {top_k_handle_x - 4, g_top_k_slider.y - 4, 8, 16};
    SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
    SDL_RenderFillRect(renderer, &top_k_handle);
    
    layout_add_spacing(&layout, 5);
    
    // Top-P slider
    char top_p_label[64];
    snprintf(top_p_label, sizeof(top_p_label), "Top-P: %.2f", top_p);
    SDL_Rect top_p_label_rect = layout_add_label(&layout, top_p_label, 16);
    draw_text(renderer, top_p_label, top_p_label_rect.x, top_p_label_rect.y, text_color);
    
    g_top_p_slider = layout_add_element(&layout, 0, 8);
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(renderer, &g_top_p_slider);
    int top_p_handle_x = g_top_p_slider.x + (int)(top_p * g_top_p_slider.w);
    SDL_Rect top_p_handle = {top_p_handle_x - 4, g_top_p_slider.y - 4, 8, 16};
    SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
    SDL_RenderFillRect(renderer, &top_p_handle);
    
    layout_add_spacing(&layout, 10);
    
    // Model browser button
    g_browse_models_btn = layout_add_button(&layout, NULL, 0, 25);
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_browse_models_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_browse_models_btn);
    draw_text(renderer, "Browse Models", g_browse_models_btn.x + 55, g_browse_models_btn.y + 6, text_color);
    
    // Thread list button
    g_thread_list_btn = layout_add_button(&layout, NULL, 0, 25);
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_thread_list_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_thread_list_btn);
    draw_text(renderer, "Conversations", g_thread_list_btn.x + 55, g_thread_list_btn.y + 6, text_color);
    
    // Legacy clear chat button removed - now using Crystalline UI button
    
    // === CHAT AREA (LEFT SIDE) ===
    int chat_width = RENDER_WIDTH - 20;  // Width is correct
    int input_height = 80;
    int chat_height = WINDOW_HEIGHT - 60 - input_height - 20;
    
    // Render chat area using Crystalline UI TextArea
    if (llm_ui.chat_area) {
        crystalline_textarea_render(llm_ui.chat_area, renderer);
    }
    
    // Store bounds for compatibility with old code
    g_chat_area = (SDL_Rect){RENDER_OFFSET_X + 10, RENDER_OFFSET_Y + 10, chat_width, chat_height};
    
    // === INPUT AREA ===
    // NOTE: Input field is now rendered by InputManager (see below)
    // Just set the bounds for the send button positioning
    int input_y = WINDOW_HEIGHT - input_height - 10;
    g_input_rect = (SDL_Rect){RENDER_OFFSET_X + 10, input_y, chat_width - 120, input_height};
    
    // Placeholder text is handled by InputManager - no manual drawing needed
    
    // Render input field and buttons using Crystalline UI
    if (llm_ui.message_input) {
        crystalline_input_render(llm_ui.message_input, renderer);
    }
    
    if (llm_ui.btn_send) {
        // Update button label based on generation state
        const char* send_text = state->llm_generating ? "..." : "SEND";
        strncpy(llm_ui.btn_send->label, send_text, sizeof(llm_ui.btn_send->label) - 1);
        crystalline_button_render(llm_ui.btn_send, renderer);
    }
    
    if (llm_ui.btn_clear) {
        crystalline_button_render(llm_ui.btn_clear, renderer);
    }
    
    // Store bounds for compatibility with old code
    g_send_btn = (SDL_Rect){g_input_rect.x + g_input_rect.w + 10, input_y, 100, input_height};
    
    // Draw overlay panels if visible
    if (model_size_dialog_visible) {
        // Semi-transparent overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        
        // Render Model Size Dialog using Crystalline UI
        if (llm_ui.size_panel) {
            crystalline_panel_render(llm_ui.size_panel, renderer);
        }
        
        if (llm_ui.size_list) {
            crystalline_list_render(llm_ui.size_list, renderer);
        }
        
        if (llm_ui.btn_size_cancel) {
            crystalline_button_render(llm_ui.btn_size_cancel, renderer);
        }
    }
    
    if (model_browser_visible) {
        // Semi-transparent overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        
        // Render Model Browser using Crystalline UI
        if (llm_ui.browser_panel) {
            crystalline_panel_render(llm_ui.browser_panel, renderer);
        }
        
        if (llm_ui.browser_list) {
            // Update list with current models if needed
            if (model_browser.needs_refresh) {
                char** items = (char**)malloc(model_browser.model_count * sizeof(char*));
                for (int i = 0; i < model_browser.model_count; i++) {
                    items[i] = model_browser.models[i].filename;
                }
                crystalline_list_set_items(llm_ui.browser_list, items, model_browser.model_count);
                free(items);
                model_browser.needs_refresh = false;
            }
            
            crystalline_list_render(llm_ui.browser_list, renderer);
        }
        
        if (llm_ui.btn_browser_refresh) {
            crystalline_button_render(llm_ui.btn_browser_refresh, renderer);
        }
        
        if (llm_ui.btn_browser_load) {
            crystalline_button_render(llm_ui.btn_browser_load, renderer);
        }
        
        if (llm_ui.btn_browser_export) {
            crystalline_button_render(llm_ui.btn_browser_export, renderer);
        }
        
        if (llm_ui.btn_browser_close) {
            crystalline_button_render(llm_ui.btn_browser_close, renderer);
        }
    }
    
    if (thread_list_visible) {
        // Semi-transparent overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        
        // Render Thread List using Crystalline UI
        if (llm_ui.thread_panel) {
            crystalline_panel_render(llm_ui.thread_panel, renderer);
        }
        
        if (llm_ui.thread_list) {
            // Update list with current threads
            char** thread_names = (char**)malloc(thread_manager.thread_count * sizeof(char*));
            for (int i = 0; i < thread_manager.thread_count; i++) {
                thread_names[i] = thread_manager.threads[i].name;
            }
            crystalline_list_set_items(llm_ui.thread_list, thread_names, thread_manager.thread_count);
            free(thread_names);
            
            // Set selected to active thread
            crystalline_list_set_selected(llm_ui.thread_list, thread_manager.active_thread_index);
            
            crystalline_list_render(llm_ui.thread_list, renderer);
        }
        
        if (llm_ui.btn_thread_close) {
            crystalline_button_render(llm_ui.btn_thread_close, renderer);
        }
    }
    
    // Draw loading indicator if generating
    if (state->llm_generating) {
        int indicator_x = RENDER_WIDTH / 2 - 100;
        int indicator_y = WINDOW_HEIGHT - 150;
        SDL_Rect indicator_bg = {indicator_x, indicator_y, 200, 40};
        SDL_SetRenderDrawColor(renderer, 40, 40, 50, 230);
        SDL_RenderFillRect(renderer, &indicator_bg);
        SDL_SetRenderDrawColor(renderer, 100, 150, 200, 255);
        SDL_RenderDrawRect(renderer, &indicator_bg);
        
        draw_text(renderer, "Thinking...", indicator_x + 60, indicator_y + 12, 
                 (SDL_Color){150, 200, 255, 255});
    }
    
    // Render all inputs through InputManager
    extern InputManager* g_input_manager;
    extern TTF_Font* get_global_font(void);
    if (g_input_manager) {
        input_manager_render(g_input_manager, renderer, get_global_font(), TAB_LLM);
    }
}

void handle_llm_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    // Handle Crystalline UI button clicks
    SDL_Event dummy_event;
    dummy_event.type = SDL_MOUSEBUTTONUP;
    dummy_event.button.x = x;
    dummy_event.button.y = y;
    
    if (llm_ui.btn_send && crystalline_button_handle_mouse(llm_ui.btn_send, &dummy_event)) {
        return;
    }
    
    if (llm_ui.btn_clear && crystalline_button_handle_mouse(llm_ui.btn_clear, &dummy_event)) {
        return;
    }
    
    // Handle TextArea clicks (for scrolling)
    if (llm_ui.chat_area && crystalline_textarea_handle_event(llm_ui.chat_area, &dummy_event)) {
        return;
    }
    
    // Check model selector click first
    if (llm_model_selector && model_selector_handle_click(llm_model_selector, x, y)) {
        return;
    }
    
    // Handle model browser panel clicks
    if (model_browser_visible) {
        // Handle browser button clicks
        if (llm_ui.btn_browser_refresh && crystalline_button_handle_mouse(llm_ui.btn_browser_refresh, &dummy_event)) {
            return;
        }
        
        if (llm_ui.btn_browser_load && crystalline_button_handle_mouse(llm_ui.btn_browser_load, &dummy_event)) {
            return;
        }
        
        if (llm_ui.btn_browser_export && crystalline_button_handle_mouse(llm_ui.btn_browser_export, &dummy_event)) {
            return;
        }
        
        if (llm_ui.btn_browser_close && crystalline_button_handle_mouse(llm_ui.btn_browser_close, &dummy_event)) {
            return;
        }
        
        // Handle list clicks
        if (llm_ui.browser_list && crystalline_list_handle_mouse(llm_ui.browser_list, &dummy_event)) {
            return;
        }
        
        // Click outside panel - close it
        int panel_w = 600;
        int panel_h = 500;
        int panel_x = (WINDOW_WIDTH - panel_w) / 2;
        int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
        
        if (x < panel_x || x > panel_x + panel_w ||
            y < panel_y || y > panel_y + panel_h) {
            model_browser_visible = false;
        }
        
        return;
    }
    
    // NOTE: Input box clicks are handled by InputManager - no need for duplicate handler here
    
    // Handle thread list panel clicks using Crystalline UI
    if (thread_list_visible) {
        // Handle close button click
        if (llm_ui.btn_thread_close && crystalline_button_handle_mouse(llm_ui.btn_thread_close, &dummy_event)) {
            return;
        }
        
        // Handle list selection
        if (llm_ui.thread_list && crystalline_list_handle_mouse(llm_ui.thread_list, &dummy_event)) {
            return;
        }
        
        // Click outside panel - close it
        int panel_w = 400;
        int panel_h = 450;
        int panel_x = (WINDOW_WIDTH - panel_w) / 2;
        int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
        
        if (x < panel_x || x > panel_x + panel_w ||
            y < panel_y || y > panel_y + panel_h) {
            thread_list_visible = false;
        }
        
        return;
    }
    
    // Browse Models button
    if (x >= g_browse_models_btn.x && x <= g_browse_models_btn.x + g_browse_models_btn.w &&
        y >= g_browse_models_btn.y && y <= g_browse_models_btn.y + g_browse_models_btn.h) {
        if (!model_browser_visible) {
            init_model_browser(state);
            scan_models_directory();
        }
        model_browser_visible = !model_browser_visible;
        return;
    }
    
    // Thread List button
    if (x >= g_thread_list_btn.x && x <= g_thread_list_btn.x + g_thread_list_btn.w &&
        y >= g_thread_list_btn.y && y <= g_thread_list_btn.y + g_thread_list_btn.h) {
        if (thread_manager.thread_count == 0) {
            init_thread_manager();
        }
        thread_list_visible = !thread_list_visible;
        return;
    }
    
    // Send button - trigger submit callback manually
    if (x >= g_send_btn.x && x <= g_send_btn.x + g_send_btn.w &&
        y >= g_send_btn.y && y <= g_send_btn.y + g_send_btn.h) {
        
        // Get current input text from global InputManager
        extern InputManager* g_input_manager;
        const char* input_text = input_manager_get_text(g_input_manager, "llm.chat_input");
        if (input_text && strlen(input_text) > 0) {
            // Trigger submit callback
            llm_input_on_submit(input_text, state);
        }
        return;
    }
    
    // Clear chat
    // Legacy clear button click handling removed - now handled by Crystalline UI
    
    // Handle model size dialog clicks using Crystalline UI
    if (model_size_dialog_visible) {
        // Handle cancel button click
        if (llm_ui.btn_size_cancel && crystalline_button_handle_mouse(llm_ui.btn_size_cancel, &dummy_event)) {
            return;
        }
        
        // Handle list selection
        if (llm_ui.size_list && crystalline_list_handle_mouse(llm_ui.size_list, &dummy_event)) {
            return;
        }
        
        // Click outside dialog - close it
        int panel_w = 500;
        int panel_h = 600;
        int panel_x = (WINDOW_WIDTH - panel_w) / 2;
        int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
        
        // Click outside dialog - close it
        if (x < panel_x || x > panel_x + panel_w ||
            y < panel_y || y > panel_y + panel_h) {
            model_size_dialog_visible = false;
        }
        return;
    }

    // Create button - show size selection dialog
    if (x >= g_create_btn.x && x <= g_create_btn.x + g_create_btn.w &&
        y >= g_create_btn.y && y <= g_create_btn.y + g_create_btn.h) {
        model_size_dialog_visible = true;
        return;
    }
    
    // Load button
    if (x >= g_load_btn.x && x <= g_load_btn.x + g_load_btn.w &&
        y >= g_load_btn.y && y <= g_load_btn.y + g_load_btn.h) {
        printf("Loading CLLM model...\n");
        
        // Use model manager to get the selected model
        extern CLLMModel* model_manager_acquire_read(const char* name);
        extern void model_manager_release_read(const char* name);
        extern const char* model_manager_get_first_name(void);
        extern bool model_manager_prepare(const char* name);
        extern bool model_manager_reload(const char* name);
        
        // Get model name from selector or use first available
        const char* model_name = llm_selected_model_name[0] ? llm_selected_model_name : model_manager_get_first_name();
        
        if (model_name) {
            // Prepare and load the model
            if (model_manager_prepare(model_name)) {
                if (model_manager_reload(model_name)) {
                    CLLMModel* loaded = model_manager_acquire_read(model_name);
                    if (loaded) {
                        // Release old model if any
                        if (state->cllm_model && llm_loaded_model_name[0]) {
                            model_manager_release_read(llm_loaded_model_name);
                        }
                        
                        state->cllm_model = loaded;
                        strncpy(llm_loaded_model_name, model_name, sizeof(llm_loaded_model_name) - 1);
                        
                        if (state->cllm_inference) {
                            cllm_inference_cleanup(state->cllm_inference);
                        }
                        state->cllm_inference = cllm_inference_init(state->cllm_model);
                        printf("✓ Model loaded: %s\n", model_name);
                    } else {
                        fprintf(stderr, "Failed to acquire model: %s\n", model_name);
                    }
                } else {
                    fprintf(stderr, "Failed to load model into memory: %s\n", model_name);
                }
            } else {
                fprintf(stderr, "Failed to prepare model: %s\n", model_name);
            }
        } else {
            fprintf(stderr, "No models available\n");
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
        state->llm_max_tokens = (int)(ratio * 2048);  // Increased from 500 to 2048
        if (state->llm_max_tokens < 1) state->llm_max_tokens = 1;
        if (state->llm_max_tokens > 2048) state->llm_max_tokens = 2048;
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

// Callback for when text changes in the input box
void llm_input_on_change(const char* text, void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state || !text) return;
    
    // Update the state's input text
    strncpy(state->llm_input_text, text, sizeof(state->llm_input_text) - 1);
    state->llm_input_text[sizeof(state->llm_input_text) - 1] = '\0';
}

// Callback for when Enter is pressed (submit)
void llm_input_on_submit(const char* text, void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state || !text || strlen(text) == 0) return;
    if (state->llm_generating) return;
    
    // CRITICAL FIX: Load model on-demand if not already loaded
    if (!state->cllm_inference && llm_selected_model_name[0]) {
        printf("Loading model '%s' for inference...\n", llm_selected_model_name);
        
        // Use default model size for now (TODO: get from model metadata)
        if (!acquire_model_for_inference(state, llm_selected_model_name, 
                                        10000, 512, 6, 8, 2048)) {
            add_chat_message("Error: Failed to load model. Please select a model first.", false);
            return;
        }
        
        printf("Model loaded successfully\n");
    }
    
    if (!state->cllm_inference) {
        add_chat_message("Error: No model loaded. Please select a model first.", false);
        return;
    }
    
    // Add user message to chat
    add_chat_message(text, true);
    
    // Generate response
    state->llm_generating = true;
    char response[MAX_MESSAGE_LENGTH];
    
    printf("=== GENERATING RESPONSE ===\n");
    printf("Prompt: %s\n", text);
    
    int result = cllm_generate(state->cllm_inference, text,
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
    
    // Clear input through global InputManager
    extern InputManager* g_input_manager;
    input_manager_set_text(g_input_manager, "llm.chat_input", "");
    state->llm_input_text[0] = '\0';
}

// Legacy functions - now just stubs that do nothing (InputManager handles everything)
void handle_llm_tab_text_input(AppState* state, const char* text) {
    (void)state;
    (void)text;
    // InputManager handles this now
}

void handle_llm_tab_key(AppState* state, SDL_Keycode key) {
    (void)state;
    (void)key;
    // InputManager handles this now
}