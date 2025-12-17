# PERMANENT RULES - READ EVERY TIME

## GitHub Push Rules (CRITICAL)
1. **ALWAYS push to main branch directly** - NO feature branches
2. **ALWAYS use this exact command to push:**
   ```
   git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
   ```
3. **VERIFY push succeeded** - Check the output shows commits pushed
4. **If token expired** - Request user to refresh it
5. **After EVERY commit** - Push immediately to GitHub
6. **NO branches** - Work directly on main, commit, and push

## Current Task Status

### ✓ COMPLETED
- [x] Build system migration to new directory structure
- [x] Remove all "Babylonian" references from codebase
- [x] Direct math library integration (removed wrappers)
- [x] Fix all include paths in source files
- [x] Fix all include paths in Makefiles (CRITICAL - removed -I../include)
- [x] All libraries building successfully
- [x] Changes committed and pushed to GitHub

### Current Build Status
- ✓ libcrystallinemath.so/a - Built successfully
- ✓ libalgorithms.so/a - Built successfully  
- ✓ libcllm.so/a - Built successfully
- ✓ All changes pushed to GitHub (5 commits ahead resolved)

### Next Steps
Ready to continue with CLLM development, training, inference, and 88D design work.