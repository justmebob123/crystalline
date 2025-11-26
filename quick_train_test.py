#!/usr/bin/env python3
"""
Quick training and inference test for CLLM system
"""

import subprocess
import os
import time
import sys

def run_command(cmd, timeout=300):
    """Run a command and return output"""
    print(f"\n{'='*60}")
    print(f"Running: {cmd}")
    print(f"{'='*60}\n")
    
    try:
        result = subprocess.run(
            cmd,
            shell=True,
            capture_output=True,
            text=True,
            timeout=timeout,
            cwd="/workspace"
        )
        print(result.stdout)
        if result.stderr:
            print("STDERR:", result.stderr)
        return result.returncode == 0, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        print(f"Command timed out after {timeout} seconds")
        return False, "", "Timeout"

def main():
    print("\n" + "="*60)
    print("CLLM QUICK TRAINING AND INFERENCE TEST")
    print("="*60 + "\n")
    
    # Set library path
    os.environ['LD_LIBRARY_PATH'] = '/workspace'
    
    # Step 1: Train a very small model on limited data
    print("\n### STEP 1: Training Model ###\n")
    train_cmd = (
        "cd /workspace && "
        "LD_LIBRARY_PATH=. ./tools/train_model ./src/ai/cllm_attention.c "
        "--epochs 2 "
        "--batch-size 4 "
        "--seq-len 16 "
        "--learning-rate 0.001 "
        "--threads 4 "
        "--vocab-size 1000 "
        "--embed-dim 64 "
        "--num-layers 2 "
        "--num-heads 2 "
        "--checkpoint-dir ./checkpoints"
    )
    
    success, stdout, stderr = run_command(train_cmd, timeout=180)
    
    if not success:
        print("\n❌ Training failed or timed out")
        print("Let's check what we have in checkpoints...")
    else:
        print("\n✅ Training completed successfully")
    
    # Step 2: Check what was created
    print("\n### STEP 2: Checking Checkpoints ###\n")
    run_command("ls -lh /workspace/checkpoints/")
    
    # Step 3: Try inference if we have a model
    print("\n### STEP 3: Testing Inference ###\n")
    
    # Check if we have a model checkpoint
    checkpoint_check = subprocess.run(
        "ls /workspace/checkpoints/*.bin 2>/dev/null | grep -v dataset | head -1",
        shell=True,
        capture_output=True,
        text=True
    )
    
    if checkpoint_check.stdout.strip():
        model_path = checkpoint_check.stdout.strip()
        print(f"Found model checkpoint: {model_path}")
        
        # Try inference
        inference_cmd = (
            f"cd /workspace && "
            f"LD_LIBRARY_PATH=. ./tools/cllm_inference "
            f"{model_path} "
            f"./checkpoints/vocab.txt "
            f"'int main' "
            f"--max-tokens 50"
        )
        
        success, stdout, stderr = run_command(inference_cmd, timeout=60)
        
        if success:
            print("\n✅ Inference completed successfully")
        else:
            print("\n❌ Inference failed")
    else:
        print("❌ No model checkpoint found")
        print("Training may not have completed successfully")
    
    print("\n" + "="*60)
    print("TEST COMPLETE")
    print("="*60 + "\n")

if __name__ == "__main__":
    main()