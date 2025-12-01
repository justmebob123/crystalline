#!/usr/bin/env python3
import re

with open('src/ai/cllm_inference.c', 'r') as f:
    content = f.read()

# Remove the old float-based cllm_feedforward_bigfixed stub (lines 256-290 approximately)
# Find and remove the entire function
pattern = r'// Feed-forward network\s+void cllm_feedforward_bigfixed\(float\* x, FeedForwardLayer\* ff\) \{.*?\n\}\n'
content = re.sub(pattern, '', content, flags=re.DOTALL)

# Also remove the old layer_norm_old function if it exists
pattern2 = r'void cllm_layer_norm_old\(float\* x, CLLMLayerNorm\* ln, uint32_t dim\) \{.*?\n\}\n'
content = re.sub(pattern2, '', content, flags=re.DOTALL)

with open('src/ai/cllm_inference.c', 'w') as f:
    f.write(content)

print("✅ Removed old float-based stubs from inference")
