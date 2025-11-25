#!/usr/bin/env python3

# Read the file
with open('algorithms/src/shared_memory.c', 'r') as f:
    lines = f.readlines()

# Find and fix the problematic line
for i, line in enumerate(lines):
    if 'region->original = (SharedMemoryRegion*)region->data; // Store original pointer' in line:
        # Replace with proper handling
        lines[i] = '                   // Free original data and replace with copy\n'
        lines.insert(i+1, '                   region->free_fn(region->data);\n')
        lines.insert(i+2, '                   region->original = NULL; // No parent region tracking\n')
        break

# Write back
with open('algorithms/src/shared_memory.c', 'w') as f:
    f.writelines(lines)

print("Fixed COW implementation")