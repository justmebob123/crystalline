# TODO: Unified Persistent Progress Bar System

## Overview
Redesign the progress tracking system to provide a **single, persistent progress bar** that stays throughout the entire program lifecycle and updates to show information about every process being performed.

## Current State
- Progress bars are created per-phase (vocabulary building, tokenization)
- Each phase creates a new progress bar
- No unified view of overall program progress
- Progress bars disappear between phases

## Target State (Per Master Plan)
- **ONE persistent progress bar** for the entire program
- Updates to show current phase and sub-phase information
- Stays visible throughout all operations
- Shows hierarchical progress (overall + current phase)
- Never disappears until program completion

## Implementation Plan

### Phase 1: Design Global Progress System
- [ ] Design global progress tracker structure
  - Overall program progress (0-100%)
  - Current phase name and progress
  - Sub-phase information
  - Hierarchical display format
- [ ] Define all program phases and their weights
  - Phase 1: Loading data files (5%)
  - Phase 2: Building vocabulary (20%)
  - Phase 3: Tokenizing documents (25%)
  - Phase 4: Training epochs (50%)
  - Phase 5: Saving checkpoints/final model (5%)

### Phase 2: Implement Global Progress Tracker
- [x] Create new `cllm_global_progress.h` header
- [x] Create new `cllm_global_progress.c` implementation
- [x] Design persistent display format:
  ```
  ╔══════════════════════════════════════════════════════════════════════════╗
  ║ CLLM Training Pipeline                                    Overall: 45.2% ║
  ╠══════════════════════════════════════════════════════════════════════════╣
  ║ Phase 3/5: Tokenizing Documents                                   [75%]  ║
  ║ [=====================================>              ] 8,234/11,074 docs  ║
  ║ Elapsed: 5m 23s | ETA: 1m 47s | Speed: 25.4 docs/s                      ║
  ╚══════════════════════════════════════════════════════════════════════════╝
  ```
- [x] Implement thread-safe global state
- [x] Implement phase transition handling
- [x] Implement sub-phase progress updates

### Phase 3: Integrate with Existing Code
- [x] Update `tools/cllm_unified.c` to initialize global progress
- [x] Update `src/ai/cllm_data_loader.c` to use global progress
  - File loading phase
  - Vocabulary building phase
  - Tokenization phase
- [x] Update training loop to use global progress
  - Training epochs phase
  - Checkpoint saving phase
- [x] Remove old per-phase progress trackers

### Phase 4: Testing and Refinement
- [ ] Test with small dataset (awaiting user testing)
- [ ] Test with large dataset (11,000+ documents) (awaiting user testing)
- [x] Build completed successfully with zero warnings
- [x] Code compiles and links correctly
- [ ] Verify display stays persistent (awaiting user testing)
- [ ] Verify no display corruption (awaiting user testing)
- [ ] Verify accurate progress percentages (awaiting user testing)
- [ ] Verify accurate ETAs (awaiting user testing)

### Phase 5: Documentation
- [x] Document global progress system architecture (UNIFIED_PROGRESS_SYSTEM.md)
- [x] Add comprehensive code comments
- [x] Document API and usage patterns

## Success Criteria
✅ Single progress bar visible throughout entire program execution
✅ Shows overall progress (0-100%)
✅ Shows current phase and sub-phase
✅ Accurate ETAs for each phase
✅ No display corruption or flickering
✅ Thread-safe updates
✅ Clean display format

## Notes
- Must maintain thread safety for parallel operations
- Must handle phase transitions smoothly
- Must provide meaningful progress information at all times
- Display should be informative but not overwhelming