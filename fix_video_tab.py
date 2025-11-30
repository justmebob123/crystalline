#!/usr/bin/env python3
"""Fix Video Tab Unicode and Layout Issues"""

# Read the file
with open('app/ui/tabs/tab_video.c', 'r') as f:
    content = f.read()

# Fix 1: Replace Unicode checkmark with ASCII
content = content.replace(
    '"✓ FFmpeg: Installed"',
    '"[OK] FFmpeg: Installed"'
)

# Fix 2: Replace Unicode X mark with ASCII
content = content.replace(
    '"✗ FFmpeg: Not Found"',
    '"[X] FFmpeg: Not Found"'
)

# Fix 3: Replace Unicode bullets with ASCII
content = content.replace(
    '"• Recording captures the main visualization area"',
    '"- Recording captures the main visualization area"'
)

content = content.replace(
    '"• Switch between tabs while recording to capture different views"',
    '"- Switch between tabs while recording to capture different views"'
)

content = content.replace(
    '"• Recording may impact performance slightly"',
    '"- Recording may impact performance slightly"'
)

# Fix 4: Fix path truncation bug (line 105-109)
# The current code has a bug where it tries to write to path_info after it's been truncated
old_truncation = '''           // File path (safely truncate if needed)
           char path_info[512];
           int path_written = snprintf(path_info, sizeof(path_info), "Output: %s", state->video_path);
           if (path_written >= (int)sizeof(path_info)) {
               // Path was truncated, add ellipsis
               strcpy(path_info + sizeof(path_info) - 4, "...");
           }'''

new_truncation = '''           // File path (safely truncate if needed)
           char path_info[512];
           int path_written = snprintf(path_info, sizeof(path_info), "Output: %s", state->video_path);
           if (path_written >= (int)sizeof(path_info)) {
               // Path was truncated, add ellipsis at end
               path_info[sizeof(path_info) - 4] = '.';
               path_info[sizeof(path_info) - 3] = '.';
               path_info[sizeof(path_info) - 2] = '.';
               path_info[sizeof(path_info) - 1] = '\\0';
           }'''

content = content.replace(old_truncation, new_truncation)

# Fix 5: Adjust title position to avoid overlap (move down by 20px)
content = content.replace(
    '    // Title\n    draw_text(renderer, "VIDEO RECORDING", x, y, text_color);\n    y += 40;',
    '    // Title (positioned to avoid submenu overlap)\n    draw_text(renderer, "VIDEO RECORDING", x, y + 20, text_color);\n    y += 60;'
)

# Write the fixed content
with open('app/ui/tabs/tab_video.c', 'w') as f:
    f.write(content)

print("✅ Fixed Video tab issues:")
print("  1. Replaced Unicode checkmark (✓) with ASCII [OK]")
print("  2. Replaced Unicode X mark (✗) with ASCII [X]")
print("  3. Replaced Unicode bullets (•) with ASCII dashes (-)")
print("  4. Fixed path truncation bug (proper null termination)")
print("  5. Adjusted title position to avoid submenu overlap")