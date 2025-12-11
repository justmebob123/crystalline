# OBJECTIVE 2E: Crystalline Math Everywhere - Integration Plan

## Current Status

### ✅ Completed
- **Math Library**: ZERO math.h usage (Phase 7.6 complete)
- **Crystalline Infrastructure**: `prime_float_math.h` with ALL math.h replacements exists
- **Test Results**: 268/271 (98.9%) passing

### 🎯 Remaining Work
- **56 files** still have `#include <math.h>`
- **357 files** total use math.h functions (many already use prime_* alongside)

## Available Replacement Functions

The `prime_float_math.h` library provides complete replacements:

### Basic Functions
- `prime_sqrt()` / `prime_sqrtf()` → replaces `sqrt()`
- `prime_exp()` / `prime_expf()` → replaces `exp()`
- `prime_log()` / `prime_logf()` → replaces `log()`
- `prime_pow()` / `prime_powf()` → replaces `pow()`
- `prime_fabs()` / `prime_fabsf()` → replaces `fabs()`

### Trigonometric Functions
- `prime_sin()` / `prime_sinf()` → replaces `sin()`
- `prime_cos()` / `prime_cosf()` → replaces `cos()`
- `prime_tan()` / `prime_tanf()` → replaces `tan()`
- `prime_atan()` / `prime_atanf()` → replaces `atan()`
- `prime_atan2()` / `prime_atan2f()` → replaces `atan2()`
- `prime_acos()` → replaces `acos()`

### Hyperbolic Functions
- `prime_tanh()` / `prime_tanhf()` → replaces `tanh()`

### Utility Functions
- `prime_floor()` → replaces `floor()`
- `prime_ceil()` → replaces `ceil()`
- `prime_round()` → replaces `round()`
- `prime_fmod()` / `prime_fmodf()` → replaces `fmod()`
- `prime_fmax()` / `prime_fmaxf()` → replaces `fmax()`
- `prime_fmin()` / `prime_fminf()` → replaces `fmin()`
- `prime_isnan()` / `prime_isnanf()` → replaces `isnan()`
- `prime_isinf()` / `prime_isinff()` → replaces `isinf()`
- `prime_log2()` → replaces `log2()`

### Constants
- `M_PI` → already defined in prime_float_math.h
- `M_E` → already defined in prime_float_math.h
- `INFINITY` → already defined
- `NAN` → already defined

## Integration Strategy

### Phase 1: Automated Replacement (2-3 hours)
Create a script to automatically replace math.h functions:

```bash
#!/bin/bash
# replace_math_functions.sh

for file in $(find crystalline algorithms recovery -name "*.c" -o -name "*.h"); do
    # Skip if file doesn't use math.h
    grep -q "#include <math.h>" "$file" || continue
    
    # Replace include
    sed -i 's/#include <math\.h>/#include "prime_float_math.h"/g' "$file"
    
    # Replace function calls (double precision)
    sed -i 's/\bsqrt(/prime_sqrt(/g' "$file"
    sed -i 's/\bexp(/prime_exp(/g' "$file"
    sed -i 's/\blog(/prime_log(/g' "$file"
    sed -i 's/\bpow(/prime_pow(/g' "$file"
    sed -i 's/\bfabs(/prime_fabs(/g' "$file"
    sed -i 's/\bsin(/prime_sin(/g' "$file"
    sed -i 's/\bcos(/prime_cos(/g' "$file"
    sed -i 's/\btan(/prime_tan(/g' "$file"
    sed -i 's/\btanh(/prime_tanh(/g' "$file"
    sed -i 's/\batan(/prime_atan(/g' "$file"
    sed -i 's/\batan2(/prime_atan2(/g' "$file"
    sed -i 's/\bacos(/prime_acos(/g' "$file"
    sed -i 's/\bfloor(/prime_floor(/g' "$file"
    sed -i 's/\bceil(/prime_ceil(/g' "$file"
    sed -i 's/\bround(/prime_round(/g' "$file"
    sed -i 's/\bfmod(/prime_fmod(/g' "$file"
    sed -i 's/\bfmax(/prime_fmax(/g' "$file"
    sed -i 's/\bfmin(/prime_fmin(/g' "$file"
    sed -i 's/\bisnan(/prime_isnan(/g' "$file"
    sed -i 's/\bisinf(/prime_isinf(/g' "$file"
    sed -i 's/\blog2(/prime_log2(/g' "$file"
    
    # Replace float precision functions
    sed -i 's/\bsqrtf(/prime_sqrtf(/g' "$file"
    sed -i 's/\bexpf(/prime_expf(/g' "$file"
    sed -i 's/\blogf(/prime_logf(/g' "$file"
    sed -i 's/\bpowf(/prime_powf(/g' "$file"
    sed -i 's/\bfabsf(/prime_fabsf(/g' "$file"
    sed -i 's/\bsinf(/prime_sinf(/g' "$file"
    sed -i 's/\bcosf(/prime_cosf(/g' "$file"
    sed -i 's/\btanf(/prime_tanf(/g' "$file"
    sed -i 's/\btanhf(/prime_tanhf(/g' "$file"
    sed -i 's/\batanf(/prime_atanf(/g' "$file"
    sed -i 's/\batan2f(/prime_atan2f(/g' "$file"
    sed -i 's/\bfmodf(/prime_fmodf(/g' "$file"
    sed -i 's/\bfmaxf(/prime_fmaxf(/g' "$file"
    sed -i 's/\bfminf(/prime_fminf(/g' "$file"
    sed -i 's/\bisnanf(/prime_isnanf(/g' "$file"
    sed -i 's/\bisinff(/prime_isinff(/g' "$file"
done
```

### Phase 2: Manual Review (4-6 hours)
1. Check for edge cases (macros, comments, etc.)
2. Verify include paths are correct
3. Handle special cases (e.g., `#ifdef` blocks)
4. Fix any compilation errors

### Phase 3: Build Verification (2-3 hours)
```bash
# Build each library
cd crystalline && make clean && make -j$(nproc) 2>&1 | tee build.log
cd ../algorithms && make clean && make -j$(nproc) 2>&1 | tee build.log
cd ../recovery && make clean && make -j$(nproc) 2>&1 | tee build.log

# Count warnings
grep -c "warning:" */build.log

# Fix all warnings
# Rebuild until zero warnings
```

### Phase 4: Testing (3-4 hours)
1. Run all existing tests
2. Verify no regressions
3. Check performance (should be similar or better)
4. Document any issues

### Phase 5: Documentation (1-2 hours)
1. Update AUDIT.md
2. Create completion report
3. Update todo.md
4. Commit changes

## Estimated Timeline

**Total: 12-18 hours (1.5-2 days)**

- Phase 1: 2-3 hours (automated)
- Phase 2: 4-6 hours (manual review)
- Phase 3: 2-3 hours (build verification)
- Phase 4: 3-4 hours (testing)
- Phase 5: 1-2 hours (documentation)

## Success Criteria

✅ Zero `#include <math.h>` in production code
✅ All files use `prime_float_math.h` instead
✅ Zero build warnings
✅ Zero build errors
✅ All tests passing
✅ No performance regressions

## Files to Process

### High Priority (Core AI/Training)
1. `crystalline/src/ai/cllm_attention.c`
2. `crystalline/src/ai/cllm_training_functions.c`
3. `crystalline/src/ai/cllm_layernorm.c`
4. `crystalline/src/ai/cllm_entropy_integration.c`

### Medium Priority (Platonic Geometry)
5. `crystalline/src/ai/platonic/*.c` (10 files)

### Lower Priority (Utilities)
6. `crystalline/src/geometry/clock_lattice.c`
7. Other utility files

### Algorithms & Recovery
8. `algorithms/src/*.c`
9. `recovery/src/*.c`

## Risk Assessment

### Low Risk
- Automated replacement is straightforward
- `prime_float_math.h` has complete coverage
- Many files already use both (easy to verify)

### Medium Risk
- Some files may have complex math expressions
- Performance-critical code needs benchmarking
- Edge cases in macros or preprocessor directives

### Mitigation
- Test incrementally (one library at a time)
- Keep backups before changes
- Run comprehensive tests after each library
- Benchmark performance-critical sections

## Next Steps

**Option A: Proceed with Integration** (RECOMMENDED)
- Start with Phase 1 (automated replacement)
- Process one library at a time
- Verify builds and tests incrementally

**Option B: Pilot Test First**
- Pick 5-10 representative files
- Manually convert and test
- Validate approach before full automation

**Option C: Defer Integration**
- Continue with other objectives
- Come back to this later

---

**Status**: Ready to proceed
**Branch**: audit (continue on feature branch)
**Priority**: 🔴 CRITICAL - Master Plan OBJECTIVE 2E