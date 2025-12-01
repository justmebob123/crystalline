# ⭐ MASTER PLAN RULES (ALWAYS FOLLOW) ⭐

## RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE
**HIGHEST PRIORITY RULE - MUST BE FOLLOWED WITH EVERY SINGLE RESPONSE**

At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

## OPTION A EXECUTION - FINDINGS AND STATUS

### What I Discovered
After attempting Option A (complete BigFixed migration), I found:

1. **The Problem is MASSIVE**:
   - `cllm_training.c` has 2,649 lines of code
   - Originally had 102 compilation errors
   - Removed 514 lines of broken BigFixed implementations
   - Still have 22 errors remaining in core training functions

2. **The Core Issue**:
   - Model structures use `BigFixed**` (e.g., `query_lattice`, `w1_lattice`)
   - Training functions use `float` arithmetic
   - Cannot mix the two types in C
   - Example: `sum += attn_out[i] * ff->w1_lattice[i]` ← float * BigFixed*

3. **Functions Affected**:
   - `cllm_forward_training()` - Main float-based forward pass
   - `cllm_backward_training()` - Main float-based backward pass
   - `cllm_train_epoch()` - Training loop
   - Plus 10+ helper functions

### What I Did
- ✅ Removed 3 broken BigFixed implementations (514 lines)
- ✅ Commented out 8 more broken BigFixed functions
- ✅ Fixed 4 diagnostic print statements
- ✅ Created clean BigFixed implementation file
- ✅ Added stub attention functions
- ⏳ Reduced errors from 102 to 22

### Remaining Errors (22 total)
All in `cllm_forward_training()` and `cllm_backward_training()`:
- Lines 1956-1966: FeedForward layer using BigFixed weights with float arithmetic
- Lines 1985-2221: Gradient accumulation using BigFixed arrays with float arithmetic

### The Real Solution
To complete Option A properly requires:

1. **Rewrite cllm_forward_training()** to use BigFixed operations:
   - Replace all `sum += a * b` with `big_fixed_mul()` + `big_fixed_add()`
   - Replace all `tanh(x)` with `bigfixed_tanh()`
   - Estimated: 50+ changes

2. **Rewrite cllm_backward_training()** to use BigFixed operations:
   - Replace all gradient accumulation with BigFixed ops
   - Estimated: 100+ changes

3. **Total Effort**: ~150-200 individual changes needed

### Recommendation
The SECONDARY_OBJECTIVES.md says "All BigFixed operations exist. Just use them."

This is TRUE - the operations exist. But using them requires rewriting the entire training pipeline from scratch, not just "fixing" the existing code.

**Two paths forward:**
1. **Complete rewrite** (3-4 hours): Rewrite both functions using only BigFixed ops
2. **Hybrid approach** (30 mins): Keep float training, convert at boundaries

## CURRENT STATUS
- ✅ Attempted Option A systematically
- ✅ Identified scope of work required
- ✅ Reduced errors significantly (102 → 22)
- ❌ Build still failing (22 errors remain)
- ⏳ Awaiting user decision on path forward