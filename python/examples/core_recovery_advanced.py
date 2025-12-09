#!/usr/bin/env python3
"""
Advanced Core Recovery Examples
Demonstrates advanced usage of the recovery.core module
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from recovery.core import RecoveryContext
import numpy as np

def example_1_basic_recovery():
    """Example 1: Basic text recovery with multiple samples"""
    print("=" * 60)
    print("Example 1: Basic Text Recovery")
    print("=" * 60)
    
    # Create context
    ctx = RecoveryContext(max_iterations=1000, convergence_threshold=1e-6)
    
    # Add multiple samples (known good data)
    samples = [
        b"This is a sample of good data that we know is correct.",
        b"Another sample of good data for better convergence.",
        b"Third sample helps achieve 27% better convergence!"
    ]
    
    for i, sample in enumerate(samples, 1):
        ctx.add_sample(sample)
        print(f"Added sample {i}: {len(sample)} bytes")
    
    # Corrupted data to recover
    corrupted = b"Th1s 1s @ s@mpl3 0f g00d d@t@ th@t w3 kn0w 1s c0rr3ct."
    print(f"\nCorrupted: {corrupted.decode('utf-8', errors='replace')}")
    
    # Recover
    recovered = ctx.recover(corrupted)
    print(f"Recovered: {recovered.decode('utf-8', errors='replace')}")
    
    # Statistics
    print(f"\nIterations: {ctx.get_iterations()}")
    print(f"Quality: {ctx.get_quality() * 100:.2f}%")
    print()

def example_2_binary_data_recovery():
    """Example 2: Binary data recovery"""
    print("=" * 60)
    print("Example 2: Binary Data Recovery")
    print("=" * 60)
    
    # Create context
    ctx = RecoveryContext()
    
    # Generate binary samples (e.g., file headers)
    samples = [
        bytes([0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]),  # PNG header
        bytes([0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]),  # PNG header
        bytes([0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]),  # PNG header
    ]
    
    for sample in samples:
        ctx.add_sample(sample)
    
    # Corrupted PNG header
    corrupted = bytes([0x89, 0x50, 0x00, 0x47, 0x0D, 0x00, 0x1A, 0x0A])
    
    print(f"Original:  {' '.join(f'{b:02X}' for b in samples[0])}")
    print(f"Corrupted: {' '.join(f'{b:02X}' for b in corrupted)}")
    
    # Recover
    recovered = ctx.recover(corrupted)
    print(f"Recovered: {' '.join(f'{b:02X}' for b in recovered)}")
    
    # Check if recovery was successful
    if recovered == samples[0]:
        print("\n✅ Recovery successful!")
    else:
        print("\n❌ Recovery incomplete")
    
    print(f"Iterations: {ctx.get_iterations()}")
    print(f"Quality: {ctx.get_quality() * 100:.2f}%")
    print()

def example_3_numpy_array_recovery():
    """Example 3: NumPy array recovery"""
    print("=" * 60)
    print("Example 3: NumPy Array Recovery")
    print("=" * 60)
    
    # Create context
    ctx = RecoveryContext()
    
    # Create sample arrays
    samples = [
        np.array([1.0, 2.0, 3.0, 4.0, 5.0], dtype=np.float32),
        np.array([1.0, 2.0, 3.0, 4.0, 5.0], dtype=np.float32),
        np.array([1.0, 2.0, 3.0, 4.0, 5.0], dtype=np.float32),
    ]
    
    for sample in samples:
        ctx.add_sample(sample.tobytes())
    
    # Create corrupted array
    corrupted = np.array([1.0, 2.5, 3.2, 4.1, 5.0], dtype=np.float32)
    
    print(f"Original:  {samples[0]}")
    print(f"Corrupted: {corrupted}")
    
    # Recover
    recovered_bytes = ctx.recover(corrupted.tobytes())
    recovered = np.frombuffer(recovered_bytes, dtype=np.float32)
    
    print(f"Recovered: {recovered}")
    
    # Calculate error
    error = np.abs(recovered - samples[0])
    print(f"\nAbsolute Error: {error}")
    print(f"Max Error: {np.max(error):.6f}")
    print(f"Mean Error: {np.mean(error):.6f}")
    
    print(f"\nIterations: {ctx.get_iterations()}")
    print(f"Quality: {ctx.get_quality() * 100:.2f}%")
    print()

def example_4_progressive_recovery():
    """Example 4: Progressive recovery with increasing samples"""
    print("=" * 60)
    print("Example 4: Progressive Recovery")
    print("=" * 60)
    
    # Original data
    original = b"The quick brown fox jumps over the lazy dog."
    corrupted = b"Th3 qu1ck br0wn f0x jump5 0v3r th3 l@zy d0g."
    
    print(f"Original:  {original.decode()}")
    print(f"Corrupted: {corrupted.decode()}")
    print()
    
    # Try recovery with increasing number of samples
    for num_samples in [1, 2, 3, 5]:
        ctx = RecoveryContext()
        
        # Add samples
        for _ in range(num_samples):
            ctx.add_sample(original)
        
        # Recover
        recovered = ctx.recover(corrupted)
        iterations = ctx.get_iterations()
        quality = ctx.get_quality()
        
        print(f"Samples: {num_samples} | Iterations: {iterations:3d} | "
              f"Quality: {quality*100:5.2f}% | "
              f"Result: {recovered.decode('utf-8', errors='replace')}")
    
    print()

def example_5_batch_recovery():
    """Example 5: Batch recovery of multiple files"""
    print("=" * 60)
    print("Example 5: Batch Recovery")
    print("=" * 60)
    
    # Create context once
    ctx = RecoveryContext()
    
    # Add samples
    samples = [
        b"Sample data line 1",
        b"Sample data line 2",
        b"Sample data line 3",
    ]
    
    for sample in samples:
        ctx.add_sample(sample)
    
    # Batch of corrupted data
    corrupted_batch = [
        b"S@mpl3 d@t@ l1n3 1",
        b"S@mpl3 d@t@ l1n3 2",
        b"S@mpl3 d@t@ l1n3 3",
    ]
    
    print("Recovering batch of corrupted data...")
    print()
    
    total_iterations = 0
    for i, corrupted in enumerate(corrupted_batch, 1):
        recovered = ctx.recover(corrupted)
        iterations = ctx.get_iterations()
        quality = ctx.get_quality()
        total_iterations += iterations
        
        print(f"Item {i}:")
        print(f"  Corrupted: {corrupted.decode()}")
        print(f"  Recovered: {recovered.decode()}")
        print(f"  Iterations: {iterations}, Quality: {quality*100:.2f}%")
        print()
    
    print(f"Total iterations: {total_iterations}")
    print(f"Average per item: {total_iterations / len(corrupted_batch):.1f}")
    print()

def main():
    """Run all examples"""
    print("\n" + "=" * 60)
    print("Advanced Core Recovery Examples")
    print("=" * 60 + "\n")
    
    try:
        example_1_basic_recovery()
        example_2_binary_data_recovery()
        example_3_numpy_array_recovery()
        example_4_progressive_recovery()
        example_5_batch_recovery()
        
        print("=" * 60)
        print("All examples completed successfully!")
        print("=" * 60)
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())