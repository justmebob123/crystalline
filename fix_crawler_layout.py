#!/usr/bin/env python3
"""Fix Crawler Tab Layout Issues"""

import re

# Read the file
with open('app/ui/tabs/tab_crawler.c', 'r') as f:
    content = f.read()

# Fix 1: Add content_width calculation in draw_column2_link_management
content = content.replace(
    '    int x = col->x + col->padding;\n    int y = col->y + col->padding;\n    \n    // Section header\n    draw_section_header(renderer, "LINK MANAGEMENT"',
    '    int x = col->x + col->padding;\n    int y = col->y + col->padding;\n    int content_width = col->width - (col->padding * 2);\n    \n    // Section header\n    draw_section_header(renderer, "LINK MANAGEMENT"'
)

# Fix 2: Replace hardcoded button widths in column 2 (Add/Clear buttons)
content = content.replace(
    '    // Add and Clear buttons - STORE BOUNDS FOR CLICK DETECTION\n    btn_add_url.bounds = (SDL_Rect){x, y, 80, 25};',
    '    // Add and Clear buttons - USE COLUMN WIDTH FOR SIZING\n    int button_width = (content_width - 10) / 2;  // Split column width between two buttons\n    btn_add_url.bounds = (SDL_Rect){x, y, button_width, 25};'
)

content = content.replace(
    '    btn_clear_url.bounds = (SDL_Rect){x + 90, y, 80, 25};',
    '    btn_clear_url.bounds = (SDL_Rect){x + button_width + 10, y, button_width, 25};'
)

# Fix 3: Add content_width calculation in draw_column3_status
content = content.replace(
    '    int x = col->x + col->padding;\n    int y = col->y + col->padding;\n    \n    // Section header\n    draw_section_header(renderer, "CRAWLER STATUS"',
    '    int x = col->x + col->padding;\n    int y = col->y + col->padding;\n    int content_width = col->width - (col->padding * 2);\n    \n    // Section header\n    draw_section_header(renderer, "CRAWLER STATUS"'
)

# Fix 4: Replace hardcoded button widths in column 3 (Start/Stop button)
content = content.replace(
    '    // Start/Stop button - STORE BOUNDS FOR CLICK DETECTION\n    // Use fixed width instead of full column to prevent extending past window\n    int button_width = 180;  // Fixed width that fits nicely\n    btn_start_crawler.bounds = (SDL_Rect){x, y, button_width, 35};',
    '    // Start/Stop button - USE COLUMN WIDTH FOR SIZING\n    btn_start_crawler.bounds = (SDL_Rect){x, y, content_width, 35};'
)

# Fix 5: Replace hardcoded button widths for Save/Load Config buttons
content = content.replace(
    '    // Save Config button - STORE BOUNDS FOR CLICK DETECTION\n    // Use fixed width to prevent extending past window\n    btn_save_config.bounds = (SDL_Rect){x, y, 180, 30};',
    '    // Save Config button - USE COLUMN WIDTH FOR SIZING\n    btn_save_config.bounds = (SDL_Rect){x, y, content_width, 30};'
)

content = content.replace(
    '    // Load Config button - STORE BOUNDS FOR CLICK DETECTION\n    // Use fixed width to prevent extending past window\n    btn_load_config.bounds = (SDL_Rect){x, y, 180, 30};',
    '    // Load Config button - USE COLUMN WIDTH FOR SIZING\n    btn_load_config.bounds = (SDL_Rect){x, y, content_width, 30};'
)

# Fix 6: Adjust title position to avoid submenu overlap (move down by 20px)
content = content.replace(
    '    // Draw main title (moved below submenu to avoid overlap)\n    draw_text(renderer, "WEB CRAWLER CONTROL CENTER", layout->content_area.x + 20,\n              layout->content_area.y + 10,',
    '    // Draw main title (positioned below submenu to avoid overlap)\n    draw_text(renderer, "WEB CRAWLER CONTROL CENTER", layout->content_area.x + 20,\n              layout->content_area.y + 30,'
)

# Write the fixed content
with open('app/ui/tabs/tab_crawler.c', 'w') as f:
    f.write(content)

print("✅ Fixed Crawler tab layout issues:")
print("  1. Added content_width calculation in column 2 and 3")
print("  2. Replaced hardcoded button widths (80px, 180px) with column-based sizing")
print("  3. Adjusted title position to avoid submenu overlap")
print("  4. All buttons now scale with column width")