#!/usr/bin/env python3
"""
Phase 3: Update all function call sites to use BigFixed versions

The BigFixed implementations already exist and are complete:
- cllm_feedforward_bigfixed()
- cllm_layer_norm_bigfixed()
- cllm_attention_forward_bigfixed()

We just need to update the CALL SITES to use these instead of the old float versions.
"""

import re
import os

def update_file_call_sites(filepath):
    """Update function calls in a file to use BigFixed versions"""
    
    if not os.path.exists(filepath):
        print(f"⚠️  File not found: {filepath}")
        return 0
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    original_content = content
    changes = 0
    
    # Replace function calls
    replacements = [
        # Feed-forward calls
        (r'\bcllm_feedforward\s*\(', 'cllm_feedforward_bigfixed(', 'feedforward'),
        (r'\bcllm_feed_forward\s*\(', 'cllm_feedforward_bigfixed(', 'feed_forward'),
        
        # Layer norm calls  
        (r'\bcllm_layer_norm_old\s*\(', 'cllm_layer_norm_bigfixed(', 'layer_norm_old'),
        (r'\bcllm_layer_norm\s*\(', 'cllm_layer_norm_bigfixed(', 'layer_norm'),
        
        # Attention calls (be careful not to replace the _bigfixed versions)
        (r'\bcllm_attention_forward\s*\((?!.*bigfixed)', 'cllm_attention_forward_bigfixed(', 'attention_forward'),
    ]
    
    for old_pattern, new_call, name in replacements:
        matches = re.findall(old_pattern, content)
        if matches:
            content = re.sub(old_pattern, new_call, content)
            changes += len(matches)
            print(f"  - Replaced {len(matches)} calls to {name}")
    
    if content != original_content:
        with open(filepath, 'w') as f:
            f.write(content)
        return changes
    
    return 0

def scan_and_update_all_files():
    """Scan all C files and update call sites"""
    
    # Files to check
    files_to_check = []
    
    # Scan src/ai directory
    for root, dirs, files in os.walk('src/ai'):
        for file in files:
            if file.endswith('.c') and not file.endswith('_bigfixed.c'):
                files_to_check.append(os.path.join(root, file))
    
    # Also check infrastructure
    for root, dirs, files in os.walk('src/ai/infrastructure'):
        for file in files:
            if file.endswith('.c'):
                files_to_check.append(os.path.join(root, file))
    
    total_changes = 0
    files_modified = 0
    
    for filepath in sorted(files_to_check):
        changes = update_file_call_sites(filepath)
        if changes > 0:
            print(f"✅ Updated {filepath} ({changes} changes)")
            total_changes += changes
            files_modified += 1
    
    return total_changes, files_modified

def main():
    print("=" * 80)
    print("PHASE 3: Update Call Sites to Use BigFixed Versions")
    print("=" * 80)
    print()
    print("Scanning all C files for function calls to update...")
    print()
    
    total_changes, files_modified = scan_and_update_all_files()
    
    print()
    print("=" * 80)
    print("PHASE 3 COMPLETE")
    print("=" * 80)
    print(f"\nTotal changes: {total_changes}")
    print(f"Files modified: {files_modified}")
    print("\nRun 'make' to check if warnings are reduced")

if __name__ == '__main__':
    main()