#!/bin/bash
# Cleanup script to remove all .lock files from crawler directories

echo "Cleaning up lock files..."

# Remove all .lock files from training_queue
find crawler_data/training_queue -name "*.lock" -type f -delete 2>/dev/null
find test_run/training_queue -name "*.lock" -type f -delete 2>/dev/null

# Count remaining .tok files
tok_count=$(find crawler_data/training_queue -name "*.tok" -type f 2>/dev/null | wc -l)
echo "✓ Lock files removed"
echo "  Remaining .tok files: $tok_count"