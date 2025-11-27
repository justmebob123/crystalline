#!/bin/bash

echo "Deleting 50 ephemeral files..."
echo ""

# Session summaries
rm -fv ./CLEANUP_SESSION_SUMMARY.md
rm -fv ./SESSION_COMPLETE_SUMMARY.md
rm -fv ./SESSION_COMPLETE_TRAINING_BUG_FIXED.md
rm -fv ./SESSION_SUMMARY_CURRENT.md
rm -fv ./SESSION_SUMMARY_PERFORMANCE_ANALYSIS.md

# Status reports
rm -fv ./64CORE_TRAINING_STATUS.md
rm -fv ./ATTENTION_BACKWARD_PROGRESS.md
rm -fv ./CURRENT_STATUS_FINAL.md
rm -fv ./CURRENT_STATUS_SUMMARY.md
rm -fv ./CURRENT_TRAINING_STATUS.md
rm -fv ./DAY4_INTEGRATION_STATUS.md
rm -fv ./FINAL_STATUS_MATHEMATICAL_PURITY.md
rm -fv ./FINAL_STATUS_REPORT.md
rm -fv ./FULL_TRAINING_IN_PROGRESS.md
rm -fv ./INTEGRATION_STATUS_DAY3.md
rm -fv ./INTEGRATION_STATUS_REPORT.md
rm -fv ./MASTER_PLAN_PROGRESS_SUMMARY.md
rm -fv ./MULTI_THREADING_STATUS.md
rm -fv ./OBJECTIVES_STATUS_REPORT.md
rm -fv ./PHASE2_PROGRESS.md
rm -fv ./PHASE3_PROGRESS_REPORT.md
rm -fv ./PROJECT_STATUS_PHASE2_DAY8.md
rm -fv ./TRAINING_IN_PROGRESS.md
rm -fv ./TRAINING_STATUS_REPORT.md
rm -fv ./TRAINING_STATUS_SUMMARY.md
rm -fv ./WARNING_FIX_PROGRESS.md

# From training data directory
rm -fv ./repo_training_data/CLEANUP_SESSION_SUMMARY.md
rm -fv ./repo_training_data/SESSION_COMPLETE_SUMMARY.md
rm -fv ./repo_training_data/SESSION_COMPLETE_TRAINING_BUG_FIXED.md
rm -fv ./repo_training_data/SESSION_SUMMARY_CURRENT.md
rm -fv ./repo_training_data/SESSION_SUMMARY_PERFORMANCE_ANALYSIS.md
rm -fv ./repo_training_data/64CORE_TRAINING_STATUS.md
rm -fv ./repo_training_data/ATTENTION_BACKWARD_PROGRESS.md
rm -fv ./repo_training_data/CURRENT_STATUS_FINAL.md
rm -fv ./repo_training_data/CURRENT_STATUS_SUMMARY.md
rm -fv ./repo_training_data/CURRENT_TRAINING_STATUS.md
rm -fv ./repo_training_data/DAY4_INTEGRATION_STATUS.md
rm -fv ./repo_training_data/FINAL_STATUS_MATHEMATICAL_PURITY.md
rm -fv ./repo_training_data/FINAL_STATUS_REPORT.md
rm -fv ./repo_training_data/INTEGRATION_STATUS_DAY3.md
rm -fv ./repo_training_data/INTEGRATION_STATUS_REPORT.md
rm -fv ./repo_training_data/MASTER_PLAN_PROGRESS_SUMMARY.md
rm -fv ./repo_training_data/MULTI_THREADING_STATUS.md
rm -fv ./repo_training_data/OBJECTIVES_STATUS_REPORT.md
rm -fv ./repo_training_data/PHASE2_PROGRESS.md
rm -fv ./repo_training_data/PHASE3_PROGRESS_REPORT.md
rm -fv ./repo_training_data/PROJECT_STATUS_PHASE2_DAY8.md
rm -fv ./repo_training_data/TRAINING_IN_PROGRESS.md
rm -fv ./repo_training_data/TRAINING_STATUS_REPORT.md
rm -fv ./repo_training_data/WARNING_FIX_PROGRESS.md

echo ""
echo "Done! Checking remaining files..."
echo "Total MD files now: $(find . -name '*.md' | wc -l)"
