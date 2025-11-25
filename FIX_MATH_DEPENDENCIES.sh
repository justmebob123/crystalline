#!/bin/bash

# Fix Math Dependencies Script
# Replaces math.h functions with custom prime_math implementations

set -e

echo "=========================================="
echo "Fix Math Dependencies Script"
echo "=========================================="
echo ""

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_status() {
    echo -e "${GREEN}✓${NC} $1"
}

print_info() {
    echo -e "${YELLOW}ℹ${NC} $1"
}

# Files to fix
FILES=(
    "src/core/cllm_thread_allocation.c"
    "src/core/cllm_sphere_position.c"
    "src/core/cllm_mathematical_constants.c"
    "src/core/cllm_sphere_stats.c"
    "src/core/cllm_angular_position.c"
)

echo "Fixing math.h dependencies in ${#FILES[@]} files..."
echo ""

for file in "${FILES[@]}"; do
    if [ ! -f "$file" ]; then
        print_info "Skipping $file (not found)"
        continue
    fi
    
    echo "Processing: $file"
    
    # Create backup
    cp "$file" "${file}.bak"
    
    # Replace math.h with prime_math_custom.h
    sed -i 's/#include <math\.h>/#include "prime_math_custom.h"/' "$file"
    
    # Replace math functions with prime_math equivalents
    sed -i 's/\bsqrt(/prime_sqrt(/g' "$file"
    sed -i 's/\blog(/prime_log(/g' "$file"
    sed -i 's/\blog2(/prime_log2(/g' "$file"
    sed -i 's/\blog10(/prime_log10(/g' "$file"
    sed -i 's/\bexp(/prime_exp(/g' "$file"
    sed -i 's/\bsin(/prime_sin(/g' "$file"
    sed -i 's/\bcos(/prime_cos(/g' "$file"
    sed -i 's/\btan(/prime_tan(/g' "$file"
    sed -i 's/\bfabs(/prime_fabs(/g' "$file"
    sed -i 's/\bpow(/prime_pow(/g' "$file"
    sed -i 's/\bfloor(/prime_floor(/g' "$file"
    sed -i 's/\bceil(/prime_ceil(/g' "$file"
    sed -i 's/\bround(/prime_round(/g' "$file"
    
    # Replace M_PI with custom constant
    sed -i 's/M_PI/PRIME_PI/g' "$file"
    
    print_status "Fixed $file"
done

echo ""
print_status "All files processed"
echo ""

# Test build
echo "Testing build..."
if make clean > /dev/null 2>&1 && make all > /tmp/fix_build.log 2>&1; then
    print_status "Build successful!"
    echo ""
    
    # Remove backups
    for file in "${FILES[@]}"; do
        if [ -f "${file}.bak" ]; then
            rm "${file}.bak"
        fi
    done
    print_status "Backups removed"
else
    echo ""
    echo "Build failed! Restoring backups..."
    for file in "${FILES[@]}"; do
        if [ -f "${file}.bak" ]; then
            mv "${file}.bak" "$file"
        fi
    done
    echo ""
    echo "Last 30 lines of build log:"
    tail -30 /tmp/fix_build.log
    exit 1
fi

echo ""
print_status "Math dependencies fixed successfully!"