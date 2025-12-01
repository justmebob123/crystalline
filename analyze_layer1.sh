#!/bin/bash

# Layer 1 Analysis Script
# Analyzes all 23 files in Layer 1 (Crystalline Library)

echo "=== LAYER 1: CRYSTALLINE LIBRARY ANALYSIS ==="
echo ""
echo "Analyzing 23 files for:"
echo "  1. math.h usage"
echo "  2. Threading primitives (actual usage, not comments)"
echo "  3. Atomic operations"
echo "  4. Standard math function calls"
echo ""

LAYER1_FILES=(
    "src/core/prime_lowlevel.c"
    "src/core/bigint_core.c"
    "src/core/bigint_conversions.c"
    "src/core/bigint_ntt.c"
    "src/core/bigfixed_core.c"
    "src/core/bigfixed_constants.c"
    "src/core/cllm_angular_position.c"
    "src/core/cllm_mathematical_constants.c"
    "src/core/crystal_abacus.c"
    "src/transcendental/prime_basic.c"
    "src/transcendental/prime_bigint_transcendental.c"
    "src/transcendental/prime_float_math.c"
    "src/transcendental/prime_math.c"
    "src/transcendental/prime_math_custom.c"
    "src/geometry/clock_lattice.c"
    "src/geometry/lattice_algorithms.c"
    "src/geometry/prime_coords.c"
    "src/geometry/prime_hyperdim.c"
    "src/geometry/prime_lattice.c"
    "src/geometry/prime_lattice_core.c"
    "src/geometry/prime_lattice_geometry.c"
    "src/geometry/prime_matrix.c"
    "src/geometry/prime_rainbow.c"
)

VIOLATIONS=0
CLEAN=0

for file in "${LAYER1_FILES[@]}"; do
    echo "----------------------------------------"
    echo "File: $file"
    
    # Check if file exists
    if [ ! -f "$file" ]; then
        echo "  ❌ FILE NOT FOUND"
        ((VIOLATIONS++))
        continue
    fi
    
    # Check for math.h
    if grep -q "^#include <math.h>" "$file"; then
        echo "  ❌ VIOLATION: Uses math.h"
        ((VIOLATIONS++))
        continue
    fi
    
    # Check for pthread (actual includes, not comments)
    if grep -q "^#include <pthread.h>" "$file"; then
        echo "  ❌ VIOLATION: Uses pthread.h"
        ((VIOLATIONS++))
        continue
    fi
    
    # Check for atomic operations (actual includes)
    if grep -q "^#include.*atomic" "$file"; then
        echo "  ❌ VIOLATION: Uses atomics"
        ((VIOLATIONS++))
        continue
    fi
    
    # Check for standard math function CALLS (not in comments or function names)
    # Look for actual function calls like sqrt(, pow(, etc.
    if grep -v "^[[:space:]]*\(//\|/\*\|\*\)" "$file" | grep -q "\<\(sqrt\|pow\|exp\|log\|sin\|cos\|tan\|fabs\|ceil\|floor\)(" | grep -v "prime_"; then
        echo "  ⚠️  WARNING: May use standard math functions"
        # Show the lines
        grep -n "\<\(sqrt\|pow\|exp\|log\|sin\|cos\|tan\|fabs\|ceil\|floor\)(" "$file" | grep -v "prime_" | grep -v "^[[:space:]]*\(//\|/\*\|\*\)" | head -3
        ((VIOLATIONS++))
        continue
    fi
    
    echo "  ✅ CLEAN"
    ((CLEAN++))
done

echo ""
echo "========================================"
echo "LAYER 1 ANALYSIS SUMMARY"
echo "========================================"
echo "Total files: ${#LAYER1_FILES[@]}"
echo "Clean files: $CLEAN"
echo "Violations:  $VIOLATIONS"
echo ""

if [ $VIOLATIONS -eq 0 ]; then
    echo "✅ LAYER 1 VALIDATION: PASSED"
    exit 0
else
    echo "❌ LAYER 1 VALIDATION: FAILED"
    exit 1
fi