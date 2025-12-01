#!/usr/bin/env python3
import re

with open('src/ai/cllm_training.c', 'r') as f:
    content = f.read()

# Remove the old float-based cllm_attention_forward_bigfixed stub
# This is around line 3391
pattern = r'void cllm_attention_forward_bigfixed\(AttentionLayer\* layer, float\* input, float\* output,\s+float\* key_cache, float\* value_cache, int seq_len\) \{.*?\n\}\n'
content = re.sub(pattern, '', content, flags=re.DOTALL)

with open('src/ai/cllm_training.c', 'w') as f:
    f.write(content)

print("✅ Removed old attention stub from training")
