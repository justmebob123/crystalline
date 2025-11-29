#!/usr/bin/env python3
"""
Fix all the drawing code to use text_input_render instead of manual drawing
"""

with open('app/ui/tabs/tab_crawler.c', 'r') as f:
    lines = f.readlines()

output = []
i = 0
skip_until = -1

while i < len(lines):
    # Skip lines we've already processed
    if i < skip_until:
        i += 1
        continue
    
    line = lines[i]
    
    # Fix frequency input drawing
    if 'SDL_Rect freq_rect = {x, y, 150, 22};' in line:
        # Replace the manual drawing with TextInput rendering
        output.append('        // Update bounds and render TextInput\n')
        output.append('        g_crawler_state.frequency_input.bounds.x = x;\n')
        output.append('        g_crawler_state.frequency_input.bounds.y = y;\n')
        output.append('        g_crawler_state.frequency_input.bounds.w = 150;\n')
        output.append('        g_crawler_state.frequency_input.bounds.h = 22;\n')
        output.append('        text_input_render(&g_crawler_state.frequency_input, renderer, NULL);\n')
        output.append('        \n')
        # Skip the old drawing code (next 5 lines)
        skip_until = i + 6
        i += 1
        continue
    
    # Fix selection input drawing
    elif 'SDL_Rect sel_rect = {x, y, 150, 22};' in line:
        output.append('        // Update bounds and render TextInput\n')
        output.append('        g_crawler_state.selection_input.bounds.x = x;\n')
        output.append('        g_crawler_state.selection_input.bounds.y = y;\n')
        output.append('        g_crawler_state.selection_input.bounds.w = 150;\n')
        output.append('        g_crawler_state.selection_input.bounds.h = 22;\n')
        output.append('        text_input_render(&g_crawler_state.selection_input, renderer, NULL);\n')
        output.append('        \n')
        skip_until = i + 6
        i += 1
        continue
    
    # Fix delay_min input drawing
    elif 'SDL_Rect min_rect = {x, y, 150, 22};' in line:
        output.append('        // Update bounds and render TextInput\n')
        output.append('        g_crawler_state.delay_min_input.bounds.x = x;\n')
        output.append('        g_crawler_state.delay_min_input.bounds.y = y;\n')
        output.append('        g_crawler_state.delay_min_input.bounds.w = 150;\n')
        output.append('        g_crawler_state.delay_min_input.bounds.h = 22;\n')
        output.append('        text_input_render(&g_crawler_state.delay_min_input, renderer, NULL);\n')
        output.append('        \n')
        skip_until = i + 6
        i += 1
        continue
    
    # Fix delay_max input drawing
    elif 'SDL_Rect max_rect = {x, y, 150, 22};' in line:
        output.append('        // Update bounds and render TextInput\n')
        output.append('        g_crawler_state.delay_max_input.bounds.x = x;\n')
        output.append('        g_crawler_state.delay_max_input.bounds.y = y;\n')
        output.append('        g_crawler_state.delay_max_input.bounds.w = 150;\n')
        output.append('        g_crawler_state.delay_max_input.bounds.h = 22;\n')
        output.append('        text_input_render(&g_crawler_state.delay_max_input, renderer, NULL);\n')
        output.append('        \n')
        skip_until = i + 6
        i += 1
        continue
    
    # Fix add_link input drawing - this is a bigger block
    elif 'SDL_Rect input_rect = {x, y, col->width - (col->padding * 2), 22};' in line:
        output.append('    // Update bounds and render TextInput\n')
        output.append('    g_crawler_state.add_link_input.bounds.x = x;\n')
        output.append('    g_crawler_state.add_link_input.bounds.y = y;\n')
        output.append('    g_crawler_state.add_link_input.bounds.w = col->width - (col->padding * 2);\n')
        output.append('    g_crawler_state.add_link_input.bounds.h = 22;\n')
        output.append('    text_input_render(&g_crawler_state.add_link_input, renderer, NULL);\n')
        # Skip the old drawing code (about 15 lines)
        skip_until = i + 16
        i += 1
        continue
    
    else:
        output.append(line)
        i += 1

# Write back
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.writelines(output)

print("Fixed all drawing code to use text_input_render")