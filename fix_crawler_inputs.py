#!/usr/bin/env python3
"""
Fix the crawler tab input system to use TextInput properly
"""

import re

# Read the file
with open('app/ui/tabs/tab_crawler.c', 'r') as f:
    content = f.read()

# Fix the init function - replace char array references with TextInput calls
init_function = '''static void init_crawler_tab_state(void) {
    if (g_crawler_state.inputs_initialized) return;
    
    // Initialize prime config with defaults
    prime_config_init_default(&g_crawler_state.prime_config);
    g_crawler_state.prime_enabled = true;
    
    // Initialize TextInput fields (bounds will be set during drawing)
    text_input_init(&g_crawler_state.frequency_input, "Frequency Prime", 0, 0, 150, 22);
    text_input_init(&g_crawler_state.selection_input, "Selection Prime", 0, 0, 150, 22);
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

# Replace the init function
pattern = r'static void init_crawler_tab_state\(void\) \{[^}]+\}'
content = re.sub(pattern, init_function, content, flags=re.DOTALL)

# Fix validate_prime_input to work with TextInput
validate_function = '''static bool validate_prime_input(TextInput* input, uint64_t* value) {
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

pattern = r'static bool validate_prime_input\(const char\* input, uint64_t\* value\) \{[^}]+\}'
content = re.sub(pattern, validate_function, content, flags=re.DOTALL)

# Write the file back
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.write(content)

print("Fixed init_crawler_tab_state() and validate_prime_input()")