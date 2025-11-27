#!/bin/bash

echo "Final cleanup: Removing last obsolete files..."
echo ""

# Remove audit documents (we're done with them)
rm -fv ./OBJECTIVE_17_DESIGN_AUDIT.md
rm -fv ./OBJECTIVE_17_PHASE2_AUDIT.md
rm -fv ./ROUND2_FILES_TO_DELETE.md
rm -fv ./ROUND3_FILES_TO_DELETE.md

# Remove historical summaries
rm -fv ./PHASE2_DAY7_SUMMARY.md
rm -fv ./PHASE3_SUMMARY.md
rm -fv ./PROJECT_COMPLETE_SUMMARY.md

# Remove obsolete analysis
rm -fv ./THREE_CRITICAL_FIXES.md
rm -fv ./TRAINING_ISSUE_ANALYSIS.md
rm -fv ./TRAINING_PERFORMANCE_ANALYSIS.md
rm -fv ./TRAINING_THREAD_UPDATE.md

# Remove old todos (we have todo.md)
rm -fv ./implementation_todo.md
rm -fv ./library_analysis_todo.md
rm -fv ./comprehensive_test_plan.md

# Remove workspace system (covered in README)
rm -fv ./WORKSPACE_SYSTEM.md

# Remove schema backup (can regenerate)
rm -fv ./SCHEMA_BACKUP.md

echo ""
echo "Done! Final count:"
echo "Total MD files now: $(find . -name '*.md' -type f | grep -v repo_training_data | wc -l)"