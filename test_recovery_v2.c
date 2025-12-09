/**
 * Test for Universal Recovery System v2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "recovery_core.h"

int main() {
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Universal Recovery System v2 - Integration Test        ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    
    // Test 1: 50% sample coverage (realistic scenario)
    printf("Test 1: 50%% Sample Coverage\n");
    printf("─────────────────────────────────────────────────────────\n");
    
    size_t data_size = 1024;  // 1KB of data
    uint8_t* original_data = malloc(data_size);
    uint8_t* corrupted_data = malloc(data_size);
    
    // Generate random data
    srand(time(NULL));
    for (size_t i = 0; i < data_size; i++) {
        original_data[i] = rand() % 256;
    }
    
    // Copy to corrupted buffer
    memcpy(corrupted_data, original_data, data_size);
    
    // Corrupt 50% of the data (first half)
    size_t num_corrupted = data_size / 2;
    for (size_t i = 0; i < num_corrupted; i++) {
        corrupted_data[i] = rand() % 256;
    }
    
    printf("Data size: %zu bytes\n", data_size);
    printf("Sample coverage: 50%% (%zu bytes)\n", data_size - num_corrupted);
    printf("Corrupted: 50%% (%zu bytes)\n\n", num_corrupted);
    
    // Initialize recovery context with verbose output
    recovery_config_t config = recovery_default_config();
    config.verbose = 1;
    config.max_iterations = 10000;
    
    recovery_context_t* ctx = recovery_init(&config);
    if (!ctx) {
        printf("❌ Failed to initialize recovery context\n");
        free(original_data);
        free(corrupted_data);
        return 1;
    }
    
    // Set Q (corrupted data)
    recovery_error_t error = recovery_set_q(ctx, corrupted_data, data_size);
    if (error != RECOVERY_OK) {
        printf("❌ Failed to set Q data: %d\n", error);
        recovery_free(ctx);
        free(original_data);
        free(corrupted_data);
        return 1;
    }
    
    // Add samples (second half as known good data)
    error = recovery_add_sample(ctx, 
                               &corrupted_data[num_corrupted],  // Second half
                               data_size - num_corrupted,       // Length
                               num_corrupted,                   // Offset
                               1.0);                            // Full confidence
    if (error != RECOVERY_OK) {
        printf("❌ Failed to add sample: %d\n", error);
        recovery_free(ctx);
        free(original_data);
        free(corrupted_data);
        return 1;
    }
    
    // Run recovery
    printf("\nRunning recovery...\n");
    error = recovery_run(ctx);
    if (error != RECOVERY_OK) {
        printf("❌ Recovery failed with error: %d\n", error);
        recovery_free(ctx);
        free(original_data);
        free(corrupted_data);
        return 1;
    }
    
    // Get results
    recovery_result_t* result = recovery_get_result(ctx);
    if (!result) {
        printf("❌ Failed to get result\n");
        recovery_free(ctx);
        free(original_data);
        free(corrupted_data);
        return 1;
    }
    
    // Analyze results
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  RECOVERY RESULTS                                        ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  Quality score: %.1f%%                                   \n", result->quality_score * 100.0);
    printf("║  Iterations: %d                                          \n", result->iterations);
    printf("║  Converged: %s                                           \n", result->converged ? "Yes" : "No");
    printf("║  Final oscillation: %.6f                                 \n", result->final_oscillation);
    printf("║  Time: %.3f seconds                                      \n", result->time_seconds);
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    
    // Verify recovery quality (check first half which was corrupted)
    size_t correctly_recovered = 0;
    for (size_t i = 0; i < num_corrupted; i++) {
        if (result->data[i] == original_data[i]) {
            correctly_recovered++;
        }
    }
    
    double actual_recovery = (double)correctly_recovered / num_corrupted;
    printf("Actual recovery (verified): %.1f%% (%zu / %zu)\n", 
           actual_recovery * 100.0, correctly_recovered, num_corrupted);
    
    // Cleanup
    recovery_free_result(result);
    recovery_free(ctx);
    free(original_data);
    free(corrupted_data);
    
    // Success criteria
    if (actual_recovery > 0.05) {  // At least 5% recovery
        printf("\n✅ TEST PASSED: Recovery rate > 5%%\n");
        return 0;
    } else {
        printf("\n❌ TEST FAILED: Recovery rate too low (%.1f%%)\n", 
               actual_recovery * 100.0);
        return 1;
    }
}