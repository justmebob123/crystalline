# Training Tab Refactor Plan - Per-Tab Model Ownership

**Date:** 2024-12-03  
**Status:** 🟡 Planning Phase  
**Priority:** HIGH

---

## 🎯 Objective

Refactor Training Tab to own its model instance independently, removing dependency on global model manager coordination layer while preserving all functionality.

---

## 📊 Current Architecture Analysis

### Current State (Complex)
```c
// Global state in AppState
struct AppState {
    CLLMModel* cllm_model;           // Shared model pointer
    CLLMTraining* cllm_training;     // Shared training state
    bool training_in_progress;
    // ... 50+ other fields ...
};

// Training flow
1. User selects model from dropdown
2. Check model_manager_get_status()
3. If not accessible, call model_manager_reload()
4. Call model_manager_acquire_write()
5. Store in state->cllm_model
6. Create training with cllm_training_init()
7. Start training thread
8. Training thread uses state->cllm_model
9. On stop, call model_manager_release_write()
10. Model stays in AppState (unclear ownership)
```

### Problems
1. **Unclear Ownership**: Who owns state->cllm_model?
2. **Global State**: AppState has 50+ fields, hard to track
3. **Coordination Overhead**: model_manager acquire/release
4. **No Concurrency**: Write lock blocks everything
5. **Memory Waste**: Model stays loaded in AppState

---

## 🏗️ New Architecture

### Proposed State (Simple)
```c
// Training Tab State (local to tab)
typedef struct {
    // Model ownership (OWNED BY THIS TAB)
    CLLMModel* model;                // NULL when not training
    char model_path[512];
    char model_name[256];
    bool model_loaded;
    
    // Training state (OWNED BY THIS TAB)
    CLLMTraining* training;          // NULL when not training
    pthread_t training_thread;
    bool is_training;
    bool should_stop;
    
    // Training statistics (UPDATED BY TRAINING THREAD)
    struct {
        int current_epoch;
        int total_epochs;
        float current_loss;
        float best_loss;
        uint64_t batches_processed;
        uint64_t tokens_processed;
        time_t training_start_time;
        time_t last_update_time;
        
        // Sphere statistics
        int active_spheres;
        uint64_t sphere_batches[12];
        float sphere_losses[12];
    } stats;
    
    // UI elements
    // ... existing UI elements ...
    
    // Training files
    TrainingFile files[100];
    int file_count;
    
} TrainingTabState;

// Global instance (only for this tab)
static TrainingTabState g_training_tab = {0};
```

### Proposed Flow (Simple)
```c
1. User selects model from dropdown
2. training_tab_load_model(model_name)
   - Builds path: ./models/{model_name}.cllm
   - Loads with cllm_read_model()
   - Stores in g_training_tab.model
   - Sets model_loaded = true
3. User clicks "Start Training"
4. training_tab_start_training()
   - Creates training with cllm_training_init()
   - Spawns training thread
   - Training thread updates g_training_tab.stats
5. Training completes or user stops
6. training_tab_stop_training()
   - Stops training thread
   - Saves model with cllm_write_model()
   - Frees training with cllm_training_free()
7. training_tab_unload_model()
   - Frees model with cllm_free_model()
   - Sets model = NULL, model_loaded = false
```

---

## 🔧 Implementation Steps

### Step 1: Add Training Tab State Structure (30 min)

**File:** `app/ui/tabs/tab_training.c`

**Add after includes:**
```c
// Training Tab State - Owns its model independently
typedef struct {
    // Model ownership
    CLLMModel* model;
    char model_path[512];
    char model_name[256];
    bool model_loaded;
    
    // Training state
    CLLMTraining* training;
    pthread_t training_thread;
    bool is_training;
    bool should_stop;
    
    // Training statistics
    struct {
        int current_epoch;
        int total_epochs;
        float current_loss;
        float best_loss;
        uint64_t batches_processed;
        uint64_t tokens_processed;
        time_t training_start_time;
        
        // Sphere statistics
        int active_spheres;
        uint64_t sphere_batches[12];
        float sphere_losses[12];
    } stats;
} TrainingTabState;

// Add to g_training_ui
static struct {
    // ... existing UI elements ...
    
    // NEW: Training tab state
    TrainingTabState tab_state;
    
} g_training_ui = {0};
```

---

### Step 2: Implement Model Loading Functions (1 hour)

**Add to tab_training.c:**

```c
/**
 * Load a model for training
 */
static bool training_tab_load_model(const char* model_name) {
    if (!model_name || !model_name[0]) {
        fprintf(stderr, "No model name provided\n");
        return false;
    }
    
    // Unload existing model if any
    training_tab_unload_model();
    
    // Build model path
    char model_path[512];
    if (!model_registry_get_path(model_name, model_path)) {
        fprintf(stderr, "Failed to get path for model: %s\n", model_name);
        return false;
    }
    
    // Load model
    printf("Loading model: %s\n", model_path);
    CLLMModel* model = cllm_read_model(model_path);
    if (!model) {
        fprintf(stderr, "Failed to load model: %s\n", model_path);
        return false;
    }
    
    // Store in tab state
    g_training_ui.tab_state.model = model;
    snprintf(g_training_ui.tab_state.model_path, 512, "%s", model_path);
    snprintf(g_training_ui.tab_state.model_name, 256, "%s", model_name);
    g_training_ui.tab_state.model_loaded = true;
    
    printf("✓ Model loaded: %s (%lu vocab, %lu dim, %u layers)\n",
           model_name, model->vocab_size, model->embedding_dim, model->num_layers);
    
    return true;
}

/**
 * Create a new model for training
 */
static bool training_tab_create_model(const char* model_name, const CLLMConfig* config) {
    if (!model_name || !config) {
        return false;
    }
    
    // Unload existing model if any
    training_tab_unload_model();
    
    // Create model
    printf("Creating model: %s\n", model_name);
    CLLMModel* model = cllm_create_model(config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return false;
    }
    
    // Build model path
    char model_path[512];
    if (!model_registry_get_path(model_name, model_path)) {
        fprintf(stderr, "Failed to get path for model: %s\n", model_name);
        cllm_free_model(model);
        return false;
    }
    
    // Store in tab state
    g_training_ui.tab_state.model = model;
    snprintf(g_training_ui.tab_state.model_path, 512, "%s", model_path);
    snprintf(g_training_ui.tab_state.model_name, 256, "%s", model_name);
    g_training_ui.tab_state.model_loaded = true;
    
    // Save to disk
    if (cllm_write_model(model, model_path) != 0) {
        fprintf(stderr, "Warning: Failed to save model to disk\n");
    }
    
    // Register in registry
    model_registry_register(model_name, model_path);
    
    printf("✓ Model created: %s\n", model_name);
    
    return true;
}

/**
 * Save the current model
 */
static bool training_tab_save_model(void) {
    if (!g_training_ui.tab_state.model_loaded || !g_training_ui.tab_state.model) {
        fprintf(stderr, "No model loaded\n");
        return false;
    }
    
    printf("Saving model: %s\n", g_training_ui.tab_state.model_path);
    
    if (cllm_write_model(g_training_ui.tab_state.model, g_training_ui.tab_state.model_path) != 0) {
        fprintf(stderr, "Failed to save model\n");
        return false;
    }
    
    // Refresh registry metadata
    model_registry_refresh(g_training_ui.tab_state.model_name);
    
    printf("✓ Model saved\n");
    return true;
}

/**
 * Unload the current model
 */
static void training_tab_unload_model(void) {
    if (!g_training_ui.tab_state.model_loaded) {
        return;
    }
    
    // Stop training if active
    if (g_training_ui.tab_state.is_training) {
        training_tab_stop_training();
    }
    
    // Free model
    if (g_training_ui.tab_state.model) {
        printf("Unloading model: %s\n", g_training_ui.tab_state.model_name);
        cllm_free_model(g_training_ui.tab_state.model);
        g_training_ui.tab_state.model = NULL;
    }
    
    // Clear state
    g_training_ui.tab_state.model_loaded = false;
    g_training_ui.tab_state.model_name[0] = '\0';
    g_training_ui.tab_state.model_path[0] = '\0';
    
    printf("✓ Model unloaded\n");
}
```

---

### Step 3: Implement Training Functions (1-2 hours)

**Add to tab_training.c:**

```c
/**
 * Training thread function
 */
static void* training_thread_func(void* arg) {
    TrainingTabState* tab = (TrainingTabState*)arg;
    
    printf("Training thread started\n");
    
    // Training loop
    for (int epoch = 0; epoch < tab->stats.total_epochs && !tab->should_stop; epoch++) {
        tab->stats.current_epoch = epoch + 1;
        
        // Train one epoch using kissing spheres
        float epoch_loss = threaded_train_epoch_lockfree(g_threaded_system, epoch);
        
        // Update statistics
        tab->stats.current_loss = epoch_loss;
        if (epoch == 0 || epoch_loss < tab->stats.best_loss) {
            tab->stats.best_loss = epoch_loss;
        }
        tab->stats.last_update_time = time(NULL);
        
        printf("Epoch %d/%d - Loss: %.4f\n", 
               epoch + 1, tab->stats.total_epochs, epoch_loss);
        
        // Auto-save every 5 epochs
        if ((epoch + 1) % 5 == 0) {
            training_tab_save_model();
        }
    }
    
    // Final save
    training_tab_save_model();
    
    // Cleanup
    tab->is_training = false;
    printf("Training thread completed\n");
    
    return NULL;
}

/**
 * Start training
 */
static bool training_tab_start_training(void) {
    if (!g_training_ui.tab_state.model_loaded) {
        fprintf(stderr, "No model loaded\n");
        return false;
    }
    
    if (g_training_ui.tab_state.is_training) {
        fprintf(stderr, "Training already in progress\n");
        return false;
    }
    
    // Create training configuration
    CLLMTrainingConfig config = {
        .num_epochs = state->training_epochs,
        .batch_size = state->training_batch_size,
        .sequence_length = state->training_sequence_length,
        .learning_rate = state->training_learning_rate,
        // ... other config ...
    };
    
    // Initialize training
    g_training_ui.tab_state.training = cllm_training_init(
        g_training_ui.tab_state.model, 
        &config
    );
    
    if (!g_training_ui.tab_state.training) {
        fprintf(stderr, "Failed to initialize training\n");
        return false;
    }
    
    // Load training data
    for (int i = 0; i < g_training_ui.file_count; i++) {
        if (g_training_ui.files[i].selected) {
            cllm_load_training_data(
                g_training_ui.tab_state.training,
                g_training_ui.files[i].filepath
            );
        }
    }
    
    // Initialize statistics
    g_training_ui.tab_state.stats.current_epoch = 0;
    g_training_ui.tab_state.stats.total_epochs = config.num_epochs;
    g_training_ui.tab_state.stats.current_loss = 0.0f;
    g_training_ui.tab_state.stats.best_loss = 999999.0f;
    g_training_ui.tab_state.stats.batches_processed = 0;
    g_training_ui.tab_state.stats.tokens_processed = 0;
    g_training_ui.tab_state.stats.training_start_time = time(NULL);
    
    // Start training thread
    g_training_ui.tab_state.is_training = true;
    g_training_ui.tab_state.should_stop = false;
    
    if (pthread_create(&g_training_ui.tab_state.training_thread, NULL,
                      training_thread_func, &g_training_ui.tab_state) != 0) {
        fprintf(stderr, "Failed to create training thread\n");
        cllm_training_free(g_training_ui.tab_state.training);
        g_training_ui.tab_state.training = NULL;
        g_training_ui.tab_state.is_training = false;
        return false;
    }
    
    printf("✓ Training started\n");
    return true;
}

/**
 * Stop training
 */
static void training_tab_stop_training(void) {
    if (!g_training_ui.tab_state.is_training) {
        return;
    }
    
    printf("Stopping training...\n");
    
    // Signal thread to stop
    g_training_ui.tab_state.should_stop = true;
    
    // Wait for thread to finish
    pthread_join(g_training_ui.tab_state.training_thread, NULL);
    
    // Cleanup training
    if (g_training_ui.tab_state.training) {
        cllm_training_free(g_training_ui.tab_state.training);
        g_training_ui.tab_state.training = NULL;
    }
    
    g_training_ui.tab_state.is_training = false;
    
    printf("✓ Training stopped\n");
}
```

---

## 📋 Detailed Implementation Checklist

### Part A: State Structure (30 min)
- [ ] Add TrainingTabState structure
- [ ] Add tab_state to g_training_ui
- [ ] Initialize tab_state in init_training_tab()
- [ ] Test compilation

### Part B: Model Management Functions (1 hour)
- [ ] Implement training_tab_load_model()
- [ ] Implement training_tab_create_model()
- [ ] Implement training_tab_save_model()
- [ ] Implement training_tab_unload_model()
- [ ] Test each function individually

### Part C: Training Functions (1-2 hours)
- [ ] Implement training_thread_func()
- [ ] Implement training_tab_start_training()
- [ ] Implement training_tab_stop_training()
- [ ] Update on_start_clicked() to use new functions
- [ ] Update on_save_clicked() to use new functions
- [ ] Test training flow

### Part D: Remove Model Manager Dependencies (30 min)
- [ ] Remove model_manager_acquire_write() call
- [ ] Remove model_manager_release_write() call
- [ ] Remove model_manager_get_status() call
- [ ] Remove model_manager_reload() call
- [ ] Remove AppState->cllm_model usage
- [ ] Remove AppState->cllm_training usage

### Part E: Update Statistics Display (30 min)
- [ ] Update visualization to use tab_state.stats
- [ ] Update metrics panel to use tab_state.stats
- [ ] Update progress bar to use tab_state.stats
- [ ] Test statistics display

### Part F: Testing (1 hour)
- [ ] Test model loading
- [ ] Test model creation
- [ ] Test training start/stop
- [ ] Test model saving
- [ ] Test model unloading
- [ ] Test statistics display
- [ ] Test with multiple training sessions

---

## 🚨 Critical Considerations

### 1. AppState Cleanup
**Problem:** AppState has many training-related fields that should move to tab state

**Fields to Move:**
```c
// From AppState to TrainingTabState
CLLMModel* cllm_model;              → tab_state.model
CLLMTraining* cllm_training;        → tab_state.training
bool training_in_progress;          → tab_state.is_training
int training_current_epoch;         → tab_state.stats.current_epoch
float training_loss;                → tab_state.stats.current_loss
// ... many more ...
```

**Solution:** Move fields incrementally, test after each move

### 2. Training Thread Integration
**Problem:** Current training thread uses AppState extensively

**Solution:** 
- Pass TrainingTabState* to training thread
- Update training thread to use tab state
- Remove AppState dependencies from training thread

### 3. Sphere Visualization
**Problem:** Sphere visualization reads from AppState->sphere_stats

**Solution:**
- Update sphere visualization to read from tab_state.stats
- Or: Keep sphere_stats in AppState but populate from tab_state

### 4. Backward Compatibility
**Problem:** Other code might depend on AppState fields

**Solution:**
- Keep AppState fields temporarily
- Add compatibility layer that copies from tab_state
- Remove after all tabs refactored

---

## 📊 Migration Strategy

### Phase A: Add New System (Non-Breaking)
1. Add TrainingTabState structure
2. Add new functions (load, create, save, unload)
3. Keep old code working
4. Test new functions independently

### Phase B: Switch to New System (Breaking)
1. Update on_start_clicked() to use new functions
2. Update on_save_clicked() to use new functions
3. Remove model_manager calls
4. Test training flow

### Phase C: Cleanup (Final)
1. Remove unused AppState fields
2. Remove compatibility layer
3. Update documentation
4. Final testing

---

## ✅ Success Criteria

### Functionality
- [ ] Can load existing models
- [ ] Can create new models
- [ ] Can start/stop training
- [ ] Can save models
- [ ] Training statistics update correctly
- [ ] Sphere visualization works
- [ ] No crashes or memory leaks

### Architecture
- [ ] Training tab owns its model
- [ ] No model_manager coordination calls
- [ ] Clear lifecycle (load → use → save → free)
- [ ] No global model state in AppState
- [ ] Training thread uses tab state

### Quality
- [ ] Zero build errors
- [ ] Zero critical warnings
- [ ] Code is simpler and clearer
- [ ] Memory usage is reasonable
- [ ] All tests pass

---

## 🎯 Next Steps

1. **Implement Part A** - Add state structure
2. **Implement Part B** - Add model management functions
3. **Test incrementally** - Verify each function works
4. **Implement Part C** - Add training functions
5. **Switch to new system** - Update button callbacks
6. **Test thoroughly** - Verify all functionality
7. **Cleanup** - Remove old code

**Estimated Time:** 4-5 hours for Training Tab  
**Risk Level:** MEDIUM (significant refactor but well-planned)

---

**Status:** 🟡 Ready to Begin Implementation  
**Next:** Part A - Add TrainingTabState structure