#!/usr/bin/env python3
"""
Complete rewrite of crawler tab to use InputManager correctly
Following the exact pattern from training tab
"""

# Read the backup file
with open('app/ui/tabs/tab_crawler.c.backup', 'r') as f:
    content = f.read()

print("Step 1: Updating state structure...")

# The state structure should NOT have TextInput instances
# Just simple state tracking
new_state = '''typedef struct {
    // Prime Configuration
    CrawlerPrimeConfig prime_config;
    bool prime_enabled;
    
    // Link Management
    LinkQueue* link_queue;
    int link_list_scroll;
    bool show_add_confirmation;
    int confirmation_timer;
    
    // URL Pattern Selection
    bool pattern_href;
    bool pattern_onclick;
    bool pattern_data_attr;
    bool pattern_meta_refresh;
    
    // Activity Log
    char activity_log[10][256];
    int activity_count;
    
    // UI State
    bool inputs_initialized;
    bool inputs_registered;
} CrawlerTabState;'''

# Replace the state structure
import re
content = re.sub(
    r'typedef struct \{.*?\} CrawlerTabState;',
    new_state,
    content,
    flags=re.DOTALL
)

print("Step 2: Creating input registration function...")

# Add input registration function after init_crawler_tab_state
registration_function = '''
// Register inputs with InputManager (called once during first draw)
static void register_crawler_inputs(const ColumnLayout* col1) {
    if (g_crawler_state.inputs_registered) return;
    
    extern InputManager* g_input_manager;
    if (!g_input_manager) return;
    
    int x = col1->x + col1->padding;
    int y_freq = col1->y + col1->padding + 65;  // After "Frequency:" label
    int y_sel = y_freq + 48;  // After frequency input
    int y_min = y_sel + 48;   // After selection input
    int y_max = y_min + 48;   // After min input
    
    // Register prime configuration inputs
    input_manager_register(g_input_manager, "crawler.frequency", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){x, y_freq, 150, 22});
    input_manager_set_text(g_input_manager, "crawler.frequency", "7");
    
    input_manager_register(g_input_manager, "crawler.selection", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){x, y_sel, 150, 22});
    input_manager_set_text(g_input_manager, "crawler.selection", "13");
    
    input_manager_register(g_input_manager, "crawler.delay_min", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){x, y_min, 150, 22});
    input_manager_set_text(g_input_manager, "crawler.delay_min", "3");
    
    input_manager_register(g_input_manager, "crawler.delay_max", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){x, y_max, 150, 22});
    input_manager_set_text(g_input_manager, "crawler.delay_max", "11");
    
    printf("Crawler tab: Registered 4 inputs with InputManager\\n");
    g_crawler_state.inputs_registered = true;
}
'''

# Insert after init_crawler_tab_state function
content = re.sub(
    r'(static void init_crawler_tab_state\(void\) \{.*?\n\})',
    r'\1' + registration_function,
    content,
    flags=re.DOTALL
)

print("Step 3: Updating validate_prime_input to use InputManager...")

# Update validate function to get text from InputManager
new_validate = '''static bool validate_prime_input(const char* input_id, uint64_t* value) {
    extern InputManager* g_input_manager;
    if (!g_input_manager) return false;
    
    const char* text = input_manager_get_text(g_input_manager, input_id);
    if (!text || text[0] == '\\0') return false;
    
    char* endptr;
    unsigned long val = strtoul(text, &endptr, 10);
    
    if (*endptr != '\\0' || val == 0) {
        return false;
    }
    
    if (!is_prime(val)) {
        return false;
    }
    
    *value = val;
    return true;
}'''

content = re.sub(
    r'static bool validate_prime_input\(const char\* input, uint64_t\* value\) \{.*?\n\}',
    new_validate,
    content,
    flags=re.DOTALL
)

print("Step 4: Updating draw functions to use InputManager...")

# Remove all manual input drawing and replace with labels + validation indicators
# The actual input rendering will be done by input_manager_render()

# Replace frequency input drawing
content = re.sub(
    r'SDL_Rect freq_rect = \{x, y, 150, 22\};.*?y \+= 30;',
    '''// Input will be rendered by InputManager
        draw_text(renderer, freq_valid ? "OK" : "X", x + 160, y + 5, freq_color);
        y += 30;''',
    content,
    flags=re.DOTALL
)

# Replace selection input drawing
content = re.sub(
    r'SDL_Rect sel_rect = \{x, y, 150, 22\};.*?y \+= 30;',
    '''// Input will be rendered by InputManager
        draw_text(renderer, sel_valid ? "OK" : "X", x + 160, y + 5, sel_color);
        y += 30;''',
    content,
    flags=re.DOTALL
)

# Replace delay_min input drawing
content = re.sub(
    r'SDL_Rect min_rect = \{x, y, 150, 22\};.*?y \+= 30;',
    '''// Input will be rendered by InputManager
        draw_text(renderer, min_valid ? "OK" : "X", x + 160, y + 5, min_color);
        y += 30;''',
    content,
    flags=re.DOTALL
)

# Replace delay_max input drawing
content = re.sub(
    r'SDL_Rect max_rect = \{x, y, 150, 22\};.*?y \+= 30;',
    '''// Input will be rendered by InputManager
        draw_text(renderer, max_valid ? "OK" : "X", x + 160, y + 5, max_color);
        y += 30;''',
    content,
    flags=re.DOTALL
)

print("Step 5: Updating validate calls to use input IDs...")

# Update all validate_prime_input calls
content = content.replace(
    'validate_prime_input(g_crawler_state.frequency_input,',
    'validate_prime_input("crawler.frequency",'
)
content = content.replace(
    'validate_prime_input(g_crawler_state.selection_input,',
    'validate_prime_input("crawler.selection",'
)
content = content.replace(
    'validate_prime_input(g_crawler_state.delay_min_input,',
    'validate_prime_input("crawler.delay_min",'
)
content = content.replace(
    'validate_prime_input(g_crawler_state.delay_max_input,',
    'validate_prime_input("crawler.delay_max",'
)

print("Step 6: Adding input_manager_render call...")

# Find the end of draw_crawler_tab_with_layout and add rendering before the closing brace
# Add before the last closing brace of the function
render_call = '''
    // Register inputs on first draw
    if (layout->num_columns >= 1) {
        register_crawler_inputs(&layout->columns[0]);
    }
    
    // Render all inputs through InputManager
    extern InputManager* g_input_manager;
    if (g_input_manager) {
        input_manager_render(g_input_manager, renderer, get_global_font(), TAB_CRAWLER);
    }
}'''

# Replace the last } of draw_crawler_tab_with_layout
content = re.sub(
    r'(\n\s+\}\n\s+\}\n)(// ============================================================================\n// CLICK HANDLER)',
    render_call + '\n\n\\2',
    content
)

print("Step 7: Simplifying event handlers...")

# Simplify click handler - InputManager handles everything
new_click_handler = '''void handle_crawler_tab_click(AppState* state, int mouse_x, int mouse_y) {
    // InputManager handles all input clicks automatically
    // This function can handle button clicks if needed
    (void)state;
    (void)mouse_x;
    (void)mouse_y;
}'''

content = re.sub(
    r'void handle_crawler_tab_click\(AppState\* state, int mouse_x, int mouse_y\) \{.*?\n\}',
    new_click_handler,
    content,
    flags=re.DOTALL
)

# Simplify keyboard handler
new_keyboard_handler = '''void handle_crawler_tab_keyboard(AppState* state, int key) {
    // InputManager handles all keyboard input automatically
    (void)state;
    (void)key;
}'''

content = re.sub(
    r'void handle_crawler_tab_keyboard\(AppState\* state, int key\) \{.*?\n\}',
    new_keyboard_handler,
    content,
    flags=re.DOTALL
)

# Write the new file
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.write(content)

print("\n✓ Complete rewrite finished!")
print("  - State structure simplified")
print("  - Input registration function added")
print("  - validate_prime_input updated for InputManager")
print("  - Manual input drawing removed")
print("  - input_manager_render() call added")
print("  - Event handlers simplified")