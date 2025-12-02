// control_thread.c - Asynchronous Control Thread for Background Initialization
// Part of CRYSTALLINE CLLM - Implements MASTER_PLAN architecture

#include "app_common.h"
#include <pthread.h>
#include <unistd.h>

// Control thread handle
static pthread_t g_control_thread = 0;
static bool g_control_thread_running = false;

// Forward declarations
extern int app_initialize_global_abacus(void);

/**
 * Control Thread Main Function
 * 
 * This thread handles ALL heavy initialization in the background:
 * 1. Crystalline abacus (rainbow table) initialization
 * 2. Model loading (if default model exists)
 * 3. Inference context creation
 * 4. Worker thread spawning (when ready)
 * 
 * The main application loop continues running while this happens.
 * UI shows initialization status and enables features when ready.
 */
void* control_thread_main(void* arg) {
    AppState* state = (AppState*)arg;
    
    printf("\n=== Control Thread Started ===\n");
    printf("Main loop continues running - UI is responsive\n\n");
    
    // PHASE 1: Initialize Crystalline Abacus (Rainbow Table)
    printf("=== Phase 1: Initializing Crystalline Abacus ===\n");
    state->abacus_initializing = true;
    state->abacus_ready = false;
    
    if (app_initialize_global_abacus() == 0) {
        state->abacus_ready = true;
        printf("✓ Abacus initialization complete\n");
    } else {
        fprintf(stderr, "✗ Abacus initialization failed\n");
    }
    state->abacus_initializing = false;
    
    // PHASE 2: Check for Default Model (but don't auto-load)
    printf("\n=== Phase 2: Checking for Models ===\n");
    const char* default_model_path = "models/saved_model.cllm";
    
    FILE* test_file = fopen(default_model_path, "rb");
    if (test_file) {
        fclose(test_file);
        printf("Found default model: %s\n", default_model_path);
        printf("NOTE: Model NOT auto-loaded to save memory (12GB+ for large models)\n");
        printf("Use LLM tab 'Load Model' button to load when needed\n");
        state->model_ready = false;
        snprintf(state->llm_output_text, sizeof(state->llm_output_text),
                "Model available. Click 'Load Model' in LLM tab to use it.");
    } else {
        printf("No default model found at %s\n", default_model_path);
        printf("Model can be created in Training tab or loaded in LLM tab\n");
        state->model_ready = false;
        snprintf(state->llm_output_text, sizeof(state->llm_output_text),
                "No model loaded. Start training to create a new model, or load an existing one.");
    }
    
    // PHASE 3: System Ready
    printf("\n=== Control Thread Initialization Complete ===\n");
    printf("System is now ready for use\n");
    printf("Abacus: %s | Model: %s\n",
           state->abacus_ready ? "✓ Ready" : "✗ Not Ready",
           state->model_ready ? "✓ Ready" : "✗ Not Ready");
    printf("\n");
    
    // Control thread continues running for coordination
    // In full implementation, this would spawn worker threads
    // and coordinate training/inference operations
    
    while (g_control_thread_running) {
        // Control thread coordination loop
        // TODO: Implement worker thread management
        // TODO: Implement batch coordination
        // TODO: Implement 12-fold symmetry distribution
        sleep(1);
    }
    
    printf("=== Control Thread Exiting ===\n");
    return NULL;
}

/**
 * Start Control Thread
 * 
 * Creates and starts the control thread for background initialization.
 * Returns immediately - main loop continues running.
 */
void start_control_thread(AppState* state) {
    if (g_control_thread_running) {
        printf("Control thread already running\n");
        return;
    }
    
    g_control_thread_running = true;
    
    if (pthread_create(&g_control_thread, NULL, control_thread_main, state) != 0) {
        fprintf(stderr, "ERROR: Failed to create control thread\n");
        g_control_thread_running = false;
        return;
    }
    
    printf("✓ Control thread started (background initialization)\n");
}

/**
 * Stop Control Thread
 * 
 * Signals control thread to stop and waits for it to exit.
 */
void stop_control_thread(void) {
    if (!g_control_thread_running) {
        return;
    }
    
    printf("Stopping control thread...\n");
    g_control_thread_running = false;
    
    if (g_control_thread) {
        pthread_join(g_control_thread, NULL);
        g_control_thread = 0;
    }
    
    printf("✓ Control thread stopped\n");
}