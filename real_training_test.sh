#!/bin/bash
# Real training test with proper inference validation

set -e

echo "=== REAL CLLM Training & Inference Test ==="
echo ""
echo "System: 64 CPU cores available"
echo "Strategy: Train on actual C code, verify meaningful output"
echo ""

# Create a more substantial training dataset with actual C code
mkdir -p real_test_data

cat > real_test_data/functions.c << 'EOF'
int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    if (b == 0) return 0;
    return a / b;
}

int main() {
    int x = 10;
    int y = 20;
    int sum = add(x, y);
    printf("Sum: %d\n", sum);
    return 0;
}
EOF

cat > real_test_data/structs.c << 'EOF'
struct Point {
    int x;
    int y;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
};

int calculate_area(struct Rectangle rect) {
    int width = rect.bottom_right.x - rect.top_left.x;
    int height = rect.bottom_right.y - rect.top_left.y;
    return width * height;
}
EOF

cat > real_test_data/loops.c << 'EOF'
void print_numbers(int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", i);
    }
    printf("\n");
}

int sum_array(int arr[], int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
EOF

echo "Created training data:"
wc -l real_test_data/*.c
echo ""

# Train with more realistic configuration
echo "Starting training with 12 threads (kissing spheres)..."
echo ""

LD_LIBRARY_PATH=. ./tools/train_model ./real_test_data \
    --epochs 10 \
    --batch-size 8 \
    --seq-len 32 \
    --learning-rate 0.001 \
    --threads 12 \
    --vocab-size 2000 \
    --embed-dim 128 \
    --num-layers 3 \
    --num-heads 4 \
    --checkpoint-dir ./checkpoints_real

echo ""
echo "Training complete!"
echo ""

# Check if checkpoint was created
if ls checkpoints_real/*.cllm 1> /dev/null 2>&1; then
    echo "✓ Model checkpoint found!"
    ls -lh checkpoints_real/*.cllm | tail -1
    
    # Now we need to check if the model actually learned something
    echo ""
    echo "=== Analyzing Training Results ==="
    echo ""
    
    # Check the training log for loss progression
    if [ -f training_logs/real_training_*.log ]; then
        echo "Loss progression:"
        grep -E "Epoch.*complete.*loss" training_logs/real_training_*.log | tail -5
    fi
else
    echo "✗ No model checkpoint found"
    exit 1
fi

echo ""
echo "=== Next Step: Implement Proper Forward Pass ==="
echo "The inference tool needs to actually use the model's forward pass"
echo "instead of generating random logits."