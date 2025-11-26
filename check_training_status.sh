#!/bin/bash
# Quick training status check

echo "╔══════════════════════════════════════════════════════════╗"
echo "║           CLLM Training Status Check                     ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Check if process is running
if pgrep -f "train_model.*checkpoints_full" > /dev/null; then
    echo "✓ Training process: RUNNING"
    echo "  PIDs: $(pgrep -f 'train_model.*checkpoints_full' | tr '\n' ' ')"
else
    echo "✗ Training process: STOPPED"
fi

echo ""
echo "Latest log entries:"
echo "-------------------"
tail -30 training_logs/full_scale_12threads_*.log 2>/dev/null | tail -15

echo ""
echo "Checking for epoch completion:"
echo "------------------------------"
grep -E "Epoch.*complete|Average loss" training_logs/full_scale_12threads_*.log 2>/dev/null | tail -5 || echo "No epochs completed yet"

echo ""
echo "Checking for errors:"
echo "--------------------"
grep -i "error\|fail\|abort" training_logs/full_scale_12threads_*.log 2>/dev/null | tail -5 || echo "No errors found"

echo ""
echo "Log file size:"
ls -lh training_logs/full_scale_12threads_*.log 2>/dev/null | awk '{print $5, $9}'

echo ""