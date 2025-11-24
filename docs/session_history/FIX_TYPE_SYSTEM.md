# Critical Type System Issues Found

## Problem 1: Type Name Collision

**Issue**: Two different structs both named `CrawlerState`:
1. Internal struct in `crawler_core.c` (the actual crawler implementation)
2. Wrapper struct in `crawler_api.c` (the high-level API)

**Header declares**:
```c
typedef struct CrawlerStateInternal CrawlerStateInternal;  // Forward declaration
typedef struct CrawlerState CrawlerState;                  // Forward declaration
```

**But crawler_core.c defines**:
```c
typedef struct {
    char data_dir[1024];
    // ... internal fields
} CrawlerState;  // WRONG NAME - should be CrawlerStateInternal
```

**And crawler_api.c defines**:
```c
struct CrawlerState {
    char data_dir[1024];
    // ... wrapper fields
};  // CORRECT NAME
```

## Problem 2: Function Signature Mismatch

**Header declares**:
```c
CrawlerStateInternal* crawler_internal_init(...);
```

**But crawler_core.c implements**:
```c
CrawlerState* crawler_internal_init(...) {  // Returns wrong type!
```

## Required Fixes

### Fix 1: Rename internal struct in crawler_core.c
Change all occurrences of `CrawlerState` to `CrawlerStateInternal` in:
- `src/crawler/crawler_core.c`

### Fix 2: Update function signatures
Ensure `crawler_internal_init` and `crawler_internal_cleanup` use `CrawlerStateInternal*`

### Fix 3: Update internal function calls
All internal functions in `crawler_core.c` should use `CrawlerStateInternal*`

## Impact

This type confusion could cause:
- Compilation warnings/errors
- Runtime crashes due to struct size mismatches
- Undefined behavior when casting between types