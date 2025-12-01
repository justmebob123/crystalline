# TODO - EXECUTING CRITICAL OBJECTIVES

## 🔒 RULES (PASTED FROM MASTER_PLAN)

### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

---

## 🎯 CURRENT OBJECTIVE: OBJECTIVE 21 - Fix Backwards "Simple Loss" Naming

### The Problem
`cllm_simple_loss.h` contains THE REAL crystalline GCD-based loss function.
The name "simple" is BACKWARDS - it implies a stub/simplified version, but it's the COMPLETE implementation!

### The Solution
Rename files to reflect reality:
- `include/ai/cllm_simple_loss.h` → `include/ai/cllm_loss.h` (THE REAL LOSS)
- `include/ai/cllm_loss.h` → `include/ai/cllm_tensor_loss.h` (Infrastructure Tensor API)

### Tasks

#### Step 1: Rename Infrastructure Loss First (to avoid conflict)
- [x] Rename `include/ai/cllm_loss.h` → `include/ai/cllm_tensor_loss.h`
- [ ] Update includes in `src/ai/infrastructure/cllm_loss.c`
- [ ] Update includes in `include/ai/cllm_backprop.h`
- [ ] Test build

#### Step 2: Rename Simple Loss to Loss
- [ ] Rename `include/ai/cllm_simple_loss.h` → `include/ai/cllm_loss.h`
- [ ] Update includes in `src/ai/cllm_training.c`
- [ ] Update includes in `src/ai/cllm_production.c`
- [ ] Update includes in `src/ai/cllm_training_threaded.c`
- [ ] Test build

#### Step 3: Verify and Commit
- [ ] Full clean build
- [ ] Verify zero errors
- [ ] Verify no new warnings
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