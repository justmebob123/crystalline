#!/usr/bin/env python3
import re

with open('tools/train_model.c', 'r') as f:
    content = f.read()

# Find the training loop section and replace it
old_pattern = r'(// Training loop\s+time_t start_time = time\(NULL\);)\s+(for \(int epoch = 0; epoch < config->num_epochs; epoch\+\+\) \{)'

new_code = r'''\1
    
    // PERFORMANCE FIX: Create threaded system ONCE before epoch loop
    printf("Creating batch iterator (reusable for all epochs)...\\n");
    CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create(
        training->tokens, training->num_tokens,
        config->batch_size, config->sequence_length, 0, 0);
    
    if (!batch_iterator) {
        fprintf(stderr, "Failed to create batch iterator\\n");
        goto cleanup;
    }
    
    printf("Creating threaded system with %d threads (ONCE)...\\n", num_threads);
    ThreadedTrainingSystem* threaded_system = threaded_training_create(
        training, batch_iterator, num_threads);
    
    if (!threaded_system) {
        cllm_batch_iterator_free(batch_iterator);
        fprintf(stderr, "Failed to create threaded system\\n");
        goto cleanup;
    }
    
    printf("✓ Threaded system created with %d workers (reusable)\\n\\n", num_threads);
    
    \2'''

content = re.sub(old_pattern, new_code, content, flags=re.DOTALL)

# Now remove the batch_iterator and threaded_system creation/destruction inside the loop
# This is complex, so let me do it step by step

with open('tools/train_model_fixed.c', 'w') as f:
    f.write(content)

print("Created tools/train_model_fixed.c")
