#!/usr/bin/env python3
"""
Complete and careful fix for the crawler tab input system
"""

import re

# Read the file
with open('app/ui/tabs/tab_crawler.c', 'r') as f:
    content = f.read()

print("Step 1: Updating state structure...")
# Replace char arrays with TextInput in the state structure
content = content.replace(
    '    char frequency_input[32];',
    '    TextInput frequency_input;'
)
content = content.replace(
    '    char selection_input[32];',
    '    TextInput selection_input;'
)
content = content.replace(
    '    char delay_min_input[32];',
    '    TextInput delay_min_input;'
)
content = content.replace(
    '    char delay_max_input[32];',
    '    TextInput delay_max_input;'
)
content = content.replace(
    '    char add_link_input[512];',
    '    TextInput add_link_input;'
)
content = content.replace(
    '    int active_input; // 0=none, 1-4=prime inputs, 5=add_link',
    '    TextInput* focused_input; // Currently focused input'
)

print("Step 2: Fixing init function...")
# Fix the init function - more targeted replacement
old_init_pattern = r'(static void init_crawler_tab_state\(void\) \{\s+if \(g_crawler_state\.inputs_initialized\) return;\s+// Initialize prime config with defaults\s+prime_config_init_default\(&g_crawler_state\.prime_config\);\s+g_crawler_state\.prime_enabled = true;)\s+// Convert defaults to strings.*?g_crawler_state\.inputs_initialized = true;\s*\}'

new_init = r'''\1
    
    // Initialize TextInput fields
    text_input_init(&g_crawler_state.frequency_input, "Frequency", 0, 0, 150, 22);
    text_input_init(&g_crawler_state.selection_input, "Selection", 0, 0, 150, 22);
    text_input_init(&g_crawler_state.delay_min_input, "Delay Min", 0, 0, 150, 22);
    text_input_init(&g_crawler_state.delay_max_input, "Delay Max", 0, 0, 150, 22);
    text_input_init(&g_crawler_state.add_link_input, "Add URL", 0, 0, 400, 25);
    
    // Set numeric constraints
    text_input_set_numeric(&g_crawler_state.frequency_input, 2, 1000000);
    text_input_set_numeric(&g_crawler_state.selection_input, 2, 1000000);
    text_input_set_numeric(&g_crawler_state.delay_min_input, 2, 1000000);
    text_input_set_numeric(&g_crawler_state.delay_max_input, 2, 1000000);
    
    // Set initial values
    char temp[32];
    snprintf(temp, sizeof(temp), "%lu", (unsigned long)g_crawler_state.prime_config.frequency_prime);
    text_input_set_text(&g_crawler_state.frequency_input, temp);
    
    snprintf(temp, sizeof(temp), "%lu", (unsigned long)g_crawler_state.prime_config.link_selection_prime);
    text_input_set_text(&g_crawler_state.selection_input, temp);
    
    snprintf(temp, sizeof(temp), "%lu", (unsigned long)g_crawler_state.prime_config.delay_min_prime);
    text_input_set_text(&g_crawler_state.delay_min_input, temp);
    
    snprintf(temp, sizeof(temp), "%lu", (unsigned long)g_crawler_state.prime_config.delay_max_prime);
    text_input_set_text(&g_crawler_state.delay_max_input, temp);
    
    // Initialize link queue
    g_crawler_state.link_queue = link_queue_create("data/crawler/link_queue.txt");
    
    // Enable all URL patterns by default
    g_crawler_state.pattern_href = true;
    g_crawler_state.pattern_onclick = true;
    g_crawler_state.pattern_data_attr = true;
    g_crawler_state.pattern_meta_refresh = true;
    
    g_crawler_state.focused_input = NULL;
    g_crawler_state.inputs_initialized = true;
}'''

content = re.sub(old_init_pattern, new_init, content, flags=re.DOTALL)

print("Step 3: Fixing validate_prime_input function...")
# Fix validate_prime_input
content = content.replace(
    'static bool validate_prime_input(const char* input, uint64_t* value) {',
    'static bool validate_prime_input(TextInput* input, uint64_t* value) {\n    const char* text = text_input_get_text(input);'
)
content = content.replace(
    '    unsigned long val = strtoul(input, &endptr, 10);',
    '    unsigned long val = strtoul(text, &endptr, 10);'
)

print("Step 4: Fixing validate_prime_input calls...")
# Fix all calls to validate_prime_input
content = content.replace(
    'validate_prime_input(g_crawler_state.frequency_input,',
    'validate_prime_input(&g_crawler_state.frequency_input,'
)
content = content.replace(
    'validate_prime_input(g_crawler_state.selection_input,',
    'validate_prime_input(&g_crawler_state.selection_input,'
)
content = content.replace(
    'validate_prime_input(g_crawler_state.delay_min_input,',
    'validate_prime_input(&g_crawler_state.delay_min_input,'
)
content = content.replace(
    'validate_prime_input(g_crawler_state.delay_max_input,',
    'validate_prime_input(&g_crawler_state.delay_max_input,'
)

print("Step 5: Fixing drawing code for inputs...")
# Fix drawing code - replace manual drawing with text_input_render
# Frequency input
content = re.sub(
    r'SDL_Rect freq_rect = \{x, y, 150, 22\};\s+SDL_SetRenderDrawColor\(renderer, 30, 30, 40, 255\);\s+SDL_RenderFillRect\(renderer, &freq_rect\);\s+SDL_SetRenderDrawColor\(renderer, freq_color\.r, freq_color\.g, freq_color\.b, 255\);\s+SDL_RenderDrawRect\(renderer, &freq_rect\);\s+draw_text\(renderer, g_crawler_state\.frequency_input, x \+ 5, y \+ 5, text_color\);',
    'g_crawler_state.frequency_input.bounds.x = x;\n        g_crawler_state.frequency_input.bounds.y = y;\n        g_crawler_state.frequency_input.bounds.w = 150;\n        g_crawler_state.frequency_input.bounds.h = 22;\n        text_input_render(&g_crawler_state.frequency_input, renderer, NULL);',
    content
)

# Selection input
content = re.sub(
    r'SDL_Rect sel_rect = \{x, y, 150, 22\};\s+SDL_SetRenderDrawColor\(renderer, 30, 30, 40, 255\);\s+SDL_RenderFillRect\(renderer, &sel_rect\);\s+SDL_SetRenderDrawColor\(renderer, sel_color\.r, sel_color\.g, sel_color\.b, 255\);\s+SDL_RenderDrawRect\(renderer, &sel_rect\);\s+draw_text\(renderer, g_crawler_state\.selection_input, x \+ 5, y \+ 5, text_color\);',
    'g_crawler_state.selection_input.bounds.x = x;\n        g_crawler_state.selection_input.bounds.y = y;\n        g_crawler_state.selection_input.bounds.w = 150;\n        g_crawler_state.selection_input.bounds.h = 22;\n        text_input_render(&g_crawler_state.selection_input, renderer, NULL);',
    content
)

# Delay min input
content = re.sub(
    r'SDL_Rect min_rect = \{x, y, 150, 22\};\s+SDL_SetRenderDrawColor\(renderer, 30, 30, 40, 255\);\s+SDL_RenderFillRect\(renderer, &min_rect\);\s+SDL_SetRenderDrawColor\(renderer, min_color\.r, min_color\.g, min_color\.b, 255\);\s+SDL_RenderDrawRect\(renderer, &min_rect\);\s+draw_text\(renderer, g_crawler_state\.delay_min_input, x \+ 5, y \+ 5, text_color\);',
    'g_crawler_state.delay_min_input.bounds.x = x;\n        g_crawler_state.delay_min_input.bounds.y = y;\n        g_crawler_state.delay_min_input.bounds.w = 150;\n        g_crawler_state.delay_min_input.bounds.h = 22;\n        text_input_render(&g_crawler_state.delay_min_input, renderer, NULL);',
    content
)

# Delay max input
content = re.sub(
    r'SDL_Rect max_rect = \{x, y, 150, 22\};\s+SDL_SetRenderDrawColor\(renderer, 30, 30, 40, 255\);\s+SDL_RenderFillRect\(renderer, &max_rect\);\s+SDL_SetRenderDrawColor\(renderer, max_color\.r, max_color\.g, max_color\.b, 255\);\s+SDL_RenderDrawRect\(renderer, &max_rect\);\s+draw_text\(renderer, g_crawler_state\.delay_max_input, x \+ 5, y \+ 5, text_color\);',
    'g_crawler_state.delay_max_input.bounds.x = x;\n        g_crawler_state.delay_max_input.bounds.y = y;\n        g_crawler_state.delay_max_input.bounds.w = 150;\n        g_crawler_state.delay_max_input.bounds.h = 22;\n        text_input_render(&g_crawler_state.delay_max_input, renderer, NULL);',
    content
)

# Add link input - this one is more complex
add_link_pattern = r'SDL_Rect input_rect = \{x, y, col->width - \(col->padding \* 2\), 22\};\s+SDL_SetRenderDrawColor\(renderer, 30, 30, 40, 255\);\s+SDL_RenderFillRect\(renderer, &input_rect\);\s+SDL_SetRenderDrawColor\(renderer, 100, 100, 120, 255\);\s+SDL_RenderDrawRect\(renderer, &input_rect\);\s+// Truncate if too long\s+char display_url\[50\];.*?draw_text\(renderer, display_url, x \+ 5, y \+ 5, text_color\);'

add_link_replacement = '''g_crawler_state.add_link_input.bounds.x = x;
    g_crawler_state.add_link_input.bounds.y = y;
    g_crawler_state.add_link_input.bounds.w = col->width - (col->padding * 2);
    g_crawler_state.add_link_input.bounds.h = 22;
    text_input_render(&g_crawler_state.add_link_input, renderer, NULL);'''

content = re.sub(add_link_pattern, add_link_replacement, content, flags=re.DOTALL)

print("Step 6: Fixing event handlers...")
# Fix click handler
old_click = r'void handle_crawler_tab_click\(AppState\* state, int mouse_x, int mouse_y\) \{\s+// TODO: Implement click handling for buttons and inputs\s+\(void\)state;\s+\(void\)mouse_x;\s+\(void\)mouse_y;\s*\}'

new_click = '''void handle_crawler_tab_click(AppState* state, int mouse_x, int mouse_y) {
    (void)state;
    
    // Create a fake SDL event for click handling
    SDL_Event click_event;
    click_event.type = SDL_MOUSEBUTTONDOWN;
    click_event.button.x = mouse_x;
    click_event.button.y = mouse_y;
    
    // Try each input field
    TextInput* inputs[] = {
        &g_crawler_state.frequency_input,
        &g_crawler_state.selection_input,
        &g_crawler_state.delay_min_input,
        &g_crawler_state.delay_max_input,
        &g_crawler_state.add_link_input
    };
    
    bool clicked_any = false;
    for (int i = 0; i < 5; i++) {
        if (text_input_handle_event(inputs[i], &click_event)) {
            // This input was clicked
            g_crawler_state.focused_input = inputs[i];
            clicked_any = true;
            
            // Deactivate all others
            for (int j = 0; j < 5; j++) {
                if (i != j) {
                    text_input_deactivate(inputs[j]);
                }
            }
            break;
        }
    }
    
    // If clicked outside all inputs, deactivate all
    if (!clicked_any && g_crawler_state.focused_input) {
        for (int i = 0; i < 5; i++) {
            text_input_deactivate(inputs[i]);
        }
        g_crawler_state.focused_input = NULL;
    }
}'''

content = re.sub(old_click, new_click, content, flags=re.DOTALL)

# Fix keyboard handler
old_keyboard = r'void handle_crawler_tab_keyboard\(AppState\* state, int key\) \{.*?\}'

new_keyboard = '''void handle_crawler_tab_keyboard(AppState* state, SDL_Event* event) {
    (void)state;
    
    // Route keyboard events to the focused input
    if (g_crawler_state.focused_input) {
        text_input_handle_event(g_crawler_state.focused_input, event);
    }
}'''

content = re.sub(old_keyboard, new_keyboard, content, flags=re.DOTALL)

# Write back
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.write(content)

print("\n✓ Complete fix applied successfully!")
print("  - Updated state structure")
print("  - Fixed init function")
print("  - Fixed validate_prime_input")
print("  - Fixed all drawing code")
print("  - Fixed event handlers")