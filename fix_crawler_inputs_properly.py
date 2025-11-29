#!/usr/bin/env python3
"""
Properly fix crawler tab to use InputManager like training tab does
"""

import re

# Read current file
with open('app/ui/tabs/tab_crawler.c', 'r') as f:
    content = f.read()

print("Step 1: Fixing init function to remove TextInput references...")

# Replace the entire init function
old_init = r'static void init_crawler_tab_state\(void\) \{.*?g_crawler_state\.inputs_initialized = true;\s*\}'

new_init = '''static void init_crawler_tab_state(void) {
    if (g_crawler_state.inputs_initialized) return;
    
    // Initialize prime config with defaults
    prime_config_init_default(&g_crawler_state.prime_config);
    g_crawler_state.prime_enabled = true;
    
    // Initialize link queue
    g_crawler_state.link_queue = link_queue_create("data/crawler/link_queue.txt");
    
    // Enable all URL patterns by default
    g_crawler_state.pattern_href = true;
    g_crawler_state.pattern_onclick = true;
    g_crawler_state.pattern_data_attr = true;
    g_crawler_state.pattern_meta_refresh = true;
    
    g_crawler_state.inputs_registered = false;
    g_crawler_state.inputs_initialized = true;
}'''

content = re.sub(old_init, new_init, content, flags=re.DOTALL)

print("Step 2: Fixing validate_prime_input to use InputManager...")

# Fix validate_prime_input to get text from InputManager
old_validate = r'static bool validate_prime_input\(TextInput\* input, uint64_t\* value\) \{.*?return true;\s*\}'

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

content = re.sub(old_validate, new_validate, content, flags=re.DOTALL)

print("Step 3: Removing all text_input_render calls and TextInput references...")

# Remove all text_input_render calls and replace with labels
content = re.sub(
    r'g_crawler_state\.frequency_input\.bounds\..*?\n.*?text_input_render\(&g_crawler_state\.frequency_input.*?\);',
    '// Input will be rendered by InputManager',
    content,
    flags=re.DOTALL
)

content = re.sub(
    r'g_crawler_state\.selection_input\.bounds\..*?\n.*?text_input_render\(&g_crawler_state\.selection_input.*?\);',
    '// Input will be rendered by InputManager',
    content,
    flags=re.DOTALL
)

content = re.sub(
    r'g_crawler_state\.delay_min_input\.bounds\..*?\n.*?text_input_render\(&g_crawler_state\.delay_min_input.*?\);',
    '// Input will be rendered by InputManager',
    content,
    flags=re.DOTALL
)

content = re.sub(
    r'g_crawler_state\.delay_max_input\.bounds\..*?\n.*?text_input_render\(&g_crawler_state\.delay_max_input.*?\);',
    '// Input will be rendered by InputManager',
    content,
    flags=re.DOTALL
)

content = re.sub(
    r'g_crawler_state\.add_link_input\.bounds\..*?\n.*?text_input_render\(&g_crawler_state\.add_link_input.*?\);',
    '// Input will be rendered by InputManager',
    content,
    flags=re.DOTALL
)

print("Step 4: Fixing validate_prime_input calls...")

# Fix all validate_prime_input calls
content = content.replace(
    'validate_prime_input(&g_crawler_state.frequency_input,',
    'validate_prime_input("crawler.frequency",'
)
content = content.replace(
    'validate_prime_input(&g_crawler_state.selection_input,',
    'validate_prime_input("crawler.selection",'
)
content = content.replace(
    'validate_prime_input(&g_crawler_state.delay_min_input,',
    'validate_prime_input("crawler.delay_min",'
)
content = content.replace(
    'validate_prime_input(&g_crawler_state.delay_max_input,',
    'validate_prime_input("crawler.delay_max",'
)

# Write back
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.write(content)

print("✓ Fixed crawler tab to prepare for InputManager integration")
print("Next: Add input registration and rendering calls")