#!/usr/bin/env python3
import os
import re

# Get all tool .c files
tool_files = []
for f in os.listdir('tools'):
    if f.endswith('.c'):
        tool_files.append(f)

# Read Makefile
with open('Makefile', 'r') as f:
    makefile = f.read()

# Check which tools are in Makefile
built_tools = []
unbuilt_tools = []

for tool in sorted(tool_files):
    tool_name = tool.replace('.c', '')
    # Check if tool is mentioned in Makefile
    if f'tools/{tool_name}' in makefile:
        built_tools.append(tool)
    else:
        unbuilt_tools.append(tool)

print(f"Total tools: {len(tool_files)}")
print(f"Built by Makefile: {len(built_tools)}")
print(f"NOT built: {len(unbuilt_tools)}")
print()

if unbuilt_tools:
    print("Tools NOT in Makefile:")
    for tool in unbuilt_tools:
        print(f"  - {tool}")
        # Check file size
        size = os.path.getsize(f'tools/{tool}')
        print(f"    Size: {size} bytes")

