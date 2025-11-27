#!/bin/bash

echo "# Training Performance Analysis"
echo ""

# Get current progress
current_batch=$(grep "Processing batch group" training_run.log | tail -1 | grep -oP '\d+' | head -1)
total_batches=424
elapsed_minutes=26.7

echo "## Progress"
echo "- Current batch group: $current_batch / $total_batches"
echo "- Progress: $((current_batch * 100 / total_batches))%"
echo "- Elapsed time: $elapsed_minutes minutes"
echo "- Batches per minute: $(echo "scale=2; $current_batch / $elapsed_minutes" | bc)"
echo "- Estimated total time: $(echo "scale=1; $total_batches / ($current_batch / $elapsed_minutes)" | bc) minutes"
echo ""

echo "## CPU Usage"
echo "- Threads configured: 8"
echo "- CPU usage: 220% (2.2 cores)"
echo "- **PROBLEM: Only 27.5% of available threads being used!**"
echo "- Expected: 800% (8 cores)"
echo "- Actual: 220% (2.2 cores)"
echo "- Efficiency: 27.5%"
echo ""

echo "## Memory Usage"
echo "- RSS: 23 MB (actual RAM used)"
echo "- VIRT: 598 MB (virtual memory)"
echo "- **Memory usage is LOW - not memory-bound**"
echo ""

echo "## Bottleneck Analysis"
echo ""
echo "### NOT Memory-Bound"
echo "- Only using 23 MB RAM"
echo "- Expected ~100 MB for 8 threads"
echo "- Memory is NOT the bottleneck"
echo ""

echo "### Thread Utilization Problem"
echo "- 8 threads configured"
echo "- Only 2.2 cores active (27.5%)"
echo "- **5.8 cores are IDLE**"
echo ""

echo "### Possible Causes:"
echo "1. **Thread synchronization issues** - Threads waiting on locks"
echo "2. **Sequential bottleneck** - Some operation not parallelized"
echo "3. **I/O bound** - Waiting on disk/memory access"
echo "4. **Batch size too small** - Not enough work per thread"
echo ""

echo "## Recommendations"
echo "1. Check for mutex contention (threads waiting on locks)"
echo "2. Profile to find sequential bottleneck"
echo "3. Increase batch size to give threads more work"
echo "4. Check if gradient accumulation is sequential"
