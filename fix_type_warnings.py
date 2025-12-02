#!/usr/bin/env python3
"""
Systematically fix float*/double* type warnings in the codebase.

Strategy:
1. Find all double* array allocations
2. Change all float* accesses to double*
3. Maintain consistency throughout
"""

import re
import sys

def fix_file(filepath, fixes):
    """Apply fixes to a file"""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
        
        original = content
        for pattern, replacement in fixes:
            content = re.sub(pattern, replacement, content)
        
        if content != original:
            with open(filepath, 'w') as f:
                f.write(content)
            return True
        return False
    except Exception as e:
        print(f"Error fixing {filepath}: {e}")
        return False

# Define fixes for each file
fixes = {
    'src/ai/cllm_training.c': [
        # Fix queries/keys/values accesses (already allocated as double*)
        (r'float\* (query|key|value) = &(queries|keys|values)\[', r'double* \1 = &\2['),
    ],
    'src/ai/cllm_positional.c': [
        # Already fixed - just verify
    ],
}

if __name__ == '__main__':
    print("Fixing type warnings...")
    for filepath, file_fixes in fixes.items():
        if fix_file(filepath, file_fixes):
            print(f"✓ Fixed {filepath}")
        else:
            print(f"  No changes needed in {filepath}")