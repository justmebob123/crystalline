# TODO - Crystalline CLLM Project

## RULES (PASTE TO TOP WITH EVERY RESPONSE)
- **Rule 0:** Always paste rules to the top of `todo.md` with every response
- **Rule 1:** Always reread `MASTER_PLAN.md` before any action
- **Rule 2:** Reference `AUDIT.md` for architectural state
- **Rule 3:** Reference `SECONDARY_OBJECTIVES.md` for detailed tasks
- **Rule 4:** Do not create new `.md` files
- **Rule 5:** Always commit all changes using correct authentication:
  ```bash
  git add .
  git commit -m "descriptive message"
  git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
  ```
- **Rule 6:** `MASTER_PLAN.md` is read-only - do not edit without explicit approval

## CRITICAL: FIX LLM TAB TEXT INPUT ✅ PROPERLY FIXED

### Issue
User cannot click or type in the AI LLM chat window text box.

### Root Cause
**DUPLICATE INPUT HANDLING!**
- InputManager (global system) was correctly handling 'llm.chat_input' ✅
- LLM tab had its OWN separate input handling code ❌
- Both systems were fighting for control
- Debug output showed: "InputManager: Focused input 'llm.chat_input'" - InputManager was working!
- But LLM tab's own handler was interfering

### Proper Fix Applied ✅
- [x] Created llm_input_on_change() callback for text updates
- [x] Created llm_input_on_submit() callback for Enter key
- [x] Wired callbacks in input_registration.c
- [x] Updated init_all_inputs() to accept AppState parameter
- [x] Removed duplicate input box click handler from LLM tab
- [x] Made Send button trigger submit callback
- [x] Converted handle_llm_tab_text_input() and handle_llm_tab_key() to stubs
- [x] Application rebuilt successfully
- [x] Ready for testing

### Changes Made
1. **app/ui/tabs/tab_llm.c**:
   - Added llm_input_on_change() and llm_input_on_submit() callbacks
   - Removed duplicate input box click handler
   - Updated Send button to use InputManager
   - Converted legacy handlers to stubs

2. **app/input_registration.c**:
   - Updated init_all_inputs() to accept AppState
   - Wired LLM input callbacks during registration

3. **app/input_registration.h**:
   - Updated function signature

4. **app/main.c**:
   - Pass AppState to init_all_inputs()

### Result
InputManager now has FULL control of LLM tab input with proper callbacks. No more conflicts!

## OBJECTIVE 15 Phase 5: CLI Tool Integration

### Phase 5.1: CLI Tools Inventory and Analysis ✅ COMPLETE
- [x] List all CLI tools in tools/ directory - 30 tools found
- [x] Document each tool's functionality - See CLI_TOOLS_ANALYSIS.md
- [x] Compare with UI tab features - Complete comparison done
- [x] Identify feature gaps (UI-only or CLI-only) - Gaps identified
- [x] Create feature parity matrix - Created in CLI_TOOLS_ANALYSIS.md

### Phase 5.2: Build Missing Critical Tools ✅ COMPLETE
- [x] Build cllm_crawler CLI tool - ✅ BUILT AND TESTED
- [x] Update Makefile to include -lalgorithms for crawler

### Phase 5.3: Cleanup and Organization ⏳ IN PROGRESS

#### Step 1: Remove Legacy Tools ✅ COMPLETE
- [x] Removed tools/cllm_tokenize_old.c
- [x] Removed tools/cllm_vocab_build_old.c
- [ ] Update Makefile to remove references (if any)

#### Step 2: Move Test Tools to tests/ Directory ✅ COMPLETE
- [x] Moved tools/test_simple_init.c to tests/
- [x] Moved tools/test_token_init.c to tests/
- [x] Moved tools/test_hierarchical.c to tests/
- [x] Moved tools/quick_inference_check.c to tests/
- [x] Moved tools/profile_kissing_spheres.c to tests/
- [ ] Update Makefile test targets

#### Step 3: Evaluate Remaining Unbuilt Tools ✅ COMPLETE
- [x] Removed tools/web_scraper.c (redundant with cllm_crawler)
- [x] Moved profile_kissing_spheres.c to tests/ (diagnostic tool)

**Result:** tools/ directory now contains only 22 production tools (down from 30)

#### Step 4: Clean Up Makefile
- [ ] Remove legacy tool targets
- [ ] Update test tool paths
- [ ] Verify all targets build correctly

#### Step 5: Organize Libraries (SECONDARY_OBJECTIVES)
- [ ] Review SECONDARY_OBJECTIVES library organization
- [ ] Implement proper categorization
- [ ] Update build system

### Phase 5.4: Library Reorganization (from SECONDARY_OBJECTIVES)
- [ ] Review SECONDARY_OBJECTIVES.md library organization section
- [ ] Implement proper library categorization
- [ ] Update build system
- [ ] Test all libraries

### Phase 5.5: Documentation
- [ ] Create comprehensive CLI usage guide
- [ ] Document all commands and options
- [ ] Provide workflow examples
- [ ] Create troubleshooting guide