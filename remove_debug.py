#!/usr/bin/env python3
"""Remove all DEBUG output from train_model.c"""

with open('tools/train_model.c', 'r') as f:
    lines = f.readlines()

new_lines = []
skip_next = False

for i, line in enumerate(lines):
    # Skip DEBUG printf lines and their fflush
    if 'printf("DEBUG:' in line or 'printf(\&quot;DEBUG:' in line:
        # Also skip the next line if it's fflush
        skip_next = True
        continue
    
    if skip_next and 'fflush(stdout)' in line:
        skip_next = False
        continue
    
    skip_next = False
    new_lines.append(line)

with open('tools/train_model.c', 'w') as f:
    f.writelines(new_lines)

print("Debug output removed from train_model.c")