#!/bin/bash

# Delete all corrupted model files
echo "Removing corrupted model files..."
rm -f models/*.cllm
rm -f data/models/*.cllm
rm -f test_models/*.cllm

# Create models directory if it doesn't exist
mkdir -p models

echo "✓ Corrupted models removed"
echo "✓ Application will create fresh model with random weights on next training"
