# TODO - Fix Model Loading Issue

## RULES (PASTED FROM MASTER_PLAN)
- RULE 0: Always paste rules to top of todo.md
- RULE 1: Always reread MASTER_PLAN.md before any action
- RULE 2: Reference AUDIT.md for architectural state
- RULE 3: Reference SECONDARY_OBJECTIVES.md for detailed tasks
- RULE 4: Do not create new MD files
- RULE 5: Always commit all changes using correct authentication
- RULE 6: MASTER_PLAN.md is READ-ONLY
- RULE 7: Fix all build warnings before proceeding

## CRITICAL ISSUE: Model Loading on Tab Open

### Problem
When the Training Tab opens, it automatically loads the first model into memory via the model selector callback. This causes:
- Massive memory consumption (12GB+ for 50K vocab models)
- Application hang during load
- Out of memory errors
- Undesirable behavior (models should load on-demand when training starts)

### Root Cause
The `on_model_selected()` callback in `tab_training.c` calls `model_manager_acquire_write()` which triggers lazy loading. This callback is invoked when the model selector initializes and sets the first model as selected.

### Correct Architecture (from MASTER_PLAN)
- Models should be REGISTERED during initialization (name + path only)
- Models should be LOADED on-demand when actually needed (training start)
- Model selector should show available models WITHOUT loading them
- Only load model when user explicitly starts training

### Fix Plan
- [x] Identify root cause (on_model_selected callback)
- [x] Remove model loading from on_model_selected callback
- [x] Store selected model NAME only (not pointer)
- [x] Load model only when "Start Training" button is clicked
- [x] Update model status display to show "Available" vs "Loaded"
- [x] Build succeeds with zero warnings
- [ ] Test that models are not loaded until training starts
- [ ] Verify memory usage stays low on tab open

### Implementation Steps
1. Modify `on_model_selected()` to store model name only
2. Add `selected_model_name` field to training tab state
3. Modify "Start Training" button to load model on-demand
4. Update model status display logic
5. Test and verify

## Phase 2: Complete UI Layout Rewrite (After Fix)
- [ ] Design new layout system for all tabs
- [ ] Implement responsive layout engine
- [ ] Apply to Training Tab
- [ ] Apply to Models Tab
- [ ] Apply to LLM Tab
- [ ] Apply to Research Tab
- [ ] Apply to Benchmark Tab
- [ ] Apply to Adapters Tab