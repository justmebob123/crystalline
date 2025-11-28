#!/usr/bin/env python3
with open('tools/train_model.c', 'r') as f:
    lines = f.readlines()

# Find the "// Training loop" line
training_loop_line = None
for i, line in enumerate(lines):
    if '// Training loop' in line and 'time_t start_time' in lines[i+1]:
        training_loop_line = i
        break

if not training_loop_line:
    print("ERROR: Could not find training loop")
    exit(1)

print(f"Found training loop at line {training_loop_line}")

# Find the for loop
for_loop_line = None
for i in range(training_loop_line, training_loop_line + 10):
    if 'for (int epoch = 0' in lines[i]:
        for_loop_line = i
        break

print(f"Found for loop at line {for_loop_line}")

# Insert setup code before for loop
setup = [
    '\n',
    '    // PERFORMANCE FIX: Create threaded system ONCE before epoch loop\n',
    '    CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create(\n',
    '        training->tokens, training->num_tokens,\n',
    '        config->batch_size, config->sequence_length, 0, 0);\n',
    '    if (!batch_iterator) {\n',
    '        fprintf(stderr, "Failed to create batch iterator\\n");\n',
    '        goto cleanup;\n',
    '    }\n',
    '    \n',
    '    ThreadedTrainingSystem* threaded_system = threaded_training_create(\n',
    '        training, batch_iterator, num_threads);\n',
    '    if (!threaded_system) {\n',
    '        cllm_batch_iterator_free(batch_iterator);\n',
    '        fprintf(stderr, "Failed to create threaded system\\n");\n',
    '        goto cleanup;\n',
    '    }\n',
    '    printf("✓ Threaded system with %d workers created (reusable)\\n\\n", num_threads);\n',
    '\n',
]

for line in reversed(setup):
    lines.insert(for_loop_line, line)

# Find and comment out the duplicate creation inside loop
i = for_loop_line + len(setup)
in_creation = False
lines_to_comment = []

while i < len(lines):
    line = lines[i]
    
    # Start of duplicate creation
    if 'CLLMBatchIterator* batch_iterator = cllm_batch_iterator_create' in line:
        in_creation = True
        lines_to_comment.append(i)
    elif in_creation:
        lines_to_comment.append(i)
        # End when we hit the training call
        if 'threaded_train_epoch' in line:
            in_creation = False
            break
    
    # Also mark the free calls for commenting
    if 'threaded_training_free(threaded_system)' in line:
        lines_to_comment.append(i)
    if 'cllm_batch_iterator_free(batch_iterator)' in line and i > for_loop_line + len(setup) + 10:
        lines_to_comment.append(i)
    
    i += 1
    if i > for_loop_line + 200:  # Safety limit
        break

print(f"Found {len(lines_to_comment)} lines to comment out")

# Comment out duplicate lines and add reset
first_comment = min(lines_to_comment) if lines_to_comment else None
if first_comment:
    # Find the epoch header (after the for loop)
    for i in range(for_loop_line + len(setup), first_comment):
        if '╚' in lines[i]:  # End of epoch box
            # Insert reset after the box
            lines.insert(i + 1, '        // Reset batch iterator for new epoch\n')
            lines.insert(i + 2, '        cllm_batch_iterator_reset(batch_iterator);\n')
            lines.insert(i + 3, '\n')
            break
    
    # Comment out the duplicate lines
    for idx in sorted(set(lines_to_comment), reverse=True):
        if idx < len(lines) and 'REMOVED' not in lines[idx]:
            lines[idx] = '        // REMOVED: ' + lines[idx].lstrip()

# Find end of for loop and add cleanup
loop_depth = 0
for i in range(for_loop_line + len(setup), len(lines)):
    if 'for (int epoch = 0' in lines[i]:
        loop_depth = 1
    elif loop_depth > 0:
        loop_depth += lines[i].count('{') - lines[i].count('}')
        if loop_depth == 0:
            # Found end of loop
            lines.insert(i, '\n')
            lines.insert(i + 1, '    // Cleanup threaded system after all epochs\n')
            lines.insert(i + 2, '    threaded_training_free(threaded_system);\n')
            lines.insert(i + 3, '    cllm_batch_iterator_free(batch_iterator);\n')
            lines.insert(i + 4, '\n')
            print(f"Added cleanup at line {i}")
            break

with open('tools/train_model.c', 'w') as f:
    f.writelines(lines)

print("✓ Fix applied successfully")
