#!/bin/bash

# Fix pointer type warnings by changing float* to double* in local variable declarations

# List of files with warnings
files=(
    "src/ai/cllm_clock_embeddings.c"
    "src/ai/cllm_crystalline_advanced.c"
    "src/ai/cllm_embedding.c"
    "src/ai/cllm_feedforward.c"
    "src/ai/cllm_inference.c"
    "src/ai/cllm_lattice_cache.c"
    "src/ai/cllm_lattice_embed.c"
    "src/ai/cllm_lattice_embeddings.c"
    "src/ai/cllm_lattice_lookup.c"
    "src/ai/cllm_layernorm.c"
    "src/ai/cllm_lll_embeddings.c"
    "src/ai/cllm_positional.c"
    "src/ai/cllm_training.c"
    "src/ai/cllm_training_threaded.c"
)

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "Processing $file..."
        # This is a complex task - need to manually fix each file
        # For now, just report the file
    fi
done

echo "Manual fixes required for each file"