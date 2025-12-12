# Shell Scripts Audit

## Total: 46 shell scripts tracked in git

### Category 1: Essential Scripts (KEEP - 4 files)
1. install_deps_ubuntu.sh - Dependency installation
2. install_deps_centos.sh - Dependency installation
3. run_training.sh - Training execution
4. tests/run_phase1_tests.sh - Test execution

### Category 2: Useful Utilities (KEEP - 3 files)
5. deploy_production.sh - Production deployment
6. monitor_training.sh - Training monitoring
7. training_monitor/monitor.sh - Training monitoring

### Category 3: Debug/Analysis Scripts (REMOVE - 25 files)
- analyze_*.sh (7 files)
- debug_*.sh (1 file)
- audit_*.sh (2 files)
- deep_analysis.sh
- check_training_status.sh
- comprehensive_monitor.sh
- monitor_and_report.sh
- monitor_and_save_model.sh
- find_double_arrays.sh
- run_gdb.sh
- create_file_inventory.sh
- categorize_md_files.sh
- delete_ephemeral_files.sh
- delete_round2_files.sh
- delete_round3_files.sh
- delete_round4_final.sh
- prepare_model_for_download.sh
- verify_mathematical_purity.sh

### Category 4: One-time Fix Scripts (REMOVE - 14 files)
- FIX_MATH_DEPENDENCIES.sh
- REORGANIZE_LIBRARIES.sh
- VALIDATE_LIBRARIES.sh
- add_comprehensive_debug.sh
- fix_model_initialization.sh
- fix_pointer_types.sh
- fix_validate_calls.sh
- implement_phase1_fixes.sh
- replace_math_deps.sh
- create_large_dataset.sh
- full_scale_training.sh
- minimal_train.sh
- real_training_test.sh
- regenerate_models.sh

## Recommendation

**Remove 39 scripts** (Categories 3 & 4)
**Keep 7 scripts** (Categories 1 & 2)

These are one-time use, debug, or analysis scripts that served their purpose and are no longer needed.

## Execution

```bash
# Remove debug/analysis scripts
git rm analyze_*.sh debug_*.sh audit_*.sh deep_analysis.sh \
  check_training_status.sh comprehensive_monitor.sh \
  monitor_and_report.sh monitor_and_save_model.sh \
  find_double_arrays.sh run_gdb.sh create_file_inventory.sh \
  categorize_md_files.sh delete_*.sh prepare_model_for_download.sh \
  verify_mathematical_purity.sh

# Remove one-time fix scripts
git rm FIX_*.sh REORGANIZE_*.sh VALIDATE_*.sh \
  add_comprehensive_debug.sh fix_*.sh implement_*.sh \
  replace_*.sh create_large_dataset.sh *_training.sh \
  regenerate_models.sh
```