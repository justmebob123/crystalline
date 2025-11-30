# TODO - Crystalline CLLM Project

## RULES (ALWAYS AT TOP)
- Rule 0: Always paste rules to top of todo.md with every response
- Rule 1: Always reread MASTER_PLAN.md before any action
- Rule 2: Reference AUDIT.md for architectural state
- Rule 3: Reference SECONDARY_OBJECTIVES.md for detailed tasks
- Rule 4: Do not create new .md files
- Rule 5: Always commit all changes using correct authentication
- Rule 6: MASTER_PLAN.md is read-only - do not edit without explicit approval

## Current Task: Implement Dynamic Training System

### NEW USER REQUIREMENTS (CRITICAL)
1. **Model Naming & Renaming:**
   - User can name models (not limited to 'crawler' or 'research')
   - User can rename models (renames files and associated data)
   - Default naming acceptable but not required
   
2. **Model Selection Across All Tabs:**
   - Each tab should have model selector dropdown
   - Main model controls under Models tab
   - All tabs can select any loaded model
   
3. **Cross-Model Training:**
   - Training tab can select any model
   - Can import and train on data from ANY other model
   - Example: Select model_A, train on model_B's data

### Phase 1: Model Metadata Enhancement ✅ COMPLETE
- [x] Add epochs_trained to CLLMModel structure
- [x] Add target_epochs to CLLMModel structure
- [x] Add queue_directory to CLLMModel structure
- [x] Add config to CLLMModel structure (training_config struct)
- [x] Add model_name field (user-defined name)
- [x] Add loss_history, history_size, history_capacity fields
- [ ] Update cllm_model_save() to serialize new fields (NEXT)
- [ ] Update cllm_model_load() to deserialize new fields (NEXT)
- [ ] Add cllm_model_get_epochs_trained()
- [ ] Add cllm_model_set_target_epochs()
- [ ] Add cllm_model_rename() function
- [ ] Add cllm_model_get_name() function
- [ ] Add cllm_model_set_name() function

### Phase 2: Enhanced Model Manager (NEW) ⏳
- [x] Add validation for unique model names (in cllm_model_metadata.c)
- [x] Add default naming scheme (model_1, model_2, etc.)
- [ ] Add model renaming capability (NEXT)
- [ ] Rename model file when model renamed
- [ ] Rename queue directory when model renamed
- [ ] Update all references when model renamed
- [ ] Add model_manager_rename_model()

### Phase 3: Per-Model Queue System ✅ COMPLETE
- [x] Create src/ai/model_queue_manager.h
- [x] Create src/ai/model_queue_manager.c
- [x] Implement create_model_queue()
- [x] Implement add_to_model_queue()
- [x] Implement get_next_from_queue()
- [x] Implement get_queue_size()
- [x] Implement clear_model_queue()
- [x] Implement import_queue_from_model() (cross-model training)
- [ ] Update crawler to use model-specific queue (LATER)
- [ ] Update continuous_training to use model-specific queue (LATER)

### Phase 4: Batch Accumulation System
- [ ] Create src/ai/batch_accumulator.h
- [ ] Create src/ai/batch_accumulator.c
- [ ] Implement batch_accumulator_create()
- [ ] Implement batch_accumulator_add()
- [ ] Implement batch_accumulator_ready()
- [ ] Implement batch_accumulator_get_batch()
- [ ] Implement batch_accumulator_clear()
- [ ] Integrate with continuous_training.c

### Phase 5: Dynamic Configuration UI - Models Tab
- [ ] Add model name input field (create/rename)
- [ ] Add rename button for existing models
- [ ] Add validation for unique names
- [ ] Add default name generation
- [ ] Update model creation dialog
- [ ] Update model details panel
- [ ] Add model selector dropdown (main control)

### Phase 6: Dynamic Configuration UI - All Tabs
- [ ] Add model selector dropdown to Training tab
- [ ] Add model selector dropdown to Crawler tab
- [ ] Add model selector dropdown to LLM tab
- [ ] Add model selector dropdown to Research tab
- [ ] Wire all selectors to model manager
- [ ] Ensure consistent model selection across tabs

### Phase 7: Training Tab Enhancements
- [ ] Add batch_size slider
- [ ] Add sequence_length slider
- [ ] Add epochs slider
- [ ] Add learning_rate input
- [ ] Add current model info display
- [ ] Add queue size display
- [ ] Add epochs_trained display
- [ ] Add target_epochs display
- [ ] Add "Import Data From Model" dropdown (NEW)
- [ ] Add "Import Data" button (NEW)
- [ ] Wire configuration to training system

### Phase 8: Epoch Tracking System
- [ ] Implement cllm_model_get_epochs_trained()
- [ ] Implement cllm_model_set_target_epochs()
- [ ] Implement cllm_model_validate_epochs()
- [ ] Implement cllm_model_continue_training()
- [ ] Update training loop to track epochs
- [ ] Update UI to display epoch information
- [ ] Add epoch validation logic

### Phase 9: Layer Validation System
- [ ] Implement cllm_model_validate_layers()
- [ ] Implement cllm_model_is_layer_reduction()
- [ ] Implement cllm_model_increase_layers()
- [ ] Implement cllm_model_get_layer_constraints()
- [ ] Add UI validation for layer changes
- [ ] Add layer info display in UI
- [ ] Prevent layer reduction on existing models

### Phase 10: Unified Data Ingestion
- [ ] Create manual file drop mechanism
- [ ] Add "Add Training Data" button to Training tab
- [ ] Implement file browser for selecting files
- [ ] Tokenize selected files
- [ ] Add to model-specific queue
- [ ] Update crawler to use same queue format
- [ ] Test unified queue system

### Phase 11: Testing & Validation
- [ ] Test per-model queues
- [ ] Test batch accumulation
- [ ] Test dynamic configuration
- [ ] Test epoch tracking
- [ ] Test layer validation
- [ ] Test unified data ingestion
- [ ] Test model selection across tabs
- [ ] Test model renaming
- [ ] Test cross-model training
- [ ] Integration testing

### Phase 12: Update SECONDARY_OBJECTIVES.md
- [ ] Add model naming and renaming requirements
- [ ] Add cross-model training requirements
- [ ] Add model selector requirements for all tabs
- [ ] Update implementation details
- [ ] Add code examples
- [ ] Document new APIs