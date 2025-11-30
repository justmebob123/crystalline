# TODO - Crystalline CLLM Project

## RULES (ALWAYS FOLLOW)

### RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE
**HIGHEST PRIORITY RULE - MUST BE FOLLOWED WITH EVERY SINGLE RESPONSE**

At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

### RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
Before taking ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Understand the current objectives
3. Verify your action aligns with the master plan
4. Check for any blocking priorities

### RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
The AUDIT.md contains:
- Current architectural violations
- Required fixes with priorities
- Implementation phases
- Testing requirements
- Success criteria

### RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
The SECONDARY_OBJECTIVES.md contains:
- Detailed implementation tasks
- Code examples
- File-by-file changes
- Testing procedures
- Validation steps

### RULE 4: DO NOT CREATE NEW MD FILES
All documentation goes in existing files or this master plan only.

### RULE 5: ALWAYS COMMIT ALL CHANGES USING CORRECT AUTHENTICATION
```bash
git add .
git commit -m "descriptive message"
git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
```

### RULE 6: THIS FILE IS READ-ONLY - DO NOT EDIT WITHOUT EXPLICIT APPROVAL
**CRITICAL**: MASTER_PLAN.md contains OBJECTIVES ONLY - NO status updates, NO ephemeral information.

---

## UNIFIED INPUT SYSTEM MIGRATION - COMPREHENSIVE PLAN

### Phase 1: Create Centralized Input Registration System - COMPLETE
- [x] Created input_registration.c with init_all_inputs() function
- [x] Created input_registration.h header file
- [x] Called init_all_inputs() during app initialization
- [x] Registers inputs for ALL tabs at startup:
  - Training tab: 5 inputs
  - Crawler tab: 5 inputs
  - LLM tab: 1 input
  - Research tab: 1 input
  - Models tab: 5 inputs
  - URL Manager tab: 2 inputs
- [x] Disabled lazy registration in training and crawler tabs
- [x] Build successful: Zero errors, zero warnings

### Phase 2: Migrate LLM Tab to InputManager
- [ ] Remove custom input_active flag
- [ ] Remove direct SDL_StartTextInput() calls
- [ ] Create register_llm_inputs() function
- [ ] Add input field for chat input
- [ ] Update click handlers to use InputManager
- [ ] Test LLM tab input functionality

### Phase 3: Add InputManager to Research Tab
- [ ] Create register_research_inputs() function
- [ ] Add search input field
- [ ] Add any other needed inputs
- [ ] Update click handlers
- [ ] Test Research tab

### Phase 4: Add InputManager to Models Tab
- [ ] Create register_models_inputs() function
- [ ] Add model name input (for create dialog)
- [ ] Add configuration inputs (vocab, layers, etc.)
- [ ] Update click handlers
- [ ] Test Models tab

### Phase 5: Add InputManager to URL Manager Tab
- [ ] Create register_url_manager_inputs() function
- [ ] Add search input field
- [ ] Add URL input field
- [ ] Add filter inputs
- [ ] Update click handlers
- [ ] Test URL Manager tab

### Phase 6: Fix Model Creation
- [ ] Add detailed logging to model_manager_create()
- [ ] Debug why creation fails
- [ ] Fix configuration issues
- [ ] Test model creation from UI

### Phase 7: Comprehensive Testing
- [ ] Test all tabs have inputs registered
- [ ] Test tab switching updates current_tab
- [ ] Test all input fields are clickable
- [ ] Test all input fields accept text
- [ ] Test all buttons work correctly
- [ ] Verify no "wrong tab" messages

---

## STATUS: PHASE 1 COMPLETE - TESTING REQUIRED

### What's Been Fixed:
1. ✅ Created centralized input registration system
2. ✅ All 19 inputs registered at startup (not lazily)
3. ✅ Training tab: 5 inputs
4. ✅ Crawler tab: 5 inputs  
5. ✅ LLM tab: 1 input (chat input)
6. ✅ Research tab: 1 input (search)
7. ✅ Models tab: 5 inputs (create dialog)
8. ✅ URL Manager tab: 2 inputs (search, add URL)

### Expected Results:
- All tabs should have inputs immediately available
- No more "input_count=0" messages
- No more "wrong tab" messages
- All input fields should be clickable
- Logging should show: "Total inputs registered: 19"

### Next Steps:
- Test application startup
- Verify all inputs registered
- Test each tab's input fields
- Continue with Phase 2-7 if needed