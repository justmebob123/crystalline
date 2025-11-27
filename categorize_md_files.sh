#!/bin/bash

echo "# MD File Categorization Report"
echo ""
echo "Total MD files: $(find . -name '*.md' | wc -l)"
echo ""

echo "## Category Breakdown"
echo ""
echo "| Category | Count | Percentage |"
echo "|----------|-------|------------|"

total=$(find . -name "*.md" | wc -l)
analysis=$(find . -name "*.md" | grep -iE "(analysis|report|audit)" | wc -l)
plans=$(find . -name "*.md" | grep -iE "(plan|implementation|roadmap)" | wc -l)
complete=$(find . -name "*.md" | grep -iE "(complete|finished|done)" | wc -l)
session=$(find . -name "*.md" | grep -iE "session" | wc -l)
status=$(find . -name "*.md" | grep -iE "(status|progress)" | wc -l)

echo "| Analysis Documents | $analysis | $((analysis * 100 / total))% |"
echo "| Implementation Plans | $plans | $((plans * 100 / total))% |"
echo "| Completion Reports | $complete | $((complete * 100 / total))% |"
echo "| Session Summaries | $session | $((session * 100 / total))% |"
echo "| Status Reports | $status | $((status * 100 / total))% |"

echo ""
echo "## Likely Candidates for Deletion"
echo ""
echo "Session summaries (ephemeral): $session files"
echo "Status reports (point-in-time): $status files"
echo ""
echo "**Estimated deletable: $((session + status)) files (~$((((session + status) * 100) / total))% of total)**"

echo ""
echo "## Files Needing Audit"
echo ""
echo "Analysis documents: $analysis files - Check if led to code changes"
echo "Implementation plans: $plans files - Check if actually implemented"
echo "Completion reports: $complete files - Verify code exists and works"
echo ""
echo "**Total needing audit: $((analysis + plans + complete)) files (~$((((analysis + plans + complete) * 100) / total))% of total)**"
