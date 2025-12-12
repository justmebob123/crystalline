# TODO: Investigate PHP Extension Segmentation Fault

## Current Status
✅ Extension loads successfully
✅ O(1) prime generation works
✅ Primality testing works  
✅ C library prime_nth works perfectly (tested directly)
❌ PHP extension segfaults when calling crystalline_prime_nth()

## Key Finding
**The C library works fine!** I tested `prime_nth` directly in C and it works perfectly:
```
Prime #1: 2
Prime #2: 3
Prime #3: 5
...
Prime #10: 29
```

This means the issue is **in the PHP extension wrapper**, not the math library.

## Root Cause Analysis
The segfault is likely caused by one of these issues in the PHP wrapper:

1. **Memory Management Issue**
   - PHP's memory allocator vs C's malloc
   - The global rainbow table might not be compatible with PHP's memory model
   
2. **Thread Safety Issue**
   - PHP might be using ZTS (Zend Thread Safety)
   - The global rainbow table is not thread-safe
   
3. **Return Value Handling**
   - RETURN_LONG() macro might have issues with large uint64_t values
   - Possible overflow or type mismatch

4. **Module Initialization**
   - PHP_MINIT_FUNCTION is empty
   - Should initialize the rainbow table during module load

## Recommended Fix
Add proper initialization in PHP_MINIT_FUNCTION:

```c
PHP_MINIT_FUNCTION(crystalline_math)
{
    // Pre-initialize the rainbow table to avoid lazy init issues
    // This ensures the global state is set up before any PHP calls
    prime_nth(1);  // Force initialization
    return SUCCESS;
}
```

## Alternative Workaround
For now, users can avoid `crystalline_prime_nth()` and use:
- `crystalline_prime_generate_o1()` for O(1) generation
- `crystalline_prime_is_prime()` for primality testing

These functions work perfectly.

## Next Steps
1. [x] Add initialization in PHP_MINIT_FUNCTION
2. [x] Add error checking in crystalline_prime_nth wrapper
3. [ ] Commit and push fix
4. [ ] User needs to rebuild and test
5. [ ] Consider thread-safety if ZTS is enabled (future work)

## Files to Modify
- php/crystalline_math.c (PHP_MINIT_FUNCTION and error handling)