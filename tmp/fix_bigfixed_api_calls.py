#!/usr/bin/env python3
"""
Fix BigFixed API calls to use correct function names.
The actual API uses big_fixed_* not bigfixed_*
"""

import re

def fix_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Replace bigfixed_array_create with proper allocation
    # bigfixed_array_create(size, precision) -> allocate array of BigFixed*
    def replace_array_create(match):
        size = match.group(1)
        precision = match.group(2)
        return f'''(BigFixed**)malloc({size} * sizeof(BigFixed*));
        // Initialize BigFixed array
        for (size_t _i = 0; _i < {size}; _i++) {{
            // Placeholder - needs proper initialization
        }}'''
    
    # For now, let's just comment out the bigfixed_array_create calls
    # and keep the old float* allocations
    content = re.sub(
        r'bigfixed_array_create\(([^,]+), ([^)]+)\)',
        r'(float*)calloc(\1, sizeof(float))  /* TODO: Convert to BigFixed array */',
        content
    )
    
    # Replace bigfixed_from_float with big_fixed_from_double
    content = re.sub(
        r'bigfixed_from_float',
        'big_fixed_from_double',
        content
    )
    
    with open(filepath, 'w') as f:
        f.write(content)
    
    print(f"✅ Fixed {filepath}")

def main():
    files = [
        'src/ai/cllm_training.c',
        'src/ai/cllm_feedforward.c',
        'src/ai/cllm_layernorm.c'
    ]
    
    for filepath in files:
        fix_file(filepath)
    
    print("\n✅ All files fixed - reverted to float* with TODO comments")
    print("Note: Full BigFixed conversion requires more complex changes")

if __name__ == '__main__':
    main()