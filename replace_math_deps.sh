#!/bin/bash

# Script to replace math.h dependencies with prime_float_math.h
# Ensures complete mathematical independence

echo "=== Mathematical Independence Replacement Script ==="
echo ""

# List of files to process
FILES=(
    "src/ai/cllm_backward.c"
    "src/ai/cllm_create.c"
    "src/ai/cllm_crystalline_advanced.c"
    "src/ai/cllm_crystalline_attention.c"
    "src/ai/cllm_crystalline_training.c"
    "src/ai/cllm_fp16.c"
    "src/ai/cllm_inference.c"
    "src/ai/cllm_lattice_conversion.c"
    "src/ai/cllm_lll_embeddings.c"
    "src/ai/cllm_production.c"
    "src/ai/cllm_root_word_modeling.c"
    "src/ai/cllm_train_complete.c"
    "src/ai/cllm_training.c"
)

# Backup directory
BACKUP_DIR="math_h_backups_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

echo "Creating backups in $BACKUP_DIR..."
for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        cp "$file" "$BACKUP_DIR/"
        echo "  ✓ Backed up: $file"
    fi
done
echo ""

echo "Replacing math.h includes..."
for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        # Replace #include <math.h> with prime_float_math.h
        sed -i 's/#include <math\.h>/#include "..\/include\/prime_float_math.h"/g' "$file"
        echo "  ✓ Updated include: $file"
    fi
done
echo ""

echo "Replacing math function calls..."
for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        # Float functions
        sed -i 's/\bsqrtf(/prime_sqrtf(/g' "$file"
        sed -i 's/\bexpf(/prime_expf(/g' "$file"
        sed -i 's/\blogf(/prime_logf(/g' "$file"
        sed -i 's/\bpowf(/prime_powf(/g' "$file"
        sed -i 's/\bsinf(/prime_sinf(/g' "$file"
        sed -i 's/\bcosf(/prime_cosf(/g' "$file"
        sed -i 's/\btanf(/prime_tanf(/g' "$file"
        sed -i 's/\btanhf(/prime_tanhf(/g' "$file"
        sed -i 's/\bfabsf(/prime_fabsf(/g' "$file"
        sed -i 's/\bfmaxf(/prime_fmaxf(/g' "$file"
        sed -i 's/\bfminf(/prime_fminf(/g' "$file"
        sed -i 's/\batanf(/prime_atanf(/g' "$file"
        sed -i 's/\batan2f(/prime_atan2f(/g' "$file"
        
        # Double functions
        sed -i 's/\bsqrt(/prime_sqrt(/g' "$file"
        sed -i 's/\bexp(/prime_exp(/g' "$file"
        sed -i 's/\blog(/prime_log(/g' "$file"
        sed -i 's/\bpow(/prime_pow(/g' "$file"
        sed -i 's/\bsin(/prime_sin(/g' "$file"
        sed -i 's/\bcos(/prime_cos(/g' "$file"
        sed -i 's/\btan(/prime_tan(/g' "$file"
        sed -i 's/\btanh(/prime_tanh(/g' "$file"
        sed -i 's/\bfabs(/prime_fabs(/g' "$file"
        sed -i 's/\bfmax(/prime_fmax(/g' "$file"
        sed -i 's/\bfmin(/prime_fmin(/g' "$file"
        sed -i 's/\batan(/prime_atan(/g' "$file"
        sed -i 's/\batan2(/prime_atan2(/g' "$file"
        sed -i 's/\bfloor(/prime_floor(/g' "$file"
        sed -i 's/\bceil(/prime_ceil(/g' "$file"
        sed -i 's/\bround(/prime_round(/g' "$file"
        sed -i 's/\bfmod(/prime_fmod(/g' "$file"
        sed -i 's/\bacos(/prime_acos(/g' "$file"
        sed -i 's/\blog2(/prime_log2(/g' "$file"
        
        # Special functions
        sed -i 's/\bisnan(/prime_isnanf(/g' "$file"
        sed -i 's/\bisinf(/prime_isinff(/g' "$file"
        
        echo "  ✓ Replaced functions: $file"
    fi
done
echo ""

echo "Verifying changes..."
MATH_H_COUNT=$(grep -c "#include <math\.h>" "${FILES[@]}" 2>/dev/null || echo "0")
PRIME_MATH_COUNT=$(grep -c "prime_float_math\.h" "${FILES[@]}" 2>/dev/null || echo "0")

echo "  math.h includes remaining: $MATH_H_COUNT"
echo "  prime_float_math.h includes: $PRIME_MATH_COUNT"
echo ""

if [ "$MATH_H_COUNT" -eq 0 ]; then
    echo "✓ SUCCESS: All math.h dependencies removed!"
else
    echo "⚠ WARNING: Some math.h includes still remain"
fi

echo ""
echo "Backups saved in: $BACKUP_DIR"
echo "To restore backups: cp $BACKUP_DIR/* src/ai/"
echo ""
echo "Next steps:"
echo "  1. Review changes: git diff"
echo "  2. Build: make clean && make"
echo "  3. Test functionality"
echo "  4. Commit if successful"