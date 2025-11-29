#!/usr/bin/env python3
"""
Fix the add_link input field to use TextInput
"""

# Read the file
with open('app/ui/tabs/tab_crawler.c', 'r') as f:
    content = f.read()

# Find and replace the add_link input drawing
old_code = '''    // Add URL input
    draw_text(renderer, "Add URL:", x, y, text_color);
    y += 18;
    
    SDL_Rect input_rect = {x, y, col->width - (col->padding * 2), 22};
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderFillRect(renderer, &input_rect);
    SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
    SDL_RenderDrawRect(renderer, &input_rect);
    
    // Truncate if too long
    char display_url[50];
    if (strlen(g_crawler_state.add_link_input) > 45) {
        strncpy(display_url, g_crawler_state.add_link_input, 42);
        display_url[42] = '.';
        display_url[43] = '.';
        display_url[44] = '.';
        display_url[45] = '\\0';
    } else {
        strcpy(display_url, g_crawler_state.add_link_input);
    }
    draw_text(renderer, display_url, x + 5, y + 5, text_color);'''

new_code = '''    // Add URL input
    draw_text(renderer, "Add URL:", x, y, text_color);
    y += 18;
    
    // Update bounds and render TextInput
    g_crawler_state.add_link_input.bounds.x = x;
    g_crawler_state.add_link_input.bounds.y = y;
    g_crawler_state.add_link_input.bounds.w = col->width - (col->padding * 2);
    g_crawler_state.add_link_input.bounds.h = 22;
    text_input_render(&g_crawler_state.add_link_input, renderer, NULL);'''

content = content.replace(old_code, new_code)

# Write back
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.write(content)

print("Fixed add_link input field")