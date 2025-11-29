#!/usr/bin/env python3
"""
Comprehensive fix for crawler tab input system
"""

import re

# Read the file
with open('app/ui/tabs/tab_crawler.c', 'r') as f:
    content = f.read()

# 1. Fix the state structure - replace char arrays with TextInput
content = re.sub(
    r'char frequency_input\[32\];',
    'TextInput frequency_input;',
    content
)
content = re.sub(
    r'char selection_input\[32\];',
    'TextInput selection_input;',
    content
)
content = re.sub(
    r'char delay_min_input\[32\];',
    'TextInput delay_min_input;',
    content
)
content = re.sub(
    r'char delay_max_input\[32\];',
    'TextInput delay_max_input;',
    content
)
content = re.sub(
    r'char add_link_input\[512\];',
    'TextInput add_link_input;',
    content
)
content = re.sub(
    r'int active_input; // 0=none, 1-4=prime inputs, 5=add_link',
    'TextInput* focused_input; // Currently focused input',
    content
)

# 2. Fix the init function
old_init = r'''static void init_crawler_tab_state\(void\) \{
    if \(g_crawler_state\.inputs_initialized\) return;
    
    // Initialize prime config with defaults
    prime_config_init_default\(&g_crawler_state\.prime_config\);
    g_crawler_state\.prime_enabled = true;
    
    // Convert defaults to strings
    snprintf\(g_crawler_state\.frequency_input, sizeof\(g_crawler_state\.frequency_input\), 
             "%lu", \(unsigned long\)g_crawler_state\.prime_config\.frequency_prime\);
    snprintf\(g_crawler_state\.selection_input, sizeof\(g_crawler_state\.selection_input\),
             "%lu", \(unsigned long\)g_crawler_state\.prime_config\.link_selection_prime\);
    snprintf\(g_crawler_state\.delay_min_input, sizeof\(g_crawler_state\.delay_min_input\),
             "%lu", \(unsigned long\)g_crawler_state\.prime_config\.delay_min_prime\);
    snprintf\(g_crawler_state\.delay_max_input, sizeof\(g_crawler_state\.delay_max_input\),
             "%lu", \(unsigned long\)g_crawler_state\.prime_config\.delay_max_prime\);
    
    // Initialize link queue
    g_crawler_state\.link_queue = link_queue_create\("data/crawler/link_queue\.txt"\);
    
    // Enable all URL patterns by default
    g_crawler_state\.pattern_href = true;
    g_crawler_state\.pattern_onclick = true;
    g_crawler_state\.pattern_data_attr = true;
    g_crawler_state\.pattern_meta_refresh = true;
    
    g_crawler_state\.inputs_initialized = true;
\}'''

new_init = '''static void init_crawler_tab_state(void) {
    if (g_crawler_state.inputs_initialized) return;
    
    // Initialize prime config with defaults
    prime_config_init_default(&g_crawler_state.prime_config);
    g_crawler_state.prime_enabled = true;
    
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

content = re.sub(old_init, new_init, content, flags=re.DOTALL)

# 3. Fix validate_prime_input function
old_validate = r'''static bool validate_prime_input\(const char\* input, uint64_t\* value\) \{
    char\* endptr;
    unsigned long val = strtoul\(input, &endptr, 10\);
    
    if \(\*endptr != '\\0' \|\| val == 0\) \{
        return false;
    \}
    
    if \(!is_prime\(val\)\) \{
        return false;
    \}
    
    \*value = val;
    return true;
\}'''

new_validate = '''static bool validate_prime_input(TextInput* input, uint64_t* value) {
    const char* text = text_input_get_text(input);
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

# 4. Fix all validate_prime_input calls
content = re.sub(
    r'validate_prime_input\(g_crawler_state\.frequency_input,',
    'validate_prime_input(&g_crawler_state.frequency_input,',
    content
)
content = re.sub(
    r'validate_prime_input\(g_crawler_state\.selection_input,',
    'validate_prime_input(&g_crawler_state.selection_input,',
    content
)
content = re.sub(
    r'validate_prime_input\(g_crawler_state\.delay_min_input,',
    'validate_prime_input(&g_crawler_state.delay_min_input,',
    content
)
content = re.sub(
    r'validate_prime_input\(g_crawler_state\.delay_max_input,',
    'validate_prime_input(&g_crawler_state.delay_max_input,',
    content
)

# Write back
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.write(content)

print("Applied comprehensive input system fixes")
print("- Updated state structure")
print("- Fixed init function")
print("- Fixed validate_prime_input function")
print("- Fixed all function calls")