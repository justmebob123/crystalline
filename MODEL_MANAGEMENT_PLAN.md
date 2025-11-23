# Model Management System - Implementation Plan

## Current Problems

1. **No model creation dialog** - App auto-loads or creates model without user choice
2. **No overwrite confirmation** - Training saves over existing model without asking
3. **No model selection** - Can't choose which model to load
4. **No model info** - Can't see model details before loading
5. **Poor UX** - User has no control over model lifecycle

## Required Functionality

### 1. Model Creation Dialog
**When**: On startup if no model exists, or when user clicks "New Model"
**Options**:
- Create new model with random weights
- Specify model parameters (vocab size, layers, etc.)
- Name the model
- Cancel

### 2. Model Load Dialog  
**When**: User clicks "Load Model"
**Shows**:
- List of available models in models/ directory
- Model info (size, date, parameters)
- Preview button
- Load/Cancel buttons

### 3. Model Save Dialog
**When**: User clicks "Save Model" or training completes
**Options**:
- Save as new model (enter name)
- Overwrite existing model (with confirmation)
- Cancel

### 4. Training Start Dialog
**When**: User clicks "START TRAINING"
**Checks**:
- Is model loaded? If not, offer to create one
- Are training files selected? If not, show error
- Confirm parameters
- Ask: "Create new model" or "Continue training existing model"

### 5. Overwrite Confirmation Dialog
**When**: Saving would overwrite existing file
**Shows**:
- "Model 'X' already exists"
- "Overwrite?" Yes/No/Cancel
- Option to save with different name

## Implementation Steps

### Step 1: Create Dialog System (2 hours)
- Generic dialog framework
- Message dialogs
- Confirmation dialogs  
- Input dialogs
- List selection dialogs

### Step 2: Model Browser (1 hour)
- Scan models/ directory
- Read model headers
- Display model info
- Select model to load

### Step 3: Training Workflow (1 hour)
- Check for model before training
- Offer to create if missing
- Confirm overwrite on save
- Handle training completion

### Step 4: UI Integration (1 hour)
- Add "New Model" button
- Add "Load Model" button with browser
- Update "Save Model" with dialog
- Update training start logic

### Step 5: Testing (30 min)
- Test all workflows
- Test edge cases
- Verify UX is smooth

## Immediate Fix (30 minutes)

For now, let me implement a quick fix:

1. **On startup**: Don't auto-load model, show "No model loaded"
2. **Before training**: Check if model exists
   - If yes: Ask "Overwrite existing model or create new?"
   - If no: Create new model with random weights
3. **On save**: Ask for filename if new, confirm if overwriting

This gives basic functionality while we build the full system.

## Code Changes Needed

### app/main.c
- Remove auto-load logic
- Add model status check

### app/ui/tabs/tab_training.c
- Add model creation button
- Add model load button with file picker
- Add save dialog
- Add training start checks

### app/simple_dialog.c (exists but not used)
- Implement modal dialogs
- Add confirmation dialogs
- Add input dialogs

## Timeline

- **Immediate fix**: 30 minutes
- **Full system**: 5-6 hours
- **Testing**: 1 hour
- **Total**: ~7 hours

## Priority

**HIGH** - This is critical UX that should have been there from the start.

User is right to be frustrated - the system makes decisions without asking.

Let me implement the immediate fix now, then we can build the full system.