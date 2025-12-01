#!/usr/bin/env python3
import re

with open('src/ai/cllm_inference.c', 'r') as f:
    content = f.read()

# Fix the attention call - add precision parameter
# The inference structure doesn't have precision, so we'll use a default
old_call = r'cllm_attention_forward_bigfixed\(attn_layer, inference->hidden_states, attn_output, NULL, NULL, 1\);'
new_call = 'cllm_attention_forward_bigfixed(attn_layer, inference->hidden_states, attn_output, NULL, NULL, 1, 128);  // Default precision'

content = content.replace(old_call, new_call)

# Also need to fix the feedforward call - it needs the layer parameter first
# Check the signature
old_ff = r'cllm_feedforward_bigfixed\(inference->hidden_states, &model->ff_layers\[layer\]\);'
new_ff = 'cllm_feedforward_bigfixed(&model->ff_layers[layer], inference->hidden_states, inference->hidden_states, 128);  // in-place, default precision'

content = re.sub(old_ff, new_ff, content)

with open('src/ai/cllm_inference.c', 'w') as f:
    f.write(content)

print("✅ Fixed inference function calls")
