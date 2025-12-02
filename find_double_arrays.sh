#!/bin/bash
# Find all double* array declarations in headers

echo "=== Double* arrays in training structures ==="
grep -h "double\*" include/cllm_training.h | grep -v "//"

echo ""
echo "=== Double* arrays in model structures ==="
grep -h "double\*" include/cllm.h | grep -v "//"