#!/usr/bin/env python3
"""
Fix the training allocation to use packed arrays instead of BigFixed**
"""

# Read the file
with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Find and replace the allocation section
old_code = '''       } else if (total_params > 0 && total_params < 100000000) {
           // Standard mode: Allocate full gradient buffers using BigFixed arrays
           // CRITICAL FIX: Use bigfixed_array_create() not individual BigFixed structures!
           // bigfixed_array_create() creates ONE array, not millions of individual structures
           printf("Allocating gradient buffers for %zu parameters (BigFixed arrays)\\n", total_params);
           
           // Use BigFixed array utilities (efficient - single allocation)
           training->gradients = bigfixed_array_create(total_params, training->precision_bits);
           training->optimizer_state = bigfixed_array_create(total_params * 2, training->precision_bits);
           
           if (!training->gradients || !training->optimizer_state) {
               fprintf(stderr, "Failed to allocate gradient buffers\\n");
               if (training->gradients) bigfixed_array_free(training->gradients, total_params);
               if (training->optimizer_state) bigfixed_array_free(training->optimizer_state, total_params * 2);
               training->gradients = NULL;
               training->optimizer_state = NULL;
           } else {
               printf("✓ Gradient buffers allocated successfully\\n");
           }'''

new_code = '''       } else if (total_params > 0 && total_params < 100000000) {
           // Standard mode: Allocate full gradient buffers using PACKED arrays
           // CRITICAL FIX: Use packed arrays instead of BigFixed** arrays!
           // Memory savings: 22M × 208 bytes → 22M × 16 bytes (13x reduction!)
           printf("Allocating gradient buffers for %zu parameters (PACKED arrays)\\n", total_params);
           
           // Calculate expected memory usage
           size_t gradient_memory = total_params * 16;  // 16 bytes per element
           size_t optimizer_memory = (total_params * 2) * 16;  // 2x for momentum+variance
           size_t total_memory = gradient_memory + optimizer_memory;
           
           printf("  Expected memory: %.2f MB (gradients) + %.2f MB (optimizer) = %.2f MB total\\n",
                  gradient_memory / (1024.0 * 1024.0),
                  optimizer_memory / (1024.0 * 1024.0),
                  total_memory / (1024.0 * 1024.0));
           
           // Use packed array utilities (13x more efficient than BigFixed**)
           training->gradients = bigfixed_packed_array_create(total_params, training->precision_bits);
           training->optimizer_state = bigfixed_packed_array_create(total_params * 2, training->precision_bits);
           
           if (!training->gradients || !training->optimizer_state) {
               fprintf(stderr, "Failed to allocate gradient buffers\\n");
               if (training->gradients) bigfixed_packed_array_free((BigFixedPackedArray*)training->gradients);
               if (training->optimizer_state) bigfixed_packed_array_free((BigFixedPackedArray*)training->optimizer_state);
               training->gradients = NULL;
               training->optimizer_state = NULL;
           } else {
               printf("✓ Gradient buffers allocated successfully\\n");
           }'''

# Replace
if old_code in content:
    content = content.replace(old_code, new_code)
    print("✓ Replacement successful")
else:
    print("✗ Old code not found - trying without Unicode character")
    # Try without the checkmark
    old_code_no_unicode = old_code.replace('✓', '\\u2713')
    if old_code_no_unicode in content:
        content = content.replace(old_code_no_unicode, new_code)
        print("✓ Replacement successful (without Unicode)")
    else:
        print("✗ Still not found - manual inspection needed")
        # Show what we're looking for
        print("\nLooking for lines around 286-304...")
        lines = content.split('\n')
        for i in range(285, 305):
            if i < len(lines):
                print(f"{i+1}: {repr(lines[i][:80])}")

# Write back
with open('src/ai/cllm_training.c', 'w') as f:
    f.write(content)

print("\nFile updated")