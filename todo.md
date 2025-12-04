# TODO - Training Tab Issues

## ✅ START Button Fixed (commit eaa3fa8)
- Buttons now receive BUTTONDOWN events correctly
- START button responds to clicks

## 🔧 FIXES IN PROGRESS

### Fix 1: Model Loading Failure ✅
**Root Cause:** Model manager uses lazy loading - models are registered but not loaded into memory
- Models have `is_accessible = false` until explicitly loaded
- `model_manager_acquire_write()` requires model to be accessible first

**Solution Applied:**
- Check if model is accessible before acquiring
- If not accessible, call `model_manager_reload()` to load from disk
- Then acquire write access for training
- Added clear status messages for each step

### Fix 2: Debug Output Flooding ✅
**Root Cause:** Excessive debug printf statements in crystalline UI elements
- List rendering: Every 60 frames
- Dropdown handling: Every 10 calls
- Checkbox clicks: Every click

**Solution Applied:**
- Removed list render debug output
- Removed dropdown handle debug output
- Removed checkbox click debug output
- Kept only button click debug (useful for testing)

## Next Steps
- [x] Build and verify zero errors/warnings ✅
- [ ] Commit fixes
- [ ] User test: Model loading and training start
- [ ] User test: Verify clean terminal output (no flooding)
- [ ] Address "no training context" error if it still occurs

## Build Status
- **Errors:** 0 ✅
- **Warnings:** 0 ✅
- **Branch:** feature/crystalline-ui-system