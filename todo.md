# TODO - CRYSTALLINE CLLM PROJECT

---

## 🔒 MANDATORY RULES (PASTE AT TOP OF EVERY RESPONSE)

### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
**HIGHEST PRIORITY RULE - MUST BE FOLLOWED WITH EVERY SINGLE RESPONSE**

At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state (if exists)
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

---

## 🎯 CORRECT UNDERSTANDING: Rainbow Table IS the Abacus

### The Fundamental Truth

The **PrimeRainbowTable** is the **fundamental representation of the crystalline lattice abacus**.

It is NOT just a visualization tool - it is:
- The **single source of truth** for all primes
- The **complete geometric representation** of the crystalline structure
- The **integration point** for clock lattice, sphere coordinates, and all geometric properties
- The **fundamental structure** used by all other systems

### Current State of Rainbow Table

**Structure exists but is EMPTY:**
```c
typedef struct {
    BigInt *prime;            // ✅ Can store primes
} RainbowEntry;

typedef struct PrimeRainbowNode {
    RainbowEntry entry;                    // ✅ Has entry
    struct PrimeRainbowNode **children;    // ✅ Tree structure
    int child_count;                       // ✅ Hierarchical
} PrimeRainbowNode;

typedef struct {
    PrimeRainbowNode *root;               // ❌ Currently NULL
    int count;                            // ❌ Currently 0
    bool is_stable;                       // ✅ Stability tracking
    double fold_progression[12];          // ✅ 12-fold structure
    double negative_space[12];            // ✅ Negative space
} PrimeRainbowTable;
```

**Functions exist but don't populate:**
- `rainbow_table_init()` - Initializes to empty
- `fast_prime_angle(prime_index)` - Uses clock lattice
- `fast_prime_radius(prime_index)` - Uses clock lattice
- `fast_prime_fold_coords(prime_index, ...)` - Uses clock lattice

**The Problem:**
- Rainbow table structure is complete
- Rainbow table functions work correctly
- But **NO PRIMES ARE ADDED** to the rainbow table
- The tree is empty (root=NULL, count=0)

---

## 🚨 CRITICAL PRIORITY - Make Rainbow Table the Complete Abacus

### Phase 1: Add Prime Storage to Rainbow Table ✅ COMPLETE

**Goal:** Populate the rainbow table with primes and make it the single source of truth

**Tasks:**
- [x] Add function: `rainbow_table_add_prime(BigInt* prime)`
  - Creates RainbowEntry with the prime
  - Creates PrimeRainbowNode
  - Adds to tree structure (hierarchical by symmetry group)
  - Increments count
  - Updates fold_progression and negative_space

- [x] Add function: `rainbow_table_get_prime(int index)`
  - Returns the nth prime from the table
  - Traverses tree to find prime at index

- [x] Add function: `rainbow_table_get_count()`
  - Returns count of primes in table

- [x] Add function: `rainbow_table_generate_primes(int count)`
  - Uses crystalline sieve to generate primes
  - Adds each prime to the table using `rainbow_table_add_prime()`
  - Computes clock position for each prime
  - Computes sphere coordinates for each prime

- [x] Add function: `rainbow_table_load_important_primes()`
  - Loads hard-coded important primes (sacred, Mersenne, etc.)
  - Adds them to the table

- [x] Added function declarations to `include/prime_rainbow.h`
- [x] Build successful with zero errors
- [x] Rainbow table now has complete prime storage capability

### Phase 2: Integrate Rainbow Table with CLLM

**Goal:** Make CLLM use rainbow table as the single source of primes

**Tasks:**
- [ ] Remove `prime_cache` array from `cllm_pure_token.c`
- [ ] Update `crystalline_get_nth_prime()` to call `rainbow_table_get_prime()`
- [ ] Update `init_prime_cache()` to call `rainbow_table_generate_primes()`
- [ ] Ensure all prime access goes through rainbow table
- [ ] Test model creation with rainbow table integration

### Phase 3: Architectural Understanding ✅ CLARIFIED

**CRITICAL DISCOVERY:** After deep audit, the architecture is actually CORRECT!

**Three Abacus Systems Serve Different Purposes:**

1. **PrimeRainbowTable** (prime_rainbow.c) ⭐ FUNDAMENTAL
   - Tree structure with geometric representation
   - Single source of truth for ALL primes
   - Stores primes with clock lattice integration
   - ✅ This IS the abacus

2. **CrystalAbacus** (crystal_abacus.c) ✅ KEEP FOR THREADING
   - Simple, fast, integer-only prime generation
   - Used by HierarchicalAbacus for threading
   - Used by CrystalAbacusBig for BigInt operations
   - ✅ CANNOT DELETE - needed for threading architecture

3. **CrystallineAbacus** (crystalline_abacus.c) ❓ OPTIONAL WRAPPER
   - Fast generation with clock lattice integration
   - Currently used only by app/cllm_integration.c
   - Can be simplified to thin wrapper or removed

**Next Steps:**
- [ ] Integrate crystalline sieve with rainbow table (100-1000x speedup)
- [ ] Update CLLM to use rainbow table
- [ ] Decide on CrystallineAbacus (keep as wrapper or remove)

---

## 📋 SECONDARY OBJECTIVES (After Rainbow Table Complete)

### Training Pipeline
- [ ] Verify crystalline training uses rainbow table
- [ ] Ensure kissing spheres uses rainbow table coordinates

### UI Integration
- [ ] Display rainbow table statistics in UI
- [ ] Show clock lattice positions from rainbow table
- [ ] Visualize rainbow table tree structure

### Documentation
- [ ] Document rainbow table as the fundamental abacus
- [ ] Document rainbow table API
- [ ] Document integration with clock lattice