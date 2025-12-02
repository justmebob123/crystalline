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

---

## CRITICAL: OBJECTIVE 31 - Implement Proper Disk-Based Memory Architecture (IN PROGRESS)

### User's Correct Vision
The system should work like a filesystem:
- **Disk-based structure** - Gradients live on disk, not RAM
- **Memory-mapped access** - Only active working set in RAM
- **64-bit operations** - Working on small chunks at a time
- **Pointer-based** - Memory pointers map to disk locations
- **Pre-caching** - Optional for performance
- **99.9% memory reduction** - Most data on disk until needed

### Current Problem
I INCORRECTLY disabled layer gradients instead of implementing proper disk-based storage!

### Phase 1: Re-enable All Gradient Structures (COMPLETE ✓)
- [x] Re-enable attention gradient allocations
- [x] Re-enable feedforward gradient allocations
- [x] Re-enable layer norm gradient allocations
- [x] Re-enable master_weights allocation

### Phase 2: Implement Memory-Mapped Disk Storage (COMPLETE ✓)
- [x] Created bigfixed_mmap.h header
- [x] Created bigfixed_mmap.c implementation
- [x] Implemented mmap() for memory-mapped file access
- [x] Implemented lazy loading (OS handles paging)
- [x] Implemented write-back caching (msync)
- [x] Added pre-caching (madvise MADV_WILLNEED)
- [x] Added access pattern hints (MADV_SEQUENTIAL/RANDOM)
- [x] Added memory statistics (mincore)

### Phase 3: Convert Gradients to Disk-Based
- [ ] Convert main gradients to disk-backed
- [ ] Convert attention gradients to disk-backed
- [ ] Convert feedforward gradients to disk-backed
- [ ] Convert layer norm gradients to disk-backed
- [ ] Convert master_weights to disk-backed

### Phase 4: Optimize Access Patterns
- [ ] Implement chunk-based access (64-bit operations)
- [ ] Add spatial locality optimization
- [ ] Implement read-ahead for sequential access
- [ ] Add write coalescing

---

## Previous Objectives (Need Revision)
- OBJECTIVE 29: Packed arrays (partial solution)
- OBJECTIVE 30: Disabled gradients (WRONG APPROACH - need to revert)
- OBJECTIVE 28: Disk-based model architecture (foundation exists)