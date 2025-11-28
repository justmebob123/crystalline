# TODO: CRITICAL Threading and UX Issues - PROPER ANALYSIS

## CRITICAL USER FEEDBACK - PREVIOUS SOLUTION WAS WRONG

**User's Correct Points:**
1. 12-fold symmetry is about DATA STRUCTURE, not thread count
2. Should work with ANY number of cores (1-N), not fixed at 13+
3. Threads should rotate through the 12 symmetry positions
4. Single small file training MUST work
5. Progress bar stuck at 85% - THIS is the actual bug
6. No loading screen during initialization
7. Should support --threads 0 for auto-detect
8. Batch size/epochs should be flexible regardless of dataset size

## Phase 1: Identify ACTUAL Bugs
- [ ] Find where progress bar gets stuck at 85%
- [ ] Identify what happens during "Initializing 12 threads"
- [ ] Find the actual hang point (not theoretical)
- [ ] Check if it's in dataset creation, model init, or thread spawn
- [ ] Analyze actual user logs/output

## Phase 2: Fix Thread Architecture
- [ ] Threads should be independent of symmetry groups
- [ ] N threads (1 to CPU count) should rotate through 12 symmetry positions
- [ ] Each thread processes batches from ANY symmetry group
- [ ] Remove fixed 13+ thread requirement
- [ ] Implement thread rotation through symmetry positions

## Phase 3: Fix Batch Processing
- [ ] Allow batch_size=1 for tiny datasets
- [ ] Allow seq_len as small as needed
- [ ] Create batches even from single lines
- [ ] Handle datasets smaller than one full batch
- [ ] Pad incomplete batches properly

## Phase 4: Fix Progress Reporting
- [ ] Add detailed progress during dataset creation
- [ ] Add progress during vocabulary building
- [ ] Add progress during model initialization
- [ ] Add progress during thread spawning
- [ ] Fix the 85% stuck issue

## Phase 5: Add Loading Screen
- [ ] Create initialization loading screen
- [ ] Show progress for each initialization step
- [ ] Display what's currently happening
- [ ] Provide time estimates
- [ ] Handle errors gracefully

## Phase 6: Add --threads Option
- [ ] Add --threads parameter to train_model
- [ ] Support --threads 0 for auto-detect
- [ ] Support --threads N for explicit count
- [ ] Validate thread count (1 to CPU count)
- [ ] Update help text

## Phase 7: Test Real Scenarios
- [ ] Test with single 10-line file
- [ ] Test with 4-core CPU
- [ ] Test with --threads 1
- [ ] Test with --threads 0 (auto)
- [ ] Verify progress bar reaches 100%
- [ ] Verify training completes

## Phase 8: Revert Bad Solution
- [ ] Remove minimum dataset requirements
- [ ] Remove thread count validation that blocks training
- [ ] Keep only helpful warnings, not blocking errors
- [ ] Allow any dataset size
- [ ] Allow any thread count (1 to CPU)