#!/bin/bash

# Comprehensive Training Monitor
# Tracks progress and prepares models for download

LOG_FILE="training_logs/training_64threads_20251126_135220.log"
STATUS_FILE="TRAINING_STATUS_LIVE.md"
CHECKPOINT_DIR="checkpoints_full"

echo "Starting comprehensive training monitor..."
echo "Monitoring log: $LOG_FILE"
echo ""

while true; do
    # Check if process is still running
    if ! ps -p 3533 > /dev/null 2>&1; then
        echo "Training process completed!"
        
        # Generate final report
        cat > TRAINING_COMPLETE_REPORT.md << 'EOF'
# Training Complete Report

## Training Configuration
- **Epochs:** 100
- **Threads:** 64
- **Batch Size:** 16
- **Sequence Length:** 64
- **Learning Rate:** 0.0005
- **Dataset:** 588 C source files (604,807 tokens)
- **Vocabulary:** 5,000 tokens
- **Model Parameters:** ~2.5M

## Final Results

EOF
        
        # Extract final loss values
        echo "### Loss Progression" >> TRAINING_COMPLETE_REPORT.md
        echo "" >> TRAINING_COMPLETE_REPORT.md
        grep "Average Loss" "$LOG_FILE" | tail -20 >> TRAINING_COMPLETE_REPORT.md
        
        # List saved checkpoints
        echo "" >> TRAINING_COMPLETE_REPORT.md
        echo "### Saved Checkpoints" >> TRAINING_COMPLETE_REPORT.md
        echo "" >> TRAINING_COMPLETE_REPORT.md
        echo "\`\`\`" >> TRAINING_COMPLETE_REPORT.md
        ls -lh "$CHECKPOINT_DIR"/*.bin 2>/dev/null >> TRAINING_COMPLETE_REPORT.md
        echo "\`\`\`" >> TRAINING_COMPLETE_REPORT.md
        
        # Package model for download
        echo "" >> TRAINING_COMPLETE_REPORT.md
        echo "### Model Package" >> TRAINING_COMPLETE_REPORT.md
        echo "" >> TRAINING_COMPLETE_REPORT.md
        
        if [ -f "$CHECKPOINT_DIR/model_final.bin" ]; then
            tar -czf cllm_trained_model_64threads.tar.gz \
                "$CHECKPOINT_DIR/model_final.bin" \
                "$CHECKPOINT_DIR/vocab.txt" \
                "$CHECKPOINT_DIR/dataset.bin" \
                TRAINING_COMPLETE_REPORT.md
            
            echo "✅ Model packaged: \`cllm_trained_model_64threads.tar.gz\`" >> TRAINING_COMPLETE_REPORT.md
            echo "Size: $(du -h cllm_trained_model_64threads.tar.gz | cut -f1)" >> TRAINING_COMPLETE_REPORT.md
        else
            echo "⚠️ Final model not found. Checking for latest checkpoint..." >> TRAINING_COMPLETE_REPORT.md
            LATEST=$(ls -t "$CHECKPOINT_DIR"/model_epoch_*.bin 2>/dev/null | head -1)
            if [ -n "$LATEST" ]; then
                tar -czf cllm_trained_model_64threads.tar.gz \
                    "$LATEST" \
                    "$CHECKPOINT_DIR/vocab.txt" \
                    "$CHECKPOINT_DIR/dataset.bin" \
                    TRAINING_COMPLETE_REPORT.md
                
                echo "✅ Model packaged: \`cllm_trained_model_64threads.tar.gz\`" >> TRAINING_COMPLETE_REPORT.md
                echo "Latest checkpoint: $(basename $LATEST)" >> TRAINING_COMPLETE_REPORT.md
                echo "Size: $(du -h cllm_trained_model_64threads.tar.gz | cut -f1)" >> TRAINING_COMPLETE_REPORT.md
            fi
        fi
        
        echo "" >> TRAINING_COMPLETE_REPORT.md
        echo "## Download Instructions" >> TRAINING_COMPLETE_REPORT.md
        echo "" >> TRAINING_COMPLETE_REPORT.md
        echo "The trained model is ready for download in the repository." >> TRAINING_COMPLETE_REPORT.md
        
        break
    fi
    
    # Update live status
    CURRENT_TIME=$(date '+%Y-%m-%d %H:%M:%S UTC')
    ELAPSED=$(($(date +%s) - 1764165140))
    ELAPSED_MIN=$((ELAPSED / 60))
    
    cat > "$STATUS_FILE" << EOF
# Live Training Status

**Last Updated:** $CURRENT_TIME

## Process Information
- **PID:** 3533
- **Status:** RUNNING ✅
- **Elapsed Time:** ${ELAPSED_MIN} minutes
- **CPU Usage:** $(ps -p 3533 -o %cpu= 2>/dev/null || echo "N/A")%
- **Memory Usage:** $(ps -p 3533 -o rss= 2>/dev/null | awk '{printf "%.1f GB", $1/1024/1024}')

## Training Progress

### Latest Log Entries
\`\`\`
$(tail -30 "$LOG_FILE" 2>/dev/null)
\`\`\`

### Epoch Progress
$(grep -E "Epoch [0-9]+/100" "$LOG_FILE" 2>/dev/null | tail -5)

### Loss Values
$(grep "Average Loss" "$LOG_FILE" 2>/dev/null | tail -10)

## Checkpoints
\`\`\`
$(ls -lh "$CHECKPOINT_DIR"/*.bin 2>/dev/null || echo "No checkpoints yet")
\`\`\`

---
*Monitoring every 2 minutes. Training expected to complete in ~25-35 hours.*
EOF
    
    # Sleep for 2 minutes
    sleep 120
done

echo "Monitor completed. Final report generated."