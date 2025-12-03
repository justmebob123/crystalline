# COMPREHENSIVE UI FIX PLAN

## CRITICAL ISSUES IDENTIFIED FROM SCREENSHOTS:

### 1. Toggle Button Not Visible
**Problem:** Button drawn BEFORE sphere visualization, so spheres draw over it
**Fix:** Move button rendering AFTER sphere visualization

### 2. Sphere Visualization Cut Off
**Problem:** Bottom spheres (5, 6, 7) not visible
**Root Cause:** 
- sphere_viz_height calculation is wrong
- Arrangement radius too large for the bounds
- Center point calculation doesn't account for full circle
**Fix:** 
- Increase sphere_viz_height to 70% of content height
- Adjust arrangement radius to fit within bounds
- Center the visualization properly

### 3. Training Tab Layout Cramped
**Problem:** Right panel overlaps sphere visualization
**Fix:**
- Reduce sphere_viz_width to 60% (from 70%)
- Increase spacing between panels
- Make right panel narrower

### 4. LLM Tab Wrong Model Path
**Problem:** Looking for "data/models/default_model.cllm"
**Fix:**
- Use model manager to get first available model
- Don't hardcode "default_model"
- Auto-load first model on tab open

### 5. Models Tab Empty
**Problem:** No UI elements visible
**Fix:**
- Check if draw_models_tab() is being called
- Verify tab initialization
- Add debug output

## IMPLEMENTATION PRIORITY:

1. Fix sphere visualization cutoff (CRITICAL)
2. Fix toggle button visibility (HIGH)
3. Fix training tab layout (HIGH)
4. Fix LLM tab model loading (MEDIUM)
5. Fix models tab rendering (MEDIUM)