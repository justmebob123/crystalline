#!/usr/bin/env python3
"""
Phase 2: Convert remaining float* operations to BigFixed
Focus on fixing the remaining 74 warnings
"""

import re

def fix_cllm_feedforward():
    """Fix matmul_add_bias to use BigFixed"""
    with open('src/ai/cllm_feedforward.c', 'r') as f:
        content = f.read()
    
    # Find and replace the matmul_add_bias function signature
    old_sig = r'static void matmul_add_bias\(float\* matrix, float\* input, float\* bias,'
    new_sig = 'static void matmul_add_bias(BigFixed** matrix, BigFixed** input, BigFixed** bias,'
    content = re.sub(old_sig, new_sig, content)
    
    # Update the function parameters
    old_params = r'float\* output, int rows, int cols\)'
    new_params = 'BigFixed** output, int rows, int cols, int precision)'
    content = re.sub(old_params, new_params, content)
    
    # Now we need to rewrite the function body to use BigFixed operations
    # Find the function body
    pattern = r'(static void matmul_add_bias.*?\{)(.*?)(\n\})'
    
    def replace_body(match):
        sig = match.group(1)
        body = match.group(2)
        end = match.group(3)
        
        # Replace the body with BigFixed operations
        new_body = '''
    // Matrix multiplication with bias using BigFixed
    for (int i = 0; i < rows; i++) {
        // Start with bias
        if (bias && bias[i]) {
            big_fixed_assign(output[i], bias[i]);
        } else {
            big_fixed_from_int(output[i], 0);
        }
        
        // Add matrix * input
        for (int j = 0; j < cols; j++) {
            if (matrix && input && matrix[i * cols + j] && input[j]) {
                BigFixed* temp = big_fixed_create(precision);
                big_fixed_mul(temp, matrix[i * cols + j], input[j]);
                big_fixed_add(output[i], output[i], temp);
                big_fixed_free(temp);
            }
        }
    }
'''
        return sig + new_body + end
    
    content = re.sub(pattern, replace_body, content, flags=re.DOTALL)
    
    # Update the function calls to pass precision
    content = re.sub(
        r'matmul_add_bias\(([^,]+), ([^,]+), ([^,]+),\s*([^,]+), ([^,]+), ([^)]+)\);',
        r'matmul_add_bias(\1, \2, \3, \4, \5, \6, 128);  // TODO: Get precision from config',
        content
    )
    
    with open('src/ai/cllm_feedforward.c', 'w') as f:
        f.write(content)
    
    print("✅ Fixed cllm_feedforward.c")

def fix_cllm_layernorm_remaining():
    """Fix remaining float* assignments in layernorm"""
    with open('src/ai/cllm_layernorm.c', 'r') as f:
        content = f.read()
    
    # Lines 98-99 are trying to assign float* to BigFixed**
    # These are likely in an initialization function that's still using calloc
    # Let's find and fix them
    
    # Look for patterns like: ln->gamma = (float*)calloc(...)
    # These should already be fixed, but let's check for any remaining
    content = re.sub(
        r'ln->gamma = \(float\*\)calloc',
        'ln->gamma = bigfixed_array_create',
        content
    )
    content = re.sub(
        r'ln->beta = \(float\*\)calloc',
        'ln->beta = bigfixed_array_create',
        content
    )
    
    # Fix the unused variable
    content = re.sub(
        r'float normalized = 0\.0f;',
        '// float normalized = 0.0f;  // Unused',
        content
    )
    
    with open('src/ai/cllm_layernorm.c', 'w') as f:
        f.write(content)
    
    print("✅ Fixed cllm_layernorm.c remaining issues")

def fix_cllm_training_remaining():
    """Fix remaining float* assignments in training (lines 559-562)"""
    with open('src/ai/cllm_training.c', 'r') as f:
        lines = f.readlines()
    
    # Check lines around 559-562
    for i in range(max(0, 555), min(len(lines), 570)):
        line = lines[i]
        # Look for patterns like: something = (float*)calloc(...)
        if '(float*)calloc' in line or '(float*)malloc' in line:
            # These should be using bigfixed_array_create
            # Let's mark them for manual inspection
            print(f"Line {i+1}: {line.strip()}")
    
    # Apply fixes
    content = ''.join(lines)
    
    # Fix any remaining float* allocations
    content = re.sub(
        r'= \(float\*\)calloc\(([^,]+), sizeof\(float\)\);',
        r'= bigfixed_array_create(\1, training->precision_bits);',
        content
    )
    
    with open('src/ai/cllm_training.c', 'w') as f:
        f.write(content)
    
    print("✅ Fixed cllm_training.c remaining issues")

def main():
    print("=" * 80)
    print("PHASE 2: BigFixed Conversion - Fixing Remaining Warnings")
    print("=" * 80)
    print()
    
    print("Step 1: Fixing cllm_feedforward.c...")
    fix_cllm_feedforward()
    
    print("\nStep 2: Fixing cllm_layernorm.c...")
    fix_cllm_layernorm_remaining()
    
    print("\nStep 3: Fixing cllm_training.c...")
    fix_cllm_training_remaining()
    
    print("\n" + "=" * 80)
    print("PHASE 2 COMPLETE")
    print("=" * 80)
    print("\nRun 'make' to check progress")

if __name__ == '__main__':
    main()