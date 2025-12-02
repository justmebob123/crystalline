#!/bin/bash
# Debug script to identify where application hangs

echo "=== Debugging Application Hang ==="
echo ""

# Check if data files exist
echo "1. Checking for training data files..."
find . -name "*.txt" -type f | head -10

echo ""
echo "2. Checking cllm_load_training_data function..."
grep -A 30 "int cllm_load_training_data" src/ai/cllm_training.c | head -40

echo ""
echo "3. Checking threaded_training_create function..."
grep -n "threaded_training_create" src/ai/cllm_training_threaded.c | head -5

echo ""
echo "4. Looking for potential blocking calls..."
grep -n "pthread_create\|pthread_join\|pthread_barrier_wait" src/ai/cllm_training_threaded.c | head -20

