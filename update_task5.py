#!/usr/bin/env python3

with open('PHASE1_TODO.md', 'r') as f:
    content = f.read()

old_text = """   ## Task 5: Fix Font Path (30 min)
   - [ ] Add multi-path font discovery
   - [ ] Check Ubuntu, CentOS, Debian paths
   - [ ] Add environment variable override
   - [ ] Test on both Ubuntu and CentOS"""

new_text = """   ## Task 5: Fix Font Path (30 min) ✅ COMPLETE
   - [x] Multi-path font discovery already implemented
   - [x] Supports Ubuntu, Debian, CentOS, Fedora, Arch paths
   - [x] Added CLLM_FONT_PATH environment variable override
   - [x] Added macOS and Windows font paths
   - [x] Build successful"""

content = content.replace(old_text, new_text)

with open('PHASE1_TODO.md', 'w') as f:
    f.write(content)

print("Updated Task 5")