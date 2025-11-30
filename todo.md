# TODO - Crystalline CLLM Project

## RULES (PASTE TO TOP WITH EVERY RESPONSE)
- **Rule 0:** Always paste rules to the top of `todo.md` with every response
- **Rule 1:** Always reread `MASTER_PLAN.md` before any action
- **Rule 2:** Reference `AUDIT.md` for architectural state
- **Rule 3:** Reference `SECONDARY_OBJECTIVES.md` for detailed tasks
- **Rule 4:** Do not create new `.md` files
- **Rule 5:** Always commit all changes using correct authentication:
  git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
- **Rule 6:** `MASTER_PLAN.md` is read-only - do not edit without explicit approval

## OBJECTIVE 15 Phase 5: CLI Tool Integration ⏳ IN PROGRESS

### Context from MASTER_PLAN
- Analyze CLI tools for feature parity with UI
- Enhance CLI tools where needed
- Document CLI usage
- Ensure UI and CLI provide equivalent functionality

### Phase 5.1: CLI Tools Inventory and Analysis ✅ COMPLETE
- [x] List all CLI tools in tools/ directory - 30 tools found
- [x] Document each tool's functionality - See CLI_TOOLS_ANALYSIS.md
- [x] Compare with UI tab features - Complete comparison done
- [x] Identify feature gaps (UI-only or CLI-only) - Gaps identified
- [x] Create feature parity matrix - Created in CLI_TOOLS_ANALYSIS.md

**Key Findings:**
- 23 tools built and functional
- 7 tools not built (including critical cllm_crawler)
- 2 legacy tools to remove (*_old.c)
- Good parity for training, inference, and model management
- Missing: crawler CLI, URL management CLI

### Phase 5.2: Build Missing Critical Tools ⏳ IN PROGRESS
- [x] Build cllm_crawler CLI tool - ✅ BUILT AND TESTED
- [x] Update Makefile to include -lalgorithms for crawler
- [ ] Test crawler functionality with real URLs
- [ ] Build web_scraper tool (or remove if redundant)
- [ ] Build or remove unclear test tools (quick_inference_check, etc.)
- [ ] Remove legacy tools (cllm_tokenize_old.c, cllm_vocab_build_old.c)

### Phase 5.3: Documentation
- [ ] Create comprehensive CLI usage guide
- [ ] Document all command-line options
- [ ] Provide usage examples
- [ ] Create troubleshooting guide
- [ ] Document UI-CLI equivalents

### Phase 5.4: Testing
- [ ] Test all CLI tools with various inputs
- [ ] Verify error handling
- [ ] Test edge cases
- [ ] Verify output formats
- [ ] Create integration tests