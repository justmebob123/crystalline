#!/bin/bash
# Add comprehensive debug output to threaded_training_create

FILE="src/ai/cllm_training_threaded.c"

# Backup original file
cp "$FILE" "${FILE}.backup"

# Use sed to add debug statements after key lines
# After function entry
sed -i '224a\    printf("DEBUG: [ENTER] threaded_training_create, num_threads=%d\\n", num_threads);\n    fflush(stdout);' "$FILE"

# After validation
sed -i '/if (!training || !batch_iterator) return NULL;/a\    printf("DEBUG: [STEP 1] Validation passed\\n");\n    fflush(stdout);' "$FILE"

# After thread count determination
sed -i '/if (num_threads < 1) num_threads = 1;/a\    printf("DEBUG: [STEP 2] Thread count: %d\\n", num_threads);\n    fflush(stdout);' "$FILE"

# After system allocation
sed -i '/ThreadedTrainingSystem\* system = (ThreadedTrainingSystem\*)calloc(1, sizeof(ThreadedTrainingSystem));/a\    printf("DEBUG: [STEP 3] System allocated: %p\\n", (void*)system);\n    fflush(stdout);' "$FILE"

# After gradient buffer creation
sed -i '/printf("  ✓ Created shared gradient buffer/a\    printf("DEBUG: [STEP 4] Gradient buffer created\\n");\n    fflush(stdout);' "$FILE"

# After barrier initialization
sed -i '/pthread_barrier_init(&system->batch_barrier, NULL, num_threads + 1);/a\    printf("DEBUG: [STEP 5] Barriers initialized (N+1=%d)\\n", num_threads + 1);\n    fflush(stdout);' "$FILE"

# After thread system creation
sed -i '/system->thread_system = threads_create(hierarchy_levels);/a\    printf("DEBUG: [STEP 6] Thread system created: %p\\n", (void*)system->thread_system);\n    fflush(stdout);' "$FILE"

# After sphere contexts allocation
sed -i '/system->sphere_contexts = (SphereTrainingContext\*\*)calloc(system->num_worker_spheres,/a\    printf("DEBUG: [STEP 7] Sphere contexts array allocated\\n");\n    fflush(stdout);' "$FILE"

# Before worker thread creation loop
sed -i '/printf("  Creating %d worker threads/a\    printf("DEBUG: [STEP 8] About to create worker threads\\n");\n    fflush(stdout);' "$FILE"

echo "Debug output added to $FILE"
echo "Original file backed up to ${FILE}.backup"