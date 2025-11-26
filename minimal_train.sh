#!/bin/bash
# Minimal training script - train on a single small file

set -e

echo "=== Minimal CLLM Training Test ==="
echo ""

# Create a small test file
mkdir -p test_data
cat > test_data/sample.txt << 'EOF'
int main() {
    printf("Hello, World!\n");
    return 0;
}

void function() {
    int x = 10;
    int y = 20;
    int z = x + y;
}

struct Point {
    int x;
    int y;
};
EOF

echo "Created test data file"
echo ""

# Train with minimal configuration
echo "Starting training..."
LD_LIBRARY_PATH=. ./tools/train_model ./test_data \
    --epochs 2 \
    --batch-size 2 \
    --seq-len 8 \
    --learning-rate 0.01 \
    --threads 1 \
    --vocab-size 500 \
    --embed-dim 32 \
    --num-layers 1 \
    --num-heads 1 \
    --checkpoint-dir ./checkpoints

echo ""
echo "Training complete!"
echo ""

# Check if checkpoint was created
if ls checkpoints/*.cllm 1> /dev/null 2>&1; then
    echo "✓ Model checkpoint found!"
    ls -lh checkpoints/*.cllm
else
    echo "✗ No model checkpoint found"
    exit 1
fi