/**
 * Minimal test for sphere visualization mutex handling
 * Tests that mutex is properly locked and unlocked
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int batches_processed[12];
    float avg_loss[12];
    int active_spheres;
    int total_batches;
    float total_gradient_norm;
} SphereStats;

typedef struct {
    SphereStats sphere_stats;
    pthread_mutex_t sphere_stats_mutex;
} TestState;

// Simulates the visualization reading sphere_stats
void* visualization_thread(void* arg) {
    TestState* state = (TestState*)arg;
    
    for (int i = 0; i < 100; i++) {
        // Lock for reading (simulates draw_sphere_visualization)
        pthread_mutex_lock(&state->sphere_stats_mutex);
        
        // Read data
        int active = state->sphere_stats.active_spheres;
        int total = state->sphere_stats.total_batches;
        
        // Unlock after reading (THE FIX)
        pthread_mutex_unlock(&state->sphere_stats_mutex);
        
        printf("Viz: active=%d, total=%d\n", active, total);
        usleep(10000); // 10ms
    }
    
    return NULL;
}

// Simulates the training thread updating sphere_stats
void* training_thread(void* arg) {
    TestState* state = (TestState*)arg;
    
    for (int i = 0; i < 100; i++) {
        // Lock for writing
        pthread_mutex_lock(&state->sphere_stats_mutex);
        
        // Update data
        state->sphere_stats.active_spheres = i % 12;
        state->sphere_stats.total_batches = i;
        
        // Unlock after writing
        pthread_mutex_unlock(&state->sphere_stats_mutex);
        
        printf("Train: updated to %d\n", i);
        usleep(10000); // 10ms
    }
    
    return NULL;
}

int main() {
    TestState state = {0};
    
    // Initialize mutex
    pthread_mutex_init(&state.sphere_stats_mutex, NULL);
    
    // Create threads
    pthread_t viz_tid, train_tid;
    pthread_create(&viz_tid, NULL, visualization_thread, &state);
    pthread_create(&train_tid, NULL, training_thread, &state);
    
    // Wait for threads
    pthread_join(viz_tid, NULL);
    pthread_join(train_tid, NULL);
    
    // Cleanup
    pthread_mutex_destroy(&state.sphere_stats_mutex);
    
    printf("✅ Test completed successfully - no deadlock!\n");
    return 0;
}