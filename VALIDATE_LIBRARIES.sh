#!/bin/bash

# Library Validation Script
# Validates mathematical purity and library boundaries

echo "=========================================="
echo "Library Validation Script"
echo "=========================================="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_pass() {
    echo -e "${GREEN}✓ PASS${NC} $1"
}

print_fail() {
    echo -e "${RED}✗ FAIL${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}⚠ WARN${NC} $1"
}

PASS_COUNT=0
FAIL_COUNT=0
WARN_COUNT=0

# Test 1: Check for math.h in crystalline library sources
echo "Test 1: Checking for math.h usage in crystalline library..."
if grep -r "#include <math.h>" src/core/ src/transcendental/ src/geometry/ 2>/dev/null || false; then
    print_fail "Found math.h includes in crystalline library"
    ((FAIL_COUNT++))
else
    print_pass "No math.h includes in crystalline library"
    ((PASS_COUNT++))
fi
echo ""

# Test 2: Check for GMP usage
echo "Test 2: Checking for GMP usage..."
if grep -r "gmp.h\|mpz_\|mpf_\|mpq_" src/ include/ 2>/dev/null || false; then
    print_fail "Found GMP usage"
    ((FAIL_COUNT++))
else
    print_pass "No GMP usage found"
    ((PASS_COUNT++))
fi
echo ""

# Test 3: Check libcrystalline.so dependencies
echo "Test 3: Checking libcrystalline.so dependencies..."
if [ -f "libcrystalline.so" ]; then
    if ldd libcrystalline.so | grep -q "libm.so"; then
        print_fail "libcrystalline.so depends on libm.so"
        ((FAIL_COUNT++))
    else
        print_pass "libcrystalline.so has no math library dependency"
        ((PASS_COUNT++))
    fi
    
    if ldd libcrystalline.so | grep -q "libgmp"; then
        print_fail "libcrystalline.so depends on GMP"
        ((FAIL_COUNT++))
    else
        print_pass "libcrystalline.so has no GMP dependency"
        ((PASS_COUNT++))
    fi
else
    print_warn "libcrystalline.so not found"
    ((WARN_COUNT++))
fi
echo ""

# Test 4: Check libcllm.so dependencies
echo "Test 4: Checking libcllm.so dependencies..."
if [ -f "libcllm.so" ]; then
    if ldd libcllm.so | grep -q "libcrystalline.so"; then
        print_pass "libcllm.so depends on libcrystalline.so"
        ((PASS_COUNT++))
    else
        print_fail "libcllm.so does not depend on libcrystalline.so"
        ((FAIL_COUNT++))
    fi
else
    print_warn "libcllm.so not found"
    ((WARN_COUNT++))
fi
echo ""

# Test 5: Check for infrastructure components in src/core/
echo "Test 5: Checking for infrastructure components in src/core/..."
INFRA_IN_CORE=0
for comp in cllm_thread_allocation cllm_shared_memory cllm_sphere_stats cllm_sphere_message cllm_message_queue cllm_lattice_hierarchy; do
    if [ -f "src/core/${comp}.c" ]; then
        print_warn "Found ${comp}.c in src/core/ (should be in src/ai/)"
        ((INFRA_IN_CORE++))
    fi
done

if [ $INFRA_IN_CORE -eq 0 ]; then
    print_pass "No infrastructure components in src/core/"
    ((PASS_COUNT++))
else
    print_fail "$INFRA_IN_CORE infrastructure components still in src/core/"
    ((FAIL_COUNT++))
fi
echo ""

# Test 6: Check for mathematical components in src/core/
echo "Test 6: Checking for mathematical components in src/core/..."
MATH_COMPONENTS=(
    "cllm_sphere_position"
    "cllm_hierarchical_abacus"
    "cllm_mathematical_constants"
    "cllm_angular_position"
)

MATH_FOUND=0
for comp in "${MATH_COMPONENTS[@]}"; do
    if [ -f "src/core/${comp}.c" ]; then
        ((MATH_FOUND++))
    fi
done

if [ $MATH_FOUND -eq 4 ]; then
    print_pass "All mathematical components in src/core/"
    ((PASS_COUNT++))
else
    print_fail "Only $MATH_FOUND/4 mathematical components in src/core/"
    ((FAIL_COUNT++))
fi
echo ""

# Test 7: Check for proper include paths
echo "Test 7: Checking include paths..."
if [ -d "src/ai/infrastructure" ]; then
    # Check if infrastructure files use ai/ prefix
    WRONG_INCLUDES=0
    for file in src/ai/infrastructure/*.c; do
        if [ -f "$file" ]; then
            if grep -q '#include "cllm_thread_allocation.h"' "$file" || \
               grep -q '#include "cllm_shared_memory.h"' "$file" || \
               grep -q '#include "cllm_sphere_stats.h"' "$file" || \
               grep -q '#include "cllm_sphere_message.h"' "$file" || \
               grep -q '#include "cllm_message_queue.h"' "$file" || \
               grep -q '#include "cllm_lattice_hierarchy.h"' "$file"; then
                print_warn "$(basename $file) has incorrect include paths"
                ((WRONG_INCLUDES++))
            fi
        fi
    done
    
    if [ $WRONG_INCLUDES -eq 0 ]; then
        print_pass "All include paths use ai/ prefix"
        ((PASS_COUNT++))
    else
        print_fail "$WRONG_INCLUDES files have incorrect include paths"
        ((FAIL_COUNT++))
    fi
else
    print_warn "src/ai/infrastructure/ directory not found"
    ((WARN_COUNT++))
fi
echo ""

# Test 8: Check for circular dependencies
echo "Test 8: Checking for circular dependencies..."
# This is a simplified check - a full check would require dependency graph analysis
# Exclude self-includes (e.g., cllm_hierarchical_abacus.h in cllm_hierarchical_abacus.c)
if grep -r '#include "cllm\.h"' src/core/ 2>/dev/null | grep -v "Binary" || false; then
    print_fail "Found cllm.h includes in src/core/ (circular dependency)"
    ((FAIL_COUNT++))
else
    print_pass "No circular dependencies detected"
    ((PASS_COUNT++))
fi
echo ""

# Test 9: Check build status
echo "Test 9: Checking build status..."
if [ -f "libcrystalline.so" ] && [ -f "libcllm.so" ]; then
    print_pass "Both libraries built successfully"
    ((PASS_COUNT++))
else
    print_fail "Libraries not built"
    ((FAIL_COUNT++))
fi
echo ""

# Test 10: Check for proper separation in Makefile
echo "Test 10: Checking Makefile structure..."
if grep -q "CRYSTALLINE_OBJECTS.*CORE_OBJECTS.*TRANS_OBJECTS.*GEOM_OBJECTS" Makefile; then
    print_pass "Crystalline library properly defined in Makefile"
    ((PASS_COUNT++))
else
    print_warn "Crystalline library definition unclear in Makefile"
    ((WARN_COUNT++))
fi
echo ""

# Summary
echo "=========================================="
echo "Validation Summary"
echo "=========================================="
echo ""
echo -e "${GREEN}Passed:${NC} $PASS_COUNT"
echo -e "${RED}Failed:${NC} $FAIL_COUNT"
echo -e "${YELLOW}Warnings:${NC} $WARN_COUNT"
echo ""

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}✓ All critical tests passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ Some tests failed. Please review and fix.${NC}"
    exit 1
fi