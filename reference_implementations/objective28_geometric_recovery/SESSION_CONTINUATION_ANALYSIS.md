# Session Continuation Analysis - December 10, 2024

## Current Status

**Baseline (v1)**: 6.0% success rate (3/50)
- Successful k values: 5, 199, 229
- All successes in specific regions

**Enhanced (v2)**: 4.0% success rate (2/50)  
- Successful k values: 5, 29
- Worse than baseline!

## Root Cause Analysis

### Problem 1: Anchor K Estimation Was Broken

**Original issue**: Used `k_estimate = angle / (π·φ)` which gave k ≈ 0-1 for all anchors
- Icosahedron vertices at 0°, 30°, 60°, ..., 330°
- All converted to k ≈ 0
- Search always centered near k=0

**Fix applied**: Map anchor index to k range
- 12 vertices → k values at 0, 25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275
- Better distribution across k space

**Result**: Improved from 2% to 4%, but still worse than baseline

### Problem 2: Search Ranges May Be Wrong

Looking at the successful cases:
- k=5 (angle 16.23°): In Q1, near anchor at k=0
- k=29 (angle 166.14°): In Q2, near anchor at k=150 or k=175
- k=199 (angle 357.98°): In Q4, near anchor at k=275
- k=229 (angle not shown): Likely in Q4

**Pattern**: Successes are near anchor points!

**Issue with v2**: 
- Q1 range: ±30 (too narrow for k=5 from anchor at k=0? No, should work)
- Q2 range: ±60 (should work for k=29 from anchor at k=150)
- But we're only getting 4% success

### Problem 3: The Baseline Uses Different Logic

Let me check what the baseline v1 actually does...

Looking at the code, v1 uses `recursive_torus_search_k` with 2 layers:
- Layer 1: ±50 range
- Layer 2: ±10 range

This is simpler and apparently more effective!

## Key Insight

**The v2 implementation is over-complicated!**

The quadrant-aware ranges and 4-layer search for Q1 are making things worse, not better.

**Why v1 works better**:
1. Simple 2-layer search: ±50, then ±10
2. Uses anchor to center search
3. Doesn't try to be too clever with quadrant-specific ranges

**Why v2 fails**:
1. Too many layers (3-4) with narrow ranges
2. Quadrant-specific logic adds complexity
3. May be missing the target due to narrow initial ranges

## Solution: Simplify v2

Instead of complex quadrant logic, use:
1. **Wider initial range**: ±100 (not ±30-100)
2. **Fewer layers**: 3 layers max
3. **Consistent across quadrants**: Don't over-optimize for Q1

## Recommended Changes

### Change 1: Widen All Initial Ranges

```c
// All quadrants use same configuration
ctx->q1_layers[0] = (SearchLayer){.range = 100, .step = 10, .max_iter = 50};
ctx->q1_layers[1] = (SearchLayer){.range = 25, .step = 2, .max_iter = 50};
ctx->q1_layers[2] = (SearchLayer){.range = 10, .step = 1, .max_iter = 50};
```

### Change 2: Use Same Config for All Quadrants

Don't try to be clever with quadrant-specific ranges. Use the same 3-layer search everywhere.

### Change 3: Better Anchor Selection

Instead of mapping index to k, use the actual angle-based distance to find which anchor is closest, then use a FIXED k grid:
- Anchor 0 (0°) → k=0
- Anchor 1 (30°) → k=25
- Anchor 2 (60°) → k=50
- ...
- Anchor 11 (330°) → k=275

But also add intermediate anchors at k=12, 37, 62, 87, etc. to fill gaps.

## Alternative Approach: Learn from v1

Instead of trying to improve v1 with complex logic, let's understand WHY v1 works:

**v1 Success Pattern**:
- k=5: angle 16.23° (Q1)
- k=199: angle 357.98° (Q4, near 0°)
- k=229: angle unknown (likely Q4)

All three are near the 0°/360° boundary!

**Hypothesis**: The anchor at 0° is the most effective anchor, and the ±50 range from v1 is perfect for catching k values near 0°.

**Test**: What if we just use a SINGLE anchor at k=0 with a wider range?

## Next Steps

1. **Simplify v2**: Use uniform 3-layer search with ±100, ±25, ±10
2. **Test simplified v2**: Should beat 6% baseline
3. **If still fails**: Go back to v1 and just increase the range to ±100, ±25, ±10
4. **Analyze why 0° anchor is so effective**: Maybe the π·φ metric has special properties near 0°

## Conclusion

**The v2 approach was over-engineered.**

Sometimes simpler is better. The baseline v1 with 2 layers and ±50 range works because:
1. It's simple
2. The range is wide enough
3. It doesn't try to be too clever

**Lesson learned**: Don't add complexity without testing each change incrementally.

**Next action**: Simplify v2 to match v1's simplicity but with slightly wider ranges.