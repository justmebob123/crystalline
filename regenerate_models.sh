#!/bin/bash

echo "=========================================="
echo "Regenerating Models with Correct Magic Number"
echo "=========================================="
echo ""

# Backup old models
echo "Backing up old models..."
mkdir -p old_models_backup
for model in test_model.cllm test_model_simple.cllm test_model_medium.cllm; do
    if [ -f "$model" ]; then
        mv "$model" "old_models_backup/$model.old"
        echo "  Backed up: $model"
    fi
done
echo ""

# Create new models with correct magic number
echo "Creating new models..."
echo ""

echo "1. Creating test_model_simple.cllm (small model)..."
./tools/cllm create -v 1000 -e 128 -l 2 -H 4 -o test_model_simple.cllm
echo ""

echo "2. Creating test_model_medium.cllm (medium model)..."
./tools/cllm create -v 5000 -e 256 -l 4 -H 8 -o test_model_medium.cllm
echo ""

echo "3. Creating test_model.cllm (default model)..."
./tools/cllm create -v 10000 -e 512 -l 6 -H 8 -o test_model.cllm
echo ""

echo "=========================================="
echo "Verifying Magic Numbers"
echo "=========================================="
echo ""
echo "Expected magic: 43 4C 4C 4D 01 00 00 00 (CLLM with version 1)"
echo ""

for model in test_model_simple.cllm test_model_medium.cllm test_model.cllm; do
    if [ -f "$model" ]; then
        echo -n "$model: "
        od -A n -t x1 -N 8 "$model" | head -1
    else
        echo "$model: NOT FOUND"
    fi
done

echo ""
echo "=========================================="
echo "Done!"
echo "=========================================="
echo ""
echo "Your models have been regenerated with the correct magic number."
echo "They should now load successfully in the application."
echo ""
echo "Old models backed up to: old_models_backup/"
