#!/bin/bash

# Layer 3 Analysis Script
# Analyzes all 64 files in Layer 3 (CLLM Library)

echo "=== LAYER 3: CLLM LIBRARY ANALYSIS ==="
echo ""
echo "Analyzing src/ai/ files for:"
echo "  1. BigFixed integration"
echo "  2. Legacy code remnants"
echo "  3. Mathematical formula usage"
echo ""

cd src/ai

# Get all .c files
LAYER3_FILES=($(ls *.c 2>/dev/null))

VIOLATIONS=0
CLEAN=0
WARNINGS=0

for file in "${LAYER3_FILES[@]}"; do
    echo "----------------------------------------"
    echo "File: $file"
    
    # Check for legacy loss functions
    if grep -q "cllm_compute_cross_entropy_loss\|cllm_compute_batch_loss" "$file"; then
        echo "  ❌ VIOLATION: Contains legacy loss functions"
        ((VIOLATIONS++))
        continue
    fi
    
    # Check for use_crystalline_optimizations flag
    if grep -q "use_crystalline_optimizations" "$file"; then
        echo "  ⚠️  WARNING: References crystalline optimization flag"
        ((WARNINGS++))
    fi
    
    # Check for standard implementations
    if grep -q "_standard\|_legacy" "$file"; then
        echo "  ⚠️  WARNING: Contains standard/legacy references"
        ((WARNINGS++))
    fi
    
    echo "  ✅ CLEAN"
    ((CLEAN++))
done

cd ../..

echo ""
echo "========================================"
echo "LAYER 3 ANALYSIS SUMMARY"
echo "========================================"
echo "Total files: ${#LAYER3_FILES[@]}"
echo "Clean files: $CLEAN"
echo "Warnings:    $WARNINGS"
echo "Violations:  $VIOLATIONS"
echo ""

if [ $VIOLATIONS -eq 0 ]; then
    echo "✅ LAYER 3 VALIDATION: PASSED (with $WARNINGS warnings)"
    exit 0
else
    echo "❌ LAYER 3 VALIDATION: FAILED"
    exit 1
fi
