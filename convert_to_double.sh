#!/bin/bash

# This script systematically converts float to double in the CLLM codebase
# It must be run carefully and tested after each phase

echo "=== CLLM Float to Double Conversion ==="
echo ""
echo "This will convert the entire codebase from float to double precision"
echo "Press Ctrl+C to cancel, or Enter to continue..."
read

# Backup critical files
echo "Creating backups..."
cp src/ai/cllm_inference.c src/ai/cllm_inference.c.backup
cp src/ai/cllm_feedforward.c src/ai/cllm_feedforward.c.backup
cp include/cllm_inference.h include/cllm_inference.h.backup

echo "Backups created"
echo ""

echo "Phase 1: Converting cllm_inference.c layer operations..."
echo "This requires manual conversion due to complexity"
echo "Please review FLOAT_TO_DOUBLE_CONVERSION_PLAN.md"