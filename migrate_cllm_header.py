#!/usr/bin/env python3
"""
Migrate CLLMModel structure to use BigFixed and CrystallineEmbeddings
"""

def migrate_cllm_header():
    with open('include/cllm.h', 'r') as f:
        lines = f.readlines()
    
    # Find and replace the float* weights line
    for i, line in enumerate(lines):
        if 'float* weights;' in line and 'Model weights' in line:
            lines[i] = '    \n'
            lines.insert(i+1, '    // BIGFIXED WEIGHTS - Arbitrary precision\n')
            lines.insert(i+2, '    BigFixed** weights;          // Model weights (arbitrary precision)\n')
            print(f"✓ Replaced weights at line {i}")
            break
    
    # Find and replace the Embeddings embeddings line
    for i, line in enumerate(lines):
        if 'Embeddings embeddings;' in line and 'Token embeddings' in line:
            lines[i] = '    \n'
            lines.insert(i+1, '    // CRYSTALLINE EMBEDDINGS - Arbitrary precision BigFixed-based\n')
            lines.insert(i+2, '    CrystallineEmbeddings* crystalline_embeddings;  // Pure lattice-based embeddings\n')
            lines.insert(i+3, '    \n')
            lines.insert(i+4, '    // DEPRECATED: Legacy float embeddings (for backward compatibility only)\n')
            lines.insert(i+5, '    Embeddings embeddings;       // DEPRECATED: Use crystalline_embeddings instead\n')
            print(f"✓ Replaced embeddings at line {i}")
            break
    
    # Find the end of CLLMModel and add precision config
    for i, line in enumerate(lines):
        if '} CLLMModel;' in line:
            # Insert before the closing brace
            lines.insert(i, '       \n')
            lines.insert(i+1, '       // NEW: Arbitrary precision configuration\n')
            lines.insert(i+2, '       int precision_bits;          // Precision for BigFixed operations (default: 256)\n')
            lines.insert(i+3, '       bool use_bigfixed;           // Always true - for migration tracking only\n')
            print(f"✓ Added precision config at line {i}")
            break
    
    # Update the comment at the top of CLLMModel
    for i, line in enumerate(lines):
        if '* CLLM Model - Complete model structure' in line:
            lines[i] = ' * CLLM Model - Complete model structure with BigFixed arbitrary precision\n'
            print(f"✓ Updated comment at line {i}")
            break
    
    # Write back
    with open('include/cllm.h', 'w') as f:
        f.writelines(lines)
    
    print("✓ Successfully migrated include/cllm.h")
    return True

if __name__ == '__main__':
    success = migrate_cllm_header()
    exit(0 if success else 1)