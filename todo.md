# TODO: Fix PHP Extension Undefined Symbols

## Issue Identified
The PHP extension compiled successfully but has undefined symbols at runtime:
- `clock_map_prime` - WRONG (doesn't exist)
- `clock_validate_position` - WRONG (doesn't exist)

## Correct Function Names (from math/include/math/clock.h)
- `clock_map_prime_to_position` - Maps prime to clock position
- `clock_is_valid_position` - Validates clock position

## Tasks
1. [x] Identify the undefined symbols
2. [x] Check the correct function names in clock.h
3. [x] Fix php/crystalline_math.c to use correct function names
4. [ ] Rebuild and test the PHP extension
5. [ ] Commit and push the fix

## Changes Made
- Line 269: `clock_map_prime` → `clock_map_prime_to_position`
- Line 305: `clock_validate_position` → `clock_is_valid_position`

## Files to Fix
- php/crystalline_math.c (lines 269 and 305)