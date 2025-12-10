# Anchor Distribution Analysis - Key Findings

## Problem Identified

The Geometric Recovery Algorithm was generating uniform candidates because:

1. **Insufficient Anchor Count**: Only 100 anchors were being used in 13-dimensional space
2. **Sparse Coverage**: With 100 anchors in 13D, each anchor's influence region was massive
3. **Large Simplices**: The nearest 3 anchors formed huge triangles (sides ~1.3-1.8 units)
4. **Repeated Vertices**: Most search positions fell within the same large simplex
5. **Uniform Interpolation**: Interpolating between the same 3 anchors produced similar candidates

## Root Cause Analysis

### Anchor Distribution Statistics (100 anchors)
- **Centroid**: Well-centered around 0.5-0.7 in each dimension
- **Variance**: ~0.1-0.15 per dimension (stddev ~0.3-0.4)
- **Pairwise Distances**:
  - Min: 0.33 units
  - Max: 3.07 units
  - Avg: 1.77 units

### Real K Position Analysis
For test k = 0x12345678:
- **Nearest 3 anchors** (shared vertices):
  - Anchor 1: 0x21985E1B (distance: 1.05)
  - Anchor 2: 0x9EBE769B (distance: 1.29)
  - Anchor 3: 0x96BD4764 (distance: 1.45)

- **Simplex Size** (distances between vertices):
  - Vertex 1 <-> Vertex 2: 1.53 units
  - Vertex 1 <-> Vertex 3: 1.80 units
  - Vertex 2 <-> Vertex 3: 1.33 units

### Why First 4 Bytes Were Identical

The 3 shared vertices had values:
- 0x21985E1B
- 0x9EBE769B
- 0x96BD4764

When interpolating between these three 32-bit values, the **high-order bytes dominated** the weighted average, causing all candidates to have similar leading bytes.

## Solution Implemented

### Increased Anchor Count to 10,000

**Memory Requirements:**
- 10,000 anchors × 13 dimensions × 2 position arrays = 260,000 doubles (~2MB)
- Plus BIGNUM and EC_POINT structures
- **Total**: ~5-10MB (well within limits)

**Performance:**
- Context creation: 0.023 seconds
- Adding 10,000 anchors: 3.3 seconds
- Initialization: 4.4 seconds
- **Total setup**: 7.7 seconds

### Results with 10,000 Anchors

**Before (100 anchors):**
```
Nearest anchors: 10, 2, 5 (same repeatedly)
Candidates: All had identical first 4 bytes
```

**After (10,000 anchors):**
```
Nearest anchors: 94, 172, 193
Nearest anchors: 172, 193, 211
Nearest anchors: 172, 211, 383
```

**Candidate Diversity:**
- `777A82C4936F1CB882677A14BE2A6DDF`
- `09C5A8BF5FD188A364D77EECEF40B3A3`
- `4FD94FAB473B6177E5BDD5AC2841C6AC`
- `6DDD3D532972EDF749E33E0A1DD24755`

## Remaining Observations

### Distance Uniformity
Even with 10,000 anchors, distances are still uniform (5.0, 5.0, 5.0). This suggests:

1. **Possible Issues:**
   - The 13D mapping might be collapsing dimensions
   - Anchors might be clustering in certain regions
   - The distance metric might need adjustment

2. **Further Investigation Needed:**
   - Analyze the actual distribution of 10,000 anchors in 13D space
   - Verify the 13D mapping isn't creating artificial structure
   - Check if the search pattern is exploring diverse enough regions

### Next Steps

1. **Verify 13D Mapping**: Ensure the mapping from 32-bit values to 13D space creates uniform distribution
2. **Analyze Search Pattern**: Confirm the expanding spiral search covers diverse regions
3. **Distance Metrics**: Consider alternative distance metrics (Manhattan, Chebyshev)
4. **Anchor Generation**: Ensure random anchor generation produces uniform coverage
5. **Dimensionality**: Consider if 13D is sufficient or if higher dimensions are needed

## Conclusion

The primary issue was **insufficient anchor count**. With only 100 anchors in 13-dimensional space, the algorithm couldn't achieve the resolution needed for diverse candidate generation. Increasing to 10,000 anchors immediately improved candidate diversity, though further optimization may be needed for the distance metrics and search patterns.

The key insight: **In high-dimensional spaces, anchor density matters exponentially**. The "curse of dimensionality" means that sparse sampling in 13D leaves huge gaps that prevent effective interpolation.