#!/usr/bin/env python3

# Read the file
with open('src/ai/cllm_training.c', 'r') as f:
    lines = f.readlines()

# Find and rename the temp variables to avoid redefinition
# Line 2396 should be temp_attn
# Line 2440 should be temp_ff

for i, line in enumerate(lines):
    line_num = i + 1
    
    # Rename temp at line 2396 to temp_attn
    if line_num == 2396 and 'BigFixed* temp = big_fixed_create' in line:
        lines[i] = line.replace('BigFixed* temp =', 'BigFixed* temp_attn =')
        print(f"Line {line_num}: Renamed to temp_attn")
    
    # Update references to temp in attention section (lines 2396-2430)
    if 2396 <= line_num <= 2430 and 'temp' in line and 'temp_attn' not in line:
        if 'big_fixed_mul(temp,' in line or 'big_fixed_add(' in line and ', temp)' in line:
            lines[i] = line.replace('temp,', 'temp_attn,').replace(', temp)', ', temp_attn)')
            print(f"Line {line_num}: Updated temp reference to temp_attn")
    
    # Free temp_attn
    if line_num == 2429 and 'big_fixed_free(temp)' in line:
        lines[i] = line.replace('big_fixed_free(temp)', 'big_fixed_free(temp_attn)')
        print(f"Line {line_num}: Updated free to temp_attn")
    
    # Rename temp at line 2440 to temp_ff
    if line_num == 2440 and 'BigFixed* temp = big_fixed_create' in line:
        lines[i] = line.replace('BigFixed* temp =', 'BigFixed* temp_ff =')
        print(f"Line {line_num}: Renamed to temp_ff")
    
    # Update references to temp in feedforward section (lines 2440-2500)
    if 2440 <= line_num <= 2500 and 'temp' in line and 'temp_ff' not in line:
        if 'big_fixed_mul(temp,' in line or 'big_fixed_add(' in line and ', temp)' in line:
            lines[i] = line.replace('temp,', 'temp_ff,').replace(', temp)', ', temp_ff)')
            print(f"Line {line_num}: Updated temp reference to temp_ff")
    
    # Free temp_ff
    if line_num == 2495 and 'big_fixed_free(temp)' in line:
        lines[i] = line.replace('big_fixed_free(temp)', 'big_fixed_free(temp_ff)')
        print(f"Line {line_num}: Updated free to temp_ff")

# Write back
with open('src/ai/cllm_training.c', 'w') as f:
    f.writelines(lines)

print("SUCCESS: Fixed temp variable redefinitions")