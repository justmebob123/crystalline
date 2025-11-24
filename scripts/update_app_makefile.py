#!/usr/bin/env python3
"""Update app/Makefile to link with libcrawler.so"""

with open('app/Makefile', 'r') as f:
    content = f.read()

# Update LDFLAGS
content = content.replace(
    'LDFLAGS = -L.. -lprimemath -lcrawler -lSDL2 -lSDL2_ttf -lm -lpthread',
    'LDFLAGS = -L.. -lprimemath -lcrawler -lSDL2 -lSDL2_ttf -lm -lpthread'
)

# Update rpath
content = content.replace(
    '-Wl,-rpath,..',
    '-Wl,-rpath,..,-rpath,.'
)

with open('app/Makefile', 'w') as f:
    f.write(content)

print("✓ app/Makefile updated successfully")