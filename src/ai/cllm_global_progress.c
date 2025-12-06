#include "cllm_global_progress.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>

// Global singleton instance
static CLLMGlobalProgress g_progress = {0};
static int g_initialized = 0;

// Get current time in milliseconds
static int64_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// Format time duration
static void format_duration(char* buffer, size_t buffer_size, double seconds) {
    if (seconds < 60) {
        snprintf(buffer, buffer_size, "%.0fs", seconds);
    } else if (seconds < 3600) {
        int mins = (int)(seconds / 60);
        int secs = (int)seconds % 60;
        snprintf(buffer, buffer_size, "%dm %ds", mins, secs);
    } else {
        int hours = (int)(seconds / 3600);
        int mins = (int)(seconds / 60) % 60;
        snprintf(buffer, buffer_size, "%dh %dm", hours, mins);
    }
}

// Calculate phase weight
static int get_phase_weight(CLLMPhase phase) {
    switch (phase) {
        case CLLM_PHASE_LOADING_DATA: return PHASE_WEIGHT_LOADING;
        case CLLM_PHASE_BUILDING_VOCAB: return PHASE_WEIGHT_VOCAB;
        case CLLM_PHASE_TOKENIZING: return PHASE_WEIGHT_TOKENIZING;
        case CLLM_PHASE_TRAINING: return PHASE_WEIGHT_TRAINING;
        case CLLM_PHASE_SAVING: return PHASE_WEIGHT_SAVING;
        default: return 0;
    }
}

// Calculate overall progress
static double calculate_overall_progress(void) {
    double progress = 0.0;
    
    // Add completed phases
    for (CLLMPhase i = CLLM_PHASE_LOADING_DATA; i < g_progress.current_phase; i++) {
        progress += get_phase_weight(i);
    }
    
    // Add current phase progress
    if (g_progress.phase_total > 0) {
        double phase_percent = (double)g_progress.phase_current / g_progress.phase_total;
        progress += phase_percent * get_phase_weight(g_progress.current_phase);
    }
    
    return progress;
}

// Calculate ETA for current phase
static double calculate_phase_eta(void) {
    if (g_progress.phase_current == 0) return 0.0;
    
    time_t now = time(NULL);
    double elapsed = difftime(now, g_progress.phase_start_time);
    
    if (elapsed < 0.1) return 0.0;
    
    double rate = (double)g_progress.phase_current / elapsed;
    if (rate < 0.001) return 0.0;
    
    size_t remaining = g_progress.phase_total - g_progress.phase_current;
    return remaining / rate;
}

// Display the progress bar
static void display_progress(void) {
    // Calculate overall progress
    g_progress.overall_progress = calculate_overall_progress();
    
    // Calculate timing
    time_t now = time(NULL);
    double program_elapsed = difftime(now, g_progress.program_start_time);
    double phase_elapsed = difftime(now, g_progress.phase_start_time);
    double phase_eta = calculate_phase_eta();
    
    // Calculate phase percentage
    double phase_percent = g_progress.phase_total > 0 ? 
                          (100.0 * g_progress.phase_current) / g_progress.phase_total : 0.0;
    
    // Calculate speed
    double speed = phase_elapsed > 0 ? g_progress.phase_current / phase_elapsed : 0.0;
    
    // Format times
    char program_elapsed_str[32];
    char phase_eta_str[32];
    format_duration(program_elapsed_str, sizeof(program_elapsed_str), program_elapsed);
    
    if (phase_eta > 0) {
        format_duration(phase_eta_str, sizeof(phase_eta_str), phase_eta);
    } else {
        snprintf(phase_eta_str, sizeof(phase_eta_str), "calculating...");
    }
    
    // Create phase progress bar (50 chars)
    char phase_bar[51];
    int phase_filled = (int)(50.0 * phase_percent / 100.0);
    for (int i = 0; i < 50; i++) {
        if (i < phase_filled) {
            phase_bar[i] = '=';
        } else if (i == phase_filled && phase_percent > 0 && phase_percent < 100) {
            phase_bar[i] = '>';
        } else {
            phase_bar[i] = ' ';
        }
    }
    phase_bar[50] = '\0';
    
    // Format speed
    char speed_str[32];
    if (speed >= 1000) {
        snprintf(speed_str, sizeof(speed_str), "%.1fk/s", speed / 1000.0);
    } else if (speed >= 1) {
        snprintf(speed_str, sizeof(speed_str), "%.0f/s", speed);
    } else {
        snprintf(speed_str, sizeof(speed_str), "%.2f/s", speed);
    }
    
    // Clear previous display (move cursor up 6 lines and clear)
    if (g_progress.last_update_ms > 0) {
        printf("\033[6A\033[J");  // Move up 6 lines and clear to end of screen
    }
    
    // Print the progress display
    printf("╔══════════════════════════════════════════════════════════════════════════╗\n");
    printf("║ CLLM Training Pipeline                            Overall: %5.1f%% ║\n", 
           g_progress.overall_progress);
    printf("╠══════════════════════════════════════════════════════════════════════════╣\n");
    
    // Phase line with training info if applicable
    if (g_progress.current_phase == CLLM_PHASE_TRAINING && g_progress.total_epochs > 0) {
        printf("║ Phase %d/5: %-40s Epoch %d/%d ║\n", 
               (int)g_progress.current_phase, g_progress.phase_name,
               g_progress.current_epoch, g_progress.total_epochs);
    } else {
        printf("║ Phase %d/5: %-57s ║\n", 
               (int)g_progress.current_phase, g_progress.phase_name);
    }
    
    // Progress bar line
    printf("║ [%s] %5.1f%% ║\n", phase_bar, phase_percent);
    
    // Stats line with training loss if applicable
    if (g_progress.current_phase == CLLM_PHASE_TRAINING && g_progress.current_loss > 0) {
        printf("║ %zu/%zu | Time: %s | ETA: %s | Speed: %s | Loss: %.4f    ║\n",
               g_progress.phase_current, g_progress.phase_total,
               program_elapsed_str, phase_eta_str, speed_str, g_progress.current_loss);
    } else {
        printf("║ %zu/%zu | Time: %s | ETA: %s | Speed: %s                    ║\n",
               g_progress.phase_current, g_progress.phase_total,
               program_elapsed_str, phase_eta_str, speed_str);
    }
    
    printf("╚══════════════════════════════════════════════════════════════════════════╝\n");
    
    fflush(stdout);
}

// Initialize global progress system
void cllm_global_progress_init(void) {
    if (g_initialized) return;
    
    memset(&g_progress, 0, sizeof(CLLMGlobalProgress));
    
    pthread_mutex_init(&g_progress.lock, NULL);
    
    g_progress.program_start_time = time(NULL);
    g_progress.update_interval_ms = 500;  // Update every 500ms
    g_progress.current_phase = CLLM_PHASE_INIT;
    g_progress.phase_name = "Initializing";
    
    g_initialized = 1;
    
    // Print initial display
    printf("\n");  // Add newline before progress display
    display_progress();
}

// Start a new phase
void cllm_global_progress_start_phase(CLLMPhase phase, const char* phase_name, size_t total) {
    pthread_mutex_lock(&g_progress.lock);
    
    g_progress.current_phase = phase;
    g_progress.phase_name = phase_name;
    g_progress.phase_current = 0;
    g_progress.phase_total = total;
    g_progress.phase_start_time = time(NULL);
    g_progress.subphase_name = NULL;
    g_progress.subphase_current = 0;
    g_progress.subphase_total = 0;
    
    display_progress();
    
    pthread_mutex_unlock(&g_progress.lock);
}

// Update phase progress
void cllm_global_progress_update(size_t current) {
    pthread_mutex_lock(&g_progress.lock);
    
    g_progress.phase_current = current;
    
    // Throttle updates
    int64_t now_ms = get_time_ms();
    if (now_ms - g_progress.last_update_ms >= g_progress.update_interval_ms) {
        display_progress();
        g_progress.last_update_ms = now_ms;
    }
    
    pthread_mutex_unlock(&g_progress.lock);
}

// Update with sub-phase information
void cllm_global_progress_update_subphase(const char* subphase_name, 
                                          size_t subphase_current, 
                                          size_t subphase_total) {
    pthread_mutex_lock(&g_progress.lock);
    
    g_progress.subphase_name = subphase_name;
    g_progress.subphase_current = subphase_current;
    g_progress.subphase_total = subphase_total;
    
    // Throttle updates
    int64_t now_ms = get_time_ms();
    if (now_ms - g_progress.last_update_ms >= g_progress.update_interval_ms) {
        display_progress();
        g_progress.last_update_ms = now_ms;
    }
    
    pthread_mutex_unlock(&g_progress.lock);
}

// Update training-specific information
void cllm_global_progress_update_training(int epoch, int total_epochs, double loss) {
    pthread_mutex_lock(&g_progress.lock);
    
    g_progress.current_epoch = epoch;
    g_progress.total_epochs = total_epochs;
    g_progress.current_loss = loss;
    
    // Throttle updates
    int64_t now_ms = get_time_ms();
    if (now_ms - g_progress.last_update_ms >= g_progress.update_interval_ms) {
        display_progress();
        g_progress.last_update_ms = now_ms;
    }
    
    pthread_mutex_unlock(&g_progress.lock);
}

// Complete current phase
void cllm_global_progress_complete_phase(void) {
    pthread_mutex_lock(&g_progress.lock);
    
    g_progress.phase_current = g_progress.phase_total;
    display_progress();
    
    pthread_mutex_unlock(&g_progress.lock);
}

// Complete entire program
void cllm_global_progress_complete(void) {
    pthread_mutex_lock(&g_progress.lock);
    
    g_progress.current_phase = CLLM_PHASE_COMPLETE;
    g_progress.phase_name = "Complete";
    g_progress.overall_progress = 100.0;
    
    display_progress();
    
    // Print final newline
    printf("\n");
    
    pthread_mutex_unlock(&g_progress.lock);
}

// Force display update
void cllm_global_progress_display(void) {
    pthread_mutex_lock(&g_progress.lock);
    display_progress();
    pthread_mutex_unlock(&g_progress.lock);
}

// Cleanup
void cllm_global_progress_cleanup(void) {
    if (!g_initialized) return;
    
    pthread_mutex_destroy(&g_progress.lock);
    g_initialized = 0;
}

// Get global progress instance
CLLMGlobalProgress* cllm_global_progress_get(void) {
    return &g_progress;
}