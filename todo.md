# TODO: Install GDB and Debug PHP Extension Properly

## Current Task
Install gdb/valgrind and use them to find the ACTUAL cause of the segfault

## Steps
1. [x] Read Master Plan
2. [x] Created debug script for user to run
3. [ ] User runs: ./debug_php_segfault.sh
4. [ ] Analyze gdb backtrace
5. [ ] Analyze valgrind output
6. [ ] Fix the REAL problem
7. [ ] Test and verify
8. [ ] Commit

## Debug Script Created
Created `debug_php_segfault.sh` that will:
- Install gdb and valgrind
- Run PHP under gdb to get full backtrace
- Run valgrind to detect memory issues
- Save outputs to /tmp/gdb_output.txt and /tmp/valgrind_output.txt

## User Action Required
Please run:
```bash
./debug_php_segfault.sh
```

Then share the output so I can identify the exact issue.