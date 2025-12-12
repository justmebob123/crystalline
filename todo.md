# TODO: Debug PHP Extension Segfault with GDB

## ROOT CAUSE FOUND! ✅

### The Problem: Circular Dependency
1. `prime_nth()` → `ensure_rainbow_coverage()`
2. `ensure_rainbow_coverage()` → `rainbow_populate_to_prime()`
3. `rainbow_populate_to_prime()` → `prime_next()`
4. `prime_next()` → `ensure_rainbow_coverage()` ← **CIRCULAR!**

This creates infinite recursion or memory corruption during initialization.

### The Fix
Modify `prime_next()` to NOT call `ensure_rainbow_coverage()` during table population.
Add a flag to track if we're currently populating the table.

## Steps
1. [x] Read Master Plan
2. [x] Identify exact cause (circular dependency)
3. [x] Implement fix with population flag
4. [x] Test C library (works perfectly)
5. [ ] User needs to rebuild PHP extension
6. [ ] Commit and push

## Fix Applied
Added `g_populating_table` flag to prevent circular recursion:
- When `ensure_rainbow_coverage()` starts populating, it sets the flag
- If called recursively during population, it returns immediately
- This breaks the circular dependency chain