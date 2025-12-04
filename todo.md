# Training Tab Complete Redesign with Crystalline UI System

## CRITICAL UNDERSTANDING
- **RECREATE THE ENTIRE TAB** using Crystalline UI system (circular buttons, proper panels)
- **DO NOT use old broken UI code** - start fresh with new system
- **PRESERVE functionality** by studying the BEFORE screenshot to understand what data to display
- **DO NOT modify sphere_visualization.c** - just call it
- **KEEP 2D/3D toggle** working

## What I Need to Recreate (from BEFORE screenshot)

### Left Panel - Visualization Area
- [ ] "TRAINING VISUALIZATION" header
- [ ] Training metrics: "Epoch: 0 / 10 | Loss: 0.0000 | Best: inf"
- [ ] Batch progress display
- [ ] Throughput display
- [ ] Time estimates (elapsed, remaining, ETA)
- [ ] Progress bar with percentage
- [ ] 3D sphere visualization (call existing sphere_visualization.c)
- [ ] "Switch to 2D" button (small, functional)
- [ ] Framework status panel (right side of sphere)
- [ ] Performance metrics panel
- [ ] Thread states legend

### Right Panel - Controls (REDESIGN with Crystalline UI)
- [ ] Model selector dropdown
- [ ] Configuration sliders (Batch Size, Sequence Length, Epochs, Learning Rate)
- [ ] Model info display
- [ ] Status indicators
- [ ] Training data file list
- [ ] Action buttons using CIRCULAR Crystalline UI buttons
- [ ] Scrollbar for panel

## Implementation Plan

### Phase 1: Study Crystalline UI System
- [ ] Examine how other tabs use circular buttons
- [ ] Understand the panel border system
- [ ] Study the layout system

### Phase 2: Create New Training Tab Structure
- [x] Remove ALL old UI code
- [x] Create new panel structure with proper borders
- [x] Use golden ratio layout (61.8% / 38.2%)

### Phase 3: Implement Left Panel (Visualization)
- [x] Create visualization panel with Crystalline borders
- [x] Add training metrics display
- [x] Add progress indicators
- [x] Integrate sphere_visualization.c (DO NOT MODIFY IT)
- [x] Add 2D/3D toggle button
- [x] Add framework status panel
- [x] Add performance metrics panel

### Phase 4: Implement Right Panel (Controls)
- [x] Create control panel with Crystalline borders
- [x] Add model selector
- [x] Add configuration sliders
- [x] Add file list display
- [x] Add CIRCULAR action buttons (SCAN, SELECT, START, SAVE, LOAD)
- [x] Proper button sizing (not oversized, not tiny)

### Phase 5: Wire Up Functionality
- [x] Connect all buttons to backend functions
- [x] Ensure training thread integration works
- [x] Ensure metrics update correctly
- [x] Test all interactive elements

### Phase 6: Testing
- [x] Build and verify no errors
- [ ] Test all buttons work (needs runtime testing)
- [ ] Test training actually runs (needs runtime testing)
- [ ] Verify all data displays correctly (needs runtime testing)

## Key Requirements
1. Use Crystalline UI circular buttons (like AFTER screenshot)
2. Proper panel borders (3px outer, 2px inner)
3. ALL functionality from BEFORE screenshot preserved
4. Clean, professional layout
5. DO NOT touch sphere_visualization.c
6. Keep 2D/3D toggle working