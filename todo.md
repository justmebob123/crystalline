# TODO - Crystalline CLLM Project

## RULES (PASTE TO TOP WITH EVERY RESPONSE)
- **Rule 0:** Always paste rules to the top of `todo.md` with every response
- **Rule 1:** Always reread `MASTER_PLAN.md` before any action
- **Rule 2:** Reference `AUDIT.md` for architectural state
- **Rule 3:** Reference `SECONDARY_OBJECTIVES.md` for detailed tasks
- **Rule 4:** Do not create new `.md` files (EXCEPTION: Critical analysis documents)
- **Rule 5:** Always commit all changes using correct authentication
- **Rule 6:** `MASTER_PLAN.md` is read-only - do not edit without explicit approval

## 🚨 CRITICAL SYSTEM FAILURES - IMMEDIATE ACTION REQUIRED

### User-Reported Issues (ALL CRITICAL)
1. ❌ **Memory Corruption** - Core dump when switching tabs
2. ❌ **Input System Broken** - Cannot type in URL Manager or Crawler inputs
3. ❌ **Crawler Database Disconnect** - Crawler ignores database, uses hardcoded URLs
4. ❌ **Forced Model Creation** - Crawler creates new model despite existing one
5. ❌ **ButtonManager Build Failure** - 13 undefined references, application won't compile
6. ❌ **Quick Fix Failed** - Send button still doesn't work

### Analysis Documents Created
- ✅ `CRITICAL_ISSUES_ANALYSIS.md` - Comprehensive root cause analysis
- ✅ `INPUT_SYSTEM_ANALYSIS.md` - Input/button system architecture
- ✅ Updated `SECONDARY_OBJECTIVES.md` - Added 6 new critical objectives

## RECOMMENDED ACTION PLAN

### Option A: REVERT AND FIX (SAFEST - 6-8 hours)
1. Revert ButtonManager changes to last working commit
2. Fix memory corruption with valgrind
3. Fix input system text entry
4. Fix crawler database integration
5. Fix model initialization
6. Test thoroughly
7. THEN properly implement ButtonManager

### Option B: FIX FORWARD (RISKIER - 10-14 hours)
1. Fix memory corruption FIRST (critical)
2. Fix input system SECOND (critical)
3. Complete ButtonManager implementation
4. Fix crawler database integration
5. Fix model initialization
6. Test everything

### Option C: HYBRID APPROACH (RECOMMENDED - 8-10 hours)
1. Revert ButtonManager (30 min)
2. Fix memory corruption (2-4 hours)
3. Fix input system (1-2 hours)
4. Fix crawler database (1-2 hours)
5. Fix model initialization (30 min)
6. Test thoroughly (1 hour)
7. Properly implement ButtonManager later (3-4 hours)

## CURRENT STATUS

### What's Broken
- ❌ Application crashes (core dump)
- ❌ Inputs don't accept text
- ❌ Buttons don't work (build failure)
- ❌ Crawler ignores database
- ❌ Unnecessary model re-initialization
- ❌ Application won't compile

### What's Working
- ✅ Database creation and URL storage
- ✅ Model loading at startup
- ✅ Basic rendering
- ✅ Tab switching (until crash)

## EXECUTION PLAN: COMPREHENSIVE FIX - ALL ISSUES

**User Decision:** Fix EVERYTHING properly - no stubs, no shortcuts, complete solutions

### Phase 1: Memory Corruption Fix (2-4 hours)
- [ ] Run valgrind to identify all memory errors
- [ ] Enable ASAN (Address Sanitizer)
- [ ] Fix all detected memory leaks
- [ ] Fix all buffer overflows
- [ ] Fix all use-after-free errors
- [ ] Test tab switching thoroughly

### Phase 2: Input System Complete Overhaul (3-4 hours)
- [ ] Fix SDL_StartTextInput() integration
- [ ] Fix text rendering in all inputs
- [ ] Fix cursor rendering and positioning
- [ ] Implement proper text editing (insert, delete, backspace)
- [ ] Fix Enter key submission
- [ ] Test all 19 inputs across all tabs
- [ ] Verify callbacks work correctly

### Phase 3: ButtonManager Complete Implementation (4-5 hours)
- [ ] Implement ALL 13 missing callbacks (NO STUBS)
- [ ] Wire all buttons to actual functionality
- [ ] Remove duplicate button handling from tabs
- [ ] Test every button on every tab
- [ ] Verify button states (hover, pressed, disabled)
- [ ] Integration test entire system

### Phase 4: Crawler Database Integration (2-3 hours)
- [ ] Wire crawler to CrawlerURLManager
- [ ] Remove ALL hardcoded URLs
- [ ] Implement proper URL queue from database
- [ ] Test URL addition workflow
- [ ] Test crawler using database URLs
- [ ] Verify URL status updates

### Phase 5: Model Management Fix (1 hour)
- [ ] Fix model name resolution
- [ ] Use existing models
- [ ] Eliminate unnecessary re-initialization
- [ ] Test model loading

### Phase 6: Comprehensive Testing (2-3 hours)
- [ ] Test all inputs on all tabs
- [ ] Test all buttons on all tabs
- [ ] Test tab switching
- [ ] Test crawler workflow
- [ ] Test model loading
- [ ] Memory leak testing
- [ ] Stress testing

**TOTAL ESTIMATED TIME: 14-22 hours**
**APPROACH: Complete, proper solutions - NO SHORTCUTS**