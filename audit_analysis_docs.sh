#!/bin/bash

echo "# Analysis Documents Audit Report"
echo ""
echo "Checking which analysis documents are still relevant..."
echo ""

# Function to check if a file references code that exists
check_code_refs() {
    local file=$1
    echo "## $file"
    
    # Check if it references specific .c or .h files
    refs=$(grep -oE "[a-z_]+\.(c|h)" "$file" 2>/dev/null | sort -u | head -5)
    if [ -n "$refs" ]; then
        echo "References code files:"
        echo "$refs" | while read ref; do
            if [ -f "src/**/$ref" ] || [ -f "include/$ref" ]; then
                echo "  ✓ $ref (exists)"
            else
                echo "  ✗ $ref (NOT FOUND)"
            fi
        done
    fi
    
    # Check file size and date
    size=$(wc -c < "$file")
    echo "Size: $size bytes"
    echo ""
}

# Check a sample of analysis documents
for doc in TRAINING_PERFORMANCE_ANALYSIS.md ANALYSIS_COMPLETE_SUMMARY.md COMPLETE_ANALYSIS_SUMMARY.md COMPREHENSIVE_ANALYSIS.md; do
    if [ -f "$doc" ]; then
        check_code_refs "$doc"
    fi
done
