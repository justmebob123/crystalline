#!/usr/bin/env python3
"""
Add minimal but effective debug output to identify hang location
"""

def main():
    with open('src/ai/cllm_training_threaded.c', 'r') as f:
        content = f.read()
    
    # Add debug at function entry - right after the opening brace
    content = content.replace(
        'int num_threads) {\n    if (!training',
        'int num_threads) {\n    printf("DEBUG: [ENTER] threaded_training_create, num_threads=%d\\n", num_threads); fflush(stdout);\n    if (!training'
    )
    
    # Add debug after barriers
    content = content.replace(
        'pthread_barrier_init(&amp;system->batch_barrier, NULL, num_threads + 1);\n    \n    // Create thread system',
        'pthread_barrier_init(&amp;system->batch_barrier, NULL, num_threads + 1);\n    printf("DEBUG: [STEP 1] Barriers initialized\\n"); fflush(stdout);\n    \n    // Create thread system'
    )
    
    # Add debug after thread system creation
    content = content.replace(
        'system->thread_system = threads_create(hierarchy_levels);\n    if (!system->thread_system)',
        'system->thread_system = threads_create(hierarchy_levels);\n    printf("DEBUG: [STEP 2] threads_create returned: %p\\n", (void*)system->thread_system); fflush(stdout);\n    if (!system->thread_system)'
    )
    
    # Add debug before worker thread creation
    content = content.replace(
        '// Create worker threads\n    printf("  Creating %d worker threads',
        '// Create worker threads\n    printf("DEBUG: [STEP 3] About to create worker threads\\n"); fflush(stdout);\n    printf("  Creating %d worker threads'
    )
    
    # Add debug in the thread creation loop
    content = content.replace(
        'for (int i = 0; i < system->num_worker_spheres; i++) {\n        int rc = pthread_create',
        'for (int i = 0; i < system->num_worker_spheres; i++) {\n        printf("DEBUG: [STEP 4] Creating thread %d/%d\\n", i+1, system->num_worker_spheres); fflush(stdout);\n        int rc = pthread_create'
    )
    
    with open('src/ai/cllm_training_threaded.c', 'w') as f:
        f.write(content)
    
    print("Minimal debug output added successfully")

if __name__ == '__main__':
    main()