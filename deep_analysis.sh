#!/bin/bash

echo "=== DEEP PERFORMANCE ANALYSIS SUITE ==="
echo ""

DATASET=$1
THREADS=${2:-64}
EPOCHS=${3:-5}

if [ -z "$DATASET" ]; then
    echo "Usage: $0 <dataset_dir> [threads] [epochs]"
    exit 1
fi

OUTPUT_DIR="analysis_results_$(date +%s)"
mkdir -p $OUTPUT_DIR

echo "Analysis output directory: $OUTPUT_DIR"
echo ""

# 1. CPU Profiling with perf
echo "1. Running with perf (CPU profiling)..."
perf record -g -o $OUTPUT_DIR/perf.data \
    ./tools/train_model $DATASET \
    --threads $THREADS --epochs $EPOCHS \
    --batch-size 16 --seq-len 64 --vocab-size 10000 \
    > $OUTPUT_DIR/training.log 2>&1

perf report -i $OUTPUT_DIR/perf.data > $OUTPUT_DIR/perf_report.txt 2>&1
echo "  ✓ Perf data saved to $OUTPUT_DIR/perf.data"

# 2. Memory profiling with valgrind massif
echo "2. Running with valgrind massif (memory profiling)..."
valgrind --tool=massif --massif-out-file=$OUTPUT_DIR/massif.out \
    ./tools/train_model $DATASET \
    --threads 4 --epochs 1 \
    --batch-size 8 --seq-len 32 --vocab-size 5000 \
    > $OUTPUT_DIR/memory.log 2>&1

ms_print $OUTPUT_DIR/massif.out > $OUTPUT_DIR/memory_report.txt
echo "  ✓ Memory profile saved to $OUTPUT_DIR/massif.out"

# 3. Cache analysis with perf
echo "3. Running cache analysis..."
perf stat -e cache-references,cache-misses,L1-dcache-loads,L1-dcache-load-misses \
    ./tools/train_model $DATASET \
    --threads $THREADS --epochs 2 \
    --batch-size 16 --seq-len 64 --vocab-size 10000 \
    > $OUTPUT_DIR/cache.log 2>&1

echo "  ✓ Cache stats saved to $OUTPUT_DIR/cache.log"

# 4. System call analysis with strace
echo "4. Running strace analysis..."
strace -c -o $OUTPUT_DIR/strace.txt \
    ./tools/train_model $DATASET \
    --threads 4 --epochs 1 \
    --batch-size 8 --seq-len 32 --vocab-size 5000 \
    > $OUTPUT_DIR/strace.log 2>&1

echo "  ✓ Syscall stats saved to $OUTPUT_DIR/strace.txt"

# 5. Thread analysis
echo "5. Analyzing thread behavior..."
# This will be done during runtime monitoring

echo ""
echo "=== ANALYSIS COMPLETE ==="
echo "Results in: $OUTPUT_DIR"
echo ""
echo "Key files:"
echo "  - perf_report.txt: CPU hotspots"
echo "  - memory_report.txt: Memory usage over time"
echo "  - cache.log: Cache hit/miss rates"
echo "  - strace.txt: System call overhead"
echo "  - training.log: Full training output"
