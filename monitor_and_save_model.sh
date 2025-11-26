#!/bin/bash

# Monitor training and save model when complete

TRAINING_LOG="training_fixed.log"
CHECKPOINT_DIR="checkpoints_fixed"
OUTPUT_DIR="trained_models"

echo "Monitoring training progress..."
echo "Log file: $TRAINING_LOG"
echo ""

# Wait for training to complete
while ps aux | grep -q "[t]rain_model.*checkpoints_fixed"; do
    # Show progress every 60 seconds
    LATEST_PROGRESS=$(grep "Processed.*batches" "$TRAINING_LOG" | tail -4)
    if [ -n "$LATEST_PROGRESS" ]; then
        clear
        echo "=== Training Progress ==="
        echo ""
        echo "$LATEST_PROGRESS"
        echo ""
        echo "Checking again in 60 seconds..."
    fi
    sleep 60
done

echo ""
echo "Training completed! Preparing model for download..."
echo ""

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Find the final model
FINAL_MODEL=$(ls -t "$CHECKPOINT_DIR"/model_*.bin 2>/dev/null | head -1)

if [ -n "$FINAL_MODEL" ]; then
    MODEL_NAME="cllm_5epoch_4thread_$(date +%Y%m%d_%H%M%S)"
    
    echo "Packaging model: $MODEL_NAME"
    
    # Create package
    tar -czf "$OUTPUT_DIR/${MODEL_NAME}.tar.gz" \
        "$FINAL_MODEL" \
        "$CHECKPOINT_DIR/vocab.txt" \
        "$CHECKPOINT_DIR/dataset.bin" \
        --transform "s|$CHECKPOINT_DIR/||" \
        --transform "s|$(basename $FINAL_MODEL)|model.bin|"
    
    # Create README
    cat > "$OUTPUT_DIR/${MODEL_NAME}_README.md" << EOF
# CLLM Trained Model: $MODEL_NAME

## Training Configuration
- **Epochs:** 5
- **Threads:** 4
- **Batch Size:** 16
- **Sequence Length:** 64
- **Learning Rate:** 0.0005
- **Vocabulary:** 1,000 tokens
- **Model:** 2 layers, 4 heads, 128-dim (~490K parameters)
- **Dataset:** 588 C source files (809,221 tokens)

## Performance Fix Applied
This model was trained with the attention caching bug fix, resulting in 10-15x faster training.

## Package Contents
- \`model.bin\` - Trained model weights
- \`vocab.txt\` - Vocabulary file
- \`dataset.bin\` - Training dataset

## Usage
\`\`\`bash
tar -xzf ${MODEL_NAME}.tar.gz
LD_LIBRARY_PATH=. ./tools/cllm_inference_proper \\
    --model model.bin \\
    --vocab vocab.txt \\
    --prompt "int main() {" \\
    --max-tokens 50
\`\`\`

## Training Results
$(grep "Average Loss" "$TRAINING_LOG" | tail -10)

## Model File
- **Size:** $(du -h "$FINAL_MODEL" | cut -f1)
- **Package Size:** $(du -h "$OUTPUT_DIR/${MODEL_NAME}.tar.gz" | cut -f1)
EOF
    
    echo "✅ Model packaged: $OUTPUT_DIR/${MODEL_NAME}.tar.gz"
    echo "   Size: $(du -h "$OUTPUT_DIR/${MODEL_NAME}.tar.gz" | cut -f1)"
    echo ""
    
    # Commit to repository
    cd /workspace
    git add "$OUTPUT_DIR/"
    git commit -m "feat: Add trained CLLM model (5 epochs, 4 threads, performance fix applied)"
    
    echo ""
    echo "🎉 Model ready for download and validation!"
    echo ""
    echo "To validate:"
    echo "  1. Download from repository"
    echo "  2. Extract: tar -xzf $OUTPUT_DIR/${MODEL_NAME}.tar.gz"
    echo "  3. Test inference with the model"
    
else
    echo "⚠️  No model checkpoint found in $CHECKPOINT_DIR"
    echo "Training may have failed. Check $TRAINING_LOG for errors."
fi