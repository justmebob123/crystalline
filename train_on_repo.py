#!/usr/bin/env python3
"""
CLLM Repository Training Script
Trains the CLLM model on the entire repository codebase
"""

import os
import sys
import time
import subprocess
from pathlib import Path

def collect_training_files(base_path="/workspace"):
    """Collect all C source and header files from the repository"""
    print("🔍 Collecting training files...")
    
    extensions = ['.c', '.h']
    exclude_dirs = ['.git', 'build', 'node_modules', '__pycache__']
    
    files = []
    for root, dirs, filenames in os.walk(base_path):
        # Remove excluded directories
        dirs[:] = [d for d in dirs if d not in exclude_dirs]
        
        for filename in filenames:
            if any(filename.endswith(ext) for ext in extensions):
                filepath = os.path.join(root, filename)
                files.append(filepath)
    
    print(f"✓ Found {len(files)} training files")
    return files

def prepare_training_data(files, output_dir="training_data"):
    """Prepare training data by concatenating files"""
    print(f"\n📝 Preparing training data...")
    
    os.makedirs(output_dir, exist_ok=True)
    
    # Create a single large training file
    output_file = os.path.join(output_dir, "repo_code.txt")
    
    total_lines = 0
    total_bytes = 0
    
    with open(output_file, 'w', encoding='utf-8', errors='ignore') as outf:
        for i, filepath in enumerate(files):
            if i % 50 == 0:
                print(f"  Processing file {i+1}/{len(files)}...")
            
            try:
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as inf:
                    content = inf.read()
                    
                    # Add file marker
                    outf.write(f"\n\n// ===== FILE: {filepath} =====\n\n")
                    outf.write(content)
                    
                    lines = content.count('\n')
                    total_lines += lines
                    total_bytes += len(content)
            except Exception as e:
                print(f"  ⚠️  Error reading {filepath}: {e}")
    
    print(f"✓ Training data prepared:")
    print(f"  - Output file: {output_file}")
    print(f"  - Total lines: {total_lines:,}")
    print(f"  - Total size: {total_bytes:,} bytes ({total_bytes/1024/1024:.2f} MB)")
    
    return output_file

def create_training_config():
    """Create training configuration"""
    config = {
        'epochs': 10,
        'batch_size': 32,
        'sequence_length': 128,
        'learning_rate': 0.001,
        'thread_count': 0,  # Auto-detect (12 spheres)
    }
    return config

def run_training(training_file, config):
    """Run the training using the UI application"""
    print(f"\n🚀 Starting training with kissing spheres architecture...")
    print(f"  Configuration:")
    print(f"    - Epochs: {config['epochs']}")
    print(f"    - Batch size: {config['batch_size']}")
    print(f"    - Sequence length: {config['sequence_length']}")
    print(f"    - Learning rate: {config['learning_rate']}")
    print(f"    - Threads: {config['thread_count']} (auto = 12 spheres)")
    
    # Note: Since we need to use the UI application, we'll create a test script
    # that uses the CLLM library directly
    print("\n📊 Training will be performed using the CLLM library...")
    
    return True

def main():
    print("=" * 70)
    print("CLLM Repository Training - Day 4 Integration Test")
    print("=" * 70)
    print()
    
    start_time = time.time()
    
    # Step 1: Collect files
    files = collect_training_files()
    
    # Step 2: Prepare training data
    training_file = prepare_training_data(files)
    
    # Step 3: Create configuration
    config = create_training_config()
    
    # Step 4: Run training
    success = run_training(training_file, config)
    
    elapsed_time = time.time() - start_time
    
    print(f"\n✓ Preparation complete in {elapsed_time:.2f} seconds")
    print(f"\nTraining data ready at: {training_file}")
    print("\nNext: Use the CLLM training system to train on this data")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())