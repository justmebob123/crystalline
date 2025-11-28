#!/usr/bin/env python3
"""
Fix train_model.c to create threaded system once instead of every epoch.
This is a CRITICAL performance fix.
"""

with open('tools/train_model.c', 'r') as f:
    lines = f.readlines()

# Find the training loop start
training_loop_start = None
for i, line in enumerate(lines):
    if '// Training loop' in line:
        training_loop_start = i
        break

if training_loop_start is None:
    print("ERROR: Could not find training loop")
    exit(1)

# Find the for loop
for_loop_line = None
for i in range(training_loop_start, min(training_loop_start + 10, len(lines))):
    if 'for (int epoch = 0' in lines[i]:
        for_loop_line = i
        break

if for_loop_line is None:
    print("ERROR: Could not find for loop")
    exit(1)

# Insert the setup code before the for loop
setup_code = '''
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
    
    printf("✓ Threaded system created with %d workers\\n\\n", num_threads);
    
'''

lines.insert(for_loop_line, setup_code)

# Now find and remove/comment the duplicate code inside the loop
# Look for the batch_iterator creation inside the loop
in_loop = False
loop_depth = 0
i = for_loop_line + 1
lines_to_remove = []

while i < len(lines):
    line = lines[i]
    
    # Track loop depth
    if '{' in line:
        loop_depth += line.count('{')
    if '}' in line:
        loop_depth -= line.count('}')
        if loop_depth == 0:
            break
    
    # Find the duplicate batch_iterator creation
    if 'CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create' in line:
        # Mark this and following lines for removal until we hit the training call
        j = i
        while j < len(lines) and 'threaded_train_epoch' not in lines[j]:
            if 'goto cleanup' in lines[j]:
                break
            lines_to_remove.append(j)
            j += 1
    
    # Find the free calls
    if 'threaded_training_free(threaded_system)' in line:
        lines_to_remove.append(i)
    if 'cllm_batch_iterator_free(batch_iterator)' in line and i not in lines_to_remove:
        lines_to_remove.append(i)
    
    i += 1

# Replace removed lines with a reset call
if lines_to_remove:
    # Keep only the first few lines (the epoch header), then add reset
    first_remove = min(lines_to_remove)
    
    # Find where to insert the reset (after the epoch printf)
    reset_line = None
    for i in range(for_loop_line + 1, first_remove):
        if 'Epoch %d/%d' in lines[i]:
            # Find the next line after the box
            for j in range(i, first_remove):
                if '╚' in lines[j]:
                    reset_line = j + 1
                    break
            break
    
    if reset_line:
        lines.insert(reset_line, '        // Reset batch iterator for new epoch\n')
        lines.insert(reset_line + 1, '        cllm_batch_iterator_reset(batch_iterator);\n')
        lines.insert(reset_line + 2, '        \n')
    
    # Comment out the removed lines
    for idx in sorted(lines_to_remove, reverse=True):
        if idx < len(lines):
            lines[idx] = '        // REMOVED (created once before loop): ' + lines[idx].lstrip()

# Add cleanup at the end of the loop (before the time calculation)
end_of_loop = for_loop_line
loop_depth = 0
for i in range(for_loop_line, len(lines)):
    if 'for (int epoch = 0' in lines[i]:
        loop_depth = 1
    elif loop_depth > 0:
        if '{' in lines[i]:
            loop_depth += lines[i].count('{')
        if '}' in lines[i]:
            loop_depth -= lines[i].count('}')
            if loop_depth == 0:
                end_of_loop = i
                break

# Insert cleanup before the loop end
cleanup_code = '''
    // Cleanup threaded system after all epochs
    threaded_training_free(threaded_system);
    cllm_batch_iterator_free(batch_iterator);
    
'''
lines.insert(end_of_loop, cleanup_code)

# Write the fixed file
with open('tools/train_model.c', 'w') as f:
    f.writelines(lines)

print("✓ Fixed train_model.c - threaded system now created once")
print(f"  - Setup code inserted at line {for_loop_line}")
print(f"  - Removed {len(lines_to_remove)} duplicate lines inside loop")
print(f"  - Added cleanup at end of loop")