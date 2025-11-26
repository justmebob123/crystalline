# Threading Architecture Fix Plan

## Current Issues

### 1. Hardcoded Thread Count
**Location**: `src/ai/cllm_training_threaded.c:204`
```c
system->thread_system = threads_create(2);  // ❌ Hardcoded to 2 levels = 13 threads
```

**Impact**:
- Level 0: 1 root sphere
- Level 1: 12 kissing spheres  
- Total: 13 threads (only uses 13 of 64 available cores!)

### 2. Hardcoded Array Sizes
**Location**: `src/ai/cllm_training_threaded.c:51`
```c
SphereTrainingContext* sphere_contexts[12];  // ❌ Fixed array size
```

**Impact**: Cannot scale beyond 12 worker spheres

### 3. Hardcoded Loop Limits
Multiple locations with `for (int i = 0; i < 12; i++)`

## Solution: Dynamic Thread Allocation

I'll implement a flexible system that:
1. Accepts thread count as parameter
2. Calculates optimal hierarchy levels
3. Scales from 1 to N threads
4. Maintains kissing spheres model as optimization

## Implementation Starting...