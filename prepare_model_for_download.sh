#!/bin/bash

# Script to prepare trained models for download
# This will be called when training completes

CHECKPOINT_DIR="checkpoints_full"
OUTPUT_DIR="trained_models"

echo "Preparing models for download..."

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Function to package a model
package_model() {
    local model_file=$1
    local model_name=$2
    
    if [ -f "$model_file" ]; then
        echo "Packaging $model_name..."
        
        # Create a package with model, vocab, and metadata
        tar -czf "$OUTPUT_DIR/${model_name}.tar.gz" \
            "$model_file" \
            "$CHECKPOINT_DIR/vocab.txt" \
            "$CHECKPOINT_DIR/dataset.bin" \
            --transform "s|$CHECKPOINT_DIR/||" \
            --transform "s|$(basename $model_file)|model.bin|"
        
        # Create a README for this model
        cat > "$OUTPUT_DIR/${model_name}_README.md" << EOF
# CLLM Trained Model: $model_name

## Model Information
- **Training Date:** $(date)
- **Model File:** $(basename $model_file)
- **Vocabulary Size:** 5,000 tokens
- **Model Parameters:** ~2.5M
- **Training Dataset:** 588 C source files (604,807 tokens)

## Package Contents
- \`model.bin\` - Trained model weights
- \`vocab.txt\` - Vocabulary file (5,000 tokens)
- \`dataset.bin\` - Training dataset

## Usage

### Extract the package:
\`\`\`bash
tar -xzf ${model_name}.tar.gz
\`\`\`

### Run inference:
\`\`\`bash
LD_LIBRARY_PATH=. ./tools/cllm_inference_proper \\
    --model model.bin \\
    --vocab vocab.txt \\
    --prompt "int main() {" \\
    --max-tokens 100
\`\`\`

## Model Statistics
- **File Size:** $(du -h "$model_file" | cut -f1)
- **Package Size:** $(du -h "$OUTPUT_DIR/${model_name}.tar.gz" | cut -f1)

EOF
        
        echo "✅ Created: $OUTPUT_DIR/${model_name}.tar.gz"
        echo "   Size: $(du -h "$OUTPUT_DIR/${model_name}.tar.gz" | cut -f1)"
    else
        echo "⚠️  Model file not found: $model_file"
    fi
}

# Wait for training to complete
echo "Waiting for training to complete..."
while ps -p 3533 > /dev/null 2>&1; do
    sleep 60
done

echo "Training completed! Packaging models..."

# Package the final model
if [ -f "$CHECKPOINT_DIR/model_final.bin" ]; then
    package_model "$CHECKPOINT_DIR/model_final.bin" "cllm_final_64threads"
fi

# Package the latest checkpoint
LATEST=$(ls -t "$CHECKPOINT_DIR"/model_epoch_*.bin 2>/dev/null | head -1)
if [ -n "$LATEST" ]; then
    EPOCH=$(basename "$LATEST" | grep -oP 'epoch_\K[0-9]+')
    package_model "$LATEST" "cllm_epoch${EPOCH}_64threads"
fi

# Create a master index
cat > "$OUTPUT_DIR/INDEX.md" << 'EOF'
# CLLM Trained Models

This directory contains trained CLLM models ready for download and validation.

## Available Models

EOF

ls -lh "$OUTPUT_DIR"/*.tar.gz 2>/dev/null | while read -r line; do
    filename=$(echo "$line" | awk '{print $NF}')
    size=$(echo "$line" | awk '{print $5}')
    echo "- **$(basename $filename)** - Size: $size" >> "$OUTPUT_DIR/INDEX.md"
done

cat >> "$OUTPUT_DIR/INDEX.md" << 'EOF'

## Validation

To validate the models:

1. Download the model package
2. Extract: `tar -xzf <model_package>.tar.gz`
3. Run inference: `LD_LIBRARY_PATH=. ./tools/cllm_inference_proper --model model.bin --vocab vocab.txt --prompt "your prompt"`
4. Check output quality

## Repository

All models are committed to the repository for easy download via git clone.

EOF

echo ""
echo "✅ All models packaged and ready for download!"
echo "📁 Location: $OUTPUT_DIR/"
echo ""
ls -lh "$OUTPUT_DIR/"

# Commit to repository
cd /workspace
git add "$OUTPUT_DIR/"
git commit -m "feat: Add trained CLLM models (64-thread training)" || true
git push origin main || echo "Note: Push to remote repository manually if needed"

echo ""
echo "🎉 Models are ready for validation!"