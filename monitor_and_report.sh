#!/bin/bash
LOG_FILE="training_logs/full_scale_12threads_$(ls -t training_logs/full_scale_12threads_*.log 2>/dev/null | head -1 | xargs basename)"
REPORT_FILE="training_logs/final_training_report.txt"

echo "Starting training monitor..." > $REPORT_FILE
echo "Monitoring log: $LOG_FILE" >> $REPORT_FILE
echo "" >> $REPORT_FILE

while true; do
    sleep 300  # Check every 5 minutes
    
    # Check if training is still running
    if ! pgrep -f "train_model.*checkpoints_full" > /dev/null; then
        echo "Training process stopped at $(date)" >> $REPORT_FILE
        
        # Get final results
        echo "" >> $REPORT_FILE
        echo "=== FINAL TRAINING RESULTS ===" >> $REPORT_FILE
        grep -E "Epoch.*complete|Training Complete|Final loss|Best loss" training_logs/full_scale_12threads_*.log | tail -20 >> $REPORT_FILE
        
        # Check if model was saved
        if [ -f checkpoints_full/final_model.cllm ]; then
            echo "" >> $REPORT_FILE
            echo "✓ Model saved successfully" >> $REPORT_FILE
            ls -lh checkpoints_full/final_model.cllm >> $REPORT_FILE
            
            # Run inference tests
            echo "" >> $REPORT_FILE
            echo "=== INFERENCE TESTS ===" >> $REPORT_FILE
            
            echo "Test 1: 'int main'" >> $REPORT_FILE
            LD_LIBRARY_PATH=. timeout 30 ./tools/cllm_inference_proper checkpoints_full/final_model.cllm checkpoints_full/vocab.txt -p "int main" -n 30 -t 0.7 >> $REPORT_FILE 2>&1
            
            echo "" >> $REPORT_FILE
            echo "Test 2: 'struct'" >> $REPORT_FILE
            LD_LIBRARY_PATH=. timeout 30 ./tools/cllm_inference_proper checkpoints_full/final_model.cllm checkpoints_full/vocab.txt -p "struct" -n 30 -t 0.7 >> $REPORT_FILE 2>&1
        else
            echo "✗ Model not found" >> $REPORT_FILE
        fi
        
        break
    fi
    
    # Log progress
    echo "Progress check at $(date):" >> $REPORT_FILE
    grep -E "Epoch.*complete" training_logs/full_scale_12threads_*.log | tail -3 >> $REPORT_FILE
    echo "" >> $REPORT_FILE
done

echo "Monitoring complete at $(date)" >> $REPORT_FILE
