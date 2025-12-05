# Cymatic Timing System - Usage Guide

**Quick Start Guide for Developers**

---

## Quick Start

### 1. Basic Setup (5 minutes)

```c
#include "ai/cllm_work_distribution_timing.h"
#include "ai/cllm_epoch_sync.h"

// Create contexts
TimedWorkDistributionContext* work_ctx = 
    timed_work_distribution_create(4, true);  // 4 threads, adaptive

EpochSyncContext* epoch_ctx = 
    epoch_sync_create(4, 10000, true);  // 4 threads, 10K samples/epoch, adaptive

// Use in training...

// Cleanup
timed_work_distribution_destroy(work_ctx);
epoch_sync_destroy(epoch_ctx);
```

### 2. Simple Training Loop (10 minutes)

```c
for (int epoch = 0; epoch < num_epochs; epoch++) {
    epoch_sync_start_epoch(epoch_ctx);
    
    while (!epoch_sync_is_epoch_complete(epoch_ctx)) {
        // Calculate work distribution
        TimedDistributionResult result;
        timed_calculate_work_distribution(work_ctx, 5, 3, &result);
        
        // Process batch
        process_batch(&result.distribution);
        
        // Record progress
        epoch_sync_record_samples(epoch_ctx, batch_size);
    }
    
    EpochTimingResult epoch_result;
    epoch_sync_end_epoch(epoch_ctx, &epoch_result);
}
```

---

## Common Use Cases

### Use Case 1: Single-Threaded Training

**Scenario**: Simple training loop with one thread.

```c
#include "ai/cllm_epoch_sync.h"

int main(void) {
    // Setup
    EpochSyncContext* ctx = epoch_sync_create(1, 1000, false);
    
    // Training
    for (int epoch = 0; epoch < 100; epoch++) {
        epoch_sync_start_epoch(ctx);
        
        for (int i = 0; i < 1000; i++) {
            // Train on sample
            train_sample(i);
            epoch_sync_record_samples(ctx, 1);
        }
        
        EpochTimingResult result;
        epoch_sync_end_epoch(ctx, &result);
        
        printf("Epoch %lu: %.2f ms\n", 
               result.epoch_number, result.epoch_time_ns / 1e6);
    }
    
    // Cleanup
    epoch_sync_destroy(ctx);
    return 0;
}
```

### Use Case 2: Multi-Threaded Training

**Scenario**: Parallel training with multiple threads.

```c
#include "ai/cllm_epoch_sync.h"
#include <pthread.h>

typedef struct {
    EpochSyncContext* ctx;
    int thread_id;
    int samples_per_thread;
} ThreadData;

void* worker_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    for (int i = 0; i < data->samples_per_thread; i++) {
        // Train on sample
        train_sample(data->thread_id, i);
        
        // Record progress
        epoch_sync_record_samples(data->ctx, 1);
        
        // Check if epoch complete
        if (epoch_sync_is_epoch_complete(data->ctx)) {
            break;
        }
    }
    
    return NULL;
}

int main(void) {
    const int num_threads = 4;
    const int samples_per_epoch = 10000;
    
    // Setup
    EpochSyncContext* ctx = epoch_sync_create(num_threads, samples_per_epoch, true);
    
    // Training
    for (int epoch = 0; epoch < 100; epoch++) {
        epoch_sync_start_epoch(ctx);
        
        // Create threads
        pthread_t threads[num_threads];
        ThreadData thread_data[num_threads];
        
        for (int i = 0; i < num_threads; i++) {
            thread_data[i].ctx = ctx;
            thread_data[i].thread_id = i;
            thread_data[i].samples_per_thread = samples_per_epoch / num_threads;
            pthread_create(&threads[i], NULL, worker_thread, &thread_data[i]);
        }
        
        // Wait for completion
        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }
        
        // End epoch
        EpochTimingResult result;
        epoch_sync_end_epoch(ctx, &result);
        
        printf("Epoch %lu: %.2f ms, %.2f samples/sec\n",
               result.epoch_number,
               result.epoch_time_ns / 1e6,
               result.samples_processed / (result.epoch_time_ns / 1e9));
    }
    
    // Print statistics
    epoch_sync_print_statistics(ctx);
    
    // Cleanup
    epoch_sync_destroy(ctx);
    return 0;
}
```

### Use Case 3: Work Distribution with Timing

**Scenario**: Distribute work among parent and children with timing.

```c
#include "ai/cllm_work_distribution_timing.h"

int main(void) {
    // Setup
    TimedWorkDistributionContext* ctx = 
        timed_work_distribution_create(1, true);
    
    // Calculate distributions
    for (int i = 0; i < 1000; i++) {
        TimedDistributionResult result;
        timed_calculate_work_distribution(ctx, 5, 3, &result);
        
        // Use distribution
        printf("Parent: %.3f, Child: %.3f\n",
               result.distribution.parent_keeps,
               result.distribution.child_gets);
        
        // Split work accordingly
        split_work(result.distribution.parent_keeps,
                   result.distribution.child_gets);
    }
    
    // Print statistics
    timed_work_distribution_print_stats(ctx);
    
    // Cleanup
    timed_work_distribution_destroy(ctx);
    return 0;
}
```

### Use Case 4: Combined Timing System

**Scenario**: Use both work distribution and epoch timing together.

```c
#include "ai/cllm_work_distribution_timing.h"
#include "ai/cllm_epoch_sync.h"

int main(void) {
    const int num_threads = 4;
    const int num_epochs = 10;
    const int samples_per_epoch = 10000;
    
    // Setup both systems
    TimedWorkDistributionContext* work_ctx = 
        timed_work_distribution_create(num_threads, true);
    EpochSyncContext* epoch_ctx = 
        epoch_sync_create(num_threads, samples_per_epoch, true);
    
    // Training loop
    for (int epoch = 0; epoch < num_epochs; epoch++) {
        epoch_sync_start_epoch(epoch_ctx);
        
        while (!epoch_sync_is_epoch_complete(epoch_ctx)) {
            // Calculate work distribution with timing
            TimedDistributionResult work_result;
            timed_calculate_work_distribution(work_ctx, 5, 3, &work_result);
            
            // Process batch using distribution
            int samples_processed = process_batch_with_distribution(
                &work_result.distribution
            );
            
            // Record progress
            epoch_sync_record_samples(epoch_ctx, samples_processed);
            epoch_sync_record_gradients(epoch_ctx, samples_processed);
        }
        
        // End epoch with synchronization
        EpochTimingResult epoch_result;
        epoch_sync_end_epoch(epoch_ctx, &epoch_result);
        
        printf("Epoch %lu: %.2f ms, overhead: %.2f%%\n",
               epoch_result.epoch_number,
               epoch_result.epoch_time_ns / 1e6,
               epoch_result.sync_overhead_percent);
    }
    
    // Print combined statistics
    printf("\n=== Work Distribution Statistics ===\n");
    timed_work_distribution_print_stats(work_ctx);
    
    printf("\n=== Epoch Synchronization Statistics ===\n");
    epoch_sync_print_statistics(epoch_ctx);
    
    // Cleanup
    timed_work_distribution_destroy(work_ctx);
    epoch_sync_destroy(epoch_ctx);
    
    return 0;
}
```

---

## Configuration Examples

### Example 1: High-Performance Configuration

**For**: Maximum throughput, minimal overhead

```c
// Use adaptive mode
TimedWorkDistributionContext* work_ctx = 
    timed_work_distribution_create(num_threads, true);  // adaptive=true

EpochSyncContext* epoch_ctx = 
    epoch_sync_create(num_threads, samples_per_epoch, true);  // adaptive=true

// Large epoch size to amortize synchronization
uint64_t samples_per_epoch = 50000;  // Large epoch
```

### Example 2: Frequent Updates Configuration

**For**: Frequent gradient updates, more synchronization

```c
// Non-adaptive for consistent timing
TimedWorkDistributionContext* work_ctx = 
    timed_work_distribution_create(num_threads, false);  // adaptive=false

EpochSyncContext* epoch_ctx = 
    epoch_sync_create(num_threads, samples_per_epoch, false);  // adaptive=false

// Small epoch size for frequent updates
uint64_t samples_per_epoch = 1000;  // Small epoch
```

### Example 3: Debug Configuration

**For**: Development and debugging

```c
// Non-adaptive for predictable behavior
TimedWorkDistributionContext* work_ctx = 
    timed_work_distribution_create(1, false);  // 1 thread, non-adaptive

EpochSyncContext* epoch_ctx = 
    epoch_sync_create(1, 100, false);  // 1 thread, small epoch, non-adaptive

// Enable detailed logging
timed_work_distribution_print_stats(work_ctx);  // After each epoch
epoch_sync_print_statistics(epoch_ctx);  // After training
```

---

## Performance Monitoring

### Monitor Overhead

```c
// Check overhead periodically
if (epoch % 10 == 0) {
    double work_overhead = calculate_timing_overhead(work_ctx);
    double epoch_overhead = epoch_sync_calculate_overhead(epoch_ctx);
    
    printf("Overhead: work=%.2f%%, epoch=%.2f%%\n",
           work_overhead, epoch_overhead);
    
    if (work_overhead > 5.0 || epoch_overhead > 10.0) {
        fprintf(stderr, "WARNING: High overhead detected\n");
    }
}
```

### Monitor Throughput

```c
// Check throughput
double samples_per_sec = epoch_sync_get_throughput(epoch_ctx);
double epochs_per_sec = epoch_sync_get_epoch_rate(epoch_ctx);

printf("Throughput: %.2f samples/sec, %.2f epochs/sec\n",
       samples_per_sec, epochs_per_sec);
```

### Get Detailed Statistics

```c
// Work distribution statistics
TimingStatistics work_stats;
timed_work_distribution_get_stats(work_ctx, &work_stats);

printf("Work Distribution:\n");
printf("  Total: %lu operations\n", work_stats.total_distributions);
printf("  Avg wait: %lu ns\n", work_stats.avg_wait_time_ns);
printf("  Overhead: %.2f%%\n", work_stats.wait_overhead_percent);

// Epoch statistics
EpochStatistics epoch_stats;
epoch_sync_get_statistics(epoch_ctx, &epoch_stats);

printf("Epoch Synchronization:\n");
printf("  Total: %lu epochs\n", epoch_stats.total_epochs);
printf("  Avg time: %lu ns\n", epoch_stats.avg_epoch_time_ns);
printf("  Overhead: %.2f%%\n", epoch_stats.avg_sync_overhead_percent);
```

---

## Error Handling

### Handle Creation Failures

```c
TimedWorkDistributionContext* work_ctx = 
    timed_work_distribution_create(num_threads, true);

if (!work_ctx) {
    fprintf(stderr, "ERROR: Failed to create work distribution context\n");
    fprintf(stderr, "  Threads: %d\n", num_threads);
    
    // Fall back to baseline
    use_baseline_work_distribution();
    return -1;
}
```

### Handle Synchronization Failures

```c
EpochTimingResult result;
bool success = epoch_sync_end_epoch(epoch_ctx, &result);

if (!success) {
    fprintf(stderr, "ERROR: Failed to end epoch\n");
    fprintf(stderr, "  Epoch: %lu\n", epoch_ctx->current_epoch);
    fprintf(stderr, "  Samples: %lu\n", epoch_ctx->samples_processed);
    
    // Reset and continue
    epoch_sync_reset_statistics(epoch_ctx);
    return -1;
}
```

### Cleanup on Error

```c
void cleanup_timing_contexts(void) {
    if (work_ctx) {
        timed_work_distribution_destroy(work_ctx);
        work_ctx = NULL;
    }
    
    if (epoch_ctx) {
        epoch_sync_destroy(epoch_ctx);
        epoch_ctx = NULL;
    }
}

// Register cleanup
atexit(cleanup_timing_contexts);

// Or use in error handling
if (error_occurred) {
    cleanup_timing_contexts();
    return -1;
}
```

---

## Tips & Tricks

### Tip 1: Start with Adaptive Mode

Always start with adaptive mode enabled. It provides the best balance of timing accuracy and performance.

```c
// Recommended
TimedWorkDistributionContext* ctx = 
    timed_work_distribution_create(num_threads, true);  // ✅ Adaptive
```

### Tip 2: Choose Appropriate Epoch Size

Epoch size should balance synchronization overhead and update frequency:
- Too small: High overhead
- Too large: Infrequent updates
- Just right: 5,000-20,000 samples

```c
// Good epoch sizes
uint64_t samples_per_epoch = 10000;  // ✅ Good balance
```

### Tip 3: Monitor Overhead Regularly

Check overhead every few epochs to catch performance issues early:

```c
if (epoch % 10 == 0) {
    double overhead = epoch_sync_calculate_overhead(epoch_ctx);
    if (overhead > 10.0) {
        log_warning("High overhead: %.2f%%", overhead);
    }
}
```

### Tip 4: Use Statistics for Debugging

Print detailed statistics when debugging performance issues:

```c
if (debug_mode) {
    timed_work_distribution_print_stats(work_ctx);
    epoch_sync_print_statistics(epoch_ctx);
}
```

### Tip 5: Reset Statistics Between Runs

Reset statistics between training runs for accurate measurements:

```c
// Between runs
timed_work_distribution_reset_stats(work_ctx);
epoch_sync_reset_statistics(epoch_ctx);
```

---

## Compilation

### Basic Compilation

```bash
gcc -o my_program my_program.c \
    -I./include -I./algorithms/include \
    -L. -L./algorithms \
    -lcllm -lalgorithms -lcrystalline \
    -lm -lpthread \
    -Wl,-rpath,'$ORIGIN'
```

### With Optimization

```bash
gcc -O2 -o my_program my_program.c \
    -I./include -I./algorithms/include \
    -L. -L./algorithms \
    -lcllm -lalgorithms -lcrystalline \
    -lm -lpthread \
    -Wl,-rpath,'$ORIGIN'
```

### With Debug Symbols

```bash
gcc -g -O0 -o my_program my_program.c \
    -I./include -I./algorithms/include \
    -L. -L./algorithms \
    -lcllm -lalgorithms -lcrystalline \
    -lm -lpthread \
    -Wl,-rpath,'$ORIGIN'
```

---

## Next Steps

1. **Read Full Documentation**: See `CYMATIC_TIMING_SYSTEM.md`
2. **Run Benchmarks**: Execute `benchmark_cymatic_timing`
3. **Review Examples**: Check test files for more examples
4. **Integrate**: Follow integration guide in main documentation

---

**Quick Reference**:
- Work Distribution: `cllm_work_distribution_timing.h`
- Epoch Sync: `cllm_epoch_sync.h`
- Benchmarks: `tests/benchmark_cymatic_timing.c`
- Full Docs: `docs/CYMATIC_TIMING_SYSTEM.md`