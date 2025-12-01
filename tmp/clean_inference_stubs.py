#!/usr/bin/env python3
import re

with open('src/ai/cllm_inference.c', 'r') as f:
    content = f.read()

# Remove old layer_norm_bigfixed stub (around line 225)
pattern = r'// Layer normalization \(old version for compatibility\)\s+void cllm_layer_norm_bigfixed\(float\* x, CLLMLayerNorm\* ln, uint32_t dim\) \{.*?\n    \}\n'
content = re.sub(pattern, '', content, flags=re.DOTALL)

# Remove any other old stubs
pattern2 = r'void cllm_layer_norm_old\(float\* x, CLLMLayerNorm\* ln, uint32_t dim\) \{.*?\n\}\n'
content = re.sub(pattern2, '', content, flags=re.DOTALL)

with open('src/ai/cllm_inference.c', 'w') as f:
    f.write(content)

print("✅ Cleaned all old stubs from inference")
