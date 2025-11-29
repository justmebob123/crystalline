# TODO - CRYSTALLINE CLLM PROJECT

---

## 🔒 MANDATORY RULES (PASTE AT TOP OF EVERY RESPONSE)

### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
**HIGHEST PRIORITY RULE - MUST BE FOLLOWED WITH EVERY SINGLE RESPONSE**

At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

This creates a permanent loop ensuring rules are never forgotten.

### RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
**SECOND HIGHEST PRIORITY RULE**

Before taking ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Understand the current objectives
3. Verify your action aligns with the master plan
4. Check for any blocking priorities

This ensures all work follows the architectural design.

### RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
The AUDIT.md contains current architectural violations, required fixes, and implementation phases.

### RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
The SECONDARY_OBJECTIVES.md contains detailed implementation tasks and code examples.

---

## 🚨 CRITICAL PRIORITY - BLOCKS ALL OTHER WORK

### CRYSTALLINE LATTICE ABACUS NOT INTEGRATED

**Status:** 🔴 CRITICAL ARCHITECTURAL VIOLATION  
**Priority:** HIGHEST - BLOCKS ALL OTHER WORK  
**Reference:** See AUDIT.md for complete analysis

**The Problem:**
The Crystalline Lattice Abacus - the CORE mathematical foundation - IS NOT BEING USED ANYWHERE.

**Current State:**
- ❌ No abacus initialization
- ❌ CLLM uses isolated prime cache
- ❌ Crystalline sieve bypasses abacus
- ❌ No hierarchical abacus for threading
- ❌ Clock lattice separate from prime generation
- ❌ Multiple duplicate prime systems

**Required State:**
- ✅ Single global abacus instance
- ✅ All prime access through abacus
- ✅ Clock lattice integrated with abacus
- ✅ Hierarchical abacus for each thread
- ✅ Control thread manages global abacus

**Implementation Phases:**
See AUDIT.md Section 4 for complete details.

### Phase 1: Create Global Abacus System (4h) - CRITICAL
- [ ] Create global abacus instance in `app/cllm_integration.c`
- [ ] Initialize at program startup
- [ ] Load important primes (Stage 1 - instant)
- [ ] Generate first 10,000 primes (Stage 2 - non-blocking)
- [ ] Integrate crystalline sieve with abacus
- [ ] Make abacus the ONLY source of primes

### Phase 2: Integrate Abacus with CLLM (2h) - CRITICAL
- [ ] Remove `prime_cache` array from `cllm_pure_token.c`
- [ ] Update `crystalline_get_nth_prime()` to use abacus
- [ ] Update `init_prime_cache()` to use abacus
- [ ] Remove duplicate prime generation code
- [ ] Ensure all prime access goes through abacus

### Phase 3: Integrate Clock Lattice with Abacus (3h) - HIGH
- [ ] Add `clock_positions` array to `CrystallineAbacus` structure
- [ ] Add `sphere_coords` array to `CrystallineAbacus` structure
- [ ] Compute mappings when primes are added
- [ ] Update `abacus_add_prime()` to compute positions
- [ ] Provide accessor functions for positions

### Phase 4: Hierarchical Abacus for Threading (4h) - HIGH
- [ ] Create hierarchical abacus for each thread
- [ ] Link child abacus to parent (global) abacus
- [ ] Implement partition filtering (symmetry groups 0-11)
- [ ] Update thread creation to initialize abacus
- [ ] Ensure threads use their hierarchical abacus

### Phase 5: Control Thread (Node 0) Implementation (2h) - MEDIUM
- [ ] Create control thread (Node 0)
- [ ] Control thread manages global abacus
- [ ] Control thread generates new primes on demand
- [ ] Control thread coordinates 12 worker threads
- [ ] Control thread NEVER processes batches

---

## 📋 CURRENT WORK SESSION

**Focus:** Creating AUDIT.md and updating MASTER_PLAN.md with new rules

**Completed:**
- [x] Created comprehensive AUDIT.md document
- [x] Updated MASTER_PLAN.md with RULE 0 and RULE 1
- [x] Added references to AUDIT.md and SECONDARY_OBJECTIVES.md
- [x] Updating todo.md with rules at top

**Next Steps:**
- [ ] Commit changes to repository
- [ ] Begin Phase 1: Create Global Abacus System
- [ ] Test and validate implementation

---

## 📚 REFERENCE DOCUMENTS

- **MASTER_PLAN.md** - Complete architectural design and objectives
- **AUDIT.md** - Current architectural violations and required fixes
- **SECONDARY_OBJECTIVES.md** - Detailed implementation tasks
- **todo.md** - Current work tracking (this file)

---

**Last Updated:** Current session  
**Status:** Ready to begin Phase 1 implementation

