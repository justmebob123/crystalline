#!/usr/bin/env python3
"""
Add comprehensive debug output to threaded_training_create function
"""

def add_debug_output():
    with open('src/ai/cllm_training_threaded.c', 'r') as f:
        lines = f.readlines()
    
    new_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]
        new_lines.append(line)
        
        # After function entry (line with "int num_threads) {")
        if 'int num_threads) {' in line and 'threaded_training_create' in lines[i-2]:
            new_lines.append('    printf("DEBUG: [ENTER] threaded_training_create, num_threads=%d\\n", num_threads);\n')
            new_lines.append('    fflush(stdout);\n')
        
        # After validation
        elif 'if (!training || !batch_iterator) return NULL;' in line:
            new_lines.append('    printf("DEBUG: [STEP 1] Validation passed\\n");\n')
            new_lines.append('    fflush(stdout);\n')
        
        # After thread count determination
        elif 'if (num_threads < 1) num_threads = 1;' in line:
            new_lines.append('    printf("DEBUG: [STEP 2] Thread count: %d\\n", num_threads);\n')
            new_lines.append('    fflush(stdout);\n')
        
        # After system allocation
        elif 'ThreadedTrainingSystem* system = (ThreadedTrainingSystem*)calloc(1, sizeof(ThreadedTrainingSystem));' in line:
            new_lines.append('    printf("DEBUG: [STEP 3] System allocated: %p\\n", (void*)system);\n')
            new_lines.append('    fflush(stdout);\n')
        
        # After gradient buffer message
        elif 'Created shared gradient buffer:' in line:
            # Add after the next line (which completes the printf)
            i += 1
            new_lines.append(lines[i])
            new_lines.append('    printf("DEBUG: [STEP 4] Gradient buffer created\\n");\n')
            new_lines.append('    fflush(stdout);\n')
        
        # After barrier initialization
        elif 'pthread_barrier_init(&system->batch_barrier, NULL, num_threads + 1);' in line:
            new_lines.append('    printf("DEBUG: [STEP 5] Barriers initialized (N+1=%d)\\n", num_threads + 1);\n')
            new_lines.append('    fflush(stdout);\n')
        
        # After thread system creation
        elif 'system->thread_system = threads_create(hierarchy_levels);' in line:
            new_lines.append('    printf("DEBUG: [STEP 6] Thread system created: %p\\n", (void*)system->thread_system);\n')
            new_lines.append('    fflush(stdout);\n')
        
        # After sphere contexts allocation (look for the line after calloc)
        elif 'system->sphere_contexts = (SphereTrainingContext**)calloc(system->num_worker_spheres,' in line:
            # Skip to the end of this statement
            while ');' not in lines[i]:
                i += 1
                new_lines.append(lines[i])
            new_lines.append('    printf("DEBUG: [STEP 7] Sphere contexts array allocated\\n");\n')
            new_lines.append('    fflush(stdout);\n')
        
        # Before worker thread creation loop
        elif 'Creating %d worker threads' in line:
            # Add after this printf
            i += 1
            new_lines.append(lines[i])
            new_lines.append('    printf("DEBUG: [STEP 8] About to create worker threads\\n");\n')
            new_lines.append('    fflush(stdout);\n')
        
        i += 1
    
    with open('src/ai/cllm_training_threaded.c', 'w') as f:
        f.writelines(new_lines)
    
    print("Debug output added successfully")

if __name__ == '__main__':
    add_debug_output()