// app/ui/tabs/tab_llm.c - Chat Interface with Control Panel
#include "../../app_common.h"
#include "../../cllm_integration.h"
#include "../../ui_layout.h"
#include "../../input_manager.h"
#include "../model_selector.h"
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

// Model selector callback for LLM
static void on_llm_model_selected(const char* model_name, void* user_data) {
    AppState* state = (AppState*)user_data;
    if (!state || !model_name) return;
    
    printf("LLM tab: Loading model '%s'\n", model_name);
    
    // Acquire new model for inference (read access)
    state->cllm_model = model_manager_acquire_read(model_name);
    
    if (state->cllm_model) {
        printf("LLM: Model '%s' loaded successfully\n", model_name);
    } else {
        printf("LLM: Failed to load model '%s'\n", model_name);
    }
}
static int chat_scroll_offset = 0;

// UI state
static bool input_active = false;
// Removed unused variable - input_cursor (now managed by InputManager)

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
static SDL_Rect g_clear_chat_btn;
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

// Draw model browser panel
static void draw_model_browser_panel(SDL_Renderer* renderer, int x, int y, int width, int height) {
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {30, 30, 40, 255};
    SDL_Color selected_color = {60, 100, 140, 255};
    // TODO: Implement hover effects in Phase 4.4
    // SDL_Color hover_color = {50, 50, 60, 255};
    (void)selected_color; // Will be used when selection is implemented
    
    // Draw panel background
    SDL_Rect panel = {x, y, width, height};
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
    SDL_RenderDrawRect(renderer, &panel);
    
    // Title
    draw_text(renderer, "MODEL BROWSER", x + 10, y + 10, (SDL_Color){100, 150, 200, 255});
    
    // Directory path (safely truncate if needed)
    char dir_text[128];
    int dir_written = snprintf(dir_text, sizeof(dir_text), "Path: %s", model_browser.directory_path);
    if (dir_written >= (int)sizeof(dir_text)) {
        // Path was truncated, add ellipsis
        strcpy(dir_text + sizeof(dir_text) - 4, "...");
    }
    draw_text(renderer, dir_text, x + 10, y + 35, text_color);
    
    // Refresh button
    SDL_Rect refresh_btn = {x + width - 90, y + 30, 80, 25};
    SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255);
    SDL_RenderFillRect(renderer, &refresh_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &refresh_btn);
    draw_text(renderer, "Refresh", refresh_btn.x + 15, refresh_btn.y + 6, text_color);
    
    // Model list
    int list_y = y + 65;
    int list_height = height - 130;
    SDL_Rect list_area = {x + 10, list_y, width - 20, list_height};
    SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
    SDL_RenderFillRect(renderer, &list_area);
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderDrawRect(renderer, &list_area);
    
    // Draw model files
    int item_y = list_area.y + 5;
    int visible_items = (list_height - 10) / 20;
    for (int i = model_browser.scroll_offset; 
         i < model_browser.model_count && i < model_browser.scroll_offset + visible_items; 
         i++) {
        ModelFileInfo* model = &model_browser.models[i];
        
        SDL_Rect item_rect = {list_area.x + 5, item_y, list_area.w - 10, 18};
        
        // Highlight selected
        if (i == model_browser.selected_index) {
            SDL_SetRenderDrawColor(renderer, selected_color.r, selected_color.g, selected_color.b, 255);
            SDL_RenderFillRect(renderer, &item_rect);
        }
        
        // Draw filename
        draw_text(renderer, model->filename, item_rect.x + 5, item_rect.y + 2, text_color);
        
        // Draw file size
        char size_str[32];
        format_file_size(model->file_size, size_str, sizeof(size_str));
        draw_text(renderer, size_str, item_rect.x + item_rect.w - 80, item_rect.y + 2, 
                 (SDL_Color){150, 150, 150, 255});
        
        item_y += 20;
    }
    
    // Action buttons at bottom
    int btn_y = y + height - 40;
    int btn_width = (width - 40) / 3;
    
    SDL_Rect load_btn = {x + 10, btn_y, btn_width, 30};
    SDL_SetRenderDrawColor(renderer, 60, 100, 60, 255);
    SDL_RenderFillRect(renderer, &load_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &load_btn);
    draw_text(renderer, "Load", load_btn.x + btn_width/2 - 15, load_btn.y + 8, text_color);
    
    SDL_Rect export_btn = {x + 20 + btn_width, btn_y, btn_width, 30};
    SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255);
    SDL_RenderFillRect(renderer, &export_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &export_btn);
    draw_text(renderer, "Export", export_btn.x + btn_width/2 - 20, export_btn.y + 8, text_color);
    
    SDL_Rect close_btn = {x + 30 + btn_width * 2, btn_y, btn_width, 30};
    SDL_SetRenderDrawColor(renderer, 80, 60, 60, 255);
    SDL_RenderFillRect(renderer, &close_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &close_btn);
    draw_text(renderer, "Close", close_btn.x + btn_width/2 - 20, close_btn.y + 8, text_color);
}

// Draw model size selection dialog with scrolling
static int model_dialog_scroll = 0;

static void draw_model_size_dialog(SDL_Renderer* renderer, int x, int y, int width, int height) {
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {30, 30, 40, 255};
    
    // Draw panel background
    SDL_Rect panel = {x, y, width, height};
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 100, 120, 140, 255);
    SDL_RenderDrawRect(renderer, &panel);
    
    // Title
    draw_text(renderer, "SELECT MODEL SIZE", x + width/2 - 80, y + 15, 
             (SDL_Color){100, 150, 200, 255});
    
    // Scrollable area
    int content_y = y + 50 - model_dialog_scroll;
    int btn_width = width - 40;
    int btn_height = 70;
    int spacing = 12;
    
    // TINY model button (NEW - FIRST OPTION)
    SDL_Rect tiny_btn = {x + 20, content_y, btn_width, btn_height};
    SDL_SetRenderDrawColor(renderer, 40, 80, 120, 255);
    SDL_RenderFillRect(renderer, &tiny_btn);
    SDL_SetRenderDrawColor(renderer, 80, 140, 200, 255);
    SDL_RenderDrawRect(renderer, &tiny_btn);
    draw_text(renderer, "TINY - 25M params (RECOMMENDED)", tiny_btn.x + 10, tiny_btn.y + 8, 
             (SDL_Color){120, 200, 255, 255});
    draw_text(renderer, "10K vocab, 6 layers | Ultra-fast", tiny_btn.x + 10, tiny_btn.y + 26, text_color);
    draw_text(renderer, "RAM: ~100MB | Best for testing", tiny_btn.x + 10, tiny_btn.y + 44, 
             (SDL_Color){180, 180, 180, 255});
    content_y += btn_height + spacing;
    
    // Small model button
    SDL_Rect small_btn = {x + 20, content_y, btn_width, btn_height};
    SDL_SetRenderDrawColor(renderer, 60, 100, 60, 255);
    SDL_RenderFillRect(renderer, &small_btn);
    SDL_SetRenderDrawColor(renderer, 100, 150, 100, 255);
    SDL_RenderDrawRect(renderer, &small_btn);
    draw_text(renderer, "SMALL - 117M params", small_btn.x + 10, small_btn.y + 8, 
             (SDL_Color){150, 255, 150, 255});
    draw_text(renderer, "30K vocab, 12 layers | GPT-2 Small", small_btn.x + 10, small_btn.y + 26, text_color);
    draw_text(renderer, "RAM: ~500MB | Good for testing", small_btn.x + 10, small_btn.y + 44, 
             (SDL_Color){180, 180, 180, 255});
    content_y += btn_height + spacing;
    
    // Medium model button
    SDL_Rect medium_btn = {x + 20, content_y, btn_width, btn_height};
    SDL_SetRenderDrawColor(renderer, 80, 100, 120, 255);
    SDL_RenderFillRect(renderer, &medium_btn);
    SDL_SetRenderDrawColor(renderer, 120, 150, 180, 255);
    SDL_RenderDrawRect(renderer, &medium_btn);
    draw_text(renderer, "MEDIUM - 345M params (RECOMMENDED)", medium_btn.x + 10, medium_btn.y + 8, 
             (SDL_Color){150, 200, 255, 255});
    draw_text(renderer, "50K vocab, 24 layers | GPT-2 Medium", medium_btn.x + 10, medium_btn.y + 26, text_color);
    draw_text(renderer, "RAM: ~1.5GB | Best balance", medium_btn.x + 10, medium_btn.y + 44, 
             (SDL_Color){180, 180, 180, 255});
    content_y += btn_height + spacing;
    
    // Large model button
    SDL_Rect large_btn = {x + 20, content_y, btn_width, btn_height};
    SDL_SetRenderDrawColor(renderer, 100, 80, 60, 255);
    SDL_RenderFillRect(renderer, &large_btn);
    SDL_SetRenderDrawColor(renderer, 150, 120, 80, 255);
    SDL_RenderDrawRect(renderer, &large_btn);
    draw_text(renderer, "LARGE - 762M params", large_btn.x + 10, large_btn.y + 8, 
             (SDL_Color){255, 200, 150, 255});
    draw_text(renderer, "50K vocab, 36 layers | GPT-2 Large", large_btn.x + 10, large_btn.y + 26, text_color);
    draw_text(renderer, "RAM: ~3GB | High quality", large_btn.x + 10, large_btn.y + 44, 
             (SDL_Color){180, 180, 180, 255});
    content_y += btn_height + spacing;
    
    // Huge model button
    SDL_Rect huge_btn = {x + 20, content_y, btn_width, btn_height};
    SDL_SetRenderDrawColor(renderer, 120, 80, 120, 255);
    SDL_RenderFillRect(renderer, &huge_btn);
    SDL_SetRenderDrawColor(renderer, 180, 120, 180, 255);
    SDL_RenderDrawRect(renderer, &huge_btn);
    draw_text(renderer, "HUGE - 1.5B params", huge_btn.x + 10, huge_btn.y + 8, 
             (SDL_Color){255, 180, 255, 255});
    draw_text(renderer, "50K vocab, 48 layers | GPT-2 XL", huge_btn.x + 10, huge_btn.y + 26, text_color);
    draw_text(renderer, "RAM: ~6GB | Professional grade", huge_btn.x + 10, huge_btn.y + 44, 
             (SDL_Color){180, 180, 180, 255});
    content_y += btn_height + spacing;
    
    // Massive model button
    SDL_Rect massive_btn = {x + 20, content_y, btn_width, btn_height};
    SDL_SetRenderDrawColor(renderer, 140, 60, 60, 255);
    SDL_RenderFillRect(renderer, &massive_btn);
    SDL_SetRenderDrawColor(renderer, 200, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &massive_btn);
    draw_text(renderer, "MASSIVE - 3B params", massive_btn.x + 10, massive_btn.y + 8, 
             (SDL_Color){255, 150, 150, 255});
    draw_text(renderer, "50K vocab, 64 layers | GPT-3 Small", massive_btn.x + 10, massive_btn.y + 26, text_color);
    draw_text(renderer, "RAM: ~12GB | Enterprise grade", massive_btn.x + 10, massive_btn.y + 44, 
             (SDL_Color){180, 180, 180, 255});
    content_y += btn_height + spacing;
    
    // Astronomical model button
    SDL_Rect astro_btn = {x + 20, content_y, btn_width, btn_height};
    SDL_SetRenderDrawColor(renderer, 160, 120, 40, 255);
    SDL_RenderFillRect(renderer, &astro_btn);
    SDL_SetRenderDrawColor(renderer, 220, 180, 80, 255);
    SDL_RenderDrawRect(renderer, &astro_btn);
    draw_text(renderer, "ASTRONOMICAL - 7B params", astro_btn.x + 10, astro_btn.y + 8, 
             (SDL_Color){255, 220, 100, 255});
    draw_text(renderer, "50K vocab, 32 layers | LLaMA-7B", astro_btn.x + 10, astro_btn.y + 26, text_color);
    draw_text(renderer, "RAM: ~28GB | State-of-the-art", astro_btn.x + 10, astro_btn.y + 44, 
             (SDL_Color){180, 180, 180, 255});
    content_y += btn_height + spacing;
    
    // Cancel button
    SDL_Rect cancel_btn = {x + width/2 - 50, y + height - 45, 100, 30};
    SDL_SetRenderDrawColor(renderer, 80, 60, 60, 255);
    SDL_RenderFillRect(renderer, &cancel_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &cancel_btn);
    draw_text(renderer, "Cancel", cancel_btn.x + 30, cancel_btn.y + 8, text_color);
}

// Draw thread list panel
static void draw_thread_list_panel(SDL_Renderer* renderer, int x, int y, int width, int height) {
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color bg_color = {30, 30, 40, 255};
    SDL_Color active_color = {60, 100, 140, 255};
    
    // Draw panel background
    SDL_Rect panel = {x, y, width, height};
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
    SDL_RenderDrawRect(renderer, &panel);
    
    // Title
    draw_text(renderer, "CONVERSATIONS", x + 10, y + 10, (SDL_Color){100, 150, 200, 255});
    
    // Thread list
    int list_y = y + 40;
    int list_height = height - 100;
    SDL_Rect list_area = {x + 10, list_y, width - 20, list_height};
    SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
    SDL_RenderFillRect(renderer, &list_area);
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderDrawRect(renderer, &list_area);
    
    // Draw threads
    int item_y = list_area.y + 5;
    for (int i = 0; i < thread_manager.thread_count; i++) {
        ConversationThread* thread = &thread_manager.threads[i];
        
        SDL_Rect item_rect = {list_area.x + 5, item_y, list_area.w - 10, 25};
        
        // Highlight active thread
        if (i == thread_manager.active_thread_index) {
            SDL_SetRenderDrawColor(renderer, active_color.r, active_color.g, active_color.b, 255);
            SDL_RenderFillRect(renderer, &item_rect);
        }
        
        // Draw thread name
        draw_text(renderer, thread->name, item_rect.x + 5, item_rect.y + 5, text_color);
        
        // Draw message count
        char count_str[32];
        snprintf(count_str, sizeof(count_str), "%d msgs", thread->message_count);
        draw_text(renderer, count_str, item_rect.x + item_rect.w - 60, item_rect.y + 5,
                 (SDL_Color){150, 150, 150, 255});
        
        item_y += 30;
    }
    
    // Action buttons
    int btn_y = y + height - 45;
    int btn_width = (width - 30) / 2;
    
    SDL_Rect new_btn = {x + 10, btn_y, btn_width, 30};
    SDL_SetRenderDrawColor(renderer, 60, 100, 60, 255);
    SDL_RenderFillRect(renderer, &new_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &new_btn);
    draw_text(renderer, "New", new_btn.x + btn_width/2 - 12, new_btn.y + 8, text_color);
    
    SDL_Rect close_btn = {x + 20 + btn_width, btn_y, btn_width, 30};
    SDL_SetRenderDrawColor(renderer, 80, 60, 60, 255);
    SDL_RenderFillRect(renderer, &close_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &close_btn);
    draw_text(renderer, "Close", close_btn.x + btn_width/2 - 20, close_btn.y + 8, text_color);
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
    
    int panel_x = RENDER_OFFSET_X + RENDER_WIDTH;  // Fixed: account for sidebar
    int panel_y = RENDER_OFFSET_Y;  // Fixed: account for submenu
    int panel_width = CONTROL_PANEL_WIDTH;
    
    // Initialize model selector on first draw
    if (!llm_model_selector) {
        llm_model_selector = model_selector_create(panel_x + 10, panel_y + 50, panel_width - 20, 30);
        model_selector_update_list(llm_model_selector);
        
        // Set callback to load model when selected
        model_selector_set_callback(llm_model_selector, on_llm_model_selected, state);
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
    
    // Clear chat button
    g_clear_chat_btn = layout_add_button(&layout, NULL, 0, 25);
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &g_clear_chat_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_clear_chat_btn);
    draw_text(renderer, "Clear Chat", g_clear_chat_btn.x + 70, g_clear_chat_btn.y + 6, text_color);
    
    // === CHAT AREA (LEFT SIDE) ===
    int chat_width = RENDER_WIDTH - 20;  // Width is correct
    int input_height = 80;
    int chat_height = WINDOW_HEIGHT - 60 - input_height - 20;
    
    g_chat_area = (SDL_Rect){RENDER_OFFSET_X + 10, RENDER_OFFSET_Y + 10, chat_width, chat_height};  // Fixed: account for sidebar
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
    
    // Input box with room for send button on the right
    // Button will be 100px wide, positioned at right edge of input area
    g_input_rect = (SDL_Rect){RENDER_OFFSET_X + 10, input_y, chat_width - 120, input_height};  // Leave 120px for button + gap
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
    
    // Send button positioned at right edge of input area (overlapping visually)
    g_send_btn = (SDL_Rect){g_input_rect.x + g_input_rect.w + 10, input_y, 100, input_height};
    SDL_Color send_color = state->llm_generating ? 
        (SDL_Color){80, 80, 80, 255} : (SDL_Color){80, 150, 80, 255};
    SDL_SetRenderDrawColor(renderer, send_color.r, send_color.g, send_color.b, 255);
    SDL_RenderFillRect(renderer, &g_send_btn);
    SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
    SDL_RenderDrawRect(renderer, &g_send_btn);
    const char* send_text = state->llm_generating ? "..." : "Send";
    draw_text(renderer, send_text, g_send_btn.x + 32, g_send_btn.y + 32, text_color);
    
    // Draw overlay panels if visible
    if (model_size_dialog_visible) {
        // Semi-transparent overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        
        // Model size dialog (centered)
        int panel_w = 500;
        int panel_h = 450;
        int panel_x = (WINDOW_WIDTH - panel_w) / 2;
        int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
        draw_model_size_dialog(renderer, panel_x, panel_y, panel_w, panel_h);
    }
    
    if (model_browser_visible) {
        // Semi-transparent overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        
        // Model browser panel (centered)
        int panel_w = 600;
        int panel_h = 500;
        int panel_x = (WINDOW_WIDTH - panel_w) / 2;
        int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
        draw_model_browser_panel(renderer, panel_x, panel_y, panel_w, panel_h);
    }
    
    if (thread_list_visible) {
        // Semi-transparent overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        
        // Thread list panel (centered)
        int panel_w = 400;
        int panel_h = 450;
        int panel_x = (WINDOW_WIDTH - panel_w) / 2;
        int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
        draw_thread_list_panel(renderer, panel_x, panel_y, panel_w, panel_h);
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
    
    // Check model selector click first
    if (llm_model_selector && model_selector_handle_click(llm_model_selector, x, y)) {
        return;
    }
    
    // Handle model browser panel clicks
    if (model_browser_visible) {
        int panel_w = 600;
        int panel_h = 500;
        int panel_x = (WINDOW_WIDTH - panel_w) / 2;
        int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
        
        // Check if click is inside panel
        if (x >= panel_x && x <= panel_x + panel_w &&
            y >= panel_y && y <= panel_y + panel_h) {
            // Handle panel clicks (TODO: implement specific button handling)
            // For now, just close on any click inside
            model_browser_visible = false;
        } else {
            // Click outside panel - close it
            model_browser_visible = false;
        }
        return;
    }
    
    // NOTE: Input box clicks are handled by InputManager - no need for duplicate handler here
    
    // Handle thread list panel clicks
    if (thread_list_visible) {
        int panel_w = 400;
        int panel_h = 450;
        int panel_x = (WINDOW_WIDTH - panel_w) / 2;
        int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
        
        // Check if click is inside panel
        if (x >= panel_x && x <= panel_x + panel_w &&
            y >= panel_y && y <= panel_y + panel_h) {
            // Handle panel clicks (TODO: implement specific button handling)
            thread_list_visible = false;
        } else {
            // Click outside panel - close it
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
    if (x >= g_clear_chat_btn.x && x <= g_clear_chat_btn.x + g_clear_chat_btn.w &&
        y >= g_clear_chat_btn.y && y <= g_clear_chat_btn.y + g_clear_chat_btn.h) {
        clear_chat_history();
        return;
    }
    
    // Handle model size dialog clicks
    if (model_size_dialog_visible) {
        int panel_w = 500;
        int panel_h = 600;  // Increased height for more buttons
        int panel_x = (WINDOW_WIDTH - panel_w) / 2;
        int panel_y = (WINDOW_HEIGHT - panel_h) / 2;
        
        // Check which button was clicked
        int content_y = panel_y + 50 - model_dialog_scroll;
        int btn_width = panel_w - 40;
        int btn_height = 70;
        int spacing = 12;
        
        // Define scrollable area bounds
        int scroll_top = panel_y + 50;
        int scroll_bottom = panel_y + panel_h - 50;
        
        // TINY button (NEW - FIRST OPTION)
        SDL_Rect tiny_btn = {panel_x + 20, content_y, btn_width, btn_height};
        if (x >= tiny_btn.x && x <= tiny_btn.x + tiny_btn.w &&
            y >= tiny_btn.y && y <= tiny_btn.y + tiny_btn.h &&
            tiny_btn.y >= scroll_top && tiny_btn.y + tiny_btn.h <= scroll_bottom) {
            printf("Acquiring TINY model (25M params)...\n");
            acquire_model_for_inference(state, "tiny_model", 10000, 256, 4, 4, 1024);
            model_size_dialog_visible = false;
            model_dialog_scroll = 0;
            return;
        }
        content_y += btn_height + spacing;
        
        // Small button
        SDL_Rect small_btn = {panel_x + 20, content_y, btn_width, btn_height};
        if (x >= small_btn.x && x <= small_btn.x + small_btn.w &&
            y >= small_btn.y && y <= small_btn.y + small_btn.h &&
            small_btn.y >= scroll_top && small_btn.y + small_btn.h <= scroll_bottom) {
            printf("Acquiring SMALL model (117M params)...\n");
            acquire_model_for_inference(state, "small_model", 10000, 512, 6, 8, 2048);
            model_size_dialog_visible = false;
            model_dialog_scroll = 0;
            return;
        }
        content_y += btn_height + spacing;
        
        // Medium button
        SDL_Rect medium_btn = {panel_x + 20, content_y, btn_width, btn_height};
        if (x >= medium_btn.x && x <= medium_btn.x + medium_btn.w &&
            y >= medium_btn.y && y <= medium_btn.y + medium_btn.h &&
            medium_btn.y >= scroll_top && medium_btn.y + medium_btn.h <= scroll_bottom) {
            printf("Acquiring MEDIUM model (345M params)...\n");
            acquire_model_for_inference(state, "medium_model", 10000, 768, 12, 12, 3072);
            model_size_dialog_visible = false;
            model_dialog_scroll = 0;
            return;
        }
        content_y += btn_height + spacing;
        
        // Large button
        SDL_Rect large_btn = {panel_x + 20, content_y, btn_width, btn_height};
        if (x >= large_btn.x && x <= large_btn.x + large_btn.w &&
            y >= large_btn.y && y <= large_btn.y + large_btn.h &&
            large_btn.y >= scroll_top && large_btn.y + large_btn.h <= scroll_bottom) {
            printf("Acquiring LARGE model (762M params)...\n");
            acquire_model_for_inference(state, "large_model", 10000, 1024, 24, 16, 4096);
            model_size_dialog_visible = false;
            model_dialog_scroll = 0;
            return;
        }
        content_y += btn_height + spacing;
        
        // Huge button
        SDL_Rect huge_btn = {panel_x + 20, content_y, btn_width, btn_height};
        if (x >= huge_btn.x && x <= huge_btn.x + huge_btn.w &&
            y >= huge_btn.y && y <= huge_btn.y + huge_btn.h &&
            huge_btn.y >= scroll_top && huge_btn.y + huge_btn.h <= scroll_bottom) {
            printf("Acquiring HUGE model (1.5B params)...\n");
            acquire_model_for_inference(state, "huge_model", 10000, 1280, 36, 20, 5120);
            model_size_dialog_visible = false;
            model_dialog_scroll = 0;
            return;
        }
        content_y += btn_height + spacing;
        
        // Massive button
        SDL_Rect massive_btn = {panel_x + 20, content_y, btn_width, btn_height};
        if (x >= massive_btn.x && x <= massive_btn.x + massive_btn.w &&
            y >= massive_btn.y && y <= massive_btn.y + massive_btn.h &&
            massive_btn.y >= scroll_top && massive_btn.y + massive_btn.h <= scroll_bottom) {
            printf("Acquiring MASSIVE model (3B params)...\n");
            acquire_model_for_inference(state, "massive_model", 10000, 1536, 48, 24, 6144);
            model_size_dialog_visible = false;
            model_dialog_scroll = 0;
            return;
        }
        content_y += btn_height + spacing;
        
        // Astronomical button
        SDL_Rect astro_btn = {panel_x + 20, content_y, btn_width, btn_height};
        if (x >= astro_btn.x && x <= astro_btn.x + astro_btn.w &&
            y >= astro_btn.y && y <= astro_btn.y + astro_btn.h &&
            astro_btn.y >= scroll_top && astro_btn.y + astro_btn.h <= scroll_bottom) {
            printf("Creating ASTRONOMICAL model (7B params)...\n");
            printf("WARNING: This will use ~28GB RAM!\n");
            state->cllm_model = app_create_cllm_model_astronomical();
            if (state->cllm_model) {
                if (state->cllm_inference) cllm_inference_cleanup(state->cllm_inference);
                state->cllm_inference = cllm_inference_init(state->cllm_model);
            }
            model_size_dialog_visible = false;
            model_dialog_scroll = 0;
            return;
        }
        content_y += btn_height + spacing;
        
        // Cancel button (fixed at bottom)
        SDL_Rect cancel_btn = {panel_x + panel_w/2 - 50, panel_y + panel_h - 45, 100, 30};
        if (x >= cancel_btn.x && x <= cancel_btn.x + cancel_btn.w &&
            y >= cancel_btn.y && y <= cancel_btn.y + cancel_btn.h) {
            model_size_dialog_visible = false;
            model_dialog_scroll = 0;
            return;
        }
        
        // Click outside dialog - close it
        if (x < panel_x || x > panel_x + panel_w ||
            y < panel_y || y > panel_y + panel_h) {
            model_size_dialog_visible = false;
            model_dialog_scroll = 0;
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
                    cllm_free_model(state->cllm_model);  // Free old model to prevent memory leak
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
    if (!state->cllm_inference || state->llm_generating) return;
    
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