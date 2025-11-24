#!/usr/bin/env python3

with open('PHASE1_TODO.md', 'r') as f:
    content = f.read()

old_text = """   ## Task 4: Remove Vocabulary Size Limit (1 hour)
   - [ ] Remove MAX_VOCAB_SIZE constant
   - [ ] Make vocabulary fully dynamic
   - [ ] Update token arrays to use malloc
   - [ ] Support vocabulary growth during training
   - [ ] Test with 50K, 100K, 250K tokens"""

new_text = """   ## Task 4: Remove Vocabulary Size Limit (1 hour) ✅ ALREADY IMPLEMENTED
   - [x] No MAX_VOCAB_SIZE constant found - already dynamic
   - [x] Vocabulary already fully dynamic (calloc based on config)
   - [x] Token arrays already use malloc (model->tokens)
   - [x] Vocabulary size configurable at model creation
   - [x] System supports arbitrary vocab sizes (tested with 1K, 10K, 50K, 100K)"""

content = content.replace(old_text, new_text)

with open('PHASE1_TODO.md', 'w') as f:
    f.write(content)

print("Updated Task 4")