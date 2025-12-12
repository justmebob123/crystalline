# Model Manager Concurrent Access Fix

## Problem Analysis

### Current Behavior (BROKEN)
1. **Training Tab**: Uses `model_manager_acquire_write()` to get write lock on model
2. **LLM Tab**: Calls `app_load_model()` which loads a **separate copy** from disk
3. **Result**: Two separate model instances, training stops when LLM tab loads model

### Root Cause
The LLM tab bypasses the model manager entirely and loads a new copy of the model from disk using `cllm_read_model()`. This creates a separate model instance that is not synchronized with the training process.

### Expected Behavior (CORRECT)
1. **Training Tab**: Uses `model_manager_acquire_write()` to get write lock
2. **LLM Tab**: Uses `model_manager_acquire_read()` to get read lock on SAME model
3. **Result**: Concurrent access - training continues while inference reads from the same model

## Solution

### Step 1: Fix LLM Tab Model Loading

**File**: `app/ui/tabs/tab_llm.c`

**Current Code (WRONG)**:
```c
static void on_model_selected(int index, void* data) {
    // ...
    CLLMModel* model = app_load_model(model_path);  // WRONG: Loads separate copy
    if (model) {
        state->cllm_inference = cllm_inference_init(model);
    }
}
```

**Fixed Code (CORRECT)**:
```c
static void on_model_selected(int index, void* data) {
    // ...
    
    // First, prepare the model (ensures abacus has enough primes)
    if (!model_manager_prepare(model_name)) {
        printf("ERROR: Failed to prepare model\n");
        add_chat_message("Error: Failed to prepare model.", false);
        return;
    }
    
    // Acquire read lock on the model (allows concurrent access during training)
    CLLMModel* model = model_manager_acquire_read(model_name);
    if (model) {
        // Store model name for later release
        strncpy(llm_ui.active_model_name, model_name, sizeof(llm_ui.active_model_name) - 1);
        
        // Create inference context
        state->cllm_inference = cllm_inference_init(model);
        
        if (state->cllm_inference) {
            printf("Model acquired for inference (read lock)\n");
            add_chat_message("Model loaded successfully. You can now start chatting!", false);
        }
    } else {
        printf("ERROR: Failed to acquire model for reading\n");
        add_chat_message("Error: Failed to load model.", false);
    }
}
```

### Step 2: Release Read Lock on Cleanup

**Add to LLM tab cleanup**:
```c
void cleanup_llm_tab(void) {
    // Release model read lock if held
    if (llm_ui.active_model_name[0]) {
        model_manager_release_read(llm_ui.active_model_name);
        llm_ui.active_model_name[0] = '\0';
    }
    
    // ... rest of cleanup
}
```

### Step 3: Add Model Name Tracking

**Add to llm_ui struct**:
```c
static struct {
    // ... existing fields
    char active_model_name[256];  // Track which model we have a read lock on
} llm_ui = {0};
```

## Benefits of This Fix

1. **Concurrent Access**: Training can continue while inference reads from the same model
2. **Memory Efficiency**: Only one model instance in memory, not two separate copies
3. **Consistency**: Inference always uses the latest trained weights
4. **Thread Safety**: Read/write locks ensure safe concurrent access
5. **Proper Resource Management**: Model manager handles all locking and lifecycle

## Implementation Steps

1. Update LLM tab to use model_manager_acquire_read()
2. Add model name tracking to llm_ui struct
3. Add model_manager_release_read() to cleanup
4. Test concurrent training + inference
5. Verify training continues while inference is active

## Testing Plan

1. Start training on a model
2. While training is running, switch to LLM tab
3. Select the same model being trained
4. Verify training continues (check terminal output)
5. Perform inference (send messages)
6. Verify inference works correctly
7. Verify training completes successfully
8. Check that model is released properly on cleanup

## Expected Results

- ✅ Training continues uninterrupted when LLM tab loads model
- ✅ Inference works correctly with the model being trained
- ✅ Only one model instance in memory
- ✅ Proper read/write lock coordination
- ✅ Clean resource management on cleanup