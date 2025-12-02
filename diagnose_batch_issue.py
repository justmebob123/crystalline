#!/usr/bin/env python3
"""
Diagnose batch size issues with small datasets
"""

def calculate_batches(num_tokens, batch_size, seq_len, drop_last=False):
    """Calculate number of batches given dataset size"""
    usable_tokens = num_tokens - 1 if num_tokens > 0 else 0
    tokens_per_batch = batch_size * seq_len
    num_batches = usable_tokens // tokens_per_batch
    
    if not drop_last and (usable_tokens % tokens_per_batch) > 0:
        num_batches += 1
    
    return num_batches, tokens_per_batch, usable_tokens

# Test with various dataset sizes
test_cases = [
    (100, 32, 128),    # Very small
    (1000, 32, 128),   # Small
    (10000, 32, 128),  # Medium
    (50000, 32, 128),  # Larger
]

print("Dataset Size Analysis")
print("=" * 80)
print(f"{'Tokens':<10} {'Batch':<8} {'SeqLen':<8} {'Batches':<10} {'Tokens/Batch':<15} {'Usable':<10}")
print("-" * 80)

for num_tokens, batch_size, seq_len in test_cases:
    num_batches, tokens_per_batch, usable = calculate_batches(num_tokens, batch_size, seq_len, drop_last=False)
    print(f"{num_tokens:<10} {batch_size:<8} {seq_len:<8} {num_batches:<10} {tokens_per_batch:<15} {usable:<10}")
    
    if num_batches == 0:
        print(f"  ⚠️  WARNING: 0 batches! Need at least {tokens_per_batch} tokens")
        # Calculate minimum batch size
        min_batch_size = max(1, usable // seq_len)
        print(f"  ✓  Suggested batch_size: {min_batch_size}")
    print()

print("\nCRITICAL INSIGHT:")
print("If num_batches = 0, workers will never receive work and may hang!")
print("The application MUST auto-adjust batch_size before creating threads.")
