/**
 * @file adaptive_threading.c
 * @brief Adaptive Threading Implementation
 */

#include "adaptive_threading.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

// ============================================================================
// SYSTEM DETECTION
// ============================================================================

uint32_t adaptive_get_available_cores(void) {
    #ifdef _SC_NPROCESSORS_ONLN
        long cores = sysconf(_SC_NPROCESSORS_ONLN);
        if (cores > 0) {
            return (uint32_t)cores;
        }
    #endif
    
    // Conservative default
    return 4;
}

uint64_t adaptive_get_available_memory_mb(void) {
    #ifdef __linux__
        struct sysinfo info;
        if (sysinfo(&info) == 0) {
            return (uint64_t)(info.totalram / (1024 * 1024));
        }
    #endif
    
    // Conservative default: 4 GB
    return 4096;
}

uint32_t adaptive_get_recommended_threads(int workload_type) {
    uint32_t cores = adaptive_get_available_cores();
    uint64_t memory_mb = adaptive_get_available_memory_mb();
    
    // Calculate based on workload type
    uint32_t recommended = cores;
    
    switch (workload_type) {
        case 0:  // CPU-bound
            // Use all available cores
            recommended = cores;
            break;
            
        case 1:  // Memory-bound
            // Limit based on available memory
            // Assume 100 MB per thread
            recommended = (uint32_t)(memory_mb / 100);
            if (recommended > cores) recommended = cores;
            if (recommended < 1) recommended = 1;
            break;
            
        case 2:  // Balanced
        default:
            // Use 75% of cores to leave room for system
            recommended = (cores * 3) / 4;
            if (recommended < 1) recommended = 1;
            break;
    }
    
    // Cap at 96 (88D maximum)
    if (recommended > 96) recommended = 96;
    
    return recommended;
}

// ============================================================================
// ADAPTIVE THREAD POOL
// ============================================================================

HierarchicalThreadPool* hierarchical_thread_pool_create_88d_adaptive(
    uint32_t base,
    uint32_t max_physical_threads
) {
    // Auto-detect if not specified
    if (max_physical_threads == 0) {
        max_physical_threads = adaptive_get_available_cores();
    }
    
    printf("Creating adaptive 88D thread pool:\n");
    printf("  Logical threads: 96 (88D structure)\n");
    printf("  Physical threads: %u (available cores)\n", max_physical_threads);
    printf("  Memory per thread: ~100 MB\n");
    printf("  Total memory: ~%u MB\n", max_physical_threads * 100);
    
    // Create standard 88D pool
    // NOTE: This currently creates 96 OS threads
    // TODO: Modify to create only max_physical_threads OS threads
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d(base);
    
    if (!pool) {
        fprintf(stderr, "Failed to create 88D thread pool\n");
        return NULL;
    }
    
    // Mark as adaptive
    pool->use_adaptive_threading = true;
    pool->max_physical_threads = max_physical_threads;
    
    // TODO: Implement work stealing scheduler
    // TODO: Implement shared memory pools
    // TODO: Reduce actual pthread_create calls to max_physical_threads
    
    printf("  ✓ Adaptive 88D thread pool created\n");
    printf("  ⚠ Note: Full adaptive implementation pending\n");
    printf("  ⚠ Currently creates 96 OS threads (will be optimized)\n");
    
    return pool;
}

int adaptive_get_statistics(
    HierarchicalThreadPool* pool,
    uint32_t* out_logical_threads,
    uint32_t* out_physical_threads,
    uint64_t* out_work_stolen,
    uint64_t* out_memory_mb
) {
    if (!pool) return -1;
    
    if (out_logical_threads) {
        *out_logical_threads = 96;  // Always 96 in 88D
    }
    
    if (out_physical_threads) {
        *out_physical_threads = pool->use_adaptive_threading ? 
            pool->max_physical_threads : pool->num_threads;
    }
    
    if (out_work_stolen) {
        // TODO: Track work stealing statistics
        *out_work_stolen = 0;
    }
    
    if (out_memory_mb) {
        // Estimate: 100 MB per physical thread
        uint32_t physical = pool->use_adaptive_threading ? 
            pool->max_physical_threads : pool->num_threads;
        *out_memory_mb = physical * 100;
    }
    
    return 0;
}

void adaptive_print_statistics(HierarchicalThreadPool* pool) {
    if (!pool) return;
    
    uint32_t logical, physical;
    uint64_t work_stolen, memory_mb;
    
    adaptive_get_statistics(pool, &logical, &physical, &work_stolen, &memory_mb);
    
    printf("\n=== Adaptive Threading Statistics ===\n");
    printf("Logical threads: %u (88D structure)\n", logical);
    printf("Physical threads: %u (OS threads)\n", physical);
    printf("Work stolen: %lu items\n", work_stolen);
    printf("Memory usage: ~%lu MB\n", memory_mb);
    printf("Efficiency: %.1f%% (logical/physical)\n", 
           (double)logical / physical * 100.0);
    printf("====================================\n\n");
}

// ============================================================================
// CONFIGURATION
// ============================================================================

AdaptiveThreadingConfig adaptive_get_default_config(void) {
    AdaptiveThreadingConfig config;
    
    config.max_physical_threads = 0;  // Auto-detect
    config.memory_limit_mb = 0;       // No limit
    config.enable_work_stealing = true;
    config.enable_shared_memory = true;
    config.enable_numa_awareness = true;
    config.memory_pool_size_mb = 100;  // 100 MB per thread
    
    return config;
}

HierarchicalThreadPool* hierarchical_thread_pool_create_88d_configured(
    uint32_t base,
    const AdaptiveThreadingConfig* config
) {
    if (!config) {
        AdaptiveThreadingConfig default_config = adaptive_get_default_config();
        return hierarchical_thread_pool_create_88d_adaptive(base, 
            default_config.max_physical_threads);
    }
    
    // Validate configuration
    uint32_t physical_threads = config->max_physical_threads;
    if (physical_threads == 0) {
        physical_threads = adaptive_get_available_cores();
    }
    
    // Check memory limit
    if (config->memory_limit_mb > 0) {
        uint64_t required_mb = physical_threads * config->memory_pool_size_mb;
        if (required_mb > config->memory_limit_mb) {
            // Reduce threads to fit memory limit
            physical_threads = config->memory_limit_mb / config->memory_pool_size_mb;
            if (physical_threads < 1) physical_threads = 1;
            
            printf("⚠ Reducing threads to %u to fit memory limit (%lu MB)\n",
                   physical_threads, config->memory_limit_mb);
        }
    }
    
    // Create adaptive pool
    return hierarchical_thread_pool_create_88d_adaptive(base, physical_threads);
}