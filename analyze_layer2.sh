#!/bin/bash

# Layer 2 Analysis Script
# Analyzes all 14 files in Layer 2 (Algorithms Library)

echo "=== LAYER 2: ALGORITHMS LIBRARY ANALYSIS ==="
echo ""
echo "Analyzing 14 files for:"
echo "  1. math.h usage (should be commented out)"
echo "  2. CLLM-specific code (should be general)"
echo "  3. Proper crystalline math usage"
echo ""

cd algorithms

LAYER2_FILES=(
    "src/numerical.c"
    "src/loss_functions.c"
    "src/optimizers.c"
    "src/backprop.c"
    "src/statistics.c"
    "src/threading.c"
    "src/shared_memory.c"
    "src/lock_free_queue.c"
    "src/sphere_packing.c"
    "src/hierarchical_primes.c"
    "src/hierarchical_structures.c"
    "src/batch_processing.c"
    "src/hierarchical_prime_partitions.c"
    "src/lattice_sphere_positions.c"
)

VIOLATIONS=0
CLEAN=0

for file in "${LAYER2_FILES[@]}"; do
    echo "----------------------------------------"
    echo "File: $file"
    
    # Check if file exists
    if [ ! -f "$file" ]; then
        echo "  ❌ FILE NOT FOUND"
        ((VIOLATIONS++))
        continue
    fi
    
    # Check for active math.h (not commented)
    if grep -q "^#include <math.h>" "$file"; then
        echo "  ❌ VIOLATION: Uses math.h (not commented)"
        ((VIOLATIONS++))
        continue
    fi
    
    # Check for CLLM-specific code (should be general algorithms)
    if grep -q "CLLMModel\|CLLMTraining\|CLLMConfig" "$file"; then
        echo "  ⚠️  WARNING: Contains CLLM-specific types"
        grep -n "CLLMModel\|CLLMTraining\|CLLMConfig" "$file" | head -3
        # Don't count as violation, just warning
    fi
    
    # Check for standard math function CALLS
    if grep -v "^[[:space:]]*\(//\|/\*\|\*\)" "$file" | grep -q "\<\(sqrt\|pow\|exp\|log\|sin\|cos\|tan\|fabs\)(" | grep -v "prime_"; then
        echo "  ❌ VIOLATION: Uses standard math functions"
        grep -n "\<\(sqrt\|pow\|exp\|log\|sin\|cos\|tan\|fabs\)(" "$file" | grep -v "prime_" | head -3
        ((VIOLATIONS++))
        continue
    fi
    
    echo "  ✅ CLEAN"
    ((CLEAN++))
done

cd ..

echo ""
echo "========================================"
echo "LAYER 2 ANALYSIS SUMMARY"
echo "========================================"
echo "Total files: ${#LAYER2_FILES[@]}"
echo "Clean files: $CLEAN"
echo "Violations:  $VIOLATIONS"
echo ""

if [ $VIOLATIONS -eq 0 ]; then
    echo "✅ LAYER 2 VALIDATION: PASSED"
    exit 0
else
    echo "❌ LAYER 2 VALIDATION: FAILED"
    exit 1
fi