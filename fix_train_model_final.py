#!/usr/bin/env python3
"""
Properly fix train_model.c to create threaded system once.
"""

with open('tools/train_model.c.backup', 'r') as f:
    content = f.read()

# Find and replace the training loop section
# We need to:
# 1. Create batch_iterator and threaded_system BEFORE the for loop
# 2. Remove their creation from INSIDE the for loop
# 3. Add reset call at start of each epoch
# 4. Add cleanup AFTER the for loop

old_section = '''    // Training loop
    time_t start_time = time(NULL);
    
    for (int epoch = 0; epoch < config->num_epochs; epoch++) {
        training->current_epoch = epoch;
        
        printf("╔══════════════════════════════════════════════════════════╗\\n");
        printf("║ Epoch %d/%d                                               ║\\n", 
               epoch + 1, config->num_epochs);
        printf("╚══════════════════════════════════════════════════════════╝\\n\\n");
        
        // Use kissing spheres multi-threaded training for better performance
        float epoch_loss;
        // ALWAYS use kissing spheres architecture (no single-threaded fallback)
        printf("DEBUG: training->tokens = %p, training->num_tokens = %zu\\n", 
               (void*)training->tokens, training->num_tokens);
        printf("DEBUG: batch_size = %d, sequence_length = %d\\n",
               config->batch_size, config->sequence_length);
        
        // Create batch iterator for kissing spheres architecture
        printf("DEBUG: About to create batch_iterator\\n");
        fflush(stdout);
        
        CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create(
            training->tokens,
            training->num_tokens,
            config->batch_size,
            config->sequence_length,
            0,  // shuffle = false (maintain order)
            0   // drop_last = false (use all data)
        );
        
        printf("DEBUG: batch_iterator = %p\\n", (void*)batch_iterator);
        fflush(stdout);
        
        printf("DEBUG: About to create threaded_system...\\n");
        fflush(stdout);
        
        if (!batch_iterator) {
            fprintf(stderr, "\\u2717 CRITICAL ERROR: Failed to create batch iterator\\n");
            fprintf(stderr, "Cannot proceed without batch processing\\n");
            goto cleanup;
        }
        
        // Use kissing spheres architecture (MANDATORY)
        printf("DEBUG: About to create threaded_system with %d threads\\n", num_threads);
        fflush(stdout);
        
        ThreadedTrainingSystem* threaded_system = threaded_training_create(
            training,
            batch_iterator,
            num_threads
        );
        
        printf("DEBUG: threaded_system = %p\\n", (void*)threaded_system);
        fflush(stdout);
        
        if (!threaded_system) {
            fprintf(stderr, "\\u2717 CRITICAL ERROR: Failed to create threaded system\\n");
            fprintf(stderr, "Cannot proceed without kissing spheres architecture\\n");
            cllm_batch_iterator_free(batch_iterator);
            goto cleanup;
        }
        
        printf("\\u2713 Using Kissing Spheres Architecture with %d worker threads\\n", 
               threaded_training_get_num_workers(threaded_system));
        epoch_loss = threaded_train_epoch(threaded_system);
        threaded_training_print_stats(threaded_system);
        threaded_training_free(threaded_system);
        cllm_batch_iterator_free(batch_iterator);'''

new_section = '''    // Training loop
    time_t start_time = time(NULL);
    
    // PERFORMANCE FIX: Create threaded system ONCE before epoch loop
    printf("Creating batch iterator (reusable for all epochs)...\\n");
    CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create(
        training->tokens,
        training->num_tokens,
        config->batch_size,
        config->sequence_length,
        0,  // shuffle = false (maintain order)
        0   // drop_last = false (use all data)
    );
    
    if (!batch_iterator) {
        fprintf(stderr, "Failed to create batch iterator\\n");
        goto cleanup;
    }
    
    printf("Creating threaded system with %d threads (ONCE for all epochs)...\\n", num_threads);
    ThreadedTrainingSystem* threaded_system = threaded_training_create(
        training,
        batch_iterator,
        num_threads
    );
    
    if (!threaded_system) {
        cllm_batch_iterator_free(batch_iterator);
        fprintf(stderr, "Failed to create threaded system\\n");
        goto cleanup;
    }
    
    printf("✓ Threaded system created with %d workers (reusable)\\n\\n", num_threads);
    
    for (int epoch = 0; epoch < config->num_epochs; epoch++) {
        training->current_epoch = epoch;
        
        printf("╔══════════════════════════════════════════════════════════╗\\n");
        printf("║ Epoch %d/%d                                               ║\\n", 
               epoch + 1, config->num_epochs);
        printf("╚══════════════════════════════════════════════════════════╝\\n\\n");
        
        // Reset batch iterator for new epoch
        cllm_batch_iterator_reset(batch_iterator);
        
        // Train one epoch (reusing the same threaded system)
        float epoch_loss = threaded_train_epoch(threaded_system);
        threaded_training_print_stats(threaded_system);'''

content = content.replace(old_section, new_section)

# Now add cleanup after the for loop (before "time_t end_time")
old_end = '''    }
    
    time_t end_time = time(NULL);'''

new_end = '''    }
    
    // Cleanup threaded system after all epochs
    threaded_training_free(threaded_system);
    cllm_batch_iterator_free(batch_iterator);
    
    time_t end_time = time(NULL);'''

content = content.replace(old_end, new_end)

with open('tools/train_model.c', 'w') as f:
    f.write(content)

print("✓ Successfully fixed train_model.c")
print("  - Threaded system now created once before epoch loop")
print("  - Batch iterator reset at start of each epoch")
print("  - Cleanup added after all epochs complete")
