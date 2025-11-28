#!/bin/bash

echo "Creating 100MB+ training dataset..."
mkdir -p data/large_dataset

# Create diverse text content
cat > data/large_dataset/base_text.txt << 'BASETEXT'
The quick brown fox jumps over the lazy dog.
Machine learning models learn patterns from data.
Neural networks consist of layers of interconnected nodes.
Backpropagation is used to train deep learning models.
Gradient descent optimizes the loss function.
Attention mechanisms allow models to focus on relevant information.
Transformers revolutionized natural language processing.
Large language models can generate human-like text.
Training requires significant computational resources.
Parallel processing accelerates model training.
SIMD instructions enable vectorized operations.
Cache locality improves memory access patterns.
Thread synchronization can create bottlenecks.
Lock-free algorithms reduce contention.
The crystalline lattice provides geometric structure.
Prime numbers form the foundation of the system.
Twelve-fold symmetry organizes the architecture.
Kissing spheres enable parallel computation.
Hierarchical structures scale efficiently.
Recursive patterns repeat at every level.
BASETEXT

# Replicate to create 100MB+ file
echo "Replicating text to reach 100MB..."
for i in {1..50000}; do
    cat data/large_dataset/base_text.txt >> data/large_dataset/training_data.txt
    if [ $((i % 10000)) -eq 0 ]; then
        echo "  Progress: $i/50000 iterations"
    fi
done

SIZE=$(du -h data/large_dataset/training_data.txt | cut -f1)
echo "Dataset created: $SIZE"
ls -lh data/large_dataset/training_data.txt
