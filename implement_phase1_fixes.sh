#!/bin/bash

echo "=== IMPLEMENTING PHASE 1 PERFORMANCE FIXES ==="
echo ""

# Fix 1: Add SIMD to gradient accumulation
echo "Fix 1: Optimizing gradient accumulation with SIMD..."
cat > fix_gradient_accumulation.patch << 'PATCH'
--- a/src/ai/cllm_training_threaded.c
+++ b/src/ai/cllm_training_threaded.c
@@ -1290,6 +1290,7 @@ static void accumulate_gradients_lockfree(ThreadedTrainingSystem* system) {
     
     // Accumulate gradients from all worker spheres
     int valid_spheres = 0;
+    
     for (int i = 0; i < system->num_worker_spheres; i++) {
         SphereTrainingContext* ctx = system->sphere_contexts[i];
         if (!ctx || !ctx->local_gradients) continue;
@@ -1307,8 +1308,18 @@ static void accumulate_gradients_lockfree(ThreadedTrainingSystem* system) {
         // Clip gradients to prevent overflow
         clip_gradients(ctx->local_gradients, ctx->gradient_size, 10.0f);
         
-        for (size_t j = 0; j < system->gradient_size; j++) {
-            system->accumulated_gradients[j] += ctx->local_gradients[j];
+        // PERFORMANCE FIX: Use SIMD for gradient accumulation
+        size_t j = 0;
+        
+        // Process 8 floats at a time with AVX
+        for (; j + 8 <= system->gradient_size; j += 8) {
+            __m256 acc = _mm256_loadu_ps(&system->accumulated_gradients[j]);
+            __m256 grad = _mm256_loadu_ps(&ctx->local_gradients[j]);
+            acc = _mm256_add_ps(acc, grad);
+            _mm256_storeu_ps(&system->accumulated_gradients[j], acc);
+        }
+        
+        // Handle remaining elements
+        for (; j < system->gradient_size; j++) {
+            system->accumulated_gradients[j] += ctx->local_gradients[j];
         }
         
         valid_spheres++;
PATCH

echo "  Created gradient accumulation patch"

# Fix 2: Remove excessive debug output
echo ""
echo "Fix 2: Removing excessive debug output..."
cat > remove_debug_output.patch << 'PATCH'
--- a/src/ai/cllm_training_threaded.c
+++ b/src/ai/cllm_training_threaded.c
@@ -1350,20 +1350,6 @@ float threaded_train_epoch(ThreadedTrainingSystem* system) {
     printf("\nStarting multi-threaded epoch training...\n");
     printf("Using %d worker threads for parallel batch processing\n\n", system->num_worker_spheres);
     
-    printf("DEBUG: About to reset batch iterator\n");
-    cllm_batch_iterator_reset(system->batch_iterator);
-    printf("DEBUG: Batch iterator reset complete\n");
+    cllm_batch_iterator_reset(system->batch_iterator);
     
     float epoch_loss = 0.0f;
     int batch_count = 0;
@@ -1374,40 +1360,20 @@ float threaded_train_epoch(ThreadedTrainingSystem* system) {
     
     while (1) {
-        printf("DEBUG: Allocating batch array for %d workers\n", system->num_worker_spheres); fflush(stdout);
         CLLMBatch** batches = (CLLMBatch**)calloc(system->num_worker_spheres, sizeof(CLLMBatch*));
-        if (!batches) {
-            printf("DEBUG: Failed to allocate batch array\n"); fflush(stdout);
-            break;
-        }
+        if (!batches) break;
         
         int batches_loaded = 0;
         
-        printf("DEBUG: Loading batches...\n"); fflush(stdout);
         // Load up to N batches (one per worker sphere)
         for (int i = 0; i < system->num_worker_spheres; i++) {
-            printf("DEBUG: Loading batch %d/%d\n", i+1, system->num_worker_spheres); fflush(stdout);
             batches[i] = cllm_batch_iterator_next(system->batch_iterator);
-            printf("DEBUG: Batch %d loaded: %p\n", i+1, (void*)batches[i]); fflush(stdout);
             if (batches[i]) {
                 batches_loaded++;
             } else {
-                // Iterator returned NULL - no more batches
-                printf("DEBUG: No more batches at index %d\n", i); fflush(stdout);
                 break;
             }
         }
-        printf("DEBUG: Loaded %d batches total\n", batches_loaded); fflush(stdout);
PATCH

echo "  Created debug removal patch"

# Fix 3: Optimize batch loading (prepare for parallelization)
echo ""
echo "Fix 3: Preparing batch loading optimization..."
echo "  (Will be implemented in Phase 2)"

echo ""
echo "=== PHASE 1 FIXES PREPARED ==="
echo ""
echo "To apply fixes:"
echo "  1. Review the patches above"
echo "  2. Apply manually or use patch command"
echo "  3. Rebuild: make clean && make"
echo "  4. Test performance improvement"
echo ""
echo "Expected improvement: 2-3x speedup"