#!/bin/bash
# Full-scale training on entire src/ directory with 64 threads

set -e

echo "╔══════════════════════════════════════════════════════════╗"
echo "║     CLLM Full-Scale Training - Meaningful Output Test   ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Configuration:"
echo "  Dataset: ./src (all C source files)"
echo "  Threads: 64 (max available)"
echo "  Epochs: 100 (reduced from 500 for time)"
echo "  Model: 6 layers, 8 heads, 256-dim"
echo "  Vocab: 5000 tokens"
echo ""
echo "Expected time: 10-20 minutes"
echo "Expected result: Meaningful code generation"
echo ""
echo "Starting training..."
echo ""

# Clean old checkpoints
rm -rf checkpoints_full
mkdir -p checkpoints_full

# Train with full configuration
LD_LIBRARY_PATH=. timeout 1800 ./tools/train_model ./src \
    --epochs 100 \
    --batch-size 16 \
    --seq-len 64 \
    --learning-rate 0.0005 \
    --threads 64 \
    --vocab-size 5000 \
    --embed-dim 256 \
    --num-layers 6 \
    --num-heads 8 \
    --checkpoint-dir ./checkpoints_full \
    2>&1 | tee training_logs/full_scale_$(date +%Y%m%d_%H%M%S).log

echo ""
echo "Training complete! Checking results..."
echo ""

# Check if model was saved
if [ -f checkpoints_full/final_model.cllm ]; then
    echo "✓ Model saved successfully"
    ls -lh checkpoints_full/final_model.cllm
    
    echo ""
    echo "Testing inference with multiple prompts..."
    echo ""
    
    # Test 1: int main
    echo "=== Test 1: 'int main' ==="
    LD_LIBRARY_PATH=. ./tools/cllm_inference_proper \
        checkpoints_full/final_model.cllm \
        checkpoints_full/vocab.txt \
        -p "int main" \
        -n 40 \
        -t 0.7
    
    echo ""
    echo "=== Test 2: 'struct' ==="
    LD_LIBRARY_PATH=. ./tools/cllm_inference_proper \
        checkpoints_full/final_model.cllm \
        checkpoints_full/vocab.txt \
        -p "struct" \
        -n 40 \
        -t 0.7
    
    echo ""
    echo "=== Test 3: 'void function' ==="
    LD_LIBRARY_PATH=. ./tools/cllm_inference_proper \
        checkpoints_full/final_model.cllm \
        checkpoints_full/vocab.txt \
        -p "void function" \
        -n 40 \
        -t 0.7
    
    echo ""
    echo "=== Loss Analysis ==="
    grep "Epoch.*complete" training_logs/full_scale_*.log | tail -10
    
else
    echo "✗ Model not found - training may have failed"
    exit 1
fi

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║              Full-Scale Training Complete                ║"
echo "╚══════════════════════════════════════════════════════════╝"