#!/bin/bash

# Comprehensive fix - Replace ALL OLD header includes with NEW library

echo "=== COMPREHENSIVE OLD HEADER REPLACEMENT ==="
echo ""

# Replace prime_float_math.h includes
echo "Replacing prime_float_math.h includes..."
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/.git/*" ! -path "*/math/*" -exec sed -i '
    s|#include "prime_float_math.h"|#include "math/arithmetic.h"\n#include "math/transcendental.h"|g
    s|#include "algorithms/include/prime_float_math.h"|#include "math/arithmetic.h"\n#include "math/transcendental.h"|g
    s|#include "../include/prime_float_math.h"|#include "math/arithmetic.h"\n#include "math/transcendental.h"|g
' {} +

echo "✓ Replaced prime_float_math.h includes"

# Now delete the OLD header files entirely
echo ""
echo "Deleting OLD library header files..."
rm -f include/prime_float_math.h
rm -f algorithms/include/prime_float_math.h
echo "✓ Deleted prime_float_math.h files"

echo ""
echo "=== COMPREHENSIVE FIX COMPLETE ==="