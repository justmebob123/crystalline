# Comprehensive Testing Guide - Crystalline CLLM Application

## Date: Session 3 - Post-Fix Testing

## Build Instructions

### 1. Clean Build
```bash
cd ~/code/math/crystalline
make clean
make
make app
```

**Expected Result**: 
- ✅ ZERO warnings
- ✅ ZERO errors
- ✅ Libraries created: libprimemath.a, libprimemath.so
- ✅ Executable created: app/hyper_prime_spiral

---

## Running the Application

### Launch Command
```bash
cd ~/code/math/crystalline
LD_LIBRARY_PATH=. app/hyper_prime_spiral
```

**Expected Output**:
```
═══════════════════════════════════════════════════════════════
        HYPER PRIME SPIRAL - PRIME MATHEMATICS EDITION
═══════════════════════════════════════════════════════════════
Controls:
  M - Cycle modes | F - Cycle folding | Tab - Switch tabs
  ...
═══════════════════════════════════════════════════════════════

Loaded font: /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf
Generated 1000 primes (old system)

=== Initializing Crystalline Lattice Cache ===
...
=== Lattice Cache Ready ===

=== Initializing CLLM System ===
...
=== CLLM System Ready ===
```

---

## Tab-by-Tab Testing

### TAB 1: Prime Spiral (Default)
**Purpose**: Visualize prime number patterns

**Tests**:
1. [ ] Application launches without crash
2. [ ] Prime spiral renders correctly
3. [ ] Mouse pan works (drag)
4. [ ] Zoom works (+/- keys)
5. [ ] Mode cycling works (M key)
6. [ ] Animation works (Space key)

**Expected**: Smooth visualization with interactive controls

---

### TAB 2: Calculator
**Purpose**: Prime number calculations

**Tests**:
1. [ ] Click tab to switch
2. [ ] Calculator interface renders
3. [ ] Input field accepts numbers
4. [ ] Calculations work
5. [ ] Results display correctly

**Expected**: Functional calculator with prime operations

---

### TAB 3: Spheres
**Purpose**: 3D sphere visualization

**Tests**:
1. [ ] Click tab to switch
2. [ ] 3D spheres render
3. [ ] Rotation works
4. [ ] Zoom works

**Expected**: 3D visualization of prime structures

---

### TAB 4: Prime Folding
**Purpose**: Prime folding patterns

**Tests**:
1. [ ] Click tab to switch
2. [ ] Folding visualization renders
3. [ ] Folding mode cycles (F key)

**Expected**: Folding pattern visualization

---

### TAB 5: Video Generator
**Purpose**: Video generation (placeholder)

**Tests**:
1. [ ] Click tab to switch
2. [ ] "Coming Soon" message displays

**Expected**: Placeholder message (not yet implemented)

---

### TAB 6: LLM (Language Model)
**Purpose**: CLLM inference and interaction

**Critical Tests**:

#### 6.1 Model Creation
1. [ ] Click "LLM" tab
2. [ ] Click "Create New" button
3. [ ] **VERIFY**: Model creates without crash
4. [ ] **VERIFY**: Status shows "Loaded" in green
5. [ ] **VERIFY**: Model info displays (vocab, layers, lattice points)

**Expected Output**:
```
Creating default CLLM model...
CLLM Configuration:
  Vocabulary Size:  10000
  Embedding Dim:    512
  Number of Layers: 6
  ...
✓ Model created
```

#### 6.2 Model Save (CRITICAL - Previously Crashed)
1. [ ] With model loaded, click "Save Model" button
2. [ ] **VERIFY**: No segmentation fault
3. [ ] **VERIFY**: Message "✓ Model saved: models/saved_model.cllm"
4. [ ] **VERIFY**: File exists: `ls -la models/saved_model.cllm`

**Expected**: Model saves successfully without crash

#### 6.3 Model Load
1. [ ] Click "Load" button
2. [ ] **VERIFY**: Attempts to load from multiple paths
3. [ ] **VERIFY**: Either loads successfully or shows "No model file found"

#### 6.4 Text Generation
1. [ ] Click in text input field
2. [ ] Type a prompt (e.g., "The prime numbers")
3. [ ] Adjust temperature slider (0.0 - 2.0)
4. [ ] Adjust max tokens slider (10 - 500)
5. [ ] Click "Generate" button
6. [ ] **VERIFY**: Output appears in output area
7. [ ] **VERIFY**: Can scroll output if long

#### 6.5 Lattice Visualization
1. [ ] With model loaded
2. [ ] **VERIFY**: 3D lattice points render on left side
3. [ ] **VERIFY**: Connections between points visible
4. [ ] **VERIFY**: Points colored based on properties

---

### TAB 7: Training
**Purpose**: Model training interface

**Critical Tests**:

#### 7.1 Directory Scanning
1. [ ] Click "Training" tab
2. [ ] Click "Scan Dir" button
3. [ ] **VERIFY**: Files from `data/training/` appear in list
4. [ ] **VERIFY**: At least `sample_training.txt` is visible

#### 7.2 Text Input Fields (CRITICAL - Previously Broken)
1. [ ] Click on "Learning Rate" input field
2. [ ] **VERIFY**: Field activates (cursor appears)
3. [ ] Type a number (e.g., "0.001")
4. [ ] **VERIFY**: Text appears in field
5. [ ] Press Enter
6. [ ] **VERIFY**: Field deactivates
7. [ ] **VERIFY**: Value updates in state

**Repeat for**:
- [ ] Epochs input field
- [ ] Batch Size input field

#### 7.3 File Selection
1. [ ] Click checkbox next to training file
2. [ ] **VERIFY**: File becomes selected (checkbox checked)
3. [ ] Click "Select All" button
4. [ ] **VERIFY**: All files selected

#### 7.4 Training Execution
1. [ ] Select at least one training file
2. [ ] Set learning rate (e.g., 0.001)
3. [ ] Set epochs (e.g., 5)
4. [ ] Click "Start Training" button
5. [ ] **VERIFY**: Training starts (progress bar appears)
6. [ ] **VERIFY**: Metrics update (loss, tokens/sec, time)
7. [ ] **VERIFY**: Loss graph updates
8. [ ] Click "Stop Training" button
9. [ ] **VERIFY**: Training stops

#### 7.5 Checkpoint Management
1. [ ] With training active or complete
2. [ ] Click "Save Checkpoint" button
3. [ ] **VERIFY**: Checkpoint saved to `checkpoints/`
4. [ ] Click "Load Checkpoint" button
5. [ ] **VERIFY**: Checkpoint loads successfully

---

### TAB 8: Research
**Purpose**: Research data browser

**Critical Tests**:

#### 8.1 Directory Scanning (CRITICAL - Previously Failed)
1. [ ] Click "Research" tab
2. [ ] **VERIFY**: No "Could not open directory" error
3. [ ] **VERIFY**: Files from `docs/research/` appear
4. [ ] **VERIFY**: At least README.md and sample_data.txt visible

#### 8.2 Search Input (CRITICAL - Previously Broken)
1. [ ] Click on search input field
2. [ ] **VERIFY**: Field activates
3. [ ] Type search query (e.g., "sample")
4. [ ] **VERIFY**: Text appears in field
5. [ ] Press Enter
6. [ ] **VERIFY**: Search executes (files filtered)

#### 8.3 File Viewing
1. [ ] Click on a file in the list
2. [ ] **VERIFY**: File content displays on left side
3. [ ] **VERIFY**: Can scroll content with mouse wheel
4. [ ] Click different file
5. [ ] **VERIFY**: Content updates

#### 8.4 Sorting
1. [ ] Click "Sort: Name" button
2. [ ] **VERIFY**: Sort mode cycles (Name → Size → Date → Type)
3. [ ] **VERIFY**: Files re-sort accordingly
4. [ ] Click sort button again
5. [ ] **VERIFY**: Sort order reverses (ascending ↔ descending)

---

### TAB 9: Benchmark
**Purpose**: Performance benchmarking

**Tests**:

#### 9.1 Benchmark Execution
1. [ ] Click "Benchmark" tab
2. [ ] Press 'B' key
3. [ ] **VERIFY**: Benchmarks start running
4. [ ] **VERIFY**: Progress bar shows progress
5. [ ] **VERIFY**: Results appear in list
6. [ ] **VERIFY**: Metrics update (avg time, tokens/sec, etc.)

#### 9.2 Results Management
1. [ ] With benchmark results visible
2. [ ] Press 'C' key
3. [ ] **VERIFY**: Results clear
4. [ ] Run benchmarks again
5. [ ] **VERIFY**: New results appear

#### 9.3 System Metrics
1. [ ] **VERIFY**: CPU usage displays
2. [ ] **VERIFY**: Memory usage displays
3. [ ] **VERIFY**: Metrics update periodically

---

## Known Issues (To Be Addressed)

### 1. Model Save Warning
**Issue**: "Warning: Model has no weights to save"
**Cause**: Newly created models don't have initialized weights yet
**Impact**: Model saves but with zero weights
**Fix Needed**: Initialize weights during model creation
**Workaround**: Train model before saving

### 2. Model Load Paths
**Issue**: Tries multiple paths before finding model
**Cause**: Multiple fallback paths configured
**Impact**: Console spam but works correctly
**Fix Needed**: Consolidate to single path or better path resolution

### 3. Text Input Validation
**Issue**: No validation on numeric inputs
**Cause**: Simple string append without validation
**Impact**: Can enter invalid values
**Fix Needed**: Add numeric validation and bounds checking

---

## Success Criteria

### Build
- [x] Compiles with ZERO warnings
- [x] Compiles with ZERO errors
- [x] All libraries created
- [x] Application executable created

### Launch
- [ ] Application launches without crash
- [ ] All initialization completes
- [ ] UI renders correctly

### LLM Tab
- [ ] Model creation works
- [ ] Model save works (no crash) ✅ FIXED
- [ ] Model load works
- [ ] Text generation works
- [ ] Lattice visualization works

### Training Tab
- [ ] Directory scanning works ✅ FIXED
- [ ] Text inputs work ✅ FIXED
- [ ] File selection works
- [ ] Training starts/stops
- [ ] Metrics update

### Research Tab
- [ ] Directory scanning works ✅ FIXED
- [ ] Search input works ✅ FIXED
- [ ] File viewing works
- [ ] Sorting works

### Benchmark Tab
- [ ] Benchmarks run
- [ ] Results display
- [ ] Metrics update

---

## Testing Commands

### Quick Test Script
```bash
# Build
cd ~/code/math/crystalline
make clean && make && make app

# Verify build
ls -la libprimemath.so app/hyper_prime_spiral

# Verify directories
ls -la docs/research/ data/training/ models/ checkpoints/

# Run application
LD_LIBRARY_PATH=. app/hyper_prime_spiral
```

### Manual Testing Checklist
1. Launch application
2. Navigate through all tabs (Tab key or click)
3. Test each tab's functionality
4. Verify no crashes
5. Verify all inputs work
6. Verify all buttons work
7. Close application cleanly (Q key)

---

## Reporting Issues

If you encounter any issues during testing:

1. **Note the exact steps** to reproduce
2. **Copy any error messages** from console
3. **Note which tab** you were on
4. **Note what you clicked/typed** before the issue
5. **Check if application crashed** or just showed error

---

## Next Steps After Testing

1. **If all tests pass**: Continue with Phase 2 Day 2 implementation
2. **If issues found**: Fix issues before continuing
3. **Document any workarounds** needed
4. **Update this guide** with findings

---

**Status**: ✅ FIXES APPLIED, READY FOR TESTING
**Build**: ✅ CLEAN (0 warnings, 0 errors)
**Date**: Session 3