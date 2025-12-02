# TODO - Crystalline CLLM Project

**CRITICAL RULES (from MASTER_PLAN.md) - MUST FOLLOW:**
- **Rule 0**: ALWAYS paste rules to top of todo.md with EVERY response, read MASTER_PLAN.md, AUDIT.md, SECONDARY_OBJECTIVES.md
- **Rule 1**: ALWAYS reread MASTER_PLAN.md before ANY action
- **Rule 2**: Reference AUDIT.md for architectural state
- **Rule 3**: Reference SECONDARY_OBJECTIVES.md for detailed tasks
- **Rule 4**: Do NOT create new .md files OR standalone .c files - integrate into existing codebase
- **Rule 5**: ALWAYS commit using: `git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main`
- **Rule 6**: MASTER_PLAN.md is READ-ONLY - do not edit without explicit approval
- **Rule 7**: FIX ALL BUILD WARNINGS before proceeding
- **Rule 8**: NO math.h usage - ONLY crystalline math functions (prime_*)

## ✅ RULE VIOLATION CORRECTED

**FINDING**: Hash map fix was ALREADY properly integrated into src/ai/cllm_training.c (lines 421-550)
**VIOLATION**: Created duplicate standalone file fix_tokenization_hang.c (5262 bytes)
**ACTION**: Remove the duplicate file immediately

## ✅ CLEANUP COMPLETED

- [x] Verified hash map fix is in src/ai/cllm_training.c
- [x] Confirmed fix uses O(1) hash map lookup instead of O(n²) linear search
- [x] Deleted fix_tokenization_hang.c (duplicate/unnecessary)
- [x] Found 11 more standalone test/debug files in root directory
- [x] Analyzed all 11 files - categorized as tests/tools/debug/config
- [x] Verified production_config.h is UNUSED (0 references in codebase)
- [x] Moved 6 test files to tests/ directory
- [x] Moved 3 tool files to tools/ directory
- [x] Deleted debug_warmup.c (temporary debug file)
- [x] Deleted production_config.h (unused config file)
- [x] Verified build still works (clean build successful)
- [ ] Commit cleanup changes with proper message

## SUMMARY OF CHANGES
**Files Moved to tests/ (6):**
- test_bigint_init.c
- test_barriers.c
- test_batch_iterator_simple.c
- test_phase4.c
- test_race_fix.c
- verify_12fold_symmetry.c

**Files Moved to tools/ (3):**
- create_test_model.c
- train_cllm_repo.c
- simple_train_and_infer.c

**Files Deleted (3):**
- fix_tokenization_hang.c (duplicate - already in src/ai/cllm_training.c)
- debug_warmup.c (temporary debug file)
- production_config.h (unused config file)

**Result:** Root directory now clean - zero standalone .c/.h files

## STANDALONE FILES ANALYSIS (11 files)

**LEGITIMATE TEST FILES - Move to tests/:**
- test_bigint_init.c (1.4K) - BigInt initialization test
- test_barriers.c (1.8K) - Thread barrier test
- test_batch_iterator_simple.c (2.3K) - Batch iterator test
- test_phase4.c (3.2K) - Phase 4 test
- test_race_fix.c (3.1K) - Race condition fix test
- verify_12fold_symmetry.c (1.5K) - 12-fold symmetry verification

**LEGITIMATE TOOLS - Move to tools/:**
- create_test_model.c (1.5K) - Test model creation utility
- train_cllm_repo.c (9.3K) - Repository training program
- simple_train_and_infer.c (3.8K) - Simple training/inference demo

**DEBUG FILES - DELETE:**
- debug_warmup.c (2.2K) - Temporary debug file

**CONFIG FILES - EVALUATE:**
- production_config.h (4.3K) - Production config (might be useful, check if used)

## NOTES
- The tokenization fix is ALREADY in the codebase where it belongs
- The standalone file was created in error and must be removed
- This is a clear example of violating Rule #4