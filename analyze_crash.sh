#!/bin/bash
echo "=== Installing GDB ==="
apt-get update && apt-get install -y gdb

echo "=== Rebuilding with debug symbols ==="
cd /workspace/crystalline
make clean

# Build library with debug symbols
CFLAGS="-Wall -Wextra -g -O0 -fPIC -I./include -mavx2 -mfma" make

# Build app with debug symbols
cd app
rm -f *.o ui/tabs/*.o hyper_prime_spiral
CFLAGS="-Wall -Wextra -g -O0 -std=c11 -I../include -I." make

echo "=== Creating test program ==="
cd /workspace/crystalline
cat > test_crystalline_crash.c << 'TESTEOF'
#include <stdio.h>
#include <stdlib.h>
#include "include/cllm.h"
#include "include/cllm_training.h"
#include "include/cllm_crystalline_training.h"

int main() {
    printf("=== Testing Crystalline Training ===\n");
    
    // Create minimal model
    CLLMConfig config = {
        .vocab_size = 611,
        .embedding_dim = 512,
        .num_layers = 6,
        .num_heads = 8,
        .head_dim = 64,
        .hidden_dim = 2048,
        .max_seq_len = 2048
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("Failed to create model\n");
        return 1;
    }
    
    // Create training config
    CLLMTrainingConfig train_config = {
        .batch_size = 4,
        .sequence_length = 32,
        .learning_rate = 0.001f,
        .num_epochs = 1,
        .save_every = 100
    };
    
    // Initialize training
    CLLMTraining* training = cllm_training_init(model, &train_config);
    if (!training) {
        printf("Failed to init training\n");
        return 1;
    }
    
    // Create dummy training data
    training->num_tokens = 128;
    training->tokens = (uint32_t*)malloc(128 * sizeof(uint32_t));
    for (int i = 0; i < 128; i++) {
        training->tokens[i] = i % 611;
    }
    training->total_batches = 1;
    
    printf("Model created, starting crystalline training...\n");
    
    // Allocate batch buffers
    uint32_t* input_tokens = (uint32_t*)malloc(128 * sizeof(uint32_t));
    uint32_t* target_tokens = (uint32_t*)malloc(128 * sizeof(uint32_t));
    
    for (int i = 0; i < 128; i++) {
        input_tokens[i] = i % 611;
        target_tokens[i] = (i + 1) % 611;
    }
    
    printf("Calling crystalline loss computation...\n");
    
    // This is where it crashes
    float loss = cllm_compute_loss_crystalline(training, input_tokens, target_tokens, 128);
    
    printf("Loss: %.4f\n", loss);
    
    free(input_tokens);
    free(target_tokens);
    cllm_training_cleanup(training);
    
    return 0;
}
TESTEOF

gcc -g -O0 -o test_crystalline_crash test_crystalline_crash.c -L. -lprimemath -lm -lpthread -Wl,-rpath,.

echo "=== Running test under GDB ==="
gdb -batch -ex "run" -ex "bt" -ex "info locals" -ex "quit" ./test_crystalline_crash

