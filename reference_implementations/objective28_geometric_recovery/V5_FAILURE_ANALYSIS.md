# V5 Failure Analysis - December 10, 2024

## What Happened

**Expected**: 20% → 35-55% success rate
**Actual**: 20% → 2% success rate (10x WORSE!)

## The Problem

### Observation from Results

Looking at the failures:
- k=2 (radius=0.75) → recovered k=290 (error=288)
- k=7 (radius=0.75) → recovered k=295 (error=288)
- k=13 (radius=0.75) → recovered k=170 (error=157)

**Pattern**: All high-radius targets recover to WRONG k values with similar radius!

### Root Cause

**The combined error metric is broken!**

```c
// Current implementation
double combined = weights.angle_weight * angle_error + 
                 weights.radius_weight * radius_error;
```

**For high radius (≥0.75)**:
- angle_weight = 0.3
- radius_weight = 0.7

**This means**: Radius error is weighted 2.3x more than angle error!

**Result**: The search finds k values with matching radius but ignores angle!

### Why This Fails

**Example: k=2**
- Target: angle=222.49°, radius=0.75
- Recovered: k=290, angle≈??°, radius≈0.75

**The search found**: A k value with radius≈0.75 (good!) but completely wrong angle (bad!)

**Why**: With 70% weight on radius, the algorithm prioritizes radius matching over angle matching.

## The Fundamental Misunderstanding

### What We Thought

"Radius is a discriminating factor" → "Weight radius heavily in search"

### The Reality

**Radius is a FILTER, not a PRIMARY METRIC!**

**Correct approach**:
1. Use angle to find candidates (primary search)
2. Use radius to FILTER/RANK candidates (secondary filter)

**Wrong approach** (what we did):
1. Use combined angle+radius metric
2. Weight radius heavily
3. Find k with matching radius but wrong angle

## Why the Statistical Analysis Was Misleading

### What We Observed

- Success avg radius: 0.7750
- Failure avg radius: 0.2750
- Conclusion: "Radius is discriminating!"

### What We Missed

**Correlation ≠ Causation!**

**The real pattern**:
- High radius k values happen to be in better-covered regions (Q3, Q4)
- Low radius k values happen to be in poorly-covered regions (Q1)
- **Radius correlates with success, but doesn't CAUSE success!**

### The True Cause

**Q4 performs well (33.3%) because**:
- Near 0°/360° boundary
- Good anchor coverage (k=0 and k=275)
- NOT because of high radius!

**Q1 performs poorly (0%) because**:
- Poor anchor coverage
- Happens to have low radius k values
- NOT because of low radius!

## The Correct Interpretation

### Radius as a Consequence, Not a Cause

**High radius k values succeed because**:
1. They're in Q3/Q4 (better anchor coverage)
2. They're near boundaries (wraparound helps)
3. They happen to be in well-covered regions

**NOT because radius itself helps the search!**

### What Radius Actually Tells Us

**Radius is a PROPERTY of k**, not a search constraint!

- k=2: radius=0.75 (happens to be in Q3)
- k=5: radius=0.25 (happens to be in Q1)

**Radius doesn't help us FIND k** - it's just a characteristic of where k maps to!

## Why V2 Works Better

### V2 Approach (Angle-Only)

```c
// Find k that minimizes angle error
for each candidate k:
    angle_error = |target_angle - k_angle|
    if angle_error < best_error:
        best_k = k
```

**This works because**:
- Angle is the PRIMARY mapping: k → angle
- Multiple k values map to similar angles (242 wraps)
- But angle is still the best discriminator we have!

### V5 Approach (Combined)

```c
// Find k that minimizes combined error
for each candidate k:
    combined_error = 0.3 * angle_error + 0.7 * radius_error
    if combined_error < best_error:
        best_k = k
```

**This fails because**:
- Radius is weighted too heavily (0.7)
- Search finds k with matching radius but wrong angle
- Completely misses the target!

## The Lesson

### Statistical Correlation ≠ Causal Relationship

**We observed**: High radius → High success
**We concluded**: Use radius in search
**We should have concluded**: High radius k values are in better regions

### The Right Way to Use Radius

**Option 1: Post-filtering**
1. Use angle-only search to find top N candidates
2. Use radius to rank/filter candidates
3. Return best candidate

**Option 2: Confidence weighting**
1. Use angle-only search
2. Use radius to compute confidence
3. Don't use radius in search itself

**Option 3: Region identification**
1. Use radius to identify which region k is in
2. Use region-specific search strategies
3. Still search primarily by angle

## Next Steps

### What NOT to Do

❌ Don't use radius in combined error metric
❌ Don't weight radius heavily in search
❌ Don't assume correlation implies causation

### What TO Do

✅ Stick with angle-only search (v2)
✅ Use radius for post-filtering or confidence
✅ Focus on improving anchor coverage
✅ Analyze why Q4 performs better (anchor positions, not radius!)

## Conclusion

**V5 was based on a flawed interpretation of the data.**

**The real insight**: High radius k values succeed because they're in better-covered regions, NOT because radius helps the search.

**The correct approach**: Keep v2's angle-only search, improve anchor coverage and search strategy.

**Status**: V2 remains the best approach at 20% success rate.