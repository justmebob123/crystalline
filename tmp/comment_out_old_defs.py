#!/usr/bin/env python3

with open('src/ai/cllm_inference.c', 'r') as f:
    lines = f.readlines()

# Find and comment out old function definitions
in_old_function = False
function_start = -1
brace_count = 0

new_lines = []
i = 0
while i < len(lines):
    line = lines[i]
    
    # Check if this is an old float-based function definition we want to remove
    if ('void cllm_layer_norm_old(' in line or 
        'void cllm_layer_norm_bigfixed(float* x' in line or
        'void cllm_feedforward_bigfixed(float* x' in line or
        'void cllm_attention_forward_bigfixed(AttentionLayer* layer, float* input' in line):
        # Start commenting out
        in_old_function = True
        function_start = i
        brace_count = 0
        new_lines.append('/* OLD STUB - REMOVED\n')
        new_lines.append(line)
        i += 1
        continue
    
    if in_old_function:
        new_lines.append(line)
        # Count braces
        brace_count += line.count('{')
        brace_count -= line.count('}')
        
        # If we've closed all braces, end the comment
        if brace_count == 0 and '}' in line:
            new_lines.append('END OLD STUB */\n')
            in_old_function = False
        i += 1
        continue
    
    new_lines.append(line)
    i += 1

with open('src/ai/cllm_inference.c', 'w') as f:
    f.writelines(new_lines)

print("✅ Commented out old function definitions")
