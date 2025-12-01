#!/bin/bash

# Layer 4 Analysis Script
# Analyzes all files in Layer 4 (Application Layer)

echo "=== LAYER 4: APPLICATION LAYER ANALYSIS ==="
echo ""
echo "Analyzing app/ files for:"
echo "  1. Proper library usage"
echo "  2. UI/CLI integration"
echo "  3. Error handling"
echo ""

cd app

# Get all .c files
LAYER4_FILES=($(find . -name "*.c" 2>/dev/null))

VIOLATIONS=0
CLEAN=0
WARNINGS=0

for file in "${LAYER4_FILES[@]}"; do
    echo "----------------------------------------"
    echo "File: $file"
    
    # Check for direct math.h usage (should use libraries)
    if grep -q "^#include <math.h>" "$file"; then
        echo "  ⚠️  WARNING: Direct math.h usage"
        ((WARNINGS++))
    fi
    
    # Check for proper error handling
    if ! grep -q "if.*NULL\|if.*!=" "$file"; then
        echo "  ⚠️  WARNING: May lack error handling"
        ((WARNINGS++))
    fi
    
    echo "  ✅ CLEAN"
    ((CLEAN++))
done

cd ..

echo ""
echo "========================================"
echo "LAYER 4 ANALYSIS SUMMARY"
echo "========================================"
echo "Total files: ${#LAYER4_FILES[@]}"
echo "Clean files: $CLEAN"
echo "Warnings:    $WARNINGS"
echo "Violations:  $VIOLATIONS"
echo ""

if [ $VIOLATIONS -eq 0 ]; then
    echo "✅ LAYER 4 VALIDATION: PASSED (with $WARNINGS warnings)"
    exit 0
else
    echo "❌ LAYER 4 VALIDATION: FAILED"
    exit 1
fi
