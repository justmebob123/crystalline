#!/bin/bash

# Library Reorganization Script
# Moves infrastructure components from src/core/ to src/ai/infrastructure/
# Updates include paths and Makefile

set -e  # Exit on error

echo "=========================================="
echo "Library Reorganization Script"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# Check if we're in the crystalline directory
if [ ! -f "Makefile" ] || [ ! -d "src/core" ]; then
    print_error "Must run from crystalline directory"
    exit 1
fi

print_status "Starting library reorganization..."
echo ""

# Step 1: Create backup
echo "Step 1: Creating backup..."
BACKUP_DIR="backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
cp -r src "$BACKUP_DIR/"
cp -r include "$BACKUP_DIR/"
cp Makefile "$BACKUP_DIR/"
print_status "Backup created in $BACKUP_DIR/"
echo ""

# Step 2: Create new directory structure
echo "Step 2: Creating new directory structure..."
mkdir -p src/ai/infrastructure
mkdir -p include/ai
print_status "Directories created"
echo ""

# Step 3: Move infrastructure components
echo "Step 3: Moving infrastructure components..."

# Components to move
COMPONENTS=(
    "cllm_thread_allocation"
    "cllm_shared_memory"
    "cllm_sphere_stats"
    "cllm_sphere_message"
    "cllm_message_queue"
    "cllm_lattice_hierarchy"
)

for comp in "${COMPONENTS[@]}"; do
    echo "  Moving $comp..."
    
    # Move header
    if [ -f "include/${comp}.h" ]; then
        git mv "include/${comp}.h" "include/ai/${comp}.h" 2>/dev/null || \
        mv "include/${comp}.h" "include/ai/${comp}.h"
        print_status "  Moved include/${comp}.h → include/ai/${comp}.h"
    else
        print_warning "  Header include/${comp}.h not found"
    fi
    
    # Move source
    if [ -f "src/core/${comp}.c" ]; then
        git mv "src/core/${comp}.c" "src/ai/infrastructure/${comp}.c" 2>/dev/null || \
        mv "src/core/${comp}.c" "src/ai/infrastructure/${comp}.c"
        print_status "  Moved src/core/${comp}.c → src/ai/infrastructure/${comp}.c"
    else
        print_warning "  Source src/core/${comp}.c not found"
    fi
    
    # Move object file if exists
    if [ -f "src/core/${comp}.o" ]; then
        rm "src/core/${comp}.o"
        print_status "  Removed old object file"
    fi
done

echo ""

# Step 4: Update include paths in moved files
echo "Step 4: Updating include paths in moved files..."

for comp in "${COMPONENTS[@]}"; do
    # Update header includes in source files
    if [ -f "src/ai/infrastructure/${comp}.c" ]; then
        sed -i 's|#include "cllm_thread_allocation.h"|#include "ai/cllm_thread_allocation.h"|g' "src/ai/infrastructure/${comp}.c"
        sed -i 's|#include "cllm_shared_memory.h"|#include "ai/cllm_shared_memory.h"|g' "src/ai/infrastructure/${comp}.c"
        sed -i 's|#include "cllm_sphere_stats.h"|#include "ai/cllm_sphere_stats.h"|g' "src/ai/infrastructure/${comp}.c"
        sed -i 's|#include "cllm_sphere_message.h"|#include "ai/cllm_sphere_message.h"|g' "src/ai/infrastructure/${comp}.c"
        sed -i 's|#include "cllm_message_queue.h"|#include "ai/cllm_message_queue.h"|g' "src/ai/infrastructure/${comp}.c"
        sed -i 's|#include "cllm_lattice_hierarchy.h"|#include "ai/cllm_lattice_hierarchy.h"|g' "src/ai/infrastructure/${comp}.c"
        print_status "  Updated includes in ${comp}.c"
    fi
    
    # Update header includes in header files
    if [ -f "include/ai/${comp}.h" ]; then
        sed -i 's|#include "cllm_thread_allocation.h"|#include "ai/cllm_thread_allocation.h"|g' "include/ai/${comp}.h"
        sed -i 's|#include "cllm_shared_memory.h"|#include "ai/cllm_shared_memory.h"|g' "include/ai/${comp}.h"
        sed -i 's|#include "cllm_sphere_stats.h"|#include "ai/cllm_sphere_stats.h"|g' "include/ai/${comp}.h"
        sed -i 's|#include "cllm_sphere_message.h"|#include "ai/cllm_sphere_message.h"|g' "include/ai/${comp}.h"
        sed -i 's|#include "cllm_message_queue.h"|#include "ai/cllm_message_queue.h"|g' "include/ai/${comp}.h"
        sed -i 's|#include "cllm_lattice_hierarchy.h"|#include "ai/cllm_lattice_hierarchy.h"|g' "include/ai/${comp}.h"
        print_status "  Updated includes in ${comp}.h"
    fi
done

echo ""

# Step 5: Update include paths in other files that reference moved components
echo "Step 5: Updating include paths in other files..."

# Update all .c and .h files in src/ and include/
find src include -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/ai/infrastructure/*" ! -path "*/ai/${comp}.h" | while read file; do
    if grep -q "cllm_thread_allocation.h\|cllm_shared_memory.h\|cllm_sphere_stats.h\|cllm_sphere_message.h\|cllm_message_queue.h\|cllm_lattice_hierarchy.h" "$file"; then
        sed -i 's|#include "cllm_thread_allocation.h"|#include "ai/cllm_thread_allocation.h"|g' "$file"
        sed -i 's|#include "cllm_shared_memory.h"|#include "ai/cllm_shared_memory.h"|g' "$file"
        sed -i 's|#include "cllm_sphere_stats.h"|#include "ai/cllm_sphere_stats.h"|g' "$file"
        sed -i 's|#include "cllm_sphere_message.h"|#include "ai/cllm_sphere_message.h"|g' "$file"
        sed -i 's|#include "cllm_message_queue.h"|#include "ai/cllm_message_queue.h"|g' "$file"
        sed -i 's|#include "cllm_lattice_hierarchy.h"|#include "ai/cllm_lattice_hierarchy.h"|g' "$file"
        echo "  Updated: $file"
    fi
done

print_status "Include paths updated in all files"
echo ""

# Step 6: Update Makefile
echo "Step 6: Updating Makefile..."

# Add infrastructure sources to AI_SOURCES
if ! grep -q "src/ai/infrastructure" Makefile; then
    # Find the AI_SOURCES line and add infrastructure
    sed -i '/^AI_SOURCES = /a\
INFRASTRUCTURE_SOURCES = $(wildcard src/ai/infrastructure/*.c)\
AI_SOURCES += $(INFRASTRUCTURE_SOURCES)' Makefile
    print_status "Added infrastructure sources to Makefile"
else
    print_warning "Infrastructure sources already in Makefile"
fi

echo ""

# Step 7: Clean build
echo "Step 7: Cleaning build artifacts..."
make clean > /dev/null 2>&1 || true
print_status "Build artifacts cleaned"
echo ""

# Step 8: Test build
echo "Step 8: Testing build..."
if make all > /tmp/build.log 2>&1; then
    print_status "Build successful!"
else
    print_error "Build failed! Check /tmp/build.log for details"
    echo ""
    echo "Last 20 lines of build log:"
    tail -20 /tmp/build.log
    echo ""
    print_warning "Backup available in $BACKUP_DIR/"
    exit 1
fi

echo ""

# Step 9: Verify library structure
echo "Step 9: Verifying library structure..."

if [ -f "libcrystalline.so" ]; then
    print_status "libcrystalline.so exists"
    
    # Check for external math dependencies
    if ldd libcrystalline.so | grep -q "libm.so"; then
        print_error "libcrystalline.so has math.h dependency!"
    else
        print_status "libcrystalline.so has no math.h dependency ✓"
    fi
fi

if [ -f "libcllm.so" ]; then
    print_status "libcllm.so exists"
    
    # Check for crystalline dependency
    if ldd libcllm.so | grep -q "libcrystalline.so"; then
        print_status "libcllm.so depends on libcrystalline.so ✓"
    else
        print_warning "libcllm.so does not depend on libcrystalline.so"
    fi
fi

echo ""

# Step 10: Summary
echo "=========================================="
echo "Reorganization Complete!"
echo "=========================================="
echo ""
echo "Summary of changes:"
echo "  • Moved 6 infrastructure components to src/ai/infrastructure/"
echo "  • Updated all include paths"
echo "  • Updated Makefile"
echo "  • Verified build"
echo ""
echo "Components moved:"
for comp in "${COMPONENTS[@]}"; do
    echo "  ✓ $comp"
done
echo ""
echo "Backup location: $BACKUP_DIR/"
echo ""
print_status "Library reorganization successful!"
echo ""
echo "Next steps:"
echo "  1. Run tests: make test"
echo "  2. Review changes: git status"
echo "  3. Commit changes: git add -A && git commit -m 'Reorganize libraries: move infrastructure to CLLM'"
echo ""