#!/bin/bash

echo "=== CRYSTALLINE CLLM PERFORMANCE ANALYSIS ==="
echo ""

export LD_LIBRARY_PATH=/workspace:$LD_LIBRARY_PATH

DATASET="data/large_dataset"
THREADS=64
EPOCHS=3

echo "Configuration:"
echo "  Dataset: $DATASET ($(du -h $DATASET/training_data.txt | cut -f1))"
echo "  Threads: $THREADS"
echo "  Epochs: $EPOCHS"
echo ""

# Create output directory
OUTPUT="perf_analysis_$(date +%Y%m%d_%H%M%S)"
mkdir -p $OUTPUT

# 1. Baseline run with timing
echo "1. Baseline Performance Test"
echo "   Running training with detailed timing..."
/usr/bin/time -v ./tools/train_model $DATASET \
    --threads $THREADS \
    --epochs $EPOCHS \
    --batch-size 32 \
    --seq-len 128 \
    --vocab-size 10000 \
    > $OUTPUT/baseline.log 2>&1

echo "   ✓ Baseline complete"
echo ""

# 2. CPU profiling with perf
echo "2. CPU Profiling (perf)"
echo "   Identifying hot functions..."
perf record -F 99 -g --call-graph dwarf \
    ./tools/train_model $DATASET \
    --threads $THREADS \
    --epochs 1 \
    --batch-size 32 \
    --seq-len 128 \
    --vocab-size 10000 \
    > $OUTPUT/perf_run.log 2>&1

perf report --stdio > $OUTPUT/perf_report.txt 2>&1
perf annotate --stdio > $OUTPUT/perf_annotate.txt 2>&1
echo "   ✓ CPU profile complete"
echo ""

# 3. Cache analysis
echo "3. Cache Performance Analysis"
echo "   Measuring cache hit rates..."
perf stat -e cache-references,cache-misses,L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,LLC-loads,LLC-load-misses,LLC-stores \
    ./tools/train_model $DATASET \
    --threads $THREADS \
    --epochs 1 \
    --batch-size 32 \
    --seq-len 128 \
    --vocab-size 10000 \
    2>&1 | tee $OUTPUT/cache_stats.txt

echo "   ✓ Cache analysis complete"
echo ""

# 4. SIMD verification
echo "4. SIMD Usage Verification"
echo "   Checking for vectorized instructions..."
objdump -d libcllm.so | grep -E "(vmovaps|vmulps|vaddps|vfmadd)" | head -n 20 > $OUTPUT/simd_instructions.txt
SIMD_COUNT=$(objdump -d libcllm.so | grep -E "(vmovaps|vmulps|vaddps|vfmadd)" | wc -l)
echo "   Found $SIMD_COUNT SIMD instructions in libcllm.so"
echo "   ✓ SIMD check complete"
echo ""

# 5. Memory bandwidth
echo "5. Memory Bandwidth Analysis"
echo "   Measuring memory access patterns..."
perf stat -e cpu/event=0xd1,umask=0x01/,cpu/event=0xd1,umask=0x02/,cpu/event=0xd1,umask=0x04/,cpu/event=0xd1,umask=0x08/ \
    ./tools/train_model $DATASET \
    --threads $THREADS \
    --epochs 1 \
    --batch-size 32 \
    --seq-len 128 \
    --vocab-size 10000 \
    2>&1 | tee $OUTPUT/memory_bandwidth.txt

echo "   ✓ Memory analysis complete"
echo ""

# 6. Thread utilization
echo "6. Thread Utilization Analysis"
echo "   Monitoring thread activity..."
# Run in background and monitor
./tools/train_model $DATASET \
    --threads $THREADS \
    --epochs 1 \
    --batch-size 32 \
    --seq-len 128 \
    --vocab-size 10000 \
    > $OUTPUT/thread_run.log 2>&1 &

PID=$!
sleep 5

# Sample thread states
for i in {1..10}; do
    echo "Sample $i:" >> $OUTPUT/thread_states.txt
    ps -T -p $PID | tail -n +2 >> $OUTPUT/thread_states.txt
    sleep 2
done

wait $PID
echo "   ✓ Thread analysis complete"
echo ""

# 7. Generate summary report
echo "7. Generating Summary Report"
cat > $OUTPUT/ANALYSIS_SUMMARY.md << 'SUMMARY'
# Performance Analysis Summary

## Test Configuration
- Dataset: 73MB text data
- Threads: 64
- Epochs: 3
- Batch size: 32
- Sequence length: 128

## Key Findings

### 1. Baseline Performance
See: baseline.log

### 2. CPU Hotspots
See: perf_report.txt
Top functions consuming CPU time.

### 3. Cache Performance
See: cache_stats.txt
Cache hit/miss rates and memory access patterns.

### 4. SIMD Usage
See: simd_instructions.txt
Verification of vectorized operations.

### 5. Memory Bandwidth
See: memory_bandwidth.txt
Memory access patterns and bandwidth utilization.

### 6. Thread Utilization
See: thread_states.txt
Thread activity and synchronization patterns.

## Recommendations
(To be filled based on analysis)

SUMMARY

echo "   ✓ Summary report created"
echo ""

echo "=== ANALYSIS COMPLETE ==="
echo ""
echo "Results directory: $OUTPUT"
echo ""
echo "Key files to review:"
echo "  1. baseline.log - Full training output with timing"
echo "  2. perf_report.txt - CPU hotspots"
echo "  3. cache_stats.txt - Cache performance"
echo "  4. simd_instructions.txt - SIMD verification"
echo "  5. ANALYSIS_SUMMARY.md - Summary report"
echo ""
echo "Next steps:"
echo "  1. Review perf_report.txt for top CPU consumers"
echo "  2. Check cache_stats.txt for cache miss rates"
echo "  3. Verify SIMD usage in simd_instructions.txt"
echo "  4. Analyze thread utilization patterns"
