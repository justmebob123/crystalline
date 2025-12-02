# RULES (MUST FOLLOW)
1. ALWAYS reread MASTER_PLAN.md before any action
2. Reference AUDIT.md for architectural state
3. Reference SECONDARY_OBJECTIVES.md for detailed tasks
4. NO standalone files - integrate into existing architecture
5. ALWAYS commit changes with correct authentication
6. This file is READ-ONLY - NO status updates without approval
7. Fix HTML entities immediately when they occur
8. Fix ALL build warnings before proceeding

# CURRENT ISSUE: Model Loading Broken

## Problem Analysis
User reports: "something you changed broke model loading again. it appears to be looking for a default model in multiple paths but can't find it and no longer loads my other models under other names. my models are under the models/ directory."

## Root Cause Identified
In `src/ai/cllm_model_manager.c`, the `model_manager_init()` function:
1. Scans the models directory for .cllm files ✅
2. Prints each found model ✅
3. **BUT NEVER REGISTERS THEM** ❌

The code at lines 88-120 finds models but only prints them:
```c
printf("  Found model: %s (will load on-demand)\n", model_name);
found_count++;
// NOTE: Model will be loaded on-demand when user explicitly requests it
```

But there's NO code to actually add the model to `g_model_manager.models[]` array!

Later, when `model_manager_acquire_read()` is called:
- It calls `find_model_by_name()` which searches `g_model_manager.models[]`
- Since the models were never added to the array, it returns NULL
- User gets "Model 'xxx' not found" error

## Solution
Add model registration during directory scan. The model should be registered with:
- Model name extracted from filename
- File path stored
- Model NOT loaded yet (lazy loading)
- Mark as `is_accessible = false` until explicitly loaded

## Tasks
- [x] Fix model_manager_init() to register scanned models
- [x] Add lazy loading to model_manager_acquire_read()
- [x] Add lazy loading to model_manager_acquire_write()
- [x] Build successful with zero errors
- [ ] Test with user's models in models/ directory
- [ ] Commit fix with clear message

## Files to Modify
- `src/ai/cllm_model_manager.c` - Add model registration during scan