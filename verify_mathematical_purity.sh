#!/bin/bash

# Comprehensive Mathematical Purity Verification Script
# Verifies that core libraries have ZERO external math dependencies

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║   CRYSTALLINE CLLM - Mathematical Purity Verification          ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

PASS=0
FAIL=0

# Function to check and report
check() {
    local test_name="$1"
    local command="$2"
    local expected="$3"
    
    result=$(eval "$command")
    
    if [ "$result" = "$expected" ]; then
        echo "✓ PASS: $test_name"
        ((PASS++))
        return 0
    else
        echo "✗ FAIL: $test_name (got: $result, expected: $expected)"
        ((FAIL++))
        return 1
    fi
}

echo "═══════════════════════════════════════════════════════════════"
echo "1. CORE CRYSTALLINE LATTICE LIBRARY"
echo "═══════════════════════════════════════════════════════════════"

check "Core source files have no math.h" \
     "find src/core -name '*.c' | xargs grep -c 'include <math\.h>' 2>/dev/null | grep -v ':0' | wc -l" \
     "0"

check "Geometry source files have no math.h" \
     "find src/geometry -name '*.c' | xargs grep -c 'include <math\.h>' 2>/dev/null | grep -v ':0' | wc -l" \
     "0"

check "Transcendental source files have no math.h" \
     "find src/transcendental -name '*.c' | xargs grep -c 'include <math\.h>' 2>/dev/null | grep -v ':0' | wc -l" \
     "0"

check "Crystalline headers have no math.h" \
     "grep -c 'include <math\.h>' include/bigfixed*.h include/bigint*.h include/crystal*.h include/prime*.h 2>/dev/null | grep -v ':0' | wc -l" \
     "0"

check "libcrystalline.so has no libm dependency" \
     "ldd libcrystalline.so 2>/dev/null | grep -c 'libm\.so' || echo 0" \
     "0"

check "libcrystalline.so has no GMP dependency" \
     "ldd libcrystalline.so 2>/dev/null | grep -c 'libgmp\.so' || echo 0" \
     "0"

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo "2. CLLM LIBRARY"
echo "═══════════════════════════════════════════════════════════════"

check "CLLM source files have no math.h" \
     "find src/ai -name '*.c' | xargs grep -c 'include <math\.h>' 2>/dev/null | grep -v ':0' | wc -l" \
     "0"

check "CLLM uses prime_float_math.h" \
     "grep -l 'prime_float_math\.h' src/ai/*.c 2>/dev/null | wc -l | awk '{if(\$1>0) print &quot;yes&quot;; else print &quot;no&quot;}'" \
     "yes"

check "libcllm.so has no libm dependency" \
     "ldd libcllm.so 2>/dev/null | grep -c 'libm\.so' || echo 0" \
     "0"

check "libcllm.so has no GMP dependency" \
     "ldd libcllm.so 2>/dev/null | grep -c 'libgmp\.so' || echo 0" \
     "0"

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo "3. CRAWLER LIBRARY"
echo "═══════════════════════════════════════════════════════════════"

check "Crawler source files have no math.h" \
     "find src/crawler -name '*.c' | xargs grep -c 'include <math\.h>' 2>/dev/null | grep -v ':0' | wc -l" \
     "0"

check "Crawler source has no direct GMP usage" \
     "grep -r 'gmp\.h\|mpz_\|mpq_' src/crawler/ 2>/dev/null | wc -l" \
     "0"

echo "  ℹ Note: libcrawler.so has transitive GMP via libcurl (ACCEPTABLE for SSL/TLS)"

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo "4. DOCUMENT PROCESSING LIBRARY"
echo "═══════════════════════════════════════════════════════════════"

check "Document processing has no math.h" \
     "find src/document_processing -name '*.c' | xargs grep -c 'include <math\.h>' 2>/dev/null | grep -v ':0' | wc -l" \
     "0"

check "libdocproc.so has no libm dependency" \
     "ldd libdocproc.so 2>/dev/null | grep -c 'libm\.so' || echo 0" \
     "0"

check "libdocproc.so has no GMP dependency" \
     "ldd libdocproc.so 2>/dev/null | grep -c 'libgmp\.so' || echo 0" \
     "0"

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo "5. BUILD SYSTEM"
echo "═══════════════════════════════════════════════════════════════"

check "Makefile has no -lm in core library builds" \
     "grep 'libcrystalline.so\|libcllm.so\|libdocproc.so' Makefile | grep -c '\-lm' || echo 0" \
     "0"

check "All libraries built successfully" \
     "ls libcrystalline.so libcllm.so libcrawler.so libdocproc.so 2>/dev/null | wc -l" \
     "4"

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo "SUMMARY"
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "Tests Passed: $PASS"
echo "Tests Failed: $FAIL"
echo ""

if [ $FAIL -eq 0 ]; then
    echo "╔════════════════════════════════════════════════════════════════╗"
    echo "║                    ✅ ALL TESTS PASSED                         ║"
    echo "║                                                                ║"
    echo "║         ABSOLUTE MATHEMATICAL PURITY VERIFIED                  ║"
    echo "║                                                                ║"
    echo "║   Core Libraries: 100% PRISTINE                                ║"
    echo "║   Status: PRODUCTION READY                                     ║"
    echo "╚════════════════════════════════════════════════════════════════╝"
    exit 0
else
    echo "╔════════════════════════════════════════════════════════════════╗"
    echo "║                    ✗ SOME TESTS FAILED                         ║"
    echo "║                                                                ║"
    echo "║   Please review the failures above                             ║"
    echo "╚════════════════════════════════════════════════════════════════╝"
    exit 1
fi