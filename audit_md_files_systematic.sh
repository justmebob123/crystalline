#!/bin/bash

echo "# Systematic MD File Audit"
echo ""
echo "## Quick Analysis of File Relevance"
echo ""

# Function to check if a file is likely obsolete
check_obsolete() {
    local file=$1
    local basename=$(basename "$file")
    
    # Check for date-specific or session-specific names
    if echo "$basename" | grep -qiE "(session|status|progress|current|final)"; then
        echo "LIKELY_OBSOLETE: $file (ephemeral/status)"
        return 0
    fi
    
    # Check for completion reports
    if echo "$basename" | grep -qiE "complete|finished|done"; then
        echo "VERIFY_CODE: $file (claims completion)"
        return 0
    fi
    
    # Check for analysis documents
    if echo "$basename" | grep -qiE "analysis|report|summary"; then
        echo "CHECK_IMPACT: $file (analysis/report)"
        return 0
    fi
    
    # Check for plans
    if echo "$basename" | grep -qiE "plan|roadmap"; then
        echo "CHECK_IMPLEMENTED: $file (plan/roadmap)"
        return 0
    fi
    
    return 1
}

# Scan all MD files (excluding repo_training_data)
find . -name "*.md" -type f | grep -v repo_training_data | while read file; do
    check_obsolete "$file"
done | sort | uniq -c | sort -rn

echo ""
echo "## Summary"
echo "Total MD files (excluding training data): $(find . -name '*.md' -type f | grep -v repo_training_data | wc -l)"
