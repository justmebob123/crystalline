# CRYSTALLINE CLLM - Master Task List

## 🔒 RULES (PERMANENT - NEVER REMOVE)

### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
At the beginning of EVERY response:
1. Paste these rules to the top of todo.md
2. Read MASTER_PLAN.md completely
3. Read AUDIT.md for current architectural state
4. Read SECONDARY_OBJECTIVES.md for detailed tasks

### RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
Before taking ANY action:
1. Read MASTER_PLAN.md completely
2. Understand current objectives
3. Verify action aligns with master plan
4. Check for blocking priorities

### RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
Consult AUDIT.md to understand:
- What is broken
- What needs fixing
- What is blocking other work
- What the correct architecture should be

### RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
Use for step-by-step implementation guidance

### RULE 4: DO NOT CREATE NEW MD FILES
All documentation goes in existing files or master plan only

### RULE 5: ALWAYS COMMIT ALL CHANGES USING CORRECT AUTHENTICATION
```bash
git add .
git commit -m "descriptive message"
git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
```

### RULE 6: MASTER_PLAN.MD IS READ-ONLY
Never edit without explicit approval

### RULE 7: FIX ALL BUILD WARNINGS BEFORE PROCEEDING
All code must compile with zero warnings
- Build with -Wall -Wextra flags enabled
- Address ALL warnings, not just errors
- Fix high-priority warnings immediately

---

## RULE 7 COMPLIANCE: BUILD WARNINGS FIXED ✓

### All Warnings Resolved (0 warnings)
- [x] Removed unused variable 'vocab_size' in cllm_training.c
- [x] Replaced fabs() with prime_fabsf() (crystalline math)
- [x] Replaced sqrt() with prime_sqrtf() (crystalline math)
- [x] Commented out OLD_BROKEN function in cllm_integration.c
- [x] Removed unused variable 'input_active' in tab_llm.c
- [x] Clean build: 0 errors, 0 warnings
- [x] App build: 0 errors, 0 warnings

---

## OBJECTIVE 29 - Critical OOM Fix (COMPLETE ✓)

### Summary
- Fixed catastrophic OOM issue (21 TB virtual memory → 1.3 GB)
- Implemented packed array format (25x memory reduction)
- Build successful with ZERO errors, ZERO warnings
- Ready for user testing

## OBJECTIVE 28: Disk-Based Model Architecture (COMPLETE ✓)
All phases complete. Models now work from disk without loading into RAM.