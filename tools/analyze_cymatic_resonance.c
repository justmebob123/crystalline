/**
 * Analyze Cymatic Resonance Tool
 * 
 * Analyzes cymatic frequency patterns over training steps.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/ai/cllm_cymatic_training.h"

void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("\n");
    printf("Analyze cymatic resonance patterns.\n");
    printf("\n");
    printf("Options:\n");
    printf("  --steps N       Number of training steps to analyze (default: 1000)\n");
    printf("  --interval N    Sampling interval (default: 100)\n");
    printf("  --help          Show this help message\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s --steps 5000 --interval 100\n", program_name);
}

int main(int argc, char** argv) {
    uint32_t num_steps = 1000;
    uint32_t interval = 100;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--steps") == 0 && i + 1 < argc) {
            num_steps = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--interval") == 0 && i + 1 < argc) {
            interval = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    printf("\n=== Cymatic Resonance Analyzer ===\n\n");
    printf("Configuration:\n");
    printf("  Total steps: %u\n", num_steps);
    printf("  Sampling interval: %u\n", interval);
    printf("  Samples: %u\n", num_steps / interval);
    printf("\n");
    
    // Analyze resonance over time
    printf("Cymatic Resonance Pattern:\n");
    printf("%-10s %-15s\n", "Step", "Modulation");
    printf("%-10s %-15s\n", "----------", "---------------");
    
    for (uint32_t step = 0; step <= num_steps; step += interval) {
        double modulation = cllm_get_cymatic_modulation(step);
        printf("%-10u %-15.6f\n", step, modulation);
    }
    
    printf("\n");
    
    // Show detailed statistics for a few key points
    printf("Detailed Statistics:\n\n");
    
    uint32_t key_steps[] = {0, num_steps / 4, num_steps / 2, 3 * num_steps / 4, num_steps};
    for (int i = 0; i < 5; i++) {
        cllm_print_cymatic_stats(key_steps[i]);
    }
    
    // Compute harmonics for 432 Hz base
    printf("Harmonic Series (432 Hz base with φ damping):\n");
    double harmonics[12];
    cllm_compute_harmonics(432.0, 12, harmonics);
    
    for (int i = 0; i < 12; i++) {
        printf("  Harmonic %2d: %.2f Hz\n", i + 1, harmonics[i]);
    }
    
    printf("\n✓ Analysis complete!\n\n");
    
    return 0;
}