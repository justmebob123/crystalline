#!/usr/bin/env python3
"""
Fix the crawler tab drawing functions to use TextInput rendering
"""

import re

# Read the file
with open('app/ui/tabs/tab_crawler.c', 'r') as f:
    content = f.read()

# Find and replace the prime config drawing section
# We need to update the drawing code to use text_input_render and set bounds

old_drawing = r'''        // Frequency Prime
        draw_text\(renderer, "Frequency:", x, y, text_color\);
        y \+= 18;
        
        uint64_t freq_val;
        bool freq_valid = validate_prime_input\(g_crawler_state\.frequency_input, &freq_val\);
        SDL_Color freq_color = freq_valid \? success_color : error_color;
        
        SDL_Rect freq_rect = \{x, y, 150, 22\};
        SDL_SetRenderDrawColor\(renderer, 30, 30, 40, 255\);
        SDL_RenderFillRect\(renderer, &freq_rect\);
        SDL_SetRenderDrawColor\(renderer, freq_color\.r, freq_color\.g, freq_color\.b, 255\);
        SDL_RenderDrawRect\(renderer, &freq_rect\);
        draw_text\(renderer, g_crawler_state\.frequency_input, x \+ 5, y \+ 5, text_color\);
        draw_text\(renderer, freq_valid \? "✓" : "✗", x \+ 160, y \+ 5, freq_color\);
        y \+= 30;'''

new_drawing = '''        // Frequency Prime
        draw_text(renderer, "Frequency:", x, y, text_color);
        y += 18;
        
        uint64_t freq_val;
        bool freq_valid = validate_prime_input(&g_crawler_state.frequency_input, &freq_val);
        SDL_Color freq_color = freq_valid ? success_color : error_color;
        
        // Update bounds and render TextInput
        g_crawler_state.frequency_input.bounds.x = x;
        g_crawler_state.frequency_input.bounds.y = y;
        g_crawler_state.frequency_input.bounds.w = 150;
        g_crawler_state.frequency_input.bounds.h = 22;
        text_input_render(&g_crawler_state.frequency_input, renderer, NULL);
        
        // Draw validation indicator
        draw_text(renderer, freq_valid ? "✓" : "✗", x + 160, y + 5, freq_color);
        y += 30;'''

content = re.sub(old_drawing, new_drawing, content, flags=re.DOTALL)

# Write back
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.write(content)

print("Fixed frequency input drawing")