#!/usr/bin/env python3
"""
Comprehensive fix for all crawler tab inputs
"""

import re

# Read the file
with open('app/ui/tabs/tab_crawler.c', 'r') as f:
    lines = f.readlines()

output = []
i = 0
while i < len(lines):
    line = lines[i]
    
    # Fix selection input drawing
    if 'draw_text(renderer, "Selection:", x, y, text_color);' in line:
        output.append(line)
        i += 1
        output.append(lines[i])  # y += 18
        i += 1
        output.append('\n')
        output.append('        uint64_t sel_val;\n')
        output.append('        bool sel_valid = validate_prime_input(&g_crawler_state.selection_input, &sel_val);\n')
        output.append('        SDL_Color sel_color = sel_valid ? success_color : error_color;\n')
        output.append('        \n')
        output.append('        // Update bounds and render TextInput\n')
        output.append('        g_crawler_state.selection_input.bounds.x = x;\n')
        output.append('        g_crawler_state.selection_input.bounds.y = y;\n')
        output.append('        g_crawler_state.selection_input.bounds.w = 150;\n')
        output.append('        g_crawler_state.selection_input.bounds.h = 22;\n')
        output.append('        text_input_render(&g_crawler_state.selection_input, renderer, NULL);\n')
        output.append('        \n')
        output.append('        // Draw validation indicator\n')
        output.append('        draw_text(renderer, sel_valid ? "✓" : "✗", x + 160, y + 5, sel_color);\n')
        output.append('        y += 30;\n')
        
        # Skip old drawing code
        while i < len(lines) and 'y += 30;' not in lines[i]:
            i += 1
        i += 1
        continue
    
    # Fix delay min input drawing
    elif 'draw_text(renderer, "Delay Min (sec):", x, y, text_color);' in line:
        output.append(line)
        i += 1
        output.append(lines[i])  # y += 18
        i += 1
        output.append('\n')
        output.append('        uint64_t min_val;\n')
        output.append('        bool min_valid = validate_prime_input(&g_crawler_state.delay_min_input, &min_val);\n')
        output.append('        SDL_Color min_color = min_valid ? success_color : error_color;\n')
        output.append('        \n')
        output.append('        // Update bounds and render TextInput\n')
        output.append('        g_crawler_state.delay_min_input.bounds.x = x;\n')
        output.append('        g_crawler_state.delay_min_input.bounds.y = y;\n')
        output.append('        g_crawler_state.delay_min_input.bounds.w = 150;\n')
        output.append('        g_crawler_state.delay_min_input.bounds.h = 22;\n')
        output.append('        text_input_render(&g_crawler_state.delay_min_input, renderer, NULL);\n')
        output.append('        \n')
        output.append('        // Draw validation indicator\n')
        output.append('        draw_text(renderer, min_valid ? "✓" : "✗", x + 160, y + 5, min_color);\n')
        output.append('        y += 30;\n')
        
        # Skip old drawing code
        while i < len(lines) and 'y += 30;' not in lines[i]:
            i += 1
        i += 1
        continue
    
    # Fix delay max input drawing
    elif 'draw_text(renderer, "Delay Max (sec):", x, y, text_color);' in line:
        output.append(line)
        i += 1
        output.append(lines[i])  # y += 18
        i += 1
        output.append('\n')
        output.append('        uint64_t max_val;\n')
        output.append('        bool max_valid = validate_prime_input(&g_crawler_state.delay_max_input, &max_val);\n')
        output.append('        SDL_Color max_color = max_valid ? success_color : error_color;\n')
        output.append('        \n')
        output.append('        // Update bounds and render TextInput\n')
        output.append('        g_crawler_state.delay_max_input.bounds.x = x;\n')
        output.append('        g_crawler_state.delay_max_input.bounds.y = y;\n')
        output.append('        g_crawler_state.delay_max_input.bounds.w = 150;\n')
        output.append('        g_crawler_state.delay_max_input.bounds.h = 22;\n')
        output.append('        text_input_render(&g_crawler_state.delay_max_input, renderer, NULL);\n')
        output.append('        \n')
        output.append('        // Draw validation indicator\n')
        output.append('        draw_text(renderer, max_valid ? "✓" : "✗", x + 160, y + 5, max_color);\n')
        output.append('        y += 30;\n')
        
        # Skip old drawing code
        while i < len(lines) and 'y += 30;' not in lines[i]:
            i += 1
        i += 1
        continue
    
    else:
        output.append(line)
        i += 1

# Write back
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.writelines(output)

print("Fixed all prime input fields")