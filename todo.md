# TODO - EXECUTING CRITICAL OBJECTIVES

## 🔒 RULES (PASTED FROM MASTER_PLAN)

### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

---

## ✅ COMPLETED: OBJECTIVE 21 - Fix Backwards "Simple Loss" Naming

Successfully renamed:
- `cllm_simple_loss.h` → `cllm_loss.h` (THE REAL LOSS)
- `cllm_loss.h` → `cllm_tensor_loss.h` (infrastructure)
- Updated all includes
- Build verified: Zero errors, 78 warnings (no new warnings)

---

## 🎯 CURRENT OBJECTIVE: OBJECTIVE 22 - Delete Unused Infrastructure Files

### The Problem
83KB of dead code in infrastructure layer that's never called

### Files to Delete
- `src/ai/infrastructure/cllm_backprop.c` (22KB) - No calls found
- `src/ai/infrastructure/cllm_loss.c` (30KB) - Only used by unused backprop
- `src/ai/infrastructure/cllm_training_loop.c` (31KB) - Defined but never called
- `include/ai/cllm_backprop.h` - Header for unused backprop
- `include/ai/cllm_training_loop.h` - Header for unused training loop

### Tasks
- [ ] Delete unused .c files
- [ ] Delete unused .h files
- [ ] Update Makefile to remove deleted files
- [ ] Test build
- [ ] Commit changes

---

## 📋 NEXT OBJECTIVES (After 21)

**OBJECTIVE 22:** Delete unused infrastructure files (83KB)
**OBJECTIVE 25:** Fix 78 build warnings
**OBJECTIVE 23:** Remove misleading qualifiers
**OBJECTIVE 24:** Investigate duplicates

---

## 📊 STATUS

- ✅ OBJECTIVE 2B: Remove legacy loss functions (COMPLETE)
- ✅ OBJECTIVE 2C: Rename crystalline to default (COMPLETE)
- ✅ Architecture analysis (COMPLETE)
- ✅ SECONDARY_OBJECTIVES updated (COMPLETE)
- 🔄 OBJECTIVE 21: Fix simple_loss naming (IN PROGRESS)